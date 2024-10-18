// cg_crossover.c -- Crossover API module for CGame
// Copyright (c) 2013 Jedi Knight Galaxies

#include "cg_local.h"
#include "../ui/ui_shared.h"

uiCrossoverExports_t *uiImports;
cgCrossoverExports_t co;

/*
 *	Returns the index of the Gang Wars team that is in use by the map (red team)
 */
int CO_GetRedTeam( void )
{
	return cgs.redTeam;
}

/*
 *	Returns the index of the Gang Wars team that is in use by the map (blue team)
 */
int CO_GetBlueTeam( void )
{
	return cgs.blueTeam;
}

/*
 *	Returns qtrue if trapEscape is tripped (and sends an ~esc if it is)
 */
qboolean CO_EscapeTrapped( void )
{
	if( cg.trapEscape )
	{
		trap->SendClientCommand("~esc");
		return qtrue;
	}
	return qfalse;
}

/*
 *	Either puts an item into an ACI slot, or clears an ACI slot.
 *	If slot is -1, it will pick the first available one.
 *	If attach is qtrue, it will fill the slot (otherwise it will clear it)
 */
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
//cgImports->SendClientCommand(va("unequip %d", pItems[nSelected].first)); --futuza: <--implement something like this above?
//cgImports->SendClientCommand(va("equip %d", pItems[nSelected].first));

/*
 *	Performs a UI -> CG request for inventory and container UIs
 */
static size_t tempSize = 0;
static void *CO_InventoryDataRequest ( jkgInventoryRequest_e data, int extra )
{
    switch ( data )
    {
        case INVENTORYREQUEST_SIZE:
			tempSize = cg.playerInventory->size();
			return &tempSize;
		case INVENTORYREQUEST_ITEMS:
			return (void *)(&(*cg.playerInventory)[0]);
        case INVENTORYREQUEST_ACI:
            return cg.playerACI;
		case INVENTORYREQUEST_CREDITS:
			return &cg.predictedPlayerState.credits;
		case INVENTORYREQUEST_OTHERCONTAINERITEMS:
			return (void*)(&(*cg.otherTradeItems)[0]);
		case INVENTORYREQUEST_OTHERCONTAINERSIZE:
			tempSize = cg.otherTradeItems->size();
			return &tempSize;
		case INVENTORYREQUEST_LOOKUPTABLE:
			return itemLookupTable;
		case INVENTORYREQUEST_ICONSHADER:
			return (*cg.playerInventory)[extra].id->visuals.itemIcon;
		case INVENTORYREQUEST_DURABILITY:
			tempSize = (*cg.playerInventory)[extra].durability;
			return &tempSize;
        default:
            return NULL;
    }
}

/*
 *	Performs a UI -> CG request for inventory and container UIs
 */
static void *CO_PartyMngtDataRequest(jkgPartyRequest_e data) {
	// Team Management data request from UI
	if (data == PARTYREQUEST_PARTY) {
		return &cgs.party;
	} else if (data == PARTYREQUEST_SEEKERS) {
		return &cgs.partyList;
	} else if (data == PARTYREQUEST_SEEKERTIME) {
		return /*(void *)*/&cgs.partyListTime;
	} else {
		return 0;
	}
}

/*
 *	Gets the predicted player state
 */
static playerState_t* CO_GetPredictedPlayerState() {
	return &cg.predictedPlayerState;
}

/*
 *	Initializes the UI -> CG communication vector (Crossover API)
 */
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
	co.GetPredictedPlayerState = CO_GetPredictedPlayerState;

	uiImports = trap->CO_InitCrossover( &co );
}
