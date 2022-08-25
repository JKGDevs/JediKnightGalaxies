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

// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#include "game/bg_saga.h"

#include "../ui/ui_shared.h"
#include "../ui/ui_public.h"

// Jedi Knight Galaxies
#include "jkg_hud.h"
#include "jkg_chatbox.h"
#include "qcommon/q_shared.h"

extern float CG_RadiusForCent( centity_t *cent );
qboolean CG_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y);
qboolean CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle );

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
vec4_t	bluehudtint = {0.5f, 0.5f, 1.0f, 1.0f};
vec4_t	redhudtint = {1.0f, 0.5f, 0.5f, 1.0f};
float	*hudTintColor;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

int lastvalidlockdif;

extern float zoomFov; //this has to be global client-side

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

#ifdef __AUTOWAYPOINT__
extern float aw_percent_complete;
extern void AIMod_AutoWaypoint_DrawProgress( void );
#endif //__AUTOWAYPOINT__

#define MAX_HUD_TICS 4
const char *armorTicName[MAX_HUD_TICS] = 
{
"armor_tic1", 
"armor_tic2", 
"armor_tic3", 
"armor_tic4", 
};

const char *healthTicName[MAX_HUD_TICS] = 
{
"health_tic1", 
"health_tic2", 
"health_tic3", 
"health_tic4", 
};

const char *forceTicName[MAX_HUD_TICS] = 
{
"force_tic1", 
"force_tic2", 
"force_tic3", 
"force_tic4", 
};

const char *ammoTicName[MAX_HUD_TICS] = 
{
"ammo_tic1", 
"ammo_tic2", 
"ammo_tic3", 
"ammo_tic4", 
};

//Called from UI shared code. For now we'll just redirect to the normal anim load function.

void ChatBox_SetPaletteAlpha(float alpha);


int UI_ParseAnimationFile(const char *filename, animation_t *animset, qboolean isHumanoid) 
{
	return BG_ParseAnimationFile(filename, animset, isHumanoid);
}

int MenuFontToHandle(int iMenuFont)
{
	switch (iMenuFont)
	{
		case FONT_SMALL:	return cgDC.Assets.qhSmallFont;
		case FONT_SMALL2:	return cgDC.Assets.qhSmall2Font;
		case FONT_MEDIUM:	return cgDC.Assets.qhMediumFont;
		case FONT_LARGE:	return cgDC.Assets.qhMediumFont;//cgDC.Assets.qhBigFont;
		case FONT_SMALL3:	return cgDC.Assets.qhSmall3Font;
		case FONT_SMALL4:	return cgDC.Assets.qhSmall4Font;
			//fixme? Big fonr isn't registered...?
	}

	return cgDC.Assets.qhMediumFont;
}

int CG_Text_Width(const char *text, float scale, int iMenuFont) 
{
	int iFontIndex = MenuFontToHandle(iMenuFont);

	return trap->R_Font_StrLenPixels(text, iFontIndex, scale);
}

int CG_Text_Height(const char *text, float scale, int iMenuFont) 
{
	int iFontIndex = MenuFontToHandle(iMenuFont);

	return trap->R_Font_HeightPixels(iFontIndex, scale);
}

#include "qcommon/qfiles.h"	// for STYLE_BLINK etc
void CG_Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style, int iMenuFont) 
{
	int iStyleOR = 0;
	int iFontIndex = MenuFontToHandle(iMenuFont);
	
	switch (style)
	{
	case  ITEM_TEXTSTYLE_NORMAL:			iStyleOR = 0;break;					// JK2 normal text
	case  ITEM_TEXTSTYLE_BLINK:				iStyleOR = STYLE_BLINK;break;		// JK2 fast blinking
	case  ITEM_TEXTSTYLE_PULSE:				iStyleOR = STYLE_BLINK;break;		// JK2 slow pulsing
	case  ITEM_TEXTSTYLE_SHADOWED:			iStyleOR = (int)STYLE_DROPSHADOW;break;	// JK2 drop shadow ( need a color for this )
	case  ITEM_TEXTSTYLE_OUTLINED:			iStyleOR = (int)STYLE_DROPSHADOW;break;	// JK2 drop shadow ( need a color for this )
	case  ITEM_TEXTSTYLE_OUTLINESHADOWED:	iStyleOR = (int)STYLE_DROPSHADOW;break;	// JK2 drop shadow ( need a color for this )
	case  ITEM_TEXTSTYLE_SHADOWEDMORE:		iStyleOR = (int)STYLE_DROPSHADOW;break;	// JK2 drop shadow ( need a color for this )
	}

	trap->R_Font_DrawString(	x,		// int ox
							y,		// int oy
							text,	// const char *text
							color,	// paletteRGBA_c c
							iStyleOR | iFontIndex,	// const int iFontHandle
							!limit?-1:limit,		// iCharLimit (-1 = none)
							scale	// const float scale = 1.0f
							);
}

/*
====================
CG_DrawReload
====================
*/

void CG_DrawReload( void ) {
	float phase;
	int x;
	static vec4_t colorBack = {0, 0, 0, 0.7f};
	static vec4_t colorFront = {1, 1, 1, 0.7f};
	
	if (!cg.reloadTimeStart) {
		return;
	}
	if (cg.time > cg.reloadTimeStart + cg.reloadTimeDuration) {
		cg.reloadTimeStart = 0;
		cg.reloadTimeDuration = 0;
		return;
	}
	x = (320 - (trap->R_Font_StrLenPixels("Reloading weapon...", 1, 0.8f) / 2));
	trap->R_Font_DrawString(
			x,
			200,
			"Reloading weapon...",
			colorWhite,
			5,
			-1,
			0.45f);
	phase = (float)(cg.time - cg.reloadTimeStart) / (float)cg.reloadTimeDuration;
	trap->R_SetColor(colorBack);
	trap->R_DrawStretchPic(243, 220, 154, 10, 0, 0, 1, 1, cgs.media.whiteShader);
	trap->R_SetColor(colorFront);
	trap->R_DrawStretchPic(245, 222, 150 * phase, 6, 0, 0, 1, 1, cgs.media.whiteShader);

	trap->R_SetColor(NULL);

}

/*
====================
CG_DrawGrenade

JKGFIXME: Rename. --eez
====================
*/

void CG_DrawGrenade( void )
{
	float	phase;
	int		x;
	static	vec4_t colorBack = { 0, 0, 0, 0.7f };
	static	vec4_t colorFront = { 1, 1, 1, 0.7f };
	
	if ( cg.snap->ps.weapon != WP_THERMAL )
	{
	    return;
	}
	
	if ( cg.jkg_grenadeCookTimer && cg.jkg_grenadeCookTimer >= cg.time )
	{
		x = ( 320 - ( trap->R_Font_StrLenPixels( "Detonation time...", 1, 0.8f ) / 2 ));
		trap->R_Font_DrawString( x, 200, "Detonation time...", colorWhite, 5, -1, 0.45f );

		phase = ( float )( cg.jkg_grenadeCookTimer - cg.time ) / ( float ) GetWeaponData( cg.snap->ps.weapon, cg.snap->ps.weaponVariation )->weaponReloadTime;
		trap->R_SetColor(colorBack);
		trap->R_DrawStretchPic(243, 220, 154, 10, 0, 0, 1, 1, cgs.media.whiteShader);
		trap->R_SetColor(colorFront);
		trap->R_DrawStretchPic(245, 222, 150 * phase, 6, 0, 0, 1, 1, cgs.media.whiteShader);
		trap->R_SetColor(NULL);
	}
}

/*
================
CG_DrawStats

================
*/
static void CG_DrawStatus( void ) 
{
	centity_t		*cent;

	cent = &cg_entities[cg.snap->ps.clientNum];

	CG_DrawHUD(cent);
}


/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawMiniScoreboard
================
*/
static float CG_DrawMiniScoreboard ( float y ) 
{
	char temp[MAX_QPATH];
	int xOffset = 0;

	if ( !cg_drawScores.integer )
	{
		return y;
	}

	if ( cgs.gametype == GT_WARZONE )
	{
		strcpy ( temp, va("Imperials: "));
		Q_strcat ( temp, MAX_QPATH, va("%i",cgs.redtickets) );
		Q_strcat ( temp, MAX_QPATH, va(" Rebels: ") );
		Q_strcat ( temp, MAX_QPATH, va("%i",cgs.bluetickets) );

		CG_Text_Paint( 600 - CG_Text_Width ( temp, 0.4f, FONT_SMALL3 ) + xOffset, y, 0.4f, colorWhite, temp, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, FONT_SMALL3 );
		y += 15;
	}
	else if ( cgs.gametype >= GT_TEAM )
	{
		// GANG WARS STUFF
		temp[0] = '\0';
		strcpy( temp, va("%s: ", CG_GetStringEdString2(bgGangWarsTeams[cgs.redTeam].name)) );
		Q_strcat ( temp, MAX_QPATH, cgs.scores1==SCORE_NOT_PRESENT?"-":(va("%i",cgs.scores1)) );
		Q_strcat ( temp, MAX_QPATH, va(" %s: ", CG_GetStringEdString2(bgGangWarsTeams[cgs.blueTeam].name)) );
		Q_strcat ( temp, MAX_QPATH, cgs.scores2==SCORE_NOT_PRESENT?"-":(va("%i",cgs.scores2)) );

		// Check if we're in spectator
		if(cgs.clientinfo[cg.clientNum].team != TEAM_RED && cgs.clientinfo[cg.clientNum].team != TEAM_BLUE)
		{
			// If so, draw normally
			CG_Text_Paint( 610 - CG_Text_Width ( temp, 0.4f, FONT_SMALL3 ) + xOffset, y - 105, 0.4f, colorWhite, temp, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, FONT_SMALL3 );
			y += 15;
		}
		else
		{
			CG_Text_Paint( 610 - CG_Text_Width ( temp, 0.4f, FONT_SMALL3 ) + xOffset, 80, 0.4f, colorWhite, temp, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, FONT_SMALL3 );
			y = 110;
		}
	}	
	

	return y;
}

/*
================
CG_DrawEnemyInfo
================
*/
static float CG_DrawEnemyInfo ( float y ) 
{
	float		size;
	int			clientNum;
	const char	*title;
	clientInfo_t	*ci;
	int xOffset = 0;


	return y; //Disables rendering of 'leader' on the top right of your screen.


	if (!cg.snap)
	{
		return y;
	}

	if ( !cg_drawEnemyInfo.integer ) 
	{
		return y;
	}

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) 
	{
		return y;
	}

	if ( cg.snap->ps.duelInProgress )
	{
//		title = "Dueling";
		title = CG_GetStringEdString("MP_INGAME", "DUELING");
		clientNum = cg.snap->ps.duelIndex;
	}
	else
	{
		//As of current, we don't want to draw the attacker. Instead, draw whoever is in first place.
		if (cgs.duelWinner < 0 || cgs.duelWinner >= MAX_CLIENTS)
		{
			return y;
		}

		title = va("%s: %i",CG_GetStringEdString("MP_INGAME", "LEADER"), cgs.scores1);

		clientNum = cgs.duelWinner;
	}

	if ( clientNum >= MAX_CLIENTS || !(&cgs.clientinfo[ clientNum ]) )
	{
		return y;
	}

	ci = &cgs.clientinfo[ clientNum ];

	size = ICON_SIZE * 1.25f;
	y += 5;

	if ( ci->modelIcon )
	{
		CG_DrawPic( 640 - size - 5 + xOffset, y, size, size, ci->modelIcon );
	}

	y += size;

	Text_DrawText(630 - Text_GetWidth(ci->name, FONT_SMALL2, 1.0f) + xOffset, y, ci->name, colorWhite, FONT_SMALL2, 0, 1.0f);

	y += 15;

	Text_DrawText(630 - Text_GetWidth(title, FONT_SMALL2, 1.0f) + xOffset, y, title, colorWhite, FONT_SMALL2, 0, 1.0f);	

	return y + BIGCHAR_HEIGHT + 2;
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;
	int			xOffset = 0;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( 635 - w + xOffset, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=====================
CG_DrawRadar

JKGFIXME: oh god, what --eez
=====================
*/
//#define RADAR_RANGE				2500
float	cg_radarRange = 2500.0f;

//#define RADAR_RADIUS			60
float RADAR_RADIUS	=		30;
#define RADAR_X					(580 - RADAR_RADIUS)
//#define RADAR_Y					10 //dynamic based on passed Y val
#define RADAR_CHAT_DURATION		6000
#define	RADAR_MISSILE_RANGE					3000.0f
#define	RADAR_ASTEROID_RANGE				10000.0f
#define	RADAR_MIN_ASTEROID_SURF_WARN_DIST	1200.0f

float CG_DrawRadar ( float y )
{
	vec4_t			color;
	vec4_t			teamColor;
	float			arrow_w;
	float			arrow_h;
	clientInfo_t	*cl;
	clientInfo_t	*local;
	int				i;
	float			arrowBaseScale;
	float			zScale;
	int				xOffset = 0;
	int				y2 = y;

	y2 = -1;//d_poff.integer;
	xOffset = 23;//d_roff.integer;
	RADAR_RADIUS = 40;//d_yoff.value;

	if (!cg.snap)
	{
		return y;
	}

	// Make sure the radar should be showing
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 )
	{
		return y;
	}

	if ( (cg.predictedPlayerState.pm_flags & PMF_FOLLOW) || 
		cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_SPECTATOR ||
		cg.predictedPlayerState.pm_type == PM_SPECTATOR )
	{
		return y;
	}
	
	if( cg.predictedPlayerState.zoomMode != 0)
		return y;

	local = &cgs.clientinfo[ cg.snap->ps.clientNum ];
	if ( !local->infoValid )
	{
		return y;
	}

	// Draw the radar background image
	color[0] = color[1] = color[2] = 1.0f;
	color[3] = 0.6f;
	trap->R_SetColor ( color );

	//Always green for your own team.
	VectorCopy ( g_color_table[ColorIndex(COLOR_GREEN)], teamColor );
	teamColor[3] = 1.0f;

	// Draw all of the radar entities.  Draw them backwards so players are drawn last
	for ( i = cg.radarEntityCount -1 ; i >= 0 ; i-- ) 
	{	
		vec3_t		dirLook;	
		vec3_t		dirPlayer;
		float		angleLook;
		float		anglePlayer;
		float		angle;
		float		distance, actualDist;
		centity_t*	cent;

		cent = &cg_entities[cg.radarEntities[i]];

		// Get the distances first
		VectorSubtract ( cg.predictedPlayerState.origin, cent->lerpOrigin, dirPlayer );		
		dirPlayer[2] = 0;
		actualDist = distance = VectorNormalize ( dirPlayer );

		if ( distance > cg_radarRange * 0.8f) 
		{
			continue;
		}

		distance  = distance / cg_radarRange;
		distance *= RADAR_RADIUS;

		AngleVectors ( cg.predictedPlayerState.viewangles, dirLook, NULL, NULL );

		dirLook[2] = 0;
		anglePlayer = atan2(dirPlayer[0],dirPlayer[1]);		
		VectorNormalize ( dirLook );
		angleLook = atan2(dirLook[0],dirLook[1]);
		angle = angleLook - anglePlayer;

		switch ( cent->currentState.eType )
		{
			default:
				{
					float  x;
					float  ly;
					qhandle_t shader;
					vec4_t    color;

					x = (float)RADAR_X + (float)RADAR_RADIUS + (float)sin (angle) * distance;
					ly = y2 + (float)RADAR_RADIUS + (float)cos (angle) * distance;

					arrowBaseScale = 9.0f;
					shader = 0;
					zScale = 1.0f;

					//we want to scale the thing up/down based on the relative Z (up/down) positioning
					if (cent->lerpOrigin[2] > cg.predictedPlayerState.origin[2])
					{ //higher, scale up (between 16 and 24)
						float dif = (cent->lerpOrigin[2] - cg.predictedPlayerState.origin[2]);
						
						//max out to 1.5x scale at 512 units above local player's height
						dif /= 1024.0f;
						if (dif > 0.5f)
						{
							dif = 0.5f;
						}
						zScale += dif;
					}
					else if (cent->lerpOrigin[2] < cg.predictedPlayerState.origin[2])
					{ //lower, scale down (between 16 and 8)
						float dif = (cg.predictedPlayerState.origin[2] - cent->lerpOrigin[2]);

						//half scale at 512 units below local player's height
						dif /= 1024.0f;
						if (dif > 0.5f)
						{
							dif = 0.5f;
						}
						zScale -= dif;
					}

					arrowBaseScale *= zScale;

					if (cent->currentState.brokenLimbs)
					{ //slightly misleading to use this value, but don't want to add more to entstate.
						//any ent with brokenLimbs non-0 and on radar is an objective ent.
						//brokenLimbs is literal team value.
						char objState[1024];
						int complete;

						//we only want to draw it if the objective for it is not complete.
						//frame represents objective num.
						trap->Cvar_VariableStringBuffer(va("team%i_objective%i", cent->currentState.brokenLimbs, cent->currentState.frame), objState, 1024);

						complete = atoi(objState);

						if (!complete)
						{
						
							// generic enemy index specifies a shader to use for the radar entity.
							if ( cent->currentState.genericenemyindex )
							{
								color[0] = color[1] = color[2] = color[3] = 1.0f;
								shader = cgs.gameIcons[cent->currentState.genericenemyindex];
							}
							else
							{
								if (cg.snap &&
									cent->currentState.brokenLimbs == cg.snap->ps.persistant[PERS_TEAM])
								{
									VectorCopy ( g_color_table[ColorIndex(COLOR_RED)], color );
								}
								else
								{
									VectorCopy ( g_color_table[ColorIndex(COLOR_GREEN)], color );
								}

								shader = cgs.media.siegeItemShader;
							}
						}
					}
					else
					{
						color[0] = color[1] = color[2] = color[3] = 1.0f;
						
						// generic enemy index specifies a shader to use for the radar entity.
						if ( cent->currentState.genericenemyindex )
						{
							shader = cgs.gameIcons[cent->currentState.genericenemyindex];
						}
						else
						{
							shader = cgs.media.siegeItemShader;
						}
					
						if (cent->currentState.eType == ET_FLAG)
						{
							if (cent->currentState.teamowner == TEAM_RED)
							{
								shader = cgs.media.redFlagRadarShader;
							}
							else if (cent->currentState.teamowner == TEAM_BLUE)
							{
								shader = cgs.media.blueFlagRadarShader;
							}
							else
							{
								shader = cgs.media.neutralFlagRadarShader;
							}
						}
					}

					if ( shader )
					{
						// Pulse the alpha if time2 is set.  time2 gets set when the entity takes pain
						if ( (cent->currentState.time2 && cg.time - cent->currentState.time2 < 5000) ||
							(cent->currentState.time2 == 0xFFFFFFFF) )
						{								
							if ( (cg.time / 200) & 1 )
							{
								color[3] = 0.1f + 0.9f * (float) (cg.time % 200) / 200.0f;
							}
							else
							{
								color[3] = 1.0f - 0.9f * (float) (cg.time % 200) / 200.0f;
							}					
						}

						trap->R_SetColor ( color );
						CG_DrawPic ( x - 4 + xOffset, ly - 4, arrowBaseScale, arrowBaseScale, shader );
					}
				}
				break;

			case ET_NPC://FIXME: draw last, with players...
				break; //maybe do something?

			case ET_MOVER:
				break;

			case ET_MISSILE:
				break;

			case ET_PLAYER:
			{
				vec4_t color;

				cl = &cgs.clientinfo[ cent->currentState.number ];

				// not valid then dont draw it
				if ( !cl->infoValid ) 
				{	
					continue;
				}

				VectorCopy4 ( teamColor, color );

				arrowBaseScale = 16.0f;
				zScale = 1.0f;

				// Pulse the radar icon after a voice message
				if ( cent->vChatTime + 2000 > cg.time )
				{
					float f = (cent->vChatTime + 2000 - cg.time) / 3000.0f;
					arrowBaseScale = 16.0f + 4.0f * f;
					color[0] = teamColor[0] + (1.0f - teamColor[0]) * f;
					color[1] = teamColor[1] + (1.0f - teamColor[1]) * f;
					color[2] = teamColor[2] + (1.0f - teamColor[2]) * f;
				}

				trap->R_SetColor ( color );

				//we want to scale the thing up/down based on the relative Z (up/down) positioning
				if (cent->lerpOrigin[2] > cg.predictedPlayerState.origin[2])
				{ //higher, scale up (between 16 and 32)
					float dif = (cent->lerpOrigin[2] - cg.predictedPlayerState.origin[2]);
					
					//max out to 2x scale at 1024 units above local player's height
					dif /= 1024.0f;
					if (dif > 1.0f)
					{
						dif = 1.0f;
					}
					zScale += dif;
				}
                else if (cent->lerpOrigin[2] < cg.predictedPlayerState.origin[2])
				{ //lower, scale down (between 16 and 8)
					float dif = (cg.predictedPlayerState.origin[2] - cent->lerpOrigin[2]);

					//half scale at 512 units below local player's height
					dif /= 1024.0f;
					if (dif > 0.5f)
					{
						dif = 0.5f;
					}
					zScale -= dif;
				}

				arrowBaseScale *= zScale;

				arrow_w = arrowBaseScale * RADAR_RADIUS / 128;
				arrow_h = arrowBaseScale * RADAR_RADIUS / 128;

				CG_DrawRotatePic2( RADAR_X + RADAR_RADIUS + sin (angle) * distance + xOffset,
								   y + RADAR_RADIUS + cos (angle) * distance, 
								   arrow_w, arrow_h, 
								   (360 - cent->lerpAngles[YAW]) + cg.predictedPlayerState.viewangles[YAW], cgs.media.mAutomapPlayerIcon );
				break;
			}
		}
	}

	arrowBaseScale = 16.0f;

	arrow_w = arrowBaseScale * RADAR_RADIUS / 128;
	arrow_h = arrowBaseScale * RADAR_RADIUS / 128;

	vec4_t newColor = {1, 1, 1, 0};
	float ironSightsPhase = JKG_CalculateIronsightsPhase (&cg.predictedPlayerState, cg.time, &cg.ironsightsBlend);
	VectorCopy4(newColor, color);

	if(ironSightsPhase > 0)
	{
		newColor[3] = 1.0f - ironSightsPhase;
	}
	else
	{
		newColor[3] = 1.0f;
	}
	trap->R_SetColor ( newColor );

	CG_DrawRotatePic2( RADAR_X + RADAR_RADIUS + xOffset, y2 + RADAR_RADIUS, arrow_w, arrow_h, 
					   0, cgs.media.mAutomapPlayerIcon );

	return y+(RADAR_RADIUS*2);
}

/*
=================
CG_DrawTeamOverlay
=================
*/
extern const char *CG_GetLocationString(const char *loc); //cg_main.c
static void CG_DrawTeamOverlay() {
	int x, w, h, y, xx;
	int i, len;
	vec4_t		hcolor;
	int pwidth;
	int plyrs;
	clientInfo_t *ci;
	int count;
	float fract;
	float wid;

	if ( !cg_drawTeamOverlay.integer ) {
		return;
	}

	if ( !cgs.party.active && cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return; // Not on any team
	}
	if (!cg.snap) {
		return;
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		ci = &cgs.clientinfo[i];

		if (i == cg.snap->ps.clientNum )
		{
			continue;
		}

		if ( ci->infoValid && (( cgs.party.active && TeamFriendly( i )) || ( !cgs.party.active && ci->team == cg.snap->ps.persistant[PERS_TEAM] )))
		{
			plyrs++;
			len = (float)trap->R_Font_StrLenPixels(ci->name, MenuFontToHandle(1), 1) *0.5f; //CG_DrawStrlen(ci->name);		//futuza note: deals with filling in the actual space and figuring out how wide to make it
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return;
	

	if (pwidth < 82) {
		pwidth = 82;
	}
	w = pwidth + 106;

	x = 0;
	y = 85;
	h = 24;

	/* Overlay design:
		
		+----------------------------------------------------------------------------------------+
		| +------------------+ +--------------------------+ +----------------------------------+ |
		| |                  | |                          | |     Force/stamina  (blue)        | |
		| |                  | |     Player name          | +----------------------------------+ |
		| |    Class         | |                          | +----------------------------------+ |
		| |                  | |                          | |     Shield         (green)       | |
		| |     Picture      | +--------------------------+ +----------------------------------+ |
		| |                  | +---------------------------------------------------------------+ |
		| |                  | |                      Health (red)                             | |
		| +------------------+ +---------------------------------------------------------------+ |
		+----------------------------------------------------------------------------------------+
	*/
	
	for ( i = 0; i < MAX_CLIENTS; i++ )
	{
		ci = &cgs.clientinfo[i];

		if ( i == cg.snap->ps.clientNum )
		{
			continue;
		}

		if ( ci->infoValid && (( cgs.party.active && TeamFriendly( i )) || ( !cgs.party.active && ci->team == cg.snap->ps.persistant[PERS_TEAM] )))
		{
			
			MAKERGBA(hcolor, 0.4f,0.4f,0.4f,0.33f);
			// Draw the box and border
			trap->R_SetColor( hcolor );
			CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
			MAKERGBA(hcolor, 0,0,0,0.33f);
			CG_DrawRect( x, y, w, h, 1, hcolor);
			trap->R_SetColor( NULL );
			
			// Draw the Class Picture
			MAKERGBA(hcolor, 1,1,1,0.6f);
			trap->R_SetColor( hcolor );
			trap->R_DrawStretchPic(x+2,y+2,20,20,0,0,1,1,cgs.media.whiteShader);
			MAKERGBA(hcolor,0,0,0,1);
			CG_DrawRect(x+2, y+2, 20, 20, 1, hcolor);


			// Draw player name
			MAKERGBA(hcolor,1,1,1,1);		
			trap->R_Font_DrawString(x+26, y+2, ci->name, hcolor, MenuFontToHandle(1) | 0x80000000, -1, 0.5f);		
			MAKERGBA(hcolor,0,0,0,1);
			CG_DrawRect(x+24, y+2, pwidth+8 , 13, 1, hcolor);

			xx = x + 34 + pwidth;
			
			// Render Health
			wid = 70 + pwidth + 10;
			fract = (float)ci->health / (float)ci->maxhealth;

			// Render active part (health)
			MAKERGBA(hcolor, 0.9f, 0.0f, 0.0f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(x+24, y+16, wid*fract ,6, 0, 0, 1, 1, cgs.media.whiteShader);
			// Render dimmed part (health)
			MAKERGBA(hcolor, 0.2f, 0.0f, 0.0f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(x+24 + wid*fract, y+16, wid*(1-fract) ,6, 0, 0, 1, 1, cgs.media.whiteShader);
			// Render border (health)
			MAKERGBA(hcolor, 0,0,0,1);
			CG_DrawRect(x+24, y+16, 70 + pwidth + 10 ,6, 1, hcolor);

			// Render Force
			wid = 70;
			fract = (float)ci->curForcePower / (float)ci->maxForcePower;

			// Render active part (Force)
			MAKERGBA(hcolor, 0.0f, 0.0f, 0.9f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(xx, y+2, wid*fract,6, 0, 0, 1, 1, cgs.media.whiteShader);

			// Render dimmed part (Force)
			MAKERGBA(hcolor, 0.0f, 0.0f, 0.2f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(xx + wid*fract, y+2, wid*(1-fract),6, 0, 0, 1, 1, cgs.media.whiteShader);

			// Draw border (Force)
			MAKERGBA(hcolor, 0,0,0,1);
			CG_DrawRect(xx, y+2, 70, 6, 1, hcolor);

			// Render Shield
			wid = 70;
			fract = (float)ci->armor / (float)ci->maxarmor;

			// Render active part (Shield)
			MAKERGBA(hcolor, 0.0f, 0.9f, 0.0f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(xx, y+9, wid*fract ,6, 0, 0, 1, 1, cgs.media.whiteShader);

			// Render dimmed part (Shield)
			MAKERGBA(hcolor, 0.0f, 0.2f, 0.0f, 0.5f);
			trap->R_SetColor(hcolor);
			trap->R_DrawStretchPic(xx+ wid*fract, y+9, wid*(1-fract) ,6, 0, 0, 1, 1, cgs.media.whiteShader);
			
			// Render border (Shield)
			MAKERGBA(hcolor, 0,0,0,1);
			CG_DrawRect(xx, y+9, 70, 6, 1, hcolor);

			trap->R_SetColor(NULL);

			y += 25;
		}
	}

	return;
}


static void CG_DrawPowerupIcons(int y)
{
	int j;
	int ico_size = 64;
	int xOffset = 0;
	gitem_t	*item;

	if (!cg.snap)
	{
		return;
	}

	y += 16;

	//Raz: fixed potential buffer overrun of cg.snap->ps.powerups
	for (j = 0; j < PW_NUM_POWERUPS; j++)
	{
		if (cg.snap->ps.powerups[j] > cg.time)
		{
			int secondsleft = (cg.snap->ps.powerups[j] - cg.time)/1000;

			item = BG_FindItemForPowerup( (powerup_t)j );

			if (item)
			{
				int icoShader = trap->R_RegisterShader( item->icon );

				CG_DrawPic( (640-(ico_size*1.1f)) + xOffset, y, ico_size, ico_size, icoShader );
	
				y += ico_size;

				if (j != PW_REDFLAG && j != PW_BLUEFLAG && secondsleft < 999)
				{
					UI_DrawProportionalString((640-(ico_size*1.1f))+(ico_size/2) + xOffset, y-8, va("%i", secondsleft), UI_CENTER | UI_BIGFONT | UI_DROPSHADOW, colorTable[CT_WHITE], FONT_MEDIUM);
				}

				y += (ico_size/3);
			}
		}
	}
}


/*
=====================
CG_DrawUpperRight

=====================
*/
extern qboolean HaveMiniMap( void );

static void CG_DrawUpperRight( void ) {
	float	y = 120.0f; // UQ1: Changed as the ACI covers this area!

	trap->R_SetColor( colorTable[CT_WHITE] );

	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}

	if ( ( cgs.gametype >= GT_TEAM ) && cg_drawRadar.integer )
	{//draw Radar in Siege mode or when in a vehicle of any kind
		if (!HaveMiniMap())
			CG_DrawRadar ( 0 );
	}

	y = CG_DrawEnemyInfo ( y );

	y = CG_DrawMiniScoreboard ( y );

	CG_DrawPowerupIcons(y);

#ifdef __AUTOWAYPOINT__
	if (aw_percent_complete > 0)
		AIMod_AutoWaypoint_DrawProgress();
#endif //__AUTOWAYPOINT__
}

/*
===============================================================================

LAGOMETER

===============================================================================
*/

//#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;  // bk010215 - FIXME char message[1024];

	if (cg.mMapChange)
	{			
		s = CG_GetStringEdString("MP_INGAME", "SERVER_CHANGING_MAPS");	// s = "Server Changing Maps";			
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

		s = CG_GetStringEdString("MP_INGAME", "PLEASE_WAIT");	// s = "Please wait...";
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( 320 - w/2, 200, s, 1.0F);
		return;
	}

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap->GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap->GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart // bk 0102165 - FIXME
		return;
	}

	// also add text in center of screen
	s = CG_GetStringEdString("MP_INGAME", "CONNECTION_INTERRUPTED"); // s = "Connection Interrupted"; // bk 010215 - FIXME
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic( x, y, 48, 48, trap->R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	x = 640 - 48;
	y = 480 - 144;

	trap->R_SetColor( NULL );
	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap->R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap->R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap->R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap->R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap->R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap->R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap->R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap->R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap->R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap->R_SetColor( NULL );

	if ( cg_noPredict.integer || g_synchronousClients.integer ) {
		CG_DrawBigString( ax, ay, "snc", 1.0f );
	}

	CG_DrawDisconnect();
}

void CG_DrawSiegeMessage( const char *str, int objectiveScreen )
{
//	if (!( trap->Key_GetCatcher() & KEYCATCH_UI ))
	{
		trap->OpenUIMenu(UIMENU_CLOSEALL);
		trap->Cvar_Set("cg_siegeMessage", str);
		if (objectiveScreen)
		{
			trap->OpenUIMenu(UIMENU_SIEGEOBJECTIVES);
		}
		else
		{
			trap->OpenUIMenu(UIMENU_SIEGEMESSAGE);
		}
	}
}

void CG_DrawSiegeMessageNonMenu( const char *str )
{
	char	text[1024];
	if (str[0]=='@')
	{	
		trap->SE_GetStringTextString(str+1, text, sizeof(text));
		str = text;
	}
	CG_CenterPrint(str, SCREEN_HEIGHT * 0.30f, BIGCHAR_WIDTH);
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

#define CG_CP_NUMBER_OF_CHARACTERS_IN_LINE	75		//--futuza: original value 50

/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;
	char	*t = NULL;
	char	*last = NULL;
	char	*lastLine = NULL;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// insert new lines when necessary --eez
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	lastLine = cg.centerPrint;

	while( 1 )
	{
		t = strchr(s, ' ');
		if(t == NULL)
		{
			break; // done.
		}

		if((t - lastLine) >= CG_CP_NUMBER_OF_CHARACTERS_IN_LINE-1)
		{
			// find the last used character
			if(last == NULL)
			{
				// no last space, ABORT!/truncate
				break;
			}
			else
			{
				// replace that one with a \n
				*last = '\n';
				lastLine = last;
			}
		}

		last = t;
		s = t;
		s += 1;
	}

	s = cg.centerPrint;

	// count the number of lines for centering
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		x, y, w;
	int h;
	float	*color;
	const float scale = 0.6f; 

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	trap->R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < CG_CP_NUMBER_OF_CHARACTERS_IN_LINE; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		h = CG_Text_Height(linebuffer, scale, FONT_SMALL3);
		w = CG_Text_Width(linebuffer, scale, FONT_SMALL3);

		x = (SCREEN_WIDTH - w ) / 2;
		CG_Text_Paint(x, y + h, scale, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, FONT_SMALL3);
		y += h + 6;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap->R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/

#define HEALTH_WIDTH		50.0f
#define HEALTH_HEIGHT		5.0f

//draw the health bar based on current "health" and maxhealth
void CG_DrawHealthBar(centity_t *cent, float chX, float chY, float chW, float chH)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = chX+((chW/2)-(HEALTH_WIDTH/2));
	float y = (chY+chH) + 8.0f;
	float percent = ((float)cent->currentState.health/(float)cent->currentState.maxhealth)*HEALTH_WIDTH;
	if (percent <= 0)
	{
		return;
	}

	//color of the bar
	if (!cent->currentState.teamowner || cgs.gametype < GT_TEAM)
	{ //not owned by a team or teamplay
		aColor[0] = 1.0f;
		aColor[1] = 1.0f;
		aColor[2] = 0.0f;
		aColor[3] = 0.4f;
	}
	else if (cent->currentState.teamowner == cg.predictedPlayerState.persistant[PERS_TEAM])
	{ //owned by my team
		aColor[0] = 0.0f;
		aColor[1] = 1.0f;
		aColor[2] = 0.0f;
		aColor[3] = 0.4f;
	}
	else
	{ //hostile
		aColor[0] = 1.0f;
		aColor[1] = 0.0f;
		aColor[2] = 0.0f;
		aColor[3] = 0.4f;
	}

	//color of the border
	bColor[0] = 0.0f;
	bColor[1] = 0.0f;
	bColor[2] = 0.0f;
	bColor[3] = 0.3f;

	//color of greyed out "missing health"
	cColor[0] = 0.5f;
	cColor[1] = 0.5f;
	cColor[2] = 0.5f;
	cColor[3] = 0.4f;

	//draw the background (black)
	CG_DrawRect(x, y, HEALTH_WIDTH, HEALTH_HEIGHT, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much health there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f, percent-1.0f, HEALTH_HEIGHT-1.0f, aColor);

	//then draw the other part greyed out
	CG_FillRect(x+percent, y+1.0f, HEALTH_WIDTH-percent-1.0f, HEALTH_HEIGHT-1.0f, cColor);
}

//same routine (at least for now), draw progress of a "hack" or whatever
void CG_DrawHaqrBar(float chX, float chY, float chW, float chH)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = chX+((chW/2)-(HEALTH_WIDTH/2));
	float y = (chY+chH) + 8.0f;
	float percent = (((float)cg.predictedPlayerState.hackingTime-(float)cg.time)/(float)cg.predictedPlayerState.hackingBaseTime)*HEALTH_WIDTH;

	if (percent > HEALTH_WIDTH ||
		percent < 1.0f)
	{
		return;
	}

	//color of the bar
	aColor[0] = 1.0f;
	aColor[1] = 1.0f;
	aColor[2] = 0.0f;
	aColor[3] = 0.4f;

	//color of the border
	bColor[0] = 0.0f;
	bColor[1] = 0.0f;
	bColor[2] = 0.0f;
	bColor[3] = 0.3f;

	//color of greyed out done area
	cColor[0] = 0.5f;
	cColor[1] = 0.5f;
	cColor[2] = 0.5f;
	cColor[3] = 0.1f;

	//draw the background (black)
	CG_DrawRect(x, y, HEALTH_WIDTH, HEALTH_HEIGHT, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much health there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f, percent-1.0f, HEALTH_HEIGHT-1.0f, aColor);

	//then draw the other part greyed out
	CG_FillRect(x+percent, y+1.0f, HEALTH_WIDTH-percent-1.0f, HEALTH_HEIGHT-1.0f, cColor);

	//draw the hacker icon
	CG_DrawPic(x, y-HEALTH_WIDTH, HEALTH_WIDTH, HEALTH_WIDTH, cgs.media.hackerIconShader);
}

//generic timing bar
int cg_genericTimerBar = 0;
int cg_genericTimerDur = 0;
vec4_t cg_genericTimerColor;
#define CGTIMERBAR_H			50.0f
#define CGTIMERBAR_W			10.0f
#define CGTIMERBAR_X			(SCREEN_WIDTH-CGTIMERBAR_W-120.0f)
#define CGTIMERBAR_Y			(SCREEN_HEIGHT-CGTIMERBAR_H-20.0f)
void CG_DrawGenericTimerBar(void)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = CGTIMERBAR_X;
	float y = CGTIMERBAR_Y;
	float percent = ((float)(cg_genericTimerBar-cg.time)/(float)cg_genericTimerDur)*CGTIMERBAR_H;

	if (percent > CGTIMERBAR_H)
	{
		return;
	}

	if (percent < 0.1f)
	{
		percent = 0.1f;
	}

	//color of the bar
	aColor[0] = cg_genericTimerColor[0];
	aColor[1] = cg_genericTimerColor[1];
	aColor[2] = cg_genericTimerColor[2];
	aColor[3] = cg_genericTimerColor[3];

	//color of the border
	bColor[0] = 0.0f;
	bColor[1] = 0.0f;
	bColor[2] = 0.0f;
	bColor[3] = 0.3f;

	//color of greyed out "missing fuel"
	cColor[0] = 0.5f;
	cColor[1] = 0.5f;
	cColor[2] = 0.5f;
	cColor[3] = 0.1f;

	//draw the background (black)
	CG_DrawRect(x, y, CGTIMERBAR_W, CGTIMERBAR_H, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much health there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f+(CGTIMERBAR_H-percent), CGTIMERBAR_W-2.0f, CGTIMERBAR_H-1.0f-(CGTIMERBAR_H-percent), aColor);

	//then draw the other part greyed out
	CG_FillRect(x+1.0f, y+1.0f, CGTIMERBAR_W-2.0f, CGTIMERBAR_H-percent, cColor);
}

/*
=================
CG_DrawCrosshair
=================
*/

float cg_crosshairPrevPosX = 0;
float cg_crosshairPrevPosY = 0;
#define CRAZY_CROSSHAIR_MAX_ERROR_X	(100.0f*640.0f/480.0f)
#define CRAZY_CROSSHAIR_MAX_ERROR_Y	(100.0f)
void CG_LerpCrosshairPos( float *x, float *y )
{
	if ( cg_crosshairPrevPosX )
	{//blend from old pos
		float maxMove = 30.0f * ((float)cg.frametime/500.0f) * 640.0f/480.0f;
		float xDiff = (*x - cg_crosshairPrevPosX);
		if ( fabs(xDiff) > CRAZY_CROSSHAIR_MAX_ERROR_X )
		{
			maxMove = CRAZY_CROSSHAIR_MAX_ERROR_X;
		}
		if ( xDiff > maxMove )
		{
			*x = cg_crosshairPrevPosX + maxMove;
		}
		else if ( xDiff < -maxMove )
		{
			*x = cg_crosshairPrevPosX - maxMove;
		}
	}
	cg_crosshairPrevPosX = *x;

	if ( cg_crosshairPrevPosY )
	{//blend from old pos
		float maxMove = 30.0f * ((float)cg.frametime/500.0f);
		float yDiff = (*y - cg_crosshairPrevPosY);
		if ( fabs(yDiff) > CRAZY_CROSSHAIR_MAX_ERROR_Y )
		{
			maxMove = CRAZY_CROSSHAIR_MAX_ERROR_X;
		}
		if ( yDiff > maxMove )
		{
			*y = cg_crosshairPrevPosY + maxMove;
		}
		else if ( yDiff < -maxMove )
		{
			*y = cg_crosshairPrevPosY - maxMove;
		}
	}
	cg_crosshairPrevPosY = *y;
}

/*static */void CG_DrawCrosshair( vec3_t worldPoint, int chEntValid ) {
	float		w, h;
	qhandle_t	hShader = 0;
	float		f;
	float		x, y;
	qboolean	corona = qfalse;
	vec4_t		ecolor = {0,0,0,0};
	centity_t	*crossEnt = NULL;
	float		chX, chY;
	weaponData_t *wp = GetWeaponData(cg.predictedPlayerState.weapon, cg.predictedPlayerState.weaponVariation);

	if ( !cg_drawCrosshair.integer ) 
	{
		return;
	}

	if (cg.snap->ps.fallingToDeath)
	{
		return;
	}

	if (cg.snap->ps.weapon == WP_MELEE || cg.snap->ps.weapon == WP_SABER || cg.snap->ps.weapon == WP_NONE) 
	{
		if (!jkg_gunlesscrosshair.integer) {
			return;			// If they player doesnt have gun-less crosshairs enabled, dont render it
		}
	}

	if ( cg.predictedPlayerState.zoomMode != 0 )
	{//not while scoped
		return;
	}

	if ( cg_crosshairHealth.integer )
	{
		vec4_t		hcolor;

		CG_ColorForHealth( hcolor );
		trap->R_SetColor( hcolor );
	}
	else
	{
		//set color based on what kind of ent is under crosshair
		//if ( cg.crosshairClientNum >= ENTITYNUM_WORLD )
		{
		    // JKG: Crosshair disappears for ironsights
		    VectorSet (ecolor, 1.0f, 1.0f, 1.0f);
		    if ( cg.renderingThirdPerson )
		    {
		        ecolor[3] = 1.0f;
		    }
		    else
		    {
				float ironSightsPhase = JKG_CalculateIronsightsPhase (&cg.predictedPlayerState, cg.time, &cg.ironsightsBlend);
				float sprintPhase = JKG_CalculateSprintPhase (&cg.predictedPlayerState);
				if(ironSightsPhase > 0)
				{
					ecolor[3] = 1.0f - ironSightsPhase;
				}
				else if(sprintPhase > 0)
				{
					ecolor[3] = 1.0f - sprintPhase;
				}
				else
				{
					ecolor[3] = 1.0f;
				}
            }

			// display heat %
			if (cg_drawWpnHeatValue.integer > 0)
			{
				char printheat[15];  
				float heatPercent = ((cg.predictedPlayerState.heat / cg.predictedPlayerState.maxHeat) * 100);
				
				if (heatPercent > 0)
				{
					if (heatPercent > 99)
						sprintf(printheat, "Heat: 100.00");

					else
						sprintf(printheat, "Heat: %.2f", heatPercent);

					CG_CenterPrint(printheat, SCREEN_HEIGHT * 0.30f, BIGCHAR_WIDTH);
				}
			}

			// Crosshair gets more red with heat
			ecolor[2] = ecolor[1] = (1.0f - (cg.predictedPlayerState.heat / cg.predictedPlayerState.maxHeat));
			if (ecolor[2] >= 1.0f)
			{
				ecolor[2] = ecolor[1] = 1.0f;
			}
			else if (ecolor[2] <= 0.0f)
			{
				ecolor[2] = ecolor[1] = 0.0f;
			}
            
			trap->R_SetColor( ecolor );
		}

		//rwwFIXMEFIXME: Write this a different way, it's getting a bit too sloppy looking
		if ( cg.crosshairClientNum < ENTITYNUM_WORLD && chEntValid &&
			(cg_entities[cg.crosshairClientNum].currentState.number < MAX_CLIENTS ||
			cg_entities[cg.crosshairClientNum].currentState.eType == ET_NPC ||
			cg_entities[cg.crosshairClientNum].currentState.shouldtarget ||
			cg_entities[cg.crosshairClientNum].currentState.health || //always show ents with health data under crosshair
			(cg_entities[cg.crosshairClientNum].currentState.eType == ET_MOVER && cg_entities[cg.crosshairClientNum].currentState.bolt1 && cg.predictedPlayerState.weapon == WP_SABER) ||
			(cg_entities[cg.crosshairClientNum].currentState.eType == ET_MOVER && cg_entities[cg.crosshairClientNum].currentState.teamowner)))
		{
			crossEnt = &cg_entities[cg.crosshairClientNum];

			if ( crossEnt->currentState.powerups & (1 <<PW_CLOAKED) )
			{ //don't show up for cloaked guys
				ecolor[0] = 1.0f;//R
				ecolor[1] = 1.0f;//G
				ecolor[2] = 1.0f;//B
			}
			else if ( crossEnt->currentState.number < MAX_CLIENTS )
			{
				// JKG - Proper crosshair coloring
				if (TeamFriendly(crossEnt->currentState.number)) {
					// He's in our party/team, so show as green
					ecolor[0] = 0.0f;
					ecolor[1] = 1.0f;
					ecolor[2] = 0.0f;
				} else {
					// Otherwise show red
					ecolor[0] = 1.0f;
					ecolor[1] = 0.0f;
					ecolor[2] = 0.0f;
				}

				if (cg.snap->ps.duelInProgress)
				{
					if (crossEnt->currentState.number != cg.snap->ps.duelIndex)
					{ //grey out crosshair for everyone but your foe if you're in a duel
						ecolor[0] = 0.4f;
						ecolor[1] = 0.4f;
						ecolor[2] = 0.4f;
					}
				}
				else if (crossEnt->currentState.bolt1)
				{ //this fellow is in a duel. We just checked if we were in a duel above, so
				  //this means we aren't and he is. Which of course means our crosshair greys out over him.
					ecolor[0] = 0.4f;
					ecolor[1] = 0.4f;
					ecolor[2] = 0.4f;
				}
			}
			else if (crossEnt->currentState.shouldtarget || crossEnt->currentState.eType == ET_NPC)
			{
				//VectorCopy( crossEnt->startRGBA, ecolor );
				if ( !ecolor[0] && !ecolor[1] && !ecolor[2] )
				{
					// We really don't want black, so set it to yellow
					ecolor[0] = 1.0F;//R
					ecolor[1] = 0.8F;//G
					ecolor[2] = 0.3F;//B
				}

				if (crossEnt->currentState.eType == ET_NPC)
				{
					int plTeam = NPCTEAM_PLAYER;

					if ( crossEnt->currentState.powerups & (1 <<PW_CLOAKED) )
					{
						ecolor[0] = 1.0f;//R
						ecolor[1] = 1.0f;//G
						ecolor[2] = 1.0f;//B
					}
					else if ( !crossEnt->currentState.teamowner )
					{ //not on a team
						if (!crossEnt->currentState.teamowner )
						{ //neutral
							if (crossEnt->currentState.owner < MAX_CLIENTS)
							{ //base color on who is pilotting this thing
								clientInfo_t *ci = &cgs.clientinfo[crossEnt->currentState.owner];

								if (cgs.gametype >= GT_TEAM && ci->team == cg.predictedPlayerState.persistant[PERS_TEAM])
								{ //friendly
									ecolor[0] = 0.0f;//R
									ecolor[1] = 1.0f;//G
									ecolor[2] = 0.0f;//B
								}
								else
								{ //hostile
									ecolor[0] = 1.0f;//R
									ecolor[1] = 0.0f;//G
									ecolor[2] = 0.0f;//B
								}
							}
							else
							{ //unmanned
								ecolor[0] = 1.0f;//R
								ecolor[1] = 1.0f;//G
								ecolor[2] = 0.0f;//B
							}
						}
						else
						{
							ecolor[0] = 1.0f;//R
							ecolor[1] = 0.0f;//G
							ecolor[2] = 0.0f;//B
						}
					}
					else if ( crossEnt->currentState.teamowner != plTeam )
					{// on enemy team
						ecolor[0] = 1.0f;//R
						ecolor[1] = 0.0f;//G
						ecolor[2] = 0.0f;//B
					}
					else
					{ //a friend
						ecolor[0] = 0.0f;//R
						ecolor[1] = 1.0f;//G
						ecolor[2] = 0.0f;//B
					}
				}
				else if ( crossEnt->currentState.teamowner == TEAM_RED
					|| crossEnt->currentState.teamowner == TEAM_BLUE )
				{
					if (cgs.gametype < GT_TEAM)
					{ //not teamplay, just neutral then
						ecolor[0] = 1.0f;//R
						ecolor[1] = 1.0f;//G
						ecolor[2] = 0.0f;//B
					}
					else if ( crossEnt->currentState.teamowner != cgs.clientinfo[cg.snap->ps.clientNum].team )
					{ //on the enemy team
						ecolor[0] = 1.0f;//R
						ecolor[1] = 0.0f;//G
						ecolor[2] = 0.0f;//B
					}
					else
					{ //on my team
						ecolor[0] = 0.0f;//R
						ecolor[1] = 1.0f;//G
						ecolor[2] = 0.0f;//B
					}
				}
				else if (crossEnt->currentState.owner == cg.snap->ps.clientNum || TeamFriendly(crossEnt->currentState.owner) ||
					(cgs.gametype >= GT_TEAM && crossEnt->currentState.teamowner == cgs.clientinfo[cg.snap->ps.clientNum].team))
				{
					// I own this entity, or a team/party member owns it
					ecolor[0] = 0.0f;//R
					ecolor[1] = 1.0f;//G
					ecolor[2] = 0.0f;//B
				}
				else if (crossEnt->currentState.teamowner == 16 ||
					(cgs.gametype >= GT_TEAM && crossEnt->currentState.teamowner && crossEnt->currentState.teamowner != cgs.clientinfo[cg.snap->ps.clientNum].team))
				{
					ecolor[0] = 1.0f;//R
					ecolor[1] = 0.0f;//G
					ecolor[2] = 0.0f;//B
				}
			}
			else if (crossEnt->currentState.eType == ET_MOVER && crossEnt->currentState.bolt1 && cg.predictedPlayerState.weapon == WP_SABER)
			{ //can push/pull this mover. Only show it if we're using the saber.
				ecolor[0] = 0.2f;
				ecolor[1] = 0.5f;
				ecolor[2] = 1.0f;

				corona = qtrue;
			}
			else if (crossEnt->currentState.eType == ET_MOVER && crossEnt->currentState.teamowner)
			{ //a team owns this - if it's my team green, if not red, if not teamplay then yellow
				if (cgs.gametype < GT_TEAM)
				{
					ecolor[0] = 1.0f;//R
					ecolor[1] = 1.0f;//G
					ecolor[2] = 0.0f;//B
				}
                else if (cg.predictedPlayerState.persistant[PERS_TEAM] != crossEnt->currentState.teamowner)
				{ //not my team
					ecolor[0] = 1.0f;//R
					ecolor[1] = 0.0f;//G
					ecolor[2] = 0.0f;//B
				}
				else
				{ //my team
					ecolor[0] = 0.0f;//R
					ecolor[1] = 1.0f;//G
					ecolor[2] = 0.0f;//B
				}
			}
			else if (crossEnt->currentState.health)
			{
				if (!crossEnt->currentState.teamowner || cgs.gametype < GT_TEAM)
				{ //not owned by a team or teamplay
					ecolor[0] = 1.0f;
					ecolor[1] = 1.0f;
					ecolor[2] = 0.0f;
				}
				else if (crossEnt->currentState.teamowner == cg.predictedPlayerState.persistant[PERS_TEAM])
				{ //owned by my team
					ecolor[0] = 0.0f;
					ecolor[1] = 1.0f;
					ecolor[2] = 0.0f;
				}
				else
				{ //hostile
					ecolor[0] = 1.0f;
					ecolor[1] = 0.0f;
					ecolor[2] = 0.0f;
				}
			}
			//[USE_ITEMS]
			else if ( crossEnt->currentState.eType == ET_ITEM )
			{
			    ecolor[0] = 1.0f;
			    ecolor[1] = 1.0f;
			    ecolor[2] = 0.0f;
			}
			//[/USE_ITEMS]

            // JKG: Crosshair disappears for ironsights
            if ( !cg.renderingThirdPerson )
            {
				float ironSightsPhase = JKG_CalculateIronsightsPhase (&cg.predictedPlayerState, cg.time, &cg.ironsightsBlend);
				float sprintPhase = JKG_CalculateSprintPhase (&cg.predictedPlayerState);
				if(ironSightsPhase > 0)
				{
					ecolor[3] = 1.0f - ironSightsPhase;
				}
				else if(sprintPhase > 0)
				{
					ecolor[3] = 1.0f - sprintPhase;
				}
            }
            else
            {
                ecolor[3] = 1.0f;
            }
            
			trap->R_SetColor( ecolor );
		}
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	if ( worldPoint && VectorLength( worldPoint ) )
	{
		if ( !CG_WorldCoordToScreenCoordFloat( worldPoint, &x, &y ) )
		{//off screen, don't draw it
			return;
		}
		//CG_LerpCrosshairPos( &x, &y );
		x -= 320;
		y -= 240;
	}
	else
	{
		x = cg_crosshairX.integer;
		y = cg_crosshairY.integer;
	}

	if ( !hShader )
	{
		char* fmShader = wp->visuals.visualFireModes[cg.predictedPlayerState.firingMode].crosshairShader;
		if (JKG_SimpleAmmoOverridePresent(ammoTable[cg.predictedPlayerState.ammoType].visualOverrides.crosshairShader))
		{
			fmShader = (char*)ammoTable[cg.predictedPlayerState.ammoType].visualOverrides.crosshairShader.second.c_str();
		}
		if ( fmShader[0] ) {
			hShader = trap->R_RegisterShaderNoMip(fmShader);
		}
		else {
			hShader = cgs.media.crosshairShader[ cg_drawCrosshair.integer % NUM_CROSSHAIRS ];
		}
	}

	chX = x + cg.refdef.x + 0.5f * (640 - w);
	chY = y + cg.refdef.y + 0.5f * (480 - h);
	trap->R_DrawStretchPic( chX, chY, w, h, 0, 0, 1, 1, hShader );

	// draw hitmarker --eez
	if(cg.hitmarkerLastTime > cg.time)
	{
		trap->R_DrawStretchPic( chX, chY, w, h, 0, 0, 1, 1, cgs.media.hitmarkerGraphic );
	}

	//draw a health bar directly under the crosshair if we're looking at something
	//that takes damage
	if (crossEnt &&
		crossEnt->currentState.maxhealth)
	{
		CG_DrawHealthBar(crossEnt, chX, chY, w, h);
		chY += HEALTH_HEIGHT*2;
	}

	if (cg.predictedPlayerState.hackingTime)
	{ //hacking something
		CG_DrawHaqrBar(chX, chY, w, h);
	}

	if (cg_genericTimerBar > cg.time)
	{ //draw generic timing bar, can be used for whatever
		CG_DrawGenericTimerBar();
	}

	if ( corona ) // drawing extra bits
	{
		ecolor[3] = 0.5f;
		ecolor[0] = ecolor[1] = ecolor[2] = (1 - ecolor[3]) * ( sin( cg.time * 0.001f ) * 0.08f + 0.35f ); // don't draw full color
		ecolor[3] = 1.0f;

		trap->R_SetColor( ecolor );

		w *= 2.0f;
		h *= 2.0f;

		trap->R_DrawStretchPic( x + cg.refdef.x + 0.5f * (640 - w), 
			y + cg.refdef.y + 0.5f * (480 - h), 
			w, h, 0, 0, 1, 1, cgs.media.forceCoronaShader );
	}

	//Raz: Was missing this
	trap->R_SetColor( NULL );
}

qboolean CG_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y)
{
	float	xcenter, ycenter;
	vec3_t	local, transformed;
	vec3_t	vfwd;
	vec3_t	vright;
	vec3_t	vup;
	float xzi;
	float yzi;

//	xcenter = cg.refdef.width / 2;//gives screen coords adjusted for resolution
//	ycenter = cg.refdef.height / 2;//gives screen coords adjusted for resolution
	
	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = 640.0f / 2.0f;//gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480.0f / 2.0f;//gives screen coords in virtual 640x480, to be adjusted when drawn

	AngleVectors (cg.refdef.viewangles, vfwd, vright, vup);

	VectorSubtract (worldCoord, cg.refdef.vieworg, local);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);		

	// Make sure Z is not negative.
	if(transformed[2] < 0.01f)
	{
		return qfalse;
	}

	xzi = xcenter / transformed[2] * (96.0f/cg.refdef.fov_x);
	yzi = ycenter / transformed[2] * (102.0f/cg.refdef.fov_y);

	*x = xcenter + xzi * transformed[0];
	*y = ycenter - yzi * transformed[1];

	return qtrue;
}

qboolean CG_WorldCoordToScreenCoord( vec3_t worldCoord, int *x, int *y )
{
	float	xF, yF;
	qboolean retVal = CG_WorldCoordToScreenCoordFloat( worldCoord, &xF, &yF );
	*x = (int)xF;
	*y = (int)yF;
	return retVal;
}

/*
====================
CG_SaberClashFlare
====================
*/
int cg_saberFlashTime = 0;
vec3_t cg_saberFlashPos = {0, 0, 0};
void CG_SaberClashFlare( void ) 
{
	int				t, maxTime = 150;
	vec3_t dif;
	vec4_t color;
	int x,y;
	float v, len;
	trace_t tr;

	t = cg.time - cg_saberFlashTime;

	if ( t <= 0 || t >= maxTime ) 
	{
		return;
	}

	// Don't do clashes for things that are behind us
	VectorSubtract( cg_saberFlashPos, cg.refdef.vieworg, dif );

	if ( DotProduct( dif, cg.refdef.viewaxis[0] ) < 0.2f )
	{
		return;
	}

	CG_Trace( &tr, cg.refdef.vieworg, NULL, NULL, cg_saberFlashPos, -1, CONTENTS_SOLID );

	if ( tr.fraction < 1.0f )
	{
		return;
	}

	len = VectorNormalize( dif );

	// clamp to a known range
	if ( len > 1200 )
	{
		return;
	}

	v = ( 1.0f - ((float)t / maxTime )) * ((1.0f - ( len / 800.0f )) * 2.0f + 0.35f);
	if (v < 0.001f)
	{
		v = 0.001f;
	}

	CG_WorldCoordToScreenCoord( cg_saberFlashPos, &x, &y );

	VectorSet4( color, 0.8f, 0.8f, 0.8f, 1.0f );
	trap->R_SetColor( color );

	CG_DrawPic( x - ( v * 300 ), y - ( v * 300 ),
				v * 600, v * 600,
				trap->R_RegisterShader( "gfx/effects/saberFlare" ));
}

void CG_DottedLine( float x1, float y1, float x2, float y2, float dotSize, int numDots, vec4_t color, float alpha )
{
	float x, y, xDiff, yDiff, xStep, yStep;
	vec4_t colorRGBA;
	int dotNum = 0;

	VectorCopy4( color, colorRGBA );
	colorRGBA[3] = alpha;

	trap->R_SetColor( colorRGBA );

	xDiff = x2-x1;
	yDiff = y2-y1;
	xStep = xDiff/(float)numDots;
	yStep = yDiff/(float)numDots;

	for ( dotNum = 0; dotNum < numDots; dotNum++ )
	{
		x = x1 + (xStep*dotNum) - (dotSize*0.5f);
		y = y1 + (yStep*dotNum) - (dotSize*0.5f);

		CG_DrawPic( x, y, dotSize, dotSize, cgs.media.whiteShader );
	}
}

static qboolean CG_IsDurationPower(int power)
{
	if (power == FP_HEAL ||
		power == FP_SPEED ||
		power == FP_TELEPATHY ||
		power == FP_RAGE ||
		power == FP_PROTECT ||
		power == FP_ABSORB ||
		power == FP_SEE)
	{
		return qtrue;
	}

	return qfalse;
}

//--------------------------------------------------------------
static void CG_DrawActivePowers(void)
//--------------------------------------------------------------
{
	int icon_size = 40;
	int i = 0;
	int startx = icon_size*2+16;
	int starty = SCREEN_HEIGHT - icon_size*2;

	int endx = icon_size;
	int endy = icon_size;

	if (cg.snap->ps.zoomMode)
	{ //don't display over zoom mask
		return;
	}

	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		return;
	}

	trap->R_SetColor( NULL );

	while (i < NUM_FORCE_POWERS)
	{
		if ((cg.snap->ps.fd.forcePowersActive & (1 << forcePowerSorted[i])) &&
			CG_IsDurationPower(forcePowerSorted[i]))
		{
			CG_DrawPic( startx, starty, endx, endy, cgs.media.forcePowerIcons[forcePowerSorted[i]]);
			startx += (icon_size+2); //+2 for spacing
			if ((startx+icon_size) >= SCREEN_WIDTH-80)
			{
				startx = icon_size*2+16;
				starty += (icon_size+2);
			}
		}

		i++;
	}

	//additionally, draw an icon force force rage recovery
	if (cg.snap->ps.fd.forceRageRecoveryTime > cg.time)
	{
		CG_DrawPic( startx, starty, endx, endy, cgs.media.rageRecShader);
	}
}

//calc the muzzle point from the e-web itself
void CG_CalcEWebMuzzlePoint(centity_t *cent, vec3_t start, vec3_t d_f, vec3_t d_rt, vec3_t d_up)
{
	int bolt = trap->G2API_AddBolt(cent->ghoul2, 0, "*cannonflash");

	assert(bolt != -1);

	if (bolt != -1)
	{
		mdxaBone_t boltMatrix;

		trap->G2API_GetBoltMatrix_NoRecNoRot(cent->ghoul2, 0, bolt, &boltMatrix, cent->lerpAngles, cent->lerpOrigin, cg.time, NULL, cent->modelScale);
		BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, start);
		BG_GiveMeVectorFromMatrix(&boltMatrix, NEGATIVE_X, d_f);

		//these things start the shot a little inside the bbox to assure not starting in something solid
		VectorMA(start, -16.0f, d_f, start);

		//I guess
		VectorClear( d_rt );//don't really need this, do we?
		VectorClear( d_up );//don't really need this, do we?
	}
}

/*
=================
CG_`Entity
=================
*/
#define MAX_XHAIR_DIST_ACCURACY	20000.0f
void CG_TraceItem ( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber );
static void CG_ScanForCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content;
	int			ignore;

	ignore = cg.predictedPlayerState.clientNum;

	if ( cg_dynamicCrosshair.integer )
	{
		vec3_t d_f, d_rt, d_up;
		//For now we still want to draw the crosshair in relation to the player's world coordinates
		//even if we have a melee weapon/no weapon.
		if (cg.snap && cg.snap->ps.weapon == WP_EMPLACED_GUN && cg.snap->ps.emplacedIndex &&
			cg_entities[cg.snap->ps.emplacedIndex].ghoul2 && cg_entities[cg.snap->ps.emplacedIndex].currentState.weapon == WP_NONE)
		{ //locked into our e-web, calc the muzzle from it
			CG_CalcEWebMuzzlePoint(&cg_entities[cg.snap->ps.emplacedIndex], start, d_f, d_rt, d_up);
		}
		else
		{
			if (cg.snap && cg.snap->ps.weapon == WP_EMPLACED_GUN && cg.snap->ps.emplacedIndex)
			{
				vec3_t pitchConstraint;

				ignore = cg.snap->ps.emplacedIndex;

				VectorCopy(cg.refdef.viewangles, pitchConstraint);

				if (cg.renderingThirdPerson)
				{
					VectorCopy(cg.predictedPlayerState.viewangles, pitchConstraint);
				}
				else
				{
					VectorCopy(cg.refdef.viewangles, pitchConstraint);
				}

				if (pitchConstraint[PITCH] > 40)
				{
					pitchConstraint[PITCH] = 40;
				}

				AngleVectors( pitchConstraint, d_f, d_rt, d_up );
			}
			else
			{
				vec3_t pitchConstraint;

				if (cg.renderingThirdPerson)
				{
					VectorCopy(cg.predictedPlayerState.viewangles, pitchConstraint);
				}
				else
				{
					VectorCopy(cg.refdef.viewangles, pitchConstraint);
				}

				AngleVectors( pitchConstraint, d_f, d_rt, d_up );
			}
			CG_CalcMuzzlePoint(cg.snap->ps.clientNum, start);
		}

		VectorMA( start, cg.distanceCull, d_f, end );
	}
	else
	{
		VectorCopy( cg.refdef.vieworg, start );
		VectorMA( start, 131072, cg.refdef.viewaxis[0], end );
	}
	//[USE_ITEMS]
	if ( cg_dynamicCrosshair.integer && cg_dynamicCrosshairPrecision.integer )
	{
	    vec3_t itemEnd;
	    vec3_t fwd;
	    
	    VectorSubtract (end, start, fwd);
	    VectorNormalize (fwd);
	    
	    VectorMA (start, 128.0f, fwd, itemEnd);
	    
	    trace.fraction = 0.0f;
	    CG_TraceItem (&trace, start, vec3_origin, vec3_origin, itemEnd, ignore);
	    if ( trace.entityNum < ENTITYNUM_NONE && trace.fraction < 1.0f )
	    {
	        cg.crosshairClientNum = trace.entityNum;
	        cg.crosshairClientTime = cg.time;
	        
	        CG_DrawCrosshair (trace.endpos, 1);
	        return;
	    }
	}
	//[/USE_ITEMS]

	if ( cg_dynamicCrosshair.integer && cg_dynamicCrosshairPrecision.integer )
	{ //then do a trace with ghoul2 models in mind
		CG_G2Trace( &trace, start, vec3_origin, vec3_origin, end, 
			ignore, CONTENTS_SOLID|CONTENTS_BODY );
	}
	else
	{
		CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
			ignore, CONTENTS_SOLID|CONTENTS_BODY );
	}

	if (trace.entityNum < MAX_CLIENTS)
	{
		if (CG_IsMindTricked(cg_entities[trace.entityNum].currentState.trickedentindex,
			cg_entities[trace.entityNum].currentState.trickedentindex2,
			cg_entities[trace.entityNum].currentState.trickedentindex3,
			cg_entities[trace.entityNum].currentState.trickedentindex4,
			cg.snap->ps.clientNum))
		{
			if (cg.crosshairClientNum == trace.entityNum)
			{
				cg.crosshairClientNum = ENTITYNUM_NONE;
				cg.crosshairClientTime = 0;
			}

			CG_DrawCrosshair(trace.endpos, 0);

			return; //this entity is mind-tricking the current client, so don't render it
		}
	}

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
	{
		if (trace.entityNum < ENTITYNUM_WORLD)
		{
			cg.crosshairClientNum = trace.entityNum;
			cg.crosshairClientTime = cg.time;

			CG_DrawCrosshair(trace.endpos, 1);
		}
		else
		{
			CG_DrawCrosshair(trace.endpos, 0);
		}
	}

	//Raz: Put this back in so fading works again
	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	// if the player is in fog, don't show it
	content = CG_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}

int			next_vischeck[MAX_GENTITIES];
qboolean	currently_visible[MAX_GENTITIES];

qboolean CG_CheckClientVisibility ( centity_t *cent )
{
	trace_t		trace;
	vec3_t		start, end;//, forward, right, up;
	centity_t	*traceEnt = NULL;

	if (next_vischeck[cent->currentState.number] > cg.time)
	{
		return currently_visible[cent->currentState.number];
	}

	next_vischeck[cent->currentState.number] = cg.time + 500 + Q_irand(500, 1000);

	VectorCopy(cg.refdef.vieworg, start);
	start[2]+=42;

	VectorCopy(cent->lerpOrigin, end);
	end[2]+=42;

	CG_Trace( &trace, start, NULL, NULL, end, cg.clientNum, MASK_PLAYERSOLID/*MASK_SHOT*/ );

	traceEnt = &cg_entities[trace.entityNum];

	if (traceEnt == cent || trace.fraction == 1.0f)
	{
		currently_visible[cent->currentState.number] = qtrue;
		return qtrue;
	}

	currently_visible[cent->currentState.number] = qfalse;
	return qfalse;
}

/*
=====================================

NPC NAMEPLATES

=====================================
*/

#include "game/bg_npcnames.h"

// Float a NPC name above their head!
void CG_DrawNPCNames( void )
{
	// FIXME: localize strings within .str files --eez

	// Load the list on first check...
	BG_Load_NPC_Names();

	for (int i = 0; i < cg.snap->numEntities; ++i)
	{
		const entityState_t* es = cg.snap->entities + i;
		centity_t *cent = &cg_entities[es->number];

		if (cent->currentState.eType != ET_NPC)
			continue;

		if (!cent->ghoul2)
			continue;

		if (!cent->npcClient)
			continue;

		if (cent->cloaked)
			continue;

		if (cent->currentState.eFlags & (EF_DEAD | EF_NODRAW))
			continue;

		static const float MAX_DISTANCE = 1024.0f;
		const float distance = Distance(cg.snap->ps.origin, cent->lerpOrigin);
		if (distance > MAX_DISTANCE)
			continue;

		if (!CG_CheckClientVisibility(cent))
			continue;

		vec4_t textColor = {0.325f, 0.325f, 1.0f, 1.0f};
		const char *npcName;
		const char *classText;
		switch( cent->currentState.NPC_class )
		{// UQ1: Supported Class Types...
		case CLASS_CIVILIAN:
			classText = "Civilian";
			VectorSet4(textColor, 0.125f, 0.125f, 0.7f, 1.0f);
			break;
		case CLASS_REBEL:
			classText = "Rebel";
			VectorSet4(textColor, 0.125f, 0.125f, 0.7f, 1.0f);
			break;
		case CLASS_JEDI:
		case CLASS_KYLE:
		case CLASS_LUKE:
		case CLASS_JAN:
		case CLASS_MONMOTHA:
		case CLASS_MORGANKATARN:
			classText = "Jedi";
			VectorSet4(textColor, 0.125f, 0.325f, 0.7f, 1.0f);
			break;
		case CLASS_GENERAL_VENDOR:
			classText = "General Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_WEAPONS_VENDOR:
			classText = "Weapons Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_ARMOR_VENDOR:
			classText = "Armor Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_SUPPLIES_VENDOR:
			classText = "Supplies Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_EQUIPMENT_VENDOR:
			classText = "Equipment Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_FOOD_VENDOR:
			classText = "Food Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_MEDICAL_VENDOR:
			classText = "Medical Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_GAMBLER_VENDOR:
			classText = "Gambling Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_TRADE_VENDOR:
			classText = "Trade Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_ODDITIES_VENDOR:
			classText = "Oddities Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_DRUG_VENDOR:
			classText = "Drug Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;
		case CLASS_TRAVELLING_VENDOR:
			classText = "Travelling Vendor";
			VectorSet4(textColor, 0.525f, 0.525f, 1.0f, 1.0f);
			break;

		//Stoiss add: Faq coler names for npcs in this class
		case CLASS_JKG_FAQ_IMP_DROID:
			classText = "Imperial FAQ Droid";//red coler
			VectorSet4(textColor, 1.0f, 0.125f, 0.125f, 1.0f);
			break;

		case CLASS_JKG_FAQ_ALLIANCE_DROID:
			classText = "Alliance FAQ Droid";//Dark Blue coler
			VectorSet4(textColor, 0.125f, 0.125f, 0.7f, 1.0f);
			break;

		case CLASS_JKG_FAQ_SPY_DROID:
			classText = "Faq Spy Droid";//Yellow colder
			VectorSet4(textColor, 0.7f, 0.7f, 0.125f, 1.0f);
			break;
		case CLASS_JKG_FAQ_CRAFTER_DROID:
			classText = "Master Crafter";//Yellow colder
			VectorSet4(textColor, 0.7f, 0.7f, 0.125f, 1.0f);
			break;
		case CLASS_JKG_FAQ_MERC_DROID:
			classText = "Merc FAQ Droid";//Yellow colder. fixme: need a better coler for mercs
			VectorSet4(textColor, 1.0f, 0.225f, 0.125f, 1.0f);
			break;
		case CLASS_JKG_FAQ_JEDI_MENTOR:
			classText = "Jedi Mentor";// Blue coler
			VectorSet4(textColor, 0.125f, 0.325f, 0.7f, 1.0f);
			break;
		case CLASS_JKG_FAQ_SITH_MENTOR:
			classText = "Sith Mentor";//oriange coler
			VectorSet4(textColor, 1.0f, 0.225f, 0.125f, 1.0f);
			break;
		case CLASS_STORMTROOPER:
		case CLASS_SWAMPTROOPER:
		case CLASS_IMPWORKER:
		case CLASS_IMPERIAL:
		case CLASS_SHADOWTROOPER:
		case CLASS_COMMANDO:
			classText = "Imperial";
			VectorSet4(textColor, 1.0f, 0.125f, 0.125f, 1.0f);
			break;
		case CLASS_MERC://Stoiss add merc class
			classText = "Merc";
			VectorSet4(textColor, 1.0f, 0.125f, 0.125f, 1.0f);
			break;
		case CLASS_TAVION:
		case CLASS_DESANN:
			classText = "Sith Boss";
			VectorSet4(textColor, 1.0f, 0.325f, 0.125f, 1.0f);
			break;
		case CLASS_REBORN:
			classText = "Sith";
			VectorSet4(textColor, 1.0f, 0.325f, 0.125f, 1.0f);
			break;
		case CLASS_REBORN_CULTIST:
			classText = "Sith Fighters";
			VectorSet4(textColor, 1.0f, 0.325f, 0.125f, 1.0f);
		case CLASS_BOBAFETT:
			classText = "Bounty Hunter";
			VectorSet4(textColor, 1.0f, 0.225f, 0.125f, 1.0f);
			break;
		case CLASS_ATST:
			classText = "Vehicle";
			VectorSet4(textColor, 1.0f, 0.225f, 0.125f, 1.0f);
			break;
		case CLASS_CLAW:
		case CLASS_FISH:
		case CLASS_FLIER2:
		case CLASS_GLIDER:
		case CLASS_HOWLER:
		case CLASS_LIZARD:
		case CLASS_MINEMONSTER:
		case CLASS_SWAMP:
		case CLASS_RANCOR:
		case CLASS_WAMPA:
		case CLASS_TUSKEN:
			classText = "Animal";
			VectorSet4(textColor, 1.0f, 0.125f, 0.125f, 1.0f);
			break;
		case CLASS_VEHICLE:
			classText = "Vehicle";
			VectorSet4(textColor, 1.0f, 0.125f, 0.125f, 1.0f);
			break;
		case CLASS_BESPIN_COP:
		case CLASS_LANDO:
		case CLASS_PRISONER:
			classText = "Rebel";
			VectorSet4(textColor, 0.125f, 0.125f, 0.7f, 1.0f);
			break;
		case CLASS_GALAK:
		case CLASS_GRAN:
		case CLASS_REELO:
		case CLASS_MURJJ:
		case CLASS_RODIAN:
		case CLASS_TRANDOSHAN:
		case CLASS_UGNAUGHT:
		case CLASS_WEEQUAY:
		case CLASS_BARTENDER:
		case CLASS_JAWA:
			if (cent->playerState->persistant[PERS_TEAM] == NPCTEAM_ENEMY)
			{
				classText = "Thug";
				VectorSet4(textColor, 0.5f, 0.5f, 0.125f, 1.0f);
			}
			else if (cent->playerState->persistant[PERS_TEAM] == NPCTEAM_PLAYER)
			{
				classText = "Rebel";
				VectorSet4(textColor, 0.125f, 0.125f, 0.7f, 1.0f);
			}
			else
			{
				classText = "Civilian";
				VectorSet4(textColor, 0.7f, 0.7f, 0.125f, 1.0f);
			}
			break;
		
		default:
			continue; // Unsupported...
		}

		if (cent->currentState.generic1 > 0)
		{
			// Was assigned a full name already! Yay!
			switch( cent->currentState.NPC_class )
			{// UQ1: Supported Class Types...
			case CLASS_CIVILIAN:
			case CLASS_GENERAL_VENDOR:
			case CLASS_WEAPONS_VENDOR:
			case CLASS_ARMOR_VENDOR:
			case CLASS_SUPPLIES_VENDOR:
			case CLASS_EQUIPMENT_VENDOR:
			case CLASS_FOOD_VENDOR:
			case CLASS_MEDICAL_VENDOR:
			case CLASS_GAMBLER_VENDOR:
			case CLASS_TRADE_VENDOR:
			case CLASS_ODDITIES_VENDOR:
			case CLASS_DRUG_VENDOR:
			case CLASS_TRAVELLING_VENDOR:
			case CLASS_JKG_FAQ_IMP_DROID:
			case CLASS_JKG_FAQ_ALLIANCE_DROID:
			case CLASS_JKG_FAQ_SPY_DROID:
			case CLASS_JKG_FAQ_CRAFTER_DROID:
			case CLASS_JKG_FAQ_MERC_DROID:
			case CLASS_JKG_FAQ_JEDI_MENTOR:
			case CLASS_JKG_FAQ_SITH_MENTOR:
			case CLASS_LUKE:
			case CLASS_JEDI:
			case CLASS_KYLE:
			case CLASS_JAN:
			case CLASS_MONMOTHA:
			case CLASS_MORGANKATARN:
			case CLASS_TAVION:
			case CLASS_REBORN:
			case CLASS_REBORN_CULTIST:
			case CLASS_DESANN:
			case CLASS_BOBAFETT:
			case CLASS_COMMANDO:
			case CLASS_WEEQUAY:
			case CLASS_BARTENDER:
			case CLASS_BESPIN_COP:
			case CLASS_GALAK:
			case CLASS_GRAN:
			case CLASS_MERC://Stoiss add merc class
			case CLASS_LANDO:			
			case CLASS_REBEL:
			case CLASS_REELO:
			case CLASS_MURJJ:
			case CLASS_PRISONER:
			case CLASS_RODIAN:
			case CLASS_TRANDOSHAN:
			case CLASS_TUSKEN:
			case CLASS_UGNAUGHT:
			case CLASS_JAWA:
				npcName = va("%s", BG_Get_NPC_Name(cent->currentState.generic1));
				break;
			case CLASS_STORMTROOPER:
			case CLASS_SWAMPTROOPER:
			case CLASS_IMPWORKER:
			case CLASS_IMPERIAL:
			case CLASS_SHADOWTROOPER:
				npcName = va("TK-%i", cent->currentState.generic1);	// EVIL. for a number of reasons --eez
				break;
			case CLASS_ATST:				// technically droid...
				npcName = "AT-ST";
				break;
			case CLASS_CLAW:
				npcName = "Claw";
				break;
			case CLASS_FISH:
				npcName = "Sea Creature";
				break;
			case CLASS_FLIER2:
				npcName = "Flier";
				break;
			case CLASS_GLIDER:
				npcName = "Glider";
				break;
			case CLASS_HOWLER:
				npcName = "Howler";
				break;
			case CLASS_LIZARD:
				npcName = "Lizard";
				break;
			case CLASS_MINEMONSTER:
				npcName = "Mine Monster";
				break;
			case CLASS_SWAMP:
				npcName = "Swamp Monster";
				break;
			case CLASS_RANCOR:
				npcName = "Rancor";
				break;
			case CLASS_WAMPA:
				npcName = "Wampa";
				break;
			case CLASS_VEHICLE:
				npcName = "";
				break;
			default:
				continue; // Unsupported...
			}
		}
		else
		{
			continue;
		}

		vec3_t origin;
		VectorCopy( cent->lerpOrigin, origin );
		origin[2] += 30;

		if ( cent->playerState->pm_flags & PMF_DUCKED )
			origin[2] -= 18;
	
		// Draw the NPC name!
		float x;
		float y;
		if (!CG_WorldCoordToScreenCoordFloat(origin, &x, &y))
			continue;

		if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT)
			continue;

		VectorCopy( cent->lerpOrigin, origin );
		origin[2] += 25;

		if ( cent->playerState->pm_flags & PMF_DUCKED )
			origin[2] -= 18;

		float multiplier = 1.0f;
		if (distance < 192.0f)
			multiplier = 200.0f /distance;

		float x2;
		float y2;
		if (!CG_WorldCoordToScreenCoordFloat(origin, &x2, &y2))
			continue;

		if (x2 < 0 || x2 > SCREEN_WIDTH || y2 < 0 || y2 > SCREEN_HEIGHT)
			continue;

		float size = Com_Clamp(distance * 0.0002, 0.01f, 0.99f);
		size = 0.3f * (1.0f - size);

		const float nameTextSize = size * 2.0f;
		const float classTextSize = size * 1.5f;

		char bracketedName[64];
		Com_sprintf(bracketedName, sizeof(bracketedName), "< %s >", npcName);

		const float h = CG_Text_Height(bracketedName, nameTextSize, FONT_SMALL);
		y += h;
		y = (y*(1-size))+((30*(1-size))*(1-size))+sqrt(sqrt((1-size)*30))+((1-multiplier)*30);
		x -= 0.5f * CG_Text_Width(bracketedName, nameTextSize, FONT_SMALL);
		
		x2 -= 0.5f * CG_Text_Width(classText, classTextSize, FONT_SMALL3);
		y2 = y + h + 6.0f;

		CG_Text_Paint(x, y, nameTextSize, textColor, bracketedName, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL);
		CG_Text_Paint(x2, y2, classTextSize, textColor, classText, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL3);
	}
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float		*color;
	vec4_t		tcolor;
	char		*name;
	int			baseColor;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	//rww - still do the trace, our dynamic crosshair depends on it

	// TESTING EXTRASTATE
	/*if ( cg_entities[cg.crosshairClientNum].extraState.testInt > 100 )
	{
		UI_DrawProportionalString (320, 170, "HERE BE WEREWOLVES", UI_CENTER, colorTable[CT_WHITE]);
		return;
	}*/
    
    //[USE_ITEMS]
    if ( cg_entities[cg.crosshairClientNum].currentState.eType == ET_ITEM )
    {
        //UI_DrawProportionalString (320, 170, "Press E to pick up weapon", UI_CENTER, colorTable[CT_WHITE], FONT_MEDIUM);
        return;
    }
    //[/USE_ITEMS]

	if (cg_entities[cg.crosshairClientNum].currentState.eType == ET_BODY)
	{
		UI_DrawProportionalString(320, 170, "Press E to loot body", UI_CENTER, colorTable[CT_WHITE], FONT_MEDIUM);
		return;
	}

	if (cg.crosshairClientNum >= MAX_CLIENTS)
	{
		return;
	}

	if (cg_entities[cg.crosshairClientNum].currentState.powerups & (1 << PW_CLOAKED))
	{
		return;
	}

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) {
		trap->R_SetColor( NULL );
		return;
	}

	name = cgs.clientinfo[ cg.crosshairClientNum ].cleanname;

	if (cgs.gametype >= GT_TEAM)
	{
		if (1)
		{ //instead of team-based we'll make it oriented based on which team we're on
			if (cgs.clientinfo[cg.crosshairClientNum].team == cg.predictedPlayerState.persistant[PERS_TEAM])
			{
				baseColor = CT_GREEN;
			}
			else
			{
				baseColor = CT_RED;
			}
		}
		else
		{
			if (cgs.clientinfo[cg.crosshairClientNum].team == TEAM_RED)
			{
				baseColor = CT_RED;
			}
			else
			{
				baseColor = CT_BLUE;
			}
		}
	}
	else
	{
		if (cgs.gametype == GT_POWERDUEL &&
			cgs.clientinfo[cg.snap->ps.clientNum].team != TEAM_SPECTATOR &&
			cgs.clientinfo[cg.crosshairClientNum].duelTeam == cgs.clientinfo[cg.predictedPlayerState.clientNum].duelTeam)
		{ //on the same duel team in powerduel, so he's a friend
			baseColor = CT_GREEN;
		}
		else
		{
			baseColor = CT_RED; //just make it red in nonteam modes since everyone is hostile and crosshair will be red on them too
		}
	}

	if (cg.snap->ps.duelInProgress)
	{
		if (cg.crosshairClientNum != cg.snap->ps.duelIndex)
		{ //grey out crosshair for everyone but your foe if you're in a duel
			baseColor = CT_BLACK;
		}
	}
	else if (cg_entities[cg.crosshairClientNum].currentState.bolt1)
	{ //this fellow is in a duel. We just checked if we were in a duel above, so
	  //this means we aren't and he is. Which of course means our crosshair greys out over him.
		baseColor = CT_BLACK;
	}

	tcolor[0] = colorTable[baseColor][0];
	tcolor[1] = colorTable[baseColor][1];
	tcolor[2] = colorTable[baseColor][2];
	tcolor[3] = color[3]*0.5f;

	UI_DrawProportionalString(320, 170, name, UI_CENTER, tcolor, FONT_MEDIUM);

	trap->R_SetColor( NULL );
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) 
{	
	const char* s;

	s = CG_GetStringEdString("MP_INGAME", "SPECTATOR");
	CG_Text_Paint ( 320 - CG_Text_Width ( s, 0.5f, 5 ) / 2, 438, 0.5f, colorWhite, s, 0, 0, 0, 5 );

	s = CG_GetStringEdString("MP_INGAME", "SPEC_CHOOSEJOIN");
	CG_Text_Paint ( 320 - CG_Text_Width ( s, 0.5f, 5 ) / 2, 450, 0.5f, colorWhite, s, 0, 0, 0, 5 );
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
	const char *s = NULL, *sParm = NULL;
	int sec;
	char sYes[20] = {0}, sNo[20] = {0}, sVote[20] = {0}, sCmd[100] = {0};

	if ( !cgs.voteTime )
		return;

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}

	if (strncmp(cgs.voteString, "map_restart", 11)==0)
	{
		trap->SE_GetStringTextString("MENUS_RESTART_MAP", sCmd, sizeof(sCmd) );
	}
	else if (strncmp(cgs.voteString, "vstr nextmap", 12)==0)
	{
		trap->SE_GetStringTextString("MENUS_NEXT_MAP", sCmd, sizeof(sCmd) );
	}
	else if (strncmp(cgs.voteString, "g_gametype", 10)==0)
	{
		trap->SE_GetStringTextString("MENUS_GAME_TYPE", sCmd, sizeof(sCmd) );
		if ( Q_stricmp("Free For All", cgs.voteString+11)==0 ) 
		{
			sParm = CG_GetStringEdString("MENUS", "FREE_FOR_ALL");
		}
		else if ( Q_stricmp("Warzone", cgs.voteString+11)==0 ) 
		{
			sParm = CG_GetStringEdString("MENUS", "WARZONE");
		}
		else if ( Q_stricmp("Duel", cgs.voteString+11)==0 ) 
		{
			sParm = CG_GetStringEdString("MENUS", "DUEL");
		}
		else if ( Q_stricmp("Holocron FFA", cgs.voteString+11)==0  ) 
		{
			sParm = CG_GetStringEdString("MENUS", "HOLOCRON_FFA");
		}
		else if ( Q_stricmp("Power Duel", cgs.voteString+11)==0  ) 
		{
			sParm = CG_GetStringEdString("MENUS", "POWERDUEL");
		}
		else if ( Q_stricmp("Team FFA", cgs.voteString+11)==0  ) 
		{
			sParm = CG_GetStringEdString("MENUS", "TEAM_FFA");
		}
		else if ( Q_stricmp("Capture the Flag", cgs.voteString+11)==0  ) 
		{
			sParm = CG_GetStringEdString("MENUS", "CAPTURE_THE_FLAG");
		}
	}
	else if (strncmp(cgs.voteString, "map", 3)==0)
	{
		trap->SE_GetStringTextString("MENUS_NEW_MAP", sCmd, sizeof(sCmd) );
		sParm = cgs.voteString+4;
	}
	else if (strncmp(cgs.voteString, "kick", 4)==0)
	{
		trap->SE_GetStringTextString("MENUS_KICK_PLAYER", sCmd, sizeof(sCmd) );
		sParm = cgs.voteString+5;
	}
	else
	{//Raz: Added an else case for custom votes like ampoll, cointoss, etc
		sParm = cgs.voteString;
	}



	trap->SE_GetStringTextString("MENUS_VOTE", sVote, sizeof(sVote) );
	trap->SE_GetStringTextString("MENUS_YES", sYes, sizeof(sYes) );
	trap->SE_GetStringTextString("MENUS_NO",  sNo,  sizeof(sNo) );

	if (sParm && sParm[0])
	{
		s = va("%s(%i):<%s %s> %s:%i %s:%i", sVote, sec, sCmd, sParm, sYes, cgs.voteYes, sNo, cgs.voteNo);
	}
	else
	{
		s = va("%s(%i):<%s> %s:%i %s:%i",    sVote, sec, sCmd,        sYes, cgs.voteYes, sNo, cgs.voteNo);
	}
	CG_DrawSmallString( 4, 58, s, 1.0F );
	s = CG_GetStringEdString("MP_INGAME", "OR_PRESS_ESC_THEN_CLICK_VOTE");	//	s = "or press ESC then click Vote";
	CG_DrawSmallString( 4, 58 + SMALLCHAR_HEIGHT + 2, s, 1.0F );
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void) {
	char	*s;
	int		sec, cs_offset;

	if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED )
		cs_offset = 0;
	else if ( cgs.clientinfo[cg.clientNum].team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !cgs.teamVoteTime[cs_offset] ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.teamVoteModified[cs_offset] ) {
		cgs.teamVoteModified[cs_offset] = qfalse;
//		trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[cs_offset] ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	if (strstr(cgs.teamVoteString[cs_offset], "leader"))
	{
		int i = 0;

		while (cgs.teamVoteString[cs_offset][i] && cgs.teamVoteString[cs_offset][i] != ' ')
		{
			i++;
		}

		if (cgs.teamVoteString[cs_offset][i] == ' ')
		{
			int voteIndex = 0;
			char voteIndexStr[256];

			i++;

			while (cgs.teamVoteString[cs_offset][i])
			{
				voteIndexStr[voteIndex] = cgs.teamVoteString[cs_offset][i];
				voteIndex++;
				i++;
			}
			voteIndexStr[voteIndex] = 0;

			voteIndex = atoi(voteIndexStr);

			s = va("TEAMVOTE(%i):(Make %s the new team leader) yes:%i no:%i", sec, cgs.clientinfo[voteIndex].name,
									cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
		}
		else
		{
			s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
									cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
		}
	}
	else
	{
		s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
								cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
	}
	CG_DrawSmallString( 4, 90, s, 1.0F );
}
/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawOldScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) 
{
	const char	*s;

	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) 
	{
		return qfalse;
	}

	if (cgs.gametype == GT_POWERDUEL)
	{
		clientInfo_t *ci = &cgs.clientinfo[ cg.snap->ps.clientNum ];

		if (ci->duelTeam == DUELTEAM_LONE)
		{
			s = CG_GetStringEdString("MP_INGAME", "FOLLOWINGLONE");
		}
		else if (ci->duelTeam == DUELTEAM_DOUBLE)
		{
			s = CG_GetStringEdString("MP_INGAME", "FOLLOWINGDOUBLE");
		}
		else
		{
			s = CG_GetStringEdString("MP_INGAME", "FOLLOWING");
		}
	}
	else
	{
		s = CG_GetStringEdString("MP_INGAME", "FOLLOWING");
	}

	CG_Text_Paint(320 - CG_Text_Width(s, 1.0f, FONT_MEDIUM) / 2, 60, 1.0f, colorWhite, s, 0, 0, 0, FONT_MEDIUM);
	
	s = cgs.clientinfo[ cg.snap->ps.clientNum ].name;
	CG_Text_Paint(320 - CG_Text_Width(s, 2.0f, FONT_MEDIUM) / 2, 80, 2.0f, colorWhite, s, 0, 0, 0, FONT_MEDIUM);

	return qtrue;
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			w;
	int			sec;
	int			i;
	float		scale, cw;
	const char	*s;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
//		s = "Waiting for players";		
		s = CG_GetStringEdString("MP_INGAME", "WAITING_FOR_PLAYERS");
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_DUEL || cgs.gametype == GT_POWERDUEL)
	{
		// find the two active players
		clientInfo_t	*ci1, *ci2, *ci3;

		ci1 = NULL;
		ci2 = NULL;
		ci3 = NULL;

		if (cgs.gametype == GT_POWERDUEL)
		{
			if (cgs.duelist1 != -1)
			{
				ci1 = &cgs.clientinfo[cgs.duelist1];
			}
			if (cgs.duelist2 != -1)
			{
				ci2 = &cgs.clientinfo[cgs.duelist2];
			}
			if (cgs.duelist3 != -1)
			{
				ci3 = &cgs.clientinfo[cgs.duelist3];
			}
		}
		else
		{
			for ( i = 0 ; i < cgs.maxclients ; i++ ) {
				if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
					if ( !ci1 ) {
						ci1 = &cgs.clientinfo[i];
					} else {
						ci2 = &cgs.clientinfo[i];
					}
				}
			}
		}
		if ( ci1 && ci2 )
		{
			if (ci3)
			{
				s = va( "%s vs %s and %s", ci1->name, ci2->name, ci3->name );
			}
			else
			{
				s = va( "%s vs %s", ci1->name, ci2->name );
			}
			w = CG_Text_Width(s, 0.6f, FONT_MEDIUM);
			CG_Text_Paint(320 - w / 2, 60, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE,FONT_MEDIUM);
		}
	} else {
		if ( cgs.gametype == GT_FFA ) {
			s = CG_GetStringEdString("MENUS", "FREE_FOR_ALL");//"Free For All";
		} else if ( cgs.gametype == GT_WARZONE ) {
			s = CG_GetStringEdString("MENUS", "WARZONE");//"Warzone";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = CG_GetStringEdString("MENUS", "TEAM_FFA");//"Team FFA";
		} else if ( cgs.gametype == GT_CTF ) {
			s = CG_GetStringEdString("MENUS", "CAPTURE_THE_FLAG");//"Capture the Flag";
		}
#ifdef __JKG_NINELIVES__
else if ( cgs.gametype == GT_LMS_NINELIVES ) {
			s = CG_GetStringEdString("MENUS", "NINELIVES_LMS");
}
#endif
#ifdef __JKG_TICKETING__
else if ( cgs.gametype == GT_LMS_TICKETED ) {
			s = CG_GetStringEdString("MENUS", "TICKETED_LMS");
}
#endif
#ifdef __JKG_ROUNDBASED__
else if ( cgs.gametype == GT_LMS_ROUNDS ) {
			s = CG_GetStringEdString("MENUS", "ROUNDBASED_LMS");
}
#endif
		else {
			s = "";
		}
		w = CG_Text_Width(s, 1.5f, FONT_MEDIUM);
		CG_Text_Paint(320 - w / 2, 90, 1.5f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE,FONT_MEDIUM);
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
//	s = va( "Starts in: %i", sec + 1 );
	s = va( "%s: %i",CG_GetStringEdString("MP_INGAME", "STARTS_IN"), sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;

		switch ( sec ) {
			case 0:
				trap->S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
				break;
			case 1:
				trap->S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
				break;
			case 2:
				trap->S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
				break;
			default:
				break;
		}
	}
	scale = 0.45f;
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		scale = 1.25f;
		break;
	case 1:
		cw = 24;
		scale = 1.15f;
		break;
	case 2:
		cw = 20;
		scale = 1.05f;
		break;
	default:
		cw = 16;
		scale = 0.9f;
		break;
	}

	w = CG_Text_Width(s, scale, FONT_MEDIUM);
	CG_Text_Paint(320 - w / 2, 125, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE, FONT_MEDIUM);
}

//==================================================================================
/* 
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus() {
	if (cg.voiceTime) {
		int t = cg.time - cg.voiceTime;
		if ( t > 2500 ) {
			Menus_CloseByName("voiceMenu");
			trap->Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}

void CG_DrawFlagStatus()
{
	int myFlagTakenShader = 0;
	int theirFlagShader = 0;
	int team = 0;
	int startDrawPos = 2;
	int ico_size = 32;

	//Raz: was missing this
	trap->R_SetColor( NULL );

	if (!cg.snap)
	{
		return;
	}

	if (cgs.gametype != GT_CTF)
	{
		return;
	}

	team = cg.snap->ps.persistant[PERS_TEAM];

	if (team == TEAM_RED)
	{
		myFlagTakenShader = trap->R_RegisterShaderNoMip( "gfx/hud/mpi_rflag_x" );
		theirFlagShader = trap->R_RegisterShaderNoMip( "gfx/hud/mpi_bflag" );
	}
	else
	{
		myFlagTakenShader = trap->R_RegisterShaderNoMip( "gfx/hud/mpi_bflag_x" );
		theirFlagShader = trap->R_RegisterShaderNoMip( "gfx/hud/mpi_rflag" );
	}

	if (CG_YourTeamHasFlag())
	{
		//CG_DrawPic( startDrawPos, 330, ico_size, ico_size, theirFlagShader );
		CG_DrawPic( 2, 330-startDrawPos, ico_size, ico_size, theirFlagShader );
		startDrawPos += ico_size+2;
	}

	if (CG_OtherTeamHasFlag())
	{
		//CG_DrawPic( startDrawPos, 330, ico_size, ico_size, myFlagTakenShader );
		CG_DrawPic( 2, 330-startDrawPos, ico_size, ico_size, myFlagTakenShader );
	}
}




void CG_DrawJetpackCloak(menuDef_t *menuHUD) {

	itemDef_t	*focusItem;
	float		percent = 100.0f;
	qhandle_t	pic = NULL_HANDLE;
	vec4_t opacity;
	int jetpack = cg.snap->ps.jetpack;
	int jetpackFuel = cg.snap->ps.jetpackFuel;
	
	MAKERGBA( opacity, 1, 1, 1, 1*cg.jkg_HUDOpacity );

	// FIXME: what happens if using more than one?
	if (cg.snap->ps.weapon == WP_SABER)
	{
		percent = cg.predictedPlayerState.blockPoints;
		pic = trap->R_RegisterShader("gfx/jkghud/ico_cloak.png");
	}

	if (jetpack != 0 && jetpackFuel < jetpackTable[jetpack -1].fuelCapacity)
	{ // Jetpack is being used or is recharging
		if (cg.snap->ps.cloakFuel >= 100 || (cg.time >> 10 & 1)) {
			percent = jetpackFuel / (float)jetpackTable[jetpack - 1].fuelCapacity * 100.0f;
			pic = trap->R_RegisterShader("gfx/jkghud/ico_jetpack.png");
		}
	}
	if (cg.snap->ps.cloakFuel < 100) {
		if (jetpackFuel >= jetpackTable[jetpack - 1].fuelCapacity || !(cg.time >> 10 & 1)) {
			percent = cg.snap->ps.cloakFuel;
			pic = trap->R_RegisterShader("gfx/jkghud/ico_cloak.png");
		}
	}

	if (percent >= 100 && cg.snap->ps.weapon != WP_SABER) {
		return;
	}

	percent /= 100.0f;
	//percent *= 0.75f;

	focusItem = Menu_FindItemByName(menuHUD, "bar1");
	if (focusItem)
	{	// The bar might not be in the HUD. But it should be located..I think in the bottom right hand corner.
		// It's a red bar, I think. It's the same as the one used for the jetpack fuel, I believe. it is green
		trap->R_SetColor( opacity );
		trap->R_DrawStretchPic(
						focusItem->window.rect.x,
						focusItem->window.rect.y,
						focusItem->window.rect.w * percent,
						focusItem->window.rect.h,
						0, 0, percent, 1,
						focusItem->window.background
						);
	}

	// Draw icon
	focusItem = Menu_FindItemByName(menuHUD, "bar1ico");
	if (focusItem)
	{
		trap->R_SetColor( opacity );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			pic 
			);			
	}
}

//draw meter showing e-web health when it is in use
#define EWEBHEALTH_H			100.0f
#define EWEBHEALTH_W			20.0f
#define EWEBHEALTH_X			(SCREEN_WIDTH-EWEBHEALTH_W-8.0f)
#define EWEBHEALTH_Y			290.0f
void CG_DrawEWebHealth(void)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = EWEBHEALTH_X;
	float y = EWEBHEALTH_Y;
	centity_t *eweb = &cg_entities[cg.predictedPlayerState.emplacedIndex];
	float percent = ((float)eweb->currentState.health/eweb->currentState.maxhealth)*EWEBHEALTH_H;

	if (percent > EWEBHEALTH_H)
	{
		return;
	}

	if (percent < 0.1f)
	{
		percent = 0.1f;
	}

	//color of the bar
	aColor[0] = 0.5f;
	aColor[1] = 0.0f;
	aColor[2] = 0.0f;
	aColor[3] = 0.8f;

	//color of the border
	bColor[0] = 0.0f;
	bColor[1] = 0.0f;
	bColor[2] = 0.0f;
	bColor[3] = 0.3f;

	//color of greyed out "missing fuel"
	cColor[0] = 0.5f;
	cColor[1] = 0.5f;
	cColor[2] = 0.5f;
	cColor[3] = 0.1f;

	//draw the background (black)
	CG_DrawRect(x, y, EWEBHEALTH_W, EWEBHEALTH_H, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much health there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f+(EWEBHEALTH_H-percent), EWEBHEALTH_W-1.0f, EWEBHEALTH_H-1.0f-(EWEBHEALTH_H-percent), aColor);

	//then draw the other part greyed out
	CG_FillRect(x+1.0f, y+1.0f, EWEBHEALTH_W-1.0f, EWEBHEALTH_H-percent, cColor);
}

qboolean gCGHasFallVector = qfalse;
vec3_t gCGFallVector;

/*
=================
CG_Draw2D
=================
*/

/*====================================
chatbox functionality -rww

This here is gettin some hefty modification here to work in JKG
====================================*/
#define	CHATBOX_CUTOFF_LEN	216 //550
#define	CHATBOX_FONT_HEIGHT	8 //20

//utility func, insert a string into a string at the specified
//place (assuming this will not overflow the buffer)
void CG_ChatBox_StrInsert(char *buffer, int bufferSize, int place, char *str)
{
	int insLen = strlen(str);
	int i = strlen(buffer);

	memmove (buffer + place + insLen, buffer + place, i - place);
	memcpy (buffer + place, str, insLen);
}

// Escape % and ", so they can be sent along properly
static const char *ChatBox_UnescapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = (char *)message;
	while (*t && s != cutoff) {
		if (*t == 0x18) {
			*s = '%';
		} else if (*t == 0x17) {
			*s = '"';
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}

const char *CG_ChatBox_TimeStamp()
{
	static char buffer[16];
	time_t tm;
	struct tm * timeinfo;
	
	time( &tm );
	timeinfo = localtime ( &tm );

	Com_sprintf(buffer, 16, "[%02i:%02i]", timeinfo->tm_hour, timeinfo->tm_min);
	return buffer;
}

//add chatbox string - Modified for JKG
void CG_ChatBox_AddString(char *chatStr, int fadeLevel)
{
	int i;
	int linecount;
	chatBoxItem_t *chat = &cg.chatItems[cg.chatItemNext];
	float chatLen;

	if (cg_chatBox.integer<=0)
	{ //don't bother then.
		return;
	}

	memset(chat, 0, sizeof(chatBoxItem_t));

	Q_strncpyz (chat->string, va("^7%s %s", CG_ChatBox_TimeStamp()  ,ChatBox_UnescapeChat(chatStr)), sizeof (chat->string));
	//chat->time = cg.time + cg_chatBox.integer;
	chat->active = 1;
	chat->lines = 1;

	chat->alpha = (float)fadeLevel / 100.0f;

	// Trick: instead of usin the right scale, get the length at scale 1
	// then scale down with a float, so we dont get rounding errors
	//chatLen = trap->R_Font_StrLenPixels(chat->string, cgDC.Assets.qhSmall4Font, 1); //CG_Text_Width(chat->string, 1.0f, FONT_SMALL);
	//chatLen *= 0.5f;
	chatLen = strlen(chat->string)*4;

	if (chatLen > CHATBOX_CUTOFF_LEN)
	{ //we have to break it into segments...
        int i = 0;
		int lastLinePt = 0;
		char buffer[200] = { 0 };
		char *writeptr = buffer;

		chatLen = 0;
		while (chat->string[i])
		{
			int colorLen = Q_parseColorString(&(chat->string[i]), nullptr);
			if (colorLen) {
				i+=colorLen;
				continue;
			}
			*writeptr = chat->string[i];
			//chatLen = ((float)trap->R_Font_StrLenPixels(buffer, cgDC.Assets.qhSmall4Font, 1) * 0.5f); //CG_Text_Width(s, 0.65f, FONT_SMALL);
			//chatLen += trap->R_Font_StrLenPixels(s, cgDC.Assets.qhSmallFont, 0.4f); //CG_Text_Width(s, 0.65f, FONT_SMALL);
			//The above is pretty derpy...
			chatLen = strlen(buffer)*4;
            writeptr++;
			if (chatLen >= CHATBOX_CUTOFF_LEN )
			{
				int j = i;
				while (j > 0 && j > lastLinePt)
				{
					if (chat->string[j] == ' ')
					{
						break;
					}
					j--;
				}
				
				chat->lines++;
				
				if (chat->string[j] == ' ')
				{
					i = j;
					chat->string[j] = '\n';
				}
                else
                {
                    CG_ChatBox_StrInsert(chat->string, sizeof (chat->string), i, "\n");
                }
                
				i++;
				chatLen = 0;
				lastLinePt = i+1;
				memset (buffer, 0, sizeof (buffer));
				writeptr = buffer;
			}
			
			i++;
		}
	}

	cg.chatItemNext++;
	if (cg.chatItemNext >= MAX_CHATBOX_ITEMS)
	{
		cg.chatItemNext = 0;
	}
	// Check if we exceeded the maximum line count
	linecount = 0;
	for (i=0; i<MAX_CHATBOX_ITEMS; i++) {
		int idx = (cg.chatItemNext + i) % MAX_CHATBOX_ITEMS;
		if (!cg.chatItems[idx].active) {
			continue;
		}
		linecount += cg.chatItems[idx].lines;
	}
	if (linecount > MAX_CHATBOX_ITEMS) {
		// We exceeded our limit, start removing old entries until we're under 12 again
		for (i=0; i<MAX_CHATBOX_ITEMS; i++) {
			int idx = (cg.chatItemNext + i) % MAX_CHATBOX_ITEMS;
			if (cg.chatItems[idx].active) {
				linecount -= cg.chatItems[idx].lines;
				cg.chatItems[idx].active = 0;
				if (linecount <= MAX_CHATBOX_ITEMS) {
					break;
				}
			}
		}
	}
}

//===============================================================
//
// NOTIFICATION SYSTEM
//
// In the future, this will have two kinds of notifications:
// - Push notification
// This functions like a Windows message box of sorts
// - Message notifications
// This just adds messages to a box, like chat
//
// VERSUS ONLY: For right now, we only have Message type notifications
//===============================================================


void CG_Notifications_Add(char *string, qboolean weapon)
{
	char buffer[MAX_NOTIFICATION_CHARS];
	notificationItem_t *notify = &cg.notificationBox[cg.notifyNext];
	int linecount, i;

	// Weapon notifications replace one that already exists
	if( weapon )
	{
		int whatWeGot = 0;
		for( i = 0; i < MAX_MESSAGE_NOTIFICATIONS; i++ )
		{
			if( cg.notificationBox[i].weaponNotification )
			{
				// Set it as this one, if it's showing
				if( cg.time < (cg.notificationBox[i].active+(MESSAGE_NOTIFICATION_TIME/2)+MESSAGE_NOTIFICATION_FADE_TIME) )
				{
					notify = &cg.notificationBox[i];
				}
			}
		}

		// String is stringed
		if(string[0] == '@')
		{
			whatWeGot = trap->SE_GetStringTextString( string+1, buffer, MAX_NOTIFICATION_CHARS );
		}
		else if(!Q_stricmpn(string, "??@", 3))
		{
			whatWeGot = trap->SE_GetStringTextString( string+3, buffer, MAX_NOTIFICATION_CHARS );
		}
		else
		{
			whatWeGot = trap->SE_GetStringTextString( string, buffer, MAX_NOTIFICATION_CHARS );
		}
		if(whatWeGot)
		{
			strcpy(string, buffer);
		}

		// Strip all newlines out of it and replace 'em with spaces
		for(i = 0; i < strlen(string) && string[i]; i++)
		{
			if(string[i] == '\n')
			{
				string[i] = ' ';
			}
		}
	}

	memset(notify, 0, sizeof(notificationItem_t));

	strcpy(notify->string, string);

	notify->active = cg.time;
	notify->weaponNotification = weapon;
	notify->lines = 1;

	notify->alpha = 1.0f;

	cg.notifyNext++; 
	if (cg.notifyNext >= MAX_MESSAGE_NOTIFICATIONS)
	{
		cg.notifyNext = 0;
	}

	// Check if we exceeded the maximum line count
	linecount = 0;
	for (i=0; i<MAX_MESSAGE_NOTIFICATIONS; i++) {
		int idx = (cg.notifyNext + i) % MAX_MESSAGE_NOTIFICATIONS;
		if (!cg.notificationBox[idx].active) {
			continue;
		}
		linecount += cg.notificationBox[idx].lines;
	}
	if (linecount > MAX_MESSAGE_NOTIFICATIONS) {
		// We exceeded our limit, start removing old entries until we're under 12 again
		for (i=0; i<MAX_MESSAGE_NOTIFICATIONS; i++) {
			int idx = (cg.notifyNext + i) % MAX_MESSAGE_NOTIFICATIONS;
			if (cg.notificationBox[idx].active) {
				linecount -= cg.notificationBox[idx].lines;
				cg.notificationBox[idx].active = 0;
				if (linecount <= MAX_MESSAGE_NOTIFICATIONS) {
					break;
				}
			}
		}
	}

}

void CG_RemoveMessageNotification(int num)
{
	// Drop this message from the stack, shifting everything else up
	int i;
	for ( i = num; i < MAX_MESSAGE_NOTIFICATIONS-1; i++ )
	{
		memcpy(&cg.notificationBox[i], &cg.notificationBox[i+1], sizeof(notificationItem_t));
	}
	cg.notificationBox[MAX_MESSAGE_NOTIFICATIONS-1].active = 0;
	cg.notificationBox[MAX_MESSAGE_NOTIFICATIONS-1].alpha = 0;
}

void CG_DrawMessageNotifications(void)
{
	menuDef_t *menuHUD;
	itemDef_t *item;
	int i;
	float x, y, w;
	float textHeight;
	vec4_t color;

	menuHUD = Menus_FindByName("hud_minimap");
	if (!menuHUD) {
		return;
	}

	item = Menu_FindItemByName(menuHUD, "notifications");
	if (!item)
	{
		return;
	}

	x = item->window.rect.x;
	y = item->window.rect.y;
	w = item->window.rect.w;
	textHeight = (trap->R_Font_HeightPixels(item->iMenuFont, 1.0f)*item->textscale);
	VectorCopy4(item->window.foreColor, color);


	for( i = 0; i < MAX_MESSAGE_NOTIFICATIONS; i++ )
	{
		int messageNotifyTime = cg.notificationBox[i].weaponNotification ? (MESSAGE_NOTIFICATION_TIME/2) : MESSAGE_NOTIFICATION_TIME;
		if(cg.time > cg.notificationBox[i].active+messageNotifyTime+MESSAGE_NOTIFICATION_FADE_TIME)
		{
			// Already faded out and dealt with
			CG_RemoveMessageNotification(i);
		}
		else if(cg.time > cg.notificationBox[i].active+messageNotifyTime)
		{
			// Fading out. Take care of that.
			cg.notificationBox[i].alpha = (float)(1-(((double)cg.time - ((double)cg.notificationBox[i].active+(double)messageNotifyTime))/(double)MESSAGE_NOTIFICATION_FADE_TIME));
		}
		// The notifications are right aligned, so do some fancy crap here
		color[3] = cg.notificationBox[i].alpha;
		if(color[3] > 0)
		{
			// Optimization --eez
			CG_Text_Paint(x + w - (trap->R_Font_StrLenPixels(cg.notificationBox[i].string, item->iMenuFont, 1.0f)*item->textscale) , y + (i*textHeight),
				item->textscale, color, cg.notificationBox[i].string, 0, -1, 0, item->iMenuFont);
		}
	}
}

float CG_GetLowHealthPhase(int reset, float multiplier) {
	static int lastCallTime = 0;
	static float phaseSmooth = 0;
	float target;
	float health;

	if (reset) {
		phaseSmooth = 0;
		lastCallTime = cg.time;
		return 0;
	}
	if (lastCallTime == cg.time) {
		return phaseSmooth;	// Only smooth once per frame, even if we get multiple calls
	}
	lastCallTime = cg.time;
	health = (float)cg.predictedPlayerState.stats[STAT_HEALTH] * multiplier;
	if (health >= 30) {
		target = 0;
	} else {
		target = (30 - health) / 30.0f;
	}
	// We got a target, check if we need to change our smoothened phase
	// Fade rate: 100% in 2 seconds

	if (phaseSmooth > target) {
		// Go down
		phaseSmooth -= (cg.frameDelta / 2000.0f);
		if (phaseSmooth < target) {
			phaseSmooth = target;
		}
	} else if (phaseSmooth < target) {
		// Go up
		phaseSmooth += (cg.frameDelta / 2000.0f);
		if (phaseSmooth > target) {
			phaseSmooth = target;
		}
	}
	return phaseSmooth;
}

//extern qboolean pm_isSprinting;
static void CG_Draw2DScreenTints( void )
{
	vec4_t			hcolor;
	
	if (cgs.clientinfo[cg.snap->ps.clientNum].team != TEAM_SPECTATOR)
	{
		float hpMultiplier = 100.0f / (float)cg.predictedPlayerState.stats[STAT_MAX_HEALTH];
		float lowHealthPhase = CG_GetLowHealthPhase(0, hpMultiplier);
		if ( ((float)cg.predictedPlayerState.stats[STAT_HEALTH] * hpMultiplier) < 30 ) {
			if (cg.deathTime && !cg.deathcamFadeStart) {	
				lowHealthPhase = 0;	// Dont glow red
				hcolor[3] = 0.2f;
				// Calculate the fadeout state
				if (cg.deathTime) { // failsafe
					hcolor[0] = hcolor[1] = hcolor[2] = 0; // Black
					if (cg.time - cg.deathTime < 2000) {
						// hcolor[3] = 0;
						// Dont render anything
					} else if (cg.time - cg.deathTime < 6000) {
						// In fade
						hcolor[3] = ((float)(cg.time - cg.deathTime - 2000) / 4000.0f); // 0 to 4000 converted to 0.0f to 1.0f
						CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
					} else {
						hcolor[3] = 1;
						CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
					}
				}
			} else if (cg.deathcamFadeStart) {
				int tr;
				float x;
				hcolor[0] = hcolor[1] = hcolor[2] = 0;
				tr = ceil((float)(cg.deathcamTime - cg.time) / 1000.0f);
				lowHealthPhase = 0;	// Dont glow red
				if (tr < 0) { 
					tr = 0;
				}
				// Time to fade back in
				if (cg.time - cg.deathcamFadeStart < 4000) {
					// Fading back in ^_^
					hcolor[3] = 1.0f - ((float)(cg.time - cg.deathcamFadeStart) / 4000.0f); // 0 to 4000 converted to 0.0f to 1.0f
					CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
				}
				trap->R_SetColor(NULL);
				// Display the 'you will be revived in xx seconds'
				CG_DrawPic(80, 390, 480, 55, cgs.media.horizgradient);
				if (tr) {
					x = 320 - (trap->R_Font_StrLenPixels(va("Your clone will be ready in %i seconds",tr), cgs.media.deathfont, 0.6f) / 2);
					trap->R_Font_DrawString(x, 410, va("Your clone will be ready in %i seconds",tr), colorWhite, cgs.media.deathfont, -1, 0.6f);
				} else {
					//x = 320 - (trap->R_Font_StrLenPixels("Press the attack key to revive yourself", cgs.media.deathfont, 0.6f) / 2);
					trap->R_Font_DrawString(165, 410, "Press the attack key to revive yourself", colorWhite, cgs.media.deathfont, -1, 0.6f);
				}
			}
		}
		if (lowHealthPhase != 0.0f) {
			hcolor[0] = 1;
			hcolor[1] = 0;
			hcolor[2] = 0;
			hcolor[3] = 0.6f * lowHealthPhase; // (float)(30 - cg.snap->ps.stats[STAT_HEALTH]) * 0.02f;
			hcolor[3] *= fabs((sinf((float)cg.time / 750.0f)));
			trap->R_SetColor(hcolor);
			CG_DrawPic(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, cgs.media.lowHealthAura);
			trap->R_SetColor(NULL);
		}
	}

	if ( (cg.refdef.viewContents&CONTENTS_LAVA) )
	{//tint screen red
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.5f + (0.15f*sin( phase ));
		hcolor[0] = 0.7f;
		hcolor[1] = 0;
		hcolor[2] = 0;
		
		CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
	}
	else if ( (cg.refdef.viewContents&CONTENTS_SLIME) )
	{//tint screen green
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.4f + (0.1f*sin( phase ));
		hcolor[0] = 0;
		hcolor[1] = 0.7f;
		hcolor[2] = 0;
		
		CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
	}
	else if ( (cg.refdef.viewContents&CONTENTS_WATER) )
	{//tint screen light blue -- FIXME: don't do this if CONTENTS_FOG? (in case someone *does* make a water shader with fog in it?)
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.3f + (0.05f*sin( phase ));
		hcolor[0] = 0;
		hcolor[1] = 0.2f;
		hcolor[2] = 0.8f;
		
		CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);
	}
}
void Cin_ProcessFade();
void Cin_ProcessFlash();
void CinBuild_Visualize2D();


void ChatBox_DrawBackdrop(menuDef_t *menu);
void ChatBox_DrawChat(menuDef_t *menu);

extern void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars, qhandle_t textShader );

void CG_DrawChatboxH(menuDef_t *menuHUD) {
	itemDef_t *item;
	vec4_t tmpCol;

	item = Menu_FindItemByName(menuHUD, "h_local");
	if (item) {
		VectorCopy4(item->window.foreColor, tmpCol);
		tmpCol[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( tmpCol );
		trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
	}
	item = Menu_FindItemByName(menuHUD, "h_guild");
	if (item) {
		VectorCopy4(item->window.foreColor, tmpCol);
		tmpCol[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( tmpCol );
		trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
	}

	item = Menu_FindItemByName(menuHUD, "h_action");
	if (item) {
		VectorCopy4(item->window.foreColor, tmpCol);
		tmpCol[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( tmpCol );
		trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
	}

	item = Menu_FindItemByName(menuHUD, "h_comm");
	if (item) {
		VectorCopy4(item->window.foreColor, tmpCol);
		tmpCol[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( tmpCol );
		trap->R_DrawStretchPic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);
	}
}

void CG_DrawChatboxFrame(menuDef_t *menuHUD) {
	itemDef_t *item = Menu_FindItemByName(menuHUD, "frame");
	if (item) {
		vec4_t color;
		MAKERGBA(color, 1, 1, 1, cg.jkg_HUDOpacity);
		trap->R_SetColor( color );	
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background 
			);	
	}
}

void CG_DrawChatboxText(menuDef_t *menuHUD) {
	itemDef_t *item = Menu_FindItemByName(menuHUD, "text");
	float opacity = 1.0f;

	if (cg.snap->ps.pm_type != PM_SPECTATOR) {
		opacity = cg.jkg_HUDOpacity;
	}

	if (item)
	{
		int i;
		int line;
		vec4_t color;

		MAKERGBA(color,0,0,0,0.3f);
		color[3] *= opacity;
		CG_FillRect(item->window.rect.x-5, item->window.rect.y-2, item->window.rect.w+10, item->window.rect.h+4, color);
		VectorCopy(colorWhite, color);
		color[3] *= opacity;
		// Center and draw the text, positioning will be finetuned later on :P
		line = 0;
		for (i=0; i<MAX_CHATBOX_ITEMS; i++) {
			int idx = (cg.chatItemNext + i) % MAX_CHATBOX_ITEMS;
			if (cg.chatItems[idx].active) {
				ChatBox_SetPaletteAlpha(cg.chatItems[idx].alpha*opacity);
				color[3] = cg.chatItems[idx].alpha*opacity;
				CG_DrawStringExt(item->window.rect.x-5, item->window.rect.y + (line * 8) - 2,
					cg.chatItems[idx].string,
					color, qfalse, qfalse, 5, 8, strlen(cg.chatItems[idx].string), cgs.media.charsetShader);
				line += cg.chatItems[idx].lines;
			}
		}
		ChatBox_SetPaletteAlpha(1);
	}
}

void CG_DrawChatbox() {
	menuDef_t *menuHUD = Menus_FindByName("hud_chatbox");
	if (!menuHUD) {
		return;
	}

	CG_DrawChatboxH(menuHUD);
	ChatBox_DrawBackdrop(menuHUD);
	CG_DrawChatboxFrame(menuHUD);
	CG_DrawChatboxText(menuHUD);
	
	vec4_t color;
	MAKERGBA(color, 1, 1, 1, 1*cg.jkg_HUDOpacity);
	trap->R_SetColor(color);

	ChatBox_DrawChat(menuHUD);
}


static int cg_performanceSampleNum = 0;

float CG_FractionalForPerformance(uint64_t sample, uint64_t maxSample)
{
	return 1.0f - (sample / (float)maxSample);
}

void CG_PerformanceAnalysis()
{
	performanceData_t* data = trap->Perf_GetData();
	int x = 10;
	int y = 90;
	char buff[128];

	trap->Cvar_VariableStringBuffer("perf", buff, 128);
	if (atoi(buff) <= 0)
	{
		return;	// perf is disabled
	}

	// Iterate through all of the data
	for (int i = 0; i < MAX_PERFORMANCE_TAGS; i++)
	{
		performanceTag_t* tag = &((*data)[i]);
		if (tag->tagUsed)
		{
			if (!Q_stricmp(tag->tagName, "renderer"))
			{
				cg_performanceData.rendererSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// purple square
				trap->R_SetColor(colorMagenta);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			else if (!Q_stricmp(tag->tagName, "cgame"))
			{
				cg_performanceData.cgameSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// red square
				trap->R_SetColor(colorRed);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			else if (!Q_stricmp(tag->tagName, "client"))
			{
				cg_performanceData.clientSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// orange square
				trap->R_SetColor(colorOrange);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			else if (!Q_stricmp(tag->tagName, "gamex86"))
			{
				cg_performanceData.gameSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// cyan square
				trap->R_SetColor(colorCyan);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			else if (!Q_stricmp(tag->tagName, "server"))
			{
				cg_performanceData.serverSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// blue square
				trap->R_SetColor(colorLtBlue);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			else if (!Q_stricmp(tag->tagName, "com_frame"))
			{
				cg_performanceData.frameSamples[cg_performanceSampleNum] = tag->timeAccumulated;

				// green square
				trap->R_SetColor(colorGreen);
				trap->R_DrawStretchPic(x - 4, y + 2.5f, 3, 3, 0, 0, 0, 0, cgs.media.whiteShader);
			}
			trap->R_Font_DrawString(x, y, va("%s - %.4f ms", tag->tagName, tag->timeAccumulated / 1000000.0f), colorWhite, cgDC.Assets.qhSmallFont, -1, 0.3f);
			y += 5;
		}
	}

	// Increment current sample counter
	cg_performanceSampleNum++;
	cg_performanceSampleNum %= LAG_SAMPLES;

	// Find the max frame time
	uint64_t maxFrameTime = 0;

	for (int i = 0; i < LAG_SAMPLES; i++)
	{
		if (cg_performanceData.frameSamples[i] > maxFrameTime)
		{
			maxFrameTime = cg_performanceData.frameSamples[i];
		}
	}

	// If the max frame time is 0, then we can't render anything
	if (maxFrameTime <= 0)
	{
		return;
	}

	x = 20;
	y = 30;

	for (int sample = cg_performanceSampleNum + 1; sample != cg_performanceSampleNum; sample++, sample %= LAG_SAMPLES)
	{
		// Draw pixels

		// purple square = renderer
		trap->R_SetColor(colorMagenta);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.rendererSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);
		
		// red square = cgame
		trap->R_SetColor(colorRed);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.cgameSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);

		// orange square = client
		trap->R_SetColor(colorOrange);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.clientSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);

		// cyan square = gamex86
		trap->R_SetColor(colorCyan);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.gameSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);

		// blue square = server
		trap->R_SetColor(colorLtBlue);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.serverSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);

		// green square = com_frame
		trap->R_SetColor(colorGreen);
		trap->R_DrawStretchPic(x, y + (CG_FractionalForPerformance(cg_performanceData.frameSamples[sample], maxFrameTime) * 50.0f), 1, 1, 0, 0, 0, 0, cgs.media.whiteShader);

		x++; // move ahead by one pixel
	}
}

void ChatBox_CloseChat();
static void CG_Draw2D( void ) {
	float			fallTime; 
	int				drawHUD	= cg_drawHUD.integer;

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}
	
	if (cg.cinematicState) {
		vec4_t cincolor = {0,0,0,1};
		float transpos = 0; // (0 to 150)
		Cin_ProcessFade(); // Cinematic effect processing
		Cin_ProcessFlash();
		// If its 0 its off, so ya :P
		if (cg.cinematicState == 2) {
			if (!jkg_noletterbox.integer) {
				trap->R_SetColor(&cincolor[0]);
				trap->R_DrawStretchPic(0,0,640,75,0,0,0,0,cgs.media.whiteShader);
				trap->R_DrawStretchPic(0,480-75,640,75,0,0,0,0,cgs.media.whiteShader);
				trap->R_SetColor(NULL);
			}
			// No HUD when in cinematic mode
			return;
		}
		// We're in transition
		if (cg.cinematicState == 1) {
			// Fade in
			if (cg.time - cg.cinematicTime > 500)  {
				transpos = 75;
				cg.cinematicState = 2;
			} else if (cg.cinematicTime > cg.time) {
				// Should never happen, but just in case
				transpos = 0;
			} else {
				transpos = ((float)(cg.time - cg.cinematicTime) / 500.0f) * 75;
			}
		} else if (cg.cinematicState == 3) {
			// Fade out
			if (cg.time - cg.cinematicTime  > 500)  {
				transpos = 0;
				cg.cinematicState = 0;
			} else if (cg.cinematicTime > cg.time) {
				// Should never happen, but just in case
				transpos = 75;
			} else {
				transpos = ((float)(cg.time - cg.cinematicTime) / 500.0f) * 75;
				transpos = 75 - transpos;
			}
		} else {
			trap->Error(ERR_DROP, va("CG_Draw2D: Invalid cinematic state %i", cg.cinematicState));
		}
		if (!jkg_noletterbox.integer) {
			trap->R_SetColor(&cincolor[0]);
			trap->R_DrawStretchPic(0,0,640,transpos,0,0,0,0,cgs.media.whiteShader);
			trap->R_DrawStretchPic(0,480-transpos,640,transpos,0,0,0,0,cgs.media.whiteShader);
			trap->R_SetColor(NULL);
		}
		return;
	}

	if ( cg_draw2D.integer == 0 ) {
		// Screeny purposes
		CG_DrawGrenade();
		return;
	}

	CG_PerformanceAnalysis();
	CinBuild_Visualize2D();

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

	CG_Draw2DScreenTints();
	if (drawHUD && !ui_hidehud.integer) {
		if (cg.snap->ps.fd.forcePowersActive || cg.snap->ps.fd.forceRageRecoveryTime > cg.time)
		{
			CG_DrawActivePowers();
		}

		if (cg.predictedPlayerState.emplacedIndex > 0)
		{
			centity_t *eweb = &cg_entities[cg.predictedPlayerState.emplacedIndex];

			if (eweb->currentState.weapon == WP_NONE)
			{ //using an e-web, draw its health
				CG_DrawEWebHealth();
			}
		}

		// Draw this before the text so that any text won't get clipped off
		if( cg.predictedPlayerState.zoomMode )
		{
			JKG_RenderScope (&cg_entities[cg.predictedPlayerState.clientNum]);
		}

		if( cg.predictedPlayerState.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) )
		{
			CG_Text_Paint( 40, 40, 0.6f, colorWhite, "Projectile Blocking Mode", 0, 0, 0, 3 );
		}
		else if( cg.predictedPlayerState.saberActionFlags & (1 << SAF_BLOCKING) )
		{
			CG_Text_Paint( 40, 40, 0.6f, colorWhite, "Blocking Mode", 0, 0, 0, 3 );
		}

		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
			CG_DrawSpectator();
			CG_DrawCrosshair(NULL, 0);
			CG_DrawCrosshairNames();
			CG_SaberClashFlare();
			CG_DrawChatbox();	// Only show the chatbox when we're alive (JKG)
		} else {
			// don't draw any status if dead or the scoreboard is being explicitly shown
			if ( cg.snap->ps.stats[STAT_HEALTH] > 0 && !cg.deathcamFadeStart) 
			{ //!cg.showScores && to bring back scoreboard on tab press.

				if ( /*cg_drawStatus.integer*/0 ) {
					//Reenable if stats are drawn with menu system again
					Menu_PaintAll();
					CG_DrawTimedMenus();
				}

				CG_DrawMessageNotifications();

				CG_DrawCrosshairNames();

				CG_SaberClashFlare();

				if (cg_drawStatus.integer)
				{
					CG_DrawFlagStatus();
					CG_DrawStatus();
				}

				// Draw Team Overlay (also used by parties and such)
				CG_DrawTeamOverlay();

				CG_DrawReload();
				CG_DrawGrenade();

				CG_DrawChatbox();	// Only show the chatbox when we're alive (JKG)
			}
			else 
			{
				CG_DrawMessageNotifications();	//continue drawing message notifications

				//if we die and we are chatting interrupt it
				if (cg.isChatting) 
				{
					if(cg.snap->ps.clientNum == cg.clientNum) { 
						ChatBox_CloseChat();
					}

					//(except on spectators)
					else { 
						CG_DrawChatbox();
					}
				}
			}
	    
		}
	}

	if (cg.snap->ps.fallingToDeath && !cg.deathcamFadeStart)
	{
		vec4_t	hcolor;

		fallTime = (float)(cg.time - cg.snap->ps.fallingToDeath);

		fallTime /= (FALL_FADE_TIME/2);

		if (fallTime < 0)
		{
			fallTime = 0;
		}
		if (fallTime > 1)
		{
			fallTime = 1;
		}

		hcolor[3] = fallTime;
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 0;

		CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hcolor);

		if (!gCGHasFallVector)
		{
			VectorCopy(cg.snap->ps.origin, gCGFallVector);
			gCGHasFallVector = qtrue;
		}
	}
	else
	{
		if (gCGHasFallVector)
		{
			gCGHasFallVector = qfalse;
			VectorClear(gCGFallVector);
		}
	}

	// UQ1: Added. Draw NPC civilian names over their heads...
	CG_DrawNPCNames();

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();
	

	if (!cl_paused.integer) {
		CG_DrawUpperRight();
	}

	if ( !CG_DrawFollow() ) {
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawOldScoreboard();
	if ( !cg.scoreBoardShowing) {
		CG_DrawCenterString();
	}
}

static void CG_DrawMiscStaticModels (void) {
	int i, j;
	refEntity_t ent;
	vec3_t cullorg;
	vec3_t diff;

	memset (&ent, 0, sizeof(ent));

	ent.reType = RT_MODEL;
	ent.frame = 0;
	ent.nonNormalizedAxes = qtrue;

	// static models don't project shadows
	ent.renderfx = RF_NOSHADOW;

	for ( i = 0; i < cgs.numMiscStaticModels; i++ ) {
		VectorCopy (cgs.miscStaticModels[i].org, cullorg);
		cullorg[2] += 1.0f;

		if ( cgs.miscStaticModels[i].zoffset ) {
			cullorg[2] += cgs.miscStaticModels[i].zoffset;
		}
		if ( cgs.miscStaticModels[i].radius ) {
			if ( CG_CullPointAndRadius (cullorg, cgs.miscStaticModels[i].radius) ) {
				continue;
			}
		}

		if ( !trap->R_InPVS (cg.refdef.vieworg, cullorg, cg.refdef.areamask) ) {
			continue;
		}

		VectorCopy (cgs.miscStaticModels[i].org, ent.origin);
		VectorCopy (cgs.miscStaticModels[i].org, ent.oldorigin);
		VectorCopy (cgs.miscStaticModels[i].org, ent.lightingOrigin);

		for ( j = 0; j < 3; j++ ) {
			VectorCopy (cgs.miscStaticModels[i].axes[j], ent.axis[j]);
		}
		ent.hModel = cgs.miscStaticModels[i].model;

		VectorSubtract (ent.origin, cg.refdef.vieworg, diff);
		if ( VectorLength (diff) - (cgs.miscStaticModels[i].radius) <= cg.distanceCull ) {
			trap->R_AddRefEntityToScene (&ent);
		}
	}
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/

int Cin_ProcessMB();
void Cin_ProcessCM();

extern void PP_PreRender (void);

void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		trap->Error( ERR_DROP, "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	cg.refdef.rdflags |= RDF_DRAWSKYBOX;

	CG_DrawMiscStaticModels ();

	// draw 3D view
	trap->R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}
		
	// draw status bar and other floating elements
 	CG_Draw2D();
}

