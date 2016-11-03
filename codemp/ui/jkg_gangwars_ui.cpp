#include "qcommon/q_shared.h"
#include "../game/jkg_gangwars.h"
#include "jkg_gangwars_ui.h"
#include "ui_shared.h"
#include "ui_local.h"

int MenuFontToHandle(int iMenuFont);

//void JKG_GangWars_TeamREDText( int iMenuFont, rectDef_t *rect, float scale, vec4_t color )
void JKG_GangWars_TeamREDText( rectDef_t *rect, float scale, vec4_t color, int iMenuFont )
{
	int x,y;
	char line[512];
	int iFontIndex = MenuFontToHandle(iMenuFont);
	int gwTeam = cgImports->GetRedTeam();
	float scale2 = scale;
	if(gwTeam < 0 || !bgGangWarsTeams[gwTeam].menustring[0])
	{
		return;
	}
	strcpy(line, UI_GetStringEdString2(bgGangWarsTeams[gwTeam].menustring));
	y = rect->y;
	x = rect->x + ((rect->w / 2) - (trap->R_Font_StrLenPixels(line, iFontIndex, scale2) / 2));
	trap->R_Font_DrawString(	x, y, line,	bgGangWarsTeams[gwTeam].teamColor, iFontIndex, -1, scale2	);
}

//void JKG_GangWars_TeamBLUEText( int iMenuFont, rectDef_t *rect, float scale, vec4_t color )
void JKG_GangWars_TeamBLUEText( rectDef_t *rect, float scale, vec4_t color, int iMenuFont )
{
	int x,y;
	char line[512];
	float scale2 = scale;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	int gwTeam = cgImports->GetBlueTeam();
	if(gwTeam < 0 || !bgGangWarsTeams[gwTeam].menustring[0])
	{
		return;
	}
	strcpy(line, UI_GetStringEdString2(bgGangWarsTeams[gwTeam].menustring));
	y = rect->y;
	x = rect->x + ((rect->w / 2) - (trap->R_Font_StrLenPixels(line, iFontIndex, scale2) / 2));
	trap->R_Font_DrawString(	x, y, line,	bgGangWarsTeams[gwTeam].teamColor, iFontIndex, -1, scale2	);
}
