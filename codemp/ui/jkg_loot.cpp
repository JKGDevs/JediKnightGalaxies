////////////////////////////////////////////////////////////
//
// Jedi Knight Galaxies
//
// BodySearch/Loot UI Module
//
// Written by eezstreet
// Based off Pazaak UI by BobaFett
//
////////////////////////////////////////////////////////////

#include "ui_shared.h"
#include "ui_local.h"

#define MAX_LOOT_DRAW	10

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);
int MenuFontToHandle(int iMenuFont);

//Item Qualities
//Any changes to this enum will need to be logged in bg_items.h, too.
enum
{
	IQUAL_LOWQUAL,
	IQUAL_NORMAL,
	IQUAL_HIQUAL,
	IQUAL_SOCKETED,
	IQUAL_UNCOMMON,
	IQUAL_RARE,
	IQUAL_UNIQUE,
	IQUAL_EPIC,
	IQUAL_MAX,
};

//Callback Types
enum
{
	LODLGRSP_OK,
	LODLGRSP_CANCEL,
	LODLGRSP_YES,
	LODLGRSP_NO,
};

//Loot Dialog Errors
enum
{
	LODLG_ERR_OUTOFSYNC,		//Out of synch
	LODLG_ERR_INVALIDENT,		//Invalid entity #
	LODLG_OVERENCUMBERED,		//You do not have enough room
	LODLG_OTHER,
	LODLG_MAX,
};

//Loot Buttons
enum
{
	LOOT_EXIT,
};

typedef void (*PDlgCallback)(int response);


static struct
{
	qboolean	dlgActive;
	int			dlgType;		// DLGTYPE_*
	char		dlgText1[256];
	char		dlgText2[256];
	int			dlgid;			// DLGID_*
	PDlgCallback callback;
} lootDLGData;

static struct
{
	qboolean	loot_running;
	int			entListener; //What ent we are looting from.
} lootUIData;

typedef struct
{
	char		name[64];
	int			quality;
} basicItemData;

basicItemData lootInContainer[MAX_LOOT_DRAW];


enum {
	DLGTYPE_BLANK,		// No prompt (server will close this)
	DLGTYPE_YESNO,		// Yes/No
	DLGTYPE_OK,			// Ok only
};

/*static void JKG_LootDialogShow(const char *line1, const char *line2, int type, PDlgCallback callback)
{
	menuDef_t *menu;

	lootDLGData.dlgActive = qtrue;
	
	if (line1) {
		Q_strncpyz(&lootDLGData.dlgText1[0], line1, 255);
	} else {
		lootDLGData.dlgText1[0] = 0;
	}

	if (line2) {
		Q_strncpyz(&lootDLGData.dlgText2[0], line2, 255);
	} else {
		lootDLGData.dlgText2[0] = 0;
	}

	lootDLGData.dlgType = type;
	lootDLGData.callback = callback;

	menu = Menus_FindByName("jkg_lootmenu");

	if (!menu) {
		return;
	}

	Menu_ClearFocus(menu);

	Menu_ShowItemByName(menu, "dialog", qtrue);

	if (type == DLGTYPE_OK) {
		Menu_ShowItemByName(menu, "btn_dialogok", qtrue);
	} else if (type == DLGTYPE_YESNO) {
		Menu_ShowItemByName(menu, "btn_dialogyesno", qtrue);
	}
}*/

static void JKG_LootDialogHide(void)
{
	menuDef_t *menu;

	lootDLGData.dlgActive = qfalse;

	menu = Menus_FindByName("jkg_lootmenu");

	if (!menu) {
		return;
	}

	Menu_ShowItemByName(menu, "dialog", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogok", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogyesno", qfalse);

	Menu_ClearFocus(menu);
}

void JKG_Loot_ScriptedDialogButton(char **args)
{
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	if (button < 0 || button > 1) {
		// Invalid button
		return;
	}
	if (lootDLGData.dlgActive) {
		(*lootDLGData.callback)(button);
	}
	JKG_LootDialogHide();
}

void JKG_LootScript_Button(char **args)
{
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	switch(button) {
		case LOOT_EXIT:
			cgImports->SendClientCommand("~inv clo");
			break;
	}
}

void JKG_LootScript_Item(char **args)
{
	int item;
	const char *text;

	if(!Int_Parse(args, &item)){
		return;
	}
	if(item < 0 && item >= MAX_LOOT_DRAW){
		//invalid item, out of range
		return;
	}

	text = lootInContainer[item].name;

	if(text == NULL || !Q_stricmp(text, "NULLSLOT") || !Q_stricmp(text, "(null)")){
		//invalid item, bad item
		return;
	}

	cgImports->SendClientCommand( va("bodyLoot %i %i", lootUIData.entListener, item ));
	Q_strncpyz(lootInContainer[item].name, "NULLSLOT", sizeof(lootInContainer[item].name)); //We strncpyz NULLSLOT to tell the game not to render this slot
}

void JKG_Loot_DrawDialog(int line, float x, float y, float w, float h)
{
	float width;
	const char *text;

	if (!lootDLGData.dlgActive) {
		return;
	}

	if (line == 1) {
		text = lootDLGData.dlgText1;
	} else{
		text = lootDLGData.dlgText2;
	}

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5f;

	x = x + ((w / 2) - (width / 2));
	trap->R_Font_DrawString(	x, y, text, colorWhite, MenuFontToHandle(1) | 0x80000000 , -1, 0.5f);
}

void JKG_Loot_DrawItemInstanceSlot(int slot, float x, float y, float w, float h)
{
//	float width;		//might need this later...
	const char *text;

	if(!lootUIData.loot_running)
		return;

	text = lootInContainer[slot].name;

	if(text != NULL && Q_stricmp(text, "(null)") && Q_stricmp(text, "NULLSLOT"))
		trap->R_Font_DrawString(x, y, text, colorWhite, MenuFontToHandle(1) | 0x80000000, -1, 0.5f);
}

void JKG_Loot_DestroyMenu(void)
{
	lootUIData.loot_running = qfalse;
	trap->Key_SetCatcher( trap->Key_GetCatcher() & ~KEYCATCH_UI );
	Menus_CloseByName("jkg_lootmenu");
}

void JKG_ProcessLoot(){
	parsebuff_t	pb;
	const char	*token, *nextToken;
	int			/*i,*/ j/*, temp*/;
	//menuDef_t *menu;

	nextToken = nullptr;

	Pzk_InitParseBuff(&pb);

	while(1)
	{
		token = Pzk_NextToken(&pb);
		if (!token || !token[0]) break;

		if(!Q_stricmp(token, "ol")) {
			//Initialize menu
			Menus_CloseAll();
			if (Menus_ActivateByName("jkg_lootmenu"))
				trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
			Menu_ClearFocus(Menus_FindByName("jkg_lootmenu"));
			lootUIData.loot_running = qtrue;
			continue;
		}
		if(!Q_stricmp(token, "cl")) {
			//Destroy menu
			JKG_Loot_DestroyMenu();
			cgImports->SendClientCommand( "~inv cls" );
		}
		if(!Q_stricmp(token, "clw")) {
			//Close whole menu, only used in certain situations
			JKG_Loot_DestroyMenu();
		}
		if(!Q_stricmp(token, "sil")) {
			//Send item list to client
			for(j = 0; j < MAX_LOOT_DRAW; j++)
			{
				nextToken = Pzk_NextToken(&pb);
				memset(&lootInContainer[j], 0, sizeof(basicItemData));
				if(nextToken == NULL)
				{
					break;
				}
				Q_strncpyz(lootInContainer[j].name, nextToken, sizeof(lootInContainer[j].name));
			}
		}
		if(!Q_stricmp(token, "sen")) {
			//Set pointer to ent (ent number)
			lootUIData.entListener = atoi(Pzk_NextToken(&pb));
		}
	}
}
