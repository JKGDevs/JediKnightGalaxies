#include "../game/bg_items.h"
#include "../game/bg_weapons.h"
#include "ui_local.h"
#include "jkg_inventory.h"
#include <expat.h>

using namespace std;

struct
{
    qboolean active;
    menuDef_t *menu;
    int selectedShopItem;
	qboolean inventoryOpen;
} shopState;

vector<itemInstance_t> UIunfilteredShopItems;
vector<itemInstance_t> UIshopItems;
static int UInumUnfilteredShopItems = 0;
static int UInumShopItems = 0;			//Number of items in the shop at the time

static int shopMenuPosition = 0;		//First entry's position in the UIshopItems array
static int previousFilter = JKGIFILTER_ALL;

void JKG_Shop_BuyConfirm_No(char **args);

void JKG_Shop_ClearFeederItems(void)
{
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_dummyFeeder");
	listBoxDef_t *listPtr = item->typeData.listbox;
	int numElements = listPtr->elementHeight;
	int i;

	for(i = 0; i < numElements; i++)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederIMG%i", i+1), qfalse);
		Menu_ShowItemByName(shopState.menu, va("shop_feederTXT%i", i+1), qfalse);
		Menu_ShowItemByName(shopState.menu, va("shop_feederAII%i", i+1), qfalse);
	}
}

void JKG_Shop_ClearSlot(int slotNum)
{
	itemDef_t *item;
	item = Menu_FindItemByName(shopState.menu, va("shop_feederIMG%i", slotNum));
	if(item)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederIMG%i", slotNum), qfalse);
	}
	item = Menu_FindItemByName(shopState.menu, va("shop_feederTXT%i", slotNum));
	if(item)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederTXT%i", slotNum), qfalse);
	}
	item = Menu_FindItemByName(shopState.menu, va("shop_feederAII%i", slotNum));
	if(item)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederAII%i", slotNum), qfalse);
	}
	item = Menu_FindItemByName(shopState.menu, va("shop_feederHIL%i", slotNum));
	if(item)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederHIL%i", slotNum), qfalse);
	}
	Menu_ShowItemByName(shopState.menu, va("shop_feederBO%i", slotNum), qfalse);
}

void JKG_Shop_ClearHighlights(int maxElements)
{
	int i;
	for(i = 1; i < maxElements+1; i++)
	{
		Menu_ShowItemByName(shopState.menu, va("shop_feederHIL%i", i), qfalse);
		Menu_ShowItemByName(shopState.menu, va("shop_feederH2L%i", i), qfalse);
		Menu_ShowItemByName(shopState.menu, va("shop_feederH3L%i", i), qfalse);
	}
	Menu_ShowGroup(shopState.menu, "shop_preview", qfalse);
	Menu_ShowGroup(shopState.menu, "shop_preview_examine", qfalse);
}

void JKG_Shop_UpdateShopStuff(int filterVal)
{
	//You should call this function whenever the screen needs updating (such as a change in state)
	//Variable declarations
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_dummyFeeder");
	itemDef_t *item2;
	listBoxDef_t *listPtr = item->typeData.listbox;
	int numElements = listPtr->elementHeight;
	int i;
	itemData_t *lookupTable = (itemData_t *)cgImports->InventoryDataRequest(6);

	//Clear the crapola
	UIshopItems.clear();
	UInumShopItems = 0;

	//Update the filters, first and foremost
	itemInstance_t* itemInstances = (itemInstance_t*)cgImports->InventoryDataRequest(4);
	UIunfilteredShopItems.clear();
	UInumUnfilteredShopItems = *(size_t*)cgImports->InventoryDataRequest( 5 );
	for (int i = 0; i < UInumUnfilteredShopItems; i++) {
		UIunfilteredShopItems.push_back(itemInstances[i]);
	}

	//if(ui_inventoryFilter.integer != JKGIFILTER_ALL)
	{
		for(i = 0; i < UInumUnfilteredShopItems; i++)
		{
			//int itemID = UIunfilteredShopItems[i].id->itemID;
			switch(filterVal)
			{
				case JKGIFILTER_ALL:
					UIshopItems.push_back(UIunfilteredShopItems[i]);
					break;
				case JKGIFILTER_ARMOR:
					if(UIunfilteredShopItems[i].id->itemType == ITEM_ARMOR || UIunfilteredShopItems[i].id->itemType == ITEM_CLOTHING)
					{
						UIshopItems.push_back(UIunfilteredShopItems[i]);
					}
					break;
				case JKGIFILTER_WEAPONS:
					if(UIunfilteredShopItems[i].id->itemType == ITEM_WEAPON)
					{
						UIshopItems.push_back(UIunfilteredShopItems[i]);
					}
					break;
				case JKGIFILTER_CONSUMABLES:
					if(UIunfilteredShopItems[i].id->itemType == ITEM_CONSUMABLE)
					{
						UIshopItems.push_back(UIunfilteredShopItems[i]);
					}
					break;
				case JKGIFILTER_MISC:
					if(UIunfilteredShopItems[i].id->itemType == ITEM_UNKNOWN)
					{
						UIshopItems.push_back(UIunfilteredShopItems[i]);
					}
					break;
			}
		}
	}
	UInumShopItems = UIshopItems.size();

	//Update all the shop items
	for(i = 0; i < numElements; i++)
	{
		if((shopMenuPosition + i) >= UInumShopItems)
		{
			JKG_Shop_ClearSlot(i+1);
			continue;	//If we're past our limit...don't draw stuff!
		}
		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederIMG%i", i+1));
		if(item2)
		{
			//Image -> get the item icon for this item
			item2->window.background = trap->R_RegisterShaderNoMip(UIshopItems[i+shopMenuPosition].id->visuals.itemIcon); //TODO: precache me
			Menu_ShowItemByName(shopState.menu, va("shop_feederIMG%i", i+1), qtrue);
		}

		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederTXT%i", i+1));
		if(item2)
		{
			//Text-> First line (aka big line) is the name, second line is the cost
			Menu_ShowItemByName(shopState.menu, va("shop_feederTXT%i", i+1), qtrue);
			strcpy(item2->text, UIshopItems[i+shopMenuPosition].id->displayName);
			if(UIshopItems[i+shopMenuPosition].id->baseCost == 0)
			{
				sprintf(item->text2, "Free");
			}
			else
			{
				sprintf(item2->text2, "Cost: %i", UIshopItems[i+shopMenuPosition].id->baseCost);
			}
			item2->textRect.w = 0;
		}

		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederAII%i", i+1));
		if(item2)
		{
			//Is this item already in our inventory? If so, draw a neat little graphic saying it is
			Menu_ShowItemByName(shopState.menu, va("shop_feederAII%i", i+1), qtrue);
		}

		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederBO%i", i+1));
		if(item2)
		{
			int credits = *(int*)cgImports->InventoryDataRequest( 3 );
			if(credits < UIshopItems[i+shopMenuPosition].id->baseCost)
			{
				Menu_ShowItemByName(shopState.menu, va("shop_feederBO%i", i+1), qtrue);
			}
			else
			{
				Menu_ShowItemByName(shopState.menu, va("shop_feederBO%i", i+1), qfalse);
			}
		}
	}
	//Update the selection data
	if(shopState.selectedShopItem > 0)
	{
		int shopSlotNumber = shopState.selectedShopItem-shopMenuPosition;
		if(shopSlotNumber < 0)
		{
			//Hide it all
hideItAll:
			shopState.selectedShopItem = 0;
			JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
			return;
		}
		else if(shopSlotNumber > numElements)
		{
			//Hide the selection stuff
			goto hideItAll;
		}
		//Hide all the other highlights
		JKG_Shop_ClearHighlights(numElements);

		//Draw the selection stuff
		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederHIL%i", shopSlotNumber));
		if(item2)
		{
			Menu_ShowItemByName(shopState.menu, va("shop_feederHIL%i", shopSlotNumber), qtrue);
		}
		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederH2L%i", shopSlotNumber));
		if(item2)
		{
			Menu_ShowItemByName(shopState.menu, va("shop_feederH2L%i", shopSlotNumber), qtrue);
		}
		item2 = Menu_FindItemByName(shopState.menu, va("shop_feederH3L%i", shopSlotNumber));
		if(item2)
		{
			Menu_ShowItemByName(shopState.menu, va("shop_feederH3L%i", shopSlotNumber), qtrue);
		}

		//Draw the preview stuff
		item2 = Menu_FindItemByName(shopState.menu, "shop_previewIcon");
		if(item2)
		{
			item2->window.background = trap->R_RegisterShaderNoMip(UIshopItems[shopSlotNumber+shopMenuPosition-1].id->visuals.itemIcon);
		}

		item2 = Menu_FindItemByName(shopState.menu, "shop_previewItemName");
		if(item2)
		{
			strcpy(item2->text, UIshopItems[shopSlotNumber+shopMenuPosition-1].id->displayName);
			//Recalculate the positioning
			item2->textRect.w = 0;
		}

		item2 = Menu_FindItemByName(shopState.menu, "shop_previewItemCost");
		if(item2)
		{
			sprintf(item2->text, va("Cost: %i", UIshopItems[shopSlotNumber+shopMenuPosition-1].id->baseCost));
			//Recalculate the positioning
			item2->textRect.w = 0;
		}

		//Show the preview group
		Menu_ShowGroup(shopState.menu, "shop_preview", qtrue);
		Menu_ShowGroup(shopState.menu, "shop_preview_examine", qtrue);
	}
	else
	{
		//Hide the selection stuff
		JKG_Shop_ClearHighlights(numElements);
	}
}

void JKG_Shop_UpdateCreditDisplay(void)
{
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shopmain_credits");
	int credits = *(int*)cgImports->InventoryDataRequest( 3 );
	if(!item)
	{
		return;
	}
	sprintf(item->text, "Credits: %i", credits);
	item->textRect.w = 0; //Hack to recalculate positioning
}

void JKG_Shop_UpdateNotify(int msg)
{
	switch(msg)
	{
		case 0:
			//Open the shop menu
			memset(&shopState, 0, sizeof(shopState));
			shopState.active = qtrue;

			shopState.menu = Menus_FindByName("jkg_shop");
			if(shopState.menu && Menus_ActivateByName("jkg_shop"))
			{
				trap->Key_SetCatcher (trap->Key_GetCatcher() | KEYCATCH_UI /*& ~KEYCATCH_CONSOLE*/);
			}
			shopState.selectedShopItem = 0;
			break;
		case 1:
			{
				JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
				JKG_Shop_UpdateCreditDisplay();
			}
			break;
	}
}

void JKG_Shop_OpenDialog(char **args)
{
	trap->Cvar_Set ("ui_hidehud", "1");
	shopState.active = qtrue;
	shopState.inventoryOpen = qfalse;

	Menu_ClearFocus(shopState.menu);
	JKG_Shop_UpdateCreditDisplay();

	//JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
	JKG_Shop_BuyConfirm_No(NULL);
}

void JKG_Shop_CloseDialog(char **args)
{
	if(!shopState.inventoryOpen)
	{
		trap->Cvar_Set ("ui_hidehud", "0");
	}
	shopState.active = qfalse;
	cgImports->SendClientCommand ("closeVendor");
}

void JKG_Shop_ItemSelect(char **args)
{
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_dummyFeeder");
	listBoxDef_t *listPtr = item->typeData.listbox;
	itemData_t *lookupTable = (itemData_t *)cgImports->InventoryDataRequest( 6 );
	int credits = *(int*)cgImports->InventoryDataRequest( 3 );
	int arg0 = atoi(args[0]);
	if(arg0 <= 0)
	{
		//Not valid
		return;
	}
	else if(arg0 > listPtr->elementHeight)
	{
		//Too high...bug?
		return;
	}
	else if (shopMenuPosition + arg0 > UInumShopItems) {
		return;
	}

	shopState.selectedShopItem = shopMenuPosition+arg0;
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
}

void JKG_Shop_ArrowPrevClean(void)
{
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_dummyFeeder");
	listBoxDef_t *listPtr = item->typeData.listbox;
	int numElements = listPtr->elementHeight;
	if(shopMenuPosition-1 < 0)
	{
		shopMenuPosition = 0;
	}
	else
	{
		shopMenuPosition--;
	}
	//Check and see if the currently selected item is offscreen
	if(shopState.selectedShopItem > shopMenuPosition+numElements)
	{
		//It's offscreen, so deselect it
		shopState.selectedShopItem = 0;
	}
	//Update the shop menu
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
}
void JKG_Shop_ArrowNextClean(void)
{
	int arg = 1;
	
	//Check if we're approaching out of bounds area
	if(arg+shopMenuPosition > UInumShopItems)
	{
		shopMenuPosition = UInumShopItems;
	}
	else
	{
		shopMenuPosition += arg;
	}

	//Check and see if the currently selected item is offscreen
	if(shopState.selectedShopItem < shopMenuPosition)
	{
		//Deselect it
		shopState.selectedShopItem = 0;
	}

	//Update the shop menu display
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
}
void JKG_Shop_ArrowPrev(char **args)
{
	int arg = atoi(args[0]);
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_dummyFeeder");
	listBoxDef_t *listPtr = item->typeData.listbox;
	int numElements = listPtr->elementHeight;

	//Check if we're approaching 0
	if(shopMenuPosition-arg < 0)
	{
		shopMenuPosition = 0;
	}
	else
	{
		shopMenuPosition -= arg;
	}
	//Check and see if the currently selected item is offscreen
	if(shopState.selectedShopItem > shopMenuPosition+numElements)
	{
		//It's offscreen, so deselect it
		shopState.selectedShopItem = 0;
	}
	//Update the shop menu
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
}

void JKG_Shop_ArrowNext(char **args)
{
	int arg = atoi(args[0]);
	
	//Check if we're approaching out of bounds area
	if(arg+shopMenuPosition > UInumShopItems)
	{
		shopMenuPosition = UInumShopItems;
	}
	else
	{
		shopMenuPosition += arg;
	}

	//Check and see if the currently selected item is offscreen
	if(shopState.selectedShopItem < shopMenuPosition)
	{
		//Deselect it
		shopState.selectedShopItem = 0;
	}

	//Update the shop menu display
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
}

void JKG_Shop_Update(char **args)
{
	//Double hack
	if(atoi(args[0]) <= 0)
	{
		shopState.selectedShopItem = 0;
		JKG_Shop_UpdateShopStuff(atoi(args[0])*-1);
	}
	else
	{
		JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
	}
}

void JKG_Shop_ClearFocus(char **args)
{
	shopState.selectedShopItem = 0;
}

void JKG_Shop_BuyConfirm_Yes(char **args)
{
	//Buying the item
	//Make sure we don't buy an item that we didn't mean to buy
	int desiredItemID = UIshopItems[shopState.selectedShopItem-1].id->itemID;
	int actualItemID;
	Menu_ShowGroup(shopState.menu, "shop_buyconfirm", qfalse);
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
	actualItemID = UIshopItems[shopState.selectedShopItem-1].id->itemID;
	if(actualItemID == desiredItemID)
	{
		// Here's a thought...why not send the item ID _instead_ of the selected index?
		// 100% guaranteed to work then --eez
		cgImports->SendClientCommand(va("buyVendor %i", shopState.selectedShopItem-1));
	}
	Menu_ItemDisable(shopState.menu, "shop_preview", qfalse);
	Menu_ItemDisable(shopState.menu, "shop_feederSel", qfalse);
	Menu_ItemDisable(shopState.menu, "shop_arrows", qfalse);
	Menu_ItemDisable(shopState.menu, "main_dialog", qfalse);
	JKG_Shop_UpdateCreditDisplay();
}

void JKG_Shop_BuyConfirm_No(char **args)
{
	Menu_ShowGroup(shopState.menu, "shop_buyconfirm", qfalse);
	JKG_Shop_UpdateShopStuff(ui_inventoryFilter.integer);
	Menu_ItemDisable(shopState.menu, "shop_preview", qfalse);
	Menu_ItemDisable(shopState.menu, "shop_feederSel", qfalse);
	Menu_ItemDisable(shopState.menu, "shop_arrows", qfalse);
	Menu_ItemDisable(shopState.menu, "main_dialog", qfalse);
}

void JKG_Shop_BuyConfirm_Display(char **args)
{
	//This sets the text on the display
	itemDef_t *item = Menu_FindItemByName(shopState.menu, "shop_buyconfirm_text");
	//itemData_t *lookupTable = (itemData_t *)cgImports->InventoryDataRequest( 6 );
	if(item)
	{
		strcpy(item->text, va("Buy this item for %i credits?", UIshopItems[shopState.selectedShopItem - 1].id->baseCost));
		item->textRect.w = 0;
	}
}

void JKG_Shop_OpenInventoryMenu(char **args)
{
	Menus_CloseByName(shopState.menu->window.name);
}

void JKG_Shop_RestoreShopMenu(void)
{
	Menus_OpenByName("jkg_shop");
}