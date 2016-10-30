/////////////////////////////////
//
// JKG Chatbox handling code
// 
// By BobaFett

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "jkg_chatcmds.h"
#include "jkg_chatbox.h"

extern displayContextDef_t cgDC;

// Chat modes (CHM)
enum {
	CHM_NORMAL,
	CHM_WHISPER,
	CHM_YELL,
	CHM_COMLINK,
	CHM_GLOBAL,
	CHM_PRIVATE,
	CHM_ACTION,
	CHM_TEAM,		// versus only --eez
	CHM_MAX,
};

struct {
	// Data for the chat contents
	char buff[145];	// 5 Bytes slack, to minimize the risk of an overflow
	int cursor;		// Location of the cursor
	int scroll;		// Scroll offset
	int len;		// Length of the text (to avoid having to use strlen a lot)

	// Defined at init or mode change
	int offset;		
	float maxwidth;

	// Overwrite mode
	qboolean overwrite;
} cb_data;

#define CHATBOX_MAXTEXT 140

static const char *chatModeText[] = { "Say^7: ", "Whisper^7: ", "Yell^7: ", "Comlink^7: ", "Global^7: ", "Private: ", "Action: ", "Team: "};

static int cb_privtarget = 0;		// Target for private chat (CHM_PRIVATE)
static int cb_chatmode = CHM_NORMAL;
static int cb_fadeTime = 0;
static int cb_fadeMode = 0; // 0 = off, 1 = fade in, 2 = on, 3 = fade out

extern void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars, qhandle_t textShader );

void ChatBox_InitChat() {
	cg.isChatting = 1;
	memset(&cb_data, 0, sizeof(cb_data));
	cb_fadeTime = trap->Milliseconds();
	cb_fadeMode = 1;
	trap->Key_SetCatcher(trap->Key_GetCatcher() | KEYCATCH_CGAME);
}

void ChatBox_CloseChat() {
	if (cg.isChatting) {
		cg.isChatting = 0;
		cb_fadeTime = trap->Milliseconds();
		cb_fadeMode = 3;
		trap->Key_SetCatcher(trap->Key_GetCatcher() & ~KEYCATCH_CGAME );
	}
}

void ChatBox_SetPaletteAlpha(float alpha) {
	// This modifies the alpha of the JA color palette
	// As well as the drop shadow color
	// Always reset this back to 1 after you're done!
	trap->JKG_SetColorTableAlpha(alpha);
	// Drop shadow color
	//*(float *)(0x5582B8 + 12) = alpha;		// FIXME: this address didn't look correct in the first place --eez
}

float Text_GetWidth(const char *text, int iFontIndex, float scale) {
	// Fixed algorithm to get text length accurately
	char s[2];
	const char *t = (char *)text;
	float w = 0;

	s[1] = 0;
	
	while (*t) {
		if (*t == '^') {
			if (*(t+1) >= '0' && *(t+1) <= '9') {
				t+=2;
				continue;
			}
			if (*(t+1) == 'x' || *(t+1) == 'X') {
				if (Text_IsExtColorCode(t+1)) {
					t+=5;
					continue;
				}
			}
		}
		s[0] = *t;
		w += ((float)trap->R_Font_StrLenPixels(s, iFontIndex, 1) * scale);
		t++;
	}
	return w;
}

static vec4_t tColorTable[10] = {
	{0, 0, 0, 1}, // ^0
	{1, 0, 0, 1}, // ^1
	{0, 1, 0, 1}, // ^2
	{1, 1, 0, 1}, // ^3
	{0, 0, 1, 1}, // ^4
	{0, 1, 1, 1}, // ^5
	{1, 0, 1, 1}, // ^6
	{1, 1, 1, 1}, // ^7
	{0, 0, 0, 1}, // ^8
	{1, 0, 0, 1}  // ^9
};

static float ExtColor_GetLevel(char chr) {
	if (chr >= '0' && chr <= '9') {
		return ( (float)(chr-'0') / 15.0f );
	}
	if (chr >= 'A' && chr <= 'F') {
		return ( (float)(chr-'A'+10) / 15.0f );
	}
	if (chr >= 'a' && chr <= 'f') {
		return ( (float)(chr-'a'+10) / 15.0f );
	}
	return -1;
}

static int Text_ExtColorCodes(const char *text, vec4_t color) {
	const char *r, *g, *b;
	float red, green, blue;
	r = text+1;
	g = text+2;
	b = text+3;
	// Get the color levels (if the numbers are invalid, it'll return -1, which we can use to validate)
	red = ExtColor_GetLevel(*r);
	green = ExtColor_GetLevel(*g);
	blue = ExtColor_GetLevel(*b);
	// Determine if all 3 are valid
	if (red == -1 || green == -1 || blue == -1) {
		return 0;
	}

	// We're clear to go, lets construct our color

	color[0] = red;
	color[1] = green;
	color[2] = blue;

	// HACK: Since cgame will use a palette override to implement dynamic opacity (like the chatbox)
	// we must ensure we use that alpha as well.
	// So copy the alpha of colorcode 0 (^0) instead of assuming 1.0

	//color[3] =*(float *)(0x56DF54 /*0x56DF48 + 12*/);
	color[3] = 1.0f;
	return 1;
}

// This function converts a text with extended color codes (^xRGB) into a text with normal color codes
// The extended colors will be clamped so the closest normal color available
// Used to display text with extended colorcodes in the console
const char *Text_ConvertExtToNormal(const char *text) {
	static char buff[2048];
	const char *r;		// Reader
	char *w;			// Writer
	char *cutoff;
	vec4_t color;
	int hicolors;
	int i;
	r = text;
	w = buff;
	cutoff = &buff[2046];
	while (*r) {
		if (w >= cutoff) {
			// Time to stop, we reached the limit
			*w = 0;
			return &buff[0];
		}
		if (*r == '^' && *(r + 1) == 'x' || *r == '^' && *(r + 1) == 'X') {
			if (Text_ExtColorCodes(r+1, color)) {
				// Extended colorcode alright, determine which base color is closest to this one
				*w = *r;	// write the ^
				w++;
				r+=5;

				// Determine how many of the R G and B components are over 50%
				hicolors = 0;
				for (i=0; i<3; i++) {
					if (color[i] >= 0.5f) {
						hicolors++;
					}
				}
				switch (hicolors) {
					case 0:
						// Color is black
						*w = '0';
						break;
					case 1:
						// It's a primary color, find out which
						if (color[0] >= 0.5f) {
							// It's red
							*w = '1';
						} else if (color[1] >= 0.5f) {
							// It's green
							*w = '2';
						} else {
							// Must be blue
							*w = '4';
						}
						break;
					case 2:
						// It's a secondary color, find out which
						if (color[0] >= 0.5f && color[1] >= 0.5f) {
							// It's yellow
							*w = '3';
						} else if (color[1] >= 0.5f && color[2] >= 0.5f) {
							// It's cyan
							*w = '5';
						} else {
							// Must be purple
							*w = '6';
						}
						break;
					case 3:
						// Color is white
						*w = '7';
						break;
					default:
						assert(0);	// Never happens, telling the compiler so
				}
				w++;
				continue;
			}
		}
		*w = *r;
		r++; w++;
	}
	*w = *r;	// Write the null terminator
	return &buff[0];
}

//todo: move all xrgb color related functions to ui_shared.cpp/h since all ui functions should be able to use it other than just chatbox
//void Text_DrawText(int x, int y, const char *text, const float* rgba, int iFontIndex, const int limit, float scale)	//now defined in ui_shared

void ChatBox_NewChatMode() {
    char *p = strchr (cb_data.buff, ' ');
    if ( !p )
    {
	    cb_data.buff[0] = 0;
	}
	else
	{
	    char temp[sizeof (cb_data.buff)] = { 0 };
	    Q_strncpyz (temp, p + 1, sizeof (temp));
	    Q_strncpyz (cb_data.buff, temp, sizeof (cb_data.buff));
	}
	cb_data.cursor = 0;
	cb_data.scroll = 0;
	cb_data.len = 0;
	cb_data.offset = Text_GetWidth(chatModeText[cb_chatmode], cgDC.Assets.qhSmall4Font, 0.6f);
	cb_data.maxwidth = 205 - cb_data.offset;
}


const char *ChatBox_PrintableText(int iFontIndex, float scale) {
	static char buff[256];
	char *u;
	const char *t;
	char s[2];
	float w = 0;

	t = (char *)&cb_data.buff[cb_data.scroll];
	u = &buff[0];

	s[1] = 0;

	while (*t) {
		if (*t == '^' && *(t + 1)) {
			if (*(t+1) >= '0' && *(t+1) <= '9') {
				*u = *t;
				u++; t++;
				*u = *t;
				u++; t++;
				continue;
			}
			if (*(t + 1) == 'x' && Text_IsExtColorCode(t + 1) || *(t + 1) == 'X' && Text_IsExtColorCode(t + 1)) {
				int i;
				for (i=0; i<5; i++) {
					*u = *t;
					u++; t++;
				}
				continue;
			}
		}
		s[0] = *t;

		w += ((float)trap->R_Font_StrLenPixels(s, iFontIndex, 1) * scale);
		if (w > cb_data.maxwidth) {
			break;
		}
		*u = *t;
		t++; u++;
	}
	*u = 0;
	return &buff[0];
}

void ChatBox_UpdateScroll() {
	float w = 0;		// Width scroll-cursor
	float wx = 0;		// Width scroll-end of string
	float wf = 0;		// Fraction w/max width
	float wxf = 0;		// Fraction wx/max width
	int i;
	char s[2];
	s[1] = 0;

	if (cb_data.cursor < cb_data.scroll) {
		// Odd condition
		for (i = cb_data.cursor; i> cb_data.scroll; i--) {
			s[0] = cb_data.buff[i];
			w += Text_GetWidth(s, cgDC.Assets.qhSmall4Font, 0.6f);
		}
		w *= -1; // Invert, since we gotta go back a lot :P
	} else {
		for (i = cb_data.scroll; i < cb_data.cursor; i++) {
			s[0] = cb_data.buff[i];
			w += Text_GetWidth(s, cgDC.Assets.qhSmall4Font, 0.6f);
		}
	}

	wf = w / cb_data.maxwidth;

	if (wf < 0.25f) {
		// Under 25%, try to scroll backward
		while (1) {
			if (cb_data.scroll == 0 || wf >= 0.25f) {
				// Cant go any further, or we're back on track
				return;
			}
			// Go back 1 character
			cb_data.scroll--;
			s[0] = cb_data.buff[cb_data.scroll];
			w += Text_GetWidth(s, cgDC.Assets.qhSmall4Font, 0.6f);
			wf = w / cb_data.maxwidth;
		}
	} else if (wf > 0.75f) {
		// Above 75%, try scrolling forward
		wx = Text_GetWidth(&cb_data.buff[cb_data.scroll], cgDC.Assets.qhSmall4Font, 0.6f);
		wxf	= wx / cb_data.maxwidth;
		while (1) {
			float wid;
			if (wf <= 0.75f || wxf < 1.0f) {
				// Cant go any further, or we're back on track
				return;
			}
			// Go forward 1 character
			cb_data.scroll++;
			s[0] = cb_data.buff[cb_data.scroll-1];
			wid = Text_GetWidth(s, cgDC.Assets.qhSmall4Font, 0.6f);
			w -= wid;
			wf = w / cb_data.maxwidth;

			wx -= wid;
			wxf	= wx / cb_data.maxwidth;
		}
	}
	// All done now
}

void ChatBox_CheckModes() {
	// To avoid too much processing, we do a systematic check before we
	// start with the string comparing
	if (cb_data.buff[0] == '/') {
		// possible command here :P
		if (cb_data.len >= 4) {
			// Check for 4 char commands
			if (!Q_stricmpn(cb_data.buff, "/me ", 4)) {
				cb_chatmode = CHM_ACTION;
				ChatBox_NewChatMode();
				return;
			}

		}
		else if (cb_data.len >= 3) {
			// Check for 3 char commands
			if (!Q_stricmpn(cb_data.buff, "/n ", 3)) {
				cb_chatmode = CHM_NORMAL;
				ChatBox_NewChatMode();
				return;
			}
			if (!Q_stricmpn(cb_data.buff, "// ", 3)) {
				cb_chatmode = CHM_GLOBAL;
				ChatBox_NewChatMode();
				return;
			}
			if (!Q_stricmpn(cb_data.buff, "/e ", 3)) {
				cb_chatmode = CHM_ACTION;
				ChatBox_NewChatMode();
				return;
			}
			if (!Q_stricmpn(cb_data.buff, "/y ", 3)) {
				cb_chatmode = CHM_YELL;
				ChatBox_NewChatMode();
				return;
			}
			if (!Q_stricmpn(cb_data.buff, "/w ", 3)) {
				cb_chatmode = CHM_WHISPER;
				ChatBox_NewChatMode();
				return;
			}
			if (!Q_stricmpn(cb_data.buff, "/t ", 3)) {
				cb_chatmode = CHM_TEAM;
				ChatBox_NewChatMode();
				return;
			}
		}

	}
}

// Escape % and ", so they can be sent along properly
static const char *ChatBox_EscapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = (char *)message;
	while (*t && s != cutoff) {
		if (*t == '%') {
			*s = 0x18;
		} else if (*t == '"') {
			*s = 0x17;
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}

void ChatBox_HandleKey(int key, qboolean down) {
	if (!down) {
		return;
	}
	
	if (key == A_ESCAPE) {
		ChatBox_CloseChat();
		return;
	}
	else if ( key == A_DELETE || key == A_KP_PERIOD ) {
		if ( cb_data.cursor < cb_data.len ) {
			memmove( cb_data.buff + cb_data.cursor, cb_data.buff + cb_data.cursor + 1, cb_data.len - cb_data.cursor);
			cb_data.len--;
			ChatBox_UpdateScroll();
		}
		return;
	}
	else if ( key == A_CURSOR_RIGHT || key == A_KP_6 ) 
	{
		if (cb_data.cursor < cb_data.len) {
			if (cb_data.buff[cb_data.cursor+1] == '^' && (cb_data.buff[cb_data.cursor+2] >= '0' && cb_data.buff[cb_data.cursor+2] <= '9')) {
				cb_data.cursor += 3;
			}
			else if (cb_data.buff[cb_data.cursor + 1] == '^' && ( (cb_data.buff[cb_data.cursor + 2] == 'x' && Text_IsExtColorCode(&cb_data.buff[cb_data.cursor + 2])) ||  (cb_data.buff[cb_data.cursor + 2] == 'X' && Text_IsExtColorCode(&cb_data.buff[cb_data.cursor + 2]))) ) {
				cb_data.cursor += 6;
			} else {
				cb_data.cursor++;
			}
			ChatBox_UpdateScroll();
		} 
		return;
	}
	else if ( key == A_CURSOR_LEFT || key == A_KP_4 ) 
	{
		if ( cb_data.cursor > 1 ) {
			if (cb_data.buff[cb_data.cursor-2] == '^' && (cb_data.buff[cb_data.cursor-1] >= '0' && cb_data.buff[cb_data.cursor-1] <= '9')) {
				// Jump over the color code
				cb_data.cursor -= 3;
			}
			else if (cb_data.cursor > 4 && cb_data.buff[cb_data.cursor - 5] == '^' && ( (cb_data.buff[cb_data.cursor - 4] == 'x' && Text_IsExtColorCode(&cb_data.buff[cb_data.cursor - 4]) || (cb_data.buff[cb_data.cursor - 4] == 'X' && Text_IsExtColorCode(&cb_data.buff[cb_data.cursor - 4]) )))) {
				// Jump over extended color code
				cb_data.cursor -= 6;
			} else {
				cb_data.cursor--;
			}
		} else if (cb_data.cursor > 0) {
			cb_data.cursor--;
		}
		ChatBox_UpdateScroll();
		return;
	}
	else if ( key == A_HOME || key == A_KP_7) {// || ( tolower(key) == 'a' && trap->Key_IsDown( K_CTRL ) ) ) {
		cb_data.cursor = 0;
		cb_data.scroll = 0;
		return;
	}
	else if ( key == A_END || key == A_KP_1)  {// ( tolower(key) == 'e' && trap->Key_IsDown( K_CTRL ) ) ) {
		cb_data.cursor = cb_data.len;
		ChatBox_UpdateScroll();
		return;
	}
	else if ( key == A_INSERT || key == A_KP_0 ) {
		cb_data.overwrite = !cb_data.overwrite;
		return;
	}
	else if ( key == A_ENTER || key == A_KP_ENTER)  {
		// Send our message ^^
		if (cb_data.buff[0]) {
			// Check if it's a command
			if (cb_data.buff[0] == '/' || cb_data.buff[0] == '\\') {
				if (CCmd_Execute(&cb_data.buff[1])) {
					ChatBox_CloseChat();
					return;		// It was a client-side chat command
				}
			}
			switch (cb_chatmode) {
				case CHM_NORMAL:
					trap->SendClientCommand(va("say \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_ACTION:
					trap->SendClientCommand(va("sayact \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_GLOBAL:
					trap->SendClientCommand(va("sayglobal \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_YELL:
					trap->SendClientCommand(va("sayyell \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_WHISPER:
					trap->SendClientCommand(va("saywhisper \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_PRIVATE:
					trap->SendClientCommand(va("tell %i \"%s\"\n", cb_privtarget, ChatBox_EscapeChat(cb_data.buff)));
					break;
				case CHM_TEAM:
					trap->SendClientCommand(va("say_team \"%s\"\n", ChatBox_EscapeChat(cb_data.buff)));
					break;
				default:
					break;
			}
		}
		ChatBox_CloseChat();
		return;
	}
	else if( key & K_CHAR_FLAG )
	{
		key &= ~K_CHAR_FLAG;

		// Failsafe, should never happen, but just in case
		if (cb_data.cursor > cb_data.len) 
			cb_data.cursor = cb_data.len;

		if (key == 'h' - 'a' + 1 )	{	// ctrl-h is backspace (so is char 8 >.>)
			if ( cb_data.cursor > 0 ) {
				memmove( &cb_data.buff[cb_data.cursor - 1], &cb_data.buff[cb_data.cursor], cb_data.len + 1 - cb_data.cursor);
				cb_data.cursor--;
				ChatBox_UpdateScroll();
				cb_data.len--;
			}
			return;
		}

		// ignore any non printable chars
		if ( key < 32) {
		    return;
	    }

		if (!cb_data.overwrite) {
			if ( cb_data.len == CHATBOX_MAXTEXT ) {
				// Filled up, ignore further input
				return;
			}
			if (cb_data.len != cb_data.cursor)
				memmove( &cb_data.buff[cb_data.cursor + 1], &cb_data.buff[cb_data.cursor], cb_data.len + 1 - cb_data.cursor );
		} else {
			if ( cb_data.len == CHATBOX_MAXTEXT  && cb_data.cursor == cb_data.len) {
				// Filled up, ignore further input
				return;
			}
		}

		cb_data.buff[cb_data.cursor] = key;

		// Null-terminate the input!
		if (cb_data.cursor == cb_data.len) {
			cb_data.len++;	// We added a char, so raise the length
			if (cb_data.cursor+1 < 2048)
			{
				cb_data.buff[cb_data.cursor+1] = 0;
			}
			else
			{
				cb_data.buff[cb_data.cursor] = 0;
			}
		} else if (!cb_data.overwrite) {
			// We inserted a char
			cb_data.len++;
		}

		if (cb_data.cursor <= cb_data.len ) {
			cb_data.cursor++;
			ChatBox_UpdateScroll();
		}
		// We typed somethin, check if we got something special here
		ChatBox_CheckModes();
	}

}

qboolean ChatBox_CanUseChat() {

	if (!cg.snap)		// HOTFIX
		return qfalse;

	if (cg.snap->ps.stats[STAT_HEALTH] < 1)
		return qfalse;
	if (cg.deathcamTime)
		return qfalse;
	if ( cg.snap->ps.pm_type == PM_INTERMISSION || cg.snap->ps.pm_type == PM_SPECTATOR )
		return qfalse;
	return qtrue;
}

void ChatBox_MessageMode() {
	// Since this gets called from the engine, the VM info might be incorrect
	// So override it so cgame is active, then restore later on	
	//int ActiveVM = *(int *)0x12A4F18;
	if (!ChatBox_CanUseChat()) {
		return;
	}

	//*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
	ChatBox_InitChat();

	// VERSUS ONLY --eez
	cb_chatmode = CHM_GLOBAL;
	ChatBox_NewChatMode();

	// Restore the active VM
	//*(int *)0x12A4F18 = ActiveVM;
}

void ChatBox_MessageMode2() {
	// Since this gets called from the engine, the VM info might be incorrect
	// So override it so cgame is active, then restore later on	
	//int ActiveVM = *(int *)0x12A4F18;
	if (!ChatBox_CanUseChat()) {
		return;
	}

	//*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
	ChatBox_InitChat();
	cb_chatmode = CHM_TEAM;
	ChatBox_NewChatMode();

	// Restore the active VM
	//*(int *)0x12A4F18 = ActiveVM;
}

void ChatBox_MessageMode3() {	// Private chat (aim trace)
	// Since this gets called from the engine, the VM info might be incorrect
	// So override it so cgame is active, then restore later on	
	//int ActiveVM = *(int *)0x12A4F18;
	int target;

	if (!ChatBox_CanUseChat()) {
		return;
	}
	//*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
	target = CG_CrosshairPlayer();
	if (target >= 0 && target < MAX_CLIENTS) {
		ChatBox_InitChat();
		cb_chatmode = CHM_PRIVATE;
		cb_privtarget = target;
		ChatBox_NewChatMode();	
	}
	
	// Restore the active VM
	//*(int *)0x12A4F18 = ActiveVM;
}

void ChatBox_MessageMode4() {
	// Since this gets called from the engine, the VM info might be incorrect
	// So override it so cgame is active, then restore later on	
	//int ActiveVM = *(int *)0x12A4F18;
	if (!ChatBox_CanUseChat()) {
		return;
	}

	// VERSUS ONLY --eez

	//*(int *)0x12A4F18 = *(int *)0x8AF0FC; // CurrentVm = cgvm;
	ChatBox_InitChat();
	cb_chatmode = CHM_ACTION;
	ChatBox_NewChatMode();

	// Restore the active VM
	//*(int *)0x12A4F18 = ActiveVM;
}

void ChatBox_UseMessageMode(int whichOne)
{
	switch(whichOne)
	{
		case 1:
			ChatBox_MessageMode();
			break;
		case 2:
			ChatBox_MessageMode2();
			break;
		case 3:
			ChatBox_MessageMode3();
			break;
		case 4:
			ChatBox_MessageMode4();
			break;
	}
}

void ChatBox_InitSystem() {
	// Relink the messagemode<x> commands to our custom functions
	/*Cmd_EditCommand("messagemode", ChatBox_MessageMode);
	Cmd_EditCommand("messagemode2", ChatBox_MessageMode2);
	Cmd_EditCommand("messagemode3", ChatBox_MessageMode3);
	Cmd_EditCommand("messagemode4", ChatBox_MessageMode4);*/
}

void ChatBox_ShutdownSystem() {
	// Relink the messagemode<x> commands to their engine functions
	/*Cmd_EditCommand("messagemode", (void *)0x417040);
	Cmd_EditCommand("messagemode2", (void *)0x417080);
	Cmd_EditCommand("messagemode3", (void *)0x4170C0);
	Cmd_EditCommand("messagemode4", (void *)0x417130);*/
}

void ChatBox_DrawBackdrop(menuDef_t *menu) {
	itemDef_t *item = NULL;
	vec4_t color;
	float phase = 0;
	int time = trap->Milliseconds();
	if (!cb_fadeMode) {
		return;
	} else if (cb_fadeMode == 2) {
		phase = 1;
	} else if (cb_fadeMode == 1) {
		// Fading in
		if (cb_fadeTime + 250 < time) {
			cb_fadeMode = 2;
			phase = 1;
		} else {
			// Still fading
			phase = (float)(time - cb_fadeTime) / 250.0f;
		}
	} else if (cb_fadeMode == 3) {
		// Fading out
		if (cb_fadeTime + 250 < time) {
			cb_fadeMode = 0;
			phase = 0;
		} else {
			// Still fading
			phase = 1 - ((float)(time - cb_fadeTime) / 250.0f);
		}
	}
	
#pragma region h_chat
	item = Menu_FindItemByName(menu, "h_chat");
	if (item) {
		VectorCopy4(item->window.foreColor, color);
		color[3] *= phase;
		color[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( color );
		trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
	}
#pragma endregion
}

void ChatBox_DrawChat(menuDef_t *menu) {
	itemDef_t *item;
	const char *text;
	unsigned int offset;
	int cursorpos;
	vec4_t	newColor;

	MAKERGBA(newColor, colorCyan[0], colorCyan[1], colorCyan[2], colorCyan[3]*cg.jkg_HUDOpacity);

	if (!cg.isChatting) {
		return;
	}
#pragma region t_chat
	item = Menu_FindItemByName(menu, "t_chat");
	if (item) {
		/*trap->R_Font_DrawString(
				item->window.rect.x,
				item->window.rect.y,
				chatModeText[cb_chatmode],
				newColor,
				cgDC.Assets.qhSmall4Font,
				-1,
				0.6f);*/
		CG_DrawStringExt(
			item->window.rect.x,
			item->window.rect.y + 12,
			chatModeText[cb_chatmode],
			newColor,
			qtrue, qfalse,
			5, 8,
			strlen(chatModeText[cb_chatmode]),
			cgs.media.charset_Arial);
		text = ChatBox_PrintableText(cgDC.Assets.qhSmall4Font, 0.6f);
		offset = cb_data.cursor - cb_data.scroll;

		MAKERGBA(newColor, colorWhite[0], colorWhite[1], colorWhite[2], colorWhite[3]*cg.jkg_HUDOpacity);
		
		// We use Text_DrawText here to ensure correct spacing
		/*Text_DrawText(
				item->window.rect.x + cb_data.offset,
				item->window.rect.y,
				text,
				newColor,
				cgDC.Assets.qhSmall4Font,
				-1,
				0.6f);*/
		CG_DrawStringExt(
			item->window.rect.x + cb_data.offset,
			item->window.rect.y + 12,
			text,
			newColor,
			qfalse, qfalse,
			4, 8,
			strlen(text),
			cgs.media.charset_Arial);

		if (cg.time >> 8 & 1) {
			// Draw the cursor
			if (offset > strlen(text)) {
				// Shouldn't happen.. but still, just in case
				//cursorpos = Text_GetWidth(text, cgDC.Assets.qhSmall4Font, 0.6f);
				cursorpos = 0;
			} else {
				((char *)text)[offset] = 0;
				//cursorpos = Text_GetWidth(text, cgDC.Assets.qhSmall4Font, 0.6f);
				cursorpos = (int)strlen(text) * 3;
			}
			/*Text_DrawText(
					item->window.rect.x + cb_data.offset + cursorpos,
					item->window.rect.y,
					cb_data.overwrite ? "_" : "|" ,
					newColor,
					cgDC.Assets.qhSmall4Font,
					-1,
					0.6f);*/
			CG_DrawStringExt(
				item->window.rect.x + cb_data.offset + cursorpos,
				item->window.rect.y + 12,
				cb_data.overwrite ? "_" : "|" ,
				newColor,
				qtrue, qfalse,
				4, 8,
				2,
				cgs.media.charset_Arial);
		}
	}
#pragma endregion
}

void ChatBox_InterruptChat() {
	if (cg.isChatting) {
		// Instead of just going away, we still send the current message :P
		if (cb_data.buff[0] == '/' || cb_data.buff[0] == '\\') {
			// Never send commands on death, as this can have unwanted consequences
			cg.isChatting = 0;
			cb_fadeTime = trap->Milliseconds();
			cb_fadeMode = 3;
			trap->Key_SetCatcher(0);
			return;
		}
		
		// Don't send empty message
		if ( cb_data.buff[0] == '\0' )
		{
		    return;
		}
		strncat(cb_data.buff, "-", sizeof(cb_data.buff)-2);
		ChatBox_HandleKey(A_ENTER, qtrue);
		// Now, this here is supposed to close the chatbox
		// If it does not however, we'll have this as a fallback
		if (cg.isChatting) {
			cg.isChatting = 0;
			cb_fadeTime = trap->Milliseconds();
			cb_fadeMode = 3;
			trap->Key_SetCatcher(0);
		}
	}
}