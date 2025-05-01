// Jedi Knight Galaxies UI Scripts

#include "ui_shared.h"
#include "ui_local.h"

#define KEYWORDHASH_SIZE	512

typedef struct JKGkeywordHashUI_s
{
	char *keyword;
	void (*func)( char **args );
	struct JKGkeywordHashUI_s *next;
} JKGkeywordHashUI_t;

typedef struct JKGkeywordHashSv_s
{
	char *keyword;
	void (*func)();
	struct JKGkeywordHashSv_s *next;
} JKGkeywordHashSv_t;

int JKGKeywordHash_Key(char *keyword) {
	int hash = 0;
	for (int i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (KEYWORDHASH_SIZE-1);
	return hash;
}

void JKGKeywordHashUI_Add(JKGkeywordHashUI_t *table[], JKGkeywordHashUI_t *key) {
	int hash;

	hash = JKGKeywordHash_Key(key->keyword);
	key->next = table[hash];
	table[hash] = key;
}

JKGkeywordHashUI_t *JKGKeywordHashUI_Find(JKGkeywordHashUI_t *table[], char *keyword)
{
	JKGkeywordHashUI_t *key;
	int hash;

	hash = JKGKeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return 0;
}

void JKGKeywordHashSv_Add(JKGkeywordHashSv_t *table[], JKGkeywordHashSv_t *key) {
	int hash;

	hash = JKGKeywordHash_Key(key->keyword);
	key->next = table[hash];
	table[hash] = key;
}

JKGkeywordHashSv_t *JKGKeywordHashSv_Find(JKGkeywordHashSv_t *table[], char *keyword)
{
	JKGkeywordHashSv_t *key;
	int hash;

	hash = JKGKeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return 0;
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);

void JKG_SendEscape(char ** args) {
	cgImports->EscapeTrapped();
}

#include "jkg_conversations.h"
#include "jkg_pazaak.h"
#include "jkg_partymanager.h"
#include "jkg_slice.h"
#include "jkg_inventory.h"

extern void UI_BuildQ3Model_List( void );
void JKG_Hack_RefreshModelList(char **args)
{
	char info[MAX_INFO_STRING];
	info[0] = '\0';
	trap->GetConfigString(CS_SERVERINFO, info, sizeof(info));

	trap->Cvar_Set("ui_gameType", Info_ValueForKey(info, "g_gametype"));
	ui_gametype.integer = atoi( Info_ValueForKey(info, "g_gametype") );
	if(ui_gametype.integer >= GT_TEAM)
	{
		// Don't do this in FFA/non gang wars mode. Causes unnecessary hitches.
		UI_BuildQ3Model_List();
	}
}

// JKG scripts used in .menu files
JKGkeywordHashUI_t JKGScripts[] = {
	// Generic
	{"sendescape",			JKG_SendEscape,				    0		},
	{"refreshmodellist",	JKG_Hack_RefreshModelList,		0		},
	// Convo
	{"jkg_convoresponse",	Conv_Script_ProcessChoice,		0		},
	{"jkg_convoslider",		Conv_Script_ConvoSlider,		0		},
	{"jkg_teaction",		Conv_Script_ProcessTextEntry,	0		},
	// Pazaak
	{"pzk_handhover",		Pazaak_Script_HandSlotHover,	0		},
	{"pzk_usecard",			Pazaak_Script_UseCard,			0		},
	{"pzk_btnpress",		Pazaak_Script_ButtonPress,		0		},
	{"pzk_dlgbutton",		Pazaak_Script_DialogButton,		0		},
	{"pzk_flip",			Pazaak_Script_Flip,				0		},
	{"pzk_onesc",			Pazaak_Script_OnEsc,			0		},
	{"pzk_cardhover",		Pazaak_Script_CardHover,		0		},
	{"pzk_selectcard",		Pazaak_Script_SelectCard,		0		},
	{"pzk_sdhover",			Pazaak_Script_SDHover,			0		},
	{"pzk_removesd",		Pazaak_Script_RemoveSD,			0		},
	// Team Management
	{"pmngt_dlgbutton",		PartyMngt_Script_DialogButton,	0		},
	{"pmngt_button",		PartyMngt_Script_Button,		0		},
	{"pmngt_open",			PartyMngt_Script_OpenDlg,		0		},
	{"pmngt_close",			PartyMngt_Script_CloseDlg,		0		},
	// Slicing
	{"slc_runprog",			JKG_Slice_Script_RunProgram,	0		},
	{"slc_dlgbutton",		JKG_Slice_Script_DialogButton,	0		},
	{"slc_stopslice",		JKG_Slice_Script_StopSlicing,	0		},
	{"slc_onesc",			JKG_Slice_Script_OnEsc,			0		},
	// Inventory
	{"inv_button",			JKG_Inventory_SelectItem,		0		},
	{"inv_arrow_next",		JKG_Inventory_ArrowDown,		0		},
	{"inv_arrow_prev",		JKG_Inventory_ArrowUp,			0		},
	{"inv_use",				JKG_Inventory_Use,				0		},
	{"inv_destroy",			JKG_Inventory_Destroy,			0		},
	{"inv_acislot",			JKG_Inventory_ACISlot,			0		},
	{"inv_aciauto",			JKG_Inventory_ACISlotAuto,		0		},
	{"inv_aciremove",		JKG_Inventory_ACIRemove,		0		},
	{"inv_equip",			JKG_Inventory_EquipArmor,		0		},
	{"inv_unequip",			JKG_Inventory_UnequipArmor,		0		},
	{"inv_open_other",		JKG_Inventory_Open,				0		},

	// Shop
	{ "shop_arrow",			JKG_ShopArrow,					0		},
	{ "shop_left_button",	JKG_Shop_SelectLeft,			0		},
	{ "shop_right_button",	JKG_Shop_SelectRight,			0		},
	{ "shop_sort",			JKG_Shop_Sort,					0		},
	{ "shop_buy",			JKG_Shop_BuyItem,				0		},
	{ "shop_sell",			JKG_Shop_SellItem,				0		},
	{ "shop_buyammo",		JKG_Shop_BuyAmmo,				0		},
	{ "shop_buyrepair",		JKG_Shop_BuyRepair,				0		},
	{ "shop_close",			JKG_Shop_Closed,				0		},
	{ "shop_examine",		JKG_Shop_Examine,				0		},

	{0,						0,					    		0		},
};

// Server commands we should process
JKGkeywordHashSv_t JKGCmds[] = {
	{"conv",			Conv_ProcessCommand_f,		    0		},
	{"pzk",				JKG_ProcessPazaak_f,		    0		},
	{"pmr",				PartyMngt_ShowMessage_f,	    0		},
	{"slc",				JKG_Slice_ProcessCommand_f,		0		},
	{0,					0,							    0		},
};

JKGkeywordHashUI_t *JKGScriptHash[KEYWORDHASH_SIZE];
JKGkeywordHashSv_t *JKGCmdsHash[KEYWORDHASH_SIZE];

/*
===============
Item_SetupKeywordHash
===============
*/
void JKGScript_SetupKeywordHash(void) {
	int i;

	memset(JKGScriptHash, 0, sizeof(JKGScriptHash));
	for (i = 0; JKGScripts[i].keyword; i++) {
		JKGKeywordHashUI_Add(JKGScriptHash, &JKGScripts[i]);
	}
	memset(JKGCmdsHash, 0, sizeof(JKGCmdsHash));
	for (i = 0; JKGCmds[i].keyword; i++) {
		JKGKeywordHashSv_Add(JKGCmdsHash, &JKGCmds[i]);
	}
}

void UI_RunJKGScript(const char *scriptname, char **args) {
	JKGkeywordHashUI_t *script = JKGKeywordHashUI_Find(JKGScriptHash, ( char * ) scriptname);
	if (!script || !script->func) return;
	script->func (args);
}

qboolean UI_RunSvCommand(const char *command) {
	// Only called by cgame, dont use trap calls inside this or nested functions!
	JKGkeywordHashSv_t *cmd = JKGKeywordHashSv_Find(JKGCmdsHash, ( char * ) command);
	if (!cmd || !cmd->func) return qfalse;
	cmd->func();
	return qtrue;
}