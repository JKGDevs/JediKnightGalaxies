////////////////////////////////////////////////////////////
//
// Jedi Knight Galaxies
//
// Pazaak UI Module
//
// Written by BobaFett
//
////////////////////////////////////////////////////////////

//--futuza todo:
/*
1. Ask the player being challenged to pazaak to accept without freezing them up.  eg: "challenger->name has challenged you to a round of Pazaak, accept?  y/n" Server gives player 10 seconds, before assuming a no.
2. Typing in /y will accept the match and proceed as normal.
3. Typing in /n will decline the match
4. If challenged by more than one person within the 10 second grace period, system will respond to new challengers with, "Sorry this person has already been challenged by another opponent, ask them again later."
5. Add cvar for whether or not 2 player pazaak matches should be invulnerable or not?

Note: much of the implementation is also in pazaak.lua or misc.lua (pzktest)
*/

#include "ui_shared.h"
#include "ui_local.h"

// UI includes
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);
//

extern displayContextDef_t *DC;

static vec4_t cardColor = {1.0f, 1.0f, 1.0f, 0.8f};
static vec4_t cardColorStand = {0.6f, 0.6f, 0.6f, 0.8f};

static int HandSlotHover[4];
static int CardSelSlotHover[23];
static int CardSDHover[10];

// Button ID's
enum {
	PBUTTON_ENDTURN = 0,
	PBUTTON_STAND,
	PBUTTON_FORFEIT,
	PBUTTON_EXIT,
	PBUTTON_CONTINUE,
};

// Card ID's
enum {
	PZCARD_BACK = -2,
	PZCARD_NONE,
	PZCARD_NORMAL_1,
	PZCARD_NORMAL_2,
	PZCARD_NORMAL_3,
	PZCARD_NORMAL_4,
	PZCARD_NORMAL_5,
	PZCARD_NORMAL_6,
	PZCARD_NORMAL_7,
	PZCARD_NORMAL_8,
	PZCARD_NORMAL_9,
	PZCARD_NORMAL_10,
	PZCARD_PLUS_1,
	PZCARD_PLUS_2,
	PZCARD_PLUS_3,
	PZCARD_PLUS_4,
	PZCARD_PLUS_5,
	PZCARD_PLUS_6, 
	PZCARD_MINUS_1,
	PZCARD_MINUS_2,
	PZCARD_MINUS_3,
	PZCARD_MINUS_4,
	PZCARD_MINUS_5,
	PZCARD_MINUS_6,
	PZCARD_FLIP_1,
	PZCARD_FLIP_2,
	PZCARD_FLIP_3,
	PZCARD_FLIP_4,
	PZCARD_FLIP_5,
	PZCARD_FLIP_6,
	PZCARD_TIEBREAKER,
	PZCARD_DOUBLE,
	PZCARD_FLIP12,
	PZCARD_2N4,
	PZCARD_3N6,
};

int MenuFontToHandle(int iMenuFont);

/* Pazaak Dialog Defines and code */

enum {
	PDLG_OK,
	PDLG_YESNO,
};

enum {
	PDLGRESP_OK,
	PDLGRESP_YES,
	PDLGRESP_NO,
};

enum {
	PDLGID_ANY,
	PDLGID_WONSET,
	PDLGID_LOSTSET,
	PDLGID_TIESET,
	PDLGID_WONMATCH,
	PDLGID_LOSTMATCH,
	PDLGID_WINFORFEIT,
	PDLGID_LOSEFORFEIT,
	PDLGID_LOSEQUIT,
};


int activeBoard = 0;	// 1 = Pazaak, 2 = Card selection
typedef void (*PDlgCallback)(int response);

// Pazaak Dialog Struct
static struct {
	int InUse;
	char line1[256];
	char line2[256];
	int type; // PDLG_xxx
	PDlgCallback callback;
} PDlgData;

typedef struct {
	int cardid;
	int param;
} pzk_card;

typedef struct {
	int amount;		// Amount we have of this card
	int inuse;		// Amount of cards of this type we got in the side-deck
} pzk_deckcard;

// Internal state of the pazaak board
static struct {
	int active;	// Whether pazaak is active or not
	// Card selection info
	pzk_deckcard playercards[23];
	int			sidedeck[10];
	int			forfeitOnQuit;

	// Waiting for opponent text
	int			waitingStartTime;	// If it's 0, dont display it

	// Card information
	pzk_card	field[18];		// Cards in field slots, 0-8 = player 1, 9-17 = player 2
	pzk_card	hand[8];		// Cards in the hands
	// Board state
	int			turn;				// Whose turn it is (1 = mine, 2 = opponent, 0 = intermission)
	int			score_player;		// Score of the player
	int			score_opponent;		
	int			total_player;		// Points (card total) of player
	int			total_opponent;
	char		name_player[64];	// Name of the player
	char		name_opponent[64];	// Name of the opponent
	int			standing_player;	// Whether or not the player is standing
	int			standing_opponent;
	int			dlgid;				// Dialog ID being displayed currently (PDLGID_xxxx)
	int			timeout;			// Time when our timeout expires (in ms), 0 if disabled
	int			buttonstate;		// State of the buttons: 0 = disabled, 1 = Enabled, 2 = All but cards enabled
} PzkState;

static struct {
	qhandle_t	deckCard;
	qhandle_t	plusCard;
	qhandle_t	minusCard;
	qhandle_t	flipPlusCard;
	qhandle_t	flipMinusCard;
	qhandle_t	specialCard;
	qhandle_t	backSide;
} PzkShaders;

static struct {
	sfxHandle_t	drawCard;
	sfxHandle_t	playCard;
	sfxHandle_t	turnSwitch;
	sfxHandle_t	bust;
	sfxHandle_t	loseSet;
	sfxHandle_t	winSet;
	sfxHandle_t	tieSet;
	sfxHandle_t	loseMatch;
	sfxHandle_t	winMatch;
} PzkSounds;

static void Pazaak_CacheShaders() {
	// Will cache the card shaders, based on resolution
	memset(&PzkShaders, 0, sizeof(PzkShaders));
	if (DC) {
		if (DC->glconfig.vidWidth > 800) { // Bigger than 800x600? (assuming normal resolutions)
			// Use med-res
			PzkShaders.backSide = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/back_m.png");
			PzkShaders.deckCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/green_m.png");
			PzkShaders.flipMinusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/multi_neg_m.png");
			PzkShaders.flipPlusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/multi_pos_m.png");
			PzkShaders.minusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/red_m.png");
			PzkShaders.plusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/blue_m.png");
			PzkShaders.specialCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/yellow_m.png");
		} else {
			// Use low-res
			PzkShaders.backSide = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/back_l.png");
			PzkShaders.deckCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/green_l.png");
			PzkShaders.flipMinusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/multi_neg_l.png");
			PzkShaders.flipPlusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/multi_pos_l.png");
			PzkShaders.minusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/red_l.png");
			PzkShaders.plusCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/blue_l.png");
			PzkShaders.specialCard = trap->R_RegisterShaderNoMip("gfx/minigames/pazaak/cards/yellow_l.png");
		}
	}
}

static void Pazaak_CacheSounds() {
	memset(&PzkSounds, 0, sizeof(PzkSounds));
	PzkSounds.drawCard = trap->S_RegisterSound("sound/minigames/pazaak/draw.wav");
	PzkSounds.playCard = trap->S_RegisterSound("sound/minigames/pazaak/playcard.wav");
	PzkSounds.turnSwitch = trap->S_RegisterSound("sound/minigames/pazaak/turn.wav");
	PzkSounds.bust = trap->S_RegisterSound("sound/minigames/pazaak/bust.wav");
	PzkSounds.loseSet = trap->S_RegisterSound("sound/minigames/pazaak/loseset.wav");
	PzkSounds.winSet = trap->S_RegisterSound("sound/minigames/pazaak/winset.wav");
	PzkSounds.tieSet = trap->S_RegisterSound("sound/minigames/pazaak/tie.wav");
	PzkSounds.loseMatch = trap->S_RegisterSound("sound/minigames/pazaak/losematch.wav");
	PzkSounds.winMatch = trap->S_RegisterSound("sound/minigames/pazaak/winmatch.wav");

}

// Open up the dialog window
// Line1 and Line2 are the text to show on the respective lines (can be NULL)
// Type is either PDLG_OK or PDLG_YESNO, depending on the desired buttons
// The callback is called when the user confirms the dialog, the response (PDLGRESP_xx) will be sent as arg

static void Pazaak_Dialog_Show(const char *line1, const char *line2, int type, PDlgCallback callback) {
	menuDef_t *menu;
	if (!callback || !activeBoard) {
		return;
	}
	
	PDlgData.InUse = 1;

	if (line1) {
		Q_strncpyz(&PDlgData.line1[0], line1, 255);
	} else {
		PDlgData.line1[0] = 0;
	}

	if (line2) {
		Q_strncpyz(&PDlgData.line2[0], line2, 255);
	} else {
		PDlgData.line2[0] = 0;
	}

	PDlgData.type = type;
	PDlgData.callback = callback;

	if (activeBoard == 1) {
		menu = Menus_FindByName("jkg_pazaakholo");
	} else if (activeBoard == 2) {
		menu = Menus_FindByName("jkg_cardselholo");
	} else {
		return;
	}

	if (!menu) {
		return;
	}
	if (activeBoard == 1) {
		Menu_ShowItemByName(menu, "buttons", qtrue);
		Menu_ShowItemByName(menu, "buttons2", qtrue);
		Menu_ShowItemByName(menu, "buttons_hover", qfalse);
		Menu_ShowItemByName(menu, "buttons_hover2", qfalse);
	} else {
		Menu_ShowItemByName(menu, "buttons", qtrue);
		Menu_ShowItemByName(menu, "buttons_hover", qfalse);
	}
	Menu_ClearFocus(menu);

	Menu_ShowItemByName(menu, "dialog_txt", qtrue);
	if (type == PDLG_OK) {
		Menu_ShowItemByName(menu, "dialog_ok", qtrue);
	} else {
		Menu_ShowItemByName(menu, "dialog_yesno", qtrue);
	}
	
	if (activeBoard == 1) {
		// Disable the buttons
		Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);
		Menu_ShowItemByName(menu, "buttons_forfeit_hitzone", qfalse);
		Menu_ShowItemByName(menu, "buttons_hand", qfalse);
	} else {
		Menu_ShowItemByName(menu, "card_buttons", qfalse);
		Menu_ShowItemByName(menu, "sd_buttons", qfalse);
		Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);
	}
}

static void Pazaak_Dialog_Close() {
	menuDef_t *menu;
	int i;
	if (!activeBoard) {
		return;
	}

	PDlgData.InUse = 0;
	
	if (activeBoard == 1) {
		menu = Menus_FindByName("jkg_pazaakholo");
	} else if (activeBoard == 2) {
		menu = Menus_FindByName("jkg_cardselholo");
	} else {
		return;
	}
	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, "dialog_ok", qfalse);
	Menu_ShowItemByName(menu, "dialog_yesno", qfalse);
	Menu_ShowItemByName(menu, "dialog_txt", qfalse);
	Menu_ShowItemByName(menu, "dialog", qfalse);
	
	if (activeBoard == 1) {
		// Re-enable the buttons, based on the buttonstate
		Menu_ShowItemByName(menu, "buttons_forfeit_hitzone", qtrue);

		switch (PzkState.buttonstate) {
			case 0:	// Disable all
				Menu_ShowItemByName(menu, "buttons_hand", qfalse);
				Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);
				Menu_ShowItemByName(menu, "buttons_hover", qfalse);
				Menu_ShowItemByName(menu, "buttons", qtrue);
				for (i=0; i < 4; i++) {
					HandSlotHover[i] = 0;
				}
				break;
			case 1:	// Enable all
				for (i=0; i < 4; i++) {
					if (PzkState.hand[i].cardid != -1) {
						Menu_ShowItemByName(menu, va("button_hand%i", i+1), qtrue);
					}
				}
				Menu_ShowItemByName(menu, "buttons_hitzone", qtrue);
				break;
			case 2: // Enable all but buttons
				Menu_ShowItemByName(menu, "buttons_hand", qfalse);
				Menu_ShowItemByName(menu, "buttons_hitzone", qtrue);
				for (i=0; i < 4; i++) {
					HandSlotHover[i] = 0;
				}
				break;
		}
	} else {
		Menu_ShowItemByName(menu, "buttons_hitzone", qtrue);
		
		Menu_ShowItemByName(menu, "card_buttons", qfalse);		
		for (i=0; i<23; i++) {
			if (PzkState.playercards[i].amount > 0) {
				Menu_ShowItemByName(menu, va("card_B%i", i), qtrue);			
			}
		}

		Menu_ShowItemByName(menu, "sd_buttons", qfalse);
		for (i=0; i<10; i++) {
			if (PzkState.sidedeck[i] >= PZCARD_PLUS_1) {
				Menu_ShowItemByName(menu, va("sd_B%i", i), qtrue);			
			}
		}
	}
}

void Pazaak_Script_DialogButton(char **args) {
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	if (button < PDLGRESP_OK || button > PDLGRESP_NO) {
		// Invalid button
		return;
	}
	if (PDlgData.InUse) {
		(*PDlgData.callback)(button);
	}
}

void JKG_Pazaak_DrawDialog(int line, float x, float y, float w, float h) {
	float width;
	const char *text;
	vec4_t shadow;

	if (!PDlgData.InUse) {
		return;
	}
	if (line == 1) {
		text = PDlgData.line1;
	} else {
		text = PDlgData.line2;
	}

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5f;

	x = x + ((w / 2) - (width / 2));
	MAKERGBA(shadow,0,0,0,0.2f);
	trap->R_Font_DrawString(	x+1, y+1, text, shadow, MenuFontToHandle(1), -1, 0.5f);
	trap->R_Font_DrawString(	x, y, text, colorWhite, MenuFontToHandle(1), -1, 0.5f);
}

static void Pazaak_Close() {
	Menus_CloseByName("jkg_pazaakholo");
	Menus_CloseByName("jkg_cardselholo");
	activeBoard = 0;
}

static qhandle_t Pazaak_GetCardShader(int ID, int param) {
	switch (ID) {
		case PZCARD_BACK:
			// Backside
			return PzkShaders.backSide;
		case PZCARD_NONE:
			// None
			return (qhandle_t)NULL;
		case PZCARD_NORMAL_1:
		case PZCARD_NORMAL_2:
		case PZCARD_NORMAL_3:
		case PZCARD_NORMAL_4:
		case PZCARD_NORMAL_5:
		case PZCARD_NORMAL_6:
		case PZCARD_NORMAL_7:
		case PZCARD_NORMAL_8:
		case PZCARD_NORMAL_9:
		case PZCARD_NORMAL_10:
			// Deck card
			return PzkShaders.deckCard;
		case PZCARD_PLUS_1:
		case PZCARD_PLUS_2:
		case PZCARD_PLUS_3:
		case PZCARD_PLUS_4:
		case PZCARD_PLUS_5:
		case PZCARD_PLUS_6:
			// Plus card
			return PzkShaders.plusCard;
		case PZCARD_MINUS_1:
		case PZCARD_MINUS_2:
		case PZCARD_MINUS_3:
		case PZCARD_MINUS_4:
		case PZCARD_MINUS_5:
		case PZCARD_MINUS_6:
			// Minus card
			return PzkShaders.minusCard;
		case PZCARD_FLIP_1:
		case PZCARD_FLIP_2:
		case PZCARD_FLIP_3:
		case PZCARD_FLIP_4:
		case PZCARD_FLIP_5:
		case PZCARD_FLIP_6:
			// +/- flip card
			if (param == 1 || param == 3) {
				// Its flipped negative
				return PzkShaders.flipMinusCard;
			} else {
				// On the positive side
				return PzkShaders.flipPlusCard;
			}
		case PZCARD_TIEBREAKER:
		case PZCARD_DOUBLE:
		case PZCARD_FLIP12:
		case PZCARD_2N4:
		case PZCARD_3N6:
			// Golden special card
			return PzkShaders.specialCard;
		default:
			return (qhandle_t)NULL;
	}
}

// Get the value of a card in neutral state (on card selection screen)
static const char *Pazaak_GetNeutralCardValue(int ID) {
	switch (ID) {
		case PZCARD_BACK:
			// Backside
			return "";
		case PZCARD_NONE:
			// None
			return "";
		case PZCARD_NORMAL_1:
		case PZCARD_NORMAL_2:
		case PZCARD_NORMAL_3:
		case PZCARD_NORMAL_4:
		case PZCARD_NORMAL_5:
		case PZCARD_NORMAL_6:
		case PZCARD_NORMAL_7:
		case PZCARD_NORMAL_8:
		case PZCARD_NORMAL_9:
		case PZCARD_NORMAL_10:
			// Normal card
			return va("%i", (ID - PZCARD_NORMAL_1) + 1);
		case PZCARD_PLUS_1:
		case PZCARD_PLUS_2:
		case PZCARD_PLUS_3:
		case PZCARD_PLUS_4:
		case PZCARD_PLUS_5:
		case PZCARD_PLUS_6:
			return va("+%i", (ID - PZCARD_PLUS_1) + 1);
		case PZCARD_MINUS_1:
		case PZCARD_MINUS_2:
		case PZCARD_MINUS_3:
		case PZCARD_MINUS_4:
		case PZCARD_MINUS_5:
		case PZCARD_MINUS_6:
			return va("-%i", (ID - PZCARD_MINUS_1) + 1);
		case PZCARD_FLIP_1:
		case PZCARD_FLIP_2:
		case PZCARD_FLIP_3:
		case PZCARD_FLIP_4:
		case PZCARD_FLIP_5:
		case PZCARD_FLIP_6:
			return va("±%i", (ID - PZCARD_FLIP_1) + 1);
		case PZCARD_TIEBREAKER:
			return "±1T";
		case PZCARD_DOUBLE:
			return "D";
		case PZCARD_FLIP12:
			return "±1/2";
		case PZCARD_2N4:
			return "2&4";
		case PZCARD_3N6:
			return "3&6";
		default:
			return "";
	}
}

static const char *Pazaak_GetCardValue(int ID, int param) {
	switch (ID) {
		case PZCARD_BACK:
			// Backside
			return "";
		case PZCARD_NONE:
			// None
			return "";
		case PZCARD_NORMAL_1:
		case PZCARD_NORMAL_2:
		case PZCARD_NORMAL_3:
		case PZCARD_NORMAL_4:
		case PZCARD_NORMAL_5:
		case PZCARD_NORMAL_6:
		case PZCARD_NORMAL_7:
		case PZCARD_NORMAL_8:
		case PZCARD_NORMAL_9:
		case PZCARD_NORMAL_10:
			// Normal card
			if (param) {
				// Flipped by a 2&4 or 3&6 card
				return va("-%i", (ID - PZCARD_NORMAL_1) + 1);
			} else {
				return va("%i", (ID - PZCARD_NORMAL_1) + 1);
			}
		case PZCARD_PLUS_1:
		case PZCARD_PLUS_2:
		case PZCARD_PLUS_3:
		case PZCARD_PLUS_4:
		case PZCARD_PLUS_5:
		case PZCARD_PLUS_6:
			// Plus card
			if (param) {
				// Flipped by a 2&4 or 3&6 card
				return va("-%i", (ID - PZCARD_PLUS_1) + 1);
			} else {
				return va("+%i", (ID - PZCARD_PLUS_1) + 1);
			}
		case PZCARD_MINUS_1:
		case PZCARD_MINUS_2:
		case PZCARD_MINUS_3:
		case PZCARD_MINUS_4:
		case PZCARD_MINUS_5:
		case PZCARD_MINUS_6:
			// Minus card
			if (param) {
				// Flipped by a 2&4 or 3&6 card
				return va("+%i", (ID - PZCARD_MINUS_1) + 1);
			} else {
				return va("-%i", (ID - PZCARD_MINUS_1) + 1);
			}
		case PZCARD_FLIP_1:
		case PZCARD_FLIP_2:
		case PZCARD_FLIP_3:
		case PZCARD_FLIP_4:
		case PZCARD_FLIP_5:
		case PZCARD_FLIP_6:
			// +/- flip card
			if (param == 1 || param == 2) {
				// Its flipped negative
				return va("-%i", (ID - PZCARD_FLIP_1) + 1);
			} else {
				// On the positive side
				return va("+%i", (ID - PZCARD_FLIP_1) + 1);
			}
		case PZCARD_TIEBREAKER:
			if (param) {
				// Its flipped negative
				return "-1T";
			} else {
				// On the positive side
				return "+1T";
			}
		case PZCARD_DOUBLE:
			if (param) {
				// It's on the field, so use its param as value
				return va("%i", param);
			} else {
				// Unused, so show D
				return "D";
			}
		case PZCARD_FLIP12:
			if (param == 0) {
				return "+1";
			} else if (param == 1) {
				return "-1";
			} else if (param == 2) {
				return "+2";
			} else if (param == 3) {
				return "-2";
			} else {
				return "";
			}
			break;
		case PZCARD_2N4:
			if (param) {
				// On the field, show 0
				return "0";
			} else {
				// In hand
				return "2&4";
			}
		case PZCARD_3N6:
			if (param) {
				// On the field, show 0
				return "0";
			} else {
				// In hand
				return "3&6";
			}
		default:
			return "";
	}
}

static int Pazaak_CanCardFlip(int ID) {
	switch (ID) {
		case PZCARD_FLIP_1:
		case PZCARD_FLIP_2:
		case PZCARD_FLIP_3:
		case PZCARD_FLIP_4:
		case PZCARD_FLIP_5:
		case PZCARD_FLIP_6:
		case PZCARD_TIEBREAKER:
		case PZCARD_FLIP12:
			return 1;
		default:
			return 0;
	}
}

static int Pazaak_CanCardFlipValue(int ID) {
	switch (ID) {
		case PZCARD_FLIP12:
			return 1;
		default:
			return 0;
	}
}

static void Pazaak_FlipHandCard(int slot) {
	int param;
	if (!PzkState.active) {
		return;
	}
	if (slot < 1 || slot > 4) {
		return;
	}
	param = PzkState.hand[slot-1].param;
	switch(PzkState.hand[slot-1].cardid) {
		case PZCARD_FLIP_1:
		case PZCARD_FLIP_2:
		case PZCARD_FLIP_3:
		case PZCARD_FLIP_4:
		case PZCARD_FLIP_5:
		case PZCARD_FLIP_6:
		case PZCARD_TIEBREAKER:
			if (param) {
				param = 0;
			} else {
				param = 1;
			}
			break;
		case PZCARD_FLIP12:
			if (param & 1) {
				param &= ~1;
			} else {
				param |= 1;
			}
			break;
		default:
			break;
	}
	PzkState.hand[slot-1].param = param;
}

static void Pazaak_FlipHandCardValue(int slot) {
	int param;
	if (!PzkState.active) {
		return;
	}
	if (slot < 1 || slot > 4) {
		return;
	}
	param = PzkState.hand[slot-1].param;
	switch(PzkState.hand[slot-1].cardid) {
		case PZCARD_FLIP12:
			if (param & 2) {
				param &= ~2;
			} else {
				param |= 2;
			}
			break;
		default:
			break;
	}
	PzkState.hand[slot-1].param = param;
}

void JKG_Pazaak_DrawCardSlot(int slot, float x, float y, float w, float h) {
	float width;
	const char *text;
	qhandle_t sh;
	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}
	if (slot > 9) {
		// Opponent side
		if (PzkState.standing_opponent) {
			trap->R_SetColor(cardColorStand);
		} else {
			trap->R_SetColor(cardColor);
		}
	} else {
		// Our side
		if (PzkState.standing_player) {
			trap->R_SetColor(cardColorStand);
		} else {
			trap->R_SetColor(cardColor);
		}
	}
	sh = Pazaak_GetCardShader(PzkState.field[slot-1].cardid, PzkState.field[slot-1].param);
	if (!sh) {
		// No shader, so the slot is probably empty
		return;
	}
	trap->R_DrawStretchPic(x,y,w,h,0,0,1,1, sh);
	// Draw the number on it
	text = Pazaak_GetCardValue(PzkState.field[slot-1].cardid, PzkState.field[slot-1].param);

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;

	x = x + (w/4) + ((w / 4) - (width / 2));
	trap->R_Font_DrawString(	x, y + (h*0.25f), text, colorWhite, MenuFontToHandle(1), -1, 0.5);
}


void JKG_Pazaak_DrawSelCardSlot(int slot, float x, float y, float w, float h) {
	float width;
	const char *text;
	qhandle_t sh;
	float x2;
	vec4_t color;

	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}
	if (!PzkState.playercards[slot].amount) {
		// We dont have this card, dont draw it
		return;
	}
	
	VectorCopy4M(cardColor, color);
	if (CardSelSlotHover[slot]) {
		color[3] *= (0.8f + 0.2f * fabs(sin((float)trap->Milliseconds()/200.f)));
	}
	trap->R_SetColor(color);

	sh = Pazaak_GetCardShader(slot + PZCARD_PLUS_1, 0);
	if (!sh) {
		// No shader, so the slot is probably empty
		return;
	}
	trap->R_DrawStretchPic(x,y,w,h,0,0,1,1, sh);
	// Draw the number on it
	text = Pazaak_GetNeutralCardValue(slot + PZCARD_PLUS_1);

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;
	x2 = x + (w/4) + ((w / 4) - (width / 2));
	trap->R_Font_DrawString(	x2, y + (h*0.25f), text, colorWhite, MenuFontToHandle(1), -1, 0.5);

	text = va("%i", PzkState.playercards[slot].amount - PzkState.playercards[slot].inuse);

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;
	x2 = x + 7;
	trap->R_Font_DrawString(	x2, y-2, text, colorWhite, MenuFontToHandle(1) | 0x80000000, -1, 0.5);
}


void JKG_Pazaak_DrawSideDeckSlot(int slot, float x, float y, float w, float h) {
	float width;
	const char *text;
	qhandle_t sh;
	float x2;
	vec4_t color;
	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}

	VectorCopy4M(cardColor, color);
	if (CardSDHover[slot]) {
		color[3] *= (0.8f + 0.2f * fabs(sin((float)trap->Milliseconds()/200.f)));
	}
	trap->R_SetColor(color);

	sh = Pazaak_GetCardShader(PzkState.sidedeck[slot], 0);
	if (!sh) {
		// No shader, so the slot is probably empty
		return;
	}
	trap->R_DrawStretchPic(x,y,w,h,0,0,1,1, sh);
	// Draw the number on it
	text = Pazaak_GetNeutralCardValue(PzkState.sidedeck[slot]);

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;
	x2 = x + (w/4) + ((w / 4) - (width / 2));
	trap->R_Font_DrawString(	x2, y + (h*0.25f), text, colorWhite, MenuFontToHandle(1), -1, 0.5);
}


void JKG_Pazaak_DrawHandSlot(int slot, float x, float y, float w, float h) {
	float width;
	const char *text;
	vec4_t color;
	qhandle_t sh;
	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}
	
	VectorCopy4M(cardColor, color);
	if (slot < 5 && HandSlotHover[slot-1]) {
		color[3] *= (0.8f + 0.2f * fabs(sin((float)trap->Milliseconds()/200.f)));
	}
	trap->R_SetColor(color);
	sh = Pazaak_GetCardShader(PzkState.hand[slot-1].cardid, PzkState.hand[slot-1].param);
	if (!sh) {
		return;
	}

	trap->R_DrawStretchPic(x,y,w,h,0,0,1,1, sh);
	// Draw the number on it
	text = Pazaak_GetCardValue(PzkState.hand[slot-1].cardid, PzkState.hand[slot-1].param);

	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;

	x = x + (w/4) + ((w / 4) - (width / 2));
	trap->R_Font_DrawString(x, y + (h  *0.25f), text, colorWhite, MenuFontToHandle(1), -1, 0.5);

}

void JKG_Pazaak_DrawNames(int player, float x, float y, float w, float h) {
	float width;
	const char *text;
	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}

	if (player == 1) {
		text = PzkState.name_player;
	} else {
		text = PzkState.name_opponent;
	}
	if (player == 2) {
		width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;	
		x=x-width;
	}

	//draw a simple text shadow outline
	char cleanText[MAX_NETNAME];
	vec4_t textShadowColor = { 0.15, 0.15, 0.15, 1 };
	Q_strncpyz(cleanText, text, sizeof(cleanText));
	Q_StripColor(cleanText);
	trap->R_Font_DrawString(x-0.6, y-0.75, cleanText, textShadowColor, MenuFontToHandle(1), -1, 0.51); //draw 'shadow' slightly off center, with a larger scale and dark grey color
	trap->R_Font_DrawString(x, y, text, colorWhite, MenuFontToHandle(1), -1, 0.5);	//full color name
}

void JKG_Pazaak_DrawPoints(int player, float x, float y, float w, float h) {
	float width;
	const char *text;

	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}

	if (player == 1) {
		if (PzkState.standing_player) {
			text = va("( %i )", PzkState.total_player);
		} else {
			text = va("( ^5%i ^7)", PzkState.total_player);
		}
	} else {
		if (PzkState.standing_opponent) {
			text = va("( %i )", PzkState.total_opponent);
		} else {
			text = va("( ^5%i ^7)", PzkState.total_opponent);
		}
	}

	if (player == 1) {
		width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.5;
		x=x-width;
	}
	trap->R_Font_DrawString(x, y, text, colorWhite, MenuFontToHandle(1), -1, 0.5);
}

void JKG_Pazaak_DrawTimeout(float x, float y, float w, float h) {
	float width;
	const char *text;
	int tr;
	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}
	if (!PzkState.timeout) {
		// No timeout set
		return;
	}
	tr = ceil((float)(PzkState.timeout - trap->Milliseconds()) / 1000.0);
	if (tr < 0) {
		tr = 0;
	}
	if (tr <= 3) {
		text = va("^1%i", tr);
	} else {
		text = va("%i", tr);
	}
	
	width = (float)trap->R_Font_StrLenPixels(text, MenuFontToHandle(1), 1) * 0.6f;
	x = x + ((w / 2) - (width / 2));
	trap->R_Font_DrawString(	x, y , text, colorWhite, MenuFontToHandle(1), -1, 0.6f);
}

void JKG_Pazaak_DrawWaiting(float x, float y, float w, float h, qhandle_t shader) {
	vec4_t color;
	int deltatime;

	if (!PzkState.active) {
		// If we aint initialized, bail out
		return;
	}
	if (!PzkState.waitingStartTime) {
		// Don't show it yet
		return;
	}
	MAKERGBA(color,1,1,1,0);
	deltatime = PzkState.waitingStartTime - trap->Milliseconds();
	color[3] = fabs((sinf((float)deltatime / 400.0f)));
	trap->R_SetColor(color);
	trap->R_DrawStretchPic(x,y,w,h,0,0,1,1,shader);
}



void JKG_Pazaak_HandSlotHover(int slot, int hover) {
	HandSlotHover[slot-1] = hover;
}

void Pazaak_Script_HandSlotHover(char **args) {
	int slot;
	int hover;
	if (Int_Parse(args, &slot) && Int_Parse(args, &hover)) {
		JKG_Pazaak_HandSlotHover(slot, hover);
	}
}

void Pazaak_Script_UseCard(char **args) {
	int card;
	menuDef_t *menu;

	if (!Int_Parse(args, &card)) {
		return;
	}
	if (card < 1 || card > 4) {
		// Invalid card
		return;
	}
	if (PzkState.hand[card-1].cardid == -1) {
		return;
	}
	cgImports->SendClientCommand(va("~pzk usecard %i %i", card, PzkState.hand[card-1].param));
	PzkState.hand[card-1].cardid = -1;
	
	menu = Menus_FindByName("jkg_pazaakholo");
	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, va("button_hand%i", card), qfalse);
}

void Pazaak_Script_Flip(char **args) {
	int type;
	int card;
	if (!Int_Parse(args, &type) || !Int_Parse(args, &card)) {
		return;
	}
	if (card < 1 || card > 4 || type < 1 || type > 2) {
		// Invalid card and/or type
		return;
	}
	if (type == 1) {
		Pazaak_FlipHandCard(card);
	} else {
		Pazaak_FlipHandCardValue(card);
	}
}

void Pazaak_Script_CardHover(char **args) {
	int slot;
	int hover;
	if (!Int_Parse(args, &slot) || !Int_Parse(args, &hover)) {
		return;
	}
	if (slot < 0 || slot > 22) {
		// Invalid card
		return;
	}
	CardSelSlotHover[slot] = hover;
}
static void Pazaak_Dlg_Null(int response);

void Pazaak_Script_SelectCard(char **args) {
	int slot;
	int i;
	menuDef_t *menu;

	if (!Int_Parse(args, &slot)) {
		return;
	}
	for (i=0; i<10; i++) {
		if (PzkState.sidedeck[i] < PZCARD_PLUS_1) {
			break;
		}
	}
	if ( i == 10 ) {
		// No slots free
		PzkState.dlgid = 0;
		Pazaak_Dialog_Show("Your side deck is full!", "Please remove a card first.", PDLG_OK, Pazaak_Dlg_Null);
		return;
	}
	// We found a free slot
	if (!PzkState.playercards[slot].amount) {
		return;
	}
	if (PzkState.playercards[slot].amount == PzkState.playercards[slot].inuse) {
		return;
	}

	PzkState.sidedeck[i] = slot + PZCARD_PLUS_1;
	PzkState.playercards[slot].inuse += 1;

	
	menu = Menus_FindByName("jkg_cardselholo");
	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, va("sd_B%i", i), qtrue);
	CardSDHover[i] = 0;
}

void Pazaak_Script_SDHover(char **args) {
	int slot;
	int hover;
	if (!Int_Parse(args, &slot) || !Int_Parse(args, &hover)) {
		return;
	}
	if (slot < 0 || slot > 9) {
		// Invalid card
		return;
	}
	CardSDHover[slot] = hover;
}

void Pazaak_Script_RemoveSD(char **args) {
	int slot;
	int card;
	menuDef_t *menu;
	if (!Int_Parse(args, &slot)) {
		return;
	}
	// Check what was in the side deck slot, and remove the card
	card = PzkState.sidedeck[slot];
	if (card < PZCARD_PLUS_1) {
		// Slot is already empty, though lets make it -1 to be sure (NOTE: THIS SHOULD NEVER BE REACHED!)
		PzkState.sidedeck[slot] = -1;
		return;
	}
	card -= PZCARD_PLUS_1;
	PzkState.playercards[card].inuse -= 1;
	PzkState.sidedeck[slot] = -1;

	menu = Menus_FindByName("jkg_cardselholo");
	if (!menu) {
		return;
	}
	Menu_ShowItemByName(menu, va("sd_B%i", slot), qfalse);		
}

static void Pazaak_Dlg_Exit(int response) {
	Pazaak_Dialog_Close();
	if (response == PDLGRESP_YES) {
		cgImports->SendClientCommand("~pzk quit");
	}
}

static void Pazaak_Dlg_Forfeit(int response) {
	Pazaak_Dialog_Close();
	if (response == PDLGRESP_YES) {
		cgImports->SendClientCommand("~pzk forfeit");
	}
}

static void Pazaak_Dlg_Null(int response) {
	Pazaak_Dialog_Close();
	PzkState.dlgid = 0;
}

static void Pazaak_Dlg_Ok(int response) {
	Pazaak_Dialog_Close();
	PzkState.dlgid = 0;
	cgImports->SendClientCommand("~pzk acceptdlg");
}

void Pazaak_Script_OnEsc(char **args) {
	// The escape button was pressed, see if we're currently displaying a dialog
	if (PDlgData.InUse) {
		// Yes we are, check the type and process it
		if (PDlgData.type == PDLG_OK) {
			// Ok only dialog, so lets pretend the user clicked on Ok
			PDlgData.callback(PDLGRESP_OK);
		} else {
			// It's a yes/no dialog, so pretend the user clicked No
			PDlgData.callback(PDLGRESP_NO);
		}
	} else {
		// Dialog is not in use, bring up the forfeit dialog
		if (activeBoard == 1) {		
			PzkState.dlgid = PDLGID_ANY;
			Pazaak_Dialog_Show("If you forfeit you will lose your wager?", "Are you sure you wish to forfeit?", PDLG_YESNO, Pazaak_Dlg_Forfeit);
		} else {
			if (!PzkState.waitingStartTime) {
				PzkState.dlgid = PDLGID_ANY;
				if (PzkState.forfeitOnQuit) {
					Pazaak_Dialog_Show("If you quit now you will lose your wager", "Are you sure you wish to quit?", PDLG_YESNO, Pazaak_Dlg_Exit);
				} else {
					Pazaak_Dialog_Show("Are you sure you wish to quit?", NULL, PDLG_YESNO, Pazaak_Dlg_Exit);
				}
			}
		}
	}
}

static void Pazaak_ConfirmSideDeckDialog(int response) {
	menuDef_t *menu;

	Pazaak_Dialog_Close();
	PzkState.dlgid = 0;
	if (response == PDLGRESP_YES) {
		// Alrighty, disable the buttons and transmit our sidedeck
		menu = Menus_FindByName("jkg_cardselholo");
		if (!menu) {
			return;
		}
		Menu_ShowItemByName(menu, "buttons", qtrue);
		Menu_ShowItemByName(menu, "buttons_hover", qfalse);
		Menu_ShowItemByName(menu, "card_buttons", qfalse);
		Menu_ShowItemByName(menu, "sd_buttons", qfalse);
		Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);

		cgImports->SendClientCommand(va("~pzk setsd %i %i %i %i %i %i %i %i %i %i",
									PzkState.sidedeck[0], PzkState.sidedeck[1],
									PzkState.sidedeck[2], PzkState.sidedeck[3],
									PzkState.sidedeck[4], PzkState.sidedeck[5],
									PzkState.sidedeck[6], PzkState.sidedeck[7],
									PzkState.sidedeck[8], PzkState.sidedeck[9]));
		PzkState.waitingStartTime = trap->Milliseconds();
								
	}
}

static void Pazaak_ConfirmSideDeck() {
	// Check if our sidedeck is good, and if so, send it to the server, after asking for confirmation :)
	int i;
	for (i=0; i<10; i++) {
		if (PzkState.sidedeck[i] < PZCARD_PLUS_1) {
			break;
		}
	}
	if (i < 10) {
		// We dont have a filled sidedeck
		PzkState.dlgid = PDLGID_ANY;
		Pazaak_Dialog_Show("Your side-deck is incomplete.", "You need to pick 10 cards to play.", PDLG_OK, Pazaak_Dlg_Null);
		return;
	}
	// Confirm we wanna use this sidedeck and then get goin
	PzkState.dlgid = PDLGID_ANY;
	Pazaak_Dialog_Show("Do you wish to play with this side deck?", NULL, PDLG_YESNO, Pazaak_ConfirmSideDeckDialog);
}

void Pazaak_Script_ButtonPress(char **args) {
	int button;
	if (!Int_Parse(args, &button)) {
		return;
	}
	switch (button) {
		case PBUTTON_ENDTURN:
			cgImports->SendClientCommand("~pzk endturn");
			break;
		case PBUTTON_STAND:
			cgImports->SendClientCommand("~pzk stand");
			break;
		case PBUTTON_FORFEIT:
			PzkState.dlgid = PDLGID_ANY;
			Pazaak_Dialog_Show("If you forfeit you will lose your wager?", "Are you sure you wish to forfeit?", PDLG_YESNO, Pazaak_Dlg_Forfeit);
			break;
		case PBUTTON_EXIT:
			PzkState.dlgid = PDLGID_ANY;
			if (PzkState.forfeitOnQuit) {
				Pazaak_Dialog_Show("If you quit now you will lose your wager", "Are you sure you wish to quit?", PDLG_YESNO, Pazaak_Dlg_Exit);
			} else {
				Pazaak_Dialog_Show("Are you sure you wish to quit?", NULL, PDLG_YESNO, Pazaak_Dlg_Exit);
			}
			break;
		case PBUTTON_CONTINUE:
			Pazaak_ConfirmSideDeck();
			break;
		default:
			Com_Printf("Error: Invalid button pressed");
			break;
	}
}

void Pzk_InitParseBuff(parsebuff_t *pb) {
	memset(pb,0,sizeof(parsebuff_t));
	pb->arg = 1;
	pb->argc = trap->Cmd_Argc();
}

const char *Pzk_NextToken(parsebuff_t *pb) {
	if (pb->arg > pb->argc) return NULL;
	trap->Cmd_Argv(pb->arg++,pb->buff, sizeof(pb->buff));
	return pb->buff;
}

qboolean Pzk_TokensAvailable(parsebuff_t *pb) {
	if (pb->arg >= pb->argc) return qfalse;
	return qtrue;
}

int Pzk_ParseInt(parsebuff_t *pb, int *num) {
	const char *token;
	token = Pzk_NextToken(pb);
	if (!token) {
		Com_Printf("WARNING: ^3Error processing pazaak instruction: Could not parse int\n");
		return 1;
	}
	*num = atoi(token);

	return 0;
}

static void Pazaak_Reset() {
	int i;

	memset(&PzkState, 0, sizeof(PzkState));
	// Reset the field
	for (i=0; i < 18; i++) {
		PzkState.field[i].cardid = PZCARD_NONE;
	}
	// Reset the hand
	for (i=0; i < 8; i++) {
		PzkState.hand[i].cardid = PZCARD_NONE;
	}
	
	for (i=0; i < 4; i++) {
		HandSlotHover[i] = 0;
	}

	for (i=0; i < 23; i++) {
		PzkState.playercards[i].amount = 0;
		PzkState.playercards[i].inuse = 0;
	}

	for (i=0; i < 10; i++) {
		PzkState.sidedeck[i] = -1;
	}

	// Cache the card shaders and sounds
	Pazaak_CacheShaders();
	Pazaak_CacheSounds();
}

void JKG_ProcessPazaak_f() {
	// Big one, this processes ALL pazaak instructions
	// TODO: Audio cues!
	parsebuff_t pb;
	const char *token;
	int i;
	int temp;
	menuDef_t *menu;

	Pzk_InitParseBuff(&pb);
	
	while (1) {
		token = Pzk_NextToken(&pb);
		if (!token || !token[0]) break;

		if (!Q_stricmp(token,"start")) {
			Pazaak_Reset();
			// Engage pazaak mode
			PzkState.active = 1;
			activeBoard = 0;
			// This instruction should be followed by a gtg or gtc
			trap->Cvar_Set("ui_hidehud", "1");
			continue;
		}
		if (!PzkState.active) {
			return;
		}
		if (!Q_stricmp(token,"stop")) {
			Pazaak_Reset();
			Pazaak_Close();
			trap->Cvar_Set("ui_hidehud", "0");
			trap->Key_SetCatcher( trap->Key_GetCatcher() & ~KEYCATCH_UI );
			continue;
		}
		if (!Q_stricmp(token,"gtg")) {
			Menus_CloseAll();
			if (Menus_ActivateByName("jkg_pazaakholo"))
			{
				trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
			}
			Menu_ClearFocus(Menus_FindByName("jkg_pazaakholo"));
			activeBoard = 1;
			continue;
		}
		if (!Q_stricmp(token,"gtc")) {
			Menus_CloseAll();
			if (Menus_ActivateByName("jkg_cardselholo"))
			{
				trap->Key_SetCatcher( trap->Key_GetCatcher() | KEYCATCH_UI );
			}
			Menu_ClearFocus(Menus_FindByName("jkg_cardselholo"));
			activeBoard = 2;
			continue;
		}
		if (!Q_stricmp(token,"sn")) {
			// sn - Set player names
			token = Pzk_NextToken(&pb);
			if (!token) return;
			Q_strncpyz(PzkState.name_player, token, sizeof(PzkState.name_player));

			token = Pzk_NextToken(&pb);
			if (!token) return;
			Q_strncpyz(PzkState.name_opponent, token, sizeof(PzkState.name_opponent));
			continue;
		}
		if (!Q_stricmp(token,"st")) {
			// st - Set Turn
			if (Pzk_ParseInt(&pb, &PzkState.turn)) return;
			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}
			Menu_ShowItemByName(menu, "turns", qfalse);
			if (PzkState.turn == 1) {
				Menu_ShowItemByName(menu, "turn_p", qtrue);
			} else if (PzkState.turn == 2) {
				Menu_ShowItemByName(menu, "turn_o", qtrue);
			}
			trap->S_StartLocalSound(PzkSounds.turnSwitch, CHAN_AUTO);
			continue;
		}
		if (!Q_stricmp(token,"sto")) {
			// sto - Set Timeout
			if (Pzk_ParseInt(&pb, &temp)) return;
			if (!temp) {
				// Clear it
				PzkState.timeout = 0;
			} else {
				PzkState.timeout = trap->Milliseconds() + (temp*1000);
			}
			continue;
		}
		if (!Q_stricmp(token,"spt")) {
			// sto - Set Points total
			if (Pzk_ParseInt(&pb, &temp)) return;
			if (temp == 1) {
				// Our points
				if (Pzk_ParseInt(&pb, &PzkState.total_player)) return;
				if (PzkState.total_player > 20) {
					trap->S_StartLocalSound(PzkSounds.bust, CHAN_AUTO);
				}
			} else {
				if (Pzk_ParseInt(&pb, &PzkState.total_opponent)) return;
				if (PzkState.total_opponent > 20) {
					trap->S_StartLocalSound(PzkSounds.bust, CHAN_AUTO);
				}
			}
			continue;
		}
		if (!Q_stricmp(token,"cf")) {
			// cf - Clear Field (and other things too :P)
			// This is actually a combo command that basically
			// prepares the field for a new set.
			// It'll clear the field, as well as resetting points and clearning the turn indicator
			// Also lifts stand-state from both players

			for (i=0; i < 18; i++) {
				PzkState.field[i].cardid = PZCARD_NONE;
				PzkState.field[i].param = 0;
			}
			PzkState.turn = 0;
			PzkState.total_player = 0;
			PzkState.total_opponent = 0;
			PzkState.standing_player = 0;
			PzkState.standing_opponent = 0;

			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}
			Menu_ShowItemByName(menu, "turns", qfalse);
			continue;
		}

		if (!Q_stricmp(token,"ch")) {
			// ch - Clear Hand
			// Use this if you need to completely reset the field, which means clear the hands too
			// This will also clear the flip buttons and descriptions

			for (i=0; i < 8; i++) {
				PzkState.hand[i].cardid = PZCARD_NONE;
				PzkState.hand[i].param = 0;
			}

			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}
			Menu_ShowItemByName(menu, "buttons_hand", qfalse);
			Menu_ShowItemByName(menu, "flip", qfalse);
			continue;
		}

		if (!Q_stricmp(token,"sc")) {
			i=0;
			// sc - Set Card (field slot)
			if (Pzk_ParseInt(&pb, &temp)) return;
			temp--; // Lua uses 1-x indexes, C uses 0-x indexes
			// Before we begin, check if this is a new card
			if (PzkState.field[temp].cardid < 0) {
				// New card
				i = 1;
			}
			if (Pzk_ParseInt(&pb, &PzkState.field[temp].cardid)) return;
			if (Pzk_ParseInt(&pb, &PzkState.field[temp].param)) return;
			if (i) {
				// New card, so play an audio cue
				if (PzkState.field[temp].cardid >= PZCARD_NORMAL_1 && PzkState.field[temp].cardid <= PZCARD_NORMAL_10) {
					// A new deck card
					trap->S_StartLocalSound(PzkSounds.drawCard, CHAN_AUTO);
				} else if (PzkState.field[temp].cardid >= PZCARD_PLUS_1 && PzkState.field[temp].cardid <= PZCARD_2N4) {
					// A hand card!
					trap->S_StartLocalSound(PzkSounds.playCard, CHAN_AUTO);
				}
			}
			continue;
		}

		if (!Q_stricmp(token,"ss")) {
			// ss - Set Standing-state
			if (Pzk_ParseInt(&pb, &temp)) return;
			if (temp == 1) {
				if (Pzk_ParseInt(&pb, &PzkState.standing_player)) return;
			} else {
				if (Pzk_ParseInt(&pb, &PzkState.standing_opponent)) return;
			}
			continue;
		}
		if (!Q_stricmp(token,"ssc")) {
			// ssc - Set Score
			if (Pzk_ParseInt(&pb, &temp)) return;
			if (temp == 1) {
				if (Pzk_ParseInt(&pb, &PzkState.score_player)) return;
				// Update the board
				menu = Menus_FindByName("jkg_pazaakholo");
				if (!menu) {
					return;
				}
				Menu_ShowItemByName(menu, "score_p1", PzkState.score_player > 0);
				Menu_ShowItemByName(menu, "score_p2", PzkState.score_player > 1);
				Menu_ShowItemByName(menu, "score_p3", PzkState.score_player > 2);
			} else {
				if (Pzk_ParseInt(&pb, &PzkState.score_opponent)) return;
				// Update the board
				menu = Menus_FindByName("jkg_pazaakholo");
				if (!menu) {
					return;
				}
				Menu_ShowItemByName(menu, "score_o1", PzkState.score_opponent > 0);
				Menu_ShowItemByName(menu, "score_o2", PzkState.score_opponent > 1);
				Menu_ShowItemByName(menu, "score_o3", PzkState.score_opponent > 2);
			}
			continue;
		}

		if (!Q_stricmp(token,"shcs")) {
			// shcs - Set hand cards (networks 4 card ID's and params, and a bitvalue)
			for (i=0; i<4; i++) {
				if (Pzk_ParseInt(&pb, &PzkState.hand[i].cardid)) return;
				if (Pzk_ParseInt(&pb, &PzkState.hand[i].param)) return;
			}
			if (Pzk_ParseInt(&pb, &temp)) return;
			// Bitvalue here, if a bit is 1, it means he has the card, if not, he hasnt
			for (i=0; i<4; i++) {
				PzkState.hand[i+4].cardid = (temp & (1 << i)) ? PZCARD_BACK : PZCARD_NONE;
			}
			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}

			// Check for flip switches
			temp = 0;	// We'll store bitvalues here to see which descriptions to show
			Menu_ShowItemByName(menu, "flip", qfalse);

			for (i=0; i<4; i++) {
				if(Pazaak_CanCardFlip(PzkState.hand[i].cardid)) {
					// Card can be flipped
					temp |= 1;		// Display the sign flip description
					Menu_ShowItemByName(menu, va("flip_s%i", i+1), qtrue);
				}
				if(Pazaak_CanCardFlipValue(PzkState.hand[i].cardid)) {
					// Card can be flipped
					temp |= 2;		// Display the value flip description
					Menu_ShowItemByName(menu, va("flip_v%i", i+1), qtrue);
				}
			}
			// Determine if we need to show the flip switch description
			if (temp & 1) {
				Menu_ShowItemByName(menu, "flip_sd", qtrue);
			}
			if (temp & 2) {
				Menu_ShowItemByName(menu, "flip_vd", qtrue);
			}
			continue;
		}

		if (!Q_stricmp(token,"shc")) {
			// shc - Set hand card
			if (Pzk_ParseInt(&pb, &temp)) return;
			temp--;	// Lua uses 1-x indexes, C uses 0-x indexes
			if (Pzk_ParseInt(&pb, &PzkState.hand[temp].cardid)) return;
			if (Pzk_ParseInt(&pb, &PzkState.hand[temp].param)) return;
			
			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}
			
			if(Pazaak_CanCardFlip(PzkState.hand[temp].cardid)) {
				// Card can be flipped
				Menu_ShowItemByName(menu, va("flip_s%i", temp+1), qtrue);
				Menu_ShowItemByName(menu, "flip_sd", qtrue);
			} else {
				Menu_ShowItemByName(menu, va("flip_s%i", temp+1), qfalse);
			}
			if(Pazaak_CanCardFlipValue(PzkState.hand[temp].cardid)) {
				// Card can be flipped
				Menu_ShowItemByName(menu, va("flip_v%i", temp+1), qtrue);
				Menu_ShowItemByName(menu, "flip_vd", qtrue);
			} else {
				Menu_ShowItemByName(menu, va("flip_v%i", temp+1), qfalse);
			}
			continue;
		}
		if (!Q_stricmp(token,"db")) {
			// shc - Set hand card
			if (Pzk_ParseInt(&pb, &PzkState.buttonstate)) return;
			
			menu = Menus_FindByName("jkg_pazaakholo");
			if (!menu) {
				return;
			}

			switch (PzkState.buttonstate) {
				case 0:	// Disable all
					Menu_ShowItemByName(menu, "buttons_hand", qfalse);
					Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);
					Menu_ShowItemByName(menu, "buttons_hover", qfalse);
					Menu_ShowItemByName(menu, "buttons", qtrue);
					for (i=0; i < 4; i++) {
						HandSlotHover[i] = 0;
					}
					break;
				case 1:	// Enable all
					for (i=0; i < 4; i++) {
						if (PzkState.hand[i].cardid != -1) {
							Menu_ShowItemByName(menu, va("button_hand%i", i+1), qtrue);
						}
					}
					Menu_ShowItemByName(menu, "buttons_hitzone", qtrue);
					break;
				case 2: // Enable all but buttons
					Menu_ShowItemByName(menu, "buttons_hand", qfalse);
					Menu_ShowItemByName(menu, "buttons_hitzone", qtrue);
					for (i=0; i < 4; i++) {
						HandSlotHover[i] = 0;
					}
					break;
			}
			// Reset the focus so we can properly handle buttons that just got reactivated
			Menu_ClearFocus(menu);
			Menu_HandleMouseMove (menu, DC->cursorx, DC->cursory);
			continue;
		}

		if (!Q_stricmp(token,"sd")) {
			// sd - Show dialog
			if (Pzk_ParseInt(&pb, &PzkState.dlgid)) return;

			switch (PzkState.dlgid) {
				case PDLGID_WONSET:
					trap->S_StartLocalSound(PzkSounds.winSet, CHAN_AUTO);
					Pazaak_Dialog_Show("You won the set.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_LOSTSET:
					trap->S_StartLocalSound(PzkSounds.loseSet, CHAN_AUTO);
					Pazaak_Dialog_Show("The opponent wins the set.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_TIESET:
					trap->S_StartLocalSound(PzkSounds.tieSet, CHAN_AUTO);
					Pazaak_Dialog_Show("The set is tied.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_WONMATCH:
					trap->S_StartLocalSound(PzkSounds.winMatch, CHAN_AUTO);
					Pazaak_Dialog_Show("You have defeated your opponent.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_LOSTMATCH:
					trap->S_StartLocalSound(PzkSounds.loseMatch, CHAN_AUTO);
					Pazaak_Dialog_Show("You have been defeated.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_WINFORFEIT:
					trap->S_StartLocalSound(PzkSounds.winMatch, CHAN_AUTO);
					Pazaak_Dialog_Show("Your opponent has forfeited the match.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_LOSEFORFEIT:
					trap->S_StartLocalSound(PzkSounds.loseMatch, CHAN_AUTO);
					Pazaak_Dialog_Show("You have forfeited the match.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
				case PDLGID_LOSEQUIT:
					trap->S_StartLocalSound(PzkSounds.loseMatch, CHAN_AUTO);
					Pazaak_Dialog_Show("You have quit the match.", NULL, PDLG_OK, Pazaak_Dlg_Ok);
					break;
			}
			continue;
		}
		if (!Q_stricmp(token,"cd")) {
			// cd - Close dialog
			if (PzkState.dlgid != 0) {	// Only close if we got one open (that isnt forfeit)
				Pazaak_Dialog_Close();
			}
			continue;
		}

		if (!Q_stricmp(token,"sdc")) {
			// sdc - Set Deck Cards
			menu = Menus_FindByName("jkg_cardselholo");
			if (!menu) {
				return;
			}
			Menu_ShowItemByName(menu, "card_buttons", qfalse);			

			for (i=0; i<23; i++) {
				if (Pzk_ParseInt(&pb, &PzkState.playercards[i].amount)) return;
				if (PzkState.playercards[i].amount > 0) {
					Menu_ShowItemByName(menu, va("card_B%i", i), qtrue);			
				}
				PzkState.playercards[i].inuse = 0;
			}
			
			// Check how many we've used in our side deck
			for (i=0; i<10; i++) {
				if (PzkState.sidedeck[i] >= PZCARD_PLUS_1) {
					PzkState.playercards[PzkState.sidedeck[i] - PZCARD_PLUS_1].inuse += 1;
				}
			}
			continue;
		}

		if (!Q_stricmp(token,"ssd")) {
			// ssd - Set Side Deck
			menu = Menus_FindByName("jkg_cardselholo");
			if (!menu) {
				return;
			}

			Menu_ShowItemByName(menu, "sd_buttons", qfalse);

			for (i=0; i<10; i++) {
				if (Pzk_ParseInt(&pb, &PzkState.sidedeck[i])) return;
				if (PzkState.sidedeck[i] >= PZCARD_PLUS_1) {
					Menu_ShowItemByName(menu, va("sd_B%i", i), qtrue);			
				}
			}

			for (i=0; i<23; i++) {
				PzkState.playercards[i].inuse = 0;
			}
			
			// Check how many we've used in our side deck
			for (i=0; i<10; i++) {
				if (PzkState.sidedeck[i] >= PZCARD_PLUS_1) {
					PzkState.playercards[PzkState.sidedeck[i] - PZCARD_PLUS_1].inuse += 1;
				}
			}

			continue;
		}

		if (!Q_stricmp(token,"foq")) {
			// foq - Forfeit On Quit
			if (Pzk_ParseInt(&pb, &PzkState.forfeitOnQuit)) return;
			continue;
		}

		if (!Q_stricmp(token,"tsdn")) {
			// tsdn - transmit side deck now
			
			menu = Menus_FindByName("jkg_cardselholo");
			if (!menu) {
				return;
			}
			Menu_ShowItemByName(menu, "buttons", qtrue);
			Menu_ShowItemByName(menu, "buttons_hover", qfalse);
			Menu_ShowItemByName(menu, "card_buttons", qfalse);
			Menu_ShowItemByName(menu, "sd_buttons", qfalse);
			Menu_ShowItemByName(menu, "buttons_hitzone", qfalse);

			cgImports->SendClientCommand(va("~pzk setsdt %i %i %i %i %i %i %i %i %i %i",
										PzkState.sidedeck[0], PzkState.sidedeck[1],
										PzkState.sidedeck[2], PzkState.sidedeck[3],
										PzkState.sidedeck[4], PzkState.sidedeck[5],
										PzkState.sidedeck[6], PzkState.sidedeck[7],
										PzkState.sidedeck[8], PzkState.sidedeck[9]));
			PzkState.waitingStartTime = trap->Milliseconds();
			continue;
		}


		
	}
}
