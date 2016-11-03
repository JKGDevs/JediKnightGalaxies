// Copyright (C) 2011 Jedi Knight Galaxies
// jkg_equip.c: Handles weapon/armor equipping procedures.
// File by eezstreet

#include "g_local.h"

void initACI(gclient_t *client)
{
    memset (&client->coreStats.ACISlots, 0, sizeof (client->coreStats.ACISlots));
    client->coreStats.aciSlotsUsed = 0;
}

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

	auto item = (*ent->inventory)[iNum];
	if (item.id->itemType == ITEM_WEAPON)
	{
	    int prevEquipped = -1;

		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (!it->id) {
				continue;
			}
			if (it->id->itemType == ITEM_WEAPON && it->equipped) {
				it->equipped = false;
				prevEquipped = it - ent->inventory->begin();
				break;
			}
		}

		(*ent->inventory)[iNum].equipped = true;
	    trap->SendServerCommand (ent->s.number, va ("ieq %d %d", iNum, prevEquipped));
		trap->SendServerCommand (ent->s.number, va ("chw %d", item.id->weaponData.varID));
	}
	else if (item.id->itemType == ITEM_ARMOR){
	    // Unequip the armor which is currently equipped at the slot the new armor will use.
	    int prevEquipped = -1;

		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (iNum == it - ent->inventory->begin()) {
				continue;
			}

			if (it->id->itemType == ITEM_ARMOR && it->equipped && item.id->armorData.armorSlot == it->id->armorData.armorSlot) {
				it->equipped = false;
				prevEquipped = it - ent->inventory->begin();
				break;
			}
		}

		(*ent->inventory)[iNum].equipped = true;
		ent->client->armorItems[item.id->armorData.armorSlot] = iNum;

		trap->SendServerCommand( ent->s.number, va("ieq %d %d", iNum, prevEquipped ));
		trap->SendServerCommand( -1, va("aequi %i %i %i", ent->client->ps.clientNum, item.id->armorData.armorSlot, item.id->armorData.armorID));
	}
	else
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"You cannot equip that item.\n\"");
	}
}

void JKG_UnequipItem(gentity_t *ent, int iNum)
{
	if(!ent->client)
		return;

	if ( iNum < 0 || iNum >= ent->inventory->size() )
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Invalid inventory slot.\n\"");
	    return;
	}

	if(!(*ent->inventory)[iNum].equipped)
	{
		//trap->SendServerCommand(ent->client->ps.clientNum, "print \"That item is not equipped.\n\"");
		return;
	}

	if((*ent->inventory)[iNum].id->itemType == ITEM_WEAPON)
	{
		(*ent->inventory)[iNum].equipped = qfalse;
	    trap->SendServerCommand (ent->s.number, va ("iueq %i", iNum));
	    trap->SendServerCommand (ent->s.number, "chw 0");
	}
	else if((*ent->inventory)[iNum].id->itemType == ITEM_ARMOR)
	{
		(*ent->inventory)[iNum].equipped = qfalse;
		ent->client->armorItems[(*ent->inventory)[iNum].id->armorData.armorSlot] = 0;
		trap->SendServerCommand (ent->s.number, va ("iueq %i", iNum));
		trap->SendServerCommand(-1, va("aequi %i %i 0", ent->client->ps.clientNum, (*ent->inventory)[iNum].id->armorData.armorSlot));
	}
	else
	{
		//trap->SendServerCommand(ent->client->ps.clientNum, "print \"You cannot unequip that item.\n\"");
	}
}
