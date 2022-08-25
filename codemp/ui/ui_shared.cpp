/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// 
// string allocation/managment

#if defined(IN_UI)
	#include "ui/ui_local.h"
	#include "ui_jkgscript.h"
	#include "jkg_inventory.h"
#elif defined(_CGAME)
	#include "cgame/cg_local.h"
#endif

#include "cgame/animtable.h"
#include "ui_shared.h"
#include "game/bg_public.h"
#include "game/anims.h"
#include "ghoul2/G2.h"

extern void UI_UpdateCharacterSkin( void );

const char *HolocronIcons[NUM_FORCE_POWERS] = {
	"gfx/mp/f_icon_lt_heal",		//FP_HEAL,
	"gfx/mp/f_icon_levitation",		//FP_LEVITATION,
	"gfx/mp/f_icon_speed",			//FP_SPEED,
	"gfx/mp/f_icon_push",			//FP_PUSH,
	"gfx/mp/f_icon_pull",			//FP_PULL,
	"gfx/mp/f_icon_lt_telepathy",	//FP_TELEPATHY,
	"gfx/mp/f_icon_dk_grip",		//FP_GRIP,
	"gfx/mp/f_icon_dk_l1",			//FP_LIGHTNING,
	"gfx/mp/f_icon_dk_rage",		//FP_RAGE,
	"gfx/mp/f_icon_lt_protect",		//FP_PROTECT,
	"gfx/mp/f_icon_lt_absorb",		//FP_ABSORB,
	"gfx/mp/f_icon_lt_healother",	//FP_TEAM_HEAL,
	"gfx/mp/f_icon_dk_forceother",	//FP_TEAM_FORCE,
	"gfx/mp/f_icon_dk_drain",		//FP_DRAIN,
	"gfx/mp/f_icon_sight",			//FP_SEE,
	"gfx/mp/f_icon_saber_attack",	//FP_SABER_OFFENSE,
	"gfx/mp/f_icon_saber_defend",	//FP_SABER_DEFENSE,
	"gfx/mp/f_icon_saber_throw"		//FP_SABERTHROW
};

#define SCROLL_TIME_START					500
#define SCROLL_TIME_ADJUST					150
#define SCROLL_TIME_ADJUSTOFFSET			40
#define SCROLL_TIME_FLOOR					20

typedef struct scrollInfo_s {
	int nextScrollTime;
	int nextAdjustTime;
	int adjustValue;
	int scrollKey;
	float xStart;
	float yStart;
	itemDef_t *item;
	qboolean scrollDir;
} scrollInfo_t;


#ifndef _CGAME	// Defined in ui_main.c, not in the namespace
// Some extern functions hoisted from the middle of this file to get all the non-cgame,
// non-namespace stuff together
extern void UI_SaberDrawBlades( itemDef_t *item, vec3_t origin, vec3_t angles );

extern void UI_SaberLoadParms( void );
extern qboolean ui_saber_parms_parsed;
extern void UI_CacheSaberGlowGraphics( void );

#endif

qboolean Item_SetFocus(itemDef_t *item, float x, float y);

static scrollInfo_t scrollInfo;

static void (*captureFunc) (void *p) = 0;
static void *captureData = NULL;
static itemDef_t *itemCapture = NULL;   // item that has the mouse captured ( if any )

displayContextDef_t *DC = NULL;

static qboolean g_waitingForKey = qfalse;
static qboolean g_editingField = qfalse;

static itemDef_t *g_bindItem = NULL;
static itemDef_t *g_editItem = NULL;

menuDef_t Menus[MAX_MENUS];      // defined menus
int menuCount = 0;               // how many

menuDef_t *menuStack[MAX_OPEN_MENUS];
int openMenuCount = 0;

static qboolean debugMode = qfalse;

#define DOUBLE_CLICK_DELAY 300
static int lastListBoxClickTime = 0;

void Item_RunScript(itemDef_t *item, const char *s);
void Item_SetupKeywordHash(void);
void Menu_SetupKeywordHash(void);
int BindingIDFromName(const char *name);
qboolean Item_Bind_HandleKey(itemDef_t *item, int key, qboolean down);
itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu);
itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu);
static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y);
static void Item_TextScroll_BuildLines ( itemDef_t* item );
void Menu_SetItemText(const menuDef_t *menu,const char *itemName, const char *text);
extern qboolean ItemParse_asset_model_go( itemDef_t *item, const char *name,int *runTimeLength );
extern qboolean ItemParse_model_g2anim_go( itemDef_t *item, const char *animName );


#ifdef _CGAME
#define MEM_POOL_SIZE  512 * 1024
#define UI_ALLOCATION_TAG	TAG_CG_UI_ALLOC
#else
//#define MEM_POOL_SIZE  1024 * 1024
#define MEM_POOL_SIZE  2048 * 1024 * 10
#define	UI_ALLOCATION_TAG	TAG_UI_ALLOC
#endif

static char		memoryPool[MEM_POOL_SIZE];
static int		allocPoint, outOfMemory;


typedef struct  itemFlagsDef_s {
	char *string;
	int value;
}	itemFlagsDef_t;

itemFlagsDef_t itemFlags [] = {
{ "WINDOW_INACTIVE",		WINDOW_INACTIVE },
{ NULL,					(int) NULL }
};

char *styles [] = {
"WINDOW_STYLE_EMPTY",
"WINDOW_STYLE_FILLED",
"WINDOW_STYLE_GRADIENT",
"WINDOW_STYLE_SHADER",
"WINDOW_STYLE_TEAMCOLOR",
"WINDOW_STYLE_CINEMATIC",
NULL
};

char *alignment [] = {
"ITEM_ALIGN_LEFT",
"ITEM_ALIGN_CENTER",
"ITEM_ALIGN_RIGHT",
NULL
};

char *types [] = {
"ITEM_TYPE_TEXT",
"ITEM_TYPE_BUTTON",
"ITEM_TYPE_RADIOBUTTON",
"ITEM_TYPE_CHECKBOX",
"ITEM_TYPE_EDITFIELD",
"ITEM_TYPE_COMBO",
"ITEM_TYPE_LISTBOX",
"ITEM_TYPE_MODEL",
"ITEM_TYPE_OWNERDRAW",
"ITEM_TYPE_NUMERICFIELD",
"ITEM_TYPE_SLIDER",
"ITEM_TYPE_YESNO",
"ITEM_TYPE_MULTI",
"ITEM_TYPE_BIND",
"ITEM_TYPE_TEXTSCROLL",
"ITEM_TYPE_BIND_FIXED",		// Jedi Knight Galaxies
NULL
};


extern int MenuFontToHandle(int iMenuFont);




/*
===============
UI_Alloc
===============
*/
void *UI_Alloc( int size ) {

	char	*p; 

	if ( allocPoint + size > MEM_POOL_SIZE ) {
		outOfMemory = qtrue;
		if (DC->Print) {
			DC->Print("UI_Alloc: Failure. Out of memory!\n");
		}
    //DC->trap->Print(S_COLOR_YELLOW"WARNING: UI Out of Memory!\n");
		return NULL;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 15 ) & ~15;

	return p;
}

/*
===============
UI_InitMemory
===============
*/
void UI_InitMemory( void ) {
	allocPoint = 0;
	outOfMemory = qfalse;
}

qboolean UI_OutOfMemory() {
	return outOfMemory;
}





#define HASH_TABLE_SIZE 2048
/*
================
return a hash value for the string
================
*/
static long hashForString(const char *str) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (str[i] != '\0') {
		letter = (char)tolower((unsigned char)str[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (HASH_TABLE_SIZE-1);
	return hash;
}

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static int strPoolIndex = 0;
static char strPool[STRING_POOL_SIZE];

static int strHandleCount = 0;
static stringDef_t *strHandle[HASH_TABLE_SIZE];


const char *String_Alloc(const char *p) {
	int len;
	long hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = hashForString(p);

	str = strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex + 1 < STRING_POOL_SIZE) {
		int ph = strPoolIndex;
		strcpy(&strPool[strPoolIndex], p);
		strPoolIndex += len + 1;

		str = strHandle[hash];
		last = str;
		while (last && last->next) 
		{
			last = last->next;
		}

		str  = (stringDef_t *) UI_Alloc(sizeof(stringDef_t));
		str->next = NULL;
		str->str = &strPool[ph];
		if (last) {
			last->next = str;
		} else {
			strHandle[hash] = str;
		}
		return &strPool[ph];
	}

	//Increase STRING_POOL_SIZE.
	assert(0);
	return NULL;
}

void String_Report() {
	float f;
	Com_Printf("Memory/String Pool Info\n");
	Com_Printf("----------------\n");
	f = strPoolIndex;
	f /= STRING_POOL_SIZE;
	f *= 100;
	Com_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRING_POOL_SIZE);
	f = allocPoint;
	f /= MEM_POOL_SIZE;
	f *= 100;
	Com_Printf("Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEM_POOL_SIZE);
}

/*
=================
String_Init
=================
*/

void String_Init() {
	int i;
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		strHandle[i] = 0;
	}
	strHandleCount = 0;
	strPoolIndex = 0;
	menuCount = 0;
	openMenuCount = 0;
	UI_InitMemory();
	Item_SetupKeywordHash();
#ifndef _CGAME
	JKGScript_SetupKeywordHash();
#endif
	Menu_SetupKeywordHash();
	if (DC && DC->getBindingBuf) {
		Controls_GetConfig();
	}
}

#if 0
/*
=================
PC_SourceWarning
=================
*/
void PC_SourceWarning(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof( string ), format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap->PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_YELLOW "WARNING: %s, line %d: %s\n", filename, line, string);
}
#endif

/*
=================
PC_SourceError
=================
*/
void PC_SourceError(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof( string ), format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap->PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);
}

/*
=================
LerpColor
=================
*/
void LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

/*
=================
Float_Parse
=================
*/
qboolean Float_Parse(char **p, float *f) {
	char	*token;
	token = COM_ParseExt((const char **)p, qfalse);
	if (token && token[0] != 0) {
		*f = atof(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Float_Parse
=================
*/
qboolean PC_Float_Parse(int handle, float *f) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap->PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap->PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected float but found %s", token.string);
		return qfalse;
	}
	if (negative)
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;
	return qtrue;
}

/*
=================
PC_Screenspace_Parse
=================
*/
bool PC_Screenspace_Parse(int handle, float *f, bool bXorWidth) {
	pc_token_t token;
	bool bNegative = false;
	bool bUsePercentage = false;

	if (!trap->PC_ReadToken(handle, &token))
		return false;
	if (token.string[strlen(token.string)-1] == '%') // CHECKME was '%%' which isn't valid ofc
		bUsePercentage = true;
	if (token.string[0] == '-') {
		if (!trap->PC_ReadToken(handle, &token))
			return false;
		bNegative = true;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected float but found %s\n", token.string);
		return qfalse;
	}
	if (bNegative)
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;
	if (bUsePercentage)
	{
		if( bXorWidth )
			*f = (*f)/(float)SCREEN_WIDTH;
		else
			*f = (*f)/(float)SCREEN_HEIGHT;
	}
	return qtrue;
}

/*
=================
Color_Parse
=================
*/
qboolean Color_Parse(char **p, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!Float_Parse(p, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
PC_Color_Parse
=================
*/
qboolean PC_Color_Parse(int handle, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
Int_Parse
=================
*/
qboolean Int_Parse(char **p, int *i) {
	char	*token;
	token = COM_ParseExt((const char **)p, qfalse);

	if (token && token[0] != 0) {
		*i = atoi(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Int_Parse
=================
*/
qboolean PC_Int_Parse(int handle, int *i) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap->PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap->PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected integer but found %s", token.string);
		return qfalse;
	}
	*i = token.intvalue;
	if (negative)
		*i = - *i;
	return qtrue;
}

/*
=================
Rect_Parse
=================
*/
qboolean Rect_Parse(char **p, rectDef_t *r) {
	if (Float_Parse(p, &r->x)) {
		if (Float_Parse(p, &r->y)) {
			if (Float_Parse(p, &r->w)) {
				if (Float_Parse(p, &r->h)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
PC_Rect_Parse
=================
*/
qboolean PC_Rect_Parse(int handle, rectDef_t *r) {
	if (PC_Screenspace_Parse(handle, &r->x, qtrue)) {
		if (PC_Screenspace_Parse(handle, &r->y, qfalse)) {
			if (PC_Screenspace_Parse(handle, &r->w, qtrue)) {
				if (PC_Screenspace_Parse(handle, &r->h, qfalse)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
String_Parse
=================
*/
qboolean String_Parse(char **p, const char **out) {
	char *token;

	token = COM_ParseExt((const char **)p, qfalse);
	if (token && token[0] != 0) {
		*(out) = String_Alloc(token);
		return *(out)!=NULL;
	}
	return qfalse;
}

/*
=================
PC_String_Parse
=================
*/
qboolean PC_String_Parse(int handle, const char **out) 
{
	static char*	squiggy = "}";
	pc_token_t		token;

	if (!trap->PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	// Save some memory by not return the end squiggy as an allocated string
	if ( !Q_stricmp ( token.string, "}" ) )
	{
		*(out) = squiggy;
	}
	else
	{
		*(out) = String_Alloc(token.string);
	}
	return qtrue;
}

/*
=================
PC_Script_Parse
=================
*/
qboolean PC_Script_Parse(int handle, const char **out) {
	char script[2048];
	pc_token_t token;

	script[0] = 0;
	// scripts start with { and have ; separated command lists.. commands are command, arg.. 
	// basically we want everything between the { } as it will be interpreted at run time
  
	if (!trap->PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
	    return qfalse;
	}

	while ( 1 ) {
		if (!trap->PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			*out = String_Alloc(script);
			return qtrue;
		}

		if (token.string[1] != '\0') {
			Q_strcat(script, 2048, va("\"%s\"", token.string));
		} else {
			Q_strcat(script, 2048, token.string);
		}
		Q_strcat(script, 2048, " ");
	}
	return qfalse; 	// bk001105 - LCC   missing return value
}

// display, window, menu, item code
// 

/*
==================
Init_Display

Initializes the display with a structure to all the drawing routines
 ==================
*/
void Init_Display(displayContextDef_t *dc) {
	DC = dc;
}



// type and style painting 

void GradientBar_Paint(rectDef_t *rect, vec4_t color) {
	// gradient bar takes two paints
	DC->setColor( color );
	DC->drawHandlePic(rect->x, rect->y, rect->w, rect->h, DC->Assets.gradientBar);
	DC->setColor( NULL );
}


/*
==================
Window_Init

Initializes a window structure ( windowDef_t ) with defaults
 
==================
*/
void Window_Init(windowDef_t *w) {
	memset(w, 0, sizeof(windowDef_t));
	w->borderSize = 1;
	w->foreColor[0] = w->foreColor[1] = w->foreColor[2] = w->foreColor[3] = 1.0;
	w->cinematic = -1;
}

void Fade(int *flags, float *f, float clamp, int *nextTime, int offsetTime, qboolean bFlags, float fadeAmount) {
  if (*flags & (WINDOW_FADINGOUT | WINDOW_FADINGIN)) {
    if (DC->realTime > *nextTime) {
      *nextTime = DC->realTime + offsetTime;
      if (*flags & WINDOW_FADINGOUT) {
        *f -= fadeAmount;
        if (bFlags && *f <= 0.0) {
          *flags &= ~(WINDOW_FADINGOUT | WINDOW_VISIBLE);
        }
      } else {
        *f += fadeAmount;
        if (*f >= clamp) {
          *f = clamp;
          if (bFlags) {
            *flags &= ~WINDOW_FADINGIN;
          }
        }
      }
    }
  }
}



void Window_Paint(windowDef_t *w, float fadeAmount, float fadeClamp, float fadeCycle) 
{
	//float bordersize = 0;
	vec4_t color;
	rectDef_t fillRect = w->rect;

	if ( w == NULL )
		return;


	if (debugMode) 
	{
		color[0] = color[1] = color[2] = color[3] = 1;
		DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, 1, color);
	}

	if (w == NULL || (w->style == 0 && w->border == 0)) 
	{
		return;
	}

	if (w->border != 0) 
	{
		fillRect.x += w->borderSize;
		fillRect.y += w->borderSize;
		fillRect.w -= w->borderSize + 1;
		fillRect.h -= w->borderSize + 1;
	}

	if (w->style == WINDOW_STYLE_FILLED) 
	{
		// box, but possible a shader that needs filled
		if (w->background) 
		{
			Fade(&w->flags, &w->backColor[3], fadeClamp, &w->nextTime, fadeCycle, qtrue, fadeAmount);
			DC->setColor(w->backColor);
			DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
			DC->setColor(NULL);
		} 
		else 
		{
			DC->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->backColor);
		}
	} 
	else if (w->style == WINDOW_STYLE_GRADIENT) 
	{
		GradientBar_Paint(&fillRect, w->backColor);
	// gradient bar
	} 
	else if (w->style == WINDOW_STYLE_SHADER) 
	{
#ifndef _CGAME
		if (w->flags & WINDOW_PLAYERCOLOR) 
		{
			vec4_t	color;
			color[0] = ui_char_color_red.integer/255.0f;
			color[1] = ui_char_color_green.integer/255.0f;
			color[2] = ui_char_color_blue.integer/255.0f;
			color[3] = 1;
			DC->setColor(color);
		}
#endif // 

		if (w->flags & WINDOW_FORECOLORSET) 
		{
			DC->setColor(w->foreColor);
		}
		DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
		DC->setColor(NULL);
	} 
	else if (w->style == WINDOW_STYLE_TEAMCOLOR) 
	{
		if (DC->getTeamColor) 
		{
			DC->getTeamColor(&color);
			DC->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, color);
		}
	} 
	else if (w->style == WINDOW_STYLE_CINEMATIC) 
	{
		if (w->cinematic == -1) 
		{
			w->cinematic = DC->playCinematic(w->cinematicName, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
			if (w->cinematic == -1) 
			{
				w->cinematic = -2;
			}
		} 
		if (w->cinematic >= 0) 
		{
			DC->runCinematicFrame(w->cinematic);
			DC->drawCinematic(w->cinematic, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
		}
	}

	if (w->border == WINDOW_BORDER_FULL) 
	{
		// full
		// HACK HACK HACK
		if (w->style == WINDOW_STYLE_TEAMCOLOR) 
		{
			if (color[0] > 0) 
			{				
				// red
				color[0] = 1;
				color[1] = color[2] = .5;
			} 
			else 
			{
				color[2] = 1;
				color[0] = color[1] = .5;
			}
			color[3] = 1;
			DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, color);
		} 
		else 
		{
			DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, w->borderColor);
		}
	} 
	else if (w->border == WINDOW_BORDER_HORZ) 
	{
		// top/bottom
		DC->setColor(w->borderColor);
		DC->drawTopBottom(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
	} 
	else if (w->border == WINDOW_BORDER_VERT) 
	{
		// left right
		DC->setColor(w->borderColor);
		DC->drawSides(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
	} 
	else if (w->border == WINDOW_BORDER_KCGRADIENT) 
	{
		// this is just two gradient bars along each horz edge
		rectDef_t r = w->rect;
		r.h = w->borderSize;
		GradientBar_Paint(&r, w->borderColor);
		r.y = w->rect.y + w->rect.h - 1;
		GradientBar_Paint(&r, w->borderColor);
	}
}


void Item_SetScreenCoords(itemDef_t *item, float x, float y) 
{ 

	if (item == NULL) 
	{
		return;
	}

	if (item->window.border != 0) 
	{
		x += item->window.borderSize;
		y += item->window.borderSize;
	}

	item->window.rect.x = x + item->window.rectClient.x;
	item->window.rect.y = y + item->window.rectClient.y;
	item->window.rect.w = item->window.rectClient.w;
	item->window.rect.h = item->window.rectClient.h;

	// force the text rects to recompute
	item->textRect.w = 0;
	item->textRect.h = 0;

	switch ( item->type)
	{
		case ITEM_TYPE_TEXTSCROLL:
		{
			textScrollDef_t *scrollPtr = item->typeData.textscroll;
			if ( scrollPtr )
			{
				scrollPtr->startPos = 0;
				scrollPtr->endPos = 0;
			}

			Item_TextScroll_BuildLines ( item );

			break;
		}
	}
}

// FIXME: consolidate this with nearby stuff
void Item_UpdatePosition(itemDef_t *item) 
{
	float x, y;
	menuDef_t *menu;
  
	if (item == NULL || item->parent == NULL) 
	{
		return;
	}

	menu = (menuDef_t *) item->parent;

	x = menu->window.rect.x;
	y = menu->window.rect.y;
  
	if (menu->window.border != 0) 
	{
		x += menu->window.borderSize;
		y += menu->window.borderSize;
	}

	Item_SetScreenCoords(item, x, y);

}

// menus
void Menu_UpdatePosition(menuDef_t *menu) {
  int i;
  float x, y;

  if (menu == NULL) {
    return;
  }
  
  x = menu->window.rect.x;
  y = menu->window.rect.y;
  if (menu->window.border != 0) {
    x += menu->window.borderSize;
    y += menu->window.borderSize;
  }

  for (i = 0; i < menu->itemCount; i++) {
    Item_SetScreenCoords(menu->items[i], x, y);
  }
}

void Menu_PostParse(menuDef_t *menu) {
	if (menu == NULL) {
		return;
	}
	if (menu->fullScreen) {
		menu->window.rect.x = 0;
		menu->window.rect.y = 0;
		menu->window.rect.w = 640;
		menu->window.rect.h = 480;
	}
	Menu_UpdatePosition(menu);
}

itemDef_t *Menu_ClearFocus(menuDef_t *menu) {
  int i;
  itemDef_t *ret = NULL;

  if (menu == NULL) {
    return NULL;
  }

  for (i = 0; i < menu->itemCount; i++) {
    if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
      ret = menu->items[i];
    } 
    menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;
    if (menu->items[i]->leaveFocus) {
      Item_RunScript(menu->items[i], menu->items[i]->leaveFocus);
    }
  }
 
  return ret;
}

qboolean IsVisible(int flags) {
  return (flags & WINDOW_VISIBLE && !(flags & WINDOW_FADINGOUT));
}

qboolean Rect_ContainsPoint(rectDef_t *rect, float x, float y) {
  if (rect) {
    if (x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h) {
      return qtrue;
    }
  }
  return qfalse;
}

int Menu_ItemsMatchingGroup(menuDef_t *menu, const char *name) 
{
	int i;
	int count = 0;

	if(!menu)
	{
		return 0;
	}

	for (i = 0; i < menu->itemCount; i++) 
	{
		if ((!menu->items[i]->window.name) && (!menu->items[i]->window.group))
		{
			Com_Printf(S_COLOR_YELLOW"WARNING: item has neither name or group\n");
			continue;
		}

		if (Q_stricmp(menu->items[i]->window.name, name) == 0 || 
			(menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) 
		{
			count++;
		} 
	}

	return count;
}

itemDef_t *Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, const char *name) {
  int i;
  int count = 0;
  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) {
      if (count == index) {
        return menu->items[i];
      }
      count++;
    } 
  }
  return NULL;
}

qboolean Script_SetColor ( itemDef_t *item, char **args ) 
{
	const char *name;
	int i;
	float f;
	vec4_t *out;
	
	// expecting type of color to set and 4 args for the color
	if (String_Parse(args, &name)) 
	{
		out = NULL;
		if (Q_stricmp(name, "backcolor") == 0) 
		{
			out = &item->window.backColor;
			item->window.flags |= WINDOW_BACKCOLORSET;
		} 
		else if (Q_stricmp(name, "forecolor") == 0) 
		{
			out = &item->window.foreColor;
			item->window.flags |= WINDOW_FORECOLORSET;
		} 
		else if (Q_stricmp(name, "bordercolor") == 0) 
		{
			out = &item->window.borderColor;
		}

		if (out) 
		{
			for (i = 0; i < 4; i++) 
			{
				if (!Float_Parse(args, &f)) 
				{
					return qtrue;
				}
			(*out)[i] = f;
			}
		}
	}

	return qtrue;
}

qboolean Script_SetAsset(itemDef_t *item, char **args) 
{
	const char *name;
	// expecting name to set asset to
	if (String_Parse(args, &name)) 
	{
		// check for a model 
		if (item->type == ITEM_TYPE_MODEL) 
		{
		}
	}
	return qtrue;
}

qboolean Script_SetBackground(itemDef_t *item, char **args) 
{
	const char *name;
	// expecting name to set asset to
	if (String_Parse(args, &name)) 
	{
		item->window.background = DC->registerShaderNoMip(name);
	}
	return qtrue;
}

qboolean Script_SetItemRectCvar(itemDef_t *item, char **args) 
{
	const char	*itemName;
	const char	*cvarName;
	char		cvarBuf[1024];
	const char	*holdVal;
	char		*holdBuf;
	itemDef_t	*item2=0;
	menuDef_t	*menu;	

	// expecting item group and cvar to get value from
	if (String_Parse(args, &itemName) && String_Parse(args, &cvarName)) 
	{
		item2 = Menu_FindItemByName((menuDef_t *) item->parent, itemName);

		if (item2)
		{
			// get cvar data
			DC->getCVarString(cvarName, cvarBuf, sizeof(cvarBuf));
			
			holdBuf = cvarBuf;
			if (String_Parse(&holdBuf,&holdVal))
			{
				menu = (menuDef_t *) item->parent;

				item2->window.rectClient.x = atof(holdVal) + menu->window.rect.x;
				if (String_Parse(&holdBuf,&holdVal))
				{
					item2->window.rectClient.y = atof(holdVal) + menu->window.rect.y;
					if (String_Parse(&holdBuf,&holdVal))
					{
						item2->window.rectClient.w = atof(holdVal);
						if (String_Parse(&holdBuf,&holdVal))
						{
							item2->window.rectClient.h = atof(holdVal);

							item2->window.rect.x = item2->window.rectClient.x;
							item2->window.rect.y = item2->window.rectClient.y;
							item2->window.rect.w = item2->window.rectClient.w;
							item2->window.rect.h = item2->window.rectClient.h;

							return qtrue;
						}
					}
				}
			}
		}
	}

	// Default values in case things screw up
	if (item2)
	{
		item2->window.rectClient.x = 0;
		item2->window.rectClient.y = 0;
		item2->window.rectClient.w = 0;
		item2->window.rectClient.h = 0;
	}

//	Com_Printf(S_COLOR_YELLOW"WARNING: SetItemRectCvar: problems. Set cvar to 0's\n" );

	return qtrue;
}

qboolean Script_SetItemBackground(itemDef_t *item, char **args) 
{
	const char *itemName;
	const char *name;

	// expecting name of shader
	if (String_Parse(args, &itemName) && String_Parse(args, &name)) 
	{
		Menu_SetItemBackground((menuDef_t *) item->parent, itemName, name);
	}
	return qtrue;
}

qboolean Script_SetItemText(itemDef_t *item, char **args) 
{
	const char *itemName;
	const char *text;

	// expecting text
	if (String_Parse(args, &itemName) && String_Parse(args, &text)) 
	{
		Menu_SetItemText((menuDef_t *) item->parent, itemName, text);
	}
	return qtrue;
}


itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p) {
  int i;
  if (menu == NULL || p == NULL) {
    return NULL;
  }

  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(p, menu->items[i]->window.name) == 0) {
      return menu->items[i];
    }
  }

  return NULL;
}

qboolean Script_SetTeamColor(itemDef_t *item, char **args) 
{
	if (DC->getTeamColor) 
	{
		int i;
		vec4_t color;
		DC->getTeamColor(&color);
		for (i = 0; i < 4; i++) 
		{
			item->window.backColor[i] = color[i];
		}
	}
	return qtrue;
}

qboolean Script_SetItemColor(itemDef_t *item, char **args) 
{
	const char *itemname;
	const char *name;
	vec4_t color;
	int i;
	vec4_t *out;

	// expecting type of color to set and 4 args for the color
	if (String_Parse(args, &itemname) && String_Parse(args, &name)) 
	{
		itemDef_t	*item2;
		int			j,count;
		char buff[1024];

		// Is is specifying a cvar to get the item name from?
		if (itemname[0] == '*')
		{
			itemname += 1;
		    DC->getCVarString(itemname, buff, sizeof(buff));
			itemname = buff;
		}

		count = Menu_ItemsMatchingGroup((menuDef_t *) item->parent, itemname);

		if (!Color_Parse(args, &color)) 
		{
			return qtrue;
		}

		for (j = 0; j < count; j++) 
		{
			item2 = Menu_GetMatchingItemByNumber((menuDef_t *) item->parent, j, itemname);
			if (item2 != NULL) 
			{
				out = NULL;
				if (Q_stricmp(name, "backcolor") == 0) 
				{
					out = &item2->window.backColor;
				} 
				else if (Q_stricmp(name, "forecolor") == 0) 
				{
					out = &item2->window.foreColor;
					item2->window.flags |= WINDOW_FORECOLORSET;
				} 
				else if (Q_stricmp(name, "bordercolor") == 0) 
				{
					out = &item2->window.borderColor;
				}

				if (out) 
				{
					for (i = 0; i < 4; i++) 
					{
						(*out)[i] = color[i];
					}
				}
			}
		}
	}

	return qtrue;
}

qboolean Script_SetItemColorCvar(itemDef_t *item, char **args) 
{
	const char *itemname;
	char	*colorCvarName,*holdBuf,*holdVal;
	char cvarBuf[1024];
	const char *name;
	vec4_t color = { 0.0f };
	int i;
	vec4_t *out;

	// expecting type of color to set and 4 args for the color
	if (String_Parse(args, &itemname) && String_Parse(args, &name)) 
	{
		itemDef_t	*item2;
		int			j,count;
		char buff[1024];

		// Is is specifying a cvar to get the item name from?
		if (itemname[0] == '*')
		{
			itemname += 1;
		    DC->getCVarString(itemname, buff, sizeof(buff));
			itemname = buff;
		}

		count = Menu_ItemsMatchingGroup((menuDef_t *) item->parent, itemname);

		// Get the cvar with the color
		if (!String_Parse(args,(const char **) &colorCvarName))
		{
			return qtrue;
		}
		else
		{
			DC->getCVarString(colorCvarName, cvarBuf, sizeof(cvarBuf));
			
			holdBuf = cvarBuf;
			if (String_Parse(&holdBuf,(const char **) &holdVal))
			{
				color[0] = atof(holdVal);
				if (String_Parse(&holdBuf,(const char **) &holdVal))
				{
					color[1] = atof(holdVal);
					if (String_Parse(&holdBuf,(const char **) &holdVal))
					{
						color[2] = atof(holdVal);
						if (String_Parse(&holdBuf,(const char **) &holdVal))
						{
							color[3] = atof(holdVal);
						}
					}
				}
			}
		}

		for (j = 0; j < count; j++) 
		{
			item2 = Menu_GetMatchingItemByNumber((menuDef_t *) item->parent, j, itemname);
			if (item2 != NULL) 
			{
				out = NULL;
				if (Q_stricmp(name, "backcolor") == 0) 
				{
					out = &item2->window.backColor;
				} 
				else if (Q_stricmp(name, "forecolor") == 0) 
				{
					out = &item2->window.foreColor;
					item2->window.flags |= WINDOW_FORECOLORSET;
				} 
				else if (Q_stricmp(name, "bordercolor") == 0) 
				{
					out = &item2->window.borderColor;
				}

				if (out) 
				{
					for (i = 0; i < 4; i++) 
					{
						(*out)[i] = color[i];
					}
				}
			}
		}
	}

	return qtrue;
}

qboolean Script_SetItemRect(itemDef_t *item, char **args) 
{
	const char *itemname;
	rectDef_t *out;
	rectDef_t rect;
	menuDef_t	*menu;	

	// expecting type of color to set and 4 args for the color
	if (String_Parse(args, &itemname)) 
	{
		itemDef_t *item2;
		int j;
		int count = Menu_ItemsMatchingGroup((menuDef_t *) item->parent, itemname);

		if (!Rect_Parse(args, &rect)) 
		{
			return qtrue;
		}

		menu = (menuDef_t *) item->parent;

		for (j = 0; j < count; j++) 
		{
			item2 = Menu_GetMatchingItemByNumber(menu, j, itemname);
			if (item2 != NULL) 
			{
				out = &item2->window.rect;

				if (out) 
				{
					item2->window.rect.x = rect.x  + menu->window.rect.x;
					item2->window.rect.y = rect.y  + menu->window.rect.y;
					item2->window.rect.w = rect.w;
					item2->window.rect.h = rect.h;					
				}
			}
		}
	}
	return qtrue;
}

void Menu_ShowGroup (menuDef_t *menu, char *groupName, qboolean showFlag)
{
	itemDef_t *item;
	int count,j;

	count = Menu_ItemsMatchingGroup( menu, groupName);
	for (j = 0; j < count; j++) 
	{
		item = Menu_GetMatchingItemByNumber( menu, j, groupName);
		if (item != NULL) 
		{
			if (showFlag) 
			{
				item->window.flags |= WINDOW_VISIBLE;
			} 
			else 
			{
				item->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS);
			}
		}
	}
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow) {
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) {
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) {
			if (bShow) {
				item->window.flags |= WINDOW_VISIBLE;
			} else {
				item->window.flags &= ~WINDOW_VISIBLE;
				item->window.flags &= ~WINDOW_MOUSEOVER;
				// stop cinematics playing in the window
				if (item->window.cinematic >= 0) {
					DC->stopCinematic(item->window.cinematic);
					item->window.cinematic = -1;
				}
			}
		}
	}
}

void Menu_FadeItemByName(menuDef_t *menu, const char *p, qboolean fadeOut) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      if (fadeOut) {
        item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
        item->window.flags &= ~WINDOW_FADINGIN;
      } else {
        item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
        item->window.flags &= ~WINDOW_FADINGOUT;
      }
    }
  }
}

menuDef_t *Menus_FindByName(const char *p) {
  int i;
  for (i = 0; i < menuCount; i++) {
    if (Q_stricmp(Menus[i].window.name, p) == 0) {
      return &Menus[i];
    } 
  }
  return NULL;
}

void Menus_ShowByName(const char *p) {
	menuDef_t *menu = Menus_FindByName(p);
	if (menu) {
		Menus_Activate(menu);
	}
}

void Menus_OpenByName(const char *p) {
  Menus_ActivateByName(p);
}

static void Menu_RunCloseScript(menuDef_t *menu) {
	if (menu && menu->window.flags & WINDOW_VISIBLE && menu->onClose) {
		itemDef_t item;
    item.parent = menu;
    Item_RunScript(&item, menu->onClose);
	}
}

void Menus_CloseByName ( const char *p )
{
	menuDef_t *menu = Menus_FindByName(p);
	
	// If the menu wasnt found just exit
	if (menu == NULL) 
	{
		return;
	}

	// Run the close script for the menu
	Menu_RunCloseScript(menu);

	// If this window had the focus then take it away
	if ( menu->window.flags & WINDOW_HASFOCUS )
	{	
		// If there is something still in the open menu list then
		// set it to have focus now
		if ( openMenuCount )
		{
			// Subtract one from the open menu count to prepare to
			// remove the top menu from the list
			openMenuCount -= 1;

			// Set the top menu to have focus now
			menuStack[openMenuCount]->window.flags |= WINDOW_HASFOCUS;

			// Remove the top menu from the list
			menuStack[openMenuCount] = NULL;
		}
	}

	// Window is now invisible and doenst have focus
	menu->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS);
}

int FPMessageTime = 0;

void Menus_CloseAll() 
{
	int i;
	
	g_waitingForKey = qfalse;
	
	for (i = 0; i < menuCount; i++) 
	{
		Menu_RunCloseScript ( &Menus[i] );
		Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
	}

	// Clear the menu stack
	openMenuCount = 0;

	FPMessageTime = 0;
}

qboolean Script_Show(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		Menu_ShowItemByName((menuDef_t *) item->parent, name, qtrue);
	}
	return qtrue;
}

qboolean Script_Hide(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		// someone is going to get the Bob Barker treatment for doing this --eez
		//if(item->parent != (void *)0xcccccccc)
		//{
			Menu_ShowItemByName((menuDef_t *) item->parent, name, qfalse);
		//}
	}
	return qtrue;
}

qboolean Script_FadeIn(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		Menu_FadeItemByName((menuDef_t *) item->parent, name, qfalse);
	}

	return qtrue;
}

qboolean Script_FadeOut(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		Menu_FadeItemByName((menuDef_t *) item->parent, name, qtrue);
	}
	return qtrue;
}

qboolean Script_Open(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		Menus_OpenByName(name);
	}
	return qtrue;
}

qboolean Script_Close(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		if (Q_stricmp(name, "all") == 0)
		{
			Menus_CloseAll();
		}
		else
		{
			Menus_CloseByName(name);
		}
	}
	return qtrue;
}

//void Menu_TransitionItemByName(menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, int time, float amt) 
void Menu_TransitionItemByName(menuDef_t *menu, const char *p, const rectDef_t *rectFrom, const rectDef_t *rectTo, int time, float amt) 
{
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) 
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			if (!rectFrom)
			{
				rectFrom = &item->window.rect;	//if there are more than one of these with the same name, they'll all use the FIRST one's FROM.
			}
			item->window.flags |= (WINDOW_INTRANSITION | WINDOW_VISIBLE);
			item->window.offsetTime = time;
			memcpy(&item->window.rectClient, rectFrom, sizeof(rectDef_t));
			memcpy(&item->window.rectEffects, rectTo, sizeof(rectDef_t));
			item->window.rectEffects2.x = fabs(rectTo->x - rectFrom->x) / amt;
			item->window.rectEffects2.y = fabs(rectTo->y - rectFrom->y) / amt;
			item->window.rectEffects2.w = fabs(rectTo->w - rectFrom->w) / amt;
			item->window.rectEffects2.h = fabs(rectTo->h - rectFrom->h) / amt;

			Item_UpdatePosition(item);
		}
	}
}

/*
=================
Menu_Transition3ItemByName
=================
*/
//JLF
#define _TRANS3
#ifdef _TRANS3
void Menu_Transition3ItemByName(menuDef_t *menu, const char *p, const float minx, const float miny, const float minz,
								const float maxx, const float maxy, const float maxz, const float fovtx, const float fovty,
								const int time, const float amt)
{
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	modelDef_t  * modelptr;
	for (i = 0; i < count; i++) 
	{
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) 
		{
			if ( item->type == ITEM_TYPE_MODEL)
			{
				modelptr = item->typeData.model;

				item->window.flags |= (WINDOW_INTRANSITIONMODEL | WINDOW_VISIBLE);
				item->window.offsetTime = time;
				modelptr->fov_x2 = fovtx;
				modelptr->fov_y2 = fovty;
				VectorSet(modelptr->g2maxs2, maxx, maxy, maxz);
				VectorSet(modelptr->g2mins2, minx, miny, minz);

//				//modelptr->g2maxs2.x= maxx;
//				modelptr->g2maxs2.y= maxy;
//				modelptr->g2maxs2.z= maxz;
//				modelptr->g2mins2.x= minx;
//				modelptr->g2mins2.y= miny;
//				modelptr->g2mins2.z= minz;

//				VectorSet(modelptr->g2maxs2, maxx, maxy, maxz);

				modelptr->g2maxsEffect[0] = fabs(modelptr->g2maxs2[0] - modelptr->g2maxs[0]) / amt;
				modelptr->g2maxsEffect[1] = fabs(modelptr->g2maxs2[1] - modelptr->g2maxs[1]) / amt;
				modelptr->g2maxsEffect[2] = fabs(modelptr->g2maxs2[2] - modelptr->g2maxs[2]) / amt;

				modelptr->g2minsEffect[0] = fabs(modelptr->g2mins2[0] - modelptr->g2mins[0]) / amt;
				modelptr->g2minsEffect[1] = fabs(modelptr->g2mins2[1] - modelptr->g2mins[1]) / amt;
				modelptr->g2minsEffect[2] = fabs(modelptr->g2mins2[2] - modelptr->g2mins[2]) / amt;
				

				modelptr->fov_Effectx = fabs(modelptr->fov_x2 - modelptr->fov_x) / amt;
				modelptr->fov_Effecty = fabs(modelptr->fov_y2 - modelptr->fov_y) / amt;
			}
				
		}
	}
}

#endif







#define MAX_DEFERRED_SCRIPT		2048

char		ui_deferredScript [ MAX_DEFERRED_SCRIPT ];
itemDef_t*	ui_deferredScriptItem = NULL;

/*
=================
Script_Defer

Defers the rest of the script based on the defer condition.  The deferred
portion of the script can later be run with the "rundeferred"
=================
*/
qboolean Script_Defer ( itemDef_t* item, char **args )
{
	// Should the script be deferred?
	if ( DC->deferScript ( (char**)args ) )
	{
		// Need the item the script was being run on
		ui_deferredScriptItem = item;

		// Save the rest of the script
		Q_strncpyz ( ui_deferredScript, *args, MAX_DEFERRED_SCRIPT );

		// No more running
		return qfalse;
	}

	// Keep running the script, its ok
	return qtrue;
}

/*
=================
Script_RunDeferred

Runs the last deferred script, there can only be one script deferred at a 
time so be careful of recursion
=================
*/
qboolean Script_RunDeferred ( itemDef_t* item, char **args )
{
	// Make sure there is something to run.
	if ( !ui_deferredScript[0] || !ui_deferredScriptItem )
	{
		return qtrue;
	}

	// Run the deferred script now
	Item_RunScript ( ui_deferredScriptItem, ui_deferredScript );

	return qtrue;
}

qboolean Script_Transition(itemDef_t *item, char **args) 
{
	const char *name;
	rectDef_t rectFrom, rectTo;
	int time;
	float amt;

	if (String_Parse(args, &name)) 
	{
	    if ( Rect_Parse(args, &rectFrom) && Rect_Parse(args, &rectTo) && Int_Parse(args, &time) && Float_Parse(args, &amt)) 
		{
			Menu_TransitionItemByName((menuDef_t *) item->parent, name, &rectFrom, &rectTo, time, amt);
		}
	}

	return qtrue;
}

void Menu_OrbitItemByName(menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time) 
{
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      item->window.flags |= (WINDOW_ORBITING | WINDOW_VISIBLE);
      item->window.offsetTime = time;
      item->window.rectEffects.x = cx;
      item->window.rectEffects.y = cy;
      item->window.rectClient.x = x;
      item->window.rectClient.y = y;
      Item_UpdatePosition(item);
    }
  }
}

void Menu_ItemDisable(menuDef_t *menu, char *name,int disableFlag)
{
	int	j,count;
	itemDef_t *itemFound;

	count = Menu_ItemsMatchingGroup(menu, name);
	// Loop through all items that have this name
	for (j = 0; j < count; j++) 
	{
		itemFound = Menu_GetMatchingItemByNumber( menu, j, name);
		if (itemFound != NULL) 
		{
			itemFound->disabled = disableFlag;
			// Just in case it had focus
			itemFound->window.flags &= ~WINDOW_MOUSEOVER;
		}
	}
}

// Set item disable flags
qboolean Script_Disable(itemDef_t *item, char **args) 
{
	char *name;
	int	value;
	menuDef_t *menu;

	if (String_Parse(args, (const char **)&name)) 
	{
		char buff[1024];

		// Is is specifying a cvar to get the item name from?
		if (name[0] == '*')
		{
			name += 1;
		    DC->getCVarString(name, buff, sizeof(buff));
			name = buff;
		}

		if ( Int_Parse(args, &value)) 
		{
			menu = Menu_GetFocused();
			Menu_ItemDisable(menu, name,value);
		}
	}

	return qtrue;
}


// Scale the given item instantly.
qboolean Script_Scale(itemDef_t *item, char **args) 
{
	const char *name;
	float scale;
	int	j,count;
	itemDef_t *itemFound;
	rectDef_t rectTo;

	if (String_Parse(args, &name)) 
	{
		char buff[1024];

		// Is is specifying a cvar to get the item name from?
		if (name[0] == '*')
		{
			name += 1;
		    DC->getCVarString(name, buff, sizeof(buff));
			name = buff;
		}

		count = Menu_ItemsMatchingGroup((menuDef_t *) item->parent, name);

		if ( Float_Parse(args, &scale)) 
		{
			for (j = 0; j < count; j++) 
			{
				itemFound = Menu_GetMatchingItemByNumber( (menuDef_t *) item->parent, j, name);
				if (itemFound != NULL) 
				{
					rectTo.h = itemFound->window.rect.h * scale;
					rectTo.w = itemFound->window.rect.w * scale;

					rectTo.x = itemFound->window.rect.x + ((itemFound->window.rect.h - rectTo.h)/2);
					rectTo.y = itemFound->window.rect.y + ((itemFound->window.rect.w - rectTo.w)/2);

					Menu_TransitionItemByName((menuDef_t *) item->parent, name, 0, &rectTo, 1, 1);
				}
			}
		}
	}

	return qtrue;
}

qboolean Script_Orbit(itemDef_t *item, char **args) 
{
	const char *name;
	float cx, cy, x, y;
	int time;

	if (String_Parse(args, &name)) 
	{
		if ( Float_Parse(args, &x) && Float_Parse(args, &y) && Float_Parse(args, &cx) && Float_Parse(args, &cy) && Int_Parse(args, &time) ) 
		{
			Menu_OrbitItemByName((menuDef_t *) item->parent, name, x, y, cx, cy, time);
		}
	}

	return qtrue;
}

qboolean Script_SetFocus(itemDef_t *item, char **args) 
{
	const char *name;
	itemDef_t *focusItem;

	if (String_Parse(args, &name)) {
		focusItem = Menu_FindItemByName((menuDef_t *) item->parent, name);
		if (focusItem && !(focusItem->window.flags & WINDOW_DECORATION) && !(focusItem->window.flags & WINDOW_HASFOCUS)) {
			Menu_ClearFocus((menuDef_t *) item->parent);
			focusItem->window.flags |= WINDOW_HASFOCUS;


			if (focusItem->onFocus) {
				Item_RunScript(focusItem, focusItem->onFocus);
			}
			if (DC->Assets.itemFocusSound) {
		  // JKG - use the CHAN_AUTO channel to avoid sound stuttering
		  //DC->startLocalSound( DC->Assets.itemFocusSound, CHAN_LOCAL_SOUND );
		  DC->startLocalSound( DC->Assets.itemFocusSound, CHAN_AUTO );
			}
		}
	}

	return qtrue;
}

qboolean Script_SetPlayerModel(itemDef_t *item, char **args) 
{
	const char *name;
	if (String_Parse(args, &name)) 
	{
		DC->setCVar("model", name);
	}

	return qtrue;
}

/*
=================
ParseRect
=================
*/
qboolean ParseRect(const char **p, rectDef_t *r) 
{
	if (!COM_ParseFloat(p, &r->x)) 
	{
		if (!COM_ParseFloat(p, &r->y)) 
		{
			if (!COM_ParseFloat(p, &r->w)) 
			{
				if (!COM_ParseFloat(p, &r->h)) 
				{
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}
/*
=================
Script_Transition2
uses current origin instead of specifing a starting origin

transition2		lfvscr		25 0 202 264  20 25
=================
*/
qboolean Script_Transition2(itemDef_t *item, char **args) 
{
	const char *name;
	rectDef_t rectTo;
	int time;
	float amt;

	if (String_Parse(args, &name)) 
	{
		if ( ParseRect((const char **) args, &rectTo) && Int_Parse(args, &time) && !COM_ParseFloat((const char **) args, &amt)) 
		{
			Menu_TransitionItemByName((menuDef_t *) item->parent, name, 0, &rectTo, time, amt);
		}
		else
		{
			Com_Printf(S_COLOR_YELLOW"WARNING: Script_Transition2: error parsing '%s'\n", name );
		}
	}

	return qtrue;
}

#ifdef _TRANS3
/*
JLF 
=================
Script_Transition3

used exclusively with model views
uses current origin instead of specifing a starting origin


transition3		lfvscr		(min extent) (max extent) (fovx,y)  20 25
=================
*/
qboolean Script_Transition3(itemDef_t *item, char **args) 
{
	const char *name = NULL, *value = NULL;
	float minx, miny, minz, maxx, maxy, maxz, fovtx, fovty;
	int time;
	float amt;

	if (String_Parse(args, &name)) 
	{	
		if (String_Parse( args, &value))
		{
			minx = atof(value);
			if (String_Parse( args, &value))
			{	
				miny = atof(value);
				if (String_Parse( args, &value))
				{
					minz = atof(value);
					if (String_Parse( args, &value))
					{	
						maxx = atof(value);
						if (String_Parse( args, &value))
						{
							maxy = atof(value);
							if (String_Parse( args, &value))
							{
								maxz = atof(value);
								if (String_Parse( args, &value))
								{
									fovtx = atof(value);
									if (String_Parse( args, &value))
									{	
										fovty = atof(value);
										
										if (String_Parse( args, &value))
										{	
											time = atoi(value);
											if (String_Parse( args, &value))
											{	
												amt = atof(value);
												//set up the variables
												Menu_Transition3ItemByName((menuDef_t *) item->parent, 
														name,
														minx, miny, minz,
														maxx, maxy, maxz, 
														fovtx, fovty,
														time, amt);
												
												return qtrue;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if ( name ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: Script_Transition2: error parsing '%s'\n", name );
	}
	return qtrue;
}
#endif

qboolean Script_SetCvar(itemDef_t *item, char **args) 
{
	const char *cvar, *val;
	if (String_Parse(args, &cvar) && String_Parse(args, &val)) 
	{
		DC->setCVar(cvar, val);
	}
	return qtrue;
}

qboolean Script_SetCvarToCvar(itemDef_t *item, char **args) {
	const char *cvar, *val;
	if (String_Parse(args, &cvar) && String_Parse(args, &val)) {
		char cvarBuf[1024];
		DC->getCVarString(val, cvarBuf, sizeof(cvarBuf));
		DC->setCVar(cvar, cvarBuf);
	}
	return qtrue;
}

qboolean Script_Exec(itemDef_t *item, char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->executeText(EXEC_APPEND, va("%s ; ", val));
	}
	return qtrue;
}

qboolean Script_Play(itemDef_t *item, char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->startLocalSound(DC->registerSound(val), CHAN_AUTO);
	}
	return qtrue;
}

qboolean Script_playLooped(itemDef_t *item, char **args) {
	const char *val;
	if (String_Parse(args, &val)) {
		DC->stopBackgroundTrack();
		DC->startBackgroundTrack(val, val, qfalse);
	}
	return qtrue;
}

//Jedi Knight Galaxies
qboolean Script_SetText(itemDef_t *item, char **args) {
	const char *itemname, *text;
	itemDef_t *itm;
	if (String_Parse(args, &itemname) && String_Parse(args, &text)) {
		itm = Menu_FindItemByName((menuDef_t *)item->parent, itemname);
		if (itm->type == ITEM_TYPE_EDITFIELD && !itm->cvar) {
			if (*text == '@') {
				text++;
				trap->SE_GetStringTextString( text, itm->typeData.edit->buffer, sizeof(itm->typeData.edit->buffer));
			} else {
				Q_strncpyz(itm->typeData.edit->buffer, text, sizeof(itm->typeData.edit->buffer));
			}
			itm->cursorPos = strlen(itm->typeData.edit->buffer);
		}
	}
	return qtrue;
}

qboolean Script_ClearText(itemDef_t *item, char **args) {
	const char *itemname;
	itemDef_t *itm;
	if (String_Parse(args, &itemname)) {
		itm = Menu_FindItemByName((menuDef_t *)item->parent, itemname);
		if (!itm) return qfalse;
		if (itm->type == ITEM_TYPE_EDITFIELD && !itm->cvar) {
			itm->typeData.edit->buffer[0] = 0;
			itm->cursorPos = 0;
		}
	}
	return qtrue;
}


qboolean Script_JKGScript(itemDef_t *item, char **args) {
#ifndef _CGAME
	const char *scriptname;
	if (String_Parse(args, &scriptname)) {
		UI_RunJKGScript(scriptname, args);
	}
#endif
	return qtrue;
}

qboolean Script_ClearFocus(itemDef_t *item, char **args) {
	Menu_ClearFocus((menuDef_t *) item->parent);
	return qtrue;
}



commandDef_t commandList[] =
{
  {"fadein", &Script_FadeIn},                   // group/name
  {"fadeout", &Script_FadeOut},                 // group/name
  {"show", &Script_Show},                       // group/name
  {"hide", &Script_Hide},                       // group/name
  {"setcolor", &Script_SetColor},               // works on this
  {"open", &Script_Open},                       // nenu
  {"close", &Script_Close},                     // menu
  {"setasset", &Script_SetAsset},               // works on this
  {"setbackground", &Script_SetBackground},     // works on this
  {"setitemrectcvar", &Script_SetItemRectCvar},			// group/name
  {"setitembackground", &Script_SetItemBackground},// group/name
  {"setitemtext", &Script_SetItemText},			// group/name
  {"setitemcolor", &Script_SetItemColor},       // group/name
  {"setitemcolorcvar", &Script_SetItemColorCvar},// group/name
  {"setitemrect", &Script_SetItemRect},			// group/name
  {"setteamcolor", &Script_SetTeamColor},       // sets this background color to team color
  {"setfocus", &Script_SetFocus},               // sets focus
  {"setplayermodel", &Script_SetPlayerModel},   // sets model
  {"transition", &Script_Transition},           // group/name
  {"setcvar", &Script_SetCvar},					// name
  {"setcvartocvar", &Script_SetCvarToCvar},     // name
  {"exec", &Script_Exec},						// group/name
  {"play", &Script_Play},						// group/name
  {"playlooped", &Script_playLooped},           // group/name
  {"orbit", &Script_Orbit},                     // group/name
  {"scale",			&Script_Scale},				// group/name
  {"disable",		&Script_Disable},			// group/name
  {"defer",			&Script_Defer},				// 
  {"rundeferred",	&Script_RunDeferred},		//
  {"transition2",	&Script_Transition2},			// group/name
  // Jedi Knight Galaxies
  {"settext",	&Script_SetText},
  {"cleartext",	&Script_ClearText},
  {"jkgscript", &Script_JKGScript},
  {"clearfocus", &Script_ClearFocus},
};

// Set all the items within a given menu, with the given itemName, to the given shader
void Menu_SetItemBackground(const menuDef_t *menu,const char *itemName, const char *background) 
{
	itemDef_t	*item;
	int			j, count;

	if (!menu)	// No menu???
	{
		return;
	}

	count = Menu_ItemsMatchingGroup( (menuDef_t *) menu, itemName);

	for (j = 0; j < count; j++) 
	{
		item = Menu_GetMatchingItemByNumber( (menuDef_t *) menu, j, itemName);
		if (item != NULL) 
		{
			item->window.background = DC->registerShaderNoMip(background);
		}
	}
}

// Set all the items within a given menu, with the given itemName, to the given text
void Menu_SetItemText(const menuDef_t *menu,const char *itemName, const char *text) 
{
	itemDef_t	*item;
	int			j, count;

	if (!menu)	// No menu???
	{
		return;
	}

	count = Menu_ItemsMatchingGroup( (menuDef_t *) menu, itemName);

	for (j = 0; j < count; j++) 
	{
		item = Menu_GetMatchingItemByNumber( (menuDef_t *) menu, j, itemName);
		if (item != NULL) 
		{
			if (text[0] == '*')
			{
				item->text[0] = '\0';		// Null this out because this would take presidence over cvar text.
				item->cvar = text+1;
				// Just copying what was in ItemParse_cvar()
				if ( item->typeData.edit )
				{
					editFieldDef_t *editPtr = item->typeData.edit;
					editPtr->minVal = -1;
					editPtr->maxVal = -1;
					editPtr->defVal = -1;
				}
			}
			else
			{
				strcpy(item->text, text);
				if ( item->type == ITEM_TYPE_TEXTSCROLL )
				{
					textScrollDef_t *scrollPtr = item->typeData.textscroll;
					if ( scrollPtr )
					{
						scrollPtr->startPos = 0;
						scrollPtr->endPos = 0;
					}

					Item_TextScroll_BuildLines ( item );
				}
			}
		}
	}
}

int scriptCommandCount = sizeof(commandList) / sizeof(commandDef_t);

void Item_RunScript(itemDef_t *item, const char *s) 
{
	char script[2048], *p;
	int i;
	qboolean bRan;

	script[0] = 0;
	
	if (item && s && s[0]) 
	{
		Q_strcat(script, 2048, s);
		p = script;
		
		while (1) 
		{
			const char *command;

			// expect command then arguments, ; ends command, NULL ends script
			if (!String_Parse(&p, &command)) 
			{
				return;
			}

			if (command[0] == ';' && command[1] == '\0') 
			{
				continue;
			}

			bRan = qfalse;
			for (i = 0; i < scriptCommandCount; i++) 
			{
				if (Q_stricmp(command, commandList[i].name) == 0) 
				{
					// Allow a script command to stop processing the script
					if ( !commandList[i].handler(item, &p) )
					{
						return;
					}

					bRan = qtrue;
					break;
				}
			}

			// not in our auto list, pass to handler
			if (!bRan) 
			{
				DC->runScript(&p);
			}
		}
	}
}


qboolean Item_EnableShowViaCvar(itemDef_t *item, int flag) {
  char script[2048], *p;
  if (item && item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) {
		char buff[2048];
	  DC->getCVarString(item->cvarTest, buff, sizeof(buff));

    Q_strncpyz(script, item->enableCvar, 2048);
    p = script;
    while (1) {
      const char *val;
      // expect value then ; or NULL, NULL ends list
      if (!String_Parse(&p, &val)) {
				return (item->cvarFlags & flag) ? qfalse : qtrue;
      }

      if (val[0] == ';' && val[1] == '\0') {
        continue;
      }

			// enable it if any of the values are true
			if (item->cvarFlags & flag) {
        if (Q_stricmp(buff, val) == 0) {
					return qtrue;
				}
			} else {
				// disable it if any of the values are true
        if (Q_stricmp(buff, val) == 0) {
					return qfalse;
				}
			}

    }
		return (item->cvarFlags & flag) ? qfalse : qtrue;
  }
	return qtrue;
}


// will optionaly set focus to this item 
qboolean Item_SetFocus(itemDef_t *item, float x, float y) {
	int i;
	itemDef_t *oldFocus;
	sfxHandle_t *sfx = &DC->Assets.itemFocusSound;
	qboolean playSound = qfalse;
	menuDef_t *parent; // bk001206: = (menuDef_t*)item->parent;
	// sanity check, non-null, not a decoration and does not already have the focus
	if (item == NULL || item->window.flags & WINDOW_DECORATION || item->window.flags & WINDOW_HASFOCUS || !(item->window.flags & WINDOW_VISIBLE)) {
		return qfalse;
	}

	// bk001206 - this can be NULL.
	parent = (menuDef_t*)item->parent; 

	// items can be enabled and disabled 
	if (item->disabled) 
	{
		return qfalse;
	}
      
	// items can be enabled and disabled based on cvars
	if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
		return qfalse;
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) {
		return qfalse;
	}

	oldFocus = Menu_ClearFocus((menuDef_t *) item->parent);

	if (item->type == ITEM_TYPE_TEXT) {
		rectDef_t r;
		r = item->textRect;
		r.y -= r.h;

		if (Rect_ContainsPoint(&r, x, y)) 
		{
			if(!(item->window.flags & WINDOW_MULTIFOCUS))
			{
				item->window.flags |= WINDOW_HASFOCUS;
			}
			if (item->focusSound) {
				sfx = &item->focusSound;
			}
			if (!(item->window.flags & WINDOW_NOFOCUSSOUND)) 
			{
				playSound = qtrue;
			}
		}
		else 
		{
			if (oldFocus) 
			{
				oldFocus->window.flags |= WINDOW_HASFOCUS;
				if (oldFocus->onFocus) 
				{
					Item_RunScript(oldFocus, oldFocus->onFocus);
				}
			}
		}
	} else {
		if(!(item->window.flags & WINDOW_MULTIFOCUS))
		{
			item->window.flags |= WINDOW_HASFOCUS;
		}
		if (item->onFocus) {
			Item_RunScript(item, item->onFocus);
		}
		if (item->focusSound) {
			sfx = &item->focusSound;
		}
		if (!(item->window.flags & WINDOW_NOFOCUSSOUND)) {
			playSound = qtrue;
		}
	}

	if (playSound && sfx) {
		// JKG - use the CHAN_AUTO channel to avoid sound stuttering
		//DC->startLocalSound( *sfx, CHAN_LOCAL_SOUND );
		DC->startLocalSound( *sfx, CHAN_AUTO );
	}

	for (i = 0; i < parent->itemCount; i++) {
		if (parent->items[i] == item) {
			parent->cursorItem = i;
			break;
		}
	}

	return qtrue;
}

int Item_TextScroll_MaxScroll ( itemDef_t *item ) 
{
	textScrollDef_t *scrollPtr = item->typeData.textscroll;
	
	int count = scrollPtr->iLineCount;
	int max   = count - (int)(item->window.rect.h / scrollPtr->lineHeight) + 1;

	if (max < 0) 
	{
		return 0;
	}

	return max;
}

int Item_TextScroll_ThumbPosition ( itemDef_t *item ) 
{
	float max, pos, size;
	textScrollDef_t *scrollPtr = item->typeData.textscroll;

	max  = Item_TextScroll_MaxScroll ( item );
	size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;

	if (max > 0) 
	{
		pos = (size-SCROLLBAR_SIZE) / (float) max;
	} 
	else 
	{
		pos = 0;
	}
	
	pos *= scrollPtr->startPos;
	
	return item->window.rect.y + 1 + SCROLLBAR_SIZE + pos;
}

int Item_TextScroll_ThumbDrawPosition ( itemDef_t *item ) 
{
	int min, max;

	if (itemCapture == item) 
	{
		min = item->window.rect.y + SCROLLBAR_SIZE + 1;
		max = item->window.rect.y + item->window.rect.h - 2*SCROLLBAR_SIZE - 1;

		if (DC->cursory >= min + SCROLLBAR_SIZE/2 && DC->cursory <= max + SCROLLBAR_SIZE/2) 
		{
			return DC->cursory - SCROLLBAR_SIZE/2;
		}

		return Item_TextScroll_ThumbPosition(item);
	}

	return Item_TextScroll_ThumbPosition(item);
}

int Item_TextScroll_OverLB ( itemDef_t *item, float x, float y ) 
{
	rectDef_t		r;
	textScrollDef_t *scrollPtr;
	int				thumbstart;
	int				count;

	scrollPtr = item->typeData.textscroll;
	count     = scrollPtr->iLineCount;

	if( item->window.flags & WINDOW_FORCESELECTIONZONE )
	{
		r.x = item->window.selectionZone.x + item->window.selectionZone.w - SCROLLBAR_SIZE;
		r.y = item->window.selectionZone.y;
	}
	else
	{
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		r.y = item->window.rect.y;
	}
	r.h = r.w = SCROLLBAR_SIZE;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_LEFTARROW;
	}

	if( item->window.flags & WINDOW_FORCESELECTIONZONE )
	{
		r.y = item->window.selectionZone.y + item->window.selectionZone.h - SCROLLBAR_SIZE;
	}
	else
	{
		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
	}
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_RIGHTARROW;
	}

	thumbstart = Item_TextScroll_ThumbPosition(item);
	r.y = thumbstart;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_THUMB;
	}

	if( item->window.flags & WINDOW_FORCESELECTIONZONE )
	{
		r.y = item->window.selectionZone.y + SCROLLBAR_SIZE;
	}
	else
	{
		r.y = item->window.rect.y + SCROLLBAR_SIZE;
	}
	r.h = thumbstart - r.y;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_PGUP;
	}

	r.y = thumbstart + SCROLLBAR_SIZE;
	r.h = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
	if (Rect_ContainsPoint(&r, x, y)) 
	{
		return WINDOW_LB_PGDN;
	}

	return 0;
}

void Item_TextScroll_MouseEnter (itemDef_t *item, float x, float y) 
{
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= Item_TextScroll_OverLB(item, x, y);
}

qboolean Item_TextScroll_HandleKey ( itemDef_t *item, int key, qboolean down, qboolean force) 
{
	textScrollDef_t *scrollPtr = item->typeData.textscroll;
	int				max;
	int				viewmax;

	if (force || 
		((item->type != ITEM_TYPE_SLIDER && item->window.flags & WINDOW_FORCESELECTIONZONE && 
		Rect_ContainsPoint(&item->window.selectionZone, DC->cursorx, DC->cursory)) ||
		(Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS) ) )
	{
		max = Item_TextScroll_MaxScroll(item);

		viewmax = (item->window.rect.h / scrollPtr->lineHeight);
		if ( key == A_CURSOR_UP || key == A_KP_8 ) 
		{
			scrollPtr->startPos--;
			if (scrollPtr->startPos < 0)
			{
				scrollPtr->startPos = 0;
			}
			return qtrue;
		}

		if ( key == A_CURSOR_DOWN || key == A_KP_2 ) 
		{
			scrollPtr->startPos++;
			if (scrollPtr->startPos > max)
			{
				scrollPtr->startPos = max;
			}

			return qtrue;
		}

		if ( key == A_MWHEELUP )
		{
			int count = trap->Key_IsDown( A_CTRL ) ? 5 : 1;
			scrollPtr->startPos -= count;
			if (scrollPtr->startPos < 0)
			{
				scrollPtr->startPos = 0;
				Display_MouseMove(NULL, DC->cursorx, DC->cursory);
				return qfalse;
			}
			Display_MouseMove(NULL, DC->cursorx, DC->cursory);
			return qtrue;
		}
		if ( key == A_MWHEELDOWN )
		{
			int count = trap->Key_IsDown( A_CTRL ) ? 5 : 1;
			scrollPtr->startPos += count;
			if (scrollPtr->startPos > max)
			{
				scrollPtr->startPos = max;
				Display_MouseMove(NULL, DC->cursorx, DC->cursory);
				return qfalse;
			}
			Display_MouseMove(NULL, DC->cursorx, DC->cursory);
			return qtrue;
		}

		// mouse hit
		if (key == A_MOUSE1 || key == A_MOUSE2)
		{
			if (item->window.flags & WINDOW_LB_LEFTARROW) 
			{
				scrollPtr->startPos--;
				if (scrollPtr->startPos < 0) 
				{
					scrollPtr->startPos = 0;
				}
			} 
			else if (item->window.flags & WINDOW_LB_RIGHTARROW) 
			{
				// one down
				scrollPtr->startPos++;
				if (scrollPtr->startPos > max) 
				{
					scrollPtr->startPos = max;
				}
			} 
			else if (item->window.flags & WINDOW_LB_PGUP) 
			{
				// page up
				scrollPtr->startPos -= viewmax;
				if (scrollPtr->startPos < 0) 
				{
					scrollPtr->startPos = 0;
				}
			} 
			else if (item->window.flags & WINDOW_LB_PGDN) 
			{
				// page down
				scrollPtr->startPos += viewmax;
				if (scrollPtr->startPos > max) 
				{
					scrollPtr->startPos = max;
				}
			} 
			else if (item->window.flags & WINDOW_LB_THUMB) 
			{
				// Display_SetCaptureItem(item);
			} 

			return qtrue;
		}

		if ( key == A_HOME || key == A_KP_7) 
		{
			// home
			scrollPtr->startPos = 0;
			return qtrue;
		}
		if ( key == A_END || key == A_KP_1) 
		{
			// end
			scrollPtr->startPos = max;
			return qtrue;
		}
		if (key == A_PAGE_UP || key == A_KP_9 ) 
		{
			scrollPtr->startPos -= viewmax;
			if (scrollPtr->startPos < 0) 
			{
					scrollPtr->startPos = 0;
			}

			return qtrue;
		}
		if ( key == A_PAGE_DOWN || key == A_KP_3 ) 
		{
			scrollPtr->startPos += viewmax;
			if (scrollPtr->startPos > max) 
			{
				scrollPtr->startPos = max;
			}
			return qtrue;
		}
	}

	return qfalse;
}

int Item_ListBox_MaxScroll(itemDef_t *item) {
	listBoxDef_t *listPtr = item->typeData.listbox;
	int count = DC->feederCount(item->special);
	int max;

	if (item->window.flags & WINDOW_HORIZONTAL) {
		max = count - (item->window.rect.w / listPtr->elementWidth) + 1;
	}
	else {
		max = count - (item->window.rect.h / listPtr->elementHeight) + 1;
	}
	if (max < 0) {
		return 0;
	}
	return max;
}

int Item_ListBox_ThumbPosition(itemDef_t *item) {
	float max, pos, size;
	listBoxDef_t *listPtr = item->typeData.listbox;

	max = Item_ListBox_MaxScroll(item);
	if (item->window.flags & WINDOW_HORIZONTAL) {
		size = item->window.rect.w - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) {
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} else {
			pos = 0;
		}
		pos *= listPtr->startPos;
		return item->window.rect.x + 1 + SCROLLBAR_SIZE + pos;
	}
	else {
		if( item->window.useNewSlider )
		{
			if( item->window.noSliderButtons )
			{
				if( item->window.floatingSlider )
				{
					size = item->window.sliderBottomY - item->window.sliderTopY - item->window.sliderThumbHeight;
					if ( max > 0 ) {
						pos = size/(float)max;
					}
					else
					{
						pos = 0;
					}
					pos *= listPtr->startPos;
					return item->window.rect.y + (item->window.sliderThumbHeight/2) + pos - 5;
				}
				else
				{
					return 1.0f; // not done
				}
			}
			else
			{
				return 1.0f; // not done
			}
		}
		else
		{
			size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;
			if (max > 0) {
				pos = (size-SCROLLBAR_SIZE) / (float) max;
			} else {
				pos = 0;
			}
			pos *= listPtr->startPos;
			return item->window.rect.y + 1 + SCROLLBAR_SIZE + pos;
		}
	}
}

int Item_ListBox_ThumbDrawPosition(itemDef_t *item) 
{
	int min, max;

	if (itemCapture == item) 
	{
		if (item->window.flags & WINDOW_HORIZONTAL) 
		{
			min = item->window.rect.x + SCROLLBAR_SIZE + 1;
			max = item->window.rect.x + item->window.rect.w - 2*SCROLLBAR_SIZE - 1;
			if (DC->cursorx >= min + SCROLLBAR_SIZE/2 && DC->cursorx <= max + SCROLLBAR_SIZE/2) 
			{
				return DC->cursorx - SCROLLBAR_SIZE/2;
			}
			else 
			{
				return Item_ListBox_ThumbPosition(item);
			}
		}
		else 
		{
			min = item->window.rect.y + SCROLLBAR_SIZE + 1;
			max = item->window.rect.y + item->window.rect.h - 2*SCROLLBAR_SIZE - 1;
			if (DC->cursory >= min + SCROLLBAR_SIZE/2 && DC->cursory <= max + SCROLLBAR_SIZE/2) 
			{
				return DC->cursory - SCROLLBAR_SIZE/2;
			}
			else 
			{
				return Item_ListBox_ThumbPosition(item);
			}
		}
	}
	else 
	{
		return Item_ListBox_ThumbPosition(item);
	}
}

float Item_Slider_ThumbPosition(itemDef_t *item) {
	float value, range, x;
	editFieldDef_t *editDef = item->typeData.edit;

	if (item->text[0]) {
		x = item->textRect.x + item->textRect.w + 8;
	} else {
		x = item->window.rect.x;
	}

	if (editDef == NULL && item->cvar) {
		return x;
	}

	value = DC->getCVarValue(item->cvar);
	if (value < editDef->minVal) {
		value = editDef->minVal;
	} else if (value > editDef->maxVal) {
		value = editDef->maxVal;
	}

	range = editDef->maxVal - editDef->minVal;
	value -= editDef->minVal;
	value /= range;
	//value /= (editDef->maxVal - editDef->minVal);
	if( item->window.rect.w && item->window.rect.h )
	{
		value *= item->window.rect.w;
	}
	else
	{
		value *= SLIDER_WIDTH;
	}
	x += value;
	// vm fuckage
	//x = x + (((float)value / editDef->maxVal) * SLIDER_WIDTH);
	return x;
}

int Item_Slider_OverSlider(itemDef_t *item, float x, float y) {
	rectDef_t r;

	r.x = Item_Slider_ThumbPosition(item) - (SLIDER_THUMB_WIDTH / 2);
	r.y = item->window.rect.y - 2;
	if(item->window.selectionZone.w)
	{
		r.w = item->window.selectionZone.w;
	}
	else
	{
		r.w = SLIDER_THUMB_WIDTH;
	}

	if(item->window.selectionZone.h)
	{
		r.h = item->window.selectionZone.h;
	}
	else
	{
		r.h = SLIDER_THUMB_HEIGHT;
	}
	//r.w = SLIDER_THUMB_WIDTH;
	//r.h = SLIDER_THUMB_HEIGHT;

	if (Rect_ContainsPoint(&r, x, y)) {
		return WINDOW_LB_THUMB;
	}
	return 0;
}

int Item_ListBox_OverLB(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	listBoxDef_t *listPtr;
	int thumbstart;
	int count;

	count = DC->feederCount(item->special);
	listPtr = item->typeData.listbox;
	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		// check if on left arrow
		r.x = item->window.rect.x;
		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		r.h = r.w = SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			return WINDOW_LB_LEFTARROW;
		}

		// check if on right arrow
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			return WINDOW_LB_RIGHTARROW;
		}

		// check if on thumb
		thumbstart = Item_ListBox_ThumbPosition(item);
		r.x = thumbstart;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			return WINDOW_LB_THUMB;
		}

		r.x = item->window.rect.x + SCROLLBAR_SIZE;
		r.w = thumbstart - r.x;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			return WINDOW_LB_PGUP;
		}

		r.x = thumbstart + SCROLLBAR_SIZE;
		r.w = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			return WINDOW_LB_PGDN;
		}
	}
	// Vertical Scroll
	else 
	{
		// Multiple rows and columns (since it's more than twice as wide as an element)
		if (( item->window.rect.w > (listPtr->elementWidth*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
		{
			r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
			r.y = item->window.rect.y;
			r.h = r.w = SCROLLBAR_SIZE;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_PGUP;
			}

			r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_PGDN;
			}

			thumbstart = Item_ListBox_ThumbPosition(item);
			r.y = thumbstart;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_THUMB;
			}

		}
		else
		{
			r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
			r.y = item->window.rect.y;
			r.h = r.w = SCROLLBAR_SIZE;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_LEFTARROW;
			}

			r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_RIGHTARROW;
			}

			thumbstart = Item_ListBox_ThumbPosition(item);
			r.y = thumbstart;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_THUMB;
			}

			r.y = item->window.rect.y + SCROLLBAR_SIZE;
			r.h = thumbstart - r.y;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_PGUP;
			}

			r.y = thumbstart + SCROLLBAR_SIZE;
			r.h = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
			if (Rect_ContainsPoint(&r, x, y)) 
			{
				return WINDOW_LB_PGDN;
			}
		}
	}
	return 0;
}


void Item_ListBox_MouseEnter(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	listBoxDef_t *listPtr = item->typeData.listbox;
        
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= Item_ListBox_OverLB(item, x, y);

	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
		if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
		{
			r.x = item->window.rect.x;
			r.y = item->window.rect.y;
			r.h = item->window.rect.h - SCROLLBAR_SIZE;
			r.w = item->window.rect.w - listPtr->drawPadding;
			// check for selection hit as we have exausted buttons and thumb
			if (listPtr->elementStyle == LISTBOX_IMAGE) 
			{
				if (Rect_ContainsPoint(&r, x, y)) 
				{
					//Jedi Knight Galaxies add: Check for multiple columns and rows
					if (( item->window.rect.h > (listPtr->elementHeight*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
					{
						int row,column,rowLength;

						row = (int)((x - 2 - r.x) / (listPtr->elementWidth+listPtr->elementSpacingW));
						rowLength = (int) r.h / listPtr->elementHeight;
						column = (int)((y - r.y) / (listPtr->elementHeight+listPtr->elementSpacingH));

						listPtr->cursorPos = (row * rowLength)+column  + listPtr->startPos;
						if (listPtr->cursorPos >= listPtr->endPos) 
						{
							listPtr->cursorPos = listPtr->endPos;
						}
					}
					else
					{
						listPtr->cursorPos =  (int)((x - r.x) / listPtr->elementWidth)  + listPtr->startPos;
						if (listPtr->cursorPos >= listPtr->endPos) 
						{
							listPtr->cursorPos = listPtr->endPos;
						}
					}
				}
			} 
			else 
			{
				// text hit.. 
			}
		}
	} 
	// Window Vertical Scroll
	else if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) 
	{
		// Calc which element the mouse is over
		r.x = item->window.rect.x;
		r.y = item->window.rect.y;
		r.w = item->window.rect.w - SCROLLBAR_SIZE;
		r.h = item->window.rect.h - listPtr->drawPadding;
		if (Rect_ContainsPoint(&r, x, y)) 
		{
			// Multiple rows and columns (since it's more than twice as wide as an element)
			if (( item->window.rect.w > (listPtr->elementWidth*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
			{
				int row,column,rowLength;

				row = (int)((y - 2 - r.y) / listPtr->elementHeight);
				rowLength = (int) r.w / listPtr->elementWidth;
				column = (int)((x - r.x) / listPtr->elementWidth);

				listPtr->cursorPos = (row * rowLength)+column  + listPtr->startPos;
				if (listPtr->cursorPos >= listPtr->endPos) 
				{
					listPtr->cursorPos = listPtr->endPos;
				}
			}
			// single column
			else
			{
				listPtr->cursorPos =  (int)((y - 2 - r.y) / listPtr->elementHeight)  + listPtr->startPos;
				if (listPtr->cursorPos > listPtr->endPos) 
				{
                    listPtr->cursorPos = listPtr->endPos;
				}
			}
		}
	}
}

void Item_MouseEnter(itemDef_t *item, float x, float y) {

	rectDef_t r;
	if (item) {
		r = item->textRect;
		r.y -= r.h;
		// in the text rect?
		if( item->window.flags & WINDOW_FORCESELECTIONZONE )
		{
			r = item->window.selectionZone;

		}

		// items can be enabled and disabled 
		if (item->disabled) 
		{
			return;
		}

		// items can be enabled and disabled based on cvars
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
			return;
		}

		if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) {
			return;
		}

		if (Rect_ContainsPoint(&r, x, y)) 
		{
			if (!(item->window.flags & WINDOW_MOUSEOVERTEXT)) {
				Item_RunScript(item, item->mouseEnterText);
				item->window.flags |= WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) {
				Item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

		} else {
			// not in the text rect
			if (item->window.flags & WINDOW_MOUSEOVERTEXT) {
				// if we were
				Item_RunScript(item, item->mouseExitText);
				item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) {
				Item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

			if (item->type == ITEM_TYPE_LISTBOX) {
				Item_ListBox_MouseEnter(item, x, y);
			}
			else if ( item->type == ITEM_TYPE_TEXTSCROLL )
			{
				Item_TextScroll_MouseEnter ( item, x, y );
			}
		}
	}
}

void Item_MouseLeave(itemDef_t *item) {
  if (item) {
    if (item->window.flags & WINDOW_MOUSEOVERTEXT) {
      Item_RunScript(item, item->mouseExitText);
      item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
    }
    Item_RunScript(item, item->mouseExit);
    item->window.flags &= ~(WINDOW_LB_RIGHTARROW | WINDOW_LB_LEFTARROW);
  }
}

itemDef_t *Menu_HitTest(menuDef_t *menu, float x, float y) {
  int i;
  for (i = 0; i < menu->itemCount; i++) {
    if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
      return menu->items[i];
    }
  }
  return NULL;
}

void Item_SetMouseOver(itemDef_t *item, qboolean focus) {
  if (item) {
    if (focus) {
      item->window.flags |= WINDOW_MOUSEOVER;
    } else {
      item->window.flags &= ~WINDOW_MOUSEOVER;
    }
  }
}


qboolean Item_OwnerDraw_HandleKey(itemDef_t *item, int key) 
{
	if (item && DC->ownerDrawHandleKey)
	{
		return DC->ownerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key, item->window.ownerDrawID);
	}

	return qfalse;
}

qboolean Item_ListBox_HandleKey(itemDef_t *item, int key, qboolean down, qboolean force) {
	listBoxDef_t *listPtr = item->typeData.listbox;
	int count = DC->feederCount(item->special);
	int max, viewmax;
	if (force || (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && (item->window.flags & WINDOW_HASFOCUS || !strcmp(item->window.name, "inventory_feederstuff"))))
	{
		max = Item_ListBox_MaxScroll(item);
		if (item->window.flags & WINDOW_HORIZONTAL) { //Horizontal scroll
			viewmax = (item->window.rect.w / listPtr->elementWidth);
			if ( key == A_CURSOR_LEFT || key == A_KP_4 ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) {
						listPtr->cursorPos = 0;
						return qfalse;
					}
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
						return qfalse;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos, NULL);
				}
				else {
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return qtrue;
			}
			else if ( key == A_CURSOR_RIGHT || key == A_KP_6 ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
						return qfalse;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
						return qfalse;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos, NULL);
				}
				else {
					listPtr->startPos++;
					if (listPtr->startPos >= count)
						listPtr->startPos = count-1;
				}
				return qtrue;
			}
			// Fucking cgame linkage...piece of shit.
#ifdef UI
			else if( item->window.name && !Q_stricmp(item->window.name, "inventory_feederstuff") )
			{
				if( key == A_MWHEELUP )
				{
					JKG_Inventory_Arrow_New(item, 1);
					return qtrue;
				}
				else if( key == A_MWHEELDOWN )
				{
					JKG_Inventory_Arrow_New(item, -1);
					return qtrue;
				}
			}
#endif
		}
		// Vertical scroll
		else {

			// Multiple rows and columns (since it's more than twice as wide as an element)
			if (( item->window.rect.w > (listPtr->elementWidth*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
			{
				viewmax = (item->window.rect.w / listPtr->elementWidth);
			}
			else 
			{
				viewmax = (item->window.rect.h / listPtr->elementHeight);
			}

			if ( key == A_CURSOR_UP || key == A_KP_8 ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) {
						listPtr->cursorPos = 0;
						return qfalse;
					}
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
						return qfalse;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos, NULL);
				}
				else {
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return qtrue;
			}
			if ( key == A_CURSOR_DOWN || key == A_KP_2 ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
						return qfalse;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
						return qfalse;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos, NULL);
				}
				else {
					listPtr->startPos++;
					if (listPtr->startPos > max)
						listPtr->startPos = max;
				}
				return qtrue;
			}

			//Raz: Added
			if ( key == A_MWHEELUP ) 
			{
				int count = trap->Key_IsDown( A_CTRL ) ? 5 : 1;
				listPtr->startPos -= ((int)item->special == FEEDER_Q3HEADS) ? viewmax : count;
				if (listPtr->startPos < 0)
				{
					listPtr->startPos = 0;
					Display_MouseMove(NULL, DC->cursorx, DC->cursory);
					return qfalse;
				}
				Display_MouseMove(NULL, DC->cursorx, DC->cursory);
				return qtrue;
			}
			if ( key == A_MWHEELDOWN ) 
			{
				int count = trap->Key_IsDown( A_CTRL ) ? 5 : 1;
				listPtr->startPos += ((int)item->special == FEEDER_Q3HEADS) ? viewmax : count;
				if (listPtr->startPos > max)
				{
					listPtr->startPos = max;
					Display_MouseMove(NULL, DC->cursorx, DC->cursory);
					return qfalse;
				}
				Display_MouseMove(NULL, DC->cursorx, DC->cursory);
				return qtrue;
			}
		}
		// mouse hit
		if (key == A_MOUSE1 || key == A_MOUSE2) {
			if (item->window.flags & WINDOW_LB_LEFTARROW) {
				listPtr->startPos--;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			} else if (item->window.flags & WINDOW_LB_RIGHTARROW) {
				// one down
				listPtr->startPos++;
				if (listPtr->startPos > max) {
					listPtr->startPos = max;
				}
			} else if (item->window.flags & WINDOW_LB_PGUP) {
				// page up
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			} else if (item->window.flags & WINDOW_LB_PGDN) {
				// page down
				listPtr->startPos += viewmax;
				if (listPtr->startPos > (max)) {
					listPtr->startPos = (max);
				}
			} else if (item->window.flags & WINDOW_LB_THUMB) {
				// Display_SetCaptureItem(item);
			} else {
				// select an item
				if (DC->realTime < lastListBoxClickTime && listPtr->doubleClick) {
					Item_RunScript(item, listPtr->doubleClick);
				}
				lastListBoxClickTime = DC->realTime + DOUBLE_CLICK_DELAY;
//				if (item->cursorPos != listPtr->cursorPos) 
				{
					int prePos = item->cursorPos;
					
					item->cursorPos = listPtr->cursorPos;

					if (!DC->feederSelection(item->special, item->cursorPos, item))
					{
						item->cursorPos = listPtr->cursorPos = prePos;
					}
				}
			}
			return qtrue;
		}
		if ( key == A_HOME || key == A_KP_7) {
			// home
			listPtr->startPos = 0;
			return qtrue;
		}
		if ( key == A_END || key == A_KP_1) {
			// end
			listPtr->startPos = max;
			return qtrue;
		}
		if (key == A_PAGE_UP || key == A_KP_9 ) {
			// page up
			if (!listPtr->notselectable) {
				listPtr->cursorPos -= viewmax;
				if (listPtr->cursorPos < 0) {
					listPtr->cursorPos = 0;
				}
				if (listPtr->cursorPos < listPtr->startPos) {
					listPtr->startPos = listPtr->cursorPos;
				}
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				}
				item->cursorPos = listPtr->cursorPos;
				DC->feederSelection(item->special, item->cursorPos, NULL);
			}
			else {
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			}
			return qtrue;
		}
		if ( key == A_PAGE_DOWN || key == A_KP_3 ) {
			// page down
			if (!listPtr->notselectable) {
				listPtr->cursorPos += viewmax;
				if (listPtr->cursorPos < listPtr->startPos) {
					listPtr->startPos = listPtr->cursorPos;
				}
				if (listPtr->cursorPos >= count) {
					listPtr->cursorPos = count-1;
				}
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				}
				item->cursorPos = listPtr->cursorPos;
				DC->feederSelection(item->special, item->cursorPos, NULL);
			}
			else {
				listPtr->startPos += viewmax;
				if (listPtr->startPos > max) {
					listPtr->startPos = max;
				}
			}
			return qtrue;
		}
	}
	return qfalse;
}

qboolean Item_YesNo_HandleKey(itemDef_t *item, int key) {
  if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS && item->cvar) 
	{
		if (key == A_MOUSE1 || key == A_ENTER || key == A_MOUSE2 || key == A_MOUSE3) 
		{
	    DC->setCVar(item->cvar, va("%i", !DC->getCVarValue(item->cvar)));
		  return qtrue;
		}
  }

  return qfalse;

}

int Item_Multi_CountSettings(itemDef_t *item) {
	multiDef_t *multiPtr = item->typeData.multi;
	if (multiPtr == NULL) {
		return 0;
	}
	return multiPtr->count;
}

int Item_Multi_FindCvarByValue(itemDef_t *item) {
	char buff[2048];
	float value = 0;
	int i;
	multiDef_t *multiPtr = item->typeData.multi;
	if (multiPtr) {
		if (multiPtr->strDef) {
	    DC->getCVarString(item->cvar, buff, sizeof(buff));
		} else {
			value = DC->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) {
			if (multiPtr->strDef) {
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) {
					return i;
				}
			} else {
 				if (multiPtr->cvarValue[i] == value) {
 					return i;
 				}
 			}
 		}
	}
	return 0;
}

const char *Item_Multi_Setting(itemDef_t *item) {
	char buff[2048];
	float value = 0;
	int i;
	multiDef_t *multiPtr = item->typeData.multi;
	if (multiPtr) 
	{
		if (multiPtr->strDef) 
		{
			if (item->cvar)
			{
			    DC->getCVarString(item->cvar, buff, sizeof(buff));
			}
		} 
		else 
		{
			if (item->cvar)	// Was a cvar given?
			{
				value = DC->getCVarValue(item->cvar);
			}
		}

		for (i = 0; i < multiPtr->count; i++) 
		{
			if (multiPtr->strDef) 
			{
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) 
				{
					return multiPtr->cvarList[i];
				}
			} 
			else 
			{
 				if (multiPtr->cvarValue[i] == value) 
				{
					return multiPtr->cvarList[i];
 				}
 			}
 		}
	}
	return "";
}

qboolean Item_Multi_HandleKey(itemDef_t *item, int key) 
{
	multiDef_t *multiPtr = item->typeData.multi;
	if (multiPtr) 
	{
		if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS) 
		{
			//Raz: Scroll on multi buttons!
			if (key == A_MOUSE1 || key == A_ENTER || key == A_MOUSE2 || key == A_MOUSE3 || key == A_MWHEELDOWN || key == A_MWHEELUP) 
//			if (key == A_MOUSE1 || key == A_ENTER || key == A_MOUSE2 || key == A_MOUSE3) 
			{
				int current = Item_Multi_FindCvarByValue(item);
				int max = Item_Multi_CountSettings(item);

				if (key == A_MOUSE2 || key == A_CURSOR_LEFT || key == A_MWHEELDOWN)	// Xbox uses CURSOR_LEFT
			//	if (key == A_MOUSE2 || key == A_CURSOR_LEFT)	// Xbox uses CURSOR_LEFT
				{
					current--;
					if ( current < 0 ) 
					{
						current = max-1;
					}
				}
				else
				{
					current++;
					if ( current >= max ) 
					{
						current = 0;
					}
				}

				if (multiPtr->strDef) 
				{
					DC->setCVar(item->cvar, multiPtr->cvarStr[current]);
				} 
				else 
				{
					float value = multiPtr->cvarValue[current];
					if (((float)((int) value)) == value) 
					{
						DC->setCVar(item->cvar, va("%i", (int) value ));
					}
					else 
					{
						DC->setCVar(item->cvar, va("%f", value ));
					}
				}
				if (item->special)
				{//its a feeder?
					DC->feederSelection(item->special, current, item);
				}

				return qtrue;
			}
		}
	}
  return qfalse;
}

// Leaving an edit field so reset it so it prints from the beginning.
void Leaving_EditField(itemDef_t *item)
{
	// switching fields so reset printed text of edit field
	if ((g_editingField==qtrue) && (item->type==ITEM_TYPE_EDITFIELD))
	{
		editFieldDef_t *editPtr = item->typeData.edit;
		if (editPtr)
		{
			editPtr->paintOffset = 0;
		}
	}
}

/* JKG - This way we can set focus to textboxes through code */
void Enter_EditField(itemDef_t *item)
{
	if (g_editingField) {
		Leaving_EditField(g_editItem);
		g_editingField = qfalse;
	}

	if (item && (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD))
	{
		g_editItem = item;
		g_editingField = qtrue;
	}
}


// JKG - TextField items are fully recoded to actually work good

static float Text_GetWidth(const char *text, int iFontIndex, float scale) {
	return trap->R_Font_StrLenPixels(text, iFontIndex, scale);
	#if 0
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
			if (*(t + 1) == 'x' || *(t + 1) == 'X') {
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
	#endif
}

void Text_DrawText(int x, int y, const char *text, const float* rgba, int iFontIndex, const int limit, float scale) {
	trap->R_Font_DrawString(	x,		// int ox
								y,		// int oy
								text,	// const char *text
								rgba,	// paletteRGBA_c c
								iFontIndex,	// const int iFontHandle
								!limit?-1:limit,		// iCharLimit (-1 = none)
								scale	// const float scale = 1.0f
							);
#if 0	
	// Custom draw algo to ensure proper spacing in compliance with Text_GetWidth
	char s[2];
	const char *t = (char *)text;
	vec4_t color;
	float xx = x;

	s[1] = 0;
	VectorCopy4M(rgba, color);
	while (*t) {
		if (*t == '^') {
			if (*(t+1) >= '0' && *(t+1) <= '9') {
				VectorCopy4M(tColorTable[*(t+1) - '0'], color);
				t+=2;
				continue;
			}
			if (*(t + 1) == 'x' || *(t + 1) == 'X') {
				// Extended colorcode
				if (Text_ExtColorCodes(t+1, color)) {
					t+=5;
					continue;
				}
			}
		}
		s[0] = *t;
		trap->R_Font_DrawString(xx, y, s, color, iFontIndex, limit, scale);
		xx += ((float)trap->R_Font_StrLenPixels(s, iFontIndex & 0xFFFF, 1) * scale);
		t++;
	}
#endif
}

static const char *Text_PrintableText(const char *buff, itemDef_t *item) {
	static char buff2[512];
	char *u;
	const char *t;
	char s[2];
	float w = 0;
	int maxwidth;
	editFieldDef_t *editPtr = item->typeData.edit;

	maxwidth = item->window.rect.w - 8;
	t = (char *)&buff[editPtr->paintOffset];
	u = &buff2[0];

	s[1] = 0;

	while (*t) {
		int colorLen = Q_parseColorString( t, nullptr );
		if ( colorLen ) {
			for( int i = 0; i < colorLen; i++ )
			{
				*u = *t;
				u++; t++;
			}
			continue;
		}
		s[0] = *t;

		w += (trap->R_Font_StrLenPixels(s, item->iMenuFont, 1) * item->textscale);
		if (w > maxwidth) {
			break;
		}
		*u = *t;
		t++; u++;
	}
	*u = 0;
	return &buff2[0];
}

static void Item_TextField_UpdateScroll(itemDef_t *item) {
	char buff[2048];
	float w = 0;		// Width scroll-cursor
	float wx = 0;		// Width scroll-end of string
	float wf = 0;		// Fraction w/max width
	float wxf = 0;		// Fraction wx/max width
	int i;
	char s[2];
	float maxwidth;
	editFieldDef_t *editPtr = item->typeData.edit;

	s[1] = 0;

	if (item->cvar) {
		buff[0] = 0;
		DC->getCVarString(item->cvar, buff, sizeof(buff));
	} else {
		Q_strncpyz(buff, editPtr->buffer,sizeof(buff));
	}

	maxwidth = item->window.rect.w - 8;

	if (item->cursorPos < editPtr->paintOffset) {
		// Odd condition
		for (i = item->cursorPos; i> editPtr->paintOffset; i--) {
			s[0] = buff[i];
			w += Text_GetWidth(s, MenuFontToHandle(item->iMenuFont), item->textscale);
		}
		w *= -1; // Invert, since we gotta go back a lot :P
	} else {
		for (i = editPtr->paintOffset; i < item->cursorPos; i++) {
			s[0] = buff[i];
			w += Text_GetWidth(s, MenuFontToHandle(item->iMenuFont), item->textscale);
		}
	}

	wf = w / maxwidth;

	if (wf < 0.25) {
		// Under 25%, try to scroll backward
		while (1) {
			if (editPtr->paintOffset == 0 || wf >= 0.25) {
				// Cant go any further, or we're back on track
				return;
			}
			// Go back 1 character
			editPtr->paintOffset--;
			s[0] = buff[editPtr->paintOffset];
			w += Text_GetWidth(s, MenuFontToHandle(item->iMenuFont), item->textscale);
			wf = w / maxwidth;
		}
	} else if (wf > 0.75) {
		// Above 75%, try scrolling forward
		wx = Text_GetWidth(&buff[editPtr->paintOffset], MenuFontToHandle(item->iMenuFont), item->textscale);
		wxf	= wx / maxwidth;
		while (1) {
			float wid;
			if (wf <= 0.75 || wxf < 1) {
				// Cant go any further, or we're back on track
				return;
			}
			// Go forward 1 character
			editPtr->paintOffset++;
			s[0] = buff[editPtr->paintOffset-1];
			wid = Text_GetWidth(s, MenuFontToHandle(item->iMenuFont), item->textscale);
			w -= wid;
			wf = w / maxwidth;

			wx -= wid;
			wxf	= wx / maxwidth;
		}
	}
	// All done now
}


qboolean Item_TextField_HandleKey(itemDef_t *item, int key) {
	char buff[2048];
	int len;
	itemDef_t *newItem = NULL;
	editFieldDef_t *editPtr = item->typeData.edit;

	if (!editPtr) {
		return qfalse;
	}

	// JKGalaxies, give textboxes an internal data storage
	if (item->cvar) {
		buff[0] = 0;
		DC->getCVarString(item->cvar, buff, sizeof(buff));
	} else {
		Q_strncpyz(buff, editPtr->buffer,sizeof(buff));
	}
	len = strlen(buff);

	// 	 We need to know the actual length, do dont pretend it's lower if it's not

	/*if (editPtr->maxChars && len > editPtr->maxChars) {
		len = editPtr->maxChars;
	} */

	if ( key & K_CHAR_FLAG ) {
		key &= ~K_CHAR_FLAG;

		// Failsafe, should never happen, but just in case
		if (item->cursorPos > len) 
			item->cursorPos = len;
		if (key == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
			if ( item->cursorPos > 0 ) {
				memmove( &buff[item->cursorPos - 1], &buff[item->cursorPos], len + 1 - item->cursorPos);
				item->cursorPos--;
				Item_TextField_UpdateScroll(item);
			}
			if (item->cvar)
				DC->setCVar(item->cvar, buff);
			else
				Q_strncpyz(editPtr->buffer, buff, sizeof(buff));
    		return qtrue;
		}


		//
		// ignore any non printable chars
		//
			if ( key < 32 || !item->cvar) {
			    return qtrue;
		    }

		if (item->type == ITEM_TYPE_NUMERICFIELD) {
			if (key == '-')	{
				if (!(item->flags & ITF_TEXTFIELD_ALLOWNEGATIVE) || item->cursorPos > 0) {
					return qtrue;		// Only allow a minus at the beginning of the number
				}
			} else if (key == '.') {
				if (!(item->flags & ITF_TEXTFIELD_ALLOWPERIOD) || strstr(buff, ".")) {
					return qtrue;		// Only allow one single period
				}
			} else if (key < '0' || key > '9') {
				return qtrue;			// If the key is not a number (or one of the above), ignore it
			}
			if (buff[0] == '-' && item->cursorPos == 0 && !DC->getOverstrikeMode()) {
				// Prevent inserting characters before the minus sign
				return qtrue;
			}
		}

		if (!DC->getOverstrikeMode()) {
			if ( len == (MAX_EDITFIELD - 1) ||
					(editPtr->maxChars && len >= editPtr->maxChars) ) {
				// Filled up, ignore further input
				return qtrue;
			}
			if (len != item->cursorPos)
				memmove( &buff[item->cursorPos + 1], &buff[item->cursorPos], len + 1 - item->cursorPos );
		} else {
			if ( len == (MAX_EDITFIELD - 1) ||
					((editPtr->maxChars && item->cursorPos >= editPtr->maxChars) &&
					 item->cursorPos == len) ) {
				// Filled up, ignore further input
				return qtrue;
			}
		}

		buff[item->cursorPos] = (char)key;

		//rww - nul-terminate!
		if (item->cursorPos == len) {
			if (item->cursorPos+1 < 2048)
			{
				buff[item->cursorPos+1] = 0;
			}
			else
			{
				buff[item->cursorPos] = 0;
			}
		}
		if (item->cvar)
			DC->setCVar(item->cvar, buff);
		else
			Q_strncpyz(editPtr->buffer, buff, sizeof(buff));

		if (item->cursorPos <= len) {
			item->cursorPos++;
			Item_TextField_UpdateScroll(item);
		}

	} else {

		if ( key == A_DELETE || key == A_KP_PERIOD ) {
			if ( item->cursorPos < len ) {
				memmove( buff + item->cursorPos, buff + item->cursorPos + 1, len - item->cursorPos);
				Item_TextField_UpdateScroll(item);
				if (item->cvar)
					DC->setCVar(item->cvar, buff);
				else
					Q_strncpyz(editPtr->buffer, buff, sizeof(buff));
			}
			return qtrue;
		}

		if ( key == A_CURSOR_RIGHT || key == A_KP_6 ) 
		{
			/*if (editPtr->maxPaintChars && item->cursorPos >= editPtr->maxPaintChars && item->cursorPos < len) {
				item->cursorPos++;
				editPtr->paintOffset++;
				return qtrue;
			}
			if (item->cursorPos < len) {
				item->cursorPos++;
			}*/
			if (item->cursorPos < len) {
				int colorLen = Q_parseColorString( &buff[item->cursorPos+1], nullptr );
				if( colorLen )
					item->cursorPos += (colorLen + 1);
				else
					item->cursorPos++;
				Item_TextField_UpdateScroll(item);
			} 
			return qtrue;
		}

		if ( key == A_CURSOR_LEFT || key == A_KP_4 ) 
		{
			if ( item->cursorPos > 1 ) {
				int colorLen = Q_parseColorString( &buff[item->cursorPos-2], nullptr );
				if( colorLen )
					item->cursorPos -= (colorLen + 1);
				else
					item->cursorPos++;
			} else if (item->cursorPos > 0) {
				item->cursorPos--;
			}
			Item_TextField_UpdateScroll(item);
			return qtrue;
		}

		if ( key == A_HOME || key == A_KP_7) {// || ( tolower(key) == 'a' && trap->Key_IsDown( K_CTRL ) ) ) {
			item->cursorPos = 0;
			editPtr->paintOffset = 0;
			return qtrue;
		}

		if ( key == A_END || key == A_KP_1)  {// ( tolower(key) == 'e' && trap->Key_IsDown( K_CTRL ) ) ) {
			item->cursorPos = len;
			Item_TextField_UpdateScroll(item);
			return qtrue;
		}

		if ( key == A_INSERT || key == A_KP_0 ) {
			DC->setOverstrikeMode(!DC->getOverstrikeMode());
			return qtrue;
		}
	}

	if (key == A_TAB || key == A_CURSOR_DOWN || key == A_KP_2)
	{
		// switching fields so reset printed text of edit field
		Leaving_EditField(item);
		g_editingField = qfalse;
		newItem = Menu_SetNextCursorItem((menuDef_t *) item->parent);
		if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD))
		{
			g_editItem = newItem;
			g_editingField = qtrue;
		}
	}

	if (key == A_CURSOR_UP || key == A_KP_8)
	{
		// switching fields so reset printed text of edit field
		Leaving_EditField(item);

		g_editingField = qfalse;
		newItem = Menu_SetPrevCursorItem((menuDef_t *) item->parent);
		if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD))
		{
			g_editItem = newItem;
			g_editingField = qtrue;
		}
	}

		/* Raz: Don't get stuck in overstrike/insert mode
		if ( key == A_ENTER || key == A_KP_ENTER || key == A_ESCAPE)  {
			*/
		if ( key == A_ENTER || key == A_KP_ENTER || key == A_ESCAPE || (key == A_MOUSE1 && !Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) )) {
			DC->setOverstrikeMode( qfalse );
		return qfalse;
	}
	
	if (key == A_ESCAPE)  {
		if (item->onEscape) {
			Item_RunScript(item, item->onEscape);
		}
		return qfalse;
	}

	return qtrue;
}

static void Scroll_TextScroll_AutoFunc (void *p) 
{
	scrollInfo_t *si = (scrollInfo_t*)p;

	if (DC->realTime > si->nextScrollTime) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_TextScroll_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) 
	{
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) 
		{
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_TextScroll_ThumbFunc(void *p) 
{
	scrollInfo_t *si = (scrollInfo_t*)p;
	rectDef_t	 r;
	int			 pos;
	int			 max;

	textScrollDef_t *scrollPtr = si->item->typeData.textscroll;

	if (DC->cursory != si->yStart) 
	{
		r.x = si->item->window.rect.x + si->item->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = si->item->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = si->item->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = Item_TextScroll_MaxScroll(si->item);
		//
		pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		if (pos < 0) 
		{
			pos = 0;
		}
		else if (pos > max) 
		{
			pos = max;
		}

		scrollPtr->startPos = pos;
		si->yStart = DC->cursory;
	}

	if (DC->realTime > si->nextScrollTime) 
	{ 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_TextScroll_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) 
	{
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) 
		{
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_ListBox_AutoFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_ListBox_ThumbFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	rectDef_t r;
	int pos, max;

	listBoxDef_t *listPtr = si->item->typeData.listbox;
	if (si->item->window.flags & WINDOW_HORIZONTAL) {
		if (DC->cursorx == si->xStart) {
			return;
		}
		r.x = si->item->window.rect.x + SCROLLBAR_SIZE + 1;
		r.y = si->item->window.rect.y + si->item->window.rect.h - SCROLLBAR_SIZE - 1;
		r.h = SCROLLBAR_SIZE;
		r.w = si->item->window.rect.w - (SCROLLBAR_SIZE*2) - 2;
		max = Item_ListBox_MaxScroll(si->item);
		//
		pos = (DC->cursorx - r.x - SCROLLBAR_SIZE/2) * max / (r.w - SCROLLBAR_SIZE);
		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->xStart = DC->cursorx;
	}
	else if (DC->cursory != si->yStart) {

		r.x = si->item->window.rect.x + si->item->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = si->item->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = si->item->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = Item_ListBox_MaxScroll(si->item);

		// Multiple rows and columns (since it's more than twice as wide as an element)
		if (( si->item->window.rect.w > (listPtr->elementWidth*2)) &&  (listPtr->elementStyle == LISTBOX_IMAGE))
		{
			int rowLength, rowMax;
			rowLength = si->item->window.rect.w / listPtr->elementWidth; 
			rowMax = max / rowLength;

			pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * rowMax / (r.h - SCROLLBAR_SIZE);
			pos *= rowLength;
		}
		else
		{
			pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		}

		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->yStart = DC->cursory;
	}

	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_Slider_ThumbFunc(void *p) {
	float x, value, cursorx;
	scrollInfo_t *si = (scrollInfo_t*)p;
	editFieldDef_t *editDef = si->item->typeData.edit;

	if (si->item->text[0]) {
		x = si->item->textRect.x + si->item->textRect.w + 8;
	} else {
		x = si->item->window.rect.x;
	}

	cursorx = DC->cursorx;

	if( si->item->window.rect.w && si->item->window.rect.h )
	{
		if (cursorx < x) {
			cursorx = x;
		} else if (cursorx > x + si->item->window.rect.w) {
			cursorx = x + si->item->window.rect.w;
		}
	}
	else
	{
		if (cursorx < x) {
			cursorx = x;
		} else if (cursorx > x + SLIDER_WIDTH) {
			cursorx = x + SLIDER_WIDTH;
		}
	}
	value = cursorx - x;
	if( si->item->window.rect.w )
	{
		value /= si->item->window.rect.w;
	}
	else
	{
		value /= SLIDER_WIDTH;
	}
	value *= (editDef->maxVal - editDef->minVal);
	value += editDef->minVal;
	DC->setCVar(si->item->cvar, va("%f", value));
}

void Item_StartCapture(itemDef_t *item, int key) 
{
	int flags;
	switch (item->type) 
	{
	    case ITEM_TYPE_EDITFIELD:
		case ITEM_TYPE_NUMERICFIELD:
		case ITEM_TYPE_LISTBOX:
		{
			flags = Item_ListBox_OverLB(item, DC->cursorx, DC->cursory);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) {
				scrollInfo.nextScrollTime = DC->realTime + SCROLL_TIME_START;
				scrollInfo.nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
				scrollInfo.adjustValue = SCROLL_TIME_START;
				scrollInfo.scrollKey = key;
				scrollInfo.scrollDir = (flags & WINDOW_LB_LEFTARROW) ? qtrue : qfalse;
				scrollInfo.item = item;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_AutoFunc;
				itemCapture = item;
			} else if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_ThumbFunc;
				itemCapture = item;
			}
			break;
		}

		case ITEM_TYPE_TEXTSCROLL:
			flags = Item_TextScroll_OverLB (item, DC->cursorx, DC->cursory);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) 
			{
				scrollInfo.nextScrollTime = DC->realTime + SCROLL_TIME_START;
				scrollInfo.nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
				scrollInfo.adjustValue = SCROLL_TIME_START;
				scrollInfo.scrollKey = key;
				scrollInfo.scrollDir = (flags & WINDOW_LB_LEFTARROW) ? qtrue : qfalse;
				scrollInfo.item = item;
				captureData = &scrollInfo;
				captureFunc = &Scroll_TextScroll_AutoFunc;
				itemCapture = item;
			} 
			else if (flags & WINDOW_LB_THUMB) 
			{
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_TextScroll_ThumbFunc;
				itemCapture = item;
			}
			break;

		case ITEM_TYPE_SLIDER:
		{
			flags = Item_Slider_OverSlider(item, DC->cursorx, DC->cursory);
			if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_Slider_ThumbFunc;
				itemCapture = item;
			}
			break;
		}
	}
}

void Item_StopCapture(itemDef_t *item) {

}

qboolean Item_Slider_HandleKey(itemDef_t *item, int key, qboolean down) {
	float x, value, width, work;

	//DC->Print("slider handle key\n");
//JLF MPMOVED
	if (item->window.flags & WINDOW_HASFOCUS && item->cvar && Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
		if (key == A_MOUSE1 || key == A_ENTER || key == A_MOUSE2 || key == A_MOUSE3) {
			editFieldDef_t *editDef = item->typeData.edit;
			if (editDef) {
				rectDef_t testRect;
				float thumbWidth;
				if( item->window.rect.w )
				{
					width = item->window.rect.w;
				}
				else
				{
					width = SLIDER_WIDTH;
				}
				if (item->text[0]) {
					x = item->textRect.x + item->textRect.w + 8;
				} else {
					x = item->window.rect.x;
				}

				if( item->window.selectionZone.w )
				{
					thumbWidth = item->window.selectionZone.w;
				}
				else
				{
					thumbWidth = SLIDER_THUMB_WIDTH;
				}

				testRect = item->window.rect;
				testRect.x = x;
				value = thumbWidth / 2;
				testRect.x -= value;
				//DC->Print("slider x: %f\n", testRect.x);
				if(item->window.rect.w)
				{
					testRect.w = (item->window.rect.w + (float)SLIDER_THUMB_WIDTH / 2);
				}
				else
				{
					testRect.w = (SLIDER_WIDTH + (float)SLIDER_THUMB_WIDTH / 2);
				}
				//DC->Print("slider w: %f\n", testRect.w);
				if (Rect_ContainsPoint(&testRect, DC->cursorx, DC->cursory)) {
					work = DC->cursorx - x;
					value = work / width;
					value *= (editDef->maxVal - editDef->minVal);
					// vm fuckage
					// value = (((float)(DC->cursorx - x)/ SLIDER_WIDTH) * (editDef->maxVal - editDef->minVal));
					value += editDef->minVal;
					DC->setCVar(item->cvar, va("%f", value));
					return qtrue;
				}

			}
		}
	}
	//Raz: Removed leftover debug print
	//DC->Print("slider handle key exit\n");
	return qfalse;
}


qboolean Item_HandleKey(itemDef_t *item, int key, qboolean down) {

	if (itemCapture) {
		Item_StopCapture(itemCapture);
		itemCapture = NULL;
		captureFunc = 0;
		captureData = NULL;
	} else {
	  // bk001206 - parentheses
		if ( down && ( key == A_MOUSE1 || key == A_MOUSE2 || key == A_MOUSE3 ) ) {
			Item_StartCapture(item, key);
		}
	}

	if (!down) {
		return qfalse;
	}

  switch (item->type) {
    case ITEM_TYPE_BUTTON:
			return qfalse;
      break;
    case ITEM_TYPE_RADIOBUTTON:
      return qfalse;
      break;
    case ITEM_TYPE_CHECKBOX:
      return qfalse;
      break;
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:
		if (key == A_MOUSE1 || key == A_MOUSE2 || key == A_ENTER)
		{
			editFieldDef_t *editPtr = item->typeData.edit;

			if (item->cvar && editPtr)
			{
				editPtr->paintOffset = 0;
			}

      //return Item_TextField_HandleKey(item, key);
		}
      return qfalse;
      break;
    case ITEM_TYPE_COMBO:
      return qfalse;
      break;
    case ITEM_TYPE_LISTBOX:
      return Item_ListBox_HandleKey(item, key, down, qfalse);
      break;
	case ITEM_TYPE_TEXTSCROLL:
      return Item_TextScroll_HandleKey(item, key, down, qfalse);
      break;
    case ITEM_TYPE_YESNO:
      return Item_YesNo_HandleKey(item, key);
      break;
    case ITEM_TYPE_MULTI:
      return Item_Multi_HandleKey(item, key);
      break;
    case ITEM_TYPE_OWNERDRAW:
      return Item_OwnerDraw_HandleKey(item, key);
      break;
	case ITEM_TYPE_BIND_FIXED:	// Jedi Knight Galaxies
    case ITEM_TYPE_BIND:
			return Item_Bind_HandleKey(item, key, down);
      break;
    case ITEM_TYPE_SLIDER:
      return Item_Slider_HandleKey(item, key, down);
      break;
    //case ITEM_TYPE_IMAGE:
    //  Item_Image_Paint(item);
    //  break;
    default:
      return qfalse;
      break;
  }

  //return qfalse;
}


//JLFACCEPT MPMOVED
/*
-----------------------------------------
Item_HandleAccept
	If Item has an accept script, run it.
-------------------------------------------
*/
qboolean Item_HandleAccept(itemDef_t * item)
{
	if (item->accept)
	{
		Item_RunScript(item, item->accept);
		return qtrue;
	}
	return qfalse;
}

void Item_Action(itemDef_t *item) {
  if (item) {
    Item_RunScript(item, item->action);
  }
}



itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu) {
	qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;

	if (menu->cursorItem < 0) {
		menu->cursorItem = menu->itemCount-1;
		wrapped = qtrue;
	} 

	while (menu->cursorItem > -1) 
	{
		menu->cursorItem--;
		if (menu->cursorItem < 0) {
			if (wrapped)
			{
				break;
			}
			wrapped = qtrue;
			menu->cursorItem = menu->itemCount -1;
		}

		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}
	}
	menu->cursorItem = oldCursor;
	return NULL;

}

itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu) {

  qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;


  if (menu->cursorItem == -1) {
    menu->cursorItem = 0;
    wrapped = qtrue;
  }

  while (menu->cursorItem < menu->itemCount) {

    menu->cursorItem++;
    if (menu->cursorItem >= menu->itemCount && !wrapped) {
      wrapped = qtrue;
      menu->cursorItem = 0;
    }
		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
      return menu->items[menu->cursorItem];
    }
    
  }

	menu->cursorItem = oldCursor;
	return NULL;
}

static void Window_CloseCinematic(windowDef_t *window) {
	if (window->style == WINDOW_STYLE_CINEMATIC && window->cinematic >= 0) {
		DC->stopCinematic(window->cinematic);
		window->cinematic = -1;
	}
}

static void Menu_CloseCinematics(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CloseCinematic(&menu->window);
	  for (i = 0; i < menu->itemCount; i++) {
		  Window_CloseCinematic(&menu->items[i]->window);
			if (menu->items[i]->type == ITEM_TYPE_OWNERDRAW) {
				DC->stopCinematic(0-menu->items[i]->window.ownerDraw);
			}
	  }
	}
}

static void Display_CloseCinematics() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CloseCinematics(&Menus[i]);
	}
}

void  Menus_Activate(menuDef_t *menu) {

	menu->window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);
	if (menu->onOpen) {
		itemDef_t item;
		item.parent = menu;
		Item_RunScript(&item, menu->onOpen);
	}

	if (menu->soundName && *menu->soundName) {
//		DC->stopBackgroundTrack();					// you don't want to do this since it will reset s_rawend
		DC->startBackgroundTrack(menu->soundName, menu->soundName, qfalse);
	}

	menu->appearanceTime = 0;
	Display_CloseCinematics();

}

int Display_VisibleMenuCount() {
	int i, count;
	count = 0;
	for (i = 0; i < menuCount; i++) {
		if (Menus[i].window.flags & (WINDOW_FORCED | WINDOW_VISIBLE)) {
			count++;
		}
	}
	return count;
}

void Menus_HandleOOBClick(menuDef_t *menu, int key, qboolean down) {
	if (menu) {
		int i;
		// basically the behaviour we are looking for is if there are windows in the stack.. see if 
		// the cursor is within any of them.. if not close them otherwise activate them and pass the 
		// key on.. force a mouse move to activate focus and script stuff 
		if (down && menu->window.flags & WINDOW_OOB_CLICK) {
			Menu_RunCloseScript(menu);
			menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
		}

		for (i = 0; i < menuCount; i++) {
			if (Menu_OverActiveItem(&Menus[i], DC->cursorx, DC->cursory)) {
				Menu_RunCloseScript(menu);
				menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
				Menus_Activate(&Menus[i]);
				Menu_HandleMouseMove(&Menus[i], DC->cursorx, DC->cursory);
				Menu_HandleKey(&Menus[i], key, down);
			}
		}

		if (Display_VisibleMenuCount() == 0) {
			if (DC->Pause) {
				DC->Pause(qfalse);
			}
		}
		Display_CloseCinematics();
	}
}

int defaultKeyBehaviour;

void Menu_NoDefaultKeyBehaviour() {		// Call this from a OwnerDrawHandleKey function
	defaultKeyBehaviour = 0;			// To disable default key handling for this control
}

void Menu_SetTextFieldFocus(itemDef_t *item) {
	if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) {
		item->cursorPos = 0;
		item->window.flags |= WINDOW_HASFOCUS;
		g_editItem = item;
		g_editingField = qtrue;
	}
}

void Menu_HandleKey(menuDef_t *menu, int key, qboolean down) {
	int i;
	itemDef_t *item = NULL;
	qboolean inHandler = qfalse;

	defaultKeyBehaviour = 1;

	if (inHandler) {
		return;
	}

	inHandler = qtrue;
	if (g_waitingForKey && down) {
		Item_Bind_HandleKey(g_bindItem, key, down);
		inHandler = qfalse;
		return;
	}

#ifdef UI_EXPORTS
	if (Q_stricmp(menu->window.name, "jkg_inventory") == 0)
	{
		if (JKG_Inventory_HandleKey(key))
		{
			return;
		}
	}
	// Scrollwheel support for sjop --eez
	else if (key == A_MWHEELDOWN || key == A_MWHEELUP)
	{
		if (Q_stricmp(menu->window.name, "jkg_shop") == 0)
		{
			JKG_ScrollShop(key == A_MWHEELUP, DC->cursorx, DC->cursory);
		}
	}
#endif

	if (g_editingField && down) 
	{
		if (!Item_TextField_HandleKey(g_editItem, key)) 
		{
			g_editingField = qfalse;
			g_editItem = NULL;
			inHandler = qfalse;
			return;
		} 
		else if (key == A_MOUSE1 || key == A_MOUSE2 || key == A_MOUSE3) 
		{
			// switching fields so reset printed text of edit field
			Leaving_EditField(g_editItem);
			g_editingField = qfalse;
			g_editItem = NULL;
			Display_MouseMove(NULL, DC->cursorx, DC->cursory);
		} 
		else if (key == A_TAB || key == A_CURSOR_UP || key == A_CURSOR_DOWN) 
		{
			return;
		}
	}

	if (menu == NULL) {
		inHandler = qfalse;
		return;
	}
		// see if the mouse is within the window bounds and if so is this a mouse click
	if (down && !(menu->window.flags & WINDOW_POPUP) && !Rect_ContainsPoint(&menu->window.rect, DC->cursorx, DC->cursory)) 
	{
		static qboolean inHandleKey = qfalse;
		// bk001206 - parentheses
		if (!inHandleKey && ( key == A_MOUSE1 || key == A_MOUSE2 || key == A_MOUSE3 ) ) {
			inHandleKey = qtrue;
			Menus_HandleOOBClick(menu, key, down);
			inHandleKey = qfalse;
			inHandler = qfalse;
			return;
		}
	}

	// get the item with focus
	for (i = 0; i < menu->itemCount; i++) {
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS || (Rect_ContainsPoint(&menu->items[i]->window.rect, DC->cursorx, DC->cursory) && (menu->items[i]->window.name && (!strcmp(menu->items[i]->window.name, "inventory_feederstuff") && !menu->items[i]->disabled)))) {
			item = menu->items[i]; //HACK
		}
	}

	// Ignore if disabled
	if (item && item->disabled)
	{
		return;
	}

	if (item != NULL) {
		if (Item_HandleKey(item, key, down)) 
		{
				// It is possible for an item to be disable after Item_HandleKey is run (like in Voice Chat)
				if (!item->disabled || Q_stricmp(item->window.name, "filtertab_btn")) 
				{
					Item_Action(item);
				}
			inHandler = qfalse;
			return;
		}
	}

	if (!down) {
		inHandler = qfalse;
		return;
	}

	if (!defaultKeyBehaviour) {
		inHandler = qfalse;
		return;
	}

	// default handling
	switch ( key ) {

		case A_F11:
			if (DC->getCVarValue("developer")) {
				debugMode ^= 1;
			}
			break;

		case A_F12:
			if (DC->getCVarValue("developer")) {
				DC->executeText(EXEC_APPEND, "screenshot\n");
			}
			break;
		case A_KP_8:
		case A_CURSOR_UP:
			Menu_SetPrevCursorItem(menu);
			break;

		case A_ESCAPE:
			if (!g_waitingForKey && menu->onESC) {
				itemDef_t it;
		    it.parent = menu;
		    Item_RunScript(&it, menu->onESC);
			}
		    g_waitingForKey = qfalse;
			break;
		case A_TAB:
		case A_KP_2:
		case A_CURSOR_DOWN:
			Menu_SetNextCursorItem(menu);
			break;

		case A_MOUSE1:
		case A_MOUSE2:
			if (item) {
				if (item->type == ITEM_TYPE_TEXT) {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory))
					{
						Item_Action(item);
					}
				} else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) 
					{	
						Item_Action(item);
						item->cursorPos = 0;
						g_editingField = qtrue;
						g_editItem = item;
						//DC->setOverstrikeMode(qtrue);
					}
				}
				
	//JLFACCEPT
// add new types here as needed
/* Notes:
	Most controls will use the dpad to move through the selection possibilies.  Buttons are the only exception.
	Buttons will be assumed to all be on one menu together.  If the start or A button is pressed on a control focus, that
	means that the menu is accepted and move onto the next menu.  If the start or A button is pressed on a button focus it
	should just process the action and not support the accept functionality.
*/

//JLFACCEPT MPMOVED
				else if ( item->type == ITEM_TYPE_MULTI || item->type == ITEM_TYPE_YESNO || item->type == ITEM_TYPE_SLIDER)
				{
					if (Item_HandleAccept(item))
					{
						//Item processed it overriding the menu processing
						return;
					}
					else if (menu->onAccept) 
					{
						itemDef_t it;
						it.parent = menu;
						Item_RunScript(&it, menu->onAccept);
					}
					else if (item->type == ITEM_TYPE_SLIDER) {
						qRectangle test;
						test.x = item->textRect.x + item->textRect.w + 8;
						test.y = item->window.rect.y;
						test.w = item->window.rect.w + item->textRect.w;
						test.h = item->window.rect.h;
						if (Rect_ContainsPoint(&test, DC->cursorx, DC->cursory)) {
							editFieldDef_t *editDef = item->typeData.edit;
							float value, x;
							if (item->text[0]) {
								x = item->textRect.x + item->textRect.w + 8;
							}
							else {
								x = item->window.rect.x;
							}
							value = DC->cursorx;
							value -= x;
							if (item->window.rect.w && item->window.rect.h)
							{
								value /= item->window.rect.w;
							}
							else
							{
								value /= SLIDER_WIDTH;
							}
							if (value < editDef->minVal) {
								value = editDef->minVal;
							}
							else if (value > editDef->maxVal) {
								value = editDef->maxVal;
							}
							DC->setCVar(item->cvar, va("%f", value));
						}
					}
				}
//END JLFACCEPT			
				else {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory))
					{

						Item_Action(item);
					}
				}
			}
			break;

		case A_JOY0:
		case A_JOY1:
		case A_JOY2:
		case A_JOY3:
		case A_JOY4:
		case A_AUX0:
		case A_AUX1:
		case A_AUX2:
		case A_AUX3:
		case A_AUX4:
		case A_AUX5:
		case A_AUX6:
		case A_AUX7:
		case A_AUX8:
		case A_AUX9:
		case A_AUX10:
		case A_AUX11:
		case A_AUX12:
		case A_AUX13:
		case A_AUX14:
		case A_AUX15:
		case A_AUX16:
			break;
		case A_KP_ENTER:
		case A_ENTER:
			if (item) {
				if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) {
					item->cursorPos = 0;
					g_editingField = qtrue;
					g_editItem = item;
					//DC->setOverstrikeMode(qtrue);
				} else {
						Item_Action(item);
				}
			}
			break;
	}
	inHandler = qfalse;
}

void ToWindowCoords(float *x, float *y, windowDef_t *window) {
	if (window->border != 0) {
		*x += window->borderSize;
		*y += window->borderSize;
	} 
	*x += window->rect.x;
	*y += window->rect.y;
}

void Rect_ToWindowCoords(rectDef_t *rect, windowDef_t *window) {
	ToWindowCoords(&rect->x, &rect->y, window);
}

void Item_SetTextExtents(itemDef_t *item, int *width, int *height, const char *text) {
	const char *textPtr = (text) ? text : item->text;
	
	if (item->type == ITEM_TYPE_TEXT && item->typeData.text && item->typeData.text->customText) {
		textPtr = item->typeData.text->text;
	}

	if (textPtr == NULL ) {
		return;
	}

	*width = item->textRect.w;
	*height = item->textRect.h;

	// keeps us from computing the widths and heights more than once
	if (*width == 0 || (item->type == ITEM_TYPE_OWNERDRAW && item->textalignment == ITEM_ALIGN_CENTER)
		|| item->window.ownerDraw == UI_JKG_ITEM_NAME
		|| item->window.ownerDraw == UI_JKG_SHOP_LEFTNAME
		|| item->window.ownerDraw == UI_JKG_SHOP_RIGHTNAME
		|| item->window.ownerDraw == UI_JKG_SHOP_LEFTPRICE
		|| item->window.ownerDraw == UI_JKG_SHOP_RIGHTPRICE
		|| (item->type == ITEM_TYPE_TEXT && item->typeData.text && item->typeData.text->customText && item->textalignment != ITEM_ALIGN_LEFT)
#ifndef _CGAME
		|| (item->text[0]=='@' && item->asset != se_language.modificationCount )	//string package language changed
#endif
		)
	{
		float originalWidth = DC->textWidth(textPtr, 1, item->iMenuFont) * item->textscale;

		if (item->type == ITEM_TYPE_OWNERDRAW && (item->textalignment == ITEM_ALIGN_CENTER || item->textalignment == ITEM_ALIGN_RIGHT)) 
		{
			originalWidth += DC->ownerDrawWidth(item->window.ownerDraw, item->window.ownerDrawID, item->textscale);
		} 
		else if (item->type == ITEM_TYPE_EDITFIELD && item->textalignment == ITEM_ALIGN_CENTER && item->cvar) 
		{
			char buff[256];
			DC->getCVarString(item->cvar, buff, 256);
			originalWidth += DC->textWidth(buff, item->textscale, item->iMenuFont);
		}

		*width = DC->textWidth(textPtr, 1, item->iMenuFont) * item->textscale;
		*height = DC->textHeight(textPtr, 1, item->iMenuFont) * item->textscale;
		
		item->textRect.w = *width;
		item->textRect.h = *height;
		item->textRect.x = item->textalignx;
		item->textRect.y = item->textaligny;
		if (item->textalignment == ITEM_ALIGN_RIGHT) {
			item->textRect.x = item->textalignx - originalWidth;
		} else if (item->textalignment == ITEM_ALIGN_CENTER) {
			item->textRect.x = item->textalignx - (originalWidth / 2);
		}

		ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
#ifndef _CGAME
		if (item->text[0]=='@' )//string package
		{//mark language
			item->asset = se_language.modificationCount;
		}
#endif
	}
}

void Item_TextColor(itemDef_t *item, vec4_t *newColor) {
	vec4_t lowLight;
	menuDef_t *parent = (menuDef_t*)item->parent;

	Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,*newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) {
		lowLight[0] = 0.8 * item->window.foreColor[0]; 
		lowLight[1] = 0.8 * item->window.foreColor[1]; 
		lowLight[2] = 0.8 * item->window.foreColor[2]; 
		lowLight[3] = 0.8 * item->window.foreColor[3]; 
		LerpColor(item->window.foreColor,lowLight,*newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else {
		memcpy(newColor, item->window.foreColor, sizeof(vec4_t));	//original
	}

	// items can be enabled and disabled based on cvars
	if (item->disabled) 
	{
		memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
	}

	if (item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) {
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
			memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
		}
	}
}

void Item_Text_AutoWrapped_Paint(itemDef_t *item) {
	char text[2048];
	const char *p, *textPtr, *newLinePtr;
	char buff[2048];
	int height, len, textWidth, newLine, newLineWidth;	//int width;
	float y;
	vec4_t color;

	textWidth = 0;
	newLinePtr = NULL;

	if (item->text[0] == '\0') {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '@')	// string reference
	{
		trap->SE_GetStringTextString( &textPtr[1], text, sizeof(text));
		textPtr = text;
	}
	if (*textPtr == '\0') {
		return;
	}
	Item_TextColor(item, &color);
	//Item_SetTextExtents(item, &width, &height, textPtr);
	//if (item->value == 0)
	//{
	//	item->value = (int)(0.5 + (float)DC->textWidth(textPtr, item->textscale, item->font) / item->window.rect.w);
	//}
	height = DC->textHeight(textPtr, item->textscale, item->iMenuFont);

	y = item->textaligny;
	len = 0;
	buff[0] = '\0';
	newLine = 0;
	newLineWidth = 0;
	p = textPtr;
	while (p) {	//findmeste (this will break widechar languages)!
		if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') {
			newLine = len;
			newLinePtr = p+1;
			newLineWidth = textWidth;
		}
		textWidth = DC->textWidth(buff, item->textscale, 0);
		if ( (newLine && textWidth > item->window.rect.w) || *p == '\n' || *p == '\0') {
			if (len) {
				if (item->textalignment == ITEM_ALIGN_LEFT) {
					item->textRect.x = item->textalignx;
				} else if (item->textalignment == ITEM_ALIGN_RIGHT) {
					item->textRect.x = item->textalignx - newLineWidth;
				} else if (item->textalignment == ITEM_ALIGN_CENTER) {
					item->textRect.x = item->textalignx - newLineWidth / 2;
				}
				item->textRect.y = y;
				ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
				//
				buff[newLine] = '\0';
				DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, buff, 0, 0, item->textStyle, item->iMenuFont);
			}
			if (*p == '\0') {
				break;
			}
			//
			y += height + 5;
			p = newLinePtr;
			len = 0;
			newLine = 0;
			newLineWidth = 0;
			continue;
		}
		buff[len++] = *p++;
		buff[len] = '\0';
	}
}

void Item_Text_Wrapped_Paint(itemDef_t *item) {
	char text[1024];
	const char *p, *start, *textPtr;
	char buff[1024];
	int width, height;
	float x, y;
	vec4_t color;

	// now paint the text and/or any optional images
	// default to left

	if (item->text[0] == '\0') {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '@')	// string reference
	{
		trap->SE_GetStringTextString( &textPtr[1], text, sizeof(text));
		textPtr = text;
	}
	if (*textPtr == '\0') {
		return;
	}

	Item_TextColor(item, &color);
	Item_SetTextExtents(item, &width, &height, textPtr);

	x = item->textRect.x;
	y = item->textRect.y;
	start = textPtr;
	p = strchr(textPtr, '\r');
	while (p && *p) {
		strncpy(buff, start, p-start+1);
		buff[p-start] = '\0';
		DC->drawText(x, y, item->textscale, color, buff, 0, 0, item->textStyle, item->iMenuFont);
		y += height + 2;
		start += p - start + 1;
		p = strchr(p+1, '\r');
	}
	DC->drawText(x, y, item->textscale, color, start, 0, 0, item->textStyle, item->iMenuFont);
}

//default version
void Item_Text_Paint(itemDef_t* item)
{
	char text[1024] = { 0 };
	const char* textPtr;
	int height, width;
	vec4_t color;

	if (item->window.flags & WINDOW_WRAPPED) {
		Item_Text_Wrapped_Paint(item);
		return;
	}
	if (item->window.flags & WINDOW_AUTOWRAPPED) {
		Item_Text_AutoWrapped_Paint(item);
		return;
	}
	// Jedi Knight Galaxies, allow ITEM_TYPE_TEXT items to have dynamic texts. (IF defined!)
	if (item->type == ITEM_TYPE_TEXT && item->typeData.text && item->typeData.text->customText) {
		textPtr = item->typeData.text->text;
		if (!textPtr) return;
	}
	else if (item->text[0] == '\0') {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '@')	// string reference
	{
		trap->SE_GetStringTextString(&textPtr[1], text, sizeof(text));
		textPtr = text;
	}

	// this needs to go here as it sets extents for cvar types as well
	Item_SetTextExtents(item, &width, &height, textPtr);

	if (*textPtr == '\0') {
		return;
	}
	
	Item_TextColor(item, &color);
	DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, textPtr, 0, 0, item->textStyle, item->iMenuFont);

	if (item->text2[0])	// Is there a second line of text?
	{
		int fontHandle = (item->iMenuFont2) ? item->iMenuFont2 : item->iMenuFont;
		float textscale = (item->textscale2) ? item->textscale2 : item->textscale;
		int textstyle = (item->textStyle2) ? item->textStyle2 : item->textStyle;
		textPtr = item->text2;
		if (*textPtr == '@')	// string reference
		{
			trap->SE_GetStringTextString(&textPtr[1], text, sizeof(text));
			textPtr = text;
		}
		Item_TextColor(item, &color);
		//Hold your fancy pants. This is all fine and dandy for left-aligned, but for anything else, it's not gud.
		if ((item->textalignment == ITEM_ALIGN_LEFT || item->textalignment == ITEM_ALIGN_CENTER) && item->text2alignment != ITEM_ALIGN_RIGHT)
		{
			DC->drawText(item->textRect.x + item->text2alignx, item->textRect.y + item->text2aligny, textscale, color, textPtr, 0, 0, textstyle, fontHandle);
		}
		else if (item->textalignment == ITEM_ALIGN_RIGHT || item->text2alignment == ITEM_ALIGN_RIGHT)
		{
			DC->drawText((item->window.rect.x + item->textalignx + item->text2alignx) - (DC->textWidth(textPtr, 1, fontHandle) * textscale), item->textRect.y + item->text2aligny, textscale, color, textPtr, 0, 0, textstyle, fontHandle);
		}
	}
}

//special version - you can pass a color along
void Item_Text_Paint(itemDef_t* item, vec4_t &color) {

	char text[1024] = { 0 };
	const char* textPtr;
	int height, width;

	if (item->window.flags & WINDOW_WRAPPED) {
		Item_Text_Wrapped_Paint(item);
		return;
	}
	if (item->window.flags & WINDOW_AUTOWRAPPED) { //need to solve what to do with specifying colors when called from JKG_SHOP_ShopItemName() etc
		if (item->window.flags & WINDOW_TEXTCOLOR)
		{
			memcpy(item->window.foreColor, color, sizeof(vec4_t));	//hack to override color
			Item_Text_AutoWrapped_Paint(item);
			return;
		}

		else
		{
			Item_Text_AutoWrapped_Paint(item);
			return;
		}
	}
	// Jedi Knight Galaxies, allow ITEM_TYPE_TEXT items to have dynamic texts. (IF defined!)
	if (item->type == ITEM_TYPE_TEXT && item->typeData.text && item->typeData.text->customText) {
		textPtr = item->typeData.text->text;
		if (!textPtr) return;
	}
	else if (item->text[0] == '\0') {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '@')	// string reference
	{
		trap->SE_GetStringTextString(&textPtr[1], text, sizeof(text));
		textPtr = text;
	}

	// this needs to go here as it sets extents for cvar types as well
	Item_SetTextExtents(item, &width, &height, textPtr);

	if (*textPtr == '\0') {
		return;
	}

	//Item_TextColor(item, &color); //don't bother
	DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, textPtr, 0, 0, item->textStyle, item->iMenuFont);

	if (item->text2[0])	// Is there a second line of text?
	{
		int fontHandle = (item->iMenuFont2) ? item->iMenuFont2 : item->iMenuFont;
		float textscale = (item->textscale2) ? item->textscale2 : item->textscale;
		int textstyle = (item->textStyle2) ? item->textStyle2 : item->textStyle;
		textPtr = item->text2;
		if (*textPtr == '@')	// string reference
		{
			trap->SE_GetStringTextString(&textPtr[1], text, sizeof(text));
			textPtr = text;
		}
		Item_TextColor(item, &color);
		//Hold your fancy pants. This is all fine and dandy for left-aligned, but for anything else, it's not gud.
		if ((item->textalignment == ITEM_ALIGN_LEFT || item->textalignment == ITEM_ALIGN_CENTER) && item->text2alignment != ITEM_ALIGN_RIGHT)
		{
			DC->drawText(item->textRect.x + item->text2alignx, item->textRect.y + item->text2aligny, textscale, color, textPtr, 0, 0, textstyle, fontHandle);
		}
		else if (item->textalignment == ITEM_ALIGN_RIGHT || item->text2alignment == ITEM_ALIGN_RIGHT)
		{
			DC->drawText((item->window.rect.x + item->textalignx + item->text2alignx) - (DC->textWidth(textPtr, 1, fontHandle) * textscale), item->textRect.y + item->text2aligny, textscale, color, textPtr, 0, 0, textstyle, fontHandle);
		}
	}
}



//float			trap->Cvar_VariableValue( const char *var_name );
//void			trap->Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// JKG - Modified to render textfields properly

void Item_TextField_Paint(itemDef_t *item) {
	char buff[1024];
	const char *text;
	char *x;
	vec4_t newColor, lowLight;
	int offset;
	int cursorpos;
	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = item->typeData.edit;

	Item_Text_Paint(item);

	buff[0] = '\0';

	//Jedi Knight Galaxies
	if (item->cvar) {
		DC->getCVarString(item->cvar, buff, sizeof(buff));
		if (buff[0] == '@')	// string reference
		{
			trap->SE_GetStringTextString( &buff[1], buff, sizeof(buff));
		}
	} else {
		Q_strncpyz(buff, editPtr->buffer,sizeof(buff));
	}
	if (item->flags & ITF_TEXTFIELD_PASSWORD) {
		x = buff;
		while (1) {
			if (*x)
				*x++ = '*';
			else
				break;
		}
	}

	parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	/*
	offset = (item->text && *item->text) ? 8 : 0;
	if (item->window.flags & WINDOW_HASFOCUS && g_editingField) {
		char cursor = DC->getOverstrikeMode() ? '_' : '|';
		DC->drawTextWithCursor(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, newColor, buff + editPtr->paintOffset, item->cursorPos - editPtr->paintOffset , cursor, item->window.rect.w, item->textStyle, item->iMenuFont);
	} else {
		DC->drawText(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, newColor, buff + editPtr->paintOffset, 0, item->window.rect.w, item->textStyle,item->iMenuFont);
	}*/
	text = Text_PrintableText(buff, item);
	offset = item->cursorPos - editPtr->paintOffset;
	
	// We use Text_DrawText here to ensure correct spacing
	Text_DrawText(
			item->textRect.x+4,
			item->textRect.y,
			text,
			newColor,
			MenuFontToHandle(item->iMenuFont),
			-1,
			item->textscale);
	if (item->window.flags & WINDOW_HASFOCUS && g_editingField) {
		// Draw the cursor
		if (offset > strlen(text)) {
			// Shouldn't happen.. but still, just in case
			cursorpos = Text_GetWidth(text, MenuFontToHandle(item->iMenuFont), item->textscale);
		} else {
			((char *)text)[offset] = 0;
			cursorpos = Text_GetWidth(text, MenuFontToHandle(item->iMenuFont), item->textscale);
		}
		Text_DrawText(
				item->textRect.x + 2 + cursorpos,
				item->textRect.y,
				DC->getOverstrikeMode() ? "_" : "|" ,
				newColor,
				MenuFontToHandle(item->iMenuFont) | 0x40000000 /*STYLE_BLINK*/,
				-1,
				item->textscale);
	}
}

void Item_YesNo_Paint(itemDef_t *item) {
	char	sYES[20];
	char	sNO[20];
	vec4_t newColor, lowLight;
	float value;
	menuDef_t *parent = (menuDef_t*)item->parent;
	const char *yesnovalue;
	
	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}


	trap->SE_GetStringTextString("MENUS_YES",sYES, sizeof(sYES));
	trap->SE_GetStringTextString("MENUS_NO", sNO,  sizeof(sNO));

//JLFYESNO MPMOVED
	if (item->invertYesNo)
		yesnovalue = (value == 0) ? sYES : sNO;
	else
		yesnovalue = (value != 0) ? sYES : sNO;
	
	if (item->text[0]) 
	{
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, yesnovalue, 0, 0, item->textStyle, item->iMenuFont);
	} 
	else 
	{
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, yesnovalue , 0, 0, item->textStyle, item->iMenuFont);
	}		

/* JLF ORIGINAL CODE
	if (item->text) {
		Item_Text_Paint(item);
//		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, (value != 0) ? sYES : sNO, 0, 0, item->textStyle);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, (value != 0) ? sYES : sNO, 0, 0, item->textStyle,item->iMenuFont);
	} else {
//		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? sYES : sNO, 0, 0, item->textStyle);
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? sYES : sNO, 0, 0, item->textStyle,item->iMenuFont);
	}
*/
}

void Item_Multi_Paint(itemDef_t *item) {
	vec4_t newColor, lowLight;
	const char *text = "";
	menuDef_t *parent = (menuDef_t*)item->parent;
	char	temp[MAX_STRING_CHARS];

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	text = Item_Multi_Setting(item);
	if (*text == '@')	// string reference
	{
		trap->SE_GetStringTextString( &text[1]  , temp, sizeof(temp));
		text = temp;
	}
	// Is is specifying a cvar to get the item name from?
	else if (*text == '*')
	{
		DC->getCVarString(&text[1], temp, sizeof(temp));
		text = temp;
	}

	if (item->text[0]) {
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, text, 0, 0, item->textStyle,item->iMenuFont);
	} else {
		//JLF added xoffset
		DC->drawText(item->textRect.x+item->xoffset, item->textRect.y, item->textscale, newColor, text, 0, 0, item->textStyle,item->iMenuFont);
	}
}


typedef struct {
	char	*command;
	int		id;
	int		defaultbind1;
	int		defaultbind2;
	int		bind1;
	int		bind2;
} bind_t;

typedef struct
{
	char*	name;
	float	defaultvalue;
	float	value;	
} configcvar_t;


static bind_t g_bindings[] = 
{
	{"+scores",			 A_TAB,				-1,		-1, -1},
	{"+button2",		 A_ENTER,			-1,		-1, -1},
	{"+speed", 			 A_SHIFT,			-1,		-1,	-1},
	{"+forward", 		 A_CURSOR_UP,		-1,		-1, -1},
	{"+back", 			 A_CURSOR_DOWN,		-1,		-1, -1},
	{"+moveleft",		 ',',				-1,		-1, -1},
	{"+moveright", 		 '.',				-1,		-1, -1},
	{"+moveup",			 A_SPACE,			-1,		-1, -1},
	{"+movedown",		 'c',				-1,		-1, -1},
	{"+left", 			 A_CURSOR_LEFT,		-1,		-1, -1},
	{"+right", 			 A_CURSOR_RIGHT,	-1,		-1, -1},
	{"+strafe", 		 A_ALT,				-1,		-1, -1},
	{"+lookup", 		 A_PAGE_DOWN,		-1,		-1, -1},
	{"+lookdown",	 	 A_DELETE,			-1,		-1, -1},
	{"+mlook", 			 '/',				-1,		-1, -1},
	{"centerview",		 A_END,				-1,		-1, -1},
//	{"+zoom", 			 -1,				-1,		-1, -1},
	{"weapon 1",		 '1',				-1,		-1, -1},
	{"weapon 2",		 '2',				-1,		-1, -1},
	{"weapon 3",		 '3',				-1,		-1, -1},
	{"weapon 4",		 '4',				-1,		-1, -1},
	{"weapon 5",		 '5',				-1,		-1, -1},
	{"weapon 6",		 '6',				-1,		-1, -1},
	{"weapon 7",		 '7',				-1,		-1, -1},
	{"weapon 8",		 '8',				-1,		-1, -1},
	{"weapon 9",		 '9',				-1,		-1, -1},
	{"weapon 0",		 '0',				-1,		-1, -1},
	{"saberAttackCycle", 'l',				-1,		-1, -1},
	{"+attack", 		 -1,			-1,		-1, -1},
	{"+altattack", 		-1,					-1,		-1,	-1},
	{"+use",			-1,					-1,		-1, -1},
	{"engage_duel",		'h',				-1,		-1, -1},
	{"taunt",			'u',				-1,		-1, -1},
	{"bow",				-1,					-1,		-1, -1},
	{"meditate",		-1,					-1,		-1, -1},
	{"flourish",		-1,					-1,		-1, -1},
	{"gloat",			-1,					-1,		-1, -1},
	{"weapprev",		 '[',				-1,		-1, -1},
	{"weapnext", 		 ']',				-1,		-1, -1},
	{"prevTeamMember",	'w',				-1,		-1, -1},
	{"nextTeamMember",	'r',				-1,		-1, -1},
	{"scoresUp",		A_INSERT,			-1,		-1, -1},
	{"scoresDown",		A_DELETE,			-1,		-1, -1},
	{"messagemode",		-1,					-1,		-1, -1},
	{"messagemode2",	-1,					-1,		-1, -1},
	{"messagemode3",	-1,					-1,		-1, -1},
	{"messagemode4",	-1,					-1,		-1, -1},
	{"+use",			-1,					-1,		-1,	-1},

	{"+useforce",		-1,					-1,		-1,	-1},
	{"forcenext",		-1,					-1,		-1,	-1},
	{"forceprev",		-1,					-1,		-1,	-1},
	{"invnext",			-1,					-1,		-1,	-1},
	{"invprev",			-1,					-1,		-1,	-1},
	{"use_seeker",		-1,					-1,		-1,	-1},
	{"use_field",		-1,					-1,		-1,	-1},
	{"use_bacta",		-1,					-1,		-1,	-1},
	{"use_electrobinoculars",	-1,			-1,		-1,	-1},
	{"use_sentry",		-1,					-1,		-1,	-1},
	{"cg_thirdperson !",-1,					-1,		-1,	-1},
	{"automap_button",	-1,					-1,		-1,	-1},
	{"automap_toggle",	-1,					-1,		-1,	-1},
	{"voicechat",		-1,					-1,		-1,	-1},

	// Jedi Knight Galaxies
	{"+button12",		A_CTRL,			A_CTRL,-1,-1},
	{"+button13",		A_MOUSE3,		A_MOUSE3,	-1, -1},
	{"reload",			-1,					-1,		-1,	-1},
	{"inventory",		-1,					-1,		-1,	-1},
	{"+camera",			-1,					-1,		-1,	-1},
	{"cameraZoomIn",	-1,					-1,		-1,	-1},
	{"cameraZoomOut",	-1,					-1,		-1,	-1},
	{"pzktest",			-1,					-1,		-1,	-1},
	{"slctest",			-1,					-1,		-1,	-1},
	{"ammocycle",		'n',				-1,		-1, -1},
};


static const int g_bindCount = sizeof(g_bindings) / sizeof(bind_t);

/*
=================
Controls_GetKeyAssignment
=================
*/
static void Controls_GetKeyAssignment (char *command, int *twokeys)
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < MAX_KEYS; j++ )
	{
		DC->getBindingBuf( j, b, 256 );
		if ( *b == 0 ) {
			continue;
		}
		if ( !Q_stricmp( b, command ) ) {
			twokeys[count] = j;
			count++;
			if (count == 2) {
				break;
			}
		}
	}
}

/*
=================
Controls_GetConfig
=================
*/
void Controls_GetConfig( void )
{
	int		i;
	int		twokeys[2];

	// iterate each command, get its numeric binding
	for (i=0; i < g_bindCount; i++)
	{

		Controls_GetKeyAssignment(g_bindings[i].command, twokeys);

		g_bindings[i].bind1 = twokeys[0];
		g_bindings[i].bind2 = twokeys[1];
	}

	//s_controls.invertmouse.curvalue  = DC->getCVarValue( "m_pitch" ) < 0;
	//s_controls.smoothmouse.curvalue  = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "m_filter" ) );
	//s_controls.alwaysrun.curvalue    = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "cl_run" ) );
	//s_controls.autoswitch.curvalue   = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "cg_autoswitch" ) );
	//s_controls.sensitivity.curvalue  = UI_ClampCvar( 2, 30, Controls_GetCvarValue( "sensitivity" ) );
	//s_controls.joyenable.curvalue    = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "in_joystick" ) );
	//s_controls.joythreshold.curvalue = UI_ClampCvar( 0.05, 0.75, Controls_GetCvarValue( "joy_threshold" ) );
	//s_controls.freelook.curvalue     = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "cl_freelook" ) );
}

/*
=================
Controls_SetConfig
=================
*/
void Controls_SetConfig( void )
{
	int		i;

	// iterate each command, get its numeric binding
	for (i=0; i < g_bindCount; i++)
	{
		if (g_bindings[i].bind1 != -1)
		{	
			DC->setBinding( g_bindings[i].bind1, g_bindings[i].command );

			if (g_bindings[i].bind2 != -1)
				DC->setBinding( g_bindings[i].bind2, g_bindings[i].command );
		}
	}
}


int BindingIDFromName(const char *name) {
	int i;
  for (i=0; i < g_bindCount; i++)
	{
		if (Q_stricmp(name, g_bindings[i].command) == 0) {
			return i;
		}
	}
	return -1;
}

char g_nameBind1[32];
char g_nameBind2[32];

void BindingFromName(const char *cvar) {
	int		i, b1, b2;
	char	sOR[32];


	// iterate each command, set its default binding
	for (i=0; i < g_bindCount; i++)
	{
		if (Q_stricmp(cvar, g_bindings[i].command) == 0) {
			b1 = g_bindings[i].bind1;
			if (b1 == -1) {
				break;
			}
				DC->keynumToStringBuf( b1, g_nameBind1, 32 );
// do NOT do this or it corrupts asian text!!!					Q_strupr(g_nameBind1);

				b2 = g_bindings[i].bind2;
				if (b2 != -1)
				{
					DC->keynumToStringBuf( b2, g_nameBind2, 32 );
// do NOT do this or it corrupts asian text!!!					Q_strupr(g_nameBind2);

					trap->SE_GetStringTextString("MENUS_KEYBIND_OR",sOR, sizeof(sOR));

					strcat( g_nameBind1, va(" %s ",sOR));
					strcat( g_nameBind1, g_nameBind2 );
				}
			return;
		}
	}
	strcpy(g_nameBind1, "???");
}

void Item_Slider_Paint(itemDef_t *item) {
	vec4_t newColor, lowLight;
	float x, y, value;
	float sW, sH;
	menuDef_t *parent = (menuDef_t*)item->parent;
	qhandle_t background, foreground;

	if(item->window.background)
	{
		background = item->window.background;
	}
	else
	{
		background = DC->Assets.sliderBar;
	}

	if(item->window.foreground)
	{
		foreground = item->window.foreground;
	}
	else
	{
		foreground = DC->Assets.sliderThumb;
	}

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	y = item->window.rect.y;
	if (item->text[0]) {
		Item_Text_Paint(item);
		x = item->textRect.x + item->textRect.w + 8;
	} else {
		x = item->window.rect.x;
	}
	DC->setColor(newColor);
	// bye-bye, hardcoded slider width and height...
	if( item->window.rect.w && item->window.rect.h )
	{
		DC->drawHandlePic( x, y, item->window.rect.w, item->window.rect.h, background );
	}
	else
	{
		DC->drawHandlePic( x, y, SLIDER_WIDTH, SLIDER_HEIGHT, background );
	}

	if( item->window.selectionZone.w )
	{
		sW = item->window.selectionZone.w;
	}
	else
	{
		sW = SLIDER_THUMB_WIDTH;
	}

	if( item->window.selectionZone.h )
	{
		sH = item->window.selectionZone.h;
	}
	else
	{
		sH = SLIDER_THUMB_HEIGHT;
	}
	//DC->drawHandlePic( x, y, SLIDER_WIDTH, SLIDER_HEIGHT, DC->Assets.sliderBar );

	x = Item_Slider_ThumbPosition(item);
	DC->drawHandlePic( x - (sW / 2), y - 2, sW, sH, foreground );
}

void Item_Bind_Paint(itemDef_t *item) 
{
	vec4_t newColor, lowLight;
	float value;
	int maxChars = 0;
	float	textScale,textWidth;
	int		textHeight,yAdj,startingXPos;


	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = item->typeData.edit;
	if (editPtr) 
	{
		maxChars = editPtr->maxPaintChars;
	}

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		if (g_bindItem == item) 
		{
			lowLight[0] = 0.8f * 1.0f;
			lowLight[1] = 0.8f * 0.0f;
			lowLight[2] = 0.8f * 0.0f;
			lowLight[3] = 0.8f * 1.0f;
		} 
		else 
		{
			lowLight[0] = 0.8f * parent->focusColor[0]; 
			lowLight[1] = 0.8f * parent->focusColor[1]; 
			lowLight[2] = 0.8f * parent->focusColor[2]; 
			lowLight[3] = 0.8f * parent->focusColor[3]; 
		}
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text[0]) 
	{
		Item_Text_Paint(item);
		BindingFromName(item->cvar);

		// If the text runs past the limit bring the scale down until it fits.
		textScale = item->textscale;
		textWidth = DC->textWidth(g_nameBind1,(float) textScale, item->iMenuFont);
		startingXPos = (item->textRect.x + item->textRect.w + 8);

		while ((startingXPos + textWidth) >= SCREEN_WIDTH)
		{
			textScale -= .05f;
			textWidth = DC->textWidth(g_nameBind1,(float) textScale, item->iMenuFont);
		}

		// Try to adjust it's y placement if the scale has changed.
		yAdj = 0;
		if (textScale != item->textscale)
		{
			textHeight = DC->textHeight(g_nameBind1, item->textscale, item->iMenuFont);
			yAdj = textHeight - DC->textHeight(g_nameBind1, textScale, item->iMenuFont);
		}

		DC->drawText(startingXPos, item->textRect.y + yAdj, textScale, newColor, g_nameBind1, 0, maxChars, item->textStyle,item->iMenuFont);
	} 
	else 
	{
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? "FIXME" : "FIXME", 0, maxChars, item->textStyle,item->iMenuFont);
	}
}

void Item_Bind_Fixed_Paint(itemDef_t *item) 
{
	vec4_t newColor, lowLight;
	float value;
	int maxChars = 0;
	float	textScale,textWidth;
	int		/*textHeight,yAdj,*/startingXPos;


	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = item->typeData.edit;
	if (editPtr) 
	{
		maxChars = editPtr->maxPaintChars;
	}

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) 
	{
		if (g_bindItem == item && g_waitingForKey) 
		{
			lowLight[0] = 0.8f * 1.0f;
			lowLight[1] = 0.8f * 0.0f;
			lowLight[2] = 0.8f * 0.0f;
			lowLight[3] = 0.8f * 1.0f;
		} 
		else 
		{
			lowLight[0] = 0.8f * parent->focusColor[0]; 
			lowLight[1] = 0.8f * parent->focusColor[1]; 
			lowLight[2] = 0.8f * parent->focusColor[2]; 
			lowLight[3] = 0.8f * parent->focusColor[3]; 
		}
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
	} 
	else 
	{
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text[0]) 
	{	
		// Somewhat of a hack --eez
		//Item_Text_Paint(item);
		char realText[256];
		textScale = item->textscale;

		if (*item->text == '@')	// string reference
		{
			trap->SE_GetStringTextString( &item->text[1], realText, sizeof(realText));
			//strcpy(item->text, realText);
		}
		textWidth = DC->textWidth(realText, 1.0f, item->iMenuFont);
		textWidth *= textScale;

		if(item->textalignment == ITEM_ALIGN_RIGHT)
		{
			startingXPos = item->window.rect.x - textWidth + item->textalignx;
		}
		else if(item->textalignment == ITEM_ALIGN_CENTER)
		{
			startingXPos = item->window.rect.x - (textWidth/2)+ item->textalignx;
		}
		else
		{
			assert(item->textalignment == ITEM_ALIGN_LEFT);
			startingXPos = item->window.rect.x + item->textalignx;
		}

		DC->drawText(startingXPos, item->window.rect.y/* + yAdj*/ + item->textaligny, textScale, item->window.foreColor, realText, 0, maxChars, item->textStyle,item->iMenuFont);

		BindingFromName(item->cvar);


		// fucks sake raven, learn how to do things RIGHT --eez
		textScale = item->textscale2;
		textWidth = DC->textWidth(g_nameBind1, 1.0f, item->iMenuFont2);
		textWidth *= textScale;
		//startingXPos = (item->textRect.x + item->textRect.w + 8); // <-- arbitrary number?? no.
		if(item->text2alignment == ITEM_ALIGN_LEFT)
		{
			startingXPos = item->window.rect.x + item->text2alignx;
		}
		else if(item->text2alignment == ITEM_ALIGN_RIGHT)
		{
			startingXPos = item->window.rect.x + textWidth + item->text2alignx;
		}
		else if(item->text2alignment == ITEM_ALIGN_CENTER)
		{
			startingXPos = item->window.rect.x + (textWidth/2)+ item->text2alignx;
		}


		/*while ((startingXPos + textWidth) >= SCREEN_WIDTH)
		{
			textScale -= .05f;
			textWidth = DC->textWidth(g_nameBind1,(float) textScale, item->iMenuFont);
		}

		// Try to adjust it's y placement if the scale has changed.
		yAdj = 0;
		if (textScale != item->textscale)
		{
			textHeight = DC->textHeight(g_nameBind1, item->textscale, item->iMenuFont);
			yAdj = textHeight - DC->textHeight(g_nameBind1, textScale, item->iMenuFont);
		}*/
		// Just let it draw however it wants, nazi bastards >_>
		// Draw a background :3
		if( item->window.background )
		{
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, item->window.background );
		}
		DC->drawText(startingXPos, item->window.rect.y/* + yAdj*/ + item->text2aligny, textScale, newColor, g_nameBind1, 0, maxChars, item->textStyle2,item->iMenuFont2);
	} 
	else 
	{
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? "FIXME" : "FIXME", 0, maxChars, item->textStyle,item->iMenuFont);
	}
}

qboolean Display_KeyBindPending() {
	return g_waitingForKey;
}

qboolean Item_Bind_HandleKey(itemDef_t *item, int key, qboolean down) {
	int			id;
	int			i;

	if (key == A_MOUSE1 && Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && !g_waitingForKey)
	{
		if (down) {
			g_waitingForKey = qtrue;
			g_bindItem = item;
		}
		return qtrue;
	}
	else if (key == A_ENTER && !g_waitingForKey)
	{
		if (down) 
		{
			g_waitingForKey = qtrue;
			g_bindItem = item;
		}
		return qtrue;
	}
	else
	{
		if (!g_waitingForKey || g_bindItem == NULL) {
			return qfalse;
		}

		if (key & K_CHAR_FLAG) {
			return qtrue;
		}

		switch (key)
		{
			case A_ESCAPE:
				g_waitingForKey = qfalse;
				return qtrue;
	
			case A_BACKSPACE:
				id = BindingIDFromName(item->cvar);
				if (id != -1) 
				{
					if ( g_bindings[id].bind1 != -1 )
					{
						DC->setBinding ( g_bindings[id].bind1, "" );
					}
					
					if ( g_bindings[id].bind2 != -1 )
					{
						DC->setBinding ( g_bindings[id].bind2, "" );
					}
								
					g_bindings[id].bind1 = -1;
					g_bindings[id].bind2 = -1;
				}
				Controls_SetConfig();
				g_waitingForKey = qfalse;
				g_bindItem = NULL;
				return qtrue;

			case '`':
				return qtrue;
		}
	}

	if (key != -1)
	{

		for (i=0; i < g_bindCount; i++)
		{

			if (g_bindings[i].bind2 == key) {
				g_bindings[i].bind2 = -1;
			}

			if (g_bindings[i].bind1 == key)
			{
				g_bindings[i].bind1 = g_bindings[i].bind2;
				g_bindings[i].bind2 = -1;
			}
		}
	}


	id = BindingIDFromName(item->cvar);

	if (id != -1) {
		if (key == -1) {
			if( g_bindings[id].bind1 != -1 ) {
				DC->setBinding( g_bindings[id].bind1, "" );
				g_bindings[id].bind1 = -1;
			}
			if( g_bindings[id].bind2 != -1 ) {
				DC->setBinding( g_bindings[id].bind2, "" );
				g_bindings[id].bind2 = -1;
			}
		}
		else if (g_bindings[id].bind1 == -1) {
			g_bindings[id].bind1 = key;
		}
		else if (g_bindings[id].bind1 != key && g_bindings[id].bind2 == -1) {
			g_bindings[id].bind2 = key;
		}
		else {
			DC->setBinding( g_bindings[id].bind1, "" );
			DC->setBinding( g_bindings[id].bind2, "" );
			g_bindings[id].bind1 = key;
			g_bindings[id].bind2 = -1;
		}						
	}

	Controls_SetConfig();	
	g_waitingForKey = qfalse;

	return qtrue;
}

void UI_ScaleModelAxis(refEntity_t	*ent)

{		// scale the model should we need to
		if (ent->modelScale[0] && ent->modelScale[0] != 1.0f)
		{
			VectorScale( ent->axis[0], ent->modelScale[0] , ent->axis[0] );
			ent->nonNormalizedAxes = qtrue;
		}
		if (ent->modelScale[1] && ent->modelScale[1] != 1.0f)
		{
			VectorScale( ent->axis[1], ent->modelScale[1] , ent->axis[1] );
			ent->nonNormalizedAxes = qtrue;
		}
		if (ent->modelScale[2] && ent->modelScale[2] != 1.0f)
		{
			VectorScale( ent->axis[2], ent->modelScale[2] , ent->axis[2] );
			ent->nonNormalizedAxes = qtrue;
		}
}

#ifndef _CGAME
extern void UI_SaberAttachToChar( itemDef_t *item );
#endif

void Item_Model_Paint(itemDef_t *item) 
{
	float x, y, w, h;
	refdef_t refdef;
	refEntity_t		ent;
	vec3_t			mins, maxs, origin;
	vec3_t			angles;
	modelDef_t *modelPtr = item->typeData.model;

	if (modelPtr == NULL) 
	{
		return;
	}

	// a moves datapad anim is playing
#ifndef _CGAME
	if (uiInfo.moveAnimTime && (uiInfo.moveAnimTime < uiInfo.uiDC.realTime))
	{
		modelDef_t *modelPtr;
		modelPtr = item->typeData.model;
		if (modelPtr)
		{
			char modelPath[MAX_QPATH];

			Com_sprintf( modelPath, sizeof( modelPath ), "models/players/%s/model.glm", UI_Cvar_VariableString ( "ui_char_model" ) );
			//HACKHACKHACK: check for any multi-part anim sequences, and play the next anim, if needbe
			switch( modelPtr->g2anim )
			{
			case BOTH_FORCEWALLREBOUND_FORWARD:
			case BOTH_FORCEJUMP1: 
				ItemParse_model_g2anim_go( item, animTable[BOTH_FORCEINAIR1].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				if ( !uiInfo.moveAnimTime )
				{
					uiInfo.moveAnimTime = 500;
				}
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_FORCEINAIR1:
				ItemParse_model_g2anim_go( item, animTable[BOTH_FORCELAND1].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_FORCEWALLRUNFLIP_START:
				ItemParse_model_g2anim_go( item, animTable[BOTH_FORCEWALLRUNFLIP_END].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_FORCELONGLEAP_START:
				ItemParse_model_g2anim_go( item, animTable[BOTH_FORCELONGLEAP_LAND].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_KNOCKDOWN3://on front - into force getup
				trap->S_StartLocalSound( uiInfo.uiDC.Assets.moveJumpSound, CHAN_LOCAL );
				ItemParse_model_g2anim_go( item, animTable[BOTH_FORCE_GETUP_F1].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_KNOCKDOWN2://on back - kick forward getup
				trap->S_StartLocalSound( uiInfo.uiDC.Assets.moveJumpSound, CHAN_LOCAL );
				ItemParse_model_g2anim_go( item, animTable[BOTH_GETUP_BROLL_F].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			case BOTH_KNOCKDOWN1://on back - roll-away
				trap->S_StartLocalSound( uiInfo.uiDC.Assets.moveRollSound, CHAN_LOCAL );
				ItemParse_model_g2anim_go( item, animTable[BOTH_GETUP_BROLL_R].name );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime += uiInfo.uiDC.realTime;
				break;
			default:
				ItemParse_model_g2anim_go( item,  uiInfo.movesBaseAnim );
				ItemParse_asset_model_go( item, modelPath, &uiInfo.moveAnimTime );
				uiInfo.moveAnimTime = 0;
				break;
			}

			UI_UpdateCharacterSkin();

			//update saber models
			UI_SaberAttachToChar( item );
		}
	}
#endif

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );

	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );
	x = item->window.rect.x+1;
	y = item->window.rect.y+1;
	w = item->window.rect.w-2;
	h = item->window.rect.h-2;

	refdef.x = x * DC->xscale;
	refdef.y = y * DC->yscale;
	refdef.width = w * DC->xscale;
	refdef.height = h * DC->yscale;

	if (item->ghoul2)
	{ //ghoul2 models don't have bounds, so we have to parse them.
		VectorCopy(modelPtr->g2mins, mins);
		VectorCopy(modelPtr->g2maxs, maxs);

		if (!mins[0] && !mins[1] && !mins[2] &&
			!maxs[0] && !maxs[1] && !maxs[2])
		{ //we'll use defaults then I suppose.
			VectorSet(mins, -16, -16, -24);
			VectorSet(maxs, 16, 16, 32);
		}
	}
	else
	{
		DC->modelBounds( item->asset, mins, maxs );
	}

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );

	// calculate distance so the model nearly fills the box
	if (qtrue) 
	{
		float len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )
		//origin[0] = len / tan(w/2);
	} 
	else 
	{
		origin[0] = item->textscale;
	}
	refdef.fov_x = (modelPtr->fov_x) ? modelPtr->fov_x : (int)((float)refdef.width / 640.0f * 90.0f);
	refdef.fov_y = (modelPtr->fov_y) ? modelPtr->fov_y : atan2( refdef.height, refdef.width / tan( refdef.fov_x / 360 * M_PI ) ) * ( 360 / M_PI );
	
	//refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	//refdef.fov_y = atan2( refdef.height, refdef.width / tan( refdef.fov_x / 360 * M_PI ) ) * ( 360 / M_PI );

	DC->clearScene();

	refdef.time = DC->realTime;

	// add the model

	memset( &ent, 0, sizeof(ent) );

	// use item storage to track
	if ( (item->flags&ITF_ISANYSABER) && !(item->flags&ITF_ISCHARACTER) )
	{//hack to put saber on it's side
		if (modelPtr->rotationSpeed) 
		{
			VectorSet( angles, modelPtr->angle+(float)refdef.time/modelPtr->rotationSpeed, 0, 90 );
		}
		else
		{
			VectorSet( angles, modelPtr->angle, 0, 90 );
		}
	}
	else if (modelPtr->rotationSpeed) 
	{
		VectorSet( angles, 0, modelPtr->angle + (float)refdef.time/modelPtr->rotationSpeed, 0 );
	}
	else 
	{
		VectorSet( angles, 0, modelPtr->angle, 0 );
	}

	AnglesToAxis( angles, ent.axis );

	if (item->ghoul2)
	{
		ent.ghoul2 = item->ghoul2;
		ent.radius = 1000;
		ent.customSkin = modelPtr->g2skin;

		VectorCopy(modelPtr->g2scale, ent.modelScale);
		UI_ScaleModelAxis(&ent);
#ifndef _CGAME
		if ( (item->flags&ITF_ISCHARACTER) )
		{
			ent.shaderRGBA[0] = (byte)ui_char_color_red.integer;
			ent.shaderRGBA[1] = (byte)ui_char_color_green.integer;
			ent.shaderRGBA[2] = (byte)ui_char_color_blue.integer;
			ent.shaderRGBA[3] = 255;
//			UI_TalkingHead(item);
		}
		if ( item->flags&ITF_ISANYSABER )
		{//UGH, draw the saber blade!
			UI_SaberDrawBlades( item, origin, angles );
		}
#endif
	}
	else
	{
		ent.hModel = item->asset;
	}
	VectorCopy( origin, ent.origin );
	VectorCopy( ent.origin, ent.oldorigin );

	// Set up lighting
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

	DC->addRefEntityToScene( &ent );
	DC->renderScene( &refdef );

}


void Item_Image_Paint(itemDef_t *item) {
	if (item == NULL) {
		return;
	}
	DC->drawHandlePic(item->window.rect.x+1, item->window.rect.y+1, item->window.rect.w-2, item->window.rect.h-2, item->asset);
}


void Item_TextScroll_Paint(itemDef_t *item) 
{
	char cvartext[1024];
	float x, y, size, count, thumb;
	int	  i;
	textScrollDef_t *scrollPtr = item->typeData.textscroll;

	count = scrollPtr->iLineCount;

	// draw scrollbar to right side of the window
	x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE - 1;
	y = item->window.rect.y + 1;
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowUp);
	y += SCROLLBAR_SIZE - 1;

	scrollPtr->endPos = scrollPtr->startPos;
	size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, size+1, DC->Assets.scrollBar);
	y += size - 1;
	DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowDown);
	
	// thumb
	thumb = Item_TextScroll_ThumbDrawPosition(item);
	if (thumb > y - SCROLLBAR_SIZE - 1) 
	{
		thumb = y - SCROLLBAR_SIZE - 1;
	}
	DC->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);

	if (item->cvar) 
	{
		DC->getCVarString(item->cvar, cvartext, sizeof(cvartext));
		Q_strncpyz(item->text, cvartext, sizeof(item->text));
		Item_TextScroll_BuildLines ( item );
	}

	// adjust size for item painting
	size = item->window.rect.h - 2;
	x	 = item->window.rect.x + item->textalignx + 1;
	y	 = item->window.rect.y + item->textaligny + 1;

	for (i = scrollPtr->startPos; i < count; i++) 
	{
		const char *text;

		text = scrollPtr->pLines[i];
		if (!text) 
		{
			continue;
		}

		DC->drawText(x + 4, y, item->textscale, item->window.foreColor, text, 0, 0, item->textStyle, item->iMenuFont);

		size -= scrollPtr->lineHeight;
		if (size < scrollPtr->lineHeight) 
		{
			scrollPtr->drawPadding = scrollPtr->lineHeight - size;
			break;
		}

		scrollPtr->endPos++;
		y += scrollPtr->lineHeight;
	}
}

#define COLOR_MAX 255.0f

// Draw routine for list boxes
void Item_ListBox_Paint(itemDef_t *item) {
	float x, y, sizeWidth, count, i, i2,sizeHeight, thumb;
	int startPos;
	qhandle_t image;
	qhandle_t optionalImage1, optionalImage2, optionalImage3;
	listBoxDef_t *listPtr = item->typeData.listbox;
//JLF MPMOVED
	int numlines;


	// the listbox is horizontal or vertical and has a fixed size scroll bar going either direction
	// elements are enumerated from the DC and either text or image handles are acquired from the DC as well
	// textscale is used to size the text, textalignx and textaligny are used to size image elements
	// there is no clipping available so only the last completely visible item is painted
	count = DC->feederCount(item->special);

	if (listPtr->startPos > (count?count-1:count))
	{//probably changed feeders, so reset
		listPtr->startPos = 0;
	}
//JLF END
	if (item->cursorPos > (count?count-1:count))
	{//probably changed feeders, so reset
		item->cursorPos = (count?count-1:count);
		// NOTE : might consider moving this to any spot in here we change the cursor position
		DC->feederSelection( item->special, item->cursorPos, NULL );
	}

	

	// default is vertical if horizontal flag is not here
	if (item->window.flags & WINDOW_HORIZONTAL) 
	{
#ifdef	_DEBUG
		const char *text;
#endif
		numlines = item->window.rect.w / listPtr->elementWidth; //FIXME: Not used?

//JLF new variable (code just indented) MPMOVED
		if (!listPtr->scrollhidden)
		{
		// draw scrollbar in bottom of the window
		// bar
			if (Item_ListBox_MaxScroll(item) > 0) 
			{
				// draw scrollbar to bottom of the window
				x = item->window.rect.x + 1;
				if(item->window.useNewSlider)
				{
					if(item->window.noSliderButtons)
					{
						// HUFF...okay, this is way more difficult.
						y = item->window.rect.y + item->window.rect.h - item->window.sliderThumbHeight - 1;
						DC->drawHandlePic(x, y, item->window.rect.w, item->window.sliderThumbHeight, item->window.sliderBackground);
						listPtr->endPos = listPtr->startPos;
						thumb = Item_ListBox_ThumbDrawPosition(item);
						DC->drawHandlePic(thumb, y, item->window.sliderThumbWidth, item->window.sliderThumbHeight, item->window.foreground);
					}
					else
					{
						y = item->window.rect.y + item->window.rect.h - item->window.sliderThumbHeight - 1;
						DC->drawHandlePic(x, y, item->window.sliderButtonWidth, item->window.sliderButtonHeight, item->window.sliderButtonUpShader);
						x += item->window.sliderButtonWidth - 1;

						listPtr->endPos = listPtr->startPos;
						sizeWidth = item->window.rect.w - (item->window.sliderButtonWidth * 2);
						DC->drawHandlePic(x, y, sizeWidth+1, item->window.sliderThumbHeight, item->window.sliderBackground);
						x += sizeWidth - 1;
						DC->drawHandlePic(x, y, item->window.sliderButtonWidth, item->window.sliderButtonHeight, item->window.sliderButtonDownShader);
						// thumb
						thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
						if (thumb > x - item->window.sliderThumbWidth - 1) {
							thumb = x - item->window.sliderThumbWidth - 1;
						}
						DC->drawHandlePic(thumb, y, item->window.sliderThumbWidth, item->window.sliderThumbHeight, item->window.foreground);
					}
				}
				else
				{
					y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE - 1;
					DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowLeft);
					x += SCROLLBAR_SIZE - 1;

					listPtr->endPos = listPtr->startPos;
					sizeWidth = item->window.rect.w - (SCROLLBAR_SIZE * 2);
					DC->drawHandlePic(x, y, sizeWidth+1, SCROLLBAR_SIZE, DC->Assets.scrollBar);
					x += sizeWidth - 1;
					DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowRight);
					// thumb
					thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
					if (thumb > x - SCROLLBAR_SIZE - 1) {
						thumb = x - SCROLLBAR_SIZE - 1;
					}
					DC->drawHandlePic(thumb, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);
				}
			}
			else if(listPtr->startPos > 0)
			{
				listPtr->startPos = 0;
			}
		}
		else
		{
			//if (Item_ListBox_MaxScroll(item) > 0) 
				listPtr->endPos = listPtr->startPos; //hack hack hack
			//else if(listPtr->startPos > 0)
				//listPtr->startPos = 0;
		}
//JLF end
		//
		//listPtr->endPos = listPtr->startPos;
		sizeWidth = item->window.rect.w - 2;
		sizeHeight = item->window.rect.h - 2;
		// items
		// size contains max available space
		if (listPtr->elementStyle == LISTBOX_IMAGE) 
		{
			// JKG Edit: Correctly calculate spacing
			float spacingW = listPtr->elementSpacingW;
			float spacingH = listPtr->elementSpacingH;
			// Jedi Knight Galaxies edit: Add support for multiple columns and rows
			if ( item->window.rect.h > (listPtr->elementHeight*2) )
			{
				startPos = listPtr->startPos;
				x = item->window.rect.x + 1; //FIXME: proper spacing?
				y = item->window.rect.y + 1;
				// Next row
				for (i2 = startPos; i2 < count; i2++)
				{
					y = item->window.rect.y + 1;
					sizeHeight = item->window.rect.h - 2;
					// Print a row
					for (i = startPos; i < count; i++)
					{


						image = DC->feederItemImage(item->special, i);
						if(image)
						{
							DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
						}

						if (i == item->cursorPos)
						{
							DC->drawRect(x, y, listPtr->elementWidth-1, listPtr->elementHeight-1, item->window.borderSize, item->window.borderColor);
						}

						sizeHeight -= listPtr->elementHeight;
						if (sizeHeight < listPtr->elementHeight)
						{
							listPtr->drawPadding = sizeHeight;
							break;
						}
						y += listPtr->elementHeight + spacingH;
						listPtr->endPos++;
					}

					sizeWidth -= listPtr->elementWidth;
					if (sizeWidth < listPtr->elementWidth)
					{
						listPtr->drawPadding = sizeWidth;
						break;
					}


					listPtr->endPos++;
					startPos = listPtr->endPos;
					x += listPtr->elementWidth + spacingW;

				}
			}
			else
			{
				// fit = 0;
				x = item->window.rect.x + 1;
				y = item->window.rect.y + 1;
				for (i = listPtr->startPos; i < count; i++) 
				{
					// always draw at least one
					// which may overdraw the box if it is too small for the element
					image = DC->feederItemImage(item->special, i);
					if (image) 
					{
	#ifndef _CGAME
						if (item->window.flags & WINDOW_PLAYERCOLOR) 
						{
							vec4_t	color;

							color[0] = ui_char_color_red.integer/COLOR_MAX;
							color[1] = ui_char_color_green.integer/COLOR_MAX;
							color[2] = ui_char_color_blue.integer/COLOR_MAX;
							color[3] = 1.0f;
							DC->setColor(color);
						}
	#endif
						DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
					}

					if (i == item->cursorPos) 
					{
						DC->drawRect(x, y, listPtr->elementWidth-1, listPtr->elementHeight-1, item->window.borderSize, item->window.borderColor);
					}

					sizeWidth -= listPtr->elementWidth;
					if (sizeWidth < listPtr->elementWidth) 
					{
						listPtr->drawPadding = sizeWidth; //listPtr->elementWidth - size;
						break;
					}
					x += listPtr->elementWidth;
					listPtr->endPos++;
					// fit++;
				}
			}
		} 
		else 
		{
			//
		}

#ifdef	_DEBUG
		// Show pic name
		text = DC->feederItemText(item->special, item->cursorPos, 0, &optionalImage1, &optionalImage2, &optionalImage3);
		if (text) 
		{
			DC->drawText(item->window.rect.x, item->window.rect.y+item->window.rect.h, item->textscale, item->window.foreColor, text, 0, 0, item->textStyle, item->iMenuFont);
		}
#endif

	} 
	// A vertical list box
	else 
	{
//JLF MPMOVED
		numlines = item->window.rect.h / listPtr->elementHeight; //FIXME: Not used?
//JLFEND
		//JLF new variable (code idented with if)
		if (!listPtr->scrollhidden)
		{
			if (Item_ListBox_MaxScroll(item) > 0)
			{
				// draw scrollbar to right side of the window
				if( item->window.useNewSlider )
				{
					if( item->window.noSliderButtons )
					{
						if( item->window.floatingSlider )
						{
							// also known as: hi complex slider stuff here...
							// real complex stuff here in fact..
							x = item->window.rect.x + item->window.rect.w - item->window.sliderBackgroundWidth - 1;
							if( item->window.sliderBackgroundHeight > item->window.rect.h )
								y = item->window.rect.y - ((item->window.sliderBackgroundHeight-item->window.rect.h)/2); // :o
							else
								y = item->window.rect.y + 1;
							DC->drawHandlePic(x, y, item->window.sliderBackgroundWidth, item->window.sliderBackgroundHeight, item->window.sliderBackground);

							listPtr->endPos = listPtr->startPos;
							x = item->window.rect.x + item->window.rect.w - item->window.sliderThumbWidth - 1;
							thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
							DC->drawHandlePic(x, thumb, item->window.sliderThumbWidth, item->window.sliderThumbHeight, item->window.foreground);
						}
						else
						{
							// same HUFF as above, this makes things slightly harder to test...
							x = item->window.rect.x + item->window.rect.w - item->window.sliderThumbWidth - 1;
							y = item->window.rect.y + 1;
							DC->drawHandlePic(x, y, item->window.sliderThumbWidth, item->window.rect.h-1, item->window.sliderBackground);

							listPtr->endPos = listPtr->startPos;
							thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
							DC->drawHandlePic(x, thumb, item->window.sliderThumbWidth, item->window.sliderThumbHeight, item->window.foreground);
						}
					}
					else
					{
						x = item->window.rect.x + item->window.rect.w - item->window.sliderButtonWidth - 1;
						y = item->window.rect.y + 1;
						DC->drawHandlePic(x, y, item->window.sliderButtonWidth, item->window.sliderButtonHeight, item->window.sliderButtonUpShader);
						y += item->window.sliderButtonHeight - 1;

						listPtr->endPos = listPtr->startPos;
						sizeHeight = item->window.rect.h - (item->window.sliderButtonHeight * 2);
						DC->drawHandlePic(x, y, item->window.sliderThumbWidth, sizeHeight+1, item->window.sliderBackground);
						y += sizeHeight - 1;
						DC->drawHandlePic(x, y, item->window.sliderButtonWidth, item->window.sliderButtonHeight, item->window.sliderButtonDownShader);
						// thumb
						thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
						if (thumb > y - item->window.sliderButtonHeight - 1) {
							thumb = y - item->window.sliderButtonHeight - 1;
						}
						DC->drawHandlePic(x, thumb, item->window.sliderThumbWidth, item->window.sliderThumbHeight, item->window.foreground);
					}
				}
				else
				{
					x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE - 1;
					y = item->window.rect.y + 1;
					DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowUp);
					y += SCROLLBAR_SIZE - 1;

					listPtr->endPos = listPtr->startPos;
					sizeHeight = item->window.rect.h - (SCROLLBAR_SIZE * 2);
					DC->drawHandlePic(x, y, SCROLLBAR_SIZE, sizeHeight+1, DC->Assets.scrollBar);
					y += sizeHeight - 1;
					DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowDown);
					// thumb
					thumb = Item_ListBox_ThumbDrawPosition(item);//Item_ListBox_ThumbPosition(item);
					if (thumb > y - SCROLLBAR_SIZE - 1) {
						thumb = y - SCROLLBAR_SIZE - 1;
					}
					DC->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);
				}
			}
			else if(listPtr->startPos > 0)
			{
				listPtr->startPos = 0;
			}
		}
//JLF end

		// adjust size for item painting
		sizeWidth = item->window.rect.w - 2;
		sizeHeight = item->window.rect.h - 2;


		if (listPtr->elementStyle == LISTBOX_IMAGE) 
		{
			// Multiple rows and columns (since it's more than twice as wide as an element)
			if ( item->window.rect.w > (listPtr->elementWidth*2) )
			{
				startPos = listPtr->startPos;
				x = item->window.rect.x + 1;
				y = item->window.rect.y + 1;
				// Next row
				for (i2 = startPos; i2 < count; i2++) 
				{
					x = item->window.rect.x + 1;
					sizeWidth = item->window.rect.w - 2;
					// print a row
					for (i = startPos; i < count; i++) 
					{
						// always draw at least one
						// which may overdraw the box if it is too small for the element
						image = DC->feederItemImage(item->special, i);
						if (image) 
						{
		#ifndef _CGAME
							if (item->window.flags & WINDOW_PLAYERCOLOR) 
							{
								vec4_t	color;

								color[0] = ui_char_color_red.integer/COLOR_MAX;
								color[1] = ui_char_color_green.integer/COLOR_MAX;
								color[2] = ui_char_color_blue.integer/COLOR_MAX;
								color[3] = 1.0f;
								DC->setColor(color);
							}
		#endif
							DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
						}

						if (i == item->cursorPos) 
						{
							DC->drawRect(x, y, listPtr->elementWidth-1, listPtr->elementHeight-1, item->window.borderSize, item->window.borderColor);
						}

						sizeWidth -= listPtr->elementWidth;
						if (sizeWidth < listPtr->elementWidth) 
						{
							listPtr->drawPadding = sizeWidth; //listPtr->elementWidth - size;
							break;
						}
						x += listPtr->elementWidth;
						listPtr->endPos++;
					}

					sizeHeight -= listPtr->elementHeight;
					if (sizeHeight < listPtr->elementHeight) 
					{
						listPtr->drawPadding = sizeHeight; //listPtr->elementWidth - size;
						break;
					}
					// NOTE : Is endPos supposed to be valid or not? It was being used as a valid entry but I changed those
					// few spots that were causing bugs
					listPtr->endPos++;
					startPos = listPtr->endPos;
					y += listPtr->elementHeight;

				}
			}
			// single column
			else
			{
				x = item->window.rect.x + 1;
				y = item->window.rect.y + 1;
				for (i = listPtr->startPos; i < count; i++) 
				{
					// always draw at least one
					// which may overdraw the box if it is too small for the element
					image = DC->feederItemImage(item->special, i);
					if (image) 
					{
						DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
					}

					if (i == item->cursorPos) 
					{
						DC->drawRect(x, y, listPtr->elementWidth - 1, listPtr->elementHeight - 1, item->window.borderSize, item->window.borderColor);
					}

					listPtr->endPos++;
					sizeHeight -= listPtr->elementHeight;
					if (sizeHeight < listPtr->elementHeight) 
					{
						listPtr->drawPadding = listPtr->elementHeight - sizeHeight;
						break;
					}
					y += listPtr->elementHeight;
					// fit++;
				}
			}
		} 
		else 
		{
			x = item->window.rect.x + 1;
			y = item->window.rect.y + 1;
//JLF MPMOVED
			y = item->window.rect.y + 1 - listPtr->elementHeight;
			i = listPtr->startPos;

			for (; i < count; i++) 
//JLF END
			{
				const char *text;
				// always draw at least one
				// which may overdraw the box if it is too small for the element
				if (listPtr->numColumns > 0) {
					int j;//, subX = listPtr->elementHeight;

					for (j = 0; j < listPtr->numColumns; j++) 
					{
						char	temp[MAX_STRING_CHARS];
						int imageStartX = listPtr->columnInfo[j].pos;
						text = DC->feederItemText(item->special, i, j, &optionalImage1, &optionalImage2, &optionalImage3);

						if( !text )
						{
							continue;
						}

						if (text[0]=='@')
						{
							trap->SE_GetStringTextString( &text[1]  , temp, sizeof(temp));
							text = temp;
						}

						/*
						if (optionalImage >= 0) {
							DC->drawHandlePic(x + 4 + listPtr->columnInfo[j].pos, y - 1 + listPtr->elementHeight / 2, listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
						} 
						else 
						*/
						if ( text )
						{
//JLF MPMOVED
							int textyOffset;
							textyOffset = 0;
//							DC->drawText(x + 4 + listPtr->columnInfo[j].pos, y + listPtr->elementHeight, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle);
	//WAS LAST						DC->drawText(x + 4 + listPtr->columnInfo[j].pos, y, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, item->iMenuFont);
							DC->drawText(x + 4 + listPtr->columnInfo[j].pos, y + listPtr->elementHeight+ textyOffset + item->textaligny, item->textscale, item->window.foreColor, text, 0,listPtr->columnInfo[j].maxChars, item->textStyle, item->iMenuFont);


//JLF END
						}
						if ( j < listPtr->numColumns - 1 )
						{
							imageStartX = listPtr->columnInfo[j+1].pos;
						}
						DC->setColor( NULL );
						if (optionalImage3 >= 0) 
						{
							DC->drawHandlePic(x + imageStartX - listPtr->elementWidth*3, y+listPtr->elementHeight+2, listPtr->elementWidth, listPtr->elementHeight, optionalImage3);
						} 
						if (optionalImage2 >= 0) 
						{
							DC->drawHandlePic(x + imageStartX - listPtr->elementWidth*2, y+listPtr->elementHeight+2, listPtr->elementWidth, listPtr->elementHeight, optionalImage2);
						} 
						if (optionalImage1 >= 0) 
						{
							DC->drawHandlePic(x + imageStartX - listPtr->elementWidth, y+listPtr->elementHeight+2, listPtr->elementWidth, listPtr->elementHeight, optionalImage1);
						} 
					}
				} 
				else 
				{
					text = DC->feederItemText(item->special, i, 0, &optionalImage1, &optionalImage2, &optionalImage3 );
					if ( optionalImage1 >= 0 || optionalImage2 >= 0 || optionalImage3 >= 0) 
					{
						//DC->drawHandlePic(x + 4 + listPtr->elementHeight, y, listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
					} 
					else if (text) 
					{
//						DC->drawText(x + 4, y + listPtr->elementHeight, item->textscale, item->window.foreColor, text, 0, 0, item->textStyle);
						DC->drawText(x + 4, y + item->textaligny, item->textscale, item->window.foreColor, text, 0, 0, item->textStyle, item->iMenuFont);
					}
				}

				if (i == item->cursorPos) 
				{
					if (!listPtr->scrollhidden) {
						DC->fillRect(x + 2, y + listPtr->elementHeight + 2, item->window.rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, item->window.outlineColor);
					} else {
						DC->fillRect(x + 2, y + listPtr->elementHeight + 2, item->window.rect.w - 6, listPtr->elementHeight, item->window.outlineColor);
					}
				}

				sizeHeight -= listPtr->elementHeight;
				if (sizeHeight < listPtr->elementHeight) 
				{
					listPtr->drawPadding = listPtr->elementHeight - sizeHeight;
					break;
				}
				listPtr->endPos++;
				y += listPtr->elementHeight;
				// fit++;
			}
		}
	}
}


void Item_OwnerDraw_Paint(itemDef_t *item) {
  menuDef_t *parent;

	if (item == NULL) {
		return;
	}
  parent = (menuDef_t*)item->parent;

	if (DC->ownerDrawItem) {
		vec4_t color, lowLight;
		menuDef_t *parent = (menuDef_t*)item->parent;
		Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);
		memcpy(&color, &item->window.foreColor, sizeof(color));
		if (item->numColors > 0 && DC->getValue) {
			// if the value is within one of the ranges then set color to that, otherwise leave at default
			int i;
			float f = DC->getValue(item->window.ownerDraw);
			for (i = 0; i < item->numColors; i++) {
				if (f >= item->colorRanges[i].low && f <= item->colorRanges[i].high) {
					memcpy(&color, &item->colorRanges[i].color, sizeof(color));
					break;
				}
			}
		}

		if (item->window.flags & WINDOW_HASFOCUS) {
			lowLight[0] = 0.8 * parent->focusColor[0]; 
			lowLight[1] = 0.8 * parent->focusColor[1]; 
			lowLight[2] = 0.8 * parent->focusColor[2]; 
			lowLight[3] = 0.8 * parent->focusColor[3]; 
			LerpColor(parent->focusColor,lowLight,color,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
		} else if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) {
			lowLight[0] = 0.8 * item->window.foreColor[0]; 
			lowLight[1] = 0.8 * item->window.foreColor[1]; 
			lowLight[2] = 0.8 * item->window.foreColor[2]; 
			lowLight[3] = 0.8 * item->window.foreColor[3]; 
			LerpColor(item->window.foreColor,lowLight,color,0.5+0.5*sin((float)(DC->realTime / PULSE_DIVISOR)));
		}

		if (item->disabled) 
		{
			memcpy(color, parent->disableColor, sizeof(vec4_t)); // bk001207 - FIXME: Com_Memcpy
		}

		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
		  memcpy(color, parent->disableColor, sizeof(vec4_t)); // bk001207 - FIXME: Com_Memcpy
		}

		if (item->text[0] && item->window.name[0]) {
				Item_Text_Paint(item);
				if (item->text[0]) {
					// +8 is an offset kludge to properly align owner draw items that have text combined with them
					DC->ownerDrawItem(item, item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle,item->iMenuFont, item->window.ownerDrawID);
				} else {
					DC->ownerDrawItem(item, item->textRect.x + item->textRect.w, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle,item->iMenuFont, item->window.ownerDrawID);
				}
			} else {
			DC->ownerDrawItem(item, item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, item->textalignx, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle,item->iMenuFont, item->window.ownerDrawID);
		}
	}
}


void Item_Paint(itemDef_t *item) 
{
	vec4_t		red;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int			xPos,textWidth;
	vec4_t		color = {1, 1, 1, 1};

	red[0] = red[3] = 1;
	red[1] = red[2] = 0;

	if (item == NULL) 
	{
		return;
	}

	if (item->window.flags & WINDOW_ORBITING) 
	{
		if (DC->realTime > item->window.nextTime) 
		{
			float rx, ry, a, c, s, w, h;
      
			item->window.nextTime = DC->realTime + item->window.offsetTime;
			// translate
			w = item->window.rectClient.w / 2;
			h = item->window.rectClient.h / 2;
			rx = item->window.rectClient.x + w - item->window.rectEffects.x;
			ry = item->window.rectClient.y + h - item->window.rectEffects.y;
			a = 3 * M_PI / 180;
			c = cos(a);
			s = sin(a);
			item->window.rectClient.x = (rx * c - ry * s) + item->window.rectEffects.x - w;
			item->window.rectClient.y = (rx * s + ry * c) + item->window.rectEffects.y - h;
			Item_UpdatePosition(item);
		}
	}


	if (item->window.flags & WINDOW_INTRANSITION) 
	{
		if (DC->realTime > item->window.nextTime) 
		{
			int done = 0;
			item->window.nextTime = DC->realTime + item->window.offsetTime;

			// transition the x,y
			if (item->window.rectClient.x == item->window.rectEffects.x) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.x < item->window.rectEffects.x) 
				{
					item->window.rectClient.x += item->window.rectEffects2.x;
					if (item->window.rectClient.x > item->window.rectEffects.x) 
					{
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.x -= item->window.rectEffects2.x;
					if (item->window.rectClient.x < item->window.rectEffects.x) 
					{
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				}
			}

			if (item->window.rectClient.y == item->window.rectEffects.y) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.y < item->window.rectEffects.y) 
				{
					item->window.rectClient.y += item->window.rectEffects2.y;
					if (item->window.rectClient.y > item->window.rectEffects.y) 
					{
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.y -= item->window.rectEffects2.y;
					if (item->window.rectClient.y < item->window.rectEffects.y) 
					{
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				}
			}

			if (item->window.rectClient.w == item->window.rectEffects.w) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.w < item->window.rectEffects.w) 
				{
					item->window.rectClient.w += item->window.rectEffects2.w;
					if (item->window.rectClient.w > item->window.rectEffects.w) 
					{
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.w -= item->window.rectEffects2.w;
					if (item->window.rectClient.w < item->window.rectEffects.w) 
					{
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				}
			}

			if (item->window.rectClient.h == item->window.rectEffects.h) 
			{
				done++;
			} 
			else 
			{
				if (item->window.rectClient.h < item->window.rectEffects.h) 
				{
					item->window.rectClient.h += item->window.rectEffects2.h;
					if (item->window.rectClient.h > item->window.rectEffects.h) 
					{
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				} 
				else 
				{
					item->window.rectClient.h -= item->window.rectEffects2.h;
					if (item->window.rectClient.h < item->window.rectEffects.h) 
					{
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				}
			}

			Item_UpdatePosition(item);

			if (done == 4) 
			{
				item->window.flags &= ~WINDOW_INTRANSITION;
			}

		}
	}

#ifdef _TRANS3
	
//JLF begin model transition stuff
	if (item->window.flags & WINDOW_INTRANSITIONMODEL) 
	{
		if ( item->type == ITEM_TYPE_MODEL)
		{
//fields ing modelptr
//				vec3_t g2mins2, g2maxs2, g2minsEffect, g2maxsEffect;
//	float fov_x2, fov_y2, fov_Effectx, fov_Effecty;

			modelDef_t  * modelptr = item->typeData.model;

			if (DC->realTime > item->window.nextTime) 
			{
				int done = 0;
				item->window.nextTime = DC->realTime + item->window.offsetTime;


// transition the x,y,z max
				if (modelptr->g2maxs[0] == modelptr->g2maxs2[0]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2maxs[0] < modelptr->g2maxs2[0]) 
					{
						modelptr->g2maxs[0] += modelptr->g2maxsEffect[0];
						if (modelptr->g2maxs[0] > modelptr->g2maxs2[0]) 
						{
							modelptr->g2maxs[0] = modelptr->g2maxs2[0];
							done++;
						}
					} 
					else 
					{
						modelptr->g2maxs[0] -= modelptr->g2maxsEffect[0];
						if (modelptr->g2maxs[0] < modelptr->g2maxs2[0]) 
						{
							modelptr->g2maxs[0] = modelptr->g2maxs2[0];
							done++;
						}
					}
				}
//y
				if (modelptr->g2maxs[1] == modelptr->g2maxs2[1]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2maxs[1] < modelptr->g2maxs2[1]) 
					{
						modelptr->g2maxs[1] += modelptr->g2maxsEffect[1];
						if (modelptr->g2maxs[1] > modelptr->g2maxs2[1]) 
						{
							modelptr->g2maxs[1] = modelptr->g2maxs2[1];
							done++;
						}
					} 
					else 
					{
						modelptr->g2maxs[1] -= modelptr->g2maxsEffect[1];
						if (modelptr->g2maxs[1] < modelptr->g2maxs2[1]) 
						{
							modelptr->g2maxs[1] = modelptr->g2maxs2[1];
							done++;
						}
					}
				}


//z

				if (modelptr->g2maxs[2] == modelptr->g2maxs2[2]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2maxs[2] < modelptr->g2maxs2[2]) 
					{
						modelptr->g2maxs[2] += modelptr->g2maxsEffect[2];
						if (modelptr->g2maxs[2] > modelptr->g2maxs2[2]) 
						{
							modelptr->g2maxs[2] = modelptr->g2maxs2[2];
							done++;
						}
					} 
					else 
					{
						modelptr->g2maxs[2] -= modelptr->g2maxsEffect[2];
						if (modelptr->g2maxs[2] < modelptr->g2maxs2[2]) 
						{
							modelptr->g2maxs[2] = modelptr->g2maxs2[2];
							done++;
						}
					}
				}

// transition the x,y,z min
				if (modelptr->g2mins[0] == modelptr->g2mins2[0]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2mins[0] < modelptr->g2mins2[0]) 
					{
						modelptr->g2mins[0] += modelptr->g2minsEffect[0];
						if (modelptr->g2mins[0] > modelptr->g2mins2[0]) 
						{
							modelptr->g2mins[0] = modelptr->g2mins2[0];
							done++;
						}
					} 
					else 
					{
						modelptr->g2mins[0] -= modelptr->g2minsEffect[0];
						if (modelptr->g2mins[0] < modelptr->g2mins2[0]) 
						{
							modelptr->g2mins[0] = modelptr->g2mins2[0];
							done++;
						}
					}
				}
//y
				if (modelptr->g2mins[1] == modelptr->g2mins2[1]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2mins[1] < modelptr->g2mins2[1]) 
					{
						modelptr->g2mins[1] += modelptr->g2minsEffect[1];
						if (modelptr->g2mins[1] > modelptr->g2mins2[1]) 
						{
							modelptr->g2mins[1] = modelptr->g2mins2[1];
							done++;
						}
					} 
					else 
					{
						modelptr->g2mins[1] -= modelptr->g2minsEffect[1];
						if (modelptr->g2mins[1] < modelptr->g2mins2[1]) 
						{
							modelptr->g2mins[1] = modelptr->g2mins2[1];
							done++;
						}
					}
				}


//z

				if (modelptr->g2mins[2] == modelptr->g2mins2[2]) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->g2mins[2] < modelptr->g2mins2[2]) 
					{
						modelptr->g2mins[2] += modelptr->g2minsEffect[2];
						if (modelptr->g2mins[2] > modelptr->g2mins2[2]) 
						{
							modelptr->g2mins[2] = modelptr->g2mins2[2];
							done++;
						}
					} 
					else 
					{
						modelptr->g2mins[2] -= modelptr->g2minsEffect[2];
						if (modelptr->g2mins[2] < modelptr->g2mins2[2]) 
						{
							modelptr->g2mins[2] = modelptr->g2mins2[2];
							done++;
						}
					}
				}



//fovx
				if (modelptr->fov_x == modelptr->fov_x2) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->fov_x < modelptr->fov_x2) 
					{
						modelptr->fov_x += modelptr->fov_Effectx;
						if (modelptr->fov_x > modelptr->fov_x2) 
						{
							modelptr->fov_x = modelptr->fov_x2;
							done++;
						}
					} 
					else 
					{
						modelptr->fov_x -= modelptr->fov_Effectx;
						if (modelptr->fov_x < modelptr->fov_x2) 
						{
							modelptr->fov_x = modelptr->fov_x2;
							done++;
						}
					}
				}

//fovy
				if (modelptr->fov_y == modelptr->fov_y2) 
				{
					done++;
				} 
				else 
				{
					if (modelptr->fov_y < modelptr->fov_y2) 
					{
						modelptr->fov_y += modelptr->fov_Effecty;
						if (modelptr->fov_y > modelptr->fov_y2) 
						{
							modelptr->fov_y = modelptr->fov_y2;
							done++;
						}
					} 
					else 
					{
						modelptr->fov_y -= modelptr->fov_Effecty;
						if (modelptr->fov_y < modelptr->fov_y2) 
						{
							modelptr->fov_y = modelptr->fov_y2;
							done++;
						}
					}
				}

				if (done == 5) 
				{
					item->window.flags &= ~WINDOW_INTRANSITIONMODEL;
				}

			}
		}
	}
#endif
//JLF end transition stuff for models



	if (item->window.ownerDrawFlags && DC->ownerDrawVisible) {
		if (!DC->ownerDrawVisible(item->window.ownerDrawFlags)) {
			item->window.flags &= ~WINDOW_VISIBLE;
		} else {
			item->window.flags |= WINDOW_VISIBLE;
		}
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE)) {
		if (!Item_EnableShowViaCvar(item, CVAR_SHOW)) {
			return;
		}
	}

  if (item->window.flags & WINDOW_TIMEDVISIBLE) {

	}

	if (!(item->window.flags & WINDOW_VISIBLE)) 
	{
		return;
	}


	if (item->window.flags & WINDOW_MOUSEOVER)
	{
		if (item->descText && !Display_KeyBindPending())
		{
			// Make DOUBLY sure that this item should have desctext.
			// NOTE : we can't just check the mouse position on this, what if we TABBED
			// to the current menu item -- in that case our mouse isn't over the item.
			// Removing the WINDOW_MOUSEOVER flag just prevents the item's OnExit script from running
	//	    if (!Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) 
	//		{	// It isn't something that should, because it isn't live anymore.
	//			item->window.flags &= ~WINDOW_MOUSEOVER;
	//		}
	//		else
			{	// Draw the desctext
				const char *textPtr = item->descText;
				if (*textPtr == '@')	// string reference
				{
					char	temp[MAX_STRING_CHARS];
					trap->SE_GetStringTextString( &textPtr[1]  , temp, sizeof(temp));
					textPtr = temp;
				}

				Item_TextColor(item, &color);

				{// stupid C language
					float fDescScale = parent->descScale ? parent->descScale : 1;
					float fDescScaleCopy = fDescScale;
					int iYadj = 0;
					while (1)
					{
						textWidth = DC->textWidth(textPtr,fDescScale, FONT_SMALL2);

						if (parent->descAlignment == ITEM_ALIGN_RIGHT)
						{
							xPos = parent->descX - textWidth;	// Right justify
						}
						else if (parent->descAlignment == ITEM_ALIGN_CENTER)
						{
							xPos = parent->descX - (textWidth/2);	// Center justify
						}
						else										// Left justify	
						{
							xPos = parent->descX;
						}

						if (parent->descAlignment == ITEM_ALIGN_CENTER)
						{
							// only this one will auto-shrink the scale until we eventually fit...
							//
							if (xPos + textWidth > (SCREEN_WIDTH-4)) {
								fDescScale -= 0.001f;
								continue;
							}
						}

						// Try to adjust it's y placement if the scale has changed...
						//
						if (fDescScale != fDescScaleCopy)
						{
							int iOriginalTextHeight = DC->textHeight(textPtr, fDescScaleCopy, FONT_MEDIUM);
							iYadj = iOriginalTextHeight - DC->textHeight(textPtr, fDescScale, FONT_MEDIUM);
						}

						DC->drawText(xPos, parent->descY + iYadj, fDescScale, parent->descColor, textPtr, 0, 0, item->textStyle, FONT_SMALL2);
						break;
					}
				}
			}
		}
	}

	// paint the rect first.. 
	Window_Paint(&item->window, parent->fadeAmount , parent->fadeClamp, parent->fadeCycle);

	// Draw box to show rectangle extents, in debug mode
	if (debugMode) 
	{
		vec4_t color;
		color[1] = color[3] = 1;
		color[0] = color[2] = 0;
		DC->drawRect(
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			1, 
			color);
	}

	//DC->drawRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 1, red);

	switch (item->type) {
	case ITEM_TYPE_OWNERDRAW:
		Item_OwnerDraw_Paint(item);
		break;
	case ITEM_TYPE_TEXT:
	case ITEM_TYPE_BUTTON:
		Item_Text_Paint(item);
		break;
	case ITEM_TYPE_RADIOBUTTON:
		break;
	case ITEM_TYPE_CHECKBOX:
		break;
	case ITEM_TYPE_EDITFIELD:
	case ITEM_TYPE_NUMERICFIELD:
		Item_TextField_Paint(item);
		break;
	case ITEM_TYPE_COMBO:
		break;
	case ITEM_TYPE_LISTBOX:
		Item_ListBox_Paint(item);
		break;
	case ITEM_TYPE_TEXTSCROLL:
		Item_TextScroll_Paint ( item );
		break;
		//case ITEM_TYPE_IMAGE:
		//  Item_Image_Paint(item);
		//  break;
	case ITEM_TYPE_MODEL:
		Item_Model_Paint(item);
		break;
	case ITEM_TYPE_YESNO:
		Item_YesNo_Paint(item);
		break;
	case ITEM_TYPE_MULTI:
		Item_Multi_Paint(item);
		break;
	case ITEM_TYPE_BIND:
		Item_Bind_Paint(item);
		break;
	case ITEM_TYPE_BIND_FIXED:
		// Jedi Knight Galaxies, fixed bind text
		Item_Bind_Fixed_Paint(item);
		break;
	case ITEM_TYPE_SLIDER:
		Item_Slider_Paint(item);
		break;
	default:
		break;
	}

	//Raz: Ran into an issue where ITEM_TYPE_MULTI wasn't resetting the color
	//		resulting in successive elements being tinted
	DC->setColor( NULL );
}

void Menu_Init(menuDef_t *menu) {
	memset(menu, 0, sizeof(menuDef_t));
	menu->cursorItem = -1;
	menu->fadeAmount = DC->Assets.fadeAmount;
	menu->fadeClamp = DC->Assets.fadeClamp;
	menu->fadeCycle = DC->Assets.fadeCycle;
	Window_Init(&menu->window);
}

itemDef_t *Menu_GetFocusedItem(menuDef_t *menu) {
	int i;
	if (menu) {
		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
				return menu->items[i];
			}
		}
	}
	return NULL;
}

menuDef_t *Menu_GetFocused() {
	int i;
	for (i = 0; i < menuCount; i++) {
		if (Menus[i].window.flags & WINDOW_HASFOCUS && Menus[i].window.flags & WINDOW_VISIBLE) {
			return &Menus[i];
		}
	}
	return NULL;
}

void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down) {
	if (menu) {
		int i;
		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder) {
				Item_ListBox_HandleKey(menu->items[i], (down) ? A_CURSOR_DOWN : A_CURSOR_UP, qtrue, qtrue);
				return;
			}
		}
	}
}



void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index, const char *name) {
	if (menu == NULL) {
		if (name == NULL) {
			menu = Menu_GetFocused();
		} else {
			menu = Menus_FindByName(name);
		}
	}

	if (menu) {
		int i;
		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder) {
				if (index == 0) {
					listBoxDef_t *listPtr = menu->items[i]->typeData.listbox;
					listPtr->cursorPos = 0;
					listPtr->startPos = 0;
				}
				menu->items[i]->cursorPos = index;
				DC->feederSelection(menu->items[i]->special, menu->items[i]->cursorPos, NULL);
				return;
			}
		}
	}
}

qboolean Menus_AnyFullScreenVisible() {
  int i;
  for (i = 0; i < menuCount; i++) {
    if (Menus[i].window.flags & WINDOW_VISIBLE && Menus[i].fullScreen) {
			return qtrue;
    }
  }
  return qfalse;
}

menuDef_t *Menus_ActivateByName(const char *p) {
  int i;
  menuDef_t *m = NULL;
	menuDef_t *focus = Menu_GetFocused();
  for (i = 0; i < menuCount; i++) {
    if (Q_stricmp(Menus[i].window.name, p) == 0) {
	    m = &Menus[i];
			Menus_Activate(m);
			if (openMenuCount < MAX_OPEN_MENUS && focus != NULL) {
				menuStack[openMenuCount++] = focus;
			}
    } else {
      Menus[i].window.flags &= ~WINDOW_HASFOCUS;
    }
  }
	Display_CloseCinematics();

	// Want to handle a mouse move on the new menu in case your already over an item
	Menu_HandleMouseMove ( m, DC->cursorx, DC->cursory );

	return m;
}


void Item_Init(itemDef_t *item) {
	memset(item, 0, sizeof(itemDef_t));
	item->textscale = 0.55f;
	Window_Init(&item->window);
}

void Menu_HandleMouseMove(menuDef_t *menu, float x, float y) {
  int i, pass;
  qboolean focusSet = qfalse;

  itemDef_t *overItem;
  if (menu == NULL) {
    return;
  }

  if (!(menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
    return;
  }

	if (itemCapture) {
		return;
	}

	if (g_waitingForKey || g_editingField) {
		return;
	}

  // FIXME: this is the whole issue of focus vs. mouse over.. 
  // need a better overall solution as i don't like going through everything twice
  for (pass = 0; pass < 2; pass++) {
    for (i = 0; i < menu->itemCount; i++) {
      // turn off focus each item
      // menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

      if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
        continue;
      }

			if (menu->items[i]->disabled) 
			{
				continue;
			}

			// items can be enabled and disabled based on cvars
			if (menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE)) {
				continue;
			}

			if (menu->items[i]->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_SHOW)) {
				continue;
			}


			if( menu->items[i]->window.flags & WINDOW_FORCESELECTIONZONE &&
				Rect_ContainsPoint(&menu->items[i]->window.selectionZone, x, y) &&
				menu->items[i]->type != ITEM_TYPE_SLIDER)
			{
				goto gohere;
			}
			if (menu->items[i]->type == ITEM_TYPE_SLIDER) {
				qRectangle test;
				test.x = menu->items[i]->textRect.x + menu->items[i]->textRect.w + 8;
				test.y = menu->items[i]->window.rect.y;
				test.w = menu->items[i]->window.rect.w + menu->items[i]->textRect.w;
				test.h = menu->items[i]->window.rect.h;
				if (Rect_ContainsPoint(&test, x, y)) {
					goto gohere;
				}
			}
            else if (!(menu->items[i]->window.flags & WINDOW_FORCESELECTIONZONE) && (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y))) {
gohere:
				if (pass == 1) {
					overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text[0]) {
						if (!Rect_ContainsPoint(&overItem->window.rect, x, y)) {
							continue;
						}
					}
					// if we are over an item
					if (IsVisible(overItem->window.flags)) {
						// different one
						Item_MouseEnter(overItem, x, y);
						// Item_SetMouseOver(overItem, qtrue);

						// if item is not a decoration see if it can take focus
						if (!focusSet) {
							focusSet = Item_SetFocus(overItem, x, y);
						}
					}
				}
			 } else if (menu->items[i]->window.flags & WINDOW_MOUSEOVER) {
				Item_MouseLeave(menu->items[i]);
				Item_SetMouseOver(menu->items[i], qfalse);
			}
    }
  }

}

void Menu_Paint(menuDef_t *menu, qboolean forcePaint) {
	int i;

	if (menu == NULL) {
		return;
	}

	if (!(menu->window.flags & WINDOW_VISIBLE) &&  !forcePaint) {
		return;
	}

	if (menu->window.ownerDrawFlags && DC->ownerDrawVisible && !DC->ownerDrawVisible(menu->window.ownerDrawFlags)) {
		return;
	}
	
	if (forcePaint) {
		menu->window.flags |= WINDOW_FORCED;
	}

	// draw the background if necessary
	if (menu->fullScreen) {
		// implies a background shader
		// FIXME: make sure we have a default shader if fullscreen is set with no background
		DC->drawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, menu->window.background );
	} else if (menu->window.background) {
		// this allows a background shader without being full screen
		//UI_DrawHandlePic(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, menu->backgroundShader);
	}

	// paint the background and or border
	Window_Paint(&menu->window, menu->fadeAmount, menu->fadeClamp, menu->fadeCycle );

	// Loop through all items for the menu and paint them
	for (i = 0; i < menu->itemCount; i++) 
	{
		if (!menu->items[i]->appearanceSlot)
		{
			Item_Paint(menu->items[i]);
		}
		else // Timed order of appearance
		{
			if (menu->appearanceTime < DC->realTime)	// Time to show another item
			{
				menu->appearanceTime = DC->realTime + menu->appearanceIncrement;
				menu->appearanceCnt++;
			}

			if (menu->items[i]->appearanceSlot<=menu->appearanceCnt)
			{
				Item_Paint(menu->items[i]);
			}
		}
	}

	if (debugMode) {
		vec4_t color;
		color[0] = color[2] = color[3] = 1;
		color[1] = 0;
		DC->drawRect(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, 1, color);
	}
}

/*
===============
Item_ValidateTypeData
===============
*/
void Item_ValidateTypeData(itemDef_t *item) 
{
	if (item->typeData.data) 
	{
		return;
	}
	// Jedi Knight Galaxies
	// Edit, dont do this by default, only if the text has the dynamictext key
	/*if (item->type == ITEM_TYPE_TEXT)
	{
		item->typeData = UI_Alloc(sizeof(textDef_t));
		memset(item->typeData, 0, sizeof(textDef_t));
	}
	else*/
	if (item->type == ITEM_TYPE_LISTBOX) 
	{
		item->typeData.listbox = (listBoxDef_t *)UI_Alloc(sizeof(listBoxDef_t));
		memset(item->typeData.listbox, 0, sizeof(listBoxDef_t));
	}
	else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD || item->type == ITEM_TYPE_YESNO || item->type == ITEM_TYPE_BIND || item->type == ITEM_TYPE_SLIDER || item->type == ITEM_TYPE_TEXT) 
	{
		item->typeData.edit = (editFieldDef_t *)UI_Alloc(sizeof(editFieldDef_t));
		memset(item->typeData.edit, 0, sizeof(editFieldDef_t));
		if (item->type == ITEM_TYPE_EDITFIELD) 
		{
			if (!item->typeData.edit->maxPaintChars) 
			{
				item->typeData.edit->maxPaintChars = MAX_EDITFIELD;
			}
		}
	} 
	else if (item->type == ITEM_TYPE_MULTI) 
	{
		item->typeData.multi = (multiDef_t *)UI_Alloc(sizeof(multiDef_t));
	} 
	else if (item->type == ITEM_TYPE_MODEL) 
	{
		item->typeData.model = (modelDef_t *)UI_Alloc(sizeof(modelDef_t));
		memset(item->typeData.model, 0, sizeof(modelDef_t));
	}
	else if (item->type == ITEM_TYPE_TEXTSCROLL )
	{
		item->typeData.textscroll = (textScrollDef_t *)UI_Alloc(sizeof(textScrollDef_t));
	}
}

/*
===============
Keyword Hash
===============
*/

#define KEYWORDHASH_SIZE	512

typedef struct keywordHash_s
{
	const char *keyword;
	qboolean (*func)(itemDef_t *item, int handle);
	struct keywordHash_s *next;
} keywordHash_t;

int KeywordHash_Key(const char *keyword) {
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

void KeywordHash_Add(keywordHash_t *table[], keywordHash_t *key) {
	int hash;

	hash = KeywordHash_Key(key->keyword);
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

keywordHash_t *KeywordHash_Find(keywordHash_t *table[], const char *keyword)
{
	keywordHash_t *key;
	int hash;

	hash = KeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return NULL;
}

/*
===============
Item Keyword Parse functions
===============
*/

// name <string>
qboolean ItemParse_name( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.name)) {
		return qfalse;
	}
	return qtrue;
}

// name <string>
qboolean ItemParse_focusSound( itemDef_t *item, int handle ) {
	pc_token_t token;
	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	item->focusSound = DC->registerSound(token.string);
	return qtrue;
}


// text <string>
qboolean ItemParse_text( itemDef_t *item, int handle ) {
	char *text = item->text;
	if (!PC_String_Parse(handle, const_cast<const char **>(&text))) {
		return qfalse;
	}
	strcpy(item->text, text);
	return qtrue;
}

/*
===============
ItemParse_descText 
	text <string>
===============
*/
qboolean ItemParse_descText( itemDef_t *item, int handle) 
{

	if (!PC_String_Parse(handle, &item->descText))
	{
		return qfalse;
	}

	return qtrue;

}


/*
===============
ItemParse_text 
	text <string>
===============
*/
qboolean ItemParse_text2( itemDef_t *item, int handle) 
{
	char *text2 = item->text2;
	if (!PC_String_Parse(handle, const_cast<const char **>(&text2)))
	{
		return qfalse;
	}
	strcpy(item->text2, text2);
	return qtrue;

}

/*
===============
ItemParse_text2alignx 
===============
*/
qboolean ItemParse_text2alignx( itemDef_t *item, int handle) 
{
	if (!PC_Screenspace_Parse(handle, &item->text2alignx, qtrue)) 
	{
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_text2aligny 
===============
*/
qboolean ItemParse_text2aligny( itemDef_t *item, int handle) 
{
	if (!PC_Screenspace_Parse(handle, &item->text2aligny, qfalse)) 
	{
		return qfalse;
	}
	return qtrue;
}

// group <string>
qboolean ItemParse_group( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.group)) {
		return qfalse;
	}
	return qtrue;
}

typedef struct uiG2PtrTracker_s uiG2PtrTracker_t;

struct uiG2PtrTracker_s
{
	void *ghoul2;
	uiG2PtrTracker_t *next;
};

uiG2PtrTracker_t *ui_G2PtrTracker = NULL;

//rww - UI G2 shared management functions.

//Insert the pointer into our chain so we can keep track of it for freeing.
void UI_InsertG2Pointer(void *ghoul2)
{
	uiG2PtrTracker_t **nextFree = &ui_G2PtrTracker;

	while ((*nextFree) && (*nextFree)->ghoul2)
	{ //check if it has a ghoul2, if not we can reuse it.
		nextFree = &((*nextFree)->next);
	}

	if (!nextFree)
	{ //shouldn't happen
		assert(0);
		return;
	}

	if (!(*nextFree))
	{ //if we aren't reusing a chain then allocate space for it.
		(*nextFree) = (uiG2PtrTracker_t *)malloc(sizeof(uiG2PtrTracker_t));
		(*nextFree)->next = NULL;
	}

	(*nextFree)->ghoul2 = ghoul2;
}

//Remove a ghoul2 pointer from the chain if it's there.
void UI_ClearG2Pointer(void *ghoul2)
{
	uiG2PtrTracker_t *next = ui_G2PtrTracker;

	if (!ghoul2)
	{
		return;
	}

	while (next)
	{
		if (next->ghoul2 == ghoul2)
		{ //found it, set it to null so we can reuse this link.
			next->ghoul2 = NULL;
			break;
		}

		next = next->next;
	}
}

//Called on shutdown, cleans up all the ghoul2 instances laying around.
void UI_CleanupGhoul2(void)
{
	uiG2PtrTracker_t *next = ui_G2PtrTracker;

	while (next)
	{
		if (next->ghoul2 && trap->G2_HaveWeGhoul2Models(next->ghoul2))
		{ //found a g2 instance, clean it.
			trap->G2API_CleanGhoul2Models(&next->ghoul2);
		}

		next = next->next;
	}
}

// asset_model <string>
int UI_ParseAnimationFile(const char *filename, animation_t *animset, qboolean isHumanoid);

/*
===============
ItemParse_asset_model 
	asset_model <string>
===============
*/
qboolean ItemParse_asset_model_go( itemDef_t *item, const char *name,int *runTimeLength ) 
{
#ifndef _CGAME
	int g2Model;
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;
	*runTimeLength =0.0f;

	if (!Q_stricmp(&name[strlen(name) - 4], ".glm"))
	{ //it's a ghoul2 model then
		if ( item->ghoul2 )
		{
			UI_ClearG2Pointer(item->ghoul2);	//remove from tracking list
			trap->G2API_CleanGhoul2Models(&item->ghoul2);	//remove ghoul info
			item->flags &= ~ITF_G2VALID;
		}

		g2Model = trap->G2API_InitGhoul2Model(&item->ghoul2, name, 0, modelPtr->g2skin, 0, 0, 0);
		if (g2Model >= 0)
		{
			UI_InsertG2Pointer(item->ghoul2); //remember it so we can free it when the ui shuts down.
			item->flags |= ITF_G2VALID;

			if (modelPtr->g2anim)
			{ //does the menu request this model be playing an animation?
//					DC->g2hilev_SetAnim(&item->ghoul2[0], "model_root", modelPtr->g2anim);

				char GLAName[MAX_QPATH];	

				GLAName[0] = 0;
				trap->G2API_GetGLAName(item->ghoul2, 0, GLAName);

				if (GLAName[0])
				{
					int animIndex;
					char *slash;

					slash = Q_strrchr( GLAName, '/' );

					if ( slash )
					{ //If this isn't true the gla path must be messed up somehow.
						strcpy(slash, "/animation.cfg");
					
						animIndex = UI_ParseAnimationFile(GLAName, NULL, qfalse);
						if (animIndex != -1)
						{ //We parsed out the animation info for whatever model this is
							animation_t *anim = &bgAllAnims[animIndex].anims[modelPtr->g2anim];

							int sFrame = anim->firstFrame;
							int eFrame = anim->firstFrame + anim->numFrames;
							int flags = BONE_ANIM_OVERRIDE_FREEZE;
							int time = DC->realTime;
							float animSpeed = 50.0f / anim->frameLerp;
							int blendTime = 150;

							if (anim->loopFrames != -1)
							{
								flags |= BONE_ANIM_OVERRIDE_LOOP;
							}

							trap->G2API_SetBoneAnim(item->ghoul2, 0, "model_root", sFrame, eFrame, flags, animSpeed, time, -1, blendTime);
							*runTimeLength =((anim->frameLerp * (anim->numFrames-2)));					
						}
					}
				} 
			}

			if ( modelPtr->g2skin )
			{
//					DC->g2_SetSkin( &item->ghoul2[0], 0, modelPtr->g2skin );//this is going to set the surfs on/off matching the skin file
				//trap->G2API_InitGhoul2Model(&item->ghoul2, name, 0, modelPtr->g2skin, 0, 0, 0);
				//ahh, what are you doing?!
				trap->G2API_SetSkin(item->ghoul2, 0, modelPtr->g2skin, modelPtr->g2skin);
			}
		}
		/*
		else
		{
			Com_Error(ERR_FATAL, "%s does not exist.", name);
		}
		*/
	}
	else if(!(item->asset)) 
	{ //guess it's just an md3
		item->asset = DC->registerModel(name);
		item->flags &= ~ITF_G2VALID;
	}
#endif
	return qtrue;
}

qboolean ItemParse_asset_model( itemDef_t *item, int handle ) {
	const char *temp;
	modelDef_t *modelPtr;
	int animRunLength;
	pc_token_t token;

	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	temp = token.string;

#ifndef _CGAME
	if (!Q_stricmp(token.string,"ui_char_model") )
	{
		char modelPath[MAX_QPATH];
		char ui_char_model[MAX_QPATH];
		trap->Cvar_VariableStringBuffer("ui_char_model", ui_char_model, sizeof(ui_char_model) );
		Com_sprintf( modelPath, sizeof( modelPath ), "models/players/%s/model.glm", ui_char_model );
		temp = modelPath;
	}
#endif
	return (ItemParse_asset_model_go( item, temp, &animRunLength ));
}

// asset_shader <string>
qboolean ItemParse_asset_shader( itemDef_t *item, int handle ) {
	pc_token_t token;
	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	item->asset = DC->registerShaderNoMip(token.string);
	return qtrue;
}

// model_origin <number> <number> <number>
qboolean ItemParse_model_origin( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (PC_Float_Parse(handle, &modelPtr->origin[0])) {
		if (PC_Float_Parse(handle, &modelPtr->origin[1])) {
			if (PC_Float_Parse(handle, &modelPtr->origin[2])) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

// model_fovx <number>
qboolean ItemParse_model_fovx( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!PC_Float_Parse(handle, &modelPtr->fov_x)) {
		return qfalse;
	}
	return qtrue;
}

// model_fovy <number>
qboolean ItemParse_model_fovy( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!PC_Float_Parse(handle, &modelPtr->fov_y)) {
		return qfalse;
	}
	return qtrue;
}

// model_rotation <integer>
qboolean ItemParse_model_rotation( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!PC_Int_Parse(handle, &modelPtr->rotationSpeed)) {
		return qfalse;
	}
	return qtrue;
}

// model_angle <integer>
qboolean ItemParse_model_angle( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!PC_Int_Parse(handle, &modelPtr->angle)) {
		return qfalse;
	}
	return qtrue;
}

// model_g2mins <number> <number> <number>
qboolean ItemParse_model_g2mins( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (PC_Float_Parse(handle, &modelPtr->g2mins[0])) {
		if (PC_Float_Parse(handle, &modelPtr->g2mins[1])) {
			if (PC_Float_Parse(handle, &modelPtr->g2mins[2])) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

// model_g2maxs <number> <number> <number>
qboolean ItemParse_model_g2maxs( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (PC_Float_Parse(handle, &modelPtr->g2maxs[0])) {
		if (PC_Float_Parse(handle, &modelPtr->g2maxs[1])) {
			if (PC_Float_Parse(handle, &modelPtr->g2maxs[2])) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

// model_g2scale <number> <number> <number>
qboolean ItemParse_model_g2scale( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (PC_Float_Parse(handle, &modelPtr->g2scale[0])) {
		if (PC_Float_Parse(handle, &modelPtr->g2scale[1])) {
			if (PC_Float_Parse(handle, &modelPtr->g2scale[2])) {
				return qtrue;
			}
		}
	}
	return qfalse;
}

// model_g2skin <string>

qboolean ItemParse_model_g2skin( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	pc_token_t token;

	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}

	if (!token.string[0])
	{ //it was parsed correctly so still return true.
		return qtrue;
	}

	modelPtr->g2skin = trap->R_RegisterSkin(token.string);

	return qtrue;
}

// model_g2anim <number>
qboolean ItemParse_model_g2anim( itemDef_t *item, int handle ) {
	modelDef_t *modelPtr;
	pc_token_t token;
	int i = 0;

	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}

	if ( !token.string[0])
	{ //it was parsed correctly so still return true.
		return qtrue;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(token.string, animTable[i].name))
		{ //found it
			modelPtr->g2anim = i;
			return qtrue;
		}
		i++;
	}

	Com_Printf("Could not find '%s' in the anim table\n", token.string);
	return qtrue;
}

// model_g2skin <string>
qboolean ItemParse_model_g2skin_go( itemDef_t *item, const char *skinName ) 
{

	modelDef_t *modelPtr;
	int defSkin;


	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!skinName || !skinName[0])
	{ //it was parsed correctly so still return true.
		modelPtr->g2skin = 0;
		trap->G2API_SetSkin(item->ghoul2, 0, 0, 0);

		return qtrue;
	}

	// set skin
	if ( item->ghoul2 )
	{
		defSkin = trap->R_RegisterSkin(skinName);
		trap->G2API_SetSkin(item->ghoul2, 0, defSkin, defSkin);
	}

	return qtrue;
}

// model_g2anim <number>
qboolean ItemParse_model_g2anim_go( itemDef_t *item, const char *animName ) 
{
	modelDef_t *modelPtr;
	int i = 0;

	Item_ValidateTypeData(item);
	modelPtr = item->typeData.model;

	if (!animName || !animName[0])
	{ //it was parsed correctly so still return true.
		return qtrue;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(animName, animTable[i].name))
		{ //found it
			modelPtr->g2anim = animTable[i].id;
			return qtrue;
		}
		i++;
	}

	Com_Printf("Could not find '%s' in the anim table\n", animName);
	return qtrue;
}

// Get the cvar, get the values and stuff them in the rect structure.
qboolean ItemParse_rectcvar( itemDef_t *item, int handle ) 
{
	char	cvarBuf[1024];
	const char	*holdVal;
	char	*holdBuf;

	// get Cvar name
	pc_token_t token;
	if (!trap->PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	// get cvar data
	DC->getCVarString(token.string, cvarBuf, sizeof(cvarBuf));
	
	holdBuf = cvarBuf;
	if (String_Parse(&holdBuf,&holdVal))
	{
		item->window.rectClient.x = atof(holdVal);
		if (String_Parse(&holdBuf,&holdVal))
		{
			item->window.rectClient.y = atof(holdVal);
			if (String_Parse(&holdBuf,&holdVal))
			{
				item->window.rectClient.w = atof(holdVal);
				if (String_Parse(&holdBuf,&holdVal))
				{
					item->window.rectClient.h = atof(holdVal);
					return qtrue;
				}
			}
		}
	}

	// There may be no cvar built for this, and that's okay. . . I guess.
	return qtrue;
}

// rect <rectangle>
qboolean ItemParse_rect( itemDef_t *item, int handle ) {
	if (!PC_Rect_Parse(handle, &item->window.rectClient)) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_flag
	style <integer>
===============
*/
qboolean ItemParse_flag( itemDef_t *item, int handle) 
{
	int		i;
	pc_token_t token;

	if (!trap->PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	i=0;
	while (styles[i])
	{
		if (Q_stricmp(token.string,itemFlags[i].string)==0)
		{
			item->window.flags |= itemFlags[i].value;
			break;
		}
		i++;
	}

	if (itemFlags[i].string == NULL)
	{
		Com_Printf( S_COLOR_YELLOW "Unknown item style value '%s'\n", token.string );
	}

	return qtrue;
}

/*
===============
ItemParse_style 
	style <integer>
===============
*/
qboolean ItemParse_style( itemDef_t *item, int handle) 
{
	if (!PC_Int_Parse(handle, &item->window.style))
	{
		Com_Printf(S_COLOR_YELLOW "Unknown item style value\n");
		return qfalse;
	}

	return qtrue;
}


// decoration
qboolean ItemParse_decoration( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_DECORATION;
	return qtrue;
}

// JKG, no sound on focus

qboolean ItemParse_nofocussound( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_NOFOCUSSOUND;
	return qtrue;
}

qboolean ItemParse_multifocus( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_MULTIFOCUS;
	return qtrue;
}

qboolean ItemParse_selectionzone( itemDef_t *item, int handle ) {

	if (!PC_Rect_Parse(handle, &item->window.selectionZone)) {
		return qfalse;
	}
	item->window.flags |= WINDOW_FORCESELECTIONZONE;
	return qtrue;
}


// JKGalaxies, password
qboolean ItemParse_password( itemDef_t *item, int handle ) {
	item->flags |= ITF_TEXTFIELD_PASSWORD;
	return qtrue;
}

// JKGalaxies, allow negative numbers (ITEM_TYPE_NUMERICFIELD only)
qboolean ItemParse_allowNegative( itemDef_t *item, int handle ) {
	item->flags |= ITF_TEXTFIELD_ALLOWNEGATIVE;
	return qtrue;
}

// JKGalaxies, allow periods for decimal spaces (ITEM_TYPE_NUMERICFIELD only)
qboolean ItemParse_allowPeriod( itemDef_t *item, int handle ) {
	item->flags |= ITF_TEXTFIELD_ALLOWPERIOD;
	return qtrue;
}

// Dynamic text
qboolean ItemParse_dynamictext( itemDef_t *item, int handle ) {
	if (item->type == ITEM_TYPE_TEXT && !item->typeData.text) {
		item->typeData.text = (textDef_t *)UI_Alloc(sizeof(textDef_t));
		memset(item->typeData.text, 0, sizeof(textDef_t));
	}
	return qtrue;
}


// notselectable
qboolean ItemParse_notselectable( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) {
		listPtr->notselectable = qtrue;
	}
	return qtrue;
}

/*
===============
ItemParse_scrollhidden
	scrollhidden
===============
*/
qboolean ItemParse_scrollhidden( itemDef_t *item , int handle) 
{
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;

	if (item->type == ITEM_TYPE_LISTBOX && listPtr) 
	{
		listPtr->scrollhidden = qtrue;
	}
	return qtrue;
}





// manually wrapped
qboolean ItemParse_wrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_WRAPPED;
	return qtrue;
}

// auto wrapped
qboolean ItemParse_autowrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_AUTOWRAPPED;
	return qtrue;
}


// horizontalscroll
qboolean ItemParse_horizontalscroll( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_HORIZONTAL;
	return qtrue;
}

/*
===============
ItemParse_type 
	type <integer>
===============
*/
qboolean ItemParse_type( itemDef_t *item, int handle  ) 
{
//	int		i,holdInt;

	if (!PC_Int_Parse(handle, &item->type)) 
	{
		return qfalse;
	}
	Item_ValidateTypeData(item);
	return qtrue;
}

// elementwidth, used for listbox image elements
// uses textalignx for storage
qboolean ItemParse_elementwidth( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;
	if (!PC_Screenspace_Parse(handle, &listPtr->elementWidth, qtrue)) {
		return qfalse;
	}
	return qtrue;
}

// elementheight, used for listbox image elements
// uses textaligny for storage
qboolean ItemParse_elementheight( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;
	if (!PC_Screenspace_Parse(handle, &listPtr->elementHeight, qfalse)) {
		return qfalse;
	}
	return qtrue;
}

// feeder <float>
qboolean ItemParse_feeder( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

// JKG Add: element spacing
qboolean ItemParse_elementSpacingW( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;
	if (!PC_Screenspace_Parse(handle, &listPtr->elementSpacingW, qtrue)) {
		return qfalse;
	}
	return qtrue;
}
qboolean ItemParse_elementSpacingH( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = item->typeData.listbox;
	if (!PC_Screenspace_Parse(handle, &listPtr->elementSpacingH, qfalse)) {
		return qfalse;
	}
	return qtrue;
}

// elementtype, used to specify what type of elements a listbox contains
// uses textstyle for storage
qboolean ItemParse_elementtype( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData.listbox)
		return qfalse;
	listPtr = item->typeData.listbox;
	if (!PC_Int_Parse(handle, &listPtr->elementStyle)) {
		return qfalse;
	}
	return qtrue;
}

// columns sets a number of columns and an x pos and width per.. 
qboolean ItemParse_columns( itemDef_t *item, int handle ) {
	int num, i;
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData.listbox)
		return qfalse;
	listPtr = item->typeData.listbox;
	if (PC_Int_Parse(handle, &num)) {
		if (num > MAX_LB_COLUMNS) {
			num = MAX_LB_COLUMNS;
		}
		listPtr->numColumns = num;
		for (i = 0; i < num; i++) {
			int pos, width, maxChars;

			if (PC_Int_Parse(handle, &pos) && PC_Int_Parse(handle, &width) && PC_Int_Parse(handle, &maxChars)) {
				listPtr->columnInfo[i].pos = pos;
				listPtr->columnInfo[i].width = width;
				listPtr->columnInfo[i].maxChars = maxChars;
			} else {
				return qfalse;
			}
		}
	} else {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_border( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_bordersize( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_visible( itemDef_t *item, int handle ) {
	int i;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		item->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean ItemParse_ownerdraw( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.ownerDraw)) {
		return qfalse;
	}
	item->type = ITEM_TYPE_OWNERDRAW;
	return qtrue;
}

// JKG
qboolean ItemParse_ownerdrawID( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.ownerDrawID)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_align( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->alignment)) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_isCharacter 

Sets item flag showing this is a character
===============
*/
qboolean ItemParse_isCharacter( itemDef_t *item, int handle  )
{
	int flag;

	if (PC_Int_Parse(handle, &flag)) 
	{
		if ( flag )
		{
			item->flags |= ITF_ISCHARACTER;
		}
		else
		{
			item->flags &= ~ITF_ISCHARACTER;
		}
		return qtrue;
	}
	return qfalse;
}


/*
===============
ItemParse_textalign 
===============
*/
qboolean ItemParse_textalign( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->textalignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown text alignment value\n");
	
		return qfalse;
	}

	return qtrue;
}

qboolean ItemParse_textalign2( itemDef_t *item, int handle )
{
	if (!PC_Int_Parse(handle, &item->textalignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown text alignment value");
	
		return qfalse;
	}

	return qtrue;
}

qboolean ItemParse_textalignx( itemDef_t *item, int handle ) {
	if (!PC_Screenspace_Parse(handle, &item->textalignx, qtrue)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textaligny( itemDef_t *item, int handle ) {
	if (!PC_Screenspace_Parse(handle, &item->textaligny, qfalse)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textscale( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textscale)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textstyle( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->textStyle)) {
		return qfalse;
	}
	return qtrue;
}

// Jedi Knight Galaxies add: Separate fields for text2 !
qboolean ItemParse_textscale2( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textscale2)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textstyle2( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->textStyle2)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_font2( itemDef_t *item, int handle ) {
	Item_ValidateTypeData(item);
	if (!PC_Int_Parse(handle, &item->iMenuFont2)) 
	{
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_text2align( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->text2alignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown text alignment value");
	
		return qfalse;
	}

	return qtrue;
}

/*
===============
ItemParse_invertyesno
===============
*/
//JLFYESNO MPMOVED
qboolean ItemParse_invertyesno( itemDef_t *item, int handle) 
{
	if (!PC_Int_Parse(handle, &item->invertYesNo)) 
	{
		return qfalse;
	}
	return qtrue;
}

/*
===============
ItemParse_xoffset (used for yes/no and multi)
===============
*/
qboolean ItemParse_xoffset( itemDef_t *item, int handle) 
{
	if (PC_Int_Parse(handle, &item->xoffset)) 
	{
		return qfalse;
	}
	return qtrue;
}


qboolean ItemParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.backColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}

		if (f < 0)
		{	//special case for player color
			item->window.flags |= WINDOW_PLAYERCOLOR;
			return qtrue;
		}

		item->window.foreColor[i]  = f;
		item->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean ItemParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_outlinecolor( itemDef_t *item, int handle ) {
	if (!PC_Color_Parse(handle, &item->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_background( itemDef_t *item, int handle ) {
	pc_token_t token;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	item->window.background = DC->registerShaderNoMip(token.string);
	return qtrue;
}

qboolean ItemParse_foreground( itemDef_t *item, int handle )
{
	pc_token_t token;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	item->window.foreground = DC->registerShaderNoMip(token.string);
	return qtrue;
}

qboolean ItemParse_cinematic( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_doubleClick( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData.listbox) {
		return qfalse;
	}

	listPtr = item->typeData.listbox;

	if (!PC_Script_Parse(handle, &listPtr->doubleClick)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_onFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->onFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_onReturn( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->onReturn)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_onEscape( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->onEscape)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_leaveFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->leaveFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnter( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnter)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExit( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExit)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnterText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnterText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExitText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExitText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_action( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->action)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_tooltip( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->tooltip)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_special( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvarTest( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->cvarTest)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvar( itemDef_t *item, int handle ) 
{
	Item_ValidateTypeData(item);
	if (!PC_String_Parse(handle, &item->cvar)) 
	{
		return qfalse;
	}

	if ( item->typeData.edit )
	{
		editFieldDef_t *editPtr;

		switch ( item->type )
		{
			case ITEM_TYPE_EDITFIELD:
			case ITEM_TYPE_NUMERICFIELD:
			case ITEM_TYPE_YESNO:
			case ITEM_TYPE_BIND:
			case ITEM_TYPE_SLIDER:
			case ITEM_TYPE_TEXT:
			case ITEM_TYPE_BIND_FIXED: // Jedi Knight Galaxies
				editPtr = item->typeData.edit;
				editPtr->minVal = -1;
				editPtr->maxVal = -1;
				editPtr->defVal = -1;
				break;
		}
	}
	return qtrue;
}

qboolean ItemParse_font( itemDef_t *item, int handle ) 
{
	Item_ValidateTypeData(item);
	if (!PC_Int_Parse(handle, &item->iMenuFont)) 
	{
		return qfalse;
	}
	return qtrue;
}


qboolean ItemParse_maxChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData.edit)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = item->typeData.edit;
	editPtr->maxChars = maxChars;
	return qtrue;
}

// JKG - Deprecated
qboolean ItemParse_maxPaintChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData.edit)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = item->typeData.edit;
	editPtr->maxPaintChars = maxChars;
	return qtrue;
}

qboolean ItemParse_maxLineChars( itemDef_t *item, int handle ) 
{
	textScrollDef_t *scrollPtr;
	int				maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData.textscroll)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) 
	{
		return qfalse;
	}

	scrollPtr = item->typeData.textscroll;
	scrollPtr->maxLineChars = maxChars;

	return qtrue;
}

qboolean ItemParse_lineHeight( itemDef_t *item, int handle ) 
{
	textScrollDef_t *scrollPtr;
	int				height;

	Item_ValidateTypeData(item);
	if (!item->typeData.textscroll)
		return qfalse;

	if (!PC_Int_Parse(handle, &height)) 
	{
		return qfalse;
	}

	scrollPtr = item->typeData.textscroll;
	scrollPtr->lineHeight = height;

	return qtrue;
}

qboolean ItemParse_cvarFloat( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData.edit)
		return qfalse;
	editPtr = item->typeData.edit;
	if (PC_String_Parse(handle, &item->cvar) &&
		PC_Float_Parse(handle, &editPtr->defVal) &&
		PC_Float_Parse(handle, &editPtr->minVal) &&
		PC_Float_Parse(handle, &editPtr->maxVal)) {
		return qtrue;
	}
	return qfalse;
}

#ifdef IN_UI
char currLanguage[32][128];
static const char languageString[32] = "@MENUS_MYLANGUAGE";
#endif

qboolean ItemParse_cvarStrList( itemDef_t *item, int handle ) {
	pc_token_t token;
	multiDef_t *multiPtr;
	int pass;
	
	Item_ValidateTypeData(item);
	if (!item->typeData.multi)
		return qfalse;
	multiPtr = item->typeData.multi;
	multiPtr->count = 0;
	multiPtr->strDef = qtrue;

	if (!trap->PC_ReadToken(handle, &token))
	{
		return qfalse;
	}

	if (!Q_stricmp(token.string,"feeder") && item->special == FEEDER_PLAYER_SPECIES) 
	{
#ifndef _CGAME
		for (; multiPtr->count < uiInfo.playerSpeciesCount; multiPtr->count++)
		{
			multiPtr->cvarList[multiPtr->count] = String_Alloc(Q_strupr(va("@MENUS_%s",uiInfo.playerSpecies[multiPtr->count].Name )));	//look up translation
			multiPtr->cvarStr[multiPtr->count] = uiInfo.playerSpecies[multiPtr->count].Name;	//value
		}
#endif
		return qtrue;
	}
	// languages
	if (!Q_stricmp(token.string,"feeder") && item->special == FEEDER_LANGUAGES) 
	{
#ifndef _CGAME
		for (; multiPtr->count < uiInfo.languageCount; multiPtr->count++)
		{
			// The displayed text
			trap->SE_GetLanguageName( (const int) multiPtr->count,(char *) currLanguage[multiPtr->count]  );	// eg "English"
			multiPtr->cvarList[multiPtr->count] = languageString;
			// The cvar value that goes into se_language
			trap->SE_GetLanguageName( (const int) multiPtr->count,(char *) currLanguage[multiPtr->count] );
			multiPtr->cvarStr[multiPtr->count] = currLanguage[multiPtr->count];
		}
#endif
		return qtrue;
	}

	if (*token.string != '{') {
		return qfalse;
	}

	pass = 0;
	while ( 1 ) {
		const char* psString;

//		if (!trap->PC_ReadToken(handle, &token)) {
//			PC_SourceError(handle, "end of file inside menu item\n");
//			return qfalse;
//		}		   

		if (!PC_String_Parse(handle, &psString)) 
		{
			PC_SourceError(handle, "end of file inside menu item");
			return qfalse;
		}

		//a normal StringAlloc ptr
		if (psString)	
		{
			if (*psString == '}') {
				return qtrue;
			}

			if (*psString == ',' || *psString == ';') {
				continue;
			}
		}

		if (pass == 0) {
			multiPtr->cvarList[multiPtr->count] = psString;
			pass = 1;
		} else {
			multiPtr->cvarStr[multiPtr->count] = psString;
			pass = 0;
			multiPtr->count++;
			if (multiPtr->count >= MAX_MULTI_CVARS) {
				return qfalse;
			}
		}

	}
	return qfalse; 	// bk001205 - LCC missing return value
}

qboolean ItemParse_cvarFloatList( itemDef_t *item, int handle ) 
{
	pc_token_t token;
	multiDef_t *multiPtr;
	
	Item_ValidateTypeData(item);
	if (!item->typeData.multi)
	{
		return qfalse;
	}
	
	multiPtr = item->typeData.multi;
	multiPtr->count = 0;
	multiPtr->strDef = qfalse;

	if (!trap->PC_ReadToken(handle, &token))
	{
		return qfalse;
	}
	
	if (*token.string != '{') 
	{
		return qfalse;
	}

	while ( 1 ) 
	{
		const char* string;

		if ( !PC_String_Parse ( handle, &string ) )
		{
			PC_SourceError(handle, "end of file inside menu item");
			return qfalse;
		}
			
		//a normal StringAlloc ptr
		if (string)	
		{
			if (*string == '}') 
			{
				return qtrue;
			}

			if (*string == ',' || *string == ';') 
			{
				continue;
			}
		}

		multiPtr->cvarList[multiPtr->count] = string;
		if (!PC_Float_Parse(handle, &multiPtr->cvarValue[multiPtr->count])) 
		{
			return qfalse;
		}

		multiPtr->count++;
		if (multiPtr->count >= MAX_MULTI_CVARS) 
		{
			return qfalse;
		}

	}
	return qfalse; 	// bk001205 - LCC missing return value
}



qboolean ItemParse_addColorRange( itemDef_t *item, int handle ) {
	colorRangeDef_t color;

	if (PC_Float_Parse(handle, &color.low) &&
		PC_Float_Parse(handle, &color.high) &&
		PC_Color_Parse(handle, &color.color) ) {
		if (item->numColors < MAX_COLOR_RANGES) {
			memcpy(&item->colorRanges[item->numColors], &color, sizeof(color));
			item->numColors++;
		}
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_ownerdrawFlag( itemDef_t *item, int handle ) {
	int i;
	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	item->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean ItemParse_enableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_ENABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_disableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_DISABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_showCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_SHOW;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_hideCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_HIDE;
		return qtrue;
	}
	return qfalse;
}

/*
===============
ItemParse_align 
===============
*/
qboolean ItemParse_Appearance_slot( itemDef_t *item, int handle ) 
{
	if (!PC_Int_Parse(handle, &item->appearanceSlot))
	{
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_isSaber( itemDef_t *item, int handle  )
{
#ifndef _CGAME

	int	i;

	if (PC_Int_Parse(handle, &i)) 
	{
		if ( i )
		{
			item->flags |= ITF_ISSABER;
			UI_CacheSaberGlowGraphics();
			if ( !ui_saber_parms_parsed )
			{
				UI_SaberLoadParms();
			}
		}
		else
		{
			item->flags &= ~ITF_ISSABER;
		}
	
		return qtrue;
	}
#endif
	return qfalse;
}

//extern void UI_SaberLoadParms( void );
//extern qboolean ui_saber_parms_parsed;
//extern void UI_CacheSaberGlowGraphics( void );

qboolean ItemParse_isSaber2( itemDef_t *item, int handle  )
{
#ifndef _CGAME
	int	i;
	if (PC_Int_Parse(handle, &i)) 
	{
		if ( i )
		{
			item->flags |= ITF_ISSABER2;
			UI_CacheSaberGlowGraphics();
			if ( !ui_saber_parms_parsed )
			{
				UI_SaberLoadParms();
			}
		}
		else
		{
			item->flags &= ~ITF_ISSABER2;
		}

		return qtrue;
	}
#endif
	
	return qfalse;
}

bool ItemParse_class( itemDef_t *item, int handle )
{
#ifndef _CGAME
	const char *classname;
	if( PC_String_Parse(handle, &classname) )
	{
		JKG_UI_SetClass( classname, item );
		return true;
	}
#endif
	return false;
}

bool ItemParse_useNewSlider( itemDef_t *item, int handle )
{
	item->window.useNewSlider = true;
	return true;
}

bool ItemParse_noSliderButtons( itemDef_t *item, int handle )
{
	item->window.noSliderButtons = true;
	return true;
}

bool ItemParse_sliderButtonUp( itemDef_t *item, int handle )
{
	const char *shaderName;
	if(!PC_String_Parse(handle, &shaderName))
		return false;
	item->window.sliderButtonUpShader = DC->registerShaderNoMip(shaderName);
	return false;
}

bool ItemParse_sliderButtonDown( itemDef_t *item, int handle )
{
	const char *shaderName;
	if(!PC_String_Parse(handle, &shaderName))
		return false;
	item->window.sliderButtonDownShader = DC->registerShaderNoMip(shaderName);
	return true;
}

bool ItemParse_sliderThumbW( itemDef_t *item, int handle )
{
	if( PC_Float_Parse(handle, &item->window.sliderThumbWidth) )
		return true;
	return true;
}

bool ItemParse_sliderThumbH( itemDef_t *item, int handle )
{
	if( PC_Float_Parse(handle, &item->window.sliderThumbHeight) )
		return true;
	return false;
}

bool ItemParse_sliderButtonW( itemDef_t *item, int handle )
{
	if( PC_Float_Parse(handle, &item->window.sliderButtonWidth) )
		return true;
	return false;
}

bool ItemParse_sliderButtonH( itemDef_t *item, int handle )
{
	if( PC_Float_Parse(handle, &item->window.sliderButtonHeight) )
		return true;
	return false;
}

bool ItemParse_sliderBackground( itemDef_t *item, int handle )
{
	const char *shaderName;
	if(!PC_String_Parse(handle, &shaderName))
	{
		return false;
	}
	item->window.sliderBackground = DC->registerShaderNoMip(shaderName);
	return true;
}

bool ItemParse_floatingSlider( itemDef_t *item, int handle )
{
	item->window.floatingSlider = true;
	return true;
}

bool ItemParse_SliderBackgroundW( itemDef_t *item, int handle )
{
	if(!PC_Float_Parse(handle, &item->window.sliderBackgroundWidth))
		return false;
	return true;
}

bool ItemParse_SliderBackgroundH( itemDef_t *item, int handle )
{
	if(!PC_Float_Parse(handle, &item->window.sliderBackgroundHeight))
		return false;
	return true;
}

bool ItemParse_sliderTop( itemDef_t *item, int handle )
{
	if(!PC_Float_Parse(handle, &item->window.sliderTopY))
		return false;
	return true;
}

bool ItemParse_sliderBottom( itemDef_t *item, int handle )
{
	if(!PC_Float_Parse(handle, &item->window.sliderBottomY))
		return false;
	return true;
}

keywordHash_t itemParseKeywords[] = {
	{"action",			ItemParse_action,			NULL	},
	{"addColorRange",	ItemParse_addColorRange,	NULL	},
	{"align",			ItemParse_align,			NULL	},
	{"autowrapped",		ItemParse_autowrapped,		NULL	},
	{"appearance_slot",	ItemParse_Appearance_slot,	NULL	},
	{"asset_model",		ItemParse_asset_model,		NULL	},
	{"asset_shader",	ItemParse_asset_shader,		NULL	},
	{"backcolor",		ItemParse_backcolor,		NULL	},
	{"background",		ItemParse_background,		NULL	},
	{"border",			ItemParse_border,			NULL	},
	{"bordercolor",		ItemParse_bordercolor,		NULL	},
	{"bordersize",		ItemParse_bordersize,		NULL	},
	{"cinematic",		ItemParse_cinematic,		NULL	},
	{"columns",			ItemParse_columns,			NULL	},
	{"cvar",			ItemParse_cvar,				NULL	},
	{"cvarFloat",		ItemParse_cvarFloat,		NULL	},
	{"cvarFloatList",	ItemParse_cvarFloatList,	NULL	},
	{"cvarStrList",		ItemParse_cvarStrList,		NULL	},
	{"cvarTest",		ItemParse_cvarTest,			NULL	},
	{"desctext",		ItemParse_descText,			NULL	},
	{"decoration",		ItemParse_decoration,		NULL	},

	// JKGalaxies
	{"nofocussound",	ItemParse_nofocussound,		NULL	},
	{"password",		ItemParse_password,			NULL	},
	{"dynamictext",		ItemParse_dynamictext,		NULL	},
	{"allownegative",	ItemParse_allowNegative,	NULL	},
	{"allowperiod",		ItemParse_allowPeriod,		NULL	},
	{"multifocus",		ItemParse_multifocus,		NULL	},
	{"selectionZone",	ItemParse_selectionzone,	NULL	},
	{"foreground",		ItemParse_foreground,		NULL	},
	//new events
	{"onReturn",		ItemParse_onReturn,			NULL	},
	{"onEscape",		ItemParse_onEscape,			NULL	},
	{"font2",			ItemParse_font2,			NULL	},
	{"textscale2",		ItemParse_textscale2,		NULL	},
	{"textstyle2",		ItemParse_textstyle2,		NULL	},
	{"text2align",	ItemParse_text2align,		NULL	},
	{"ownerdrawID",		ItemParse_ownerdrawID,		NULL	},
	{"ownerdrawFlag",	ItemParse_ownerdrawFlag,	NULL	},
	// Image scroll specific
	{"elementSpacingW",	ItemParse_elementSpacingW,	NULL	},
	{"elementSpacingH",	ItemParse_elementSpacingH,	NULL	},
	// style sheets
	{"class",			ItemParse_class,			NULL	},

	// new slider crap
	{"useNewSlider",	ItemParse_useNewSlider,		NULL	},
	{"noSliderButtons",	ItemParse_noSliderButtons,	NULL	},
	{"sliderButtonUp",	ItemParse_sliderButtonUp,	NULL	},
	{"sliderButtonDown", ItemParse_sliderButtonDown, NULL	},
	{"sliderThumbW",	ItemParse_sliderThumbW,		NULL	},
	{"sliderThumbH",	ItemParse_sliderThumbH,		NULL	},
	{"sliderButtonW",	ItemParse_sliderButtonW,	NULL	},
	{"sliderButtonH",	ItemParse_sliderButtonH,	NULL	},
	{"sliderBackground", ItemParse_sliderBackground, NULL	},

	{"floatingSlider",	ItemParse_floatingSlider,	NULL	},
	{"sliderBackgroundW", ItemParse_SliderBackgroundW, NULL	},
	{"sliderBackgroundH", ItemParse_SliderBackgroundH, NULL },
	{"sliderTop",		ItemParse_sliderTop,		NULL	},
	{"sliderBottom",	ItemParse_sliderBottom,		NULL	},
	// END JEDI KNIGHT GALAXIES

	{"disableCvar",		ItemParse_disableCvar,		NULL	},
	{"doubleclick",		ItemParse_doubleClick,		NULL	},
	{"elementheight",	ItemParse_elementheight,	NULL	},
	{"elementtype",		ItemParse_elementtype,		NULL	},
	{"elementwidth",	ItemParse_elementwidth,		NULL	},
	{"enableCvar",		ItemParse_enableCvar,		NULL	},
	{"feeder",			ItemParse_feeder,			NULL	},
	{"flag",			ItemParse_flag,				NULL	},
	{"focusSound",		ItemParse_focusSound,		NULL	},
	{"font",			ItemParse_font,				NULL	},
	{"forecolor",		ItemParse_forecolor,		NULL	},
	{"group",			ItemParse_group,			NULL	},
	{"hideCvar",		ItemParse_hideCvar,			NULL	},
	{"horizontalscroll", ItemParse_horizontalscroll, NULL	},
	{"isCharacter",		ItemParse_isCharacter,		NULL	},
	{"isSaber",			ItemParse_isSaber,			NULL	},
	{"isSaber2",		ItemParse_isSaber2,			NULL	},
	{"leaveFocus",		ItemParse_leaveFocus,		NULL	},
	{"maxChars",		ItemParse_maxChars,			NULL	},
	{"maxPaintChars",	ItemParse_maxPaintChars,	NULL	},
	{"model_angle",		ItemParse_model_angle,		NULL	},
	{"model_fovx",		ItemParse_model_fovx,		NULL	},
	{"model_fovy",		ItemParse_model_fovy,		NULL	},
	{"model_origin",	ItemParse_model_origin,		NULL	},
	{"model_rotation",	ItemParse_model_rotation,	NULL	},
	//rww - g2 begin
	{"model_g2mins",	ItemParse_model_g2mins,		NULL	},
	{"model_g2maxs",	ItemParse_model_g2maxs,		NULL	},
	{"model_g2scale",	ItemParse_model_g2scale,	NULL	},
	{"model_g2skin",	ItemParse_model_g2skin,		NULL	},
	{"model_g2anim",	ItemParse_model_g2anim,		NULL	},
	//rww - g2 end
	{"mouseEnter",		ItemParse_mouseEnter,		NULL	},
	{"mouseEnterText",	ItemParse_mouseEnterText,	NULL	},
	{"mouseExit",		ItemParse_mouseExit,		NULL	},
	{"mouseExitText",	ItemParse_mouseExitText,	NULL	},
	{"name",			ItemParse_name,				NULL	},
	{"notselectable",	ItemParse_notselectable,	NULL	},
	{"onFocus",			ItemParse_onFocus,			NULL	},
	{"outlinecolor",	ItemParse_outlinecolor,		NULL	},
	{"ownerdraw",		ItemParse_ownerdraw,		NULL	},
	{"rect",			ItemParse_rect,				NULL	},
	{"rectcvar",		ItemParse_rectcvar,			NULL	},
	{"showCvar",		ItemParse_showCvar,			NULL	},
	{"special",			ItemParse_special,			NULL	},
	{"style",			ItemParse_style,			NULL	},
	{"text",			ItemParse_text,				NULL	},
	{"textalign",		ItemParse_textalign,		NULL	},
	{"textalignx",		ItemParse_textalignx,		NULL	},
	{"textaligny",		ItemParse_textaligny,		NULL	},
	{"textscale",		ItemParse_textscale,		NULL	},
	{"textstyle",		ItemParse_textstyle,		NULL	},
	{"text2",			ItemParse_text2,			NULL	},
	{"text2alignx",		ItemParse_text2alignx,		NULL	},
	{"text2aligny",		ItemParse_text2aligny,		NULL	},
	{"tooltip",			ItemParse_tooltip,			NULL	},
	{"type",			ItemParse_type,				NULL	},
	{"visible",			ItemParse_visible,			NULL	},
	{"wrapped",			ItemParse_wrapped,			NULL	},

	// Text scroll specific
	{"maxLineChars",	ItemParse_maxLineChars,		NULL	},
	{"lineHeight",		ItemParse_lineHeight,		NULL	},
	{"invertyesno",		ItemParse_invertyesno,		NULL	},
	//JLF MPMOVED
	{"scrollhidden",	ItemParse_scrollhidden,		NULL	},
	{"xoffset		",	ItemParse_xoffset,			NULL	},
	//JLF end
	{0,					0,							0		}
};

keywordHash_t *itemParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Item_SetupKeywordHash
===============
*/
void Item_SetupKeywordHash(void) {
	int i;

	memset(itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));
	for (i = 0; itemParseKeywords[i].keyword; i++) {
		KeywordHash_Add(itemParseKeywordHash, &itemParseKeywords[i]);
	}
}

/*
===============
Item_Parse
===============
*/
qboolean Item_Parse(int handle, itemDef_t *item) {
	pc_token_t token;
	keywordHash_t *key;


	if (!trap->PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
	while ( 1 ) {
		if (!trap->PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = KeywordHash_Find(itemParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown menu item keyword %s", token.string);
			continue;
		}
		if ( !key->func(item, handle) ) {
			PC_SourceError(handle, "couldn't parse menu item keyword %s", token.string);
			return qfalse;
		}
	}
	return qfalse; 	// bk001205 - LCC missing return value
}

static void Item_TextScroll_BuildLines ( itemDef_t* item )
{
	char text[2048];

#if 1

	// new asian-aware line breaker...  (pasted from elsewhere late @ night, hence aliasing-vars ;-)
	//
	textScrollDef_t* scrollPtr = item->typeData.textscroll;
	const char *psText = item->text;	// for copy/paste ease
	int iBoxWidth = item->window.rect.w - SCROLLBAR_SIZE - 10;

	// this could probably be simplified now, but it was converted from something else I didn't originally write, 
	//	and it works anyway so wtf...
	//
	const char *psCurrentTextReadPos;
	const char *psReadPosAtLineStart;	
	const char *psBestLineBreakSrcPos;
	const char *psLastGood_s;	// needed if we get a full screen of chars with no punctuation or space (see usage notes)
	qboolean bIsTrailingPunctuation;
	unsigned int uiLetter;

	if (*psText == '@')	// string reference
	{
		trap->SE_GetStringTextString( &psText[1], text, sizeof(text));
		psText = text;
	}

	psCurrentTextReadPos = psText;
	psReadPosAtLineStart = psCurrentTextReadPos;
	psBestLineBreakSrcPos = psCurrentTextReadPos;

	scrollPtr->iLineCount = 0;
	memset((char*)scrollPtr->pLines,0,sizeof(scrollPtr->pLines));

	while (*psCurrentTextReadPos && (scrollPtr->iLineCount < MAX_TEXTSCROLL_LINES) )
	{
		char sLineForDisplay[2048];	// ott

		// construct a line...
		//
		psCurrentTextReadPos = psReadPosAtLineStart;
		sLineForDisplay[0] = '\0';
		while ( *psCurrentTextReadPos )
		{
			int iAdvanceCount;
			psLastGood_s = psCurrentTextReadPos;

			// read letter...
			//
			uiLetter = trap->R_AnyLanguage_ReadCharFromString(psCurrentTextReadPos, &iAdvanceCount, &bIsTrailingPunctuation);
			psCurrentTextReadPos += iAdvanceCount;

			// concat onto string so far...
			//
			if (uiLetter == 32 && sLineForDisplay[0] == '\0')
			{
				psReadPosAtLineStart++;
				continue;	// unless it's a space at the start of a line, in which case ignore it.
			}

			if (uiLetter > 255)
			{
				Q_strcat(sLineForDisplay, sizeof(sLineForDisplay),va("%c%c",uiLetter >> 8, uiLetter & 0xFF));
			}
			else
			{
				Q_strcat(sLineForDisplay, sizeof(sLineForDisplay),va("%c",uiLetter & 0xFF));
			}

			if (uiLetter == '\n')
			{
				// explicit new line...
				//
				sLineForDisplay[ strlen(sLineForDisplay)-1 ] = '\0';	// kill the CR
				psReadPosAtLineStart = psCurrentTextReadPos;
				psBestLineBreakSrcPos = psCurrentTextReadPos;

				// hack it to fit in with this code...
				//
				scrollPtr->pLines[ scrollPtr->iLineCount ] = String_Alloc ( sLineForDisplay );
				break;	// print this line
			}
			else if ( (DC->textWidth( sLineForDisplay, 1.0f, item->iMenuFont ) * item->textscale) >= iBoxWidth )
			{					
				// reached screen edge, so cap off string at bytepos after last good position...
				//
				if (uiLetter > 255 && bIsTrailingPunctuation && !trap->R_Language_UsesSpaces())
				{
					// Special case, don't consider line breaking if you're on an asian punctuation char of
					//	a language that doesn't use spaces...
					//
				}
				else
				{
					if (psBestLineBreakSrcPos == psReadPosAtLineStart)
					{
						//  aarrrggh!!!!!   we'll only get here is someone has fed in a (probably) garbage string,
						//		since it doesn't have a single space or punctuation mark right the way across one line
						//		of the screen.  So far, this has only happened in testing when I hardwired a taiwanese 
						//		string into this function while the game was running in english (which should NEVER happen 
						//		normally).  On the other hand I suppose it's entirely possible that some taiwanese string 
						//		might have no punctuation at all, so...
						//
						psBestLineBreakSrcPos = psLastGood_s;	// force a break after last good letter
					}

					sLineForDisplay[ psBestLineBreakSrcPos - psReadPosAtLineStart ] = '\0';
					psReadPosAtLineStart = psCurrentTextReadPos = psBestLineBreakSrcPos;

					// hack it to fit in with this code...
					//
					scrollPtr->pLines[ scrollPtr->iLineCount ] = String_Alloc( sLineForDisplay );
					break;	// print this line
				}
			}

			// record last-good linebreak pos...  (ie if we've just concat'd a punctuation point (western or asian) or space)
			//
			if (bIsTrailingPunctuation || uiLetter == ' ' || (uiLetter > 255 && !trap->R_Language_UsesSpaces()))
			{
				psBestLineBreakSrcPos = psCurrentTextReadPos;
			}
		}

		/// arrgghh, this is gettng horrible now...
		//
		if (scrollPtr->pLines[ scrollPtr->iLineCount ] == NULL && strlen(sLineForDisplay))
		{
			// then this is the last line and we've just run out of text, no CR, no overflow etc...
			//
			scrollPtr->pLines[ scrollPtr->iLineCount ] = String_Alloc( sLineForDisplay );
		}
			
		scrollPtr->iLineCount++;
	}
	
#else
	// old version...
	//
	int				 width;
	char*			 lineStart;
	char*			 lineEnd;
	float			 w;
	float			 cw;

	textScrollDef_t* scrollPtr = item->typeData.textscroll;

	scrollPtr->iLineCount = 0;
	width = scrollPtr->maxLineChars;
	lineStart = (char*)item->text;
	lineEnd   = lineStart;
	w		  = 0;

	// Keep going as long as there are more lines
	while ( scrollPtr->iLineCount < MAX_TEXTSCROLL_LINES )
	{
		// End of the road
		if ( *lineEnd == '\0')
		{
			if ( lineStart < lineEnd )
			{
				scrollPtr->pLines[ scrollPtr->iLineCount++ ] = lineStart;
			}

			break;
		}

		// Force a line end if its a '\n'
		else if ( *lineEnd == '\n' )
		{
			*lineEnd = '\0';
			scrollPtr->pLines[ scrollPtr->iLineCount++ ] = lineStart;
			lineStart = lineEnd + 1;
			lineEnd   = lineStart;
			w = 0;
			continue;
		}

		// Get the current character width 
		cw = DC->textWidth ( va("%c", *lineEnd), item->textscale, item->iMenuFont );

		// Past the end of the boundary?
		if ( w + cw > (item->window.rect.w - SCROLLBAR_SIZE - 10) )
		{
			// Past the end so backtrack to the word boundary
			while ( *lineEnd != ' ' && *lineEnd != '\t' && lineEnd > lineStart )
			{
				lineEnd--;
			}					

			*lineEnd = '\0';
			scrollPtr->pLines[ scrollPtr->iLineCount++ ] = lineStart;

			// Skip any whitespaces
			lineEnd++;
			while ( (*lineEnd == ' ' || *lineEnd == '\t') && *lineEnd )
			{
				lineEnd++;
			}

			lineStart = lineEnd;
			w = 0;
		}
		else
		{
			w += cw;
			lineEnd++;
		}
	}
#endif
}

// Item_InitControls
// init's special control types
void Item_InitControls(itemDef_t *item) 
{
	if (item == NULL) 
	{
		return;
	}

	switch ( item->type )
	{
		case ITEM_TYPE_LISTBOX:
		{
			listBoxDef_t *listPtr = item->typeData.listbox;
			item->cursorPos = 0;
			if (listPtr) 
			{
				listPtr->cursorPos = 0;
				listPtr->startPos = 0;
				listPtr->endPos = 0;
				listPtr->cursorPos = 0;
			}

			break;
		}
	}
}

/*
===============
Menu Keyword Parse functions
===============
*/

qboolean MenuParse_font( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_String_Parse(handle, &menu->font)) {
		return qfalse;
	}
	if (!DC->Assets.fontRegistered) {
		//DC->registerFont(menu->font, 48, &DC->Assets.textFont);
		DC->Assets.qhMediumFont = DC->RegisterFont(menu->font);
		DC->Assets.fontRegistered = qtrue;
	}
	return qtrue;
}

qboolean MenuParse_name( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_String_Parse(handle, &menu->window.name)) {
		return qfalse;
	}
	if (Q_stricmp(menu->window.name, "main") == 0) {
		// default main as having focus
		//menu->window.flags |= WINDOW_HASFOCUS;
	}
	return qtrue;
}

qboolean MenuParse_fullscreen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, (int*) &menu->fullScreen)) { // bk001206 - cast qboolean
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_rect( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Rect_Parse(handle, &menu->window.rect)) {
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_style
=================
*/
qboolean MenuParse_style( itemDef_t *item, int handle) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->window.style))
	{
		Com_Printf(S_COLOR_YELLOW "Unknown menu style value\n");
		return qfalse;
	}

	return qtrue;
}

qboolean MenuParse_visible( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		menu->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean MenuParse_handleEsc( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->handleEsc = 1;
	return qtrue;
}

qboolean MenuParse_onOpen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onOpen)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onClose( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onClose)) {
		return qfalse;
	}
	return qtrue;
}

//JLFACCEPT MPMOVED
/*
=================
MenuParse_onAccept
=================
*/
qboolean MenuParse_onAccept( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Script_Parse(handle, &menu->onAccept)) 
	{
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onESC( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onESC)) {
		return qfalse;
	}
	return qtrue;
}


qboolean MenuParse_border( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, &menu->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_borderSize( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Float_Parse(handle, &menu->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.backColor[i]  = f;
	}
	return qtrue;
}

/*
=================
MenuParse_descAlignment
=================
*/
qboolean MenuParse_descAlignment( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descAlignment)) 
	{
		Com_Printf(S_COLOR_YELLOW "Unknown desc alignment value\n");
		return qfalse;
	}

	return qtrue;
}

/*
=================
MenuParse_descX
=================
*/
qboolean MenuParse_descX( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descX))
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_descY
=================
*/
qboolean MenuParse_descY( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->descY))
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_descScale
=================
*/
qboolean MenuParse_descScale( itemDef_t *item, int handle) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->descScale)) 
	{
		return qfalse;
	}
	return qtrue;
}

/*
=================
MenuParse_descColor
=================
*/
qboolean MenuParse_descColor( itemDef_t *item, int handle) 
{
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) 
	{
		if (!PC_Float_Parse(handle, &f)) 
		{
			return qfalse;
		}
		menu->descColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		if (f < 0)
		{	//special case for player color
			menu->window.flags |= WINDOW_PLAYERCOLOR;
			return qtrue;
		}
		menu->window.foreColor[i]  = f;
		menu->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean MenuParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_focuscolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->focusColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_disablecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;
	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->disableColor[i]  = f;
	}
	return qtrue;
}


qboolean MenuParse_outlinecolor( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Color_Parse(handle, &menu->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_background( itemDef_t *item, int handle ) {
	pc_token_t token;
	menuDef_t *menu = (menuDef_t*)item;

	if (!trap->PC_ReadToken(handle, &token)) {
		return qfalse;
	}
	menu->window.background = DC->registerShaderNoMip(token.string);
	return qtrue;
}

qboolean MenuParse_cinematic( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_ownerdrawFlag( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	menu->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean MenuParse_ownerdraw( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->window.ownerDraw)) {
		return qfalse;
	}
	return qtrue;
}


// decoration
qboolean MenuParse_popup( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->window.flags |= WINDOW_POPUP;
	return qtrue;
}


qboolean MenuParse_outOfBounds( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	menu->window.flags |= WINDOW_OOB_CLICK;
	return qtrue;
}

qboolean MenuParse_soundLoop( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->soundName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeClamp( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeClamp)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeAmount( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeAmount)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_stylesheet( itemDef_t *item, int handle ) {
#ifndef _CGAME
	const char *text;
	if (!PC_String_Parse(handle, &text))
		return qfalse;
	JKG_UI_LoadStylesheet(text);
#endif
	return qtrue;
}

qboolean MenuParse_fadeCycle( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->fadeCycle)) {
		return qfalse;
	}
	return qtrue;
}


qboolean MenuParse_itemDef( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (menu->itemCount < MAX_MENUITEMS) {
		menu->items[menu->itemCount] = (itemDef_t *) UI_Alloc(sizeof(itemDef_t));
		Item_Init(menu->items[menu->itemCount]);
		if (!Item_Parse(handle, menu->items[menu->itemCount])) {
			return qfalse;
		}
		Item_InitControls(menu->items[menu->itemCount]);
		menu->items[menu->itemCount++]->parent = menu;
	}
	return qtrue;
}
/*
=================
MenuParse_focuscolor
=================
*/
qboolean MenuParse_appearanceIncrement( itemDef_t *item, int handle ) 
{
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->appearanceIncrement))
	{
		return qfalse;
	}
	return qtrue;
}

keywordHash_t menuParseKeywords[] = {
	{"appearanceIncrement",	MenuParse_appearanceIncrement,	NULL	},
	{"backcolor",			MenuParse_backcolor,	NULL	},
	{"background",			MenuParse_background,	NULL	},
	{"border",				MenuParse_border,		NULL	},
	{"bordercolor",			MenuParse_bordercolor,	NULL	},
	{"borderSize",			MenuParse_borderSize,	NULL	},
	{"cinematic",			MenuParse_cinematic,	NULL	},
	{"descAlignment",		MenuParse_descAlignment,NULL	},
	{"desccolor",			MenuParse_descColor,	NULL	},
	{"descX",				MenuParse_descX,		NULL	},
	{"descY",				MenuParse_descY,		NULL	},
	{"descScale",			MenuParse_descScale,	NULL	},
	{"disablecolor",		MenuParse_disablecolor, NULL	},
	{"fadeAmount",			MenuParse_fadeAmount,	NULL	},
	{"fadeClamp",			MenuParse_fadeClamp,	NULL	},
	{"fadeCycle",			MenuParse_fadeCycle,	NULL	},
	{"focuscolor",			MenuParse_focuscolor,	NULL	},
	{"font",				MenuParse_font,			NULL	},
	{"forecolor",			MenuParse_forecolor,	NULL	},
	{"fullscreen",			MenuParse_fullscreen,	NULL	},
	{"itemDef",				MenuParse_itemDef,		NULL	},
	{"name",				MenuParse_name,			NULL	},
	{"onClose",				MenuParse_onClose,		NULL	},
	//JLFACCEPT MPMOVED
	{"onAccept",			MenuParse_onAccept,		NULL    },
	// Jedi Knight Galaxies
	{"handleEsc",			MenuParse_handleEsc,	NULL	},
	{"stylesheet",			MenuParse_stylesheet,	NULL	},

	{"onESC",				MenuParse_onESC,		NULL	},
	{"outOfBoundsClick",	MenuParse_outOfBounds,	NULL	},
	{"onOpen",				MenuParse_onOpen,		NULL	},
	{"outlinecolor",		MenuParse_outlinecolor, NULL	},
	{"ownerdraw",			MenuParse_ownerdraw,	NULL	},
	{"ownerdrawFlag",		MenuParse_ownerdrawFlag,NULL	},
	{"popup",				MenuParse_popup,		NULL	},
	{"rect",				MenuParse_rect,			NULL	},
	{"soundLoop",			MenuParse_soundLoop,	NULL	},
	{"style",				MenuParse_style,		NULL	},
	{"visible",				MenuParse_visible,		NULL	},
	{0,						0,						0		}
};

keywordHash_t *menuParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Menu_SetupKeywordHash
===============
*/
void Menu_SetupKeywordHash(void) {
	int i;

	memset(menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));
	for (i = 0; menuParseKeywords[i].keyword; i++) {
		KeywordHash_Add(menuParseKeywordHash, &menuParseKeywords[i]);
	}
}

/*
===============
Menu_Parse
===============
*/
qboolean Menu_Parse(int handle, menuDef_t *menu) {
	pc_token_t token;
	keywordHash_t *key;

	if (!trap->PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!trap->PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = KeywordHash_Find(menuParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown menu keyword %s", token.string);
			continue;
		}
		if ( !key->func((itemDef_t*)menu, handle) ) {
			PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
			return qfalse;
		}
	}
	return qfalse; 	// bk001205 - LCC missing return value
}

/*
===============
Menu_New
===============
*/
void Menu_New(int handle) {
	menuDef_t *menu = &Menus[menuCount];

	if (menuCount < MAX_MENUS) {
		Menu_Init(menu);
		if (Menu_Parse(handle, menu)) {
			Menu_PostParse(menu);
			menuCount++;
		}
	}
}

int Menu_Count() {
	return menuCount;
}

void Menu_PaintAll() {
	int i;
	if (captureFunc) {
		captureFunc(captureData);
	}

	for (i = 0; i < Menu_Count(); i++) {
		Menu_Paint(&Menus[i], qfalse);
	}

	if (debugMode) {
		vec4_t v = {1, 1, 1, 1};
		menuDef_t *menu;
		itemDef_t *item;

		menu = Menu_GetFocused();
		item = Menu_GetFocusedItem(menu);
		DC->drawText(5, 5, .5, v, va("fps: %.0f", DC->FPS), 0, 0, 0, 0);
		DC->drawText(5, 15, .5, v, va("x: %d  y:%d", DC->cursorx,DC->cursory), 0, 0, 0, 0);
		if (menu) {
			DC->drawText(5, 25, .5, v, va("menu: %s (%s)", menu->window.name, menu->window.group), 0, 0, 0, 0);
			if (item) {
				DC->drawText(5, 35, .5, v, va("item: %s (%s)", item->window.name, item->window.group), 0, 0, 0, 0);
			}
		}
	}
}

void Menu_Reset() {
	menuCount = 0;
}

displayContextDef_t *Display_GetContext() {
	return DC;
}
 
void *Display_CaptureItem(int x, int y) {
	int i;

	for (i = 0; i < menuCount; i++) {
		// turn off focus each item
		// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;
		if( Menus[i].window.flags & WINDOW_FORCESELECTIONZONE )
		{
			if(Rect_ContainsPoint(&Menus[i].window.selectionZone, x, y) )
			{
				return &Menus[i];
			}
		}
		else if (Rect_ContainsPoint(&Menus[i].window.rect, x, y)) {
			return &Menus[i];
		}
	}
	return NULL;
}


// FIXME: 
qboolean Display_MouseMove(void *p, int x, int y) {
	int i;
	menuDef_t *menu = (menuDef_t *) p;

	if (menu == NULL) {
		menu = Menu_GetFocused();
		if (menu) {
			if (menu->window.flags & WINDOW_POPUP) {
				Menu_HandleMouseMove(menu, x, y);
				return qtrue;
			}
		}
		for (i = 0; i < menuCount; i++) {
			Menu_HandleMouseMove(&Menus[i], x, y);
		}
	} else {
		menu->window.rect.x += x;
		menu->window.rect.y += y;
		Menu_UpdatePosition(menu);
	}
 	return qtrue;

}

int Display_CursorType(int x, int y) {
	int i;
	for (i = 0; i < menuCount; i++) {
		rectDef_t r2;
		r2.x = Menus[i].window.rect.x - 3;
		r2.y = Menus[i].window.rect.y - 3;
		r2.w = r2.h = 7;
		if (Rect_ContainsPoint(&r2, x, y)) {
			return CURSOR_SIZER;
		}
	}
	return CURSOR_ARROW;
}


void Display_HandleKey(int key, qboolean down, int x, int y) {
	menuDef_t *menu = (menuDef_t *) Display_CaptureItem(x, y);
	if (menu == NULL) {  
		menu = Menu_GetFocused();
	}
	if (menu) {
		Menu_HandleKey(menu, key, down );
	}
}

static void Window_CacheContents(windowDef_t *window) {
	if (window) {
		if (window->cinematicName) {
			int cin = DC->playCinematic(window->cinematicName, 0, 0, 0, 0);
			DC->stopCinematic(cin);
		}
	}
}


static void Item_CacheContents(itemDef_t *item) {
	if (item) {
		Window_CacheContents(&item->window);
	}

}

static void Menu_CacheContents(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CacheContents(&menu->window);
		for (i = 0; i < menu->itemCount; i++) {
			Item_CacheContents(menu->items[i]);
		}

		if (menu->soundName && *menu->soundName) {
			DC->registerSound(menu->soundName);
		}
	}

}

void Display_CacheAll() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CacheContents(&Menus[i]);
	}
}


static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y) {
 	if (menu && menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)) {
		if (Rect_ContainsPoint(&menu->window.rect, x, y)) {
			int i;
			for (i = 0; i < menu->itemCount; i++) {
				// turn off focus each item
				// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

				if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
					continue;
				}

				if (menu->items[i]->window.flags & WINDOW_DECORATION) {
					continue;
				}
				if( menu->items[i]->window.flags & WINDOW_FORCESELECTIONZONE &&
					Rect_ContainsPoint(&menu->items[i]->window.selectionZone, x, y) &&
					menu->items[i]->type != ITEM_TYPE_SLIDER) {
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && *overItem->text) {
						if (Rect_ContainsPoint(&overItem->window.selectionZone, x, y)) {
							return qtrue;
						} else {
							continue;
						}
					} else {
						return qtrue;
					}
				}
				else if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && *overItem->text) {
						if (Rect_ContainsPoint(&overItem->window.rect, x, y)) {
							return qtrue;
						} else {
							continue;
						}
					} else {
						return qtrue;
					}
				}
			}

		}
	}
	return qfalse;
}
