#include "g_local.h"
#include "jkg_treasureclass.h"

using namespace std;

/*
====================
JKG_target_vendor_think

Run every time the vendor needs to refresh its stock.
====================
*/
static void JKG_target_vendor_think(gentity_t* self) {
	gentity_t* npc = nullptr;
	if (self->genericValue1 == -1) {
		// We weren't able to find the entity upon init, let's give it another shot now.
		npc = G_Find(npc, FOFS(targetname), self->target);
		if (npc != nullptr) {
			self->genericValue1 = npc->s.number;
			npc->genericValue1 = ENTITYNUM_NONE;
			npc->use = JKG_target_vendor_use;
			npc->r.svFlags |= SVF_PLAYER_USABLE;
		}
		else {
			self->nextthink = level.time + 5000;
			return;
		}
	}
	else {
		npc = &g_entities[self->genericValue1];
	}

	npc->flags |= FL_GODMODE;
	npc->flags |= FL_NOTARGET;
	npc->flags |= FL_NO_KNOCKBACK;

	TreasureClass* pTC = nullptr;
	vector<int> items;
	auto tc = umTreasureClasses.find(self->treasureclass);

	if (tc == umTreasureClasses.end()) {
		Com_Printf("couldn't find vendor treasure class: %s\n", self->targetname);
		return;
	}
	pTC = tc->second;

	//notify players when the shops refresh if announcing is enabled
	if(jkg_announceShopRefresh.integer > 0 && ((level.time - level.startTime) > jkg_shop_replenish_time.integer * 1000 -1))
	{
		trap->SendServerCommand(-1, va("chat 100 \"Vendor, %s, replenished their stock.", self->targetname));	//--futuza: maybe not send in chat? this'll do for now
	}

	// Use the treasure class to pick items
	self->s.seed = Q_irandSafe((time(0) % 10000), QRAND_MAX - 1) + Q_irand(0, 10000);		//take current time (so every game is different and we aren't just pseudo random), but also add pseudo random so each vendor is different  --futuza
	npc->inventory->clear();
	pTC->Pick(items, self->s.seed);

	// Add the items that we've picked to the vendor's inventory
	npc->inventory->clear();
	for (auto it = items.begin(); it != items.end(); ++it) {
		itemInstance_t item = BG_ItemInstance(*it, 1);
		BG_GiveItemNonNetworked(npc, item);
	}
	
	// If the vendor has a client using it, we need to make sure to update that client
	if (npc->genericValue1 != ENTITYNUM_NONE) {
		gentity_t* patron = &g_entities[npc->genericValue1];
		if (npc->inventory->size() == 0)
		{
			BG_SendTradePacket(IPT_TRADECANCEL, patron, npc, nullptr, 0, 0);
			npc->genericValue1 = ENTITYNUM_NONE;	// cancel the trade because we have no items
		}
		else
		{
			BG_SendTradePacket(IPT_TRADE, patron, npc, &(*npc->inventory)[0], npc->inventory->size(), 0);
		}
		
	}

	self->nextthink = level.time + (jkg_shop_replenish_time.integer * 1000);
}

/*
====================
JKG_target_vendor_use

Called when the vendor NPC is used, NOT the target_vendor associated with it
====================
*/
extern gentity_t	*NPC;
extern gNPC_t		*NPCInfo;
extern usercmd_t	ucmd;
extern qboolean NPC_FaceEntity(gentity_t *ent, qboolean doPitch);
extern void G_SoundOnEnt(gentity_t *ent, soundChannel_t channel, const char *soundPath);
extern qboolean NPC_VendorHasConversationSounds(gentity_t *conversationalist, char *name);
extern qboolean NPC_VendorHasVendorSound(gentity_t *conversationalist, char *name);
extern void NPC_ConversationAnimation(gentity_t *NPC);
void JKG_target_vendor_use(gentity_t* self, gentity_t* other, gentity_t* activator) {
	char			filename[256];

	//Checks
	if (activator->r.svFlags & SVF_BOT)
		return; // Bot's dont need this...

	if (!activator->client)
	{
		//trap->Print("VENDOR DEBUG: Vendor activator is not a client!\n");
		return;	//Check #1 - Is this a client?
	}

	if (activator->NPC)
	{
		//trap->Print("VENDOR DEBUG: Vendor has no NPC information!\n");
		return; //Check #2 - And not an NPC?
	}

	if (activator->client->ps.useDelay > level.time)
		return; // wait... dont spam shopopen!

	self->flags |= FL_GODMODE;
	self->flags |= FL_NOTARGET;
	self->flags |= FL_NO_KNOCKBACK;

	if (self->s.eType == ET_NPC)
	{
		// UQ1: Face the customer...
		NPC = self;
		NPCInfo = NPC->NPC;
		ucmd = NPC->client->pers.cmd;
		NPC_FaceEntity(activator, qfalse);

		if (NPC_VendorHasVendorSound(self, "welcome00"))
		{// This NPC has it's own vendor specific sound(s)...
			char	filename[256];
			int		max = 1;

			while (NPC_VendorHasVendorSound(self, va("welcome0%i", max))) max++;

			strcpy(filename, va("sound/vendor/%s/welcome0%i.mp3", self->NPC_type, irand(0, max - 1)));
			NPC_ConversationAnimation(self);
			G_SoundOnEnt(self, CHAN_VOICE_ATTEN, filename);
		}
		else if (NPC_VendorHasConversationSounds(self, "conversation00"))
		{// Override with generic chat sounds for this specific NPC...
			char filename[256];
			int max = 1;

			while (NPC_VendorHasConversationSounds(self, va("conversation0%i", max))) 
				max++;

			strcpy(filename, va("sound/conversation/civilian_%s/conversation0%i.mp3", self->NPC_type, irand(0, max - 1)));
			NPC_ConversationAnimation(self);
			G_SoundOnEnt(self, CHAN_VOICE_ATTEN, filename);
		}
		else
		{// Use generic shop open sound (english)... Meh! Couldn't find any... Needs to be looked into...
			strcpy(filename, va("sound/vendor/generic/welcome0%i.mp3", irand(0, 1)));
			NPC_ConversationAnimation(self);
			G_SoundOnEnt(self, CHAN_VOICE_ATTEN, filename);
		}
	}
	
	if (self->inventory->size() == 0) {
		trap->SendServerCommand(g_entities - activator, "print\"That vendor cannot be traded with (it lacks a treasure class..!)\n\"");
		activator->client->ps.useDelay = level.time + 500;
		return;
	}

	activator->flags |= FL_BUSYMODE; //mark activator as busy
	BG_SendTradePacket(IPT_TRADEOPEN, activator, self, &(*self->inventory)[0], self->inventory->size(), 0);

	if (self->s.eType == ET_NPC)
	{
		if (self->client->NPC_class == CLASS_TRAVELLING_VENDOR)
			self->NPC->walkDebounceTime = level.time + 60000; // UQ1: Wait 60 seconds before moving...
	}

	activator->client->ps.useDelay = level.time + 500;
	activator->client->currentTrader = self;
	activator->client->pmnomove = true;
	self->genericValue1 = activator->s.number;
}

/*
====================
JKG_SP_target_vendor

Vendor spawning function. Requires an NPC as a target.
====================
*/
void JKG_SP_target_vendor(gentity_t *ent) {

	char* tc;
	G_SpawnString("treasureclass", "genericvendor", &tc);
	if (Q_stricmp(tc, "genericvendor")) {
		tc = Q_strlwr(tc);
	}
	Q_strncpyz(ent->treasureclass, tc, sizeof(ent->treasureclass));

	gentity_t *targetVendor = NULL;
	ent->genericValue1 = -1;
	ent->think = JKG_target_vendor_think;
	ent->nextthink = level.time + 50;
	targetVendor = G_Find(targetVendor, FOFS(targetname), ent->target);
	if (targetVendor == nullptr) {
		return;
	}
	targetVendor->use = JKG_target_vendor_use;
	targetVendor->r.svFlags |= SVF_PLAYER_USABLE;
	targetVendor->flags |= FL_GODMODE;
	targetVendor->flags |= FL_NOTARGET;
	targetVendor->flags |= FL_NO_KNOCKBACK;

	// Generic value 1 = Vendor entity number
	// targetname gets set to the treasureclass. seems pretty hack but it's a good idea
	ent->genericValue1 = targetVendor->s.number;
	targetVendor->genericValue1 = ENTITYNUM_NONE;
}

/*
====================
JKG_MakeNPCVendor

Makes an NPC into a vendor (with the designated treasure class)
====================
*/
void JKG_MakeNPCVendor(gentity_t* ent, char* szTreasureClassName)
{
	szTreasureClassName = Q_strlwr(szTreasureClassName);
	Q_strncpyz(ent->treasureclass, szTreasureClassName, sizeof(ent->treasureclass));

	//--Futuza: FIXME spawned vendors, need to add thinking to spawned vendors to get them to refresh - this crashes right now
	/*ent->think = JKG_target_vendor_think;
	ent->nextthink = level.time + 50;*/


	ent->use = JKG_GenericVendorUse;
	ent->r.svFlags |= SVF_PLAYER_USABLE;
	ent->flags |= FL_GODMODE;
	ent->flags |= FL_NOTARGET;
	ent->flags |= FL_NO_KNOCKBACK;
	ent->bVendor = true;
	ent->s.seed = Q_irandSafe((time(0) % 10000), QRAND_MAX-1) + Q_irand(0, QRAND_MAX - 1);		//take current time (so every game is different and we aren't just pseudo random), but also add pseudo random so each vendor is different  --futuza
	ent->genericValue1 = ENTITYNUM_NONE;

	Com_Printf("Attempting to spawn npc vendor: %s\n", ent->targetname);

	JKG_RegenerateStock(ent);
}


/*
JKG_UnmakeNPCVendor

Removes the vendor properties of an NPC
*/
void JKG_UnmakeNPCVendor(gentity_t* ent)
{
	if (!ent)
	{
		return;
	}

	char* empty = "";
	Q_strncpyz(ent->treasureclass, empty, sizeof(ent->treasureclass));
	ent->bVendor = false;
}

/*
====================
JKG_GenericVendorUse

Gets called when a vendor (spawned vendor, not placed with map) is used.
====================
*/
void JKG_GenericVendorUse(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	self->genericValue1 = other->s.number;

	if (self->s.eType == ET_NPC)
	{
		NPC_FaceEntity(activator, qfalse);

		if (self->client->NPC_class == CLASS_TRAVELLING_VENDOR)
			self->NPC->walkDebounceTime = level.time + 60000; // UQ1: Wait 60 seconds before moving...
	}

	BG_SendTradePacket(IPT_TRADEOPEN, activator, self, &(*self->inventory)[0], self->inventory->size(), 0);

	activator->client->ps.useDelay = level.time + 500;
	activator->client->currentTrader = self;
	activator->client->pmnomove = true;
}

/*
====================
JKG_RegenerateStock

Regenerates the stock of a vendor (generic)
====================
*/
void JKG_RegenerateStock(gentity_t* ent)
{
	TreasureClass* pTC = nullptr;
	vector<int> items;
	auto tc = umTreasureClasses.find(ent->treasureclass);

	if (tc == umTreasureClasses.end()) {
		Com_Printf("couldn't find vendor treasure class: %s\n", ent->treasureclass);
		return;
	}
	pTC = tc->second;

	// Use the treasure class to pick items
	ent->s.seed = Q_irandSafe((time(0) % 10000), QRAND_MAX - 1) + Q_irand(0, QRAND_MAX - 1); // temp
	ent->inventory->clear();
	pTC->Pick(items, ent->s.seed);

	// Add the items that we've picked to the vendor's inventory
	for (auto it = items.begin(); it != items.end(); ++it) {
		itemInstance_t item = BG_ItemInstance(*it, 1);
		BG_GiveItemNonNetworked(ent, item);
	}

	// If the vendor has a client using it, we need to make sure to update that client
	if (ent->genericValue1 != ENTITYNUM_NONE) {
		gentity_t* patron = &g_entities[ent->genericValue1];
		if (ent->inventory->size() == 0)
		{
			BG_SendTradePacket(IPT_TRADECANCEL, patron, ent, nullptr, 0, 0);
			ent->genericValue1 = ENTITYNUM_NONE;	// cancel the trade because we have no items
		}
		else
		{
			BG_SendTradePacket(IPT_TRADE, patron, ent, &(*ent->inventory)[0], ent->inventory->size(), 0);
		}
	}

}