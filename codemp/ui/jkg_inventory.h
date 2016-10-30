#pragma once

#include "game/bg_items.h"
#include "game/bg_weapons.h"
#include "qcommon/q_shared.h"

extern vmCvar_t ui_inventoryFilter;

#define MAX_XML_BUFFER_SIZE	4096

typedef enum {
	JKGIFILTER_ALL,
	JKGIFILTER_WEAPONS,
	JKGIFILTER_ARMOR,
	JKGIFILTER_CONSUMABLES,
	JKGIFILTER_MISC,
} jkgFilterItems_t;

void JKG_Shop_OpenDialog(char **args);
void JKG_Shop_CloseDialog ( char **args );
void JKG_Shop_ItemSelect(char **args);
void JKG_Shop_ArrowPrev(char **args);
void JKG_Shop_ArrowNext(char **args);
void JKG_Shop_Update(char **args);
void JKG_Shop_UpdateShopStuff(int filterVal);
void JKG_Shop_ClearFocus(char **args);
void JKG_Shop_BuyConfirm_Yes(char **args);
void JKG_Shop_BuyConfirm_No(char **args);
void JKG_Shop_UpdateCreditDisplay(void);
void JKG_Shop_BuyConfirm_Display(char **args);
void JKG_Shop_OpenInventoryMenu(char **args);
void JKG_Shop_UpdateNotify(int msg);
void JKG_Inventory_CloseFromShop( char **args );

void JKG_Shop_RestoreShopMenu(void);

void JKG_Shop_ArrowPrevClean(void);
void JKG_Shop_ArrowNextClean(void);

void JKG_Inventory_CheckACIKeyStroke(int key);
void JKG_Inventory_UpdateNotify(int msg);

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