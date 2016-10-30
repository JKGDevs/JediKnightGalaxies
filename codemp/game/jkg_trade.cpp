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

	// Use the treasure class to pick items
	self->s.seed = Q_irand(0, 10000); // temp
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
		BG_SendTradePacket(IPT_TRADE, patron, npc, &(*npc->inventory)[0], npc->inventory->size(), 0);
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
extern qboolean NPC_VendorHasConversationSounds(gentity_t *conversationalist);
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
		else if (NPC_VendorHasConversationSounds(self))
		{// Override with generic chat sounds for this specific NPC...
			strcpy(filename, va("sound/conversation/%s/conversation00.mp3", self->NPC_type));
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
		trap->SendServerCommand(g_entities - activator, "print\"That vendor cannot be traded with (it lacks a treasure class..!)\"");
		activator->client->ps.useDelay = level.time + 500;
		return;
	}
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