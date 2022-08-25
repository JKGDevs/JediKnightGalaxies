#include "../game/bg_items.h"
#include "../game/bg_weapons.h"
#include "ui_local.h"
#include "jkg_inventory.h"
#include <algorithm>


using namespace std;

/* Global variables */
static bool bLeftSelected = false;		/* Is the left side (or the right side) selected */
static int nSelected = -1;				/* The currently selected element */

static size_t nNumberInventoryItems;	/* The number of inventory items */
static size_t nNumberShopItems;			/* The number of shop items */
static size_t nNumberUnfilteredIItems;	/* The total number of inventory items before filtering */
static size_t nNumberUnfilteredSItems;	/* The total number of shop items before filtering */

static vector<pair<int, itemInstance_t*>> vInventoryItems;	/* The inventory items, after filtering */
static vector<pair<int, itemInstance_t*>> vShopItems;		/* The shop items, after filtering */
static vector<std::string> vShopItemDesc;					/* The description of the currently selected shop item */
static vector<pair<int, int>> vPriceCheckedAmmo;			/* Price check of ammo on items in inventory */

static size_t nInventoryScroll = 0;		// How far we've scrolled in the menu
static size_t nShopScroll = 0;			// How far we've scrolled in the menu
static bool bPriceCheckComplete = false;// Whether or not we've completed the ammo price check
static int nPriceCheckCost = -1;
static bool bExamineMenuOpen = false;	// Whether we are examining things

// This function updates the status of price checking. Only called when bPriceCheckComplete is false.
void JKG_Shop_UpdatePriceCheck()
{
	int id = vInventoryItems[nSelected].first;

	if (vInventoryItems[nSelected].second->id->itemType != ITEM_WEAPON)
	{
		return;	// don't worry about price checking something that isn't a weapon
	}

	//check if the gun we're getting ammo for is the starting weapon
	char info[MAX_INFO_VALUE];
	trap->GetConfigString(CS_SERVERINFO, info, sizeof(info));
	if (!Q_stricmp(vInventoryItems[nSelected].second->id->internalName, Info_ValueForKey(info, "jkg_startingGun")))
	{
		bPriceCheckComplete = true;
		nPriceCheckCost = 0;	//its free!
		return;
	}

	for (auto it = vPriceCheckedAmmo.begin(); it != vPriceCheckedAmmo.end(); ++it)
	{
		if (it->first == id)
		{
			bPriceCheckComplete = true;
			nPriceCheckCost = it->second;
			return;
		}
	}
}

// This function constructs both the inventory and shop lists
void JKG_ConstructShopLists() {
	vInventoryItems.clear();
	vShopItems.clear();
	vShopItemDesc.clear();
	vPriceCheckedAmmo.clear();
	bPriceCheckComplete = false;
	nPriceCheckCost = -1;

	if (cgImports == nullptr) {
		// This gets called when the game starts, because ui_inventoryFilter gets modified
		return;
	}

	Menu_ShowGroup(Menus_FindByName("jkg_shop"), "shop_examine", bExamineMenuOpen);
	Menu_ShowGroup(Menus_FindByName("jkg_shop"), "shop_left", !bExamineMenuOpen);

	nNumberUnfilteredIItems = *(size_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_SIZE, -1);
	nNumberUnfilteredSItems = *(size_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_OTHERCONTAINERSIZE, -1);

	if (nNumberUnfilteredIItems > 0) {
		itemInstance_t* pAllInventoryItems = (itemInstance_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_ITEMS, -1);

		//
		// Filter the items
		//
		for (int i = 0; i < nNumberUnfilteredIItems; i++) {
			itemInstance_t* pThisItem = &pAllInventoryItems[i];
			if (ui_inventoryFilter.integer == JKGIFILTER_ARMOR 
				&& pThisItem->id->itemType != ITEM_ARMOR 
				&& pThisItem->id->itemType != ITEM_SHIELD
				&& pThisItem->id->itemType != ITEM_JETPACK
				&& pThisItem->id->itemType != ITEM_CLOTHING) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_WEAPONS && pThisItem->id->itemType != ITEM_WEAPON) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_CONSUMABLES && pThisItem->id->itemType != ITEM_CONSUMABLE) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_TOOLS && pThisItem->id->itemType != ITEM_TOOL)
			{
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_AMMO && pThisItem->id->itemType != ITEM_AMMO)
			{
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_MISC) {
				continue; // FIXME
			}
			vInventoryItems.push_back(make_pair(i, pThisItem));
		}

		//
		// Sort the filtered list of items
		//
		if (ui_inventorySortMode.integer == 0) {
			// If it's 0, then we're sorting by item name
			sort(vInventoryItems.begin(), vInventoryItems.end(),
				[](const pair<int, itemInstance_t*>& a, const pair<int, itemInstance_t*>& b) -> bool {
				if (ui_inventorySortType.integer) {
					return Q_stricmp(UI_GetStringEdString2(a.second->id->displayName), UI_GetStringEdString3(b.second->id->displayName)) > 0;
				}
				else {
					return Q_stricmp(UI_GetStringEdString2(b.second->id->displayName), UI_GetStringEdString3(a.second->id->displayName)) > 0;
				}
			});
		}
		else if (ui_inventorySortMode.integer == 1) {
			// If it's 1, then we're sorting by price
			sort(vInventoryItems.begin(), vInventoryItems.end(),
				[](const pair<int, itemInstance_t*>& a, const pair<int, itemInstance_t*>& b) -> bool {
				if (ui_inventorySortType.integer) {
					return a.second->id->baseCost * a.second->quantity > b.second->id->baseCost * b.second->quantity;
				}
				else {
					return a.second->id->baseCost * a.second->quantity < b.second->id->baseCost * b.second->quantity;
				}
			});
		}
	}

	if (nNumberUnfilteredSItems > 0) {
		itemInstance_t* pAllShopItems = (itemInstance_t*)cgImports->InventoryDataRequest(INVENTORYREQUEST_OTHERCONTAINERITEMS, -1);

		//
		// Filter the list of items
		//
		for (int i = 0; i < nNumberUnfilteredSItems; i++) {
			itemInstance_t* pThisItem = &pAllShopItems[i];
			if (ui_inventoryFilter.integer == JKGIFILTER_ARMOR 
				&& pThisItem->id->itemType != ITEM_ARMOR 
				&& pThisItem->id->itemType != ITEM_SHIELD
				&& pThisItem->id->itemType != ITEM_JETPACK
				&& pThisItem->id->itemType != ITEM_CLOTHING) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_WEAPONS && pThisItem->id->itemType != ITEM_WEAPON) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_CONSUMABLES && pThisItem->id->itemType != ITEM_CONSUMABLE) {
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_TOOLS && pThisItem->id->itemType != ITEM_TOOL)
			{
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_AMMO && pThisItem->id->itemType != ITEM_AMMO)
			{
				continue;
			}
			else if (ui_inventoryFilter.integer == JKGIFILTER_MISC) {
				continue;
			}
			vShopItems.push_back(make_pair(i, pThisItem));
		}

		//
		// Sort the filtered list of items
		//
		if (ui_shopSortMode.integer == 0) {
			// If it's 0, then we're sorting by item name
			sort(vShopItems.begin(), vShopItems.end(),
				[](const pair<int, itemInstance_t*>& a, const pair<int, itemInstance_t*>& b) -> bool {
				if (ui_shopSortType.integer) {
					return Q_stricmp(UI_GetStringEdString2(a.second->id->displayName), UI_GetStringEdString3(b.second->id->displayName)) > 0;
				}
				else {
					return Q_stricmp(UI_GetStringEdString2(b.second->id->displayName), UI_GetStringEdString3(a.second->id->displayName)) > 0;
				}
			});
		}
		else if (ui_shopSortMode.integer == 1) {
			// If it's 1, then we're sorting by price
			sort(vShopItems.begin(), vShopItems.end(),
				[](const pair<int, itemInstance_t*>& a, const pair<int, itemInstance_t*>& b) -> bool {
				if (ui_shopSortType.integer) {
					return a.second->id->baseCost > b.second->id->baseCost;
				}
				else {
					return a.second->id->baseCost < b.second->id->baseCost;
				}
			});
		}
	}

	// Reset the selection/scroll if it goes out of bounds
	nNumberInventoryItems = vInventoryItems.size();
	nNumberShopItems = vShopItems.size();
	if (bLeftSelected && nSelected >= nNumberInventoryItems) {
		nSelected = -1;
	}
	else if (!bLeftSelected && nSelected >= nNumberShopItems) {
		nSelected = -1;
	}
	else if (!bLeftSelected)
	{
		JKG_ConstructItemDescription(vShopItems[nSelected].second, vShopItemDesc);
	}

	if (nInventoryScroll >= nNumberInventoryItems) {
		nInventoryScroll = 0;
	}

	if (nShopScroll >= nNumberShopItems) {
		nShopScroll = 0;
	}
}

void JKG_ShopInventorySortChanged() {
	JKG_ConstructShopLists();
}

// The script that gets run when a shop arrow button is pressed
void JKG_ShopArrow(char** args) {
	const char* side;
	const char* direction;
	int count;

	if (!String_Parse(args, &side)) {
		trap->Print("Couldn't parse shop_arrow jkgscript\n");
		return;
	}
	else if (!String_Parse(args, &direction)) {
		trap->Print("Couldn't parse shop_arrow jkgscript\n");
		return;
	}
	else if (!Int_Parse(args, &count)) {
		trap->Print("Couldn't parse shop_arrow jkgscript\n");
		return;
	}

	if (count <= 0) {
		trap->Print("shop_arrow called with 0 or negative scroll value\n");
		return;
	}

	if (!Q_stricmp(direction, "up")) {
		count *= -1;
	}

	if (!Q_stricmp(side, "left")) {
		if (nShopScroll == 0 && count < 0) {
			nShopScroll = 0;
		}
		else if (nNumberInventoryItems > 0 && nInventoryScroll + count >= nNumberInventoryItems) {
			nInventoryScroll = nNumberInventoryItems - 1;
		}
		else {
			nInventoryScroll += count;
		}
	}
	else if (!Q_stricmp(side, "right")) {
		if (nShopScroll == 0 && count < 0) {
			nShopScroll = 0;
		}
		else if (nNumberShopItems > 0 && nShopScroll + count >= nNumberShopItems) {
			nShopScroll = nNumberShopItems - 1;
		}
		else {
			nShopScroll += count;
		}
	}
	else {
		trap->Print("Unknown side '%s' used for shop_arrow jkgscript\n", side);
		return;
	}

	JKG_ConstructShopLists();
}

// The action that occurs when scrolling with the mouse wheel
void JKG_ScrollShop(qboolean bUp, int nMouseX, int nMouseY)
{
	int* ptScroll;
	int nTotal;
	int nCount;

	if (bUp)
	{
		nCount = -1;
	}
	else
	{
		nCount = 1;
	}

	if (nMouseX < SCREEN_WIDTH / 2)
	{
		// scrolling the left side - the inventory side
		ptScroll = (int*)&nInventoryScroll;
		nTotal = nNumberInventoryItems;
	}
	else
	{
		// scrolling the right side - the shop side
		ptScroll = (int*)&nShopScroll;
		nTotal = nNumberShopItems;
	}

	if (*ptScroll + nCount < 0)
	{
		*ptScroll = 0;
	}
	else if (*ptScroll + nCount >= nTotal && nTotal > 0)
	{
		*ptScroll = nTotal - 1;
	}
	else
	{
		*ptScroll = *ptScroll + nCount;
	}
}

//
// The icon that shows up for each item button
// 
void JKG_ShopIconLeft(itemDef_t* item, int nOwnerDrawID) {
	if (nInventoryScroll + nOwnerDrawID >= nNumberInventoryItems) {
		// Don't draw it if there isn't an item in the slot
		return;
	}

	itemInstance_t* pThisItem = vInventoryItems[nInventoryScroll + nOwnerDrawID].second;
	qhandle_t shader = trap->R_RegisterShaderNoMip(pThisItem->id->visuals.itemIcon);
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, shader);
}

void JKG_ShopIconRight(itemDef_t* item, int nOwnerDrawID) {
	if (nShopScroll + nOwnerDrawID >= nNumberShopItems) {
		// Don't draw it if there isn't an item in the slot
		return;
	}

	itemInstance_t* pThisItem = vShopItems[nShopScroll + nOwnerDrawID].second;
	qhandle_t shader = trap->R_RegisterShaderNoMip(pThisItem->id->visuals.itemIcon);
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, shader);
}

//
// The selection highlight that shows on the item we have selected
//
void JKG_Shop_InventorySelection(itemDef_t* item, int nOwnerDrawID) {
	if (!bLeftSelected) {
		return; // An item on the left hand side is not selected.
	}
	if (nSelected != nInventoryScroll + nOwnerDrawID) {
		return; // The item we have selected is not this one.
	}

	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, item->window.background);
}

void JKG_Shop_ShopSelection(itemDef_t* item, int nOwnerDrawID) {
	if (bLeftSelected) {
		return; // An item on the right hand side is not selected.
	}
	if (nSelected != nShopScroll + nOwnerDrawID) {
		return; // The item we have selected is not this one.
	}
	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, item->window.background);
}

//
// The name of the item that shows on each button
//
extern void Item_Text_Paint(itemDef_t *item);
extern void Item_Text_Paint(itemDef_t* item, vec4_t& color);
void JKG_Shop_InventoryItemName(itemDef_t* item, int nOwnerDrawID) {
	if (nInventoryScroll + nOwnerDrawID >= nNumberInventoryItems) {
		memset(item->text, 0, sizeof(item->text));
		Item_Text_Paint(item); // FIXME: should we really be trying to paint a blank string?
		return; // There isn't an item in this slot.
	}
	itemInstance_t* pItem = vInventoryItems[nInventoryScroll + nOwnerDrawID].second;
	Q_strncpyz(item->text, pItem->id->displayName, sizeof(item->text));

	//figure out color based on item tier
	vec4_t color;
	JKG_SetTierColor(pItem->id->itemTier, color);
	item->window.flags |= WINDOW_TEXTCOLOR; //we're overriding the color
	Item_Text_Paint(item, color);
}

void JKG_Shop_ShopItemName(itemDef_t* item, int nOwnerDrawID) {
	if (nShopScroll + nOwnerDrawID >= nNumberShopItems) {
		memset(item->text, 0, sizeof(item->text));
		Item_Text_Paint(item); // FIXME: should we really be trying to paint a blank string?
		return; // There isn't an item in this slot.
	}
	itemInstance_t* pItem = vShopItems[nShopScroll + nOwnerDrawID].second;
	Q_strncpyz(item->text, pItem->id->displayName, sizeof(item->text));
	
	//figure out color based on item tier
	vec4_t color;
	JKG_SetTierColor(pItem->id->itemTier, color);
	item->window.flags |= WINDOW_TEXTCOLOR; //we're overriding the color
	Item_Text_Paint(item, color);
}

//
// The cost of the item that shows on each button
// 

void JKG_Shop_InventoryItemCost(itemDef_t* item, int nOwnerDrawID) {
	if (nInventoryScroll + nOwnerDrawID >= nNumberInventoryItems) {
		memset(item->text, 0, sizeof(item->text));
		Item_Text_Paint(item); // FIXME: should we really be trying to paint a blank string?
		return; // There isn't an item in this slot.
	}
	itemInstance_t* pItem = vInventoryItems[nInventoryScroll + nOwnerDrawID].second;

	char info[MAX_INFO_VALUE];
	trap->GetConfigString(CS_SERVERINFO, info, sizeof(info));

	if (!Q_stricmp(pItem->id->internalName, Info_ValueForKey(info, "jkg_startingGun")) && nNumberInventoryItems > 1)		//selling our starting gun is worth only one credit
		sprintf(item->text, "%i", 1);
	
	else // we only get 1/2 the cost back
	{
		sprintf(item->text, "%i", pItem->id->baseCost / 2 * pItem->quantity);
	}
		
	Item_Text_Paint(item);
}

void JKG_Shop_ShopItemCost(itemDef_t* item, int nOwnerDrawID) {
	if (nShopScroll + nOwnerDrawID >= nNumberShopItems) {
		memset(item->text, 0, sizeof(item->text));
		Item_Text_Paint(item); // FIXME: should we really be trying to paint a blank string?
		return; // There isn't an item in this slot.
	}
	itemInstance_t* pItem = vShopItems[nShopScroll + nOwnerDrawID].second;
	sprintf(item->text, "%i", pItem->id->baseCost);
	Item_Text_Paint(item);
}

void JKG_Shop_ShopAmmoCost(itemDef_t* item)
{
	if (!bLeftSelected)
	{
	return;
	}
	else if (nSelected < 0 || nSelected >= nNumberInventoryItems)
	{
		return;
	}

	itemInstance_t* pItem = vInventoryItems[nSelected].second;
	if (pItem->id->itemType != ITEM_WEAPON)
	{
		return;
	}

	JKG_Shop_UpdatePriceCheck();
	if (bPriceCheckComplete)
	{
		sprintf(item->text, "%i", nPriceCheckCost);
	}
	else
	{
		sprintf(item->text, "...");
	}

	Item_Text_Paint(item);
}

//
// These five functions are used to calculate the width of the text for alignment purposes
// See UI_OwnerDrawWidth in ui_main.cpp for more information.
//

char* JKG_Shop_LeftNameText(int ownerDrawID) {
	if (nInventoryScroll + ownerDrawID >= nNumberInventoryItems) {
		return nullptr;
	}
	itemInstance_t* pItem = vInventoryItems[nInventoryScroll + ownerDrawID].second;
	return pItem->id->displayName;
}

char* JKG_Shop_LeftPriceText(int ownerDrawID) {
	if (nInventoryScroll + ownerDrawID >= nNumberInventoryItems) {
		return nullptr;
	}
	itemInstance_t* pItem = vInventoryItems[nInventoryScroll + ownerDrawID].second;
	return va("%i", pItem->id->baseCost / 2 * pItem->quantity);

}

char* JKG_Shop_RightNameText(int ownerDrawID) {
	if (nShopScroll + ownerDrawID >= nNumberShopItems) {
		return nullptr;
	}
	itemInstance_t* pItem = vShopItems[nShopScroll + ownerDrawID].second;
	return pItem->id->displayName;
}

char* JKG_Shop_RightPriceText(int ownerDrawID) {
	if (nShopScroll + ownerDrawID >= nNumberShopItems) {
		return nullptr;
	}
	itemInstance_t* pItem = vShopItems[nShopScroll + ownerDrawID].second;
	return va("%i", pItem->id->baseCost);
}

char* JKG_ShopAmmoPriceText() {
	if (nSelected < 0 || nSelected >= nNumberInventoryItems || !bLeftSelected) {
		return nullptr;
	}
	itemInstance_t* pItem = vInventoryItems[nSelected].second;
	if (pItem->id->itemType != ITEM_WEAPON) {
		return nullptr;
	}

	JKG_Shop_UpdatePriceCheck();
	if (bPriceCheckComplete)
	{
		return va("%i", nPriceCheckCost);
	}
	else
	{
		return "...";
	}
}

/*#include <game/g_local.h>*/
char* JKG_ShopRefreshTimeText()
{
	/*if (jkg_announceShopRefresh.integer > 0
		&& ((level.time - level.startTime) > jkg_shop_replenish_time.integer * 1000 - 1))*/

	//need access to level.time to figure out how much refresh time is left
	int timeRemaining = 300;
	return va("%i", timeRemaining);
}

//
// The action that gets performed when we select an item
//

void JKG_Shop_SelectLeft(char** args) {
	int id;
	if (!Int_Parse(args, &id)) {
		trap->Print("Couldn't parse selectleft object\n");
		return;
	}
	if (id + nInventoryScroll >= nNumberInventoryItems) {
		nSelected = -1;
		return;
	}
	bLeftSelected = true;
	nSelected = nInventoryScroll + id;
	bPriceCheckComplete = false;
	nPriceCheckCost = -1;
	vShopItemDesc.clear();

	if (nSelected < 0)
	{
		return;
	}

	if (vInventoryItems[nSelected].second->id->itemType == ITEM_WEAPON)
	{
		Menu_ShowGroup(Menus_FindByName("jkg_shop"), "shop_ammobuttons", qtrue);

		// Do a price check if we haven't cached one yet
		for (auto it = vPriceCheckedAmmo.begin(); it != vPriceCheckedAmmo.end(); ++it)
		{
			if (it->first == vInventoryItems[nSelected].first)
			{
				bPriceCheckComplete = true;
				nPriceCheckCost = it->second;
				break;
			}
		}

		// Perform a price check on this item
		cgImports->SendClientCommand(va("ammopricecheck %i silent", vInventoryItems[nSelected].first));
	}
	else
	{
		Menu_ShowGroup(Menus_FindByName("jkg_shop"), "shop_ammobuttons", qfalse);
	}
}

void JKG_Shop_SelectRight(char** args) {
	int id;
	if (!Int_Parse(args, &id)) {
		trap->Print("Couldn't parse selectleft object\n");
		return;
	}
	if (id + nShopScroll >= nNumberShopItems) {
		nSelected = -1;
		return;
	}
	bLeftSelected = false;
	nSelected = nShopScroll + id;
	vShopItemDesc.clear();

	Menu_ShowGroup(Menus_FindByName("jkg_shop"), "shop_ammobuttons", qfalse);

	if (nSelected >= 0 && nSelected < vShopItems.size())
	{
		JKG_ConstructItemDescription(vShopItems[nSelected].second, vShopItemDesc);
	}
}

void JKG_Shop_Sort(char** args) {
	const char* side;
	const char* criteria;

	if (!String_Parse(args, &side)) {
		trap->Print("Couldn't parse shop_sort script\n");
		return;
	}
	else if (!String_Parse(args, &criteria)) {
		trap->Print("Couldn't parse shop_sort script\n");
		return;
	}

	if (!Q_stricmp(side, "left")) {
		if (!Q_stricmp(criteria, "name")) {
			trap->Cvar_Set("ui_inventorySortMode", "0");
			trap->Cvar_Set("ui_inventorySortType", va("%i", !ui_inventorySortType.integer));
		}
		else if (!Q_stricmp(criteria, "price")) {
			trap->Cvar_Set("ui_inventorySortMode", "1");
			trap->Cvar_Set("ui_inventorySortType", va("%i", !ui_inventorySortType.integer));
		}
		else {
			trap->Print("Side %s has bad criteria '%s' for filter!\n", side, criteria);
			return;
		}
		// Set selection cursor back to first so we go back to the top of the list automatically
		nInventoryScroll = 0;
	}
	else if (!Q_stricmp(side, "right")) {
		if (!Q_stricmp(criteria, "name")) {
			trap->Cvar_Set("ui_shopSortMode", "0");
			trap->Cvar_Set("ui_shopSortType", va("%i", !ui_shopSortType.integer));
		}
		else if (!Q_stricmp(criteria, "price")) {
			trap->Cvar_Set("ui_shopSortMode", "1");
			trap->Cvar_Set("ui_shopSortType", va("%i", !ui_shopSortType.integer));
		}
		else {
			trap->Print("Side %s has bad criteria '%s' for filter!\n", side, criteria);
			return;
		}
		// Set selection cursor back to first so we go back to the top of the list automatically
		nShopScroll = 0;
	}
	else {
		trap->Print("Bad side '%s'\n", side);
		return;
	}
	JKG_ConstructShopLists();
}

void JKG_Shop_SortSelectionName(itemDef_t* item, int ownerDrawID) {
	if (ownerDrawID && ui_shopSortMode.integer != 0) {
		return;
	}
	else if (!ownerDrawID && ui_inventorySortMode.integer != 0) {
		return;
	}

	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, item->window.background);
}

void JKG_Shop_SortSelectionPrice(itemDef_t* item, int ownerDrawID) {
	if (ownerDrawID && ui_shopSortMode.integer != 1) {
		return;
	}
	else if (!ownerDrawID && ui_inventorySortMode.integer != 1) {
		return;
	}

	trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h,
		0, 0, 1, 1, item->window.background);
}

void JKG_Shop_BuyItem(char** args) {
	if (bLeftSelected) {
		return; // Can't buy an item from your inventory
	}
	if (nSelected < 0 || nSelected >= nNumberShopItems) {
		return; // Invalid selection
	}
	cgImports->SendClientCommand(va("buyVendor %i", vShopItems[nSelected].first));
}

void JKG_Shop_SellItem(char** args) {
	if (!bLeftSelected) {
		return; // Can't sell an item from the shop
	}
	if (nSelected < 0 || nSelected >= nNumberInventoryItems) {
		return; // Invalid selection
	}
	cgImports->SendClientCommand(va("inventorySell %i", vInventoryItems[nSelected].first));
}

void JKG_Shop_BuyAmmo(char** args) {
	if (!bLeftSelected) {
		return; // Can't buy ammo for something that we don't own
	}
	if (nSelected < 0 || nSelected >= nNumberInventoryItems) {
		return; // Invalid selection
	}
	cgImports->SendClientCommand(va("buyammo %i", vInventoryItems[nSelected].first));
}

void JKG_Shop_Closed(char** args) {
	cgImports->SendClientCommand("closeVendor");
}

void JKG_ShopNotify(jkgShopNotify_e msg)
{
	// Only one type of notify, so no need to make a switch here.
	menuDef_t* menu = Menus_FindByName("jkg_shop");
	if (menu && Menus_ActivateByName("jkg_shop"))
	{
		JKG_ConstructShopLists();
		trap->Key_SetCatcher(trap->Key_GetCatcher() | KEYCATCH_UI);
	}
}

// Performed when a price check is returned by the server
void JKG_Shop_PriceCheckComplete(int nInventoryID, int nPrice)
{
	std::pair<int, int> pair = std::make_pair(nInventoryID, nPrice);

	// Remove the old cost record, if it exists
	for (auto it = vPriceCheckedAmmo.begin(); it != vPriceCheckedAmmo.end(); ++it)
	{
		if (it->first == nInventoryID)
		{
			it = vPriceCheckedAmmo.erase(it);
			bPriceCheckComplete = false;
			break;
		}
	}

	vPriceCheckedAmmo.push_back(pair);
}

// Ownerdraw: Draw the shop description line indicated
void JKG_Shop_DrawShopDescriptionLine(itemDef_t* item, int nOwnerDrawID)
{
	if (!bExamineMenuOpen)
	{
		return;
	}

	if (nOwnerDrawID >= vShopItemDesc.size())
	{
		item->text[0] = '\0';
		Item_Text_Paint(item);
		return;
	}

	Q_strncpyz(item->text, vShopItemDesc[nOwnerDrawID].c_str(), sizeof(item->text));
	Item_Text_Paint(item);
}

void JKG_Shop_Examine(char** args)
{
	bExamineMenuOpen = !bExamineMenuOpen;
	JKG_ConstructShopLists();
}