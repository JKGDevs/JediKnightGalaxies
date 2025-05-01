////////////////////////////////////////////////////////////
//
// Jedi Knight Galaxies
//
// Slicing UI Module
//
// Written by BobaFett
//
////////////////////////////////////////////////////////////

#include "ui_shared.h"
#include "ui_local.h"

#include <encoding/base128.h>
#include <encoding/bitstream.h>

#ifdef _MSC_VER
#pragma warning (disable : 4090) // Different 'const' qualifiers
#endif

// UI includes
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);
void Menu_SetItemText(const menuDef_t *menu,const char *itemName, const char *text);
void Menu_ItemDisable(menuDef_t *menu, char *name,int disableFlag);
//

int MenuFontToHandle(int iMenuFont);

extern displayContextDef_t *DC;

// Game state structures
typedef struct {
	int active;
	int marked;
	int type;
	int revealTime;
	int blinkTime;
	int blinkColor;
} nodedata_t;


enum {
	PROGTYPE_NORMAL,		// Normal program
	PROGTYPE_NODE,			// Program requires node as argument
	PROGTYPE_INACTIVENODE,	// Program requires an inactive node as argument
	PROGTYPE_LINE,			// Program requires a line (row/column) as argument
};

typedef struct
{
	char ID[16];
	char name[32];
	char desc[32];
	int type;
} sliceprog_t;

typedef struct {
	int value;
	int alarms;
} summary_t;

enum {
	INPUTSTATE_NORMAL,			// Normal
	INPUTSTATE_AWAITINGNODE,	// Awaiting selection of a node
	INPUTSTATE_AWAITINGINACTIVENODE,	// Awaiting selection of an inactive node
	INPUTSTATE_AWAITINGLINE,	// Program requires a line (row/column) as argument
};


static struct
{
	qboolean	active;
	
	int			width;
	int			height;

	nodedata_t	grid[8][8];		// [row][col]

	qboolean	summariesKnown;
	summary_t	summaries[16]; // (0-7 = cols, 8-15 = rows)

	int			securityLevels;
	int			securityState[5];

	int			selectedProgram;
	int			programCount;
	sliceprog_t programs[16];

	int			inputState;		// 0 = Default, 1 = Program awaiting node, 2 = Program awaiting line

	int			warningLevel;
	int			warningThreshold;

	qboolean	intrusionDetection;
	int			intrusionTime;
	int			intrusionState;
	int			intrusionStart;

	qboolean	fieldLocked;

	qboolean	dlgActive;
	int			dlgType;		// DLGTYPE_*
	char		dlgText1[256];
	char		dlgText2[256];
	char		dlgText3[256];
	int			dlgid;			// DLGID_*
	
} sliceData;

enum {
	DLGID_NONE,
	DLGID_SERVER,
	DLGID_STOPSLICING,
};

enum {
	DLGTYPE_BLANK,		// No prompt (server will close this)
	DLGTYPE_YESNO,		// Yes/No
	DLGTYPE_OK,			// Ok only
};

enum {
	SLICECMD_EOM,
	SLICECMD_START,
	SLICECMD_STOP,
	SLICECMD_CONFIG,
	SLICECMD_REVEAL,
	SLICECMD_LOCK,
	SLICECMD_PROGLST,
	SLICECMD_SHOWMSG,
	SLICECMD_ENDMSG,
	SLICECMD_SUMMARY,
	SLICECMD_SECUPDATE,
	SLICECMD_INTRUSION,
	SLICECMD_WARNLEVEL,
	SLICECMD_BLINKNODE,
	SLICECMD_INITFIELD,
	SLICECMD_ALARM,
};


void JKG_Slice_ProgramListReset()
{
	menuDef_t *menu;
	itemDef_t *item;
	listBoxDef_t *listPtr;

	menu = Menus_FindByName("jkg_slice");
	if (!menu) return;

	item = Menu_FindItemByName(menu, "proglist");

	item->cursorPos = -1;
	listPtr = item->typeData.listbox;
	listPtr->cursorPos = -1;

	Menu_SetItemText(menu, "progdesc", "Please select a program");
}

void JKG_Slice_ProgramSetParameter(int param)
{
	menuDef_t *menu;
	itemDef_t *item;

	cgImports->SendClientCommand(va("~slc runprog %s %i", sliceData.programs[sliceData.selectedProgram].ID, param));
	sliceData.inputState = 0;

	menu = Menus_FindByName("jkg_slice");
	if (!menu) return;

	item = Menu_FindItemByName(menu, "proglist");
	item->disabled = qfalse;

	Menu_SetItemText(menu, "progdesc", sliceData.programs[sliceData.selectedProgram].desc);
	Menu_SetItemText(menu, "btn_runprogram", "Run program");
	Menu_ItemDisable(menu, "btn_stopslicing", 0);
}

static void JKG_Slice_Dialog_Show(const char *line1, const char *line2, const char *line3, int type, int dlgid) {
	menuDef_t *menu;
	
	sliceData.dlgActive = qtrue;

	if (line1) {
		Q_strncpyz(&sliceData.dlgText1[0], line1, 255);
	} else {
		sliceData.dlgText1[0] = 0;
	}

	if (line2) {
		Q_strncpyz(&sliceData.dlgText2[0], line2, 255);
	} else {
		sliceData.dlgText2[0] = 0;
	}

	if (line3) {
		Q_strncpyz(&sliceData.dlgText3[0], line3, 255);
	} else {
		sliceData.dlgText3[0] = 0;
	}

	sliceData.dlgType = type;
	sliceData.dlgid = dlgid;

	
	menu = Menus_FindByName("jkg_slice");

	if (!menu) {
		return;
	}

	// First, disable all controls on the interface
	Menu_ItemDisable(menu, "grid", 1);
	Menu_ItemDisable(menu, "prog", 1);
	Menu_ItemDisable(menu, "btns", 1);
	
	Menu_ClearFocus(menu);

	Menu_ShowItemByName(menu, "dialog", qtrue);
	if (type == DLGTYPE_OK) {
		Menu_ShowItemByName(menu, "btn_dialogok", qtrue);
	} else if (type == DLGTYPE_YESNO) {
		Menu_ShowItemByName(menu, "btn_dialogyesno", qtrue);
	}
}

static void JKG_Slice_Dialog_Close() {
	menuDef_t *menu;

	sliceData.dlgActive = 0;
	
	menu = Menus_FindByName("jkg_slice");

	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, "dialog", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogok", qfalse);
	Menu_ShowItemByName(menu, "btn_dialogyesno", qfalse);
	
	Menu_ItemDisable(menu, "grid", 0);
	Menu_ItemDisable(menu, "prog", 0);
	Menu_ItemDisable(menu, "btns", 0);

	Menu_ClearFocus(menu);
}

void JKG_Slice_Script_DialogButton(char **args) {
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	if (button < 0 || button > 1) {
		// Invalid button
		return;
	}
	if (sliceData.dlgActive) {
		if (sliceData.dlgid == DLGID_SERVER) {
			cgImports->SendClientCommand(va("~slc dlgresp %i", button));
		} else if (sliceData.dlgid == DLGID_STOPSLICING && button == 1) {
			cgImports->SendClientCommand("~slc stop");
		}
	}
	JKG_Slice_Dialog_Close();
}

void JKG_Slice_DrawDialog(int line, float x, float y, float w, float h) {
	float width;
	const char *text;

	if (!sliceData.dlgActive) {
		return;
	}
	if (line == 1) {
		text = sliceData.dlgText1;
	} else if (line == 2) {
		text = sliceData.dlgText2;
	} else {
		text = sliceData.dlgText3;
	}

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5f;

	x = x + ((w / 2) - (width / 2));
	trap->R_Font_DrawString(	x, y, text, colorWhite, MenuFontToHandle(1) | 0x80000000 , -1, 0.5f);
}

// Message Processor
void JKG_Slice_ProcessCommand_f(void)
{
	char arg[1024] = {0};
	char data[840];

	int len;

	int i, row, col;

	sfxHandle_t sfx;

	bitstream_t stream;

	trap->Cmd_Argv(1, arg, 1024);

	len = Base128_DecodeLength(strlen(arg));
	Base128_Decode(arg, strlen(arg), data, 840);

	

	BitStream_Init(&stream, (unsigned char *)data, len);
	BitStream_BeginReading(&stream);

	for (;;)
	{
		switch (BitStream_ReadBits(&stream, 4)) // Get the next instruction
		{
		case SLICECMD_EOM:
			// End of message
			return;
		case SLICECMD_START:
			// Reset data
			memset(&sliceData, 0, sizeof(sliceData));
			sliceData.active = qtrue;

			// Bring up UI
			trap->Cvar_Set("ui_hidehud", "1");
			Menus_CloseAll();
			if (Menus_ActivateByName("jkg_slice"))
			{
				trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
			}
			Menu_ClearFocus(Menus_FindByName("jkg_slice"));

			// Field is locked until all data is available
			sliceData.fieldLocked = qtrue;
			break;
		case SLICECMD_STOP:
			// End the slicing minigame
			sliceData.active = qfalse;

			Menus_CloseByName("jkg_slice");
			trap->Cvar_Set("ui_hidehud", "0");
			trap->Key_SetCatcher( trap->Key_GetCatcher() & ~KEYCATCH_UI );
			break;
		case SLICECMD_CONFIG:
			// Receive configuration
			sliceData.width = BitStream_ReadBits(&stream, 3) + 1;
			sliceData.height = BitStream_ReadBits(&stream, 3) + 1;
			sliceData.securityLevels = BitStream_ReadBits(&stream, 3);
			sliceData.warningThreshold = BitStream_ReadBits(&stream, 5);
			sliceData.intrusionDetection = BitStream_ReadBool(&stream);
			if (sliceData.intrusionDetection) {
				sliceData.intrusionTime = BitStream_ReadByte(&stream) * 10;
			} else {
				sliceData.intrusionTime = 0;
			}
			sliceData.intrusionStart = 0;
			break;
		case SLICECMD_REVEAL:		
			row = BitStream_ReadBits(&stream, 3);
			col = BitStream_ReadBits(&stream, 3);
			sliceData.grid[row][col].active = 1;
			sliceData.grid[row][col].revealTime = trap->Milliseconds();
			sliceData.grid[row][col].type = BitStream_ReadBits(&stream, 3);
			break;
		case SLICECMD_LOCK:
			if (BitStream_ReadBool(&stream)) {
				sliceData.fieldLocked = qtrue;
			} else {
				sliceData.fieldLocked = qfalse;
			}
			break;
		case SLICECMD_PROGLST:
			JKG_Slice_ProgramListReset();
			sliceData.selectedProgram = -1;
			sliceData.programCount = BitStream_ReadBits(&stream, 4);
			for (i = 0; i < sliceData.programCount; i++) 
			{
				Q_strncpyz(sliceData.programs[i].ID, BitStream_ReadStringBuffered(&stream), sizeof(sliceData.programs[i].ID));
				Q_strncpyz(sliceData.programs[i].name, BitStream_ReadStringBuffered(&stream), sizeof(sliceData.programs[i].name));
				Q_strncpyz(sliceData.programs[i].desc, BitStream_ReadStringBuffered(&stream), sizeof(sliceData.programs[i].desc));
				
				sliceData.programs[i].type = BitStream_ReadBits(&stream, 2);
			}
			break;
		case SLICECMD_SHOWMSG:
			{
				int mode = BitStream_ReadBits(&stream, 2);
				char buffer[3][256];
				BitStream_ReadString(&stream, buffer[0], 256);
				BitStream_ReadString(&stream, buffer[1], 256);
				BitStream_ReadString(&stream, buffer[2], 256);

				JKG_Slice_Dialog_Show(buffer[0], buffer[1], buffer[2], mode, DLGID_SERVER);
			}
			break;
		case SLICECMD_ENDMSG:
			JKG_Slice_Dialog_Close();
			break;
		case SLICECMD_SUMMARY:
			// Process column summaries
			for (i=0; i < sliceData.width; i++) {
				sliceData.summaries[i].value = BitStream_ReadBits(&stream, 6);
				sliceData.summaries[i].alarms = BitStream_ReadBits(&stream, 4);
			}

			// Process row summaries
			for (i=0; i < sliceData.height; i++) {
				sliceData.summaries[8+i].value = BitStream_ReadBits(&stream, 6);
				sliceData.summaries[8+i].alarms = BitStream_ReadBits(&stream, 4);
			}

			sliceData.summariesKnown = qtrue;
			break;
		case SLICECMD_SECUPDATE:
			for (i = 0; i < sliceData.securityLevels; i++) {
				sliceData.securityState[i] = BitStream_ReadBits(&stream, 2);
			}
			break;
		case SLICECMD_INTRUSION:
			// TODO: Play sound effect?
			sliceData.intrusionState = BitStream_ReadBits(&stream, 2);
			if (sliceData.intrusionState == 1) {
				sliceData.intrusionStart = trap->Milliseconds();
			} else {
				sliceData.intrusionStart = 0;
			}
			break;
		case SLICECMD_WARNLEVEL:
			sliceData.warningLevel = BitStream_ReadBits(&stream, 5);
			break;
		case SLICECMD_BLINKNODE:
			row = BitStream_ReadBits(&stream, 3);
			col = BitStream_ReadBits(&stream, 3);
			sliceData.grid[row][col].blinkTime = trap->Milliseconds();
			sliceData.grid[row][col].blinkColor = BitStream_ReadBool(&stream);
			break;
		case SLICECMD_INITFIELD:
			// Ready to play
			sliceData.fieldLocked = qfalse;
			break;
		case SLICECMD_ALARM:
			sfx = trap->S_RegisterSound("sound/effects/mpalarm.wav");
			trap->S_StartLocalSound(sfx, CHAN_AUTO);
			break;
		default:
			Com_Printf("Error processing slice command, unknown command ID\n");
			return;
		}
	}
}

// Color definitions
static const vec4_t black = {0, 0, 0, 1.0f};
static const vec4_t disabled = {0, 0, 0, 0.5f};
static const vec4_t white = {1, 1, 1, 1};
static const vec4_t offcolor = {0.1f, 0.1f, 0.6f, 1.0f};
//static const vec4_t green = {0.2f, 1.0f, 0.2f, 0.5f};
//static const vec4_t orange = {1.0f, 0.6f, 0.2f, 0.5f};
//static const vec4_t yellow = {1.0f, 1.0f, 0.2f, 0.5f};
//static const vec4_t red = {1.0f, 0.2f, 0.2f, 0.5f};

static const vec4_t green = {0.3f, 0.9f, 0.1f, 0.8f};
static const vec4_t orange = {1.0f, 0.5f, 0.15f, 0.8f};
static const vec4_t yellow = {1.0f, 0.95f, 0.0f, 0.8f};
static const vec4_t red = {0.93f, 0.1f, 0.15f, 0.8f};


static const vec4_t redfont = {1.0f, 0.25f, 0.25f, 1.0f};

void JKG_Slice_DrawGridSlot(int slot, float x, float y, float w, float h)
{
	vec4_t color;
	vec4_t color2;

	int row = slot >> 3;
	int col = slot & 7;

	const char *text = NULL;
	float w2;

	if (row >= sliceData.height || col >= sliceData.width) {
		UI_DrawRect(x, y, w, h, disabled);
	} else {
		if (sliceData.grid[row][col].active) {
			// Determine the color to show
			switch (sliceData.grid[row][col].type) {
				case 0:	// Alarm node
					VectorCopy4M(red, color2);
					text = NULL;
					break;
				case 1:	// Relay node
					VectorCopy4M(orange, color2);
					text = NULL;
					break;
				case 2: // Reset node
					VectorCopy4M(yellow, color2);
					text = "R";
					break;
				case 3: // Access level 1
				case 4:	// Access level 2
				case 5:	// Access level 3
				case 6:	// Access level 4
				case 7: // Access level 5
					VectorCopy4M(green, color2);
					text = va("%i", sliceData.grid[row][col].type - 2);
					break;
			}

			if (sliceData.grid[row][col].revealTime) {
				float phase = (float)(trap->Milliseconds() - sliceData.grid[row][col].revealTime) / 250.0f;
				if (phase > 1.0f) phase = 1.0f;

				if (trap->Milliseconds() > sliceData.grid[row][col].revealTime + 250) {
					sliceData.grid[row][col].revealTime = 0;
				}

				LerpColor((float *)white, color2, color, phase);
				UI_FillRect(x, y, w, h, color);
				
				if (text) {
					color[0] = color[1] = color[2] = 1.0f;
					color[3] = phase;
					w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
					DC->drawText(x + (w/2) - (w2/2), y+(h*0.2f), 0.4f, color, text, 0, 0, 0, 0 );
				}
			} else {
				UI_FillRect(x, y, w, h, color2);
				if (text) {
					w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
					DC->drawText(x + (w/2) - (w2/2), y+(h*0.2f), 0.4f, (float *)(white), text, 0, 0, 0, 0 );
				}
			}
		} else {
			if (sliceData.grid[row][col].blinkTime) {
				int delta = trap->Milliseconds() - sliceData.grid[row][col].blinkTime;
				float phase;
				if (delta > 2350) {	 // ~(7.5 * PI)*100, so the node blinks 4 times and stops when faded out
					sliceData.grid[row][col].blinkTime = 0;
				}

				phase = 0.5f + (sin((float)delta / 100.0f) * 0.5);

				if (sliceData.grid[row][col].blinkColor) {
					LerpColor((float *)offcolor, (float *)green, color, phase);
				} else {
					LerpColor((float *)offcolor, (float *)red, color, phase);
				}

				UI_FillRect(x, y, w, h, color);
			} else {
				if (sliceData.grid[row][col].marked) {
					if (sliceData.grid[row][col].marked == 1) {
						MAKERGBA(color, 0.3f, 0.3f, 1.0f, 0.7f + sin((float)trap->Milliseconds() / 150.0f) * 0.1f);
					} else {
						MAKERGBA(color, 1.0f, 0.3f, .3f, 0.7f + sin((float)trap->Milliseconds() / 150.0f) * 0.1f);
					}
					UI_FillRect(x, y, w, h, color);
				} else {
					UI_FillRect(x, y, w, h, offcolor);
				}
			}
		}

		if (sliceData.inputState == INPUTSTATE_AWAITINGNODE || (sliceData.inputState == INPUTSTATE_AWAITINGINACTIVENODE && !sliceData.grid[row][col].active)) {
			float phase = 0.7f + sin((float)trap->Milliseconds() / 150.0f) * 0.1f;
			MAKERGBA(color, phase, phase, phase, 1.0f);
			UI_DrawRect(x, y, w, h, color);
		} else {
			UI_DrawRect(x, y, w, h, black);
		}
	}
}

void JKG_Slice_DrawGridSummary(int slot, float x, float y, float w, float h) {
	static const vec4_t topcol = {.2, 1, .2, 0.2f};
	static const vec4_t botcol = {1, 0.2, 0.2, 0.2f};

	int orientation = (slot >> 3) & 1;		// 0 = Columns, 1 = Rows
	int index = slot & 7;

	vec4_t color;

	const char *text;

	float w2;

	if ((orientation == 0 && index >= sliceData.width) || (orientation == 1 && index >= sliceData.height) ) {
		UI_DrawRect(x, y, w, h, disabled);
		return;
	} else {
		UI_FillRect(x, y, w, h/2, topcol);
		UI_FillRect(x, y + (0.5f * h), w, h/2, botcol);
		if (sliceData.inputState == INPUTSTATE_AWAITINGLINE) {
			float phase = 0.7f + sin((float)trap->Milliseconds() / 150.0f) * 0.1f;
			MAKERGBA(color, phase, phase, phase, 1.0f);
			UI_DrawRect(x, y, w, h, color);
		} else {
			UI_DrawRect(x, y, w, h, black);
		}
	}


	if (sliceData.summariesKnown) {
		text = va("%i", sliceData.summaries[slot].value);
		w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
		DC->drawText(x + (w/2) - (w2/2), y-1, 0.4f, const_cast<float *>(white), text, 0, 0, 0, 0 );

		text = va("%i", sliceData.summaries[slot].alarms);
		w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
		DC->drawText(x + (w/2) - (w2/2), y+(h*0.5f)-1, 0.4f, const_cast<float *>(white), text, 0, 0, 0, 0 );
	} else {
		text = "?";
		w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
		DC->drawText(x + (w/2) - (w2/2), y-1, 0.4f, const_cast<float *>(white), text, 0, 0, 0, 0 );

		w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.4f;
		DC->drawText(x + (w/2) - (w2/2), y+(h*0.5f)-1, 0.4f, const_cast<float *>(white), text, 0, 0, 0, 0 );
	}

}

void JKG_Slice_DrawSecurityClearance(int slot, float x, float y, float w, float h) {

	const char *text;

	float w2;

	if (slot >= sliceData.securityLevels) {
		UI_DrawRect(x, y, w, h, disabled);
		return;
	} else {
		switch (sliceData.securityState[slot])
		{
			case 0:
				UI_FillRect(x, y, w, h, red);
				break;
			case 1:
				UI_FillRect(x, y, w, h, orange);
				break;
			case 2:
				UI_FillRect(x, y, w, h, yellow);
				break;
			case 3:
				UI_FillRect(x, y, w, h, green);
				break;
		}

		UI_DrawRect(x, y, w, h, black);
	}


	text = va("%i", slot + 1);
	
	w2 = trap->R_Font_StrLenPixels(text, MenuFontToHandle(0), 1.0f) * 0.5f;
	DC->drawText(x + (w/2) - (w2/2), y+(h*0.2f), 0.5f, const_cast<float *>(white), text, 0, 0, 0, 0 );

}

void JKG_Slice_DrawWarningLevel(float x, float y, float w, float h)
{
	const char *text = va("Warning level: %i / %i", sliceData.warningLevel, sliceData.warningThreshold);

	DC->drawText(x, y, 0.6f, const_cast<float *>(redfont), text, 0, 0, sliceData.warningLevel >= sliceData.warningThreshold ? ITEM_TEXTSTYLE_BLINK : 0, 1 );
}

void JKG_Slice_DrawIntrusion(int field, float x, float y, float w, float h)
{
	const char *text = NULL;
	if (!sliceData.intrusionDetection) {
		return;
	}

	if (field == 0) {
		// State
		switch (sliceData.intrusionState) {
			case 0:
				text = "Intrusion Detection Active";
				break;
			case 1:
				text = "Intrusion Detection Triggered";
				break;
			case 2:
				text = "Intrusion Detection Inactive";
				break;
			case 3:
				text = "Intrusion Detection Triggered";
				break;
		}

		DC->drawText(x, y, 0.5f, const_cast<float *>(redfont), text, 0, 0, 0, 1 );
	} else {
		// Time
		int mins;
		int secs;
		int time;

		if (!sliceData.intrusionStart) {
			// Time hasnt started yet
			time = sliceData.intrusionTime;
		} else {
			time = sliceData.intrusionTime - ((trap->Milliseconds() - sliceData.intrusionStart) / 1000);
		}
		
		if (time < 0) {
			time = 0;
		}

		mins = time / 60;
		secs = time - (mins * 60);
		
		if (sliceData.intrusionState == 0 || sliceData.intrusionState == 1) {
			text = va("%02i:%02i remaining until lockdown", mins, secs);
			DC->drawText(x, y, 0.4f, const_cast<float *>(redfont), text, 0, 0, time <= 10 ? ITEM_TEXTSTYLE_BLINK : 0, 1 );
		} else if (sliceData.intrusionState == 3) {
			text = "Lockdown activated!";
			DC->drawText(x, y, 0.4f, const_cast<float *>(redfont), text, 0, 0, ITEM_TEXTSTYLE_BLINK, 1 );
		}
	}
}

qboolean JKG_Slice_Grid_HandleKey(int slot, int flags, float *special, int key) {
	int row = slot >> 3;
	int col = slot & 7;
	
	if (key == A_MOUSE1) {
		if (sliceData.inputState == INPUTSTATE_NORMAL) {
			if (!sliceData.grid[row][col].active) {
				cgImports->SendClientCommand(va("~slc actnode %i", slot));
			}
		} else if (sliceData.inputState == INPUTSTATE_AWAITINGNODE  || (sliceData.inputState == INPUTSTATE_AWAITINGINACTIVENODE && !sliceData.grid[row][col].active)) {
			JKG_Slice_ProgramSetParameter(slot);
		}
	} else if (key == A_MOUSE2) {
		if (!sliceData.grid[row][col].active) {
			sliceData.grid[row][col].marked++;
			if (sliceData.grid[row][col].marked > 2) {
				sliceData.grid[row][col].marked = 0;
			}
		}
	}
	return qfalse;
}

qboolean JKG_Slice_Summary_HandleKey(int slot, int flags, float *special, int key) {
	
	if (key == A_MOUSE1) {
		if (sliceData.inputState == INPUTSTATE_AWAITINGLINE) {
			JKG_Slice_ProgramSetParameter(slot);
		}
	}
	return qfalse;
}

void JKG_Slice_Script_RunProgram(char **args) {
	menuDef_t *menu;

	menu = Menus_FindByName("jkg_slice");

	if (!menu) {
		// wtf?
		return;
	}
	
	if (sliceData.selectedProgram == -1) {
		return;
	}

	if (sliceData.inputState != INPUTSTATE_NORMAL) {
		// Abort program
		sliceData.inputState = INPUTSTATE_NORMAL;
		Menu_ItemDisable(menu, "btn_stopslicing", 0);
		Menu_SetItemText(menu, "btn_runprogram", "Run program");
		Menu_ItemDisable(menu, "proglist", 0);
		Menu_SetItemText(menu, "progdesc", sliceData.programs[sliceData.selectedProgram].desc);
		return;
	}

	switch (sliceData.programs[sliceData.selectedProgram].type)
	{
	case PROGTYPE_NORMAL:
		cgImports->SendClientCommand(va("~slc runprog %s", sliceData.programs[sliceData.selectedProgram].ID));
		break;
	case PROGTYPE_NODE:
		sliceData.inputState = INPUTSTATE_AWAITINGNODE;
		Menu_ItemDisable(menu, "btn_stopslicing", 1);
		Menu_SetItemText(menu, "btn_runprogram", "Abort program");
		Menu_ItemDisable(menu, "proglist", 1);
		Menu_SetItemText(menu, "progdesc", "Please choose a node to run this program on");
		break;
	case PROGTYPE_INACTIVENODE:
		sliceData.inputState = INPUTSTATE_AWAITINGINACTIVENODE;
		Menu_ItemDisable(menu, "btn_stopslicing", 1);
		Menu_SetItemText(menu, "btn_runprogram", "Abort program");
		Menu_ItemDisable(menu, "proglist", 1);
		Menu_SetItemText(menu, "progdesc", "Please choose an inactive node to run this program on");
		break;
	case PROGTYPE_LINE:
		sliceData.inputState = INPUTSTATE_AWAITINGLINE;
		Menu_ItemDisable(menu, "btn_stopslicing", 1);
		Menu_SetItemText(menu, "btn_runprogram", "Abort program");
		Menu_ItemDisable(menu, "proglist", 1);
		Menu_SetItemText(menu, "progdesc", "Please choose a row or column to run this program on");
		break;
	}
}

void JKG_Slice_Script_StopSlicing(char **args) {
	
	if (sliceData.inputState != INPUTSTATE_NORMAL) {
		return;
	}
	
	JKG_Slice_Dialog_Show("Are you sure you wish to stop slicing?", NULL, NULL, DLGTYPE_YESNO, DLGID_STOPSLICING);
}

void JKG_Slice_Script_OnEsc(char **args) {

	menuDef_t *menu;

	menu = Menus_FindByName("jkg_slice");

	if (!menu) {
		// wtf?
		return;
	}

	// The escape button was pressed, see if we're currently displaying a dialog
	if (sliceData.dlgActive) {
		// Yes we are, check the type and process it
		if (sliceData.dlgType != DLGTYPE_BLANK) {
			if (sliceData.dlgid == DLGID_SERVER) {
				cgImports->SendClientCommand("~slc dlgresp 0");
			}
			JKG_Slice_Dialog_Close();
		}
	} else {
		// No dialogs are up, see if we're trying to run a program
		if (sliceData.inputState != INPUTSTATE_NORMAL) {
			// Abort the program
			sliceData.inputState = INPUTSTATE_NORMAL;
			Menu_ItemDisable(menu, "btn_stopslicing", 0);
			Menu_SetItemText(menu, "btn_runprogram", "Run program");
			Menu_ItemDisable(menu, "proglist", 0);
			Menu_SetItemText(menu, "progdesc", sliceData.programs[sliceData.selectedProgram].desc);
		} else {
			// Bring up the stop slicing dialog
			JKG_Slice_Dialog_Show("Are you sure you wish to stop slicing?", NULL, NULL, DLGTYPE_YESNO, DLGID_STOPSLICING);
		}
	}
}



int JKG_Slice_ProgramCount() {
	return sliceData.programCount;
}

qboolean JKG_Slice_ProgramSelection(int index) {
	menuDef_t *menu;

	sliceData.selectedProgram = index;
	
	menu = Menus_FindByName("jkg_slice");

	if (menu) {
		Menu_SetItemText(menu, "progdesc", sliceData.programs[index].desc);
	}
	return qtrue;
}

const char *JKG_Slice_ProgramItemText(int index, int column, qhandle_t *handle1, qhandle_t *handle2, qhandle_t *handle3) {

	if (index < 0) {
		return "";
	}

	return sliceData.programs[index].name;
}
