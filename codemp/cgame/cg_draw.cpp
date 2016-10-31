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

// The time at which you died and the time it will take for you to rejoin game.
int cg_siegeDeathTime = 0;

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

char *showPowersName[] = 
{
	"HEAL2",//FP_HEAL
	"JUMP2",//FP_LEVITATION
	"SPEED2",//FP_SPEED
	"PUSH2",//FP_PUSH
	"PULL2",//FP_PULL
	"MINDTRICK2",//FP_TELEPTAHY
	"GRIP2",//FP_GRIP
	"LIGHTNING2",//FP_LIGHTNING
	"DARK_RAGE2",//FP_RAGE
	"PROTECT2",//FP_PROTECT
	"ABSORB2",//FP_ABSORB
	"TEAM_HEAL2",//FP_TEAM_HEAL
	"TEAM_REPLENISH2",//FP_TEAM_FORCE
	"DRAIN2",//FP_DRAIN
	"SEEING2",//FP_SEE
	"SABER_OFFENSE2",//FP_SABER_OFFENSE
	"SABER_DEFENSE2",//FP_SABER_DEFENSE
	"SABER_THROW2",//FP_SABERTHROW
	NULL
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
qboolean CG_WorldCoordToScreenCoord(vec3_t worldCoord, int *x, int *y)

  Take any world coord and convert it to a 2D virtual 640x480 screen coord
*/
/*
qboolean CG_WorldCoordToScreenCoordFloat(vec3_t worldCoord, float *x, float *y)
{
	int	xcenter, ycenter;
	vec3_t	local, transformed;

//	xcenter = cg.refdef.width / 2;//gives screen coords adjusted for resolution
//	ycenter = cg.refdef.height / 2;//gives screen coords adjusted for resolution
	
	//NOTE: did it this way because most draw functions expect virtual 640x480 coords
	//	and adjust them for current resolution
	xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
	ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn

	VectorSubtract (worldCoord, cg.refdef.vieworg, local);

	transformed[0] = DotProduct(local,vright);
	transformed[1] = DotProduct(local,vup);
	transformed[2] = DotProduct(local,vfwd);		

	// Make sure Z is not negative.
	if(transformed[2] < 0.01f)
	{
		return qfalse;
	}
	// Simple convert to screen coords.
	float xzi = xcenter / transformed[2] * (90.0f/cg.refdef.fov_x);
	float yzi = ycenter / transformed[2] * (90.0f/cg.refdef.fov_y);

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
*/

// Convert HSL to RGB
void HSL2RGB(float h, float s, float l, float *r, float *g, float *b) {
	double tr, tg, tb;
	double v;

	tr = l;
	tg = l;
	tb = l;
	v = (l <= 0.5f) ? (l * (1.0f + (l*2))) : (l + l - l * l);
    if (v > 0) {
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m ) / v;
		h *= 6.0f;
		sextant = (int)h;
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant)
		{
			case 0:
			default:
				tr = v;
				tg = mid1;
				tb = m;
				break;
			case 1:
				tr = mid2;
				tg = v;
				tb = m;
				break;
			case 2:
				tr = m;
				tg = v;
				tb = mid1;
				break;
			case 3:
				tr = m;
				tg = mid2;
				tb = v;
				break;
			case 4:
				tr = mid1;
				tg = m;
				tb = v;
				break;
			case 5:
				tr = v;
				tg = m;
				tb = mid2;
				break;
          }
    }
    *r = tr;
    *g = tg;
	*b = tb;
}

/*
================
CG_Draw3DModel

JKGFIXME: Again, probably another func on the chopping block... we don't need this at all.

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, void *ghoul2, int g2radius, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.ghoul2 = ghoul2;
	ent.radius = g2radius;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap->R_ClearScene();
	trap->R_AddRefEntityToScene( &ent );
	trap->R_RenderScene( &refdef );
}

#define MAX_SHOWPOWERS NUM_FORCE_POWERS

qboolean ForcePower_Valid(int i)
{
	if (i == FP_LEVITATION ||
		i == FP_SABER_OFFENSE ||
		i == FP_SABER_DEFENSE ||
		i == FP_SABERTHROW)
	{
		return qfalse;
	}

	if (cg.snap->ps.fd.forcePowersKnown & (1 << i))
	{
		return qtrue;
	}
	
	return qfalse;
}

/*
===================
CG_DrawForceSelect
===================
*/
void CG_DrawForceSelect( void ) 
{
	int		i;
	int		count;
	int		smallIconSize,bigIconSize;
	int		holdX,x,y,x2,y2,pad,length;
	int		sideLeftIconCnt,sideRightIconCnt;
	int		sideMax,holdCount,iconCnt;
	int		yOffset = 0;


	x2 = 0;
	y2 = 0;

	// don't display if dead
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		return;
	}

	if ((cg.forceSelectTime+WEAPON_SELECT_TIME)<cg.time)	// Time is up for the HUD to display
	{
		cg.forceSelect = cg.snap->ps.fd.forcePowerSelected;
		return;
	}

	if (!cg.snap->ps.fd.forcePowersKnown)
	{
		return;
	}

	// count the number of powers owned
	count = 0;

	for (i=0;i < NUM_FORCE_POWERS;++i)
	{
		if (ForcePower_Valid(i))
		{
			count++;
		}
	}

	if (count == 0)	// If no force powers, don't display
	{
		return;
	}

	sideMax = 3;	// Max number of icons on the side

	// Calculate how many icons will appear to either side of the center one
	holdCount = count - 1;	// -1 for the center icon
	if (holdCount == 0)			// No icons to either side
	{
		sideLeftIconCnt = 0;
		sideRightIconCnt = 0;
	}
	else if (count > (2*sideMax))	// Go to the max on each side
	{
		sideLeftIconCnt = sideMax;
		sideRightIconCnt = sideMax;
	}
	else							// Less than max, so do the calc
	{
		sideLeftIconCnt = holdCount/2;
		sideRightIconCnt = holdCount - sideLeftIconCnt;
	}

	smallIconSize = 30;
	bigIconSize = 60;
	pad = 12;

	x = 320;
	y = 425;

	// Background
	length = (sideLeftIconCnt * smallIconSize) + (sideLeftIconCnt*pad) +
			bigIconSize + (sideRightIconCnt * smallIconSize) + (sideRightIconCnt*pad) + 12;
	
	i = BG_ProperForceIndex(cg.forceSelect) - 1;
	if (i < 0)
	{
		i = MAX_SHOWPOWERS;
	}

	trap->R_SetColor(NULL);
	// Work backwards from current icon
	holdX = x - ((bigIconSize/2) + pad + smallIconSize);
	for (iconCnt=1;iconCnt<(sideLeftIconCnt+1);i--)
	{
		if (i < 0)
		{
			i = MAX_SHOWPOWERS;
		}

		if (!ForcePower_Valid(forcePowerSorted[i]))	// Does he have this power?
		{
			continue;
		}

		++iconCnt;					// Good icon

		if (cgs.media.forcePowerIcons[forcePowerSorted[i]])
		{
			CG_DrawPic( holdX, y + yOffset, smallIconSize, smallIconSize, cgs.media.forcePowerIcons[forcePowerSorted[i]] ); 
			holdX -= (smallIconSize+pad);
		}
	}

	if (ForcePower_Valid(cg.forceSelect))
	{
		// Current Center Icon
		if (cgs.media.forcePowerIcons[cg.forceSelect])
		{
			CG_DrawPic( x-(bigIconSize/2), (y-((bigIconSize-smallIconSize)/2)) + yOffset, bigIconSize, bigIconSize, cgs.media.forcePowerIcons[cg.forceSelect] ); //only cache the icon for display
		}
	}

	i = BG_ProperForceIndex(cg.forceSelect) + 1;
	if (i>MAX_SHOWPOWERS)
	{
		i = 0;
	}

	// Work forwards from current icon
	holdX = x + (bigIconSize/2) + pad;
	for (iconCnt=1;iconCnt<(sideRightIconCnt+1);i++)
	{
		if (i>MAX_SHOWPOWERS)
		{
			i = 0;
		}

		if (!ForcePower_Valid(forcePowerSorted[i]))	// Does he have this power?
		{
			continue;
		}

		++iconCnt;					// Good icon

		if (cgs.media.forcePowerIcons[forcePowerSorted[i]])
		{
			CG_DrawPic( holdX, y + yOffset, smallIconSize, smallIconSize, cgs.media.forcePowerIcons[forcePowerSorted[i]] ); //only cache the icon for display
			holdX += (smallIconSize+pad);
		}
	}

	if ( showPowersName[cg.forceSelect] ) 
	{
		UI_DrawProportionalString(320, y + 30 + yOffset, CG_GetStringEdString("SP_INGAME", showPowersName[cg.forceSelect]), UI_CENTER | UI_SMALLFONT, colorTable[CT_ICON_BLUE], FONT_SMALL);
	}
}

int cg_targVeh = ENTITYNUM_NONE;
int cg_targVehLastTime = 0;
qboolean CG_CheckTargetVehicle( centity_t **pTargetVeh, float *alpha )
{
	int targetNum = ENTITYNUM_NONE;
	centity_t	*targetVeh = NULL;
	
	if ( !pTargetVeh || !alpha )
	{//hey, where are my pointers?
		return qfalse;
	}

	*alpha = 1.0f;

	//FIXME: need to clear all of these when you die?
	if ( cg.predictedPlayerState.rocketLockIndex < ENTITYNUM_WORLD )
	{
		targetNum = cg.predictedPlayerState.rocketLockIndex;
	}
	else if ( cg.crosshairVehNum < ENTITYNUM_WORLD 
		&& cg.time - cg.crosshairVehTime < 3000 )
	{//crosshair was on a vehicle in the last 3 seconds
		targetNum = cg.crosshairVehNum;
	}
    else if ( cg.crosshairClientNum < ENTITYNUM_WORLD )
	{
		targetNum = cg.crosshairClientNum;
	}

	if ( targetNum < MAX_CLIENTS )
	{//real client
		if ( cg_entities[targetNum].currentState.m_iVehicleNum >= MAX_CLIENTS )
		{//in a vehicle
			targetNum = cg_entities[targetNum].currentState.m_iVehicleNum;
		}
	}
    if ( targetNum < ENTITYNUM_WORLD 
		&& targetNum >= MAX_CLIENTS )
	{
		centity_t *targetVeh = &cg_entities[targetNum];
		if ( targetVeh->currentState.NPC_class == CLASS_VEHICLE 
			&& targetVeh->m_pVehicle
			&& targetVeh->m_pVehicle->m_pVehicleInfo
			&& targetVeh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
		{//it's a vehicle
			cg_targVeh = targetNum;
			cg_targVehLastTime = cg.time;
			*alpha = 1.0f;
		}
		else
		{
			targetVeh = NULL;
		}
	}
	if ( !targetVeh )
	{
		*pTargetVeh = targetVeh;
		return qtrue;
	}

	if ( cg_targVehLastTime && cg.time - cg_targVehLastTime < 3000 )
	{
		targetVeh = &cg_entities[cg_targVeh];

		//stay at full alpha for 1 sec after lose them from crosshair
		if ( cg.time-cg_targVehLastTime < 1000 )
			*alpha = 1.0f;
		else //fade out over 2 secs
			*alpha = 1.0f-((cg.time-cg_targVehLastTime-1000)/2000.0f);
	}
	return qfalse;
}

#define MAX_VHUD_SHIELD_TICS 12
#define MAX_VHUD_SPEED_TICS 5
#define MAX_VHUD_ARMOR_TICS 5
#define MAX_VHUD_AMMO_TICS 5

float CG_DrawVehicleShields( const menuDef_t	*menuHUD, const centity_t *veh )
{
	int				i;
	char			itemName[64];
	float			inc, currValue,maxShields;
	vec4_t			calcColor;
	itemDef_t		*item;
	float			percShields;

	item = Menu_FindItemByName((menuDef_t	*) menuHUD, "armorbackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	maxShields = veh->m_pVehicle->m_pVehicleInfo->shields;
	currValue = cg.predictedVehicleState.stats[STAT_ARMOR];
	percShields = (float)currValue/(float)maxShields;
	// Print all the tics of the shield graphic
	// Look at the amount of health left and show only as much of the graphic as there is health.
	// Use alpha to fade out partial section of health
	inc = (float) maxShields / MAX_VHUD_ARMOR_TICS;
	for (i=1;i<=MAX_VHUD_ARMOR_TICS;i++)
	{
		sprintf( itemName, "armor_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t *) menuHUD, itemName);

		if (!item)
		{
			continue;
		}

		memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));

		if (currValue <= 0)	// don't show tic
		{
			break;
		}
		else if (currValue < inc)	// partial tic (alpha it out)
		{
			float percent = currValue / inc;
			calcColor[3] *= percent;		// Fade it out
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}

	return percShields;
}

int cg_vehicleAmmoWarning = 0;
int cg_vehicleAmmoWarningTime = 0;
void CG_DrawVehicleAmmo( const menuDef_t *menuHUD, const centity_t *veh )
{
	int i;
	char itemName[64];
	float inc, currValue,maxAmmo;
	vec4_t	calcColor;
	itemDef_t	*item;

	item = Menu_FindItemByName((menuDef_t *) menuHUD, "ammobackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	maxAmmo = veh->m_pVehicle->m_pVehicleInfo->weapon[0].ammoMax;
	currValue = cg.predictedVehicleState.ammo;
	
	inc = (float) maxAmmo / MAX_VHUD_AMMO_TICS;
	for (i=1;i<=MAX_VHUD_AMMO_TICS;i++)
	{
		sprintf( itemName, "ammo_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t *)menuHUD, itemName);

		if (!item)
		{
			continue;
		}

		if ( cg_vehicleAmmoWarningTime > cg.time 
			&& cg_vehicleAmmoWarning == 0 )
		{
			memcpy(calcColor, g_color_table[ColorIndex(COLOR_RED)], sizeof(vec4_t));
			calcColor[3] = sin(cg.time*0.005f)*0.5f+0.5f;
		}
		else
		{
			memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));

			if (currValue <= 0)	// don't show tic
			{
				break;
			}
			else if (currValue < inc)	// partial tic (alpha it out)
			{
				float percent = currValue / inc;
				calcColor[3] *= percent;		// Fade it out
			}
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}
}


void CG_DrawVehicleAmmoUpper( const menuDef_t *menuHUD, const centity_t *veh )
{
	int			i;
	char		itemName[64];
	float		inc, currValue,maxAmmo;
	vec4_t		calcColor;
	itemDef_t	*item;

	item = Menu_FindItemByName((menuDef_t *)menuHUD, "ammoupperbackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	maxAmmo = veh->m_pVehicle->m_pVehicleInfo->weapon[0].ammoMax;
	currValue = cg.predictedVehicleState.ammo;

	inc = (float) maxAmmo / MAX_VHUD_AMMO_TICS;
	for (i=1;i<MAX_VHUD_AMMO_TICS;i++)
	{
		sprintf( itemName, "ammoupper_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t *)menuHUD, itemName);

		if (!item)
		{
			continue;
		}

		if ( cg_vehicleAmmoWarningTime > cg.time 
			&& cg_vehicleAmmoWarning == 0 )
		{
			memcpy(calcColor, g_color_table[ColorIndex(COLOR_RED)], sizeof(vec4_t));
			calcColor[3] = sin(cg.time*0.005f)*0.5f+0.5f;
		}
		else
		{
			memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));

			if (currValue <= 0)	// don't show tic
			{
				break;
			}
			else if (currValue < inc)	// partial tic (alpha it out)
			{
				float percent = currValue / inc;
				calcColor[3] *= percent;		// Fade it out
			}
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}
}


void CG_DrawVehicleAmmoLower( const menuDef_t *menuHUD, const centity_t *veh )
{
	int				i;
	char			itemName[64];
	float			inc, currValue,maxAmmo;
	vec4_t			calcColor;
	itemDef_t		*item;


	item = Menu_FindItemByName((menuDef_t *)menuHUD, "ammolowerbackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	maxAmmo = veh->m_pVehicle->m_pVehicleInfo->weapon[1].ammoMax;
	currValue = cg.predictedVehicleState.ammo;

	inc = (float) maxAmmo / MAX_VHUD_AMMO_TICS;
	for (i=1;i<MAX_VHUD_AMMO_TICS;i++)
	{
		sprintf( itemName, "ammolower_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t *)menuHUD, itemName);

		if (!item)
		{
			continue;
		}

		if ( cg_vehicleAmmoWarningTime > cg.time 
			&& cg_vehicleAmmoWarning == 1 )
		{
			memcpy(calcColor, g_color_table[ColorIndex(COLOR_RED)], sizeof(vec4_t));
			calcColor[3] = sin(cg.time*0.005f)*0.5f+0.5f;
		}
		else
		{
			memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));

			if (currValue <= 0)	// don't show tic
			{
				break;
			}
			else if (currValue < inc)	// partial tic (alpha it out)
			{
				float percent = currValue / inc;
				calcColor[3] *= percent;		// Fade it out
			}
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}
}

// The HUD.menu file has the graphic print with a negative height, so it will print from the bottom up.
void CG_DrawVehicleTurboRecharge( const menuDef_t	*menuHUD, const centity_t *veh )
{
	itemDef_t	*item;
	int			height;

	item = Menu_FindItemByName( (menuDef_t	*) menuHUD, "turborecharge");

	if (item)
	{
		float percent=0.0f;
		int diff = ( cg.time - veh->m_pVehicle->m_iTurboTime );

		height = item->window.rect.h;

		if (diff > veh->m_pVehicle->m_pVehicleInfo->turboRecharge)
		{
			percent = 1.0f;
			trap->R_SetColor( colorTable[CT_GREEN] );
		}
		else 
		{
			percent = (float) diff / veh->m_pVehicle->m_pVehicleInfo->turboRecharge;
			if (percent < 0.0f)
			{
				percent = 0.0f;
			}
			trap->R_SetColor( colorTable[CT_RED] );
		}

		height *= percent;

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			height, 
			cgs.media.whiteShader);	
	}
}

qboolean cg_drawLink = qfalse;
void CG_DrawVehicleWeaponsLinked( const menuDef_t	*menuHUD, const centity_t *veh )
{
	qboolean drawLink = qfalse;
	if ( veh->m_pVehicle
		&& veh->m_pVehicle->m_pVehicleInfo
		&& (veh->m_pVehicle->m_pVehicleInfo->weapon[0].linkable == 2|| veh->m_pVehicle->m_pVehicleInfo->weapon[1].linkable == 2) )
	{//weapon is always linked
		drawLink = qtrue;
	}
	else
	{
//MP way:
		//must get sent over network
		if ( cg.predictedVehicleState.vehWeaponsLinked )
		{
			drawLink = qtrue;
		}
//NOTE: below is SP way
/*
		//just cheat it
		if ( veh->gent->m_pVehicle->weaponStatus[0].linked
			|| veh->gent->m_pVehicle->weaponStatus[1].linked )
		{
			drawLink = qtrue;
		}
*/
	}

	if ( cg_drawLink != drawLink )
	{//state changed, play sound
		cg_drawLink = drawLink;
		trap->S_StartSound (NULL, cg.predictedPlayerState.clientNum, CHAN_LOCAL, trap->S_RegisterSound( "sound/vehicles/common/linkweaps.wav" ) );
	}

	if ( drawLink )
	{
		itemDef_t	*item;

		item = Menu_FindItemByName( (menuDef_t	*) menuHUD, "weaponslinked");

		if (item)
		{
			trap->R_SetColor( colorTable[CT_CYAN] );

				CG_DrawPic( 
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h, 
				cgs.media.whiteShader);	
		}
	}
}

void CG_DrawVehicleSpeed( const menuDef_t	*menuHUD, const centity_t *veh )
{
	int i;
	char itemName[64];
	float inc, currValue,maxSpeed;
	vec4_t		calcColor;
	itemDef_t	*item;

	item = Menu_FindItemByName((menuDef_t *) menuHUD, "speedbackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	maxSpeed = veh->m_pVehicle->m_pVehicleInfo->speedMax;
	currValue = cg.predictedVehicleState.speed;


	// Print all the tics of the shield graphic
	// Look at the amount of health left and show only as much of the graphic as there is health.
	// Use alpha to fade out partial section of health
	inc = (float) maxSpeed / MAX_VHUD_SPEED_TICS;
	for (i=1;i<=MAX_VHUD_SPEED_TICS;i++)
	{
		sprintf( itemName, "speed_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t *)menuHUD, itemName);

		if (!item)
		{
			continue;
		}

		if ( cg.time > veh->m_pVehicle->m_iTurboTime )
		{
			memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));
		}
		else	// In turbo mode
		{
			if (cg.VHUDFlashTime < cg.time)	
			{
				cg.VHUDFlashTime = cg.time + 200;
				if (cg.VHUDTurboFlag)
				{
					cg.VHUDTurboFlag = qfalse;
				}
				else
				{
					cg.VHUDTurboFlag = qtrue;
				}
			}

			if (cg.VHUDTurboFlag)
			{
				memcpy(calcColor, colorTable[CT_LTRED1], sizeof(vec4_t));
			}
			else
			{
				memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));
			}  
		}


		if (currValue <= 0)	// don't show tic
		{
			break;
		}
		else if (currValue < inc)	// partial tic (alpha it out)
		{
			float percent = currValue / inc;
			calcColor[3] *= percent;		// Fade it out
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}
}

void CG_DrawVehicleArmor( const menuDef_t *menuHUD, const centity_t *veh )
{
	int			i;
	vec4_t		calcColor;
	char		itemName[64];
	float		inc, currValue,maxArmor;
	itemDef_t	*item;

	maxArmor = veh->m_pVehicle->m_pVehicleInfo->armor;
	currValue = cg.predictedVehicleState.stats[STAT_HEALTH];

	item = Menu_FindItemByName( (menuDef_t	*) menuHUD, "shieldbackground");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}


	// Print all the tics of the shield graphic
	// Look at the amount of health left and show only as much of the graphic as there is health.
	// Use alpha to fade out partial section of health
	inc = (float) maxArmor / MAX_VHUD_SHIELD_TICS;
	for (i=1;i <= MAX_VHUD_SHIELD_TICS;i++)
	{
		sprintf( itemName, "shield_tic%d",	i );

		item = Menu_FindItemByName((menuDef_t	*) menuHUD, itemName);

		if (!item)
		{
			continue;
		}


		memcpy(calcColor, item->window.foreColor, sizeof(vec4_t));

		if (currValue <= 0)	// don't show tic
		{
			break;
		}
		else if (currValue < inc)	// partial tic (alpha it out)
		{
			float percent = currValue / inc;
			calcColor[3] *= percent;		// Fade it out
		}

		trap->R_SetColor( calcColor);

		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );

		currValue -= inc;
	}
}

enum
{
	VEH_DAMAGE_FRONT=0,
	VEH_DAMAGE_BACK,
	VEH_DAMAGE_LEFT,
	VEH_DAMAGE_RIGHT,
};

typedef struct 
{
	const char	*itemName;
	short	heavyDamage;
	short	lightDamage;
} veh_damage_t;

veh_damage_t vehDamageData[4] = 
{
	{ "vehicle_front",SHIPSURF_DAMAGE_FRONT_HEAVY,SHIPSURF_DAMAGE_FRONT_LIGHT },
	{ "vehicle_back",SHIPSURF_DAMAGE_BACK_HEAVY,SHIPSURF_DAMAGE_BACK_LIGHT },
	{ "vehicle_left",SHIPSURF_DAMAGE_LEFT_HEAVY,SHIPSURF_DAMAGE_LEFT_LIGHT },
	{ "vehicle_right",SHIPSURF_DAMAGE_RIGHT_HEAVY,SHIPSURF_DAMAGE_RIGHT_LIGHT },
};

// Draw health graphic for given part of vehicle
void CG_DrawVehicleDamage(const centity_t *veh,int brokenLimbs,const menuDef_t	*menuHUD,float alpha,int index)
{
	itemDef_t		*item;
	int				colorI;
	vec4_t			color;
	int				graphicHandle=0;

	item = Menu_FindItemByName((menuDef_t *)menuHUD, vehDamageData[index].itemName);
	if (item)
	{
		if (brokenLimbs & (1<<vehDamageData[index].heavyDamage))
		{
			colorI = CT_RED;
			if (brokenLimbs & (1<<vehDamageData[index].lightDamage))
			{
				colorI = CT_DKGREY;
			}
		}
		else if (brokenLimbs & (1<<vehDamageData[index].lightDamage))
		{
			colorI = CT_YELLOW;
		}
		else
		{
			colorI = CT_GREEN;
		}

		VectorCopy4 ( colorTable[colorI], color );
		color[3] = alpha;
		trap->R_SetColor( color );

		switch ( index )
		{
			case VEH_DAMAGE_FRONT :
				graphicHandle = veh->m_pVehicle->m_pVehicleInfo->iconFrontHandle;
				break;
			case VEH_DAMAGE_BACK :
				graphicHandle = veh->m_pVehicle->m_pVehicleInfo->iconBackHandle;
				break;
			case VEH_DAMAGE_LEFT :
				graphicHandle = veh->m_pVehicle->m_pVehicleInfo->iconLeftHandle;
				break;
			case VEH_DAMAGE_RIGHT :
				graphicHandle = veh->m_pVehicle->m_pVehicleInfo->iconRightHandle;
				break;
		}

		if (graphicHandle)
		{
			CG_DrawPic( 
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h, 
				graphicHandle );
		}
	}
}


// Used on both damage indicators :  player vehicle and the vehicle the player is locked on 
void CG_DrawVehicleDamageHUD(const centity_t *veh,int brokenLimbs,float percShields,char *menuName, float alpha)
{
	menuDef_t		*menuHUD;
	itemDef_t		*item;
	vec4_t			color;

	menuHUD = Menus_FindByName(menuName);

	if ( !menuHUD )
	{
		return;
	}

	item = Menu_FindItemByName(menuHUD, "background");
	if (item)
	{
		if (veh->m_pVehicle->m_pVehicleInfo->dmgIndicBackgroundHandle)
		{
			if ( veh->damageTime > cg.time )
			{//ship shields currently taking damage
				//NOTE: cent->damageAngle can be accessed to get the direction from the ship origin to the impact point (in 3-D space)
				float perc = 1.0f - ((veh->damageTime - cg.time) / 2000.0f/*MIN_SHIELD_TIME*/);
				if ( perc < 0.0f )
				{
					perc = 0.0f;
				}
				else if ( perc > 1.0f )
				{
					perc = 1.0f;
				}
				color[0] = item->window.foreColor[0];//flash red
				color[1] = item->window.foreColor[1]*perc;//fade other colors back in over time
				color[2] = item->window.foreColor[2]*perc;//fade other colors back in over time
				color[3] = item->window.foreColor[3];//always normal alpha
				trap->R_SetColor( color );
			}
			else
			{
				trap->R_SetColor( item->window.foreColor );
			}

			CG_DrawPic( 
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h, 
				veh->m_pVehicle->m_pVehicleInfo->dmgIndicBackgroundHandle );
		}
	}

	item = Menu_FindItemByName(menuHUD, "outer_frame");
	if (item)
	{
		if (veh->m_pVehicle->m_pVehicleInfo->dmgIndicFrameHandle)
		{
			trap->R_SetColor( item->window.foreColor );
			CG_DrawPic( 
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h, 
				veh->m_pVehicle->m_pVehicleInfo->dmgIndicFrameHandle );
		}
	}

	item = Menu_FindItemByName(menuHUD, "shields");
	if (item)
	{
		if (veh->m_pVehicle->m_pVehicleInfo->dmgIndicShieldHandle)
		{
			VectorCopy4 ( colorTable[CT_HUD_GREEN], color );
			color[3] = percShields;
			trap->R_SetColor( color );
			CG_DrawPic( 
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h, 
				veh->m_pVehicle->m_pVehicleInfo->dmgIndicShieldHandle );
		}
	}

	//TODO: if we check nextState.brokenLimbs & prevState.brokenLimbs, we can tell when a damage flag has been added and flash that part of the ship
	//FIXME: when ship explodes, either stop drawing ship or draw all parts black
	CG_DrawVehicleDamage(veh,brokenLimbs,menuHUD,alpha,VEH_DAMAGE_FRONT);
	CG_DrawVehicleDamage(veh,brokenLimbs,menuHUD,alpha,VEH_DAMAGE_BACK);
	CG_DrawVehicleDamage(veh,brokenLimbs,menuHUD,alpha,VEH_DAMAGE_LEFT);
	CG_DrawVehicleDamage(veh,brokenLimbs,menuHUD,alpha,VEH_DAMAGE_RIGHT);
}

qboolean CG_DrawVehicleHud( const centity_t *cent )
{
	itemDef_t		*item;
	menuDef_t		*menuHUD;
	playerState_t	*ps;
	centity_t		*veh;
	float			shieldPerc,alpha;
	
	menuHUD = Menus_FindByName("swoopvehiclehud");
	if (!menuHUD)
	{
		return qtrue;	// Draw player HUD
	}

	ps = &cg.predictedPlayerState;

	if (!ps || !(ps->m_iVehicleNum))
	{
		return qtrue;	// Draw player HUD
	}
	veh = &cg_entities[ps->m_iVehicleNum];

	if ( !veh || !veh->m_pVehicle )
	{
		return qtrue;	// Draw player HUD
	}

	CG_DrawVehicleTurboRecharge( menuHUD, veh );
	CG_DrawVehicleWeaponsLinked( menuHUD, veh );

	item = Menu_FindItemByName(menuHUD, "leftframe");

	// Draw frame
	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}

	item = Menu_FindItemByName(menuHUD, "rightframe");

	if (item)
	{
		trap->R_SetColor( item->window.foreColor );
		CG_DrawPic( 
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h, 
			item->window.background );
	}


	CG_DrawVehicleArmor( menuHUD, veh );

	// Get animal hud for speed
//	if (veh->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
//	{
//		menuHUD = Menus_FindByName("tauntaunhud");
//	}
	

	CG_DrawVehicleSpeed( menuHUD, veh );

	// Revert to swoophud
//	if (veh->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
//	{
//		menuHUD = Menus_FindByName("swoopvehiclehud");
//	}

//	if (veh->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL)
//	{
		shieldPerc = CG_DrawVehicleShields( menuHUD, veh );
//	}

	if (veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID && !veh->m_pVehicle->m_pVehicleInfo->weapon[1].ID)
	{
		CG_DrawVehicleAmmo( menuHUD, veh );
	}
	else if (veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID && veh->m_pVehicle->m_pVehicleInfo->weapon[1].ID)
	{
		CG_DrawVehicleAmmoUpper( menuHUD, veh );
		CG_DrawVehicleAmmoLower( menuHUD, veh );
	}

	// If he's hidden, he must be in a vehicle
	if (veh->m_pVehicle->m_pVehicleInfo->hideRider)
	{
		CG_DrawVehicleDamageHUD(veh,cg.predictedVehicleState.brokenLimbs,shieldPerc,"vehicledamagehud",1.0f);

		// Has he targeted an enemy?
		if (CG_CheckTargetVehicle( &veh, &alpha ))
		{
			CG_DrawVehicleDamageHUD(veh,veh->currentState.brokenLimbs,((float)veh->currentState.generic1/10.0f),"enemyvehicledamagehud",alpha);
		}

		return qfalse;	// Don't draw player HUD
	}

	return qtrue;	// Draw player HUD

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
static void CG_DrawStats( void ) 
{
	centity_t		*cent;
	playerState_t	*ps;
	qboolean		drawHUD = qtrue;
	cent = &cg_entities[cg.snap->ps.clientNum];

	if ( cent )
	{
		ps = &cg.predictedPlayerState;

		if ( (ps->m_iVehicleNum ) )	// In a vehicle???
		{
			drawHUD = CG_DrawVehicleHud( cent );
		}
	}

	if (drawHUD)
	{
		CG_DrawHUD(cent);
	}

}

/*
===================
CG_DrawPickupItem
===================
*/
static void CG_DrawPickupItem( void ) {
	int		value;
	float	*fadeColor;

	value = cg.itemPickup;
	if ( value && cg_items[ value ].icon != -1 ) 
	{
		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if ( fadeColor ) 
		{
			CG_RegisterItemVisuals( value );
			trap->R_SetColor( fadeColor );
			CG_DrawPic( 573, 320, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
			trap->R_SetColor( NULL );
		}
	}
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


	//I have no idea what this is lol - maybe leaderboard thing that shows up in the top right of jka?  --futuza
	//CG_Text_Paint( 630 - CG_Text_Width ( ci->name, 1.0f, FONT_SMALL2 ) + xOffset, y, 1.0f, colorWhite, ci->name, 0, 0, 0, FONT_SMALL2 );	//futuza note: old way
	Text_DrawText(630 - Text_GetWidth(ci->name, FONT_SMALL2, 1.0f) + xOffset, y, ci->name, colorWhite, FONT_SMALL2, 0, 1.0f);				//futuza note: new way

	y += 15;
	//note might want to change this back, title's probably can't use color codes anyway
	//CG_Text_Paint( 630 - CG_Text_Width ( title, 1.0f, FONT_SMALL2 ) + xOffset, y, 1.0f, colorWhite, title, 0, 0, 0, FONT_SMALL2 );			//futuza note: old way
	Text_DrawText(630 - Text_GetWidth(title, FONT_SMALL2, 1.0f) + xOffset, y, title, colorWhite, FONT_SMALL2, 0, 1.0f);							//futuza note: new way

	//ohhh is this dueling only?
	//okay okay
	

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
static int radarLockSoundDebounceTime = 0;
static int impactSoundDebounceTime = 0;
#define	RADAR_MISSILE_RANGE					3000.0f
#define	RADAR_ASTEROID_RANGE				10000.0f
#define	RADAR_MIN_ASTEROID_SURF_WARN_DIST	1200.0f

extern vmCvar_t	d_poff;
extern vmCvar_t	d_roff;
extern vmCvar_t	d_yoff;

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
			if ( (cent->currentState.eFlags & EF_RADAROBJECT)//still want to draw the direction
				|| ( cent->currentState.eType==ET_NPC//FIXME: draw last, with players...
					&& cent->currentState.NPC_class == CLASS_VEHICLE 
					&& cent->currentState.speed > 0 ) )//always draw vehicles
			{ 
				distance = cg_radarRange*0.8f;
			}
			else
			{
				continue;
			}
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
				if ( cent->currentState.NPC_class == CLASS_VEHICLE 
					&& cent->currentState.speed > 0 )
				{
					if ( cent->m_pVehicle && cent->m_pVehicle->m_pVehicleInfo->radarIconHandle )
					{
						float  x;
						float  ly;
			
						x = (float)RADAR_X + (float)RADAR_RADIUS + (float)sin (angle) * distance;
						ly = y2 + (float)RADAR_RADIUS + (float)cos (angle) * distance;

						arrowBaseScale = 9.0f;
						zScale = 1.0f;

						//we want to scale the thing up/down based on the relative Z (up/down) positioning
						if (cent->lerpOrigin[2] > cg.predictedPlayerState.origin[2])
						{ //higher, scale up (between 16 and 24)
							float dif = (cent->lerpOrigin[2] - cg.predictedPlayerState.origin[2]);
							
							//max out to 1.5x scale at 512 units above local player's height
							dif /= 4096.0f;
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
							dif /= 4096.0f;
							if (dif > 0.5f)
							{
								dif = 0.5f;
							}
							zScale -= dif;
						}

						arrowBaseScale *= zScale;

						if ( cent->currentState.m_iVehicleNum //vehicle has a driver
							&& cgs.clientinfo[ cent->currentState.m_iVehicleNum-1 ].infoValid )
						{
							if ( cgs.clientinfo[ cent->currentState.m_iVehicleNum-1 ].team == local->team )
							{
								trap->R_SetColor ( teamColor );
							}
							else
							{
								trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
							}
						}
						else
						{
							trap->R_SetColor ( NULL );
						}
						CG_DrawPic ( x - 4 + xOffset, ly - 4, arrowBaseScale, arrowBaseScale, cent->m_pVehicle->m_pVehicleInfo->radarIconHandle );
					}
				}
				break; //maybe do something?

			case ET_MOVER:
				if ( cent->currentState.speed//the mover's size, actually
					&& actualDist < (cent->currentState.speed+RADAR_ASTEROID_RANGE)
					&& cg.predictedPlayerState.m_iVehicleNum )
				{//a mover that's close to me and I'm in a vehicle
					qboolean mayImpact = qfalse;
					float surfaceDist = (actualDist-cent->currentState.speed);
					if ( surfaceDist < 0.0f )
					{
						surfaceDist = 0.0f;
					}
					if ( surfaceDist < RADAR_MIN_ASTEROID_SURF_WARN_DIST )
					{//always warn!
						mayImpact = qtrue;
					}
					else
					{//not close enough to always warn, yet, so check its direction
						vec3_t	asteroidPos, myPos, moveDir;
						int		predictTime, timeStep = 500;
						float	newDist;
						for ( predictTime = timeStep; predictTime < 5000; predictTime+=timeStep )
						{
							//asteroid dir, speed, size, + my dir & speed...
							BG_EvaluateTrajectory( &cent->currentState.pos, cg.time+predictTime, asteroidPos );
							//FIXME: I don't think it's calcing "myPos" correctly
							AngleVectors( cg.predictedVehicleState.viewangles, moveDir, NULL, NULL );
							VectorMA( cg.predictedVehicleState.origin, cg.predictedVehicleState.speed*predictTime/1000.0f, moveDir, myPos );
							newDist = Distance( myPos, asteroidPos );
							if ( (newDist-cent->currentState.speed) <= RADAR_MIN_ASTEROID_SURF_WARN_DIST )//200.0f )
							{//heading for an impact within the next 5 seconds
								mayImpact = qtrue;
								break;
							}
						}
					}
					if ( mayImpact )
					{//possible collision
						vec4_t	asteroidColor = {0.5f,0.5f,0.5f,1.0f};
						float  x;
						float  ly;
						float asteroidScale = (cent->currentState.speed/2000.0f);//average asteroid radius?
						if ( actualDist > RADAR_ASTEROID_RANGE )
						{
							actualDist = RADAR_ASTEROID_RANGE;
						}
						distance = (actualDist/RADAR_ASTEROID_RANGE)*RADAR_RADIUS;

						x = (float)RADAR_X + (float)RADAR_RADIUS + (float)sin (angle) * distance;
						ly = y2 + (float)RADAR_RADIUS + (float)cos (angle) * distance;
						
						if ( asteroidScale > 3.0f )
						{
							asteroidScale = 3.0f;
						}
						else if ( asteroidScale < 0.2f )
						{
							asteroidScale = 0.2f;
						}
						arrowBaseScale = (9.0f*asteroidScale);
						if ( impactSoundDebounceTime < cg.time )
						{
							vec3_t	soundOrg;
							if ( surfaceDist > RADAR_ASTEROID_RANGE*0.66f )
							{
								impactSoundDebounceTime = cg.time + 1000;
							}
							else if ( surfaceDist > RADAR_ASTEROID_RANGE/3.0f )
							{
								impactSoundDebounceTime = cg.time + 400;
							}
							else 
							{
								impactSoundDebounceTime = cg.time + 100;
							}
							VectorMA( cg.refdef.vieworg, -500.0f*(surfaceDist/RADAR_ASTEROID_RANGE), dirPlayer, soundOrg );
							trap->S_StartSound( soundOrg, ENTITYNUM_WORLD, CHAN_AUTO, trap->S_RegisterSound( "sound/vehicles/common/impactalarm.wav" ) );
						}
						//brighten it the closer it is
						if ( surfaceDist > RADAR_ASTEROID_RANGE*0.66f )
						{
							asteroidColor[0] = asteroidColor[1] = asteroidColor[2] = 0.7f;
						}
						else if ( surfaceDist > RADAR_ASTEROID_RANGE/3.0f )
						{
							asteroidColor[0] = asteroidColor[1] = asteroidColor[2] = 0.85f;
						}
						else 
						{
							asteroidColor[0] = asteroidColor[1] = asteroidColor[2] = 1.0f;
						}
						//alpha out the longer it's been since it was considered dangerous
						if ( (cg.time-impactSoundDebounceTime) > 100 )
						{
							asteroidColor[3] = (float)((cg.time-impactSoundDebounceTime)-100)/900.0f;
						}

						trap->R_SetColor ( asteroidColor );
						CG_DrawPic ( x - 4 + xOffset, ly - 4, arrowBaseScale, arrowBaseScale, trap->R_RegisterShaderNoMip( "gfx/menus/radar/asteroid" ) );
					}
				}
				break;

			case ET_MISSILE:
				if ( //cent->currentState.weapon == WP_ROCKET_LAUNCHER &&//a rocket
					cent->currentState.owner > MAX_CLIENTS //belongs to an NPC
					&& cg_entities[cent->currentState.owner].currentState.NPC_class == CLASS_VEHICLE )
				{//a rocket belonging to an NPC, FIXME: only tracking rockets!
					float  x;
					float  ly;
		
					x = (float)RADAR_X + (float)RADAR_RADIUS + (float)sin (angle) * distance;
					ly = y2 + (float)RADAR_RADIUS + (float)cos (angle) * distance;

					arrowBaseScale = 3.0f;
					if ( cg.predictedPlayerState.m_iVehicleNum )
					{//I'm in a vehicle
						//if it's targetting me, then play an alarm sound if I'm in a vehicle
						if ( cent->currentState.otherEntityNum == cg.predictedPlayerState.clientNum || cent->currentState.otherEntityNum == cg.predictedPlayerState.m_iVehicleNum )
						{
							if ( radarLockSoundDebounceTime < cg.time )
							{
								vec3_t	soundOrg;
								int		alarmSound;
								if ( actualDist > RADAR_MISSILE_RANGE * 0.66f )
								{
									radarLockSoundDebounceTime = cg.time + 1000;
									arrowBaseScale = 3.0f;
									alarmSound = trap->S_RegisterSound( "sound/vehicles/common/lockalarm1.wav" );
								}
								else if ( actualDist > RADAR_MISSILE_RANGE/3.0f )
								{
									radarLockSoundDebounceTime = cg.time + 500;
									arrowBaseScale = 6.0f;
									alarmSound = trap->S_RegisterSound( "sound/vehicles/common/lockalarm2.wav" );
								}
								else
								{
									radarLockSoundDebounceTime = cg.time + 250;
									arrowBaseScale = 9.0f;
									alarmSound = trap->S_RegisterSound( "sound/vehicles/common/lockalarm3.wav" );
								}
								if ( actualDist > RADAR_MISSILE_RANGE )
								{
									actualDist = RADAR_MISSILE_RANGE;
								}
								VectorMA( cg.refdef.vieworg, -500.0f*(actualDist/RADAR_MISSILE_RANGE), dirPlayer, soundOrg );
								trap->S_StartSound( soundOrg, ENTITYNUM_WORLD, CHAN_AUTO, alarmSound );
							}
						}
					}
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

					if ( cent->currentState.owner >= MAX_CLIENTS//missile owned by an NPC
						&& cg_entities[cent->currentState.owner].currentState.NPC_class == CLASS_VEHICLE//NPC is a vehicle
						&& cg_entities[cent->currentState.owner].currentState.m_iVehicleNum <= MAX_CLIENTS//Vehicle has a player driver
						&& cgs.clientinfo[cg_entities[cent->currentState.owner].currentState.m_iVehicleNum-1].infoValid ) //player driver is valid
					{
						cl = &cgs.clientinfo[cg_entities[cent->currentState.owner].currentState.m_iVehicleNum-1];
						if ( cl->team == local->team )
						{
							trap->R_SetColor ( teamColor );
						}
						else
						{
							trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
						}
					}
					else
					{
						trap->R_SetColor ( NULL );
					}
					CG_DrawPic ( x - 4 + xOffset, ly - 4, arrowBaseScale, arrowBaseScale, cgs.media.mAutomapRocketIcon );
				}
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
			trap->R_Font_DrawString(x+26, y+2, ci->name, hcolor, MenuFontToHandle(1) | 0x80000000, -1, 0.5f);		//old way
			//Text_DrawText(x + 26, y + 2, ci->name, hcolor, MenuFontToHandle(1) | 0x80000000, -1, 0.5f);		// futuza note: xRGB fix (see tr_font.cpp) should allow us to use old way, not necessary?
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

	if ( ( cgs.gametype >= GT_TEAM || cg.predictedPlayerState.m_iVehicleNum )
		&& cg_drawRadar.integer )
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

#define	LAG_SAMPLES		128


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

extern vmCvar_t jkg_gunlesscrosshair;
vec3_t cg_crosshairPos={0,0,0};
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

	if ( worldPoint )
	{
		VectorCopy( worldPoint, cg_crosshairPos );
	}

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
						if (!crossEnt->currentState.teamowner ||
							crossEnt->currentState.NPC_class == CLASS_VEHICLE)
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

	if ( cg.predictedPlayerState.m_iVehicleNum )
	{//I'm in a vehicle
		centity_t *vehCent = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
	    if ( vehCent 
			&& vehCent->m_pVehicle 
			&& vehCent->m_pVehicle->m_pVehicleInfo 
			&& vehCent->m_pVehicle->m_pVehicleInfo->crosshairShaderHandle )
		{
			hShader = vehCent->m_pVehicle->m_pVehicleInfo->crosshairShaderHandle;
		}
		//bigger by default
		w = cg_crosshairSize.value*2.0f;
		h = w;
	}
	else
	{
		w = h = cg_crosshairSize.value;
	}

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
	else if (crossEnt && crossEnt->currentState.number < MAX_CLIENTS)
	{
		if (cg.crosshairVehNum && cg.time == cg.crosshairVehTime)
		{ //it was in the crosshair this frame
			centity_t *hisVeh = &cg_entities[cg.crosshairVehNum];

			if (hisVeh->currentState.eType == ET_NPC &&
				hisVeh->currentState.NPC_class == CLASS_VEHICLE &&
				hisVeh->currentState.maxhealth &&
				hisVeh->m_pVehicle)
			{ //draw the health for this vehicle
				CG_DrawHealthBar(hisVeh, chX, chY, w, h);
				chY += HEALTH_HEIGHT*2;
			}
		}
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
	vec3_t color;
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

	VectorSet( color, 0.8f, 0.8f, 0.8f );
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

void CG_BracketEntity( centity_t *cent, float radius )
{
	trace_t tr;
	vec3_t dif;
	float	len, size, lineLength, lineWidth;
	float	x,	y;
	clientInfo_t *local;
	qboolean isEnemy = qfalse;

	VectorSubtract( cent->lerpOrigin, cg.refdef.vieworg, dif );
	len = VectorNormalize( dif );

	if ( cg.crosshairClientNum != cent->currentState.clientNum
		&& (!cg.snap||cg.snap->ps.rocketLockIndex!= cent->currentState.clientNum) )
	{//if they're the entity you're locking onto or under your crosshair, always draw bracket
		//Hmm... for now, if they're closer than 2000, don't bracket?
		if ( len < 2000.0f )
		{
			return;
		}

		CG_Trace( &tr, cg.refdef.vieworg, NULL, NULL, cent->lerpOrigin, -1, CONTENTS_OPAQUE );

		//don't bracket if can't see them
		if ( tr.fraction < 1.0f )
		{
			return;
		}
	}

	if ( !CG_WorldCoordToScreenCoordFloat(cent->lerpOrigin, &x, &y) )
	{//off-screen, don't draw it
		return;
	}

	//just to see if it's centered
	//CG_DrawPic( x-2, y-2, 4, 4, cgs.media.whiteShader );

	local = &cgs.clientinfo[cg.snap->ps.clientNum];
	if ( cent->currentState.m_iVehicleNum //vehicle has a driver
		&& (cent->currentState.m_iVehicleNum-1) < MAX_CLIENTS
		&& cgs.clientinfo[ cent->currentState.m_iVehicleNum-1 ].infoValid )
	{
		if ( cgs.gametype < GT_TEAM )
		{//ffa?
			isEnemy = qtrue;
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
		}
		else if ( cgs.clientinfo[ cent->currentState.m_iVehicleNum-1 ].team == local->team )
		{
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_GREEN)] );
		}
		else
		{
			isEnemy = qtrue;
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
		}
	}
	else if ( cent->currentState.teamowner )
	{
		if ( cgs.gametype < GT_TEAM )
		{//ffa?
			isEnemy = qtrue;
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
		}
		else if ( cent->currentState.teamowner != cg.predictedPlayerState.persistant[PERS_TEAM] )
		{// on enemy team
			isEnemy = qtrue;
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
		}
		else
		{ //a friend
			trap->R_SetColor ( g_color_table[ColorIndex(COLOR_GREEN)] );
		}
	}
	else
	{//FIXME: if we want to ever bracket anything besides vehicles (like siege objectives we want to blow up), we should handle the coloring here
		trap->R_SetColor ( NULL );
	}
	
	if ( len <= 1.0f )
	{//super-close, max out at 400 times radius (which is HUGE)
		size = radius*400.0f;
	}
	else
	{//scale by dist
		size = radius*(400.0f/len);
	}

	if ( size < 1.0f )
	{
		size = 1.0f;
	}
	
	//length scales with dist
	lineLength = (size*0.1f);
	if ( lineLength < 0.5f )
	{//always visible
		lineLength = 0.5f;
	}
	//always visible width
	lineWidth = 1.0f;

	x -= (size*0.5f);
	y -= (size*0.5f);

	/*
	if ( x >= 0 && x <= 640
		&& y >= 0 && y <= 480 )
	*/
	{//brackets would be drawn on the screen, so draw them
	//upper left corner
		//horz
        CG_DrawPic( x, y, lineLength, lineWidth, cgs.media.whiteShader );
		//vert
        CG_DrawPic( x, y, lineWidth, lineLength, cgs.media.whiteShader );
	//upper right corner
		//horz
        CG_DrawPic( x+size-lineLength, y, lineLength, lineWidth, cgs.media.whiteShader );
		//vert
        CG_DrawPic( x+size-lineWidth, y, lineWidth, lineLength, cgs.media.whiteShader );
	//lower left corner
		//horz
        CG_DrawPic( x, y+size-lineWidth, lineLength, lineWidth, cgs.media.whiteShader );
		//vert
        CG_DrawPic( x, y+size-lineLength, lineWidth, lineLength, cgs.media.whiteShader );
	//lower right corner
		//horz
        CG_DrawPic( x+size-lineLength, y+size-lineWidth, lineLength, lineWidth, cgs.media.whiteShader );
		//vert
        CG_DrawPic( x+size-lineWidth, y+size-lineLength, lineWidth, lineLength, cgs.media.whiteShader );
	}
	//Lead Indicator...
	if ( cg_drawVehLeadIndicator.integer )
	{//draw the lead indicator
		if ( isEnemy )
		{//an enemy object
			if ( cent->currentState.NPC_class == CLASS_VEHICLE )
			{//enemy vehicle
				if ( !VectorCompare( cent->currentState.pos.trDelta, vec3_origin ) )
				{//enemy vehicle is moving
					if ( cg.predictedPlayerState.m_iVehicleNum )
					{//I'm in a vehicle
						centity_t		*veh = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
						if ( veh //vehicle cent
							&& veh->m_pVehicle//vehicle
							&& veh->m_pVehicle->m_pVehicleInfo//vehicle stats
							&& veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID > VEH_WEAPON_BASE )//valid vehicle weapon
						{
							vehWeaponInfo_t *vehWeapon = &g_vehWeaponInfo[veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID];
							if ( vehWeapon 
								&& vehWeapon->bIsProjectile//primary weapon's shot is a projectile
								&& !vehWeapon->bHasGravity//primary weapon's shot is not affected by gravity
								&& !vehWeapon->fHoming//primary weapon's shot is not homing
								&& vehWeapon->fSpeed )//primary weapon's shot has speed
							{//our primary weapon's projectile has a speed
								vec3_t vehDiff, vehLeadPos;
								float vehDist, eta;
								float leadX, leadY;
								
								VectorSubtract( cent->lerpOrigin, cg.predictedVehicleState.origin, vehDiff );
								vehDist = VectorNormalize( vehDiff );
								eta = (vehDist/vehWeapon->fSpeed);//how many seconds it would take for my primary weapon's projectile to get from my ship to theirs
								//now extrapolate their position that number of seconds into the future based on their velocity
								VectorMA( cent->lerpOrigin, eta, cent->currentState.pos.trDelta, vehLeadPos );
								//now we have where we should be aiming at, project that onto the screen at a 2D co-ord
								if ( !CG_WorldCoordToScreenCoordFloat(cent->lerpOrigin, &x, &y) )
								{//off-screen, don't draw it
									return;
								}
								if ( !CG_WorldCoordToScreenCoordFloat(vehLeadPos, &leadX, &leadY) )
								{//off-screen, don't draw it
									//just draw the line
									CG_DottedLine( x, y, leadX, leadY, 1, 10, g_color_table[ColorIndex(COLOR_RED)], 0.5f );
									return;
								}
								//draw a line from the ship's cur pos to the lead pos
								CG_DottedLine( x, y, leadX, leadY, 1, 10, g_color_table[ColorIndex(COLOR_RED)], 0.5f );
								//now draw the lead indicator
								trap->R_SetColor ( g_color_table[ColorIndex(COLOR_RED)] );
								CG_DrawPic( leadX-8, leadY-8, 16, 16, trap->R_RegisterShader( "gfx/menus/radar/lead" ) );
							}
						}
					}
				}
			}
		}
	}
}

qboolean CG_InFighter( void )
{
	if ( cg.predictedPlayerState.m_iVehicleNum )
	{//I'm in a vehicle
		centity_t *vehCent = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
	    if ( vehCent 
			&& vehCent->m_pVehicle 
			&& vehCent->m_pVehicle->m_pVehicleInfo
			&& vehCent->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER )
		{//I'm in a fighter
			return qtrue;
		}
	}
	return qfalse;
}

qboolean CG_InATST( void )
{
	if ( cg.predictedPlayerState.m_iVehicleNum )
	{//I'm in a vehicle
		centity_t *vehCent = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
	    if ( vehCent 
			&& vehCent->m_pVehicle 
			&& vehCent->m_pVehicle->m_pVehicleInfo
			&& vehCent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
		{//I'm in an atst
			return qtrue;
		}
	}
	return qfalse;
}

void CG_DrawBracketedEntities( void )
{
	int i;
	for ( i = 0; i < cg.bracketedEntityCount; i++ ) 
	{	
		centity_t *cent = &cg_entities[cg.bracketedEntities[i]];
		CG_BracketEntity( cent, CG_RadiusForCent( cent ) );
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

//--------------------------------------------------------------
static void CG_DrawRocketLocking( int lockEntNum, int lockTime )
//--------------------------------------------------------------
{
	int		cx, cy;
	vec3_t	org;
	static	int oldDif = 0;
	centity_t *cent = &cg_entities[lockEntNum];
	vec4_t color={0.0f,0.0f,0.0f,0.0f};
	float lockTimeInterval = 1200.0f/16.0f;
	//FIXME: if in a vehicle, use the vehicle's lockOnTime...
	int dif = (cg.time - cg.snap->ps.rocketLockTime)/lockTimeInterval;
	int i;

	if (!cg.snap->ps.rocketLockTime)
	{
		return;
	}

	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		return;
	}

	if ( cg.snap->ps.m_iVehicleNum )
	{//driving a vehicle
		centity_t *veh = &cg_entities[cg.snap->ps.m_iVehicleNum];
		if ( veh->m_pVehicle )
		{
			vehWeaponInfo_t *vehWeapon = NULL;
			if ( cg.predictedVehicleState.weaponstate == WEAPON_CHARGING_ALT )
			{
				if ( veh->m_pVehicle->m_pVehicleInfo->weapon[1].ID > VEH_WEAPON_BASE 
					&& veh->m_pVehicle->m_pVehicleInfo->weapon[1].ID < MAX_VEH_WEAPONS )
				{
					vehWeapon = &g_vehWeaponInfo[veh->m_pVehicle->m_pVehicleInfo->weapon[1].ID];
				}
			}
			else
			{
				if ( veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID > VEH_WEAPON_BASE 
					&& veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID < MAX_VEH_WEAPONS )
				{
					vehWeapon = &g_vehWeaponInfo[veh->m_pVehicle->m_pVehicleInfo->weapon[0].ID];
				}
			}
			if ( vehWeapon != NULL )
			{//we are trying to lock on with a valid vehicle weapon, so use *its* locktime, not the hard-coded one
				if ( !vehWeapon->iLockOnTime )
				{//instant lock-on
					dif = 10.0f;
				}
				else
				{//use the custom vehicle lockOnTime
					lockTimeInterval = (vehWeapon->iLockOnTime/16.0f);
					dif = (cg.time - cg.snap->ps.rocketLockTime)/lockTimeInterval;
				}
			}
		}
	}
	//We can't check to see in pmove if players are on the same team, so we resort
	//to just not drawing the lock if a teammate is the locked on ent
	if (cg.snap->ps.rocketLockIndex >= 0 &&
		cg.snap->ps.rocketLockIndex < ENTITYNUM_NONE)
	{
		clientInfo_t *ci = NULL;

		if (cg.snap->ps.rocketLockIndex < MAX_CLIENTS)
		{
			ci = &cgs.clientinfo[cg.snap->ps.rocketLockIndex];
		}
		else
		{
			ci = cg_entities[cg.snap->ps.rocketLockIndex].npcClient;
		}

		if (ci)
		{
			if (ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team)
			{
				if (cgs.gametype >= GT_TEAM)
				{
					return;
				}
			}
			else if (cgs.gametype >= GT_TEAM)
			{
				centity_t *hitEnt = &cg_entities[cg.snap->ps.rocketLockIndex];
				if (hitEnt->currentState.eType == ET_NPC &&
					hitEnt->currentState.NPC_class == CLASS_VEHICLE &&
					hitEnt->currentState.owner < ENTITYNUM_WORLD)
				{ //this is a vehicle, if it has a pilot and that pilot is on my team, then...
					if (hitEnt->currentState.owner < MAX_CLIENTS)
					{
						ci = &cgs.clientinfo[hitEnt->currentState.owner];
					}
					else
					{
						ci = cg_entities[hitEnt->currentState.owner].npcClient;
					}
					if (ci && ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team)
					{
						return;
					}
				}
			}
		}
	}

	if (cg.snap->ps.rocketLockTime != -1)
	{
		lastvalidlockdif = dif;
	}
	else
	{
		dif = lastvalidlockdif;
	}

	if ( !cent )
	{
		return;
	}

	VectorCopy( cent->lerpOrigin, org );

	if ( CG_WorldCoordToScreenCoord( org, &cx, &cy ))
	{
		// we care about distance from enemy to eye, so this is good enough
		float sz = Distance( cent->lerpOrigin, cg.refdef.vieworg ) / 1024.0f; 
		
		if ( sz > 1.0f )
		{
			sz = 1.0f;
		}
		else if ( sz < 0.0f )
		{
			sz = 0.0f;
		}

		sz = (1.0f - sz) * (1.0f - sz) * 32 + 6;

		cy += sz * 0.5f;
		
		if ( dif < 0 )
		{
			oldDif = 0;
			return;
		}
		else if ( dif > 8 )
		{
			dif = 8;
		}

		// do sounds
		if ( oldDif != dif )
		{
			if ( dif == 8 )
			{
				if ( cg.snap->ps.m_iVehicleNum )
				{
					trap->S_StartSound( org, 0, CHAN_AUTO, trap->S_RegisterSound( "sound/vehicles/weapons/common/lock.wav" ));
				}
				else
				{
					trap->S_StartSound( org, 0, CHAN_AUTO, trap->S_RegisterSound( "sound/weapons/rocket/lock.wav" ));
				}
			}
			else
			{
				if ( cg.snap->ps.m_iVehicleNum )
				{
					trap->S_StartSound( org, 0, CHAN_AUTO, trap->S_RegisterSound( "sound/vehicles/weapons/common/tick.wav" ));
				}
				else
				{
					trap->S_StartSound( org, 0, CHAN_AUTO, trap->S_RegisterSound( "sound/weapons/rocket/tick.wav" ));
				}
			}
		}

		oldDif = dif;

		for ( i = 0; i < dif; i++ )
		{
			color[0] = 1.0f;
			color[1] = 0.0f;
			color[2] = 0.0f;
			color[3] = 0.1f * i + 0.2f;

			trap->R_SetColor( color );

			// our slices are offset by about 45 degrees.
			CG_DrawRotatePic( cx - sz, cy - sz, sz, sz, i * 45.0f, trap->R_RegisterShaderNoMip( "gfx/2d/wedge" ));
		}

		// we are locked and loaded baby
		if ( dif == 8 )
		{
			color[0] = color[1] = color[2] = sin( cg.time * 0.05f ) * 0.5f + 0.5f;
			color[3] = 1.0f; // this art is additive, so the alpha value does nothing

			trap->R_SetColor( color );

			CG_DrawPic( cx - sz, cy - sz * 2, sz * 2, sz * 2, trap->R_RegisterShaderNoMip( "gfx/2d/lock" ));
		}
	}
}

extern void CG_CalcVehMuzzle(Vehicle_t *pVeh, centity_t *ent, int muzzleNum);
qboolean CG_CalcVehicleMuzzlePoint( int entityNum, vec3_t start, vec3_t d_f, vec3_t d_rt, vec3_t d_up)
{
	centity_t *vehCent = &cg_entities[entityNum];
	if ( vehCent->m_pVehicle && vehCent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
	{//draw from barrels
		VectorCopy( vehCent->lerpOrigin, start );
		start[2] += vehCent->m_pVehicle->m_pVehicleInfo->height-DEFAULT_MINS_2-48;
		AngleVectors( vehCent->lerpAngles, d_f, d_rt, d_up );
	}
	else
	{
		//check to see if we're a turret gunner on this vehicle
		if ( cg.predictedPlayerState.generic1 )//as a passenger
		{//passenger in a vehicle
			if ( vehCent->m_pVehicle
				&& vehCent->m_pVehicle->m_pVehicleInfo 
				&& vehCent->m_pVehicle->m_pVehicleInfo->maxPassengers )
			{//a vehicle capable of carrying passengers
				int turretNum;
				for ( turretNum = 0; turretNum < MAX_VEHICLE_TURRETS; turretNum++ )
				{
					if ( vehCent->m_pVehicle->m_pVehicleInfo->turret[turretNum].iAmmoMax )
					{// valid turret
						if ( vehCent->m_pVehicle->m_pVehicleInfo->turret[turretNum].passengerNum == cg.predictedPlayerState.generic1 )
						{//I control this turret
							//Go through all muzzles, average their positions and directions and use the result for crosshair trace
							int vehMuzzle, numMuzzles = 0;
							vec3_t	muzzlesAvgPos={0},muzzlesAvgDir={0};
							int	i;

							for ( i = 0; i < MAX_VEHICLE_TURRET_MUZZLES; i++ )
							{
								vehMuzzle = vehCent->m_pVehicle->m_pVehicleInfo->turret[turretNum].iMuzzle[i];
								if ( vehMuzzle )
								{
									vehMuzzle -= 1;
									CG_CalcVehMuzzle( vehCent->m_pVehicle, vehCent, vehMuzzle );
									VectorAdd( muzzlesAvgPos, vehCent->m_pVehicle->m_vMuzzlePos[vehMuzzle], muzzlesAvgPos );
									VectorAdd( muzzlesAvgDir, vehCent->m_pVehicle->m_vMuzzleDir[vehMuzzle], muzzlesAvgDir );
									numMuzzles++;
								}
								if ( numMuzzles )
								{
									VectorScale( muzzlesAvgPos, 1.0f/(float)numMuzzles, start );
									VectorScale( muzzlesAvgDir, 1.0f/(float)numMuzzles, d_f );
									VectorClear( d_rt );
									VectorClear( d_up );
									return qtrue;
								}
							}
						}
					}
				}
			}
		}
		VectorCopy( vehCent->lerpOrigin, start );
		AngleVectors( vehCent->lerpAngles, d_f, d_rt, d_up );
	}
	return qfalse;
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
	qboolean	bVehCheckTraceFromCamPos = qfalse;

	ignore = cg.predictedPlayerState.clientNum;

	if ( cg_dynamicCrosshair.integer )
	{
		vec3_t d_f, d_rt, d_up;
		//For now we still want to draw the crosshair in relation to the player's world coordinates
		//even if we have a melee weapon/no weapon.
		if ( cg.predictedPlayerState.m_iVehicleNum && (cg.predictedPlayerState.eFlags&EF_NODRAW) )
		{//we're *inside* a vehicle
			//do the vehicle's crosshair instead
			centity_t *veh = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
			qboolean gunner = qfalse;

			//if (veh->currentState.owner == cg.predictedPlayerState.clientNum)
			{ //the pilot
				ignore = cg.predictedPlayerState.m_iVehicleNum;
				gunner = CG_CalcVehicleMuzzlePoint(cg.predictedPlayerState.m_iVehicleNum, start, d_f, d_rt, d_up);
			}
			if ( veh->m_pVehicle 
				&& veh->m_pVehicle->m_pVehicleInfo 
				&& veh->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER 
				&& cg.distanceCull > MAX_XHAIR_DIST_ACCURACY 
				&& !gunner )
			{	
				//NOTE: on huge maps, the crosshair gets inaccurate at close range, 
				//		so we'll do an extra G2 trace from the cg.refdef.vieworg
				//		to see if we hit anything closer and auto-aim at it if so
				bVehCheckTraceFromCamPos = qtrue;
			}
		}
		else if (cg.snap && cg.snap->ps.weapon == WP_EMPLACED_GUN && cg.snap->ps.emplacedIndex &&
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
		if ( bVehCheckTraceFromCamPos )
		{
			//NOTE: this MUST stay up to date with the method used in WP_VehCheckTraceFromCamPos
			centity_t *veh = &cg_entities[cg.predictedPlayerState.m_iVehicleNum];
			trace_t	extraTrace;
			vec3_t	viewDir2End, extraEnd;
			float	minAutoAimDist = Distance( veh->lerpOrigin, cg.refdef.vieworg ) + (veh->m_pVehicle->m_pVehicleInfo->length/2.0f) + 200.0f;

			VectorSubtract( end, cg.refdef.vieworg, viewDir2End );
			VectorNormalize( viewDir2End );
			VectorMA( cg.refdef.vieworg, MAX_XHAIR_DIST_ACCURACY, viewDir2End, extraEnd );
			CG_G2Trace( &extraTrace, cg.refdef.vieworg, vec3_origin, vec3_origin, extraEnd, 
				ignore, CONTENTS_SOLID|CONTENTS_BODY );
			if ( !extraTrace.allsolid
				&& !extraTrace.startsolid )
			{
				if ( extraTrace.fraction < 1.0f )
				{
					if ( (extraTrace.fraction*MAX_XHAIR_DIST_ACCURACY) > minAutoAimDist )
					{
						if ( ((extraTrace.fraction*MAX_XHAIR_DIST_ACCURACY)-Distance( veh->lerpOrigin, cg.refdef.vieworg )) < (trace.fraction*cg.distanceCull) )
						{//this trace hit *something* that's closer than the thing the main trace hit, so use this result instead
							memcpy( &trace, &extraTrace, sizeof( trace_t ) );
						}
					}
				}
			}
		}
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
		if (trace.entityNum < /*MAX_CLIENTS*/ENTITYNUM_WORLD)
		{
			cg.crosshairClientNum = trace.entityNum;
			cg.crosshairClientTime = cg.time;

			if (cg.crosshairClientNum < ENTITYNUM_WORLD)
			{
				centity_t *veh = &cg_entities[cg.crosshairClientNum];

				if (veh->currentState.eType == ET_NPC &&
					veh->currentState.NPC_class == CLASS_VEHICLE &&
					veh->currentState.owner < MAX_CLIENTS)
				{ //draw the name of the pilot then
					cg.crosshairClientNum = veh->currentState.owner;
					cg.crosshairVehNum = veh->currentState.number;
					cg.crosshairVehTime = cg.time;
				}
			}

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

extern vmCvar_t	d_poff;
extern vmCvar_t	d_roff;
#define clamp(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/*
=====================================

NPC NAMEPLATES

=====================================
*/

qboolean HumanNamesLoaded = qfalse;

// JKGFIXME: yea, this is pretty obvious that this was jacked from somewhere, just have no clue where from --eez
typedef struct
{// FIXME: Add other species name files...
	char	HumanNames[MAX_QPATH];
} name_list_t;

name_list_t NPC_NAME_LIST[8000];

int NUM_HUMAN_NAMES = 0;

void Load_NPC_Names ( void )
{				// Load bot first names from external file.
	char			*s, *t;
	int				len;
	fileHandle_t	f;
	char			*buf;
	char			*loadPath;
	int				num = 0;

	if (HumanNamesLoaded)
		return;

	loadPath = va( "npc_names_list.dat" );

	len = trap->FS_Open( loadPath, &f, FS_READ );

	HumanNamesLoaded = qtrue;

	if ( !f )
	{
		return;
	}

	if ( !len )
	{			//empty file
		trap->FS_Close( f );
		return;
	}

	if ( (buf = (char *)malloc( len + 1)) == 0 )
	{			//alloc memory for buffer
		trap->FS_Close( f );
		return;
	}

	trap->FS_Read( buf, len, f );
	buf[len] = 0;
	trap->FS_Close( f );

	strcpy( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, "NONAME");
	NUM_HUMAN_NAMES++;
	strcpy( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, "R2D2 Droid");
	NUM_HUMAN_NAMES++;
	strcpy( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, "R5D2 Droid");
	NUM_HUMAN_NAMES++;
	strcpy( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, "Protocol Droid");
	NUM_HUMAN_NAMES++;
	strcpy( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, "Weequay");
	NUM_HUMAN_NAMES++;

	for ( t = s = buf; *t; /* */ )
	{
		num++;
		s = strchr( s, '\n' );
		if ( !s || num > len )
		{
			break;
		}

		while ( *s == '\n' )
		{
			*s++ = 0;
		}

		if ( *t )
		{
			if ( !Q_strncmp( "//", va( "%s", t), 2) == 0 && strlen( va( "%s", t)) > 0 )
			{	// Not a comment either... Record it in our list...
				Q_strncpyz( NPC_NAME_LIST[NUM_HUMAN_NAMES].HumanNames, va( "%s", t), strlen( va( "%s", t)) );
				NUM_HUMAN_NAMES++;
			}
		}

		t = s;
	}

	NUM_HUMAN_NAMES--;
}

void CG_DrawNPCNames( void )
{// Float a NPC name above their head!
	// FIXME: localize strings within .str files --eez
	int				i;

	// Load the list on first check...
	Load_NPC_Names();

	for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
	{// Cycle through them...
		vec3_t			origin;
		centity_t		*cent = &cg_entities[i];
		char			*str1, *str2;
		int				w, w2;
		float			size, x, y, x2, y2, dist;
		vec4_t			tclr =	{ 0.325f,	0.325f,	1.0f,	1.0f	};
		vec4_t			tclr2 = { 0.325f,	0.325f,	1.0f,	1.0f	};
		int				baseColor = CT_BLUE;
		float			multiplier = 1.0f;

		if (!cent)
			continue;

		if (cent->currentState.eType != ET_NPC)
			continue;

		if (!cent->ghoul2)
			continue;

		if (!cent->npcClient)
			continue;

		if (cent->cloaked)
			continue;

		switch( cent->currentState.NPC_class )
		{// UQ1: Supported Class Types...
		case CLASS_CIVILIAN:
			str2 = va("< Civilian >");
			tclr[0] = 0.125f;
			tclr[1] = 0.125f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_REBEL:
			str2 = va("< Rebel >");
			tclr[0] = 0.125f;
			tclr[1] = 0.125f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_JEDI:
		case CLASS_KYLE:
		case CLASS_LUKE:
		case CLASS_JAN:
		case CLASS_MONMOTHA:			
		case CLASS_MORGANKATARN:
			str2 = va("< Jedi >");
			tclr[0] = 0.125f;
			tclr[1] = 0.325f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.325f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_GENERAL_VENDOR:
			str2 = va("< General Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_WEAPONS_VENDOR:
			str2 = va("< Weapons Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_ARMOR_VENDOR:
			str2 = va("< Armor Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_SUPPLIES_VENDOR:
			str2 = va("< Supplies Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_FOOD_VENDOR:
			str2 = va("< Food Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_MEDICAL_VENDOR:
			str2 = va("< Medical Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_GAMBLER_VENDOR:
			str2 = va("< Gambling Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_TRADE_VENDOR:
			str2 = va("< Trade Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_ODDITIES_VENDOR:
			str2 = va("< Oddities Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_DRUG_VENDOR:
			str2 = va("< Drug Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_TRAVELLING_VENDOR:
			str2 = va("< Travelling Vendor >");
			tclr[0] = 0.525f;
			tclr[1] = 0.525f;
			tclr[2] = 1.0f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.525f;
			tclr2[1] = 0.525f;
			tclr2[2] = 1.0f;
			tclr2[3] = 1.0f;
			break;
			//Stoiss add: Faq coler names for npcs in this class
			case CLASS_JKG_FAQ_IMP_DROID:
				str2 = va("< Imperial FAQ Droid >");//red coler
			tclr[0] = 1.0f;
			tclr[1] = 0.125f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_JKG_FAQ_ALLIANCE_DROID:
				str2 = va("< Alliance FAQ Droid >");//Dark Blue coler
			tclr[0] = 0.125f;
			tclr[1] = 0.125f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_JKG_FAQ_SPY_DROID:
				str2 = va("< Faq Spy Droid >");//Yellow colder
			tclr[0] = 0.7f;
			tclr[1] = 0.7f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.7f;
			tclr2[1] = 0.7f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;

			case CLASS_JKG_FAQ_CRAFTER_DROID:
				str2 = va("< Master Crafter >");//Yellow colder
			tclr[0] = 0.7f;
			tclr[1] = 0.7f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.7f;
			tclr2[1] = 0.7f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_JKG_FAQ_MERC_DROID:
				str2 = va("< Merc FAQ Droid >");//Yellow colder. fixme: need a better coler for mercs
			tclr[0] = 1.0f;
			tclr[1] = 0.225f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.225f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_JKG_FAQ_JEDI_MENTOR:
				str2 = va("< Jedi Mentor >");// Blue coler
			tclr[0] = 0.125f;
			tclr[1] = 0.325f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.325f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_JKG_FAQ_SITH_MENTOR:
				str2 = va("< Sith Mentor >");//oriange coler
			tclr[0] = 1.0f;
			tclr[1] = 0.225f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.225f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;//Stoiss end
		case CLASS_STORMTROOPER:
		case CLASS_SWAMPTROOPER:
		case CLASS_IMPWORKER:
		case CLASS_IMPERIAL:
		case CLASS_SHADOWTROOPER:
		case CLASS_COMMANDO:
			str2 = va("< Imperial >");
			tclr[0] = 1.0f;
			tclr[1] = 0.125f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_MERC://Stoiss add merc class
			str2 = va("< Merc >");
			tclr[0] = 1.0f;
			tclr[1] = 0.125f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_TAVION:
		case CLASS_DESANN:
			str2 = va("< Sith Boss>");
			tclr[0] = 1.0f;
			tclr[1] = 0.325f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.325f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_REBORN:
			str2 = va("< Sith >");
			tclr[0] = 1.0f;
			tclr[1] = 0.325f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.325f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
			case CLASS_REBORN_CULTIST:
			str2 = va("< Sith Fighters >");
			tclr[0] = 1.0f;
			tclr[1] = 0.325f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.325f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_BOBAFETT:
			str2 = va("< Bounty Hunter >");
			tclr[0] = 1.0f;
			tclr[1] = 0.225f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.225f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_ATST:
			str2 = va("< Vehicle >");
			tclr[0] = 1.0f;
			tclr[1] = 0.225f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.225f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
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
			str2 = va("< Animal >");
			tclr[0] = 1.0f;
			tclr[1] = 0.125f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_VEHICLE:
			str2 = va("< Vehicle >");
			tclr[0] = 1.0f;
			tclr[1] = 0.125f;
			tclr[2] = 0.125f;
			tclr[3] = 1.0f;

			tclr2[0] = 1.0f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.125f;
			tclr2[3] = 1.0f;
			break;
		case CLASS_BESPIN_COP:
		case CLASS_LANDO:
		case CLASS_PRISONER:
			str2 = va("< Rebel >");
			tclr[0] = 0.125f;
			tclr[1] = 0.125f;
			tclr[2] = 0.7f;
			tclr[3] = 1.0f;

			tclr2[0] = 0.125f;
			tclr2[1] = 0.125f;
			tclr2[2] = 0.7f;
			tclr2[3] = 1.0f;
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
				str2 = va("< Thug >");
				tclr[0] = 0.5f;
				tclr[1] = 0.5f;
				tclr[2] = 0.125f;
				tclr[3] = 1.0f;

				tclr2[0] = 0.5f;
				tclr2[1] = 0.5f;
				tclr2[2] = 0.125f;
				tclr2[3] = 1.0f;
			}
			else if (cent->playerState->persistant[PERS_TEAM] == NPCTEAM_PLAYER)
			{
				str2 = va("< Rebel >");
				tclr[0] = 0.125f;
				tclr[1] = 0.125f;
				tclr[2] = 0.7f;
				tclr[3] = 1.0f;

				tclr2[0] = 0.125f;
				tclr2[1] = 0.125f;
				tclr2[2] = 0.7f;
				tclr2[3] = 1.0f;
			}
			else
			{
				str2 = va("< Civilian >");
				tclr[0] = 0.7f;
				tclr[1] = 0.7f;
				tclr[2] = 0.125f;
				tclr[3] = 1.0f;

				tclr2[0] = 0.7f;
				tclr2[1] = 0.7f;
				tclr2[2] = 0.125f;
				tclr2[3] = 1.0f;
			}
			break;
		
		default:
			//CG_Printf("NPC %i is not a civilian or vendor (class %i).\n", cent->currentState.number, cent->currentState.NPC_class);
			continue; // Unsupported...
			break;
		}

		if (cent->currentState.generic1 > 0)
		{// Was assigned a full name already! Yay!
			switch( cent->currentState.NPC_class )
			{// UQ1: Supported Class Types...
			case CLASS_CIVILIAN:
			case CLASS_GENERAL_VENDOR:
			case CLASS_WEAPONS_VENDOR:
			case CLASS_ARMOR_VENDOR:
			case CLASS_SUPPLIES_VENDOR:
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
			case CLASS_UGNAUGHT:
			case CLASS_JAWA:
				str1 = va("%s", NPC_NAME_LIST[cent->currentState.generic1].HumanNames);
				break;
			case CLASS_STORMTROOPER:
			case CLASS_SWAMPTROOPER:
			case CLASS_IMPWORKER:
			case CLASS_IMPERIAL:
			case CLASS_SHADOWTROOPER:
				str1 = va("TK-%i", cent->currentState.generic1);	// EVIL. for a number of reasons --eez
				break;
			case CLASS_ATST:				// technically droid...
				str1 = va("AT-ST");
				break;
			case CLASS_CLAW:
				str1 = va("Claw");
				break;
			case CLASS_FISH:
				str1 = va("Sea Creature");
				break;
			case CLASS_FLIER2:
				str1 = va("Flier");
				break;
			case CLASS_GLIDER:
				str1 = va("Glider");
				break;
			case CLASS_HOWLER:
				str1 = va("Howler");
				break;
			case CLASS_LIZARD:
				str1 = va("Lizard");
				break;
			case CLASS_MINEMONSTER:
				str1 = va("Mine Monster");
				break;
			case CLASS_SWAMP:
				str1 = va("Swamp Monster");
				break;
			case CLASS_RANCOR:
				str1 = va("Rancor");
				break;
			case CLASS_WAMPA:
				str1 = va("Wampa");
				break;
			case CLASS_VEHICLE:
				str1 = va("");
				break;
			default:
				//CG_Printf("NPC %i is not a civilian or vendor (class %i).\n", cent->currentState.number, cent->currentState.NPC_class);
				continue; // Unsupported...
				break;
			}
		}
		else
		{
			continue;
		}

		if (cent->currentState.eFlags & EF_DEAD)
		{
			//CG_Printf("NPC is dead.\n");
			continue;
		}

		if (cent->currentState.eFlags & EF_NODRAW)
		{
			//CG_Printf("NPC is NODRAW.\n");
			continue;
		}

		VectorCopy( cent->lerpOrigin, origin );
		origin[2] += 30;//60;

		// Account for ducking
		if ( cent->playerState->pm_flags & PMF_DUCKED )
			origin[2] -= 18;
	
		// Draw the NPC name!
		if (!CG_WorldCoordToScreenCoordFloat(origin, &x, &y))
		{
			//CG_Printf("FAILED %i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);
			continue;
		}

		if (x < 0 || x > 640 || y < 0 || y > 480)
		{
			//CG_Printf("FAILED2 %i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);
			continue;
		}

		VectorCopy( cent->lerpOrigin, origin );
		origin[2] += 25;//50;

		// Account for ducking
		if ( cent->playerState->pm_flags & PMF_DUCKED )
			origin[2] -= 18;

		dist = Distance(cg.snap->ps.origin, origin);
		
		if (dist > 1024.0f/*2500.0f*/) continue; // Too far...
		if (dist < 192.0f/*d_roff.value*//*350.0f*/) multiplier = 200.0f/*d_poff.value*//dist; // Cap short ranges...

		if (!CG_WorldCoordToScreenCoordFloat(origin, &x2, &y2))
		{
			//CG_Printf("FAILED %i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);
			continue;
		}

		if (x2 < 0 || x2 > 640 || y2 < 0 || y2 > 480)
		{
			//CG_Printf("FAILED2 %i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);
			continue;
		}

		//CG_Printf("%i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);

		if (!CG_CheckClientVisibility(cent))
		{
			//CG_Printf("NPC is NOT visible.\n");
			continue;
		}

		//CG_Printf("%i screen coords are %fx%f. (%f %f %f)\n", cent->currentState.number, x, y, origin[0], origin[1], origin[2]);

		Q_StripColor(str1);
		Q_StripColor(str2);
		
		size = dist * 0.0002;
		
		if (size > 0.99f) size = 0.99f;
		if (size < 0.01f) size = 0.01f;

		size = 1 - size;

		size *= 0.3;

		w = CG_Text_Width(str1, size*2, FONT_SMALL);
		y = y + CG_Text_Height(str1, size*2, FONT_SMALL);
		x -= (w * 0.5f);
		
		w2 = CG_Text_Width(str2, size*1.5, FONT_SMALL3);
		x2 -= (w2 * 0.5f);
		y2 = y + 6 + CG_Text_Height(str1, size*2, FONT_SMALL);

		CG_Text_Paint( x, (y*(1-size))+((30*(1-size))*(1-size))+sqrt(sqrt((1-size)*30))+((1-multiplier)*30), size*2, tclr, str1, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL);
		CG_Text_Paint( x2, (y2*(1-size))+((30*(1-size))*(1-size))+sqrt(sqrt((1-size)*30))+((1-multiplier)*30), size*1.5, tclr2, str2, 0, 0, ITEM_TEXTSTYLE_SHADOWED, FONT_SMALL3);

		//CG_Printf("Draw %s - %s as size %f.\n", sanitized1, sanitized2, size);
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
	qboolean	isVeh = qfalse;

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
        UI_DrawProportionalString (320, 170, "Press E to pick up weapon", UI_CENTER, colorTable[CT_WHITE], FONT_MEDIUM);
        return;
    }
    else
    //[/USE_ITEMS]
	if (cg.crosshairClientNum < ENTITYNUM_WORLD)
	{
		centity_t *veh = &cg_entities[cg.crosshairClientNum];

		if (veh->currentState.eType == ET_NPC &&
			veh->currentState.NPC_class == CLASS_VEHICLE &&
			veh->currentState.owner < MAX_CLIENTS)
		{ //draw the name of the pilot then
			cg.crosshairClientNum = veh->currentState.owner;
			cg.crosshairVehNum = veh->currentState.number;
			cg.crosshairVehTime = cg.time;
			isVeh = qtrue; //so we know we're drawing the pilot's name
		}
	}

	if (cg.crosshairClientNum >= MAX_CLIENTS)
	{
		/*centity_t *NPC = &cg_entities[cg.crosshairClientNum];

		if (NPC->currentState.eType == ET_NPC
			&& NPC->currentState.NPC_class == CLASS_CIVILIAN
			&& NPC->currentState.generic1 > 0
			&& NPC->currentState.generic1 < NPC_NT_NUM)
		{// Civilian NPCs should show that they are... (FIXME: Also add shopkeeper names!)

		}
		else*/
		{
			return;
		}
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

	/*if (cg_entities[cg.crosshairClientNum].currentState.eType == ET_NPC
		&& cg_entities[cg.crosshairClientNum].currentState.NPC_class == CLASS_CIVILIAN
		&& cg_entities[cg.crosshairClientNum].currentState.generic1 > 0
		&& cg_entities[cg.crosshairClientNum].currentState.generic1 < NPC_NT_NUM)
	{// Civilian NPCs should show that they are... (FIXME: Also add shopkeeper names!)
		name = va("^5[ ^3Civilian %s ^5]", NPC_NAMES[cg_entities[cg.crosshairClientNum].currentState.generic1]);
	}
	else*/
	{
		name = cgs.clientinfo[ cg.crosshairClientNum ].cleanname;
	}

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

	if (isVeh)
	{
		char str[MAX_STRING_CHARS];
		Com_sprintf(str, MAX_STRING_CHARS, "%s (pilot)", name);
		UI_DrawProportionalString(320, 170, str, UI_CENTER, tcolor, FONT_MEDIUM);
	}
	else
	{
		UI_DrawProportionalString(320, 170, name, UI_CENTER, tcolor, FONT_MEDIUM);
	}

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
//	int key;
	//if (cg_singlePlayer.integer) {
	//	CG_DrawCenterString();
	//	return;
	//}
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

//	s = "following";
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

#if 0
static void CG_DrawTemporaryStats()
{ //placeholder for testing (draws ammo and force power)
	char s[512];

	if (!cg.snap)
	{
		return;
	}

	sprintf(s, "Force: %i", cg.snap->ps.fd.forcePower);

	CG_DrawBigString(SCREEN_WIDTH-164, SCREEN_HEIGHT-dmgIndicSize, s, 1.0f);

	sprintf(s, "Ammo: %i", cg.snap->ps.ammo[weaponData[cg.snap->ps.weapon].ammoIndex]);

	CG_DrawBigString(SCREEN_WIDTH-164, SCREEN_HEIGHT-112, s, 1.0f);

	sprintf(s, "Health: %i", cg.snap->ps.stats[STAT_HEALTH]);

	CG_DrawBigString(8, SCREEN_HEIGHT-dmgIndicSize, s, 1.0f);

	sprintf(s, "Armor: %i", cg.snap->ps.stats[STAT_ARMOR]);

	CG_DrawBigString(8, SCREEN_HEIGHT-112, s, 1.0f);
}
#endif

/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
#if 0
	const char	*s;
	int			w;

	if (!cg_drawStatus.integer)
	{
		return;
	}

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.lowAmmoWarning ) {
		return;
	}

	if ( cg.lowAmmoWarning == 2 ) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
#endif
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
	float		percent = 100;
	qhandle_t	pic = 0;
	vec4_t opacity;
	
	MAKERGBA( opacity, 1, 1, 1, 1*cg.jkg_HUDOpacity );

	// FIXME: what happens if using more than one?
	if (cg.snap->ps.weapon == WP_SABER)
	{
		percent = cg.predictedPlayerState.blockPoints;
		pic = trap->R_RegisterShader("gfx/jkghud/ico_cloak.png");
	}

	if (cg.snap->ps.jetpackFuel < 100)
	{ // Jetpack is being used or is recharging
		if (cg.snap->ps.cloakFuel >= 100 || (cg.time >> 10 & 1)) {
			percent = cg.snap->ps.jetpackFuel;
			pic = trap->R_RegisterShader("gfx/jkghud/ico_jetpack.png");
		}
	}
	if (cg.snap->ps.cloakFuel < 100) {
		if (cg.snap->ps.jetpackFuel >= 100 || !(cg.time >> 10 & 1)) {
			percent = cg.snap->ps.cloakFuel;
			pic = trap->R_RegisterShader("gfx/jkghud/ico_cloak.png");
		}
	}

	if (percent == 100 && cg.snap->ps.weapon != WP_SABER) {
		return;
	}

	percent /= 100;
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


//draw meter showing jetpack fuel when it's not full
#define JPFUELBAR_H			100.0f
#define JPFUELBAR_W			20.0f
#define JPFUELBAR_X			(SCREEN_WIDTH-JPFUELBAR_W-8.0f)
#define JPFUELBAR_Y			260.0f
void CG_DrawJetpackFuel(void)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = JPFUELBAR_X;
	float y = JPFUELBAR_Y;
	float percent = ((float)cg.snap->ps.jetpackFuel/100.0f)*JPFUELBAR_H;

	if (percent > JPFUELBAR_H)
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
	CG_DrawRect(x, y, JPFUELBAR_W, JPFUELBAR_H, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much health there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f+(JPFUELBAR_H-percent), JPFUELBAR_W-1.0f, JPFUELBAR_H-1.0f-(JPFUELBAR_H-percent), aColor);

	//then draw the other part greyed out
	CG_FillRect(x+1.0f, y+1.0f, JPFUELBAR_W-1.0f, JPFUELBAR_H-percent, cColor);
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

	//kind of hacky, need to pass a coordinate in here
	if (cg.snap->ps.jetpackFuel < 100)
	{
		x -= (JPFUELBAR_W+8.0f);
	}
	if (cg.snap->ps.cloakFuel < 100)
	{
		x -= (JPFUELBAR_W+8.0f);
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

//draw meter showing cloak fuel when it's not full
#define CLFUELBAR_H			100.0f
#define CLFUELBAR_W			20.0f
#define CLFUELBAR_X			(SCREEN_WIDTH-CLFUELBAR_W-8.0f)
#define CLFUELBAR_Y			260.0f
void CG_DrawCloakFuel(void)
{
	vec4_t aColor;
	vec4_t bColor;
	vec4_t cColor;
	float x = CLFUELBAR_X;
	float y = CLFUELBAR_Y;
	float percent = ((float)cg.snap->ps.cloakFuel/100.0f)*CLFUELBAR_H;

	if (percent > CLFUELBAR_H)
	{
		return;
	}

	if ( cg.snap->ps.jetpackFuel < 100 )
	{//if drawing jetpack fuel bar too, then move this over...?
		x -= (JPFUELBAR_W+8.0f);
	}

	if (percent < 0.1f)
	{
		percent = 0.1f;
	}

	//color of the bar
	aColor[0] = 0.0f;
	aColor[1] = 0.0f;
	aColor[2] = 0.6f;
	aColor[3] = 0.8f;

	//color of the border
	bColor[0] = 0.0f;
	bColor[1] = 0.0f;
	bColor[2] = 0.0f;
	bColor[3] = 0.3f;

	//color of greyed out "missing fuel"
	cColor[0] = 0.1f;
	cColor[1] = 0.1f;
	cColor[2] = 0.3f;
	cColor[3] = 0.1f;

	//draw the background (black)
	CG_DrawRect(x, y, CLFUELBAR_W, CLFUELBAR_H, 1.0f, colorTable[CT_BLACK]);

	//now draw the part to show how much fuel there is in the color specified
	CG_FillRect(x+1.0f, y+1.0f+(CLFUELBAR_H-percent), CLFUELBAR_W-1.0f, CLFUELBAR_H-1.0f-(CLFUELBAR_H-percent), aColor);

	//then draw the other part greyed out
	CG_FillRect(x+1.0f, y+1.0f, CLFUELBAR_W-1.0f, CLFUELBAR_H-percent, cColor);
}

int cgRageTime = 0;
int cgRageFadeTime = 0;
float cgRageFadeVal = 0;

int cgRageRecTime = 0;
int cgRageRecFadeTime = 0;
float cgRageRecFadeVal = 0;

int cgAbsorbTime = 0;
int cgAbsorbFadeTime = 0;
float cgAbsorbFadeVal = 0;

int cgProtectTime = 0;
int cgProtectFadeTime = 0;
float cgProtectFadeVal = 0;

int cgYsalTime = 0;
int cgYsalFadeTime = 0;
float cgYsalFadeVal = 0;

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
			/*if (chat->string[i] == '^') {
				// Dont include color codes in here :P
				if (chat->string[i+1] >= '0' && chat->string[i+1] <= '9') {
					i+=2;
					continue;
				}
				if (chat->string[i + 1] == 'x' && Text_IsExtColorCode(&chat->string[i + 1]) || chat->string[i + 1] == 'X' && Text_IsExtColorCode(&chat->string[i + 1])) {		//futuza also allow 'X'
					i+=5;
					continue;
				}
			}*/
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
/*
//insert item into array (rearranging the array if necessary)
void CG_ChatBox_ArrayInsert(chatBoxItem_t **array, int insPoint, int maxNum, chatBoxItem_t *item)
{
    if (array[insPoint])
	{ //recursively call, to move everything up to the top
		if (insPoint+1 >= maxNum)
		{
			CG_Error("CG_ChatBox_ArrayInsert: Exceeded array size");
		}
		CG_ChatBox_ArrayInsert(array, insPoint+1, maxNum, array[insPoint]);
	}

	//now that we have moved anything that would be in this slot up, insert what we want into the slot
	array[insPoint] = item;
}

//go through all the chat strings and draw them if they are not yet expired
static CGAME_INLINE void CG_ChatBox_DrawStrings(void)
{
	chatBoxItem_t *drawThese[MAX_CHATBOX_ITEMS];
	int numToDraw = 0;
	int linesToDraw = 0;
	int i = 0;
	int x = 30;
	int y = cg.scoreBoardShowing ? 475 : cg_chatBoxHeight.integer;
	float fontScale = 0.65f;

	if (!cg_chatBox.integer)
	{
		return;
	}

	memset(drawThese, 0, sizeof(drawThese));

	while (i < MAX_CHATBOX_ITEMS)
	{
		if (cg.chatItems[i].time >= cg.time)
		{
			int check = numToDraw;
			int insertionPoint = numToDraw;

			while (check >= 0)
			{
				if (drawThese[check] &&
					cg.chatItems[i].time < drawThese[check]->time)
				{ //insert here
					insertionPoint = check;
				}
				check--;
			}
			CG_ChatBox_ArrayInsert(drawThese, insertionPoint, MAX_CHATBOX_ITEMS, &cg.chatItems[i]);
			numToDraw++;
			linesToDraw += cg.chatItems[i].lines;
		}
		i++;
	}

	if (!numToDraw)
	{ //nothing, then, just get out of here now.
		return;
	}

	//move initial point up so we draw bottom-up (visually)
	y -= (CHATBOX_FONT_HEIGHT*fontScale)*linesToDraw;

	//we have the items we want to draw, just quickly loop through them now
	i = 0;
	while (i < numToDraw)
	{
		CG_Text_Paint(x, y, fontScale, colorWhite, drawThese[i]->string, 0, 0, ITEM_TEXTSTYLE_OUTLINED, FONT_SMALL );
		y += ((CHATBOX_FONT_HEIGHT*fontScale)*drawThese[i]->lines);
		i++;
	}
}
*/


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
	float			rageTime, rageRecTime, absorbTime, protectTime;
	vec4_t			hcolor;
	
	if (cgs.clientinfo[cg.snap->ps.clientNum].team != TEAM_SPECTATOR)
	{
		float hpMultiplier = 100.0f / (float)cg.predictedPlayerState.stats[STAT_MAX_HEALTH];
		float lowHealthPhase = CG_GetLowHealthPhase(0, hpMultiplier);
		if ( ((float)cg.predictedPlayerState.stats[STAT_HEALTH] * hpMultiplier) < 30 ) {
			if (cg.deathTime && !cg.deathcamFadeStart) {	
				lowHealthPhase = 0;	// Dont glow red
				hcolor[3] = 0.2f;
				//CG_DrawRect(0, 0, 640, 480, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
				// Calculate the fadeout state
				if (cg.deathTime) { // failsafe
					hcolor[0] = hcolor[1] = hcolor[2] = 0; // Black
					if (cg.time - cg.deathTime < 2000) {
						// hcolor[3] = 0;
						// Dont render anything
					} else if (cg.time - cg.deathTime < 6000) {
						// In fade
						hcolor[3] = ((float)(cg.time - cg.deathTime - 2000) / 4000.0f); // 0 to 4000 converted to 0.0f to 1.0f
						CG_DrawRect(0, 0, 640, 480, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
					} else {
						hcolor[3] = 1;
						CG_DrawRect(0, 0, 640, 480, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
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
					CG_DrawRect(0, 0, 640, 480, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
				}
				trap->R_SetColor(NULL);
				// Display the 'you will be revived in xx seconds'
				trap->R_DrawStretchPic(80,390, 480, 55, 0, 0, 1, 1, cgs.media.horizgradient);
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
			trap->R_DrawStretchPic(0,0,640,480,0, 0, 1, 1, cgs.media.lowHealthAura);
			trap->R_SetColor(NULL);
		}
		if (cg.snap->ps.fd.forcePowersActive & (1 << FP_RAGE))
		{
			if (!cgRageTime)
			{
				cgRageTime = cg.time;
			}
			
			rageTime = (float)(cg.time - cgRageTime);
			
			rageTime /= 9000;
			
			if (rageTime < 0)
			{
				rageTime = 0;
			}
			if (rageTime > 0.15f)
			{
				rageTime = 0.15f;
			}
			
			hcolor[3] = rageTime;
			hcolor[0] = 0.7f;
			hcolor[1] = 0;
			hcolor[2] = 0;
			
			if (!cg.renderingThirdPerson)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			
			cgRageFadeTime = 0;
			cgRageFadeVal = 0;
		}
		else if (cgRageTime)
		{
			if (!cgRageFadeTime)
			{
				cgRageFadeTime = cg.time;
				cgRageFadeVal = 0.15f;
			}
			
			rageTime = cgRageFadeVal;
			
			cgRageFadeVal -= (cg.time - cgRageFadeTime)*0.000005f;
			
			if (rageTime < 0)
			{
				rageTime = 0;
			}
			if (rageTime > 0.15f)
			{
				rageTime = 0.15f;
			}
			
			if (cg.snap->ps.fd.forceRageRecoveryTime > cg.time)
			{
				float checkRageRecTime = rageTime;
				
				if (checkRageRecTime < 0.15f)
				{
					checkRageRecTime = 0.15f;
				}
				
				hcolor[3] = checkRageRecTime;
				hcolor[0] = rageTime*4;
				if (hcolor[0] < 0.2f)
				{
					hcolor[0] = 0.2f;
				}
				hcolor[1] = 0.2f;
				hcolor[2] = 0.2f;
			}
			else
			{
				hcolor[3] = rageTime;
				hcolor[0] = 0.7f;
				hcolor[1] = 0;
				hcolor[2] = 0;
			}
			
			if (!cg.renderingThirdPerson && rageTime)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			else
			{
				if (cg.snap->ps.fd.forceRageRecoveryTime > cg.time)
				{
					hcolor[3] = 0.15f;
					hcolor[0] = 0.2f;
					hcolor[1] = 0.2f;
					hcolor[2] = 0.2f;
					CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
				}
				cgRageTime = 0;
			}
		}
		else if (cg.snap->ps.fd.forceRageRecoveryTime > cg.time)
		{
			if (!cgRageRecTime)
			{
				cgRageRecTime = cg.time;
			}
			
			rageRecTime = (float)(cg.time - cgRageRecTime);
			
			rageRecTime /= 9000;
			
			if (rageRecTime < 0.15f)//0)
			{
				rageRecTime = 0.15f;//0;
			}
			if (rageRecTime > 0.15f)
			{
				rageRecTime = 0.15f;
			}
			
			hcolor[3] = rageRecTime;
			hcolor[0] = 0.2f;
			hcolor[1] = 0.2f;
			hcolor[2] = 0.2f;
			
			if (!cg.renderingThirdPerson)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			
			cgRageRecFadeTime = 0;
			cgRageRecFadeVal = 0;
		}
		else if (cgRageRecTime)
		{
			if (!cgRageRecFadeTime)
			{
				cgRageRecFadeTime = cg.time;
				cgRageRecFadeVal = 0.15f;
			}
			
			rageRecTime = cgRageRecFadeVal;
			
			cgRageRecFadeVal -= (cg.time - cgRageRecFadeTime)*0.000005f;
			
			if (rageRecTime < 0)
			{
				rageRecTime = 0;
			}
			if (rageRecTime > 0.15f)
			{
				rageRecTime = 0.15f;
			}
			
			hcolor[3] = rageRecTime;
			hcolor[0] = 0.2f;
			hcolor[1] = 0.2f;
			hcolor[2] = 0.2f;
			
			if (!cg.renderingThirdPerson && rageRecTime)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			else
			{
				cgRageRecTime = 0;
			}
		}
		
		if (cg.snap->ps.fd.forcePowersActive & (1 << FP_ABSORB))
		{
			if (!cgAbsorbTime)
			{
				cgAbsorbTime = cg.time;
			}
			
			absorbTime = (float)(cg.time - cgAbsorbTime);
			
			absorbTime /= 9000;
			
			if (absorbTime < 0)
			{
				absorbTime = 0;
			}
			if (absorbTime > 0.15f)
			{
				absorbTime = 0.15f;
			}
			
			hcolor[3] = absorbTime/2;
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 0.7f;
			
			if (!cg.renderingThirdPerson)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			
			cgAbsorbFadeTime = 0;
			cgAbsorbFadeVal = 0;
		}
		else if (cgAbsorbTime)
		{
			if (!cgAbsorbFadeTime)
			{
				cgAbsorbFadeTime = cg.time;
				cgAbsorbFadeVal = 0.15f;
			}
			
			absorbTime = cgAbsorbFadeVal;
			
			cgAbsorbFadeVal -= (cg.time - cgAbsorbFadeTime)*0.000005f;
			
			if (absorbTime < 0)
			{
				absorbTime = 0;
			}
			if (absorbTime > 0.15f)
			{
				absorbTime = 0.15f;
			}
			
			hcolor[3] = absorbTime/2;
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 0.7f;
			
			if (!cg.renderingThirdPerson && absorbTime)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			else
			{
				cgAbsorbTime = 0;
			}
		}
		
		if (cg.snap->ps.fd.forcePowersActive & (1 << FP_PROTECT))
		{
			if (!cgProtectTime)
			{
				cgProtectTime = cg.time;
			}
			
			protectTime = (float)(cg.time - cgProtectTime);
			
			protectTime /= 9000;
			
			if (protectTime < 0)
			{
				protectTime = 0;
			}
			if (protectTime > 0.15f)
			{
				protectTime = 0.15f;
			}
			
			hcolor[3] = protectTime/2;
			hcolor[0] = 0;
			hcolor[1] = 0.7f;
			hcolor[2] = 0;
			
			if (!cg.renderingThirdPerson)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			
			cgProtectFadeTime = 0;
			cgProtectFadeVal = 0;
		}
		else if (cgProtectTime)
		{
			if (!cgProtectFadeTime)
			{
				cgProtectFadeTime = cg.time;
				cgProtectFadeVal = 0.15f;
			}
			
			protectTime = cgProtectFadeVal;
			
			cgProtectFadeVal -= (cg.time - cgProtectFadeTime)*0.000005f;
			
			if (protectTime < 0)
			{
				protectTime = 0;
			}
			if (protectTime > 0.15f)
			{
				protectTime = 0.15f;
			}
			
			hcolor[3] = protectTime/2;
			hcolor[0] = 0;
			hcolor[1] = 0.7f;
			hcolor[2] = 0;
			
			if (!cg.renderingThirdPerson && protectTime)
			{
				CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);
			}
			else
			{
				cgProtectTime = 0;
			}
		}
		
		if (cg.snap->ps.rocketLockIndex != ENTITYNUM_NONE && (cg.time - cg.snap->ps.rocketLockTime) > 0)
		{
			CG_DrawRocketLocking( cg.snap->ps.rocketLockIndex, cg.snap->ps.rocketLockTime );
		}
	}

	if ( (cg.refdef.viewContents&CONTENTS_LAVA) )
	{//tint screen red
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.5f + (0.15f*sin( phase ));
		hcolor[0] = 0.7f;
		hcolor[1] = 0;
		hcolor[2] = 0;
		
		CG_DrawRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor  );
	}
	else if ( (cg.refdef.viewContents&CONTENTS_SLIME) )
	{//tint screen green
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.4f + (0.1f*sin( phase ));
		hcolor[0] = 0;
		hcolor[1] = 0.7f;
		hcolor[2] = 0;
		
		CG_DrawRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor  );
	}
	else if ( (cg.refdef.viewContents&CONTENTS_WATER) )
	{//tint screen light blue -- FIXME: don't do this if CONTENTS_FOG? (in case someone *does* make a water shader with fog in it?)
		float phase = cg.time / 1000.0f * WAVE_FREQUENCY * M_PI * 2;
		hcolor[3] = 0.3f + (0.05f*sin( phase ));
		hcolor[0] = 0;
		hcolor[1] = 0.2f;
		hcolor[2] = 0.8f;
		
		CG_DrawRect( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor  );
	}
}
void Cin_ProcessFade();
void Cin_ProcessFlash();

extern vmCvar_t jkg_noletterbox;
void CinBuild_Visualize2D();


void ChatBox_DrawBackdrop(menuDef_t *menu);
void ChatBox_DrawChat(menuDef_t *menu);

extern void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars, qhandle_t textShader );

void CG_DrawChatbox() {
	menuDef_t *menuHUD;
	itemDef_t *item;
	vec4_t		color;
	vec4_t tmpCol;

	menuHUD = Menus_FindByName("hud_chatbox");
	if (!menuHUD) {
		return;
	}
#pragma region h_
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
#pragma endregion
	ChatBox_DrawBackdrop(menuHUD);



#pragma region frame
	item = Menu_FindItemByName(menuHUD, "frame");
	if (item) {
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
#pragma endregion
#pragma region text
	item = Menu_FindItemByName(menuHUD, "text");

	if (item)
	{
		int i;
		int line;

		MAKERGBA(color,0,0,0,0.3f);
		color[3] *= cg.jkg_HUDOpacity;
		CG_FillRect(item->window.rect.x-5, item->window.rect.y-2, item->window.rect.w+10, item->window.rect.h+4, color);
		VectorCopy(colorWhite, color);
		color[3] *= cg.jkg_HUDOpacity;
		// Center and draw the text, positioning will be finetuned later on :P
		line = 0;
		for (i=0; i<12; i++) {
			int idx = (cg.chatItemNext + i) % 12;
			if (cg.chatItems[idx].active) {
				ChatBox_SetPaletteAlpha(cg.chatItems[idx].alpha*cg.jkg_HUDOpacity);
				color[3] = cg.chatItems[idx].alpha*cg.jkg_HUDOpacity;
				CG_DrawStringExt(item->window.rect.x, item->window.rect.y + (line * 8),
					cg.chatItems[idx].string,
					color, qfalse, qfalse, 5, 8, strlen(cg.chatItems[idx].string), cgs.media.charset_Segoeui);
				line += cg.chatItems[idx].lines;
			}
		}
		ChatBox_SetPaletteAlpha(1);
	}
#pragma endregion
	
	MAKERGBA(color, 1, 1, 1, 1*cg.jkg_HUDOpacity);
	trap->R_SetColor(color);

	ChatBox_DrawChat(menuHUD);
}

void ChatBox_CloseChat();
static void CG_Draw2D( void ) {
	float			inTime = cg.invenSelectTime+WEAPON_SELECT_TIME;
	float			wpTime = cg.weaponSelectTime+WEAPON_SELECT_TIME;
	float			fallTime; 
	int				drawSelect = 0;
	int				drawHUD	= cg_drawHUD.integer;

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		cgRageTime = 0;
		cgRageFadeTime = 0;
		cgRageFadeVal = 0;

		cgRageRecTime = 0;
		cgRageRecFadeTime = 0;
		cgRageRecFadeVal = 0;

		cgAbsorbTime = 0;
		cgAbsorbFadeTime = 0;
		cgAbsorbFadeVal = 0;

		cgProtectTime = 0;
		cgProtectFadeTime = 0;
		cgProtectFadeVal = 0;

		cgYsalTime = 0;
		cgYsalFadeTime = 0;
		cgYsalFadeVal = 0;
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
	CinBuild_Visualize2D();


	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		//CG_ChatBox_DrawStrings();
		return;
	}

	CG_Draw2DScreenTints();
	if (drawHUD && !ui_hidehud.integer) {
		if (cg.snap->ps.rocketLockIndex != ENTITYNUM_NONE && (cg.time - cg.snap->ps.rocketLockTime) > 0)
		{
			CG_DrawRocketLocking( cg.snap->ps.rocketLockIndex, cg.snap->ps.rocketLockTime );
		}

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
			if (cg.isChatting) {
				ChatBox_CloseChat();
			}

		} else {
			// don't draw any status if dead or the scoreboard is being explicitly shown
			if ( cg.snap->ps.stats[STAT_HEALTH] > 0 && !cg.deathcamFadeStart ) { //!cg.showScores && to bring back scoreboard on tab press.

				if ( /*cg_drawStatus.integer*/0 ) {
					//Reenable if stats are drawn with menu system again
					Menu_PaintAll();
					CG_DrawTimedMenus();
				}
	      
				//CG_DrawTemporaryStats();
#ifdef __SWF__
				CG_DrawPic(0, 0, 60, 60, cgs.media.swfTestShader);
#endif

				CG_DrawAmmoWarning();

				CG_DrawMessageNotifications();

				CG_DrawCrosshairNames();

				if (cg_drawStatus.integer)
				{
					CG_DrawIconBackground();
				}

				if (cg.forceSelectTime+5000 > cg.time)
				{
					drawSelect = 2;
					CG_DrawForceSelect();
				}

				if (cg_drawStatus.integer)
				{
					//Powerups now done with upperright stuff

					CG_DrawFlagStatus();
				}

				CG_SaberClashFlare();

				if (cg_drawStatus.integer)
				{
					CG_DrawStats();
				}

				// Draw Team Overlay (also used by parties and such)
				CG_DrawTeamOverlay();

				CG_DrawReload();
				CG_DrawGrenade();

				CG_DrawPickupItem();

				//Do we want to use this system again at some point?
				//CG_DrawReward();
				CG_DrawChatbox();	// Only show the chatbox when we're alive (JKG)
			} else {
				if (cg.isChatting) {
					ChatBox_CloseChat();
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

		CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, hcolor);

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
		CG_DrawBracketedEntities();
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
	// always draw chat
	//CG_ChatBox_DrawStrings();
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

extern vmCvar_t r_bloom_factor;
extern vmCvar_t r_bloom_threshold;

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

