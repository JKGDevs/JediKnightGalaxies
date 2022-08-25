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
	if (ent->client->ps.stats[STAT_MAX_SHIELD] <= ent->client->ps.stats[STAT_SHIELD] && ent->client->ps.stats[STAT_SHIELD] != 0) {
		// If we're at max shield, and upgrading capacity, increase our shield amount to match
		ent->client->ps.stats[STAT_SHIELD] = item->id->shieldData.capacity;
	}
	item->equipped = qtrue;
	ent->client->ps.stats[STAT_MAX_SHIELD] = item->id->shieldData.capacity;
	ent->client->shieldEquipped = qtrue;
	ent->client->shieldRechargeLast = ent->client->shieldRegenLast = level.time;
	ent->client->shieldRechargeTime = item->id->shieldData.cooldown;
	ent->client->shieldRecharging = qfalse;
	ent->client->shieldRegenTime = item->id->shieldData.regenrate;

	if (playSound && item->id->shieldData.equippedSoundEffect[0]) {
		G_Sound(ent, CHAN_AUTO, G_SoundIndex(item->id->shieldData.equippedSoundEffect));
	}
}

/*
====================================
Cmd_ShieldUnequipped

====================================
*/
void Cmd_ShieldUnequipped(gentity_t* ent)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (ent->client->shieldEquipped) {
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
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

//overloaded version - if you know the shield's index start there
void Cmd_ShieldUnequipped(gentity_t* ent, unsigned int index)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (index > ent->inventory->size())
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
}

/*
====================================
Cmd_JetpackUnequipped

====================================
*/
void Cmd_JetpackUnequipped(gentity_t* ent)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}
	// Iterate through the inventory and remove the jetpack that is equipped
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); it++) {
		if (it->equipped && it->id->itemType == ITEM_JETPACK) {
			it->equipped = qfalse;
		}
	}

	ent->client->jetpackEquipped = qfalse;
	ent->client->pItemJetpack = nullptr;
	ent->client->ps.jetpack = 0;
}

//overloaded version - if you know the jetpack's index start there
void Cmd_JetpackUnequipped(gentity_t* ent, unsigned int index)
{
	if (JKG_HasFreezingBuff(ent->playerState)) //no changing equipment while stunned/frozen etc
	{
		return;
	}

	if (index > ent->inventory->size())
	{
		trap->SendServerCommand(ent - g_entities, "Cmd_ShieldUnequipped() called with out of bounds index! Defaulting to 0.\n");
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
	
	bool haveFullHP = false;
	if (ent->client->ps.stats[STAT_HEALTH] >= ent->client->ps.stats[STAT_MAX_HEALTH]) //check if we have full health already
		haveFullHP = true;

	ent->client->ps.stats[STAT_MAX_HEALTH] = ent->client->pers.maxHealth;
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		if (it->equipped && it->id->itemType == ITEM_ARMOR) {
			ent->client->ps.stats[STAT_MAX_HEALTH] += it->id->armorData.pArm->hp;
		}
	}
	if (haveFullHP)
	{
		if(ent->client->ps.stats[STAT_HEALTH] < ent->client->ps.stats[STAT_MAX_HEALTH])
			ent->health = ent->client->ps.stats[STAT_HEALTH] = ent->client->ps.stats[STAT_MAX_HEALTH]; //set current health to same as maximum
	}
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
	else if (item.id->itemType == ITEM_ARMOR)
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
				if (it->equipped && it->id->itemType == ITEM_ARMOR
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
	else if (item.id->itemType == ITEM_JETPACK) {
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
#define JETPACK_TOGGLE_TIME		200
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
{ //create effects?
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
	if (ent->client->ps.powerups[PW_REDFLAG] || ent->client->ps.powerups[PW_BLUEFLAG])
	{
		//can't activate certain jetpacks while carrying flag
		if(!jet->move.loadBearingAllowed)
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
					G_PlayEffectID(G_EffectIndex( "effects/Player/electrocute.efx"), higher, ent->client->ps.viewangles);
					return;
				}
			}
		}
	}

	
	if (jet->visuals.activateSound[0])
		G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.activateSound));

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

	if (ent->client->jetPackToggleTime >= level.time)
	{
		return;
	}

	if (ent->health <= 0 ||
		ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		(ent->client->ps.eFlags & EF_DEAD) ||
		ent->client->ps.pm_type == PM_DEAD)
	{ //can't use it when dead under any circumstances.
		return;
	}

	if (!ent->client->ps.jetpack) {
		// they don't have a jetpack equipped
		return;
	}

	if (!(ent->client->ps.eFlags & EF_JETPACK_ACTIVE) &&
		ent->client->ps.jetpackFuel < 5)
	{ //too low on fuel to start it up
		jetpackData_t* jet = &jetpackTable[ent->client->ps.jetpack - 1];
		if(jet->visuals.sputterSound[0])
			G_Sound(ent, CHAN_AUTO, G_SoundIndex(jet->visuals.sputterSound));

		return;
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
					return;
				}
			}
		}
	}

	if (ent->client->ps.eFlags & EF_JETPACK_ACTIVE)
	{
		Jetpack_Off(ent);
	}
	else
	{
		Jetpack_On(ent);
	}

	ent->client->jetPackToggleTime = level.time + JETPACK_TOGGLE_TIME;
}