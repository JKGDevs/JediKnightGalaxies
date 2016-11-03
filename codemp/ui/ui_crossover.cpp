// ui_crossover.c -- Crossover API module for UI
// Copyright (c) 2013 Jedi Knight Galaxies

#include "ui_local.h"

cgCrossoverExports_t *cgImports;

uiCrossoverExports_t ui;

static qboolean coTrapEscape = qfalse;

qboolean UI_RunSvCommand(const char *command);
void JKG_PartyMngt_UpdateNotify(int msg);
void JKG_ConstructShopLists();
void JKG_Shop_UpdateNotify(int msg) {
	menuDef_t* menu = Menus_FindByName("jkg_shop");
	if (menu && Menus_ActivateByName("jkg_shop"))
	{
		JKG_ConstructShopLists();
		trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
	}
}
void JKG_Inventory_UpdateNotify(int msg);
extern void JKG_ConstructInventoryList();

void JKG_ForceItemMenuUpdates() {
	menuDef_t* focusedMenu = Menu_GetFocused();
	if (focusedMenu == nullptr) {
		return; // No focused menu?
	}
	if (!Q_stricmp(focusedMenu->window.name, "jkg_inventory")) {
		JKG_ConstructInventoryList();
	}
	else if (!Q_stricmp(focusedMenu->window.name, "jkg_shop")) {
		JKG_ShopInventorySortChanged();
	}
}

void CO_SetEscapeTrapped( qboolean trapped )
{
	coTrapEscape = trapped;
}

uiCrossoverExports_t *UI_InitializeCrossoverAPI( cgCrossoverExports_t *cg )
{
	cgImports = cg;

	ui.HandleServerCommand = UI_RunSvCommand;
	ui.InventoryNotify = JKG_Inventory_UpdateNotify;
	ui.PartyMngtNotify = JKG_PartyMngt_UpdateNotify;
	ui.SetEscapeTrap = CO_SetEscapeTrapped;
	ui.ShopNotify = JKG_Shop_UpdateNotify;
	ui.ItemsUpdated = JKG_ForceItemMenuUpdates;

	return &ui;
}
