/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
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

// cg_info.c -- display information while data is being loading

#include "cg_local.h"
#include "json/cJSON.h"

#define LOADTIP_LEN	160

struct loadingTip_t {
	char tipText[LOADTIP_LEN];
};
static std::vector<loadingTip_t> cg_loadingTips;
static int cg_displayTipNumber = -1;

/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

	trap->UpdateScreen();
#ifdef _DEBUG
	Com_Printf(va("Loading %s\n", s));	//also output current status to console
#endif
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem( int itemNum ) {
	gitem_t		*item;
	char	upperKey[1024];

	item = &bg_itemlist[itemNum];

	if (!item->classname || !item->classname[0])
	{
	//	CG_LoadingString( "Unknown item" );
		return;
	}

	strcpy(upperKey, item->classname);
	CG_LoadingString( CG_GetStringEdString("SP_INGAME",Q_strupr(upperKey)) );
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient( int clientNum ) {
	const char		*info;
	char			personality[MAX_QPATH];

	info = CG_ConfigString( CS_PLAYERS + clientNum );

	Q_strncpyz( personality, Info_ValueForKey( info, "n" ), sizeof(personality) );
	Q_CleanStr( personality );

	CG_LoadingString( personality );
}

/*
===================
CG_LoadBar
===================
*/
void CG_LoadBar(void)
{
	const int numticks = 9, tickwidth = 40, tickheight = 8;
	const int tickpadx = 20, tickpady = 12;
	const int capwidth = 8;
	const int barwidth = numticks * tickwidth + tickpadx * 2 + capwidth * 2, barleft = ((640 - barwidth) / 2);
	const int barheight = tickheight + tickpady * 2, bartop = 480 - barheight;
	const int capleft = barleft + tickpadx, tickleft = capleft + capwidth, ticktop = bartop + tickpady;

	trap->R_SetColor(colorWhite);
	// Draw background
	CG_DrawPic(barleft, bartop, barwidth, barheight, cgs.media.loadBarLEDSurround);

	// Draw left cap (backwards)
	CG_DrawPic(tickleft, ticktop, -capwidth, tickheight, cgs.media.loadBarLEDCap);

	// Draw bar
	CG_DrawPic(tickleft, ticktop, tickwidth*cg.loadLCARSStage, tickheight, cgs.media.loadBarLED);

	// Draw right cap
	CG_DrawPic(tickleft + tickwidth * cg.loadLCARSStage, ticktop, capwidth, tickheight, cgs.media.loadBarLEDCap);
}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
overlays UI_DrawConnectScreen
*/
#define UI_INFOFONT (UI_BIGFONT)
void CG_DrawInformation( void ) {
	const char	*s;
	const char	*info;
	const char	*sysInfo;
	//int			y;
	//int			value, valueNOFP;
	qhandle_t	levelshot;
	//char		buf[1024];
//	int			iPropHeight = 18;	// I know, this is total crap, but as a post release asian-hack....  -Ste
	
	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	s = Info_ValueForKey( info, "mapname" );
	levelshot = trap->R_RegisterShaderNoMip( va( "levelshots/%s", s ) );
	if ( !levelshot ) {
		levelshot = trap->R_RegisterShaderNoMip( "menu/art/unknownmap_mp" );
	}
	trap->R_SetColor( NULL );
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot );

	CG_LoadBar();

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if ( cg.infoScreenText[0] ) {
		const char *psLoading = CG_GetStringEdString("MENUS", "LOADING_MAPNAME");
		UI_DrawProportionalString( 425, 105, ( const char * ) va(( char * ) /*"Loading... %s"*/ psLoading, cg.infoScreenText),
			UI_RIGHT|UI_BIGFONT|UI_DROPSHADOW, colorWhite, FONT_SMALL3 );		
	} else {
		const char *psAwaitingSnapshot = CG_GetStringEdString("MENUS", "AWAITING_SNAPSHOT");
		UI_DrawProportionalString( 425, 128-32, ( const char * )  /*"Awaiting snapshot..."*/psAwaitingSnapshot,
			UI_RIGHT|UI_INFOFONT|UI_DROPSHADOW, colorWhite, FONT_SMALL3 );			
	}

	// Draw the loading screen tip
	if (cg_loadingTips.size() > 0 && cg_displayTipNumber != -1)
	{
		const char* loadingTip = CG_GetStringEdString2(cg_loadingTips.at(cg_displayTipNumber).tipText);
		int x = 320 - CG_Text_Width(loadingTip, 0.3f, FONT_SMALL) / 2;
		int y = 440;
		CG_Text_Paint(x, y, 0.3, colorWhite, loadingTip, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL);
	}
}

/*
===================
CG_ParseLoadingScreenTips
===================
*/
void CG_ParseLoadingScreenTips(void)
{
	fileHandle_t f;
	int len = trap->FS_Open("ext_data/tables/loadingTips.json", &f, FS_READ);
	char* buffer;
	char error[1024];

	memset(error, 0, sizeof(error));

	if (len == 0 || f == NULL_HANDLE)
	{
		Com_Printf("No loading tips to display\n");
		return;
	}

	trap->TrueMalloc((void**)&buffer, len);
	if (buffer == nullptr)
	{
		return;
	}
	trap->FS_Read(buffer, len, f);
	trap->FS_Close(f);

	cJSON* json = cJSON_ParsePooled(buffer, error, 1024);
	if (json == nullptr)
	{
		Com_Printf("Couldn't parse loading tips: %s\n", error);
		trap->TrueFree((void**)&buffer);
		return;
	}

	cJSON* child = cJSON_GetObjectItem(json, "tips");
	if (child)
	{
		int arraySize = cJSON_GetArraySize(child);
		for (int i = 0; i < arraySize; i++)
		{
			cJSON* arrayItem = cJSON_GetArrayItem(child, i);
			loadingTip_t tip;

			Q_strncpyz(tip.tipText, cJSON_ToStringOpt(arrayItem, ""), sizeof(tip));
			cg_loadingTips.push_back(tip);
		}
	}
	trap->TrueFree((void**)&buffer);

	// Display this tip
	cg_displayTipNumber = rand() % (cg_loadingTips.size() - 1);
	//cg_displayTipNumber = Q_irand(0, cg_loadingTips.size() - 1);
}
