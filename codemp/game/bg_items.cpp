#include "qcommon/q_shared.h"
#include "bg_items.h"
#include "bg_weapons.h"
#if defined(_GAME)
    #include "g_local.h"
#elif defined(_CGAME)
    #include "../cgame/cg_local.h"
#endif
#include <json/cJSON.h>

itemData_t* itemLookupTable; //total list of items


const stringID_table_s itemPacketNames[] = {
	ENUM2STRING(IPT_ADD),
	ENUM2STRING(IPT_REM),
	ENUM2STRING(IPT_ADDACI),
	ENUM2STRING(IPT_CLEAR),
	ENUM2STRING(IPT_OPEN),
	ENUM2STRING(IPT_QUANT),
	ENUM2STRING(IPT_RESET),
	ENUM2STRING(IPT_EQUIP),
	ENUM2STRING(IPT_UNEQUIP),
	{ nullptr, IPT_NULL }
};

const stringID_table_s tradePacketNames[] = {
	ENUM2STRING(IPT_TRADE),
	ENUM2STRING(IPT_TRADEOPEN),
	ENUM2STRING(IPT_TRADETRANS),
	ENUM2STRING(IPT_TRADESINGLE),
	ENUM2STRING(IPT_TRADEREADY),
	ENUM2STRING(IPT_TRADECREDITS),
	{ nullptr, IPT_NULL }
};

/*
====================
BG_ItemPacketFromName
====================
*/
itemPacketType_t BG_ItemPacketFromName(const char* szPacketName) {
	int i = 0;
	while (itemPacketNames[i].name != nullptr) {
		if (!Q_stricmp(itemPacketNames[i].name, szPacketName)) {
			return (itemPacketType_t)itemPacketNames[i].id;
		}
		i++;
	}
	return IPT_NULL;
}

/*
====================
BG_ItemPacketName
====================
*/
const char* BG_ItemPacketName(const int itemPacket) {
	return itemPacketNames[itemPacket].name;
}

/*
====================
BG_TradePacketFromName
====================
*/
itemTradePacketType_t BG_TradePacketFromName(const char* szPacketName) {
	int i = 0;
	while (tradePacketNames[i].name != nullptr) {
		if (!Q_stricmp(tradePacketNames[i].name, szPacketName)) {
			return (itemTradePacketType_t)tradePacketNames[i].id;
		}
		i++;
	}
	return IPT_TRADECANCEL;
}

/*
====================
BG_TradePacketName
====================
*/
const char* BG_TradePacketName(const int itemPacket) {
	return tradePacketNames[itemPacket].name;
}

/*
====================
BG_GetNextFreeItemSlot
====================
*/
static int BG_GetNextFreeItemSlot ( void )
{
    int i = 1;
    for ( i = 1; i < MAX_ITEM_TABLE_SIZE; i++ )
    {
        if ( !itemLookupTable[i].itemID )
        {
            return i;
        }
    }
    
    return 0;
}

/*
====================
BG_GetItemByWeaponIndex
====================
*/
itemData_t *BG_GetItemByWeaponIndex(int weaponIndex)
{
	int i;
	for (i = 0; i < MAX_ITEM_TABLE_SIZE; i++)
	{
		itemData_t *item = &itemLookupTable[i];
		if (item->itemID && item->weaponData.varID == weaponIndex)
		{
			return item;
		}
	}

	return NULL;
}

/*
====================
BG_FindItemByInternal
====================
*/
int BG_FindItemByInternal(const char *internalName)
{
	for (int i = 1; i < MAX_ITEM_TABLE_SIZE; i++)
	{
		if (itemLookupTable[i].itemID)
		{
			if (Q_stricmp(itemLookupTable[i].internalName, internalName) == 0)
			{
				return i;
			}
		}
	}
	return 0;
}

/*
====================
BG_PrintItemList
====================
*/
void BG_PrintItemList(void)
{
	for (int i = 0; i < MAX_ITEM_TABLE_SIZE; i++)
	{
		if (itemLookupTable[i].itemID)
		{
			trap->Print("%8i\t%s\n", itemLookupTable[i].itemID, itemLookupTable[i].internalName);
		}
	}
}

/*
====================
BG_HasWeaponItem
====================
*/
qboolean BG_HasWeaponItem ( int clientNum, int weaponId )
{
	int weaponNum;
    // Hate having to do it like this, but seems like the only way atm...
#ifdef _CGAME
	for (auto it = cg.playerInventory->begin(); it != cg.playerInventory->end(); ++it)
    {
		if (!it->id || !it->id->itemID)
			break;
		if (it->id->itemType != ITEM_WEAPON)
			continue;
		weaponNum = BG_GetWeaponIndex(it->id->weaponData.weapon, it->id->weaponData.variation);
		if( weaponNum == weaponId )
			return qtrue;
    }
#else
    gentity_t *ent = &g_entities[clientNum];
	for(auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		if(!it->id || !it->id->itemID) {
			continue;
		}
		if(it->id->itemType != ITEM_WEAPON) {
			continue;
		}
		weaponNum = BG_GetWeaponIndex(it->id->weaponData.weapon, it->id->weaponData.variation);
		if(weaponNum == weaponId) {
			return qtrue;
		}
	}
#endif

    return qfalse;
}

/*
====================
BG_FindItemDataByName

Finds an item instance based on internal name
====================
*/
itemData_t* BG_FindItemDataByName(const char* internalName) {
	for (int i = 0; i < MAX_ITEM_TABLE_SIZE; i++) {
		itemData_t* pItemData = &itemLookupTable[i];
		if (pItemData->itemID == 0) {
			continue;
		}
		if (!Q_stricmp(pItemData->internalName, internalName)) {
			return pItemData;
		}
	}
	return nullptr;
}

/*
====================
BG_SendItemPacket

Server only-send an item packet
====================
*/
#ifdef _GAME
void BG_SendItemPacket(itemPacketType_t packetType, gentity_t* ent, void* memData, int intData, int intData2) {
	const char* packetName = BG_ItemPacketName(packetType);
	char packet[MAX_STRING_CHARS] = {0};
	switch(packetType) {
		case IPT_ADD:
		case IPT_ADDACI:
			{
				itemInstance_t* pItem = (itemInstance_t*)memData;
				Com_sprintf(packet, sizeof(packet), "pInv %s %i %i", packetName, pItem->id->itemID, pItem->quantity);
			}
			break;
		case IPT_REM:
		case IPT_EQUIP:
		case IPT_UNEQUIP:
			{
				int itemSlot = intData;
				Com_sprintf(packet, sizeof(packet), "pInv %s %i", packetName, itemSlot);
			}
			break;
		case IPT_OPEN:
		case IPT_CLEAR:
			Com_sprintf(packet, sizeof(packet), "pInv %s", packetName);
			break;
		case IPT_QUANT:
			Com_sprintf(packet, sizeof(packet), "pInv %s %i %i", packetName, intData, intData2);
			break;
		case IPT_RESET:
			Com_sprintf(packet, sizeof(packet), "pInv %s %i ", packetName, intData);
			{
				for(auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
					// big thanks to Daggo for fixing this
					Q_strncpyz(packet, va("%s %i %i", packet, it->id->itemID, it->quantity), sizeof(packet));
				}
			}
			break;
		default:
			break;
	}
	trap->SendServerCommand(ent - g_entities, packet);
}
#endif

/*
====================
BG_ReceivedItemPacket

Client only-received an item packet
====================
*/
#ifdef _CGAME
extern void JKG_CG_FillACISlot(int itemNum, int slot);
extern void JKG_CG_ACIPostFix(int itemSlot);

void BG_ReceivedItemPacket(itemPacketType_t packetType) {
	switch (packetType) {
		case IPT_ADD:
			{
				// Add the item to our inventory
				int itemID = atoi(CG_Argv(2));
				int itemQuantity = atoi(CG_Argv(3));
				itemInstance_t item = BG_ItemInstance(itemID, itemQuantity);
				BG_GiveItem(item);
			}
			break;
		case IPT_ADDACI:
			{
				// Add the item to our inventory
				int itemID = atoi(CG_Argv(2));
				int itemQuantity = atoi(CG_Argv(3));
				itemInstance_t item = BG_ItemInstance(itemID, itemQuantity);
				BG_GiveItem(item);
				JKG_CG_FillACISlot(cg.playerInventory->size() - 1, -1);	// WARNING: this should be avoided with items that can stack
			}
			break;
		case IPT_QUANT:
			{
				// Change the quantity on an item stack
				int itemStack = atoi(CG_Argv(2));
				int newQuant = atoi(CG_Argv(3));
				(*cg.playerInventory)[itemStack].quantity = newQuant;
			}
			break;
		case IPT_REM:
			// Remove the item from our inventory
			{
				int itemStack = atoi(CG_Argv(2));
				BG_RemoveItemStack(itemStack);
				JKG_CG_ACIPostFix(itemStack);
			}
			
			break;
		case IPT_CLEAR:
			// Clear the inventory
			cg.playerInventory->clear();
			break;
		case IPT_OPEN:
			// Open the inventory menu
			uiImports->InventoryNotify(INVENTORYNOTIFY_OPEN);
			break;
		case IPT_RESET:
			// Clear the inventory and fill it with fresh data (usually from a vid_restart)
			cg.playerInventory->clear();
			{
				int numItems = atoi(CG_Argv(2));
				for (int i = 0; i < numItems; i++) {
					int itemID = atoi(CG_Argv(3 + (2 * i)));
					int quant = atoi(CG_Argv(4 + (2 * i)));
					itemInstance_t item = BG_ItemInstance(itemID, quant);
					cg.playerInventory->push_back(item);
				}
			}
			break;
		case IPT_EQUIP:
			// Equipped an item
			{
				int invID = atoi(CG_Argv(2));
				if(invID < 0 || invID >= cg.playerInventory->size()) {
					return;
				}
				(*cg.playerInventory)[invID].equipped = qtrue;

				// In the case of armor, we need to be smart enough to recognize that we could be overriding
				// a previous piece of equipment. BUT, if we send two packets, there's no guarantee that they're in order.
				// So we simulate the behavior on the client.
				for (int i = 0; i < cg.playerInventory->size(); i++)
				{
					if (i != invID && (*cg.playerInventory)[i].id->itemType == ITEM_ARMOR)
					{
						if ((*cg.playerInventory)[i].id->armorData.pArm->slot == (*cg.playerInventory)[invID].id->armorData.pArm->slot) //if its equipped and takes up the same slot type, we can only have one per slot
						{
							(*cg.playerInventory)[i].equipped = qfalse;
						}
					}
				}
			}
			break;
		case IPT_UNEQUIP:
			// Unequipped an item
			{
				int invID = atoi(CG_Argv(2));
				if(invID < 0 || invID >= cg.playerInventory->size()) {
					return;
				}
				(*cg.playerInventory)[invID].equipped = qfalse;
			}
			break;
		default:
			break;
	}
}
#endif

/*
====================
BG_SendTradePacket

Server only-send a trade packet to the client
====================
*/
#ifdef _GAME
void BG_SendTradePacket(itemTradePacketType_t packetType, gentity_t* ent, gentity_t* other, void* memData, int intData, int intData2) {
	const char* packetName = BG_TradePacketName(packetType);
	char packet[MAX_STRING_CHARS] = {0};
	if(!ent || !other) {
		return;
	}
	switch(packetType) {
		case IPT_TRADE: // Sent when an item is modified. We need to resend all
		case IPT_TRADEOPEN: // Sent when we should open the trade menu
			{
				int numItems = intData;
				itemInstance_t* pItems = (itemInstance_t*)memData;
				Com_sprintf(packet, sizeof(packet), "pTrade %s %i %i ", packetName, other->s.number, numItems);
				for(int i = 0; i < numItems; i++) {
					itemInstance_t* pItem = &pItems[i];
					Q_strcat(packet, sizeof(packet), va("%i %i ", pItem->id->itemID, pItem->quantity));
				}
			}
			break;
		case IPT_TRADECANCEL:
		case IPT_TRADETRANS:
		case IPT_TRADEREADY:
			Com_sprintf(packet, sizeof(packet), "pTrade %s", packetType);
			break;
		case IPT_TRADESINGLE:
			{
				itemInstance_t* item = (itemInstance_t*)memData;
				int credits = intData;
				Com_sprintf(packet, sizeof(packet), "pTrade %s %i %i %i", packetName, credits, item->id->itemID, item->quantity);
			}
			break;
		case IPT_TRADECREDITS:
			{
				int credits = intData;
				Com_sprintf(packet, sizeof(packet), "pTrade %s %i", packetName, credits);
			}
			break;
	}
	trap->SendServerCommand(ent - g_entities, packet);
}
#endif

/*
====================
BG_ReceivedTradePacket

Server only-send a trade packet to the client
====================
*/
#ifdef _CGAME
extern void JKG_OpenShopMenu_f(void);
void BG_ReceivedTradePacket(itemTradePacketType_t packet) {
	switch (packet) {
		// TODO: generalize sending/receiving of item instances
		case IPT_TRADE:
			cg.otherTradeItems->clear();
			if (cg.currentlyTradingWith == ENTITYNUM_NONE) {
				Com_Error(ERR_DROP, "IPT_TRADE sent while not trading");
				return;
			}
			{
				int otherEntity = atoi(CG_Argv(2));
				if (otherEntity != cg.currentlyTradingWith) {
					Com_Printf("WARNING: Orphaned trade entity %i (improperly terminated trade?)\n", cg.currentlyTradingWith);
				}
				int numItems = atoi(CG_Argv(3));
				for (int i = 0; i < numItems; i++) {
					int itemID = atoi(CG_Argv(4 + (2*i)));
					int quantity = atoi(CG_Argv(5 + (2*i)));
					itemInstance_t item = BG_ItemInstance(itemID, quantity);
					cg.otherTradeItems->push_back(item);
				}
				cg.currentlyTradingWith = otherEntity;
			}
			break;
		case IPT_TRADEOPEN:
			cg.otherTradeItems->clear();
			cg.ourTradeItems->clear();
			{
				// Parse the input, including the items
				int otherEntity = atoi(CG_Argv(2));
				int numItems = atoi(CG_Argv(3));
				for (int i = 0; i < numItems; i++) {
					int itemID = atoi(CG_Argv(4 + (2 * i)));
					int quantity = atoi(CG_Argv(5 + (2 * i)));
					itemInstance_t item = BG_ItemInstance(itemID, quantity);
					cg.otherTradeItems->push_back(item);
				}
				cg.currentlyTradingWith = otherEntity;

				// Open the appropriate menu, if not in a demo
				if (!cg.demoPlayback) {
					centity_t* cent = &cg_entities[otherEntity];
					if (cent->currentState.eType == ET_PLAYER) {
						// TODO - trade between players
					}
					else if (cent->currentState.eType == ET_NPC) {
						JKG_OpenShopMenu_f();
					}
					else {
						// TODO - corpse/container looting
					}
				}
			}
			break;
		case IPT_TRADETRANS:
			// Transfer the items
			{
				// First remove the items from our inventory. THIS MUST BE DONE THE SAME WAY ON THE SERVER!
				for (auto it = cg.ourTradeItems->begin(); it != cg.ourTradeItems->end(); ++it) {
					BG_RemoveItemNonNetworked(*it);
				}

				// Add the new items to our inventory
				for (auto it = cg.otherTradeItems->begin(); it != cg.otherTradeItems->end(); ++it) {
					BG_GiveItem(*it);
				}
			}
			cg.otherTradeItems->clear();
			cg.ourTradeItems->clear();
			cg.currentlyTradingWith = ENTITYNUM_NONE;
			break;
		case IPT_TRADESINGLE:
			{
				int itemID = atoi(CG_Argv(3));
				int quantity = atoi(CG_Argv(4));
				itemInstance_t item = BG_ItemInstance(itemID, quantity);
				BG_GiveItem(item);
			}
			cg.ourTradeItems->clear();
			break;
		case IPT_TRADECANCEL:
			cg.otherTradeItems->clear();
			cg.ourTradeItems->clear();
			cg.currentlyTradingWith = ENTITYNUM_NONE;
			break;
		case IPT_TRADEREADY:
			break;
		case IPT_TRADECREDITS:
			cg.otherTradeCredits = atoi(CG_Argv(2));
			break;
	}
}
#endif

/*
====================
BG_ItemInstance

Creates a new itemInstance object.
This will cap at max quantity.
====================
*/
itemInstance_t BG_ItemInstance(itemData_t* pItemData, const int quantity) {
	itemInstance_t returnValue = { nullptr, 0, false };

	if (pItemData == nullptr) {
		return returnValue;
	}
	returnValue.quantity = quantity;
	returnValue.id = pItemData;
	if (pItemData->maxStack != 0 && quantity > pItemData->maxStack) {
		returnValue.quantity = pItemData->maxStack;
	}
	return returnValue;
}

itemInstance_t BG_ItemInstance(const char* internalName, const int quantity) {
	itemData_t* pItemData = BG_FindItemDataByName(internalName);
	return BG_ItemInstance(pItemData, quantity);
}

itemInstance_t BG_ItemInstance(const int itemID, const int quantity) {
	itemData_t* pItemData = &itemLookupTable[itemID];
	return BG_ItemInstance(pItemData, quantity);
}

/*
====================
BG_FirstStack

Checks to see if a container (vector of item instances) contains this item
Returns the location of the first stack of this item instance
====================
*/
int BG_FirstStack(const std::vector<itemInstance_t>& container, const int itemID) {
	for (auto it = container.begin(); it != container.end(); ++it) {
		if (it->id != nullptr && it->id->itemID == itemID) {
			return it - container.begin();
		}
	}
	return -1;
}

/*
====================
BG_NextStack

Finds the next stack in a vector of item instances, starting from the current.
====================
*/
int BG_NextStack(const std::vector<itemInstance_t>& container, const int itemID, const int prevStack) {
	for (auto it = container.begin() + prevStack + 1; it != container.end(); ++it) {
		if (it->id != nullptr && it->id->itemID == itemID) {
			return it - container.begin();
		}
	}
	return -1;
}

/*
====================
BG_GiveItem

Make sure that the behavior matches the server!!
====================
*/
#ifdef _CGAME
void BG_GiveItem(itemInstance_t item) {
	BG_GiveItemNonNetworked(item);
}
#else
void BG_GiveItem(gentity_t* ent, itemInstance_t item, qboolean ACI) {
	// Basic checks
	if(!item.id || !item.id->itemID) {
		return;
	}

	BG_GiveItemNonNetworked(ent, item);

	// Network data to the client
	if(ACI) {
		BG_SendItemPacket(IPT_ADDACI, ent, &item, 0, 0);
	} else {
		BG_SendItemPacket(IPT_ADD, ent, &item, 0, 0);
	}
}
#endif

/*
====================
BG_GiveItemNonNetworked

Should only be used with regards to trade.
On the server, this is also internally called from BG_GiveItem.
====================
*/
#ifdef _GAME
void BG_GiveItemNonNetworked(gentity_t* ent, itemInstance_t item) {
	// Basic checks
	if (!item.id || !item.id->itemID) {	
		return;
	}

	// Fill any incomplete stacks.
	int nItemID = item.id->itemID;
	int nMaxStack = item.id->maxStack;
	if (nMaxStack > 0) {
		for (int nStack = BG_FirstStack(*ent->inventory, nItemID); nStack != -1; nStack = BG_NextStack(*ent->inventory, nItemID, nStack)) {
			if ((*ent->inventory)[nStack].quantity < nMaxStack) {
				int diff = nMaxStack - (*ent->inventory)[nStack].quantity;
				if (diff < item.quantity) {
					item.quantity -= diff;
					(*ent->inventory)[nStack].quantity += diff;
				}
				else {
					(*ent->inventory)[nStack].quantity += item.quantity;
					item.quantity = 0;
					break;
				}
			}
		}
		if (item.quantity <= 0) {
			return;
		}
	}

	// Add the new item stack to the inventory
	ent->inventory->push_back(item);
	//do special checks for shields and jetpacks (autoequipping)
	if ((item.id->itemType == ITEM_SHIELD || item.id->itemType == ITEM_JETPACK) && ent->s.eType == ET_PLAYER) //for now don't give anyone except players autoequip shields/jetpacks
	{
		bool alreadyEquipped = false; int equipLoc = -1;
		int specialType = item.id->itemType;

		//search the inventory for existing shields or jetpacks
		for (int i = 0; i < ent->inventory->size()-1; i++)	//check everything except what we just added
		{
			if (ent->inventory->at(i).id->itemType == specialType && ent->inventory->at(i).equipped) //if a shield/jetpack is already equipped
			{
				equipLoc = i;
				alreadyEquipped = true;
				break;
			}
		}

		//we already have a shield/jetpack equipped, we need to remove the old one first
		if (alreadyEquipped)
		{
			const auto& toRemove = ent->inventory->at(equipLoc);
			int itemSlot = ent->inventory->size() - 1;

			//remove old shield/jetpack and equip new one
			if (toRemove.id->itemType == ITEM_SHIELD)
			{
				Cmd_ShieldUnequipped(ent, equipLoc);
				JKG_ShieldEquipped(ent, itemSlot, qtrue);
			}

			else if (toRemove.id->itemType == ITEM_JETPACK)
			{
				Cmd_JetpackUnequipped(ent, equipLoc);
				JKG_JetpackEquipped(ent, itemSlot);
			}

			else
			{
				Com_Printf(S_COLOR_RED "Unable to replace equipped item, non jetpack/shield detected.  Replace manually in inventory menu.\n");
				return;
			}

		}
		
		//if a shield/jetpack isn't already equipped, equip the new one
		else
		{

			int itemSlot = ent->inventory->size()-1;
			if (specialType == ITEM_SHIELD)
			{
				JKG_ShieldEquipped(ent, itemSlot, qtrue);
			}
			else if (specialType == ITEM_JETPACK)
			{
				JKG_JetpackEquipped(ent, itemSlot);
			}
		}
	}

	//handle weapons - sometime?  Currently weapons only exist in ACI and don't have an 'equipped state'
	/*if (ent->s.eType == ET_PLAYER && item.id->itemType == ITEM_WEAPON)
	{
		JKG_EquipItem(ent, ent->inventory->size() - 1);
	}*/
}
#elif _CGAME
void BG_GiveItemNonNetworked(itemInstance_t item)
{
	// Basic checks
	if (!item.id || !item.id->itemID) {
		return;
	}

	// The player cannot actually acquire ammo items.
	if (item.id->itemType == ITEM_AMMO)
	{
		return;
	}

	// Fill any incomplete stacks.
	int nItemID = item.id->itemID;
	int nMaxStack = item.id->maxStack;
	if (nMaxStack > 0) {
		for (int nStack = BG_FirstStack(*cg.playerInventory, nItemID); nStack != -1; nStack = BG_NextStack(*cg.playerInventory, nItemID, nStack)) {
			if ((*cg.playerInventory)[nStack].quantity < nMaxStack) {
				int diff = nMaxStack - (*cg.playerInventory)[nStack].quantity;
				if (diff < item.quantity) {
					item.quantity -= diff;
					(*cg.playerInventory)[nStack].quantity += diff;
				}
				else {
					(*cg.playerInventory)[nStack].quantity += item.quantity;
					item.quantity = 0;
					break;
				}
			}
		}
		if (item.quantity <= 0) {
			return;
		}
	}
	cg.playerInventory->push_back(item);

	// If this item is a weapon, shield, jetpack or consumable - which is not already in our ACI, and the ACI is not full, add it.
	if(item.id->itemType == ITEM_WEAPON || item.id->itemType == ITEM_SHIELD || item.id->itemType == ITEM_JETPACK || item.id->itemType == ITEM_CONSUMABLE) 
	{
		bool bInACIAlready = false;
		int nFreeACISlot = -1; int specialType = 0;

		//if a shield or jetpack, save type
		if (item.id->itemType == ITEM_SHIELD || item.id->itemType == ITEM_JETPACK)
			specialType = item.id->itemType;

		for(int i = 0; i < MAX_ACI_SLOTS; i++) 
		{
			if(cg.playerACI[i] == -1 && nFreeACISlot == -1) 
			{
				nFreeACISlot = i;
				continue;
			} 
			else if(cg.playerACI[i] == -1) 
			{
				continue;
			} 
			else if(cg.playerACI[i] >= cg.playerInventory->size()) 
			{
				// This item in our ACI is invalid, remove it
				cg.playerACI[i] = -1;
				continue;
			}
			if(!Q_stricmp((*cg.playerInventory)[cg.playerACI[i]].id->internalName, item.id->internalName)) 
			{
				bInACIAlready = true; 
			}
			if((*cg.playerInventory)[cg.playerACI[i]].id->itemType == specialType && specialType)
			{
				//we found another special item equipped - remove it
				cg.playerACI[i] = -1;
				nFreeACISlot = i;
				continue;
			}
			if(bInACIAlready && nFreeACISlot >= 0) 
			{ // already found everything we need to know, just die
				break;
			}
		}

		//not already in aci
		if (!bInACIAlready) 
		{
			//all slots are taken - replace the 0th slot
			if (nFreeACISlot == -1)
			{
				nFreeACISlot = 0;
				auto slotZero = (*cg.playerInventory)[cg.playerACI[nFreeACISlot]].id->itemType; //remember what was at slot 0

				if (slotZero == ITEM_SHIELD) //if we had a shield equipped
				{
					//if we're equipping anything other than a shield
					if (item.id->itemType != ITEM_SHIELD)
					{
						if(slotZero == ITEM_SHIELD) //if the thing we're replacing was a shield, then unequip
							trap->SendClientCommand("unequipShield");

						if(slotZero == ITEM_JETPACK) //if the thing we're replacing was a jetpack, then unequip
							trap->SendClientCommand("unequipJetpack");
					}
				}
				if (slotZero == ITEM_JETPACK) //handle jetpacks like shields above
				{
					if (item.id->itemType != ITEM_JETPACK)
					{
						if (slotZero == ITEM_SHIELD) 
							trap->SendClientCommand("unequipShield");

						if (slotZero == ITEM_JETPACK) 
							trap->SendClientCommand("unequipJetpack");
					}
				}
			}

			//otherwise we found a free slot (since nFreeACISLot != -1)
			cg.playerACI[nFreeACISlot] = cg.playerInventory->size() - 1; //assign the aci slot to the inventory just added

			if (item.id->itemType == ITEM_WEAPON)
			{
				(*cg.playerInventory)[cg.playerACI[nFreeACISlot]].id->weaponData.holsterState = false; //set default holsterstate
			}
		}
	}

}
#endif

/*
====================
BG_RemoveItemStack

Server tells client to remove item as well.
====================
*/
#ifdef _GAME
extern void JKG_UnequipItem(gentity_t *ent, int iNum);
void BG_RemoveItemStack(gentity_t* ent, int itemStack) {
	itemInstance_t item = (*ent->inventory)[itemStack];

	// If it's something we have equipped, remove it
	if (item.equipped)
	{
		JKG_UnequipItem(ent, itemStack);
	}

	ent->inventory->erase(ent->inventory->begin() + itemStack);

	BG_SendItemPacket(IPT_REM, ent, nullptr, itemStack, 0);
}
#else
void BG_RemoveItemStack(int itemStack) {
	cg.playerInventory->erase(cg.playerInventory->begin() + itemStack);
}
#endif

/*
====================
BG_RemoveItem

Server does NOT tell client to remove item
====================
*/
#ifdef _GAME
void BG_RemoveItemNonNetworked(gentity_t* ent, itemInstance_t item) {
	itemData_t* pItemData = item.id;
	int quantity = item.quantity;

	for(auto it = ent->inventory->begin(); it != ent->inventory->end() && quantity > 0; ++it) {
		if (pItemData->itemID == it->id->itemID) {
			// This item ID matches
			if (quantity >= it->quantity) {
				quantity -= it->quantity;
				it = ent->inventory->erase(it);
			}
			else {
				it->quantity -= quantity;
				break;
			}
		}
	}
}
#elif _CGAME
void BG_RemoveItemNonNetworked(itemInstance_t item) {
	itemData_t* pItemData = item.id;
	int quantity = item.quantity;
	for (auto it = cg.playerInventory->begin(); it != cg.playerInventory->end() && quantity > 0; ++it) {
		if (pItemData->itemID == it->id->itemID) {
			// This item ID matches
			if (quantity >= it->quantity) {
				quantity -= it->quantity;
				it = cg.playerInventory->erase(it);
			}
			else {
				it->quantity -= quantity;
				break;
			}
		}
	}
}
#endif

/*
====================
BG_ChangeItemStackQuantity

Server tells client to change quantity as well.
====================
*/
#ifdef _GAME
void BG_ChangeItemStackQuantity(gentity_t* ent, int itemStack, int newQuantity) {
	if(newQuantity <= 0) {
		BG_RemoveItemStack(ent, itemStack);
		return;
	}
	(*ent->inventory)[itemStack].quantity = newQuantity;

	BG_SendItemPacket(IPT_QUANT, ent, nullptr, itemStack, newQuantity);
}
#else
void BG_ChangeItemStackQuantity(int itemStack, int newQuantity) {
	if (newQuantity == 0) {
		BG_RemoveItemStack(itemStack);
		return;
	}
	(*cg.playerInventory)[itemStack].quantity = newQuantity;
}
#endif

/*
====================
BG_AdjustItemStackQuantity

Adds/subtracts item stack quantity
====================
*/
#ifdef _GAME
void BG_AdjustItemStackQuantity(gentity_t* ent, int itemStack, int adjustment) {
	if(itemStack < 0 || itemStack >= ent->inventory->size()) {
		trap->Print("client %i tried to change stack quantity of invalid slot %i!!\n", ent->s.number);
		return;
	}
	BG_ChangeItemStackQuantity(ent, itemStack, (*ent->inventory)[itemStack].quantity + adjustment);
}
#else
void BG_AdjustItemStackQuantity(int itemStack, int adjustment) {
	BG_ChangeItemStackQuantity(itemStack, (*cg.playerInventory)[itemStack].quantity + adjustment);
}
#endif

/*
====================
BG_AddItemToACI
====================
*/
#ifdef _CGAME
void BG_AddItemToACI(int itemStackNum, int aciSlot) {
	// Basic checks
	if (itemStackNum >= cg.playerInventory->size()) {
		return;
	}

	int desiredItemID = (*cg.playerInventory)[itemStackNum].id->itemID;
	if (desiredItemID == 0) {
		return;
	}

	// Automatically unassign slots which have the same item ID as us
	// NOTENOTE: remember to remove this if we want to have items with different qualities, stats, etc
	for (int i = 0; i < MAX_ACI_SLOTS; i++) {
		int thisSlot = cg.playerACI[i];
		if (thisSlot == -1) {
			continue;
		}
		if (thisSlot >= cg.playerInventory->size()) {
			cg.playerACI[i] = -1;
			continue;
		}
		if (!(*cg.playerInventory)[thisSlot].id) {
			continue;
		}
		if ((*cg.playerInventory)[thisSlot].id->itemID == desiredItemID) {
			cg.playerACI[i] = -1;
		}
	}

	// If the ACI slot is -1, assign it to the first available slot.
	if (aciSlot == -1) {
		for (int i = 0; i < MAX_ACI_SLOTS; i++) {
			if (cg.playerACI[i] == -1) {
				cg.playerACI[i] = itemStackNum;
				return;
			}
		}
		return;	// Couldn't find an available slot
	}
	cg.playerACI[aciSlot] = itemStackNum;
}
#endif

/*
====================
BG_ConsumeItem
====================
*/
#ifdef _GAME
extern void GLua_ConsumeItem(gentity_t* consumer, itemInstance_t* item);
int BG_ConsumeItem(gentity_t* ent, int itemStackNum) {
	itemInstance_t* item;
	int consumeAmount;

	if (itemStackNum < 0 || itemStackNum >= ent->inventory->size()) {
		// Invalid inventory ID
		return 1;
	}

	item = &(*ent->inventory)[itemStackNum];
	if (item->id->itemType != ITEM_CONSUMABLE) {
		// Not a consumable item
		return 1;
	}

	consumeAmount = item->id->consumableData.consumeAmount;
	if (consumeAmount > item->quantity) {
		// Not enough quantity to consume this item
		return 2;
	}

	if (ent->client->ps.consumableTime > level.time)
	{
		//still on cooldown for consumables
		G_Sound(ent, CHAN_AUTO, G_SoundIndex("sound/interface/ammocon_done.mp3"));
		return 3;
	}
	

	if(item->id->consumableData.partHealthReq && ent->health >= ent->client->ps.stats[STAT_MAX_HEALTH] )
	{
		return 4;
	}

	if (item->id->consumableData.partStaminaReq && ent->playerState->forcePower >= ent->client->ps.stats[STAT_MAX_STAMINA] )
	{
		return 4;
	}

	GLua_ConsumeItem(ent, item);
	BG_ChangeItemStackQuantity(ent, itemStackNum, item->quantity - consumeAmount);
	ent->client->ps.consumableTime = level.time + bgConstants.consumableTime;
	return 0;
}
#endif

/*
====================
BG_LoadDefaultWeaponItems

Creates weightless weapon items if no item files exist for the weapons.
====================
*/
void BG_LoadDefaultWeaponItems ( void )
{
    int i = 0;
    int end = BG_NumberOfLoadedWeapons();
    qboolean weaponHasItem[MAX_WEAPON_TABLE_SIZE] = { qfalse };
    
    for ( i = 1; i < MAX_ITEM_TABLE_SIZE; i++ )
    {
		itemData_t *item = &itemLookupTable[i];
        if ( !item->itemID )
        {
            continue;
        }
        
        if ( item->itemType != ITEM_WEAPON )
        {
            continue;
        }
        
		weaponHasItem[item->weaponData.varID] = qtrue;
    }
    
    for ( i = 0; i < end; i++ )
    {
        weaponData_t *weaponData;
        int weapon, variation;
        int itemID;
		itemData_t item;
        
        if ( weaponHasItem[i] )
        {
            continue;
        }
        
        if ( !BG_GetWeaponByIndex (i, &weapon, &variation) )
        {
            break;
        }
        
        memset (&item, 0, sizeof (item));
        weaponData = GetWeaponData (weapon, variation);
        Q_strncpyz (item.displayName, weaponData->displayName, sizeof (item.displayName));
        itemID = BG_GetNextFreeItemSlot();
        if ( !itemID )
        {
            Com_Printf ("Ran out of item space for weapons.\n");
            break;
        }
        
        item.itemID = itemID;
        item.itemType = ITEM_WEAPON;
		item.weaponData.weapon = weapon;
		item.weaponData.variation = variation;
		item.weaponData.varID = i;
#ifdef _CGAME
		Q_strncpyz(item.visuals.itemIcon, weaponData->visuals.icon, MAX_QPATH);
#endif
        
        itemLookupTable[itemID] = item;
    }
}

/*
====================
BG_LoadItem
====================
*/
int lastUsedItemID = 0;
extern stringID_table_t WPTable[]; // From bg_saga.c
static bool BG_LoadItem(const char *itemFilePath, itemData_t *itemData)
{
	cJSON *json = NULL;
	cJSON *jsonNode = NULL;

	char error[MAX_STRING_CHARS];
	const char *str = NULL;
	int	item;

	char itemFileData[MAX_ITEM_FILE_LENGTH];
	fileHandle_t f;
	int fileLen = trap->FS_Open(itemFilePath, &f, FS_READ);

	if (!f || fileLen == -1)
	{
		Com_Printf(S_COLOR_RED "Unreadable or empty item file %s\n", itemFilePath);
		return false;
	}

	if ((fileLen + 1) >= MAX_ITEM_FILE_LENGTH)
	{
		trap->FS_Close(f);
		Com_Printf(S_COLOR_RED "%s item file too large\n", itemFilePath);
		return false;
	}

	trap->FS_Read(&itemFileData, fileLen, f);
	itemFileData[fileLen] = '\0';

	trap->FS_Close(f);

	json = cJSON_ParsePooled(itemFileData, error, sizeof(error));
	if (json == NULL)
	{
		Com_Printf(S_COLOR_RED "%s: %s\n", itemFilePath, error);
		return false;
	}

	//Basic Item Information
	jsonNode = cJSON_GetObjectItem(json, "name");
	str = cJSON_ToString(jsonNode);
	strcpy(itemData->displayName, str);

	jsonNode = cJSON_GetObjectItem(json, "internal");
	str = cJSON_ToString(jsonNode);
	strcpy(itemData->internalName, str);

	jsonNode = cJSON_GetObjectItem(json, "id");
	if (jsonNode) {
		item = cJSON_ToNumber(jsonNode);
		itemData->itemID = item;
	}
	else {
		itemData->itemID = lastUsedItemID;
#ifdef _DEBUG
		Com_Printf("^3DEBUG: autoassigning item ID %i\n", itemData->itemID);
#endif
	}

	lastUsedItemID++;

	// Visuals
#ifdef _CGAME
	jsonNode = cJSON_GetObjectItem(json, "itemIcon");
	Q_strncpyz(itemData->visuals.itemIcon, cJSON_ToStringOpt(jsonNode, "gfx/Item_Icons/default.tga"), MAX_QPATH);
#endif

	jsonNode = cJSON_GetObjectItem(json, "itemtype");
	str = cJSON_ToString(jsonNode);
	if (Q_stricmp(str, "armor") == 0)
		itemData->itemType = ITEM_ARMOR;
	else if (Q_stricmp(str, "weapon") == 0)
		itemData->itemType = ITEM_WEAPON;
	else if (Q_stricmp(str, "tool") == 0)
		itemData->itemType = ITEM_TOOL;
	else if (Q_stricmp(str, "clothing") == 0)
		itemData->itemType = ITEM_CLOTHING;
	else if (Q_stricmp(str, "consumable") == 0)
		itemData->itemType = ITEM_CONSUMABLE;
	else if (Q_stricmp(str, "shield") == 0)
		itemData->itemType = ITEM_SHIELD;
	else if (Q_stricmp(str, "jetpack") == 0)
		itemData->itemType = ITEM_JETPACK;
	else if (Q_stricmp(str, "ammo") == 0)
		itemData->itemType = ITEM_AMMO;
	else
		itemData->itemType = ITEM_UNKNOWN;

	jsonNode = cJSON_GetObjectItem(json, "tier");
	item = cJSON_ToIntegerOpt(jsonNode, 1);
	if (item >= NUM_ITEM_TIERS || item < TIER_SCRAP) //out of range
	{
		Com_Printf(S_COLOR_YELLOW "Invalid item tier (%i) detected in %s, defaulting to common.\n", item, itemFilePath);
		item = 1;
	}
	itemData->itemTier = static_cast<itemTier_t>(item);

	jsonNode = cJSON_GetObjectItem(json, "tradeable");
	itemData->tradeable = cJSON_ToBooleanOpt(jsonNode, true);

	jsonNode = cJSON_GetObjectItem(json, "segregated");
	itemData->segregated = cJSON_ToBooleanOpt(jsonNode, false);

	jsonNode = cJSON_GetObjectItem(json, "droppable");
	itemData->droppable = cJSON_ToBooleanOpt(jsonNode, true);

	//legendary items are special
	if (itemData->itemTier == TIER_LEGENDARY)
	{
		itemData->tradeable = true;
		itemData->droppable = true;
	}

	jsonNode = cJSON_GetObjectItem(json, "itemDescription");
	str = cJSON_ToStringOpt(jsonNode, "No description available.");
	Q_strncpyz(itemData->itemDescription, cJSON_ToStringOpt(jsonNode, "No description available."), MAX_ITEM_DESCRIPTION);

	jsonNode = cJSON_GetObjectItem(json, "weight");
	item = cJSON_ToNumberOpt(jsonNode, 1.0f);
	if (item < 0) { item = 0.0f; } // no negative weight
	itemData->weight = item;

	jsonNode = cJSON_GetObjectItem(json, "cost");
	item = cJSON_ToIntegerOpt(jsonNode, 100);
	itemData->baseCost = item;

	jsonNode = cJSON_GetObjectItem(json, "maxStack");
	item = cJSON_ToIntegerOpt(jsonNode, 1);
	itemData->maxStack = item;

	//Equipment Info
	if (itemData->itemType == ITEM_WEAPON) {
		//This is a weapon. Grab the data.
		jsonNode = cJSON_GetObjectItem(json, "weapon");
		str = cJSON_ToString(jsonNode);
		if (!atoi(str))
		{
			itemData->weaponData.weapon = GetIDForString(WPTable, str);
		}
		else
		{
			itemData->weaponData.weapon = atoi(str);
		}

		jsonNode = cJSON_GetObjectItem(json, "variation");
		item = cJSON_ToNumber(jsonNode);
		itemData->weaponData.variation = item;

		itemData->weaponData.varID = BG_GetWeaponIndex(itemData->weaponData.weapon, itemData->weaponData.variation);
	}
	else if (itemData->itemType == ITEM_AMMO) {
		jsonNode = cJSON_GetObjectItem(json, "ammoclass");
		Q_strncpyz(itemData->ammoData.ref, cJSON_ToStringOpt(jsonNode, ""), sizeof(itemData->armorData.ref));
		itemData->ammoData.ammoIndex = BG_GetAmmo(itemData->ammoData.ref) - ammoTable;

		jsonNode = cJSON_GetObjectItem(json, "ammoquantity");
		itemData->ammoData.quantity = cJSON_ToIntegerOpt(jsonNode, 1);
	}
	else if (itemData->itemType == ITEM_ARMOR) {
		jsonNode = cJSON_GetObjectItem(json, "armor");
		Q_strncpyz(itemData->armorData.ref, cJSON_ToStringOpt(jsonNode, ""), sizeof(itemData->armorData.ref));
		
		itemData->armorData.pArm = JKG_FindArmorByName(itemData->armorData.ref);
	}
	else if (itemData->itemType == ITEM_CONSUMABLE) {
		// consumeScript controls the script that gets run when we consume the item
		jsonNode = cJSON_GetObjectItem(json, "consumeScript");
		Q_strncpyz(itemData->consumableData.consumeScript, cJSON_ToStringOpt(jsonNode, "noscript"), MAX_CONSUMABLE_SCRIPTNAME);

		// consumeAmount controls the amount of items in the stack that get consumed
		jsonNode = cJSON_GetObjectItem(json, "consumeAmount");
		itemData->consumableData.consumeAmount = cJSON_ToIntegerOpt(jsonNode, 1);

		// partHealthReq only allows the item to be consumed if the entity has partial health
		jsonNode = cJSON_GetObjectItem(json, "partHealthReq");
		itemData->consumableData.partHealthReq = (qboolean)cJSON_ToBooleanOpt(jsonNode, false);

		// partStaminaReq only allows the item to be consumed if the entity lacks stamina
		jsonNode = cJSON_GetObjectItem(json, "partStaminaReq");
		itemData->consumableData.partStaminaReq = (qboolean)cJSON_ToBooleanOpt(jsonNode, false);
	}
	else if (itemData->itemType == ITEM_SHIELD) {
		memset(&itemData->shieldData, 0, sizeof(itemData->shieldData));

		jsonNode = cJSON_GetObjectItem(json, "capacity");
		itemData->shieldData.capacity = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_CAPACITY);

		jsonNode = cJSON_GetObjectItem(json, "cooldown");
		itemData->shieldData.cooldown = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_COOLDOWN);

		jsonNode = cJSON_GetObjectItem(json, "regenrate");
		itemData->shieldData.regenrate = cJSON_ToIntegerOpt(jsonNode, SHIELD_DEFAULT_REGEN);

		jsonNode = cJSON_GetObjectItem(json, "rechargeSoundEffect");
		if (jsonNode) {
			Q_strncpyz(itemData->shieldData.rechargeSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
		}

		jsonNode = cJSON_GetObjectItem(json, "brokenSoundEffect");
		if (jsonNode) {
			Q_strncpyz(itemData->shieldData.brokenSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
		}

		jsonNode = cJSON_GetObjectItem(json, "equippedSoundEffect");
		if (jsonNode) {
			Q_strncpyz(itemData->shieldData.equippedSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
		}

		jsonNode = cJSON_GetObjectItem(json, "chargedSoundEffect");
		if (jsonNode) {
			Q_strncpyz(itemData->shieldData.chargedSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
		}

		jsonNode = cJSON_GetObjectItem(json, "malfunctionSoundEffect");
		if (jsonNode) {
			Q_strncpyz(itemData->shieldData.malfunctionSoundEffect, cJSON_ToString(jsonNode), MAX_QPATH);
		}
	}
	else if (itemData->itemType == ITEM_JETPACK) {
		jsonNode = cJSON_GetObjectItem(json, "jetpack");
		Q_strncpyz(itemData->jetpackData.ref, cJSON_ToString(jsonNode), sizeof(itemData->jetpackData.ref));

		itemData->jetpackData.pJetpackData = JKG_FindJetpackByName(itemData->jetpackData.ref);
		if (itemData->jetpackData.pJetpackData == nullptr) {
			Com_Printf(S_COLOR_YELLOW "WARNING: %s is a jetpack, but doesn't have a valid reference to a .jet file!\n", itemFilePath);
		}
	}

	cJSON_Delete(json);

	return qtrue;
}

/*
====================
BG_LoadItems

Loads each individual item file
====================
*/
static bool BG_LoadItems(void)
{
	int i, j;
	char itemFiles[8192];
	int numFiles = Q_FSGetFileListSorted("ext_data/items/", ".itm", itemFiles, sizeof(itemFiles));
	const char *itemFile = itemFiles;
	int successful = 0;
	int failed = 0;

	lastUsedItemID = 1;

	Com_Printf("------- Constructing Item Table -------\n");
	if (numFiles > MAX_ITEM_TABLE_SIZE)
	{
		Com_Printf(S_COLOR_RED "ERROR: Not enough memory reserved for item table.\nIncrease MAX_ITEM_TABLE_SIZE.  Capacity: (%i / %i)\n", numFiles, MAX_ITEM_TABLE_SIZE);
		return false;
	}
		

	for (i = 0; i < numFiles; i++)
	{
		itemData_t dummy;
		if (!BG_LoadItem(va("ext_data/items/%s", itemFile), &dummy))
		{
			failed++;
			itemFile += strlen(itemFile) + 1;
			continue;
		}

		if (dummy.itemID > 0 && dummy.itemID < MAX_ITEM_TABLE_SIZE)
		{
			successful++;
		}
		else
		{
			failed++;
			continue;
		}

		if (dummy.itemID >= MAX_ITEM_TABLE_SIZE){
			Com_Printf(S_COLOR_RED "ERROR: item ID out of range \nItem: ext_data/items/%s, ID: %d\n", itemFile, dummy.itemID);
			Com_Printf(S_COLOR_RED "Attempting to correct this now...\n");
			for (j = 1; j < MAX_ITEM_TABLE_SIZE; j++)
			{
				if (!itemLookupTable[j].itemID)
				{
					dummy.itemID = j;
					Com_Printf(S_COLOR_YELLOW "New itemID: %d\n", dummy.itemID);
					break;
				}
			}
		}

		if (itemLookupTable[dummy.itemID].itemID)
			Com_Printf(S_COLOR_YELLOW "Duplicate item id: %d\n", dummy.itemID);
		itemLookupTable[dummy.itemID] = dummy;

		itemFile += strlen(itemFile) + 1;
	}
	Com_Printf("Item Table: %d successful, %d failed.\n", successful, failed);
	Com_Printf("-------------------------------------\n");

	return successful > 0;
}

/*
====================
BG_InitItems

Starts the loading process
====================
*/
void BG_InitItems() {

	//--Futuza: For now, don't do this dynamically - just allocate a static block on the heap
	//In the future consider refactoring itemLookupTable into a object or using vector etc
	
	itemLookupTable = (itemData_t*)malloc(sizeof(itemData_t) * MAX_ITEM_TABLE_SIZE);
	if (itemLookupTable == nullptr)
	{
		Com_Error(ERR_DROP, "could not allocate memory for items...");
		return;
	}

	memset(itemLookupTable, 0, sizeof(itemData_t) * MAX_ITEM_TABLE_SIZE);
	

	if (BG_LoadItems() == false) {
		Com_Error(ERR_DROP, "Unable to load items...");
		return;
	}
}

/*
=====================
BG_ShutdownItems

Frees the memory associated with items
=====================
*/
void BG_ShutdownItems() {
	if (itemLookupTable != nullptr)
	{
		free(itemLookupTable);
	}
}