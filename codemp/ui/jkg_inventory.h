#pragma once

#include "game/bg_items.h"
#include "game/bg_weapons.h"
#include "qcommon/q_shared.h"
#include <cctype>

typedef enum {
	JKGIFILTER_ALL,
	JKGIFILTER_WEAPONS,
	JKGIFILTER_ARMOR,
	JKGIFILTER_CONSUMABLES,
	JKGIFILTER_MISC,
	JKGIFILTER_TOOLS,
	JKGIFILTER_AMMO,
	/*JKGFILTER_CRAFTING,*/
} jkgFilterItems_t;


void JKG_Inventory_UpdateNotify(jkgInventoryNotify_e msg);

void JKG_Inventory_OwnerDraw_CreditsText(itemDef_t* item);
void JKG_Inventory_OwnerDraw_ItemIcon(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_ItemName(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_SelHighlight(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_SelItemIcon(itemDef_t* item);
void JKG_Inventory_OwnerDraw_SelItemName(itemDef_t* item);
void JKG_Inventory_OwnerDraw_SelItemDesc(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_ItemTagTop(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_ItemTagBottom(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_Interact(itemDef_t* item, int ownerDrawID);
void JKG_Inventory_OwnerDraw_Interact_Button(int ownerDrawID, int key);
void JKG_Inventory_SelectItem(char** args);
void JKG_Inventory_ArrowUp(char** args);
void JKG_Inventory_ArrowDown(char** args);
void JKG_Inventory_Use(char** args);
void JKG_Inventory_Destroy(char** args);
void JKG_Inventory_ACISlot(char** args);
void JKG_Inventory_ACISlotAuto(char** args);
void JKG_Inventory_ACIRemove(char** args);
void JKG_Inventory_EquipArmor(char** args);
void JKG_Inventory_UnequipArmor(char** args);
void JKG_Inventory_Open(char** args);
void JKG_ConstructInventoryList();
void JKG_ConstructItemDescription(itemInstance_t* pItem, std::vector<std::string>& vDescLines);
void JKG_SplitDescriptionLines(const std::string& s, std::vector<std::string>& vDescLines);
qboolean JKG_Inventory_HandleKey(int key);
void JKG_LightenTextColor(vec4_t& color);
void JKG_SetTierColor(int tier, vec4_t& color);


//
// Shop specific stuff
//

void JKG_ConstructShopLists();
void JKG_ShopInventorySortChanged();
void JKG_ShopArrow(char** args);
void JKG_ShopIconLeft(itemDef_t* item, int nOwnerDrawID);
void JKG_ShopIconRight(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_InventorySelection(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_ShopSelection(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_InventoryItemName(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_ShopItemName(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_InventoryItemCost(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_ShopItemCost(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_ShopAmmoCost(itemDef_t* item);
void JKG_Shop_SelectLeft(char** args);
void JKG_Shop_SelectRight(char** args);
char* JKG_Shop_LeftNameText(int ownerDrawID);
char* JKG_Shop_LeftPriceText(int ownerDrawID);
char* JKG_Shop_RightNameText(int ownerDrawID);
char* JKG_Shop_RightPriceText(int ownerDrawID);
char* JKG_ShopAmmoPriceText();
char* JKG_ShopRefreshTimeText();
void JKG_Shop_Sort(char** args);
void JKG_Shop_SortSelectionName(itemDef_t* item, int ownerDrawID);
void JKG_Shop_SortSelectionPrice(itemDef_t* item, int ownerDrawID);
void JKG_Shop_BuyItem(char** args);
void JKG_Shop_BuyAmmo(char** args);
void JKG_Shop_SellItem(char** args);
void JKG_Shop_Closed(char** args);
void JKG_ScrollShop(qboolean bUp, int nMouseX, int nMouseY);
void JKG_ShopNotify(jkgShopNotify_e msg);
void JKG_Shop_PriceCheckComplete(int nInventoryID, int nPrice);
void JKG_Shop_DrawShopDescriptionLine(itemDef_t* item, int nOwnerDrawID);
void JKG_Shop_Examine(char** args);