// cg_crossover.c -- Crossover API module for CGame
// Copyright (c) 2013 Jedi Knight Galaxies

#include "cg_local.h"
#include "../ui/ui_shared.h"

uiCrossoverExports_t *uiImports;

cgCrossoverExports_t co;

int CO_GetRedTeam( void )
{
	return cgs.redTeam;
}

int CO_GetBlueTeam( void )
{
	return cgs.blueTeam;
}

qboolean CO_EscapeTrapped( void )
{
	if( cg.trapEscape )
	{
		trap->SendClientCommand("~esc");
		return qtrue;
	}
	return qfalse;
}

// TODO: put inventory/shop/pazaak crap into appropriate files, this is really bad --eez
static void CO_InventoryAttachToACI ( int itemNum, int slot, int attach )
{
    if ( attach )
    {
        JKG_CG_FillACISlot (itemNum, slot);
    }
	else if (slot == -1)
    {
		// No slot specified, find the first one that has this item
		for (int i = 0; i < MAX_ACI_SLOTS; i++) {
			if (cg.playerACI[i] == itemNum) {
				JKG_CG_ClearACISlot(i);
				return;
			}
		}
	}
	else
	{
		JKG_CG_ClearACISlot(slot);
	}
}

static size_t tempSize = 0;
static void *CO_InventoryDataRequest ( int data )
{
	if(data >= 50)
	{
		//HACK ALERT
		if((*cg.playerInventory)[data-50].id && (data-50) >= 0 && (data-50) < MAX_INVENTORY_ITEMS)
		{
			return (*cg.playerInventory)[data-50].id->visuals.itemIcon;
		}
		else
		{
			return nullptr;
		}
	}
    switch ( data )
    {	// FIXME: enumerable --eez
        case 0: // inventory count
			tempSize = cg.playerInventory->size();
			return &tempSize;
		case 1: // inventory list
			return (void *)(&(*cg.playerInventory)[0]);	// feels like hack
        case 2:
            return cg.playerACI;
		case 3:
			return &cg.predictedPlayerState.credits;
		case 4:
			return (void*)(&(*cg.otherTradeItems)[0]);
		case 5:
			tempSize = cg.otherTradeItems->size();
			return &tempSize;
		case 6:
			return itemLookupTable;
        default:
            return NULL;
    }
}

static void *CO_PartyMngtDataRequest(int data) {
	// FIXME: enumerable, this needs moved elsewhere also --eez
	// Team Management data request from UI
	// Data 0 = Current party/invitations
	// Data 1 = Seeking players
	// Data 2 = Last seeking players refresh time (needed for delta feed)
	if (data == 0) {
		return &cgs.party;
	} else if (data == 1) {
		return &cgs.partyList;
	} else if (data == 2) {
		return /*(void *)*/&cgs.partyListTime;
	} else {
		return 0;
	}
}

void CG_InitializeCrossoverAPI( void )
{
	co.GetBlueTeam = CO_GetBlueTeam;
	co.GetRedTeam = CO_GetRedTeam;
	co.EscapeTrapped = CO_EscapeTrapped;
	co.GetWeaponDatas = GetWeaponData;
	co.InventoryAttachToACI = CO_InventoryAttachToACI;
	co.InventoryDataRequest = CO_InventoryDataRequest;
	co.PartyMngtDataRequest = CO_PartyMngtDataRequest;
	co.SendClientCommand = trap->SendClientCommand;

	uiImports = trap->CO_InitCrossover( &co );
}
