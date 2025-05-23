// Copyright (C) 2011 Jedi Knight Galaxies
// jkg_equip.c: Handles weapon/armor equipping procedures.
// File by eezstreet

#include "g_local.h"

/*
====================================
JKG_ShieldEquipped

====================================
*/
void JKG_ShieldEquipped(gentity_t* ent, int shieldItemNumber, qboolean playSound) {
	if (shieldItemNumber < 0 || shieldItemNumber >= ent->inventory->size()) {
		trap->SendServerCommand(ent - g_entities, "print \"Invalid inventory index.\n\"");
		return;
	}

	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (ent->client->shieldEquipped) {
		// Already have a shield equipped. Mark the other shield as not being equipped.
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (it->equipped && it->id->itemType == ITEM_SHIELD) {
				it->equipped = qfalse;
			}
		}
	}

	itemInstance_t* item = &(*ent->inventory)[shieldItemNumber];
	item->equipped = qtrue;
	ent->client->ps.stats[STAT_MAX_SHIELD] = item->id->shieldData.pShieldData->capacity;
	ent->client->shieldEquipped = qtrue;
	ent->client->shieldRechargeLast = ent->client->shieldRegenLast = level.time;
	ent->client->shieldRechargeTime = item->id->shieldData.pShieldData->cooldown;
	ent->client->shieldRecharging = qfalse;
	ent->client->shieldRegenTime = item->id->shieldData.pShieldData->regenrate;

	if (playSound && item->id->shieldData.pShieldData->equippedSoundEffect[0]) {
		G_Sound(ent, CHAN_AUTO, G_SoundIndex(item->id->shieldData.pShieldData->equippedSoundEffect));
	}
}

/*
====================================
Cmd_ShieldUnequipped

====================================
*/
void Cmd_ShieldUnequipped(gentity_t* ent)
{
	Cmd_ShieldUnequipped(ent, 0);	//index not specified, start at beginning
}

//overloaded version - if you know the shield's index start there
void Cmd_ShieldUnequipped(gentity_t* ent, unsigned int index)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (index > ent->inventory->size() )
	{
		trap->SendServerCommand(ent - g_entities, "Cmd_ShieldUnequipped() called with out of bounds index! Defaulting to 0.\n");
		index = 0;
	}

	if (ent->client->shieldEquipped) {
		for (auto it = ent->inventory->begin()+index; it != ent->inventory->end(); ++it) {
			if (it->equipped && it->id->itemType == ITEM_SHIELD) {
				it->equipped = qfalse;
			}
		}
	}

	ent->client->ps.stats[STAT_MAX_SHIELD] = 0;
	ent->client->shieldEquipped = qfalse;
	ent->client->shieldRechargeLast = ent->client->shieldRegenLast = level.time;
	ent->client->shieldRecharging = qfalse;
	ent->client->shieldRegenTime = ent->client->shieldRechargeTime = 0;
}

/*
====================================
ItemUse_Shield

====================================
*/
void ItemUse_Shield(gentity_t* ent)
{
#ifdef _DEBUG
	trap->SendServerCommand(ent - g_entities, "print \"ItemUse_Shields() not implmented!\n\"");
#endif
	return; //disable this function for now it isn't ready!

	assert(ent && ent->client);

	//shield has to be equipped
	if (!ent->client->shieldEquipped) 
		return;

	//gotta be alive
	if (!JKG_ClientAlive(ent))
		return;
	
	// find out which shield is equipped
	shieldData_t* shield = nullptr;
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it)
	{
		if (it->equipped && it->id->itemType == ITEM_SHIELD)
		{
			shield = it->id->shieldData.pShieldData;
			break;
		}
	}
	assert(shield);	//no shield found, wut?

	//if shield has 50% charge
	if (ent->client->ps.stats[STAT_SHIELD] < ent->client->ps.stats[STAT_MAX_SHIELD] * 0.5)
	{
		trap->SendServerCommand(ent - g_entities, "print \"Insufficient charge to activate!\n\"");
		return;
	}

	//various shield checks examples, such as regen etc.
	/*
	if (ent->client->shieldRechargeLast + ent->client->shieldRechargeTime < level.time)
	{
		if (ent->client->shieldRegenLast + ent->client->shieldRegenTime < level.time)
		{
			ent->client->ps.stats[STAT_SHIELD]++;
			ent->client->shieldRegenLast = level.time + ent->client->shieldRegenTime;
		}

		if (!ent->client->shieldRecharging)
		{
			// In the previous frame, our shield was not recharging
			ent->client->shieldRecharging = qtrue;

			//how to play a shield noise
			if (shield->rechargeSoundEffect[0])
			{
				G_Sound(ent, CHAN_AUTO, G_SoundIndex(shield->rechargeSoundEffect));

				// Play the effect for shield recharging
				gentity_t* evEnt;
				evEnt = G_TempEntity(ent->r.currentOrigin, EV_SHIELD_RECHARGE);
				evEnt->s.otherEntityNum = ent->s.number;
			}
		}
	}
	else	//if full
	{
		if (ent->client->shieldRecharging)
		{
			ent->client->shieldRecharging = qfalse; // In the previous frame, our shield was recharging - we need to turn charging off
			if (shield->rechargeSoundEffect[0])
			{
				// Play the sound for 100% charge and effect for shield recharging
				G_Sound(ent, CHAN_AUTO, G_SoundIndex(shield->chargedSoundEffect));
				gentity_t* evEnt;
				evEnt = G_TempEntity(ent->r.currentOrigin, EV_SHIELD_RECHARGE);
				evEnt->s.otherEntityNum = ent->s.number;
			}
		}
	}
	*/

	//can't activate shield if empstaggered by debuff
	if (ent->client->ps.buffsActive)
	{
		for (int i = 0; i < PLAYERBUFF_BITS; i++)
		{
			if (ent->client->ps.buffsActive & (1 << i))
			{
				jkgBuff_t* pBuff = &buffTable[ent->client->ps.buffs[i].buffID];
				if (pBuff->passive.empstaggered)
				{
					vec3_t higher;
					VectorCopy(ent->client->ps.origin, higher);
					higher[2] += 20.0f;
					G_Sound(ent, CHAN_AUTO, G_SoundIndex("sound/effects/energy_crackle.wav"));
					G_PlayEffectID(G_EffectIndex("effects/Player/electrocute.efx"), higher, ent->client->ps.viewangles);
					return;
				}
			}
		}
	}

	//okay we got here, actually activate the shield effect

	//ent->client->jetPackToggleTime = level.time + jet->move.cooldown; //reset cooldown
}


/*
====================================
JKG_JetpackEquipped

====================================
*/
void JKG_JetpackEquipped(gentity_t* ent, int jetpackItemNumber) {
	if (jetpackItemNumber < 0 || jetpackItemNumber >= ent->inventory->size()) {
		trap->SendServerCommand(ent - g_entities, "print \"Invalid item number.\n\"");
		return;
	}

	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	itemInstance_t* item = &(*ent->inventory)[jetpackItemNumber];
	if (item->id->itemType != ITEM_JETPACK) {
		trap->SendServerCommand(ent - g_entities, "print \"That item is not a jetpack.\n\"");
		return;
	}

	// Unequip the previous jetpack first
	Cmd_JetpackUnequipped(ent);

	item->equipped = qtrue;
	ent->client->jetpackEquipped = qtrue;
	ent->client->pItemJetpack = &item->id->jetpackData;
	ent->client->ps.jetpack = ent->client->pItemJetpack->pJetpackData - jetpackTable + 1;
	if(ent->client->ps.jetpackFuel > jetpackTable[ent->client->ps.jetpack - 1].fuelCapacity)	//if we have more fuel than our capacity, reduce current fuel to that amount
		ent->client->ps.jetpackFuel = jetpackTable[ent->client->ps.jetpack - 1].fuelCapacity;	
}

/*
====================================
Cmd_JetpackUnequipped

====================================
*/
void Cmd_JetpackUnequipped(gentity_t* ent)
{
	Cmd_JetpackUnequipped(ent, 0);	//index not provided start at beginning
}

//overloaded version - if you know the jetpack's index start there
void Cmd_JetpackUnequipped(gentity_t* ent, unsigned int index)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (index > ent->inventory->size() )
	{
		trap->SendServerCommand(ent - g_entities, "Cmd_JetpackUnequipped() called with out of bounds index! Defaulting to 0.\n");
		index = 0;
	}

	// Iterate through the inventory and remove the jetpack that is equipped
	for (auto it = ent->inventory->begin()+index; it != ent->inventory->end(); it++) {
		if (it->equipped && it->id->itemType == ITEM_JETPACK) {
			it->equipped = qfalse;
		}
	}

	ent->client->jetpackEquipped = qfalse;
	ent->client->pItemJetpack = nullptr;
	ent->client->ps.jetpack = 0;
}

/*
====================================
JKG_ArmorChanged

Our armor has been changed. Recalculate stats.
====================================
*/
void JKG_ArmorChanged(gentity_t* ent) {
	
	bool haveFullHP = false, haveFullStamina = false, haveFilterEquip = false, haveAntitoxEquip = false;

	if (ent->client->ps.stats[STAT_HEALTH] >= ent->client->ps.stats[STAT_MAX_HEALTH]) //check if we have full health already
		haveFullHP = true;

	if (ent->playerState->forcePower >= ent->client->ps.stats[STAT_MAX_STAMINA])	//check if forcepower is full
		haveFullStamina = true;

	//reset maxhealth/stamina to max
	ent->client->ps.stats[STAT_MAX_HEALTH] = ent->client->pers.maxHealth;
	ent->client->ps.stats[STAT_MAX_STAMINA] = ent->client->pers.maxStamina;
	ent->client->ps.buffFilterActive = haveFilterEquip;
	ent->client->ps.buffAntitoxActive = haveAntitoxEquip;

	//loop through inventory and check for stat increases
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		if (it->equipped && (it->id->itemType == ITEM_ARMOR || it->id->itemType == ITEM_CLOTHING))
		{
			ent->client->ps.stats[STAT_MAX_HEALTH] += it->id->armorData.pArm->hp;
			ent->client->ps.stats[STAT_MAX_STAMINA] += it->id->armorData.pArm->stamina;

			if (it->id->armorData.pArm->filter)
			{
				haveFilterEquip = true;
			}

			if (it->id->armorData.pArm->antitoxin)
			{
				haveAntitoxEquip = true;
			}
		}
	}

	//set current health/stamina to same as maximum
	if (haveFullHP)
	{
		if(ent->client->ps.stats[STAT_HEALTH] < ent->client->ps.stats[STAT_MAX_HEALTH])
			ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH]; 
	}
	if (haveFullStamina)
	{
		if (ent->playerState->forcePower < ent->client->ps.stats[STAT_MAX_STAMINA])
			ent->playerState->forcePower = ent->client->ps.stats[STAT_MAX_STAMINA];
	}

	//set to false if we didn't have any, set to true if we had at least 1
	ent->client->filterEquipped = ent->client->ps.buffFilterActive = haveFilterEquip;
	ent->client->antitoxEquipped = ent->client->ps.buffAntitoxActive = haveAntitoxEquip;
}

/*
====================================
JKG_EquipItem

====================================
*/
void JKG_UnequipItem(gentity_t *ent, int iNum);
void JKG_EquipItem(gentity_t *ent, int iNum)
{
	if(!ent->client)
		return;

	if ( iNum < 0 || iNum >= ent->inventory->size() )
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Invalid inventory slot.\n\"");
	    return;
	}

	if((*ent->inventory)[iNum].equipped)
	{
		//trap->SendServerCommand(ent->client->ps.clientNum, "print \"That item is already equipped.\n\"");
		return;
	}

	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"You cannot change your equipment right now.\n\"");
		return;
	}

	auto item = (*ent->inventory)[iNum];
	if (item.id->itemType == ITEM_WEAPON)
	{
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (!it->id) {
				continue;
			}
			if (it->id->itemType == ITEM_WEAPON && it->equipped) {
				it->equipped = false;
				break;
			}
		}

		(*ent->inventory)[iNum].equipped = true;
	}
	else if (item.id->itemType == ITEM_ARMOR || item.id->itemType == ITEM_CLOTHING)
	{
		armorData_t* pArm = item.id->armorData.pArm;
		int previousArmor = ent->client->ps.armor[pArm->slot] - 1;
		ent->client->ps.armor[pArm->slot] = pArm - armorTable + 1;

		if (previousArmor >= 0)
		{
			// We just equipped a new piece of armor over top of our old one.
			// We need to iterate through the inventory and remove the old equipped item.
			for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it)
			{
				if ( (it->equipped && it->id->itemType == ITEM_ARMOR || item.id->itemType == ITEM_CLOTHING)
					&& it->id->armorData.pArm->slot == armorTable[previousArmor].slot) //if its equipped armor, and takes up the same slot type, we can only have one per slot
				{
					it->equipped = qfalse;
				}
			}
		}
		(*ent->inventory)[iNum].equipped = true; // set this armor piece to "equipped" so we know that we can unequip it later
		JKG_ArmorChanged(ent);
	}
	else if (item.id->itemType == ITEM_SHIELD) {
		JKG_ShieldEquipped(ent, iNum, qtrue);
	}
	else if (item.id->itemType == ITEM_JETPACK) 
	{
		JKG_JetpackEquipped(ent, iNum);
	}
	else
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"You cannot equip that item.\n\"");
		return;
	}
	BG_SendItemPacket(IPT_EQUIP, ent, nullptr, iNum, 0);
}

/*
====================================
JKG_UnequipItem

====================================
*/
void JKG_UnequipItem(gentity_t *ent, int iNum)
{
	itemInstance_t* item;
	if(!ent->client)
		return;

	if ( iNum < 0 || iNum >= ent->inventory->size() )
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Invalid inventory slot.\n\"");
	    return;
	}

	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"You cannot change your equipment right now.\n\"");
		return;
	}

	item = &(*ent->inventory)[iNum];

	if(!item->equipped)
	{
		return;
	}

	if(item->id->itemType == ITEM_WEAPON)
	{
		item->equipped = qfalse;
	    trap->SendServerCommand (ent->s.number, "chw 0");
	}
	else if(item->id->itemType == ITEM_ARMOR)
	{
		item->equipped = qfalse;
		ent->client->ps.armor[item->id->armorData.pArm->slot] = 0;
		JKG_ArmorChanged(ent);
	}
	else if (item->id->itemType == ITEM_SHIELD) {
		Cmd_ShieldUnequipped(ent);
	}
	else if (item->id->itemType == ITEM_JETPACK) {
		Cmd_JetpackUnequipped(ent);
	}
	BG_SendItemPacket(IPT_UNEQUIP, ent, nullptr, iNum, 0);
}

/*
====================================
Jetpack_Off

====================================
*/
void Jetpack_Off(gentity_t *ent)
{ //create effects?
	assert(ent && ent->client);

	ent->client->ps.eFlags &= ~EF_JETPACK_ACTIVE;

	jetpackData_t* jet = &jetpackTable[ent->client->ps.jetpack - 1];
	if (jet->visuals.deactivateSound[0])
		G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.deactivateSound));
}

/*
====================================
Jetpack_On

====================================
*/
void Jetpack_On(gentity_t *ent)
{
	assert(ent && ent->client);

	if (ent->client->ps.fd.forceGripBeingGripped >= level.time)
	{ //can't turn on during grip interval
		return;
	}

	if (ent->client->ps.fallingToDeath)
	{ //too late!
		return;
	}

	jetpackData_t* jet = &jetpackTable[ent->client->ps.jetpack - 1];
	if (jet->visuals.activateSound[0])
		G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.activateSound));

	int startupCost = static_cast<int>(jetpackTable[ent->client->ps.jetpack - 1].fuelCapacity * 0.1f); //costs 10% of fuelcapacity for startup
	//cost capped at 5, minimum of 1 required
	if (startupCost > 5)
		startupCost = 5;
	if (startupCost < 1)
		startupCost = 1;
	ent->client->ps.jetpackFuel -= startupCost;
	ent->client->ps.eFlags |= EF_JETPACK_ACTIVE;
}

/*
====================================
ItemUse_Jetpack

====================================
*/
void ItemUse_Jetpack(gentity_t *ent)
{
	assert(ent && ent->client);

	if (!ent->client->ps.jetpack) {
		// they don't have a jetpack equipped
		return;
	}

	//can't use it when dead under any circumstances.
	if (!JKG_ClientAlive(ent))
		return;

	jetpackData_t* jet = &jetpackTable[ent->client->ps.jetpack - 1];	//get jetpack data

	//still on cooldown
	if (ent->client->jetPackToggleTime >= level.time)
	{
		//if the jetpack is on - allow it to be turned off
		if (ent->client->ps.eFlags & EF_JETPACK_ACTIVE)
			;
		
		//can't turn back on till cooldown done
		else
		{
			if (jet->visuals.sputterSound[0])
				G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.sputterSound));

			return;
		}
	}

	//too low on fuel to start it up
	if (!(ent->client->ps.eFlags & EF_JETPACK_ACTIVE) && ent->client->ps.jetpackFuel < 5)
	{ 
		if(jet->visuals.sputterSound[0])
			G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.sputterSound));

		return;
	}

	//can't activate certain jetpacks while carrying flag
	if (ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG])
	{
		if (!jet->move.loadBearingAllowed)
		{
			if (jet->visuals.sputterSound[0])
				G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.sputterSound));
			return;
		}
	}

	//can't activate jetpack if empstaggered by debuff
	if (ent->client->ps.buffsActive)
	{
		for (int i = 0; i < PLAYERBUFF_BITS; i++)
		{
			if (ent->client->ps.buffsActive & (1 << i))
			{
				jkgBuff_t* pBuff = &buffTable[ent->client->ps.buffs[i].buffID];
				if (pBuff->passive.empstaggered)
				{
					vec3_t higher;
					VectorCopy(ent->client->ps.origin, higher);
					higher[2] += 20.0f;
					G_Sound(ent, CHAN_AUTO, G_SoundIndex("sound/effects/energy_crackle.wav"));
					G_PlayEffectID(G_EffectIndex("effects/Player/electrocute.efx"), higher, ent->client->ps.viewangles);
					Jetpack_Off(ent);	//switch it off
					return;
				}
			}
		}
	}

	//okay we got here, actually change the jetpacks on/off status
	if (ent->client->ps.eFlags & EF_JETPACK_ACTIVE)
	{
		Jetpack_Off(ent);
	}
	else
	{
		Jetpack_On(ent);
	}

	ent->client->jetPackToggleTime = level.time + jet->move.cooldown;
}