#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "qcommon/q_shared.h"

extern displayContextDef_t cgDC;
extern float *hudTintColor;
extern vec4_t bluehudtint;
extern vec4_t redhudtint;

int MenuFontToHandle(int iMenuFont);
void ChatBox_SetPaletteAlpha(float alpha);
void MiniMap_Render(menuDef_t *menu, float radiusScale);
void CG_DrawJetpackCloak(menuDef_t *menuHUD);

static __inline void AdjustOpacityLevels(void)
{
	//playerState_t *ps = &cg.predictedPlayerState;

	if ( (cg.predictedPlayerState.stats[STAT_HEALTH] < 1 || cg.deathcamTime) ||
		cg.predictedPlayerState.zoomMode ||
		cg.predictedPlayerState.pm_type == PM_INTERMISSION ||
		cg.spectatorTime)
	{
		cg.jkg_HUDOpacity -= ((float)cg.frameDelta/200.0f);
	} else {
		cg.jkg_HUDOpacity += ((float)cg.frameDelta/200.0f);	
	}

	CLAMP( cg.jkg_HUDOpacity, 0.0f, 1.0f );

	if( cg.predictedPlayerState.weaponTime <= 0 )
	{
		cg.jkg_WHUDOpacity = 1.0f;
	}
	else
	{
		if( cg.predictedPlayerState.weaponstate == WEAPON_RAISING )
		{
			// pm->ps->weaponTime += 350;
			int time = cg.predictedPlayerState.weaponTime-200;
			BUMP( time, 0 );

			cg.jkg_WHUDOpacity = 1.0f-((float)time/150);
		}
		else if( cg.predictedPlayerState.weaponstate == WEAPON_DROPPING )
		{
			// pm->ps->weaponTime += 300;
			// Lerp it. Needs to be a 100 ms period where we can't see anything
			int time = cg.predictedPlayerState.weaponTime-150;
			BUMP( time, 0 );

			cg.jkg_WHUDOpacity = (float)time/150;
		}
		else
		{
			// Not switching weapons I guess (must be firing or something). Just set it at 1.0f
			cg.jkg_WHUDOpacity = 1.0f;
		}
	}


	return;
}

/*
================
CG_DrawSaberStyle

If the weapon is a light saber (which needs no ammo) then draw a graphic showing
the saber style (fast, medium, strong)
================
*/
static void CG_DrawSaberStyle( centity_t *cent, menuDef_t *menuHUD)
{

	const weaponInfo_t *weaponInfo;
	itemDef_t		*focusItem;
	const char		*text;
	float			width;


	if (!cent->currentState.weapon ) // We don't have a weapon right now
	{
		return;
	}

	if ( cent->currentState.weapon != WP_SABER )
	{
		return;
	}

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

    weaponInfo = CG_WeaponInfo (WP_SABER, 0);

	// draw the current saber style in this window
	// TODO: cvar plz
	if(jkg_simpleHUD.integer)
	{
		text = va( "Style: %s", SaberStances[cg.predictedPlayerState.fd.saberDrawAnimLevel].saberName_simple );
	}
	else
	{
		text = va( "Stance: %s", SaberStances[cg.predictedPlayerState.fd.saberDrawAnimLevel].saberName_technical );
	}

	// Now then, lets render this text ^_^

	width = (float)trap->R_Font_StrLenPixels(text, cgDC.Assets.qhSmall3Font, 1) * 0.6f;
				
	focusItem = Menu_FindItemByName(menuHUD, "infobar");
	if (focusItem)
	{
		trap->R_Font_DrawString(focusItem->window.rect.x + ((focusItem->window.rect.w/2) - (width/2)), focusItem->window.rect.y, text, colorWhite, cgDC.Assets.qhSmall3Font, -1, 0.6f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "weapicon");
	if (focusItem)
	{
		trap->R_SetColor( NULL );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			weaponInfo->hudIcon
			);			
	}
}

/*
================
CG_DrawAmmo
================
*/
static void CG_DrawAmmo( centity_t	*cent,menuDef_t *menuHUD)
{
	playerState_t	*ps;
	int				ammo;
	itemDef_t		*focusItem;
	float			width;
	const char		*text;
	vec4_t			opacity;
	const weaponInfo_t *weaponInfo;
	weaponData_t* wp;

	if( cg.jkg_WHUDOpacity  < 1.0f )
	{
		MAKERGBA(opacity, 1,1,1, cg.jkg_WHUDOpacity);
	}
	else
	{
		MAKERGBA(opacity, 1,1,1, cg.jkg_HUDOpacity);
	}

	ps = &cg.snap->ps;

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	if (!cent->currentState.weapon || cent->currentState.weapon == WP_MELEE ) // We don't have a weapon right now
	{
		return;
	}

	wp = GetWeaponData(cent->currentState.weapon, cent->currentState.weaponVariation);

#ifndef NO_SP_STYLE_AMMO
	// Figure out whether or not we want to do the thing where we highlight the text whenever we consume ammo or change firing mode (or, change weapon)
	if (cg.lastAmmo != cg.predictedPlayerState.stats[STAT_AMMO] || cg.lastAmmoGun != cg.predictedPlayerState.weaponId)
	{
		cg.lastAmmo = cg.predictedPlayerState.stats[STAT_AMMO];
		cg.lastAmmoTime = cg.time + 200; // matches SP 1:1
		cg.lastAmmoGun = cg.predictedPlayerState.weaponId;
	}

	if(cg.lastAmmoTime > cg.time)
	{
		vec4_t colorCopy = { 0.2, 0.72, 0.86, 1 };
		Q_RGBCopy(&opacity, colorCopy);
	}
#endif
	
	weaponInfo = CG_WeaponInfo (cent->currentState.weapon, cent->currentState.weaponVariation);

	if ( GetWeaponData( cent->currentState.weapon, cent->currentState.weaponVariation )->firemodes[0].cost == 0 && GetWeaponData( cent->currentState.weapon, cent->currentState.weaponVariation )->firemodes[1].cost == 0)
	{ //just draw "infinite"
		text = "Ammo: Infinite";
	}
	else
	{
		if ( wp->firemodes[cent->currentState.firingMode].clipSize )
		{
			// Display the amount of clips too
			float temp;
			ammo = ps->stats[STAT_AMMO];
			temp = ceil((float)ps->stats[STAT_TOTALAMMO] / (float)wp->firemodes[cent->currentState.firingMode].clipSize);
			text = va( "Ammo: %i (%i) ", ammo, ( int ) temp );
		}
		else
		{
			ammo = ps->stats[STAT_TOTALAMMO];
			text = va( "Ammo: %i ", ammo );
		}
	}

	// Now then, lets render this text ^_^

	width = (float)trap->R_Font_StrLenPixels(text, cgDC.Assets.qhSmallFont, 1) * 0.6f;
				
	focusItem = Menu_FindItemByName(menuHUD, "infobar");
	if (focusItem)
	{
		trap->R_Font_DrawString(focusItem->window.rect.x + ((focusItem->window.rect.w/2) - (width/2)), focusItem->window.rect.y, text, opacity, cgDC.Assets.qhSmall3Font, -1, 0.5f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "weapicon");
	if (focusItem)
	{
		trap->R_SetColor( opacity );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			weaponInfo->hudIcon
			);			
	}
}

/*
================
JKG_DrawFiringMode
================
*/
extern int JKG_GetTransitionForFiringModeSet(int previous, int next);
static void JKG_DrawFiringMode( menuDef_t *menuHUD )
{
	// Menu ain't used...yet.
	float x, y, w;
	char *text;
	int textWidth;
	weaponData_t *wp = BG_GetWeaponDataByIndex(cg.predictedPlayerState.weaponId);
	unsigned char firingMode = cg.predictedPlayerState.firingMode;
	vec4_t opacity;

	if( cg.predictedPlayerState.weapon == WP_SABER )
	{
		// Sabers don't have firing modes, loel
		return;
	}

	if( cg.jkg_WHUDOpacity  < 1.0f )
	{
		MAKERGBA(opacity, 1,1,1, cg.jkg_WHUDOpacity);
	}
	else
	{
		MAKERGBA(opacity, 1,1,1, cg.jkg_HUDOpacity);
	}

	if(!wp)
	{
		return;
	}

	if(wp->numFiringModes <= 1)
	{
		return;
	}

#ifndef NO_SP_STYLE_AMMO
	if(cg.lastFiringModeTime > cg.time)
	{
		vec4_t colorCopy = { 0.2, 0.72, 0.86, 1 };
		Q_RGBCopy(&opacity, colorCopy);
	}
#endif

	// Set us some basic defaults (for now. these will be replaced by the jkg_hud.menu)
	x = 500.0f;
	y = 448.0f;
	w = 120.0f;

	// Right. Now let's get the display string.
	// FIXME: Localize this text --eez
	text = va("Mode: %s", CG_GetStringEdString2(wp->visuals.visualFireModes[firingMode].displayName));
	if(!text || !text[0])
	{	// ok maybe this isn't realistic at all but i'm paranoid
		return;
	}

	textWidth = trap->R_Font_StrLenPixels(text, cgDC.Assets.qhSmall3Font, 0.4f);
	trap->R_Font_DrawString(x + ((w/2) - (textWidth/2)), y, text, opacity, cgDC.Assets.qhSmall3Font, -1, 0.4f);
}

/*
================
JKG_DrawAmmoType
================
*/
static void JKG_DrawAmmoType(menuDef_t* menuHUD) {
	playerState_t* ps = &cg.predictedPlayerState;
	weaponData_t* wp = BG_GetWeaponDataByIndex(ps->weaponId);
	ammo_t* ammo = BG_GetAmmo(ps->ammoType);
	char* displayStr;
	vec4_t opacity;
	float textWidth;
	float x, y, w;

	// Check whether we have an ammo type
	if (ammo == nullptr) {
		return;
	}

	// Check whether we have a valid firing mode
	if (ps->firingMode < 0 || ps->firingMode >= wp->numFiringModes) {
		return;
	}

	// Check against using an invalid weapon type
	if (wp->weaponBaseIndex == WP_SABER || wp->weaponBaseIndex == WP_NONE || wp->weaponBaseIndex == WP_MELEE) {
		return;
	}

	// Check against the weapon using quantity
	if (wp->firemodes[ps->firingMode].useQuantity) {
		return;
	}

	// Check against the weapon having alternate ammo
	if (!BG_WeaponAcceptsAlternateAmmo(ps->weapon, ps->weaponVariation))
	{
		return;
	}

	if (cg.jkg_WHUDOpacity  < 1.0f)
	{
		MAKERGBA(opacity, 1, 1, 1, cg.jkg_WHUDOpacity);
	}
	else
	{
		MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);
	}

#ifndef NO_SP_STYLE_AMMO
	// Figure out whether or not we want to do the thing where we highlight the text whenever we consume ammo or change firing mode (or, change weapon)
	if (cg.lastAmmoType != cg.predictedPlayerState.ammoType)
	{
		cg.lastAmmoType = cg.predictedPlayerState.ammoType;
		cg.lastAmmoTypeTime = cg.time + 200; // matches SP 1:1
	}

	if (cg.lastAmmoTypeTime > cg.time)
	{
		vec4_t colorCopy = { 0.2, 0.72, 0.86, 1 };
		Q_RGBCopy(&opacity, colorCopy);
	}
#endif

	// Set us some basic defaults (for now. these will be replaced by the jkg_hud.menu)
	x = 500.0f;
	y = 440.0f;
	w = 120.0f;

	displayStr = (char*)CG_GetStringEdString2(ammo->shortname);
	textWidth = trap->R_Font_StrLenPixels(displayStr, cgDC.Assets.qhSmall3Font, 0.4f);
	trap->R_Font_DrawString(x + ((w / 2) - (textWidth / 2)), y, displayStr, opacity, cgDC.Assets.qhSmall3Font, -1, 0.4f);
}

/*
================
CG_DrawForcePower
================
*/
static void CG_DrawForcePower( menuDef_t *menuHUD )
{

	// This goes in the lower bar on the hud	
	int				stamina, maxStamina = 100;
	vec4_t			calcColor;
	float			percent;
	qboolean	flash=qfalse;
	itemDef_t	*focusItem;
	vec4_t	opacity;
	vec4_t	glowColor;
	playerState_t* ps;

	ps = &cg.snap->ps;

	// What's the maximum stamina/forcepower?
	maxStamina = ps->stats[STAT_MAX_STAMINA];
	stamina = cg.predictedPlayerState.forcePower;
	
	MAKERGBA( opacity, 1, 1, 1, 1*cg.jkg_HUDOpacity );

	if (!menuHUD)
	{
		return;
	}

	// Make the hud flash by setting forceHUDTotalFlashTime above cg.time
	if (cg.forceHUDTotalFlashTime > cg.time )
	{
		flash = qtrue;
		if (cg.forceHUDNextFlashTime < cg.time)	
		{
			cg.forceHUDNextFlashTime = cg.time + 400;
			trap->S_StartSound (NULL, 0, CHAN_LOCAL, cgs.media.noforceSound );

			if (cg.forceHUDActive)
			{
				cg.forceHUDActive = qfalse;
			}
			else
			{
				cg.forceHUDActive = qtrue;
			}

		}
	}
	else	// turn HUD back on if it had just finished flashing time.
	{
		cg.forceHUDNextFlashTime = 0;
		cg.forceHUDActive = qtrue;
	}

//	if (!cg.forceHUDActive)
//	{
//		return;
//	}


	if (flash)
	{
		MAKERGBA(calcColor, 1, 1, 1, 0.6f*cg.jkg_HUDOpacity);
	}
	else 
	{
		MAKERGBA(calcColor, 1, 1, 1, 1*cg.jkg_HUDOpacity);
	}

	/*focusItem = Menu_FindItemByName(menuHUD, "bar2ico");
	if (focusItem)
	{
		calcColor[3] *= cg.jkg_HUDOpacity;
		trap->R_SetColor( calcColor );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			trap->R_RegisterShader("gfx/jkghud/ico_stamina.png") // TODO: Precache this
			);			
	}*/

	// Work out the bar now

	percent = (float)stamina / (float)maxStamina;
	//percent *= 0.75f; // Range of the bar is 0 to 0.75f

	focusItem = Menu_FindItemByName(menuHUD, "staminabar");
	if (focusItem)
	{
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

	{
		char *Force = va("%i / %i", stamina, maxStamina);
		VectorCopy4(colorWhite, glowColor);
		glowColor[3] *= cg.jkg_HUDOpacity;
		trap->R_Font_DrawString(focusItem->window.rect.x + (focusItem->window.rect.w / 2) - (trap->R_Font_StrLenPixels(Force, 1, 0.6) / 2),
			focusItem->window.rect.y - 2/* + (trap->R_Font_HeightPixels(1, 1.0)*0.4)*/,
			Force, glowColor, cgs.media.hudfont1, -1, 0.6f);
	}
}

/*
===================
FlagCaptureBar
===================
*/

void FlagCaptureBar(void)
{
	const int numticks = 50, tickwidth = 1, tickheight = 3;
	const int tickpadx = 2, tickpady = 2;
	const int capwidth = 2;
	const int barwidth = numticks*tickwidth+tickpadx*2+capwidth*2;
	const int barleft = ((1202-barwidth)/2);
	const int barheight = tickheight + tickpady*2;
	const int bartop = 173-barheight;
	const int capleft = barleft+tickpadx;
	const int tickleft = capleft+capwidth, ticktop = bartop+tickpady;
	float percentage = 0.0f;

	if (cg.captureFlagPercent <= 0)
		return;

	percentage = cg.captureFlagPercent/2;

	trap->R_SetColor( colorWhite );
	// Draw background
	CG_DrawPic(barleft, bartop, barwidth, barheight, cgs.media.loadBarLEDSurround);

	// Draw left cap (backwards)
	CG_DrawPic(tickleft, ticktop, -capwidth, tickheight, cgs.media.loadBarLEDCap);

	// Draw bar
	CG_DrawPic(tickleft, ticktop, tickwidth*percentage, tickheight, cgs.media.loadBarLED);

	// Draw right cap
	CG_DrawPic(tickleft+tickwidth*percentage, ticktop, capwidth, tickheight, cgs.media.loadBarLEDCap);
}

static int color_selector = 0;
int next_color_update = 0;
qboolean color_forwards = qtrue;

/*
================
CG_DrawHealth
================
*/
static void CG_DrawHealth( menuDef_t *menuHUD )
{
	vec4_t			glowColor;
	playerState_t	*ps;
	int				healthAmt, maxAmt;
	//int				i,currValue,inc;
	double			percentage, factor;
	itemDef_t		*focusItem;

	const char *text;
	int x;

	static double	fcurrent = 0;
	vec4_t	fadecolor = {1, 1, 1, 0.5f};
	static vec4_t	draincolor = {1, 0.4f, 0.4f, 1};
	static vec4_t	fillcolor = {0.4f, 1, 0.4f, 1};
	vec4_t	opacity;

	int state = 0;

	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	ps = &cg.snap->ps;

	// What's the health?
	healthAmt = ps->stats[STAT_HEALTH];
	maxAmt = ps->stats[STAT_MAX_HEALTH];
	//if (healthAmt > ps->stats[STAT_MAX_HEALTH])
	//{
	//	healthAmt = ps->stats[STAT_MAX_HEALTH];
	//}

	focusItem = Menu_FindItemByName(menuHUD, "healthbar");

	if (focusItem)
	{
		percentage = (double)healthAmt / (double)maxAmt;
		if (percentage > 1) {
			percentage = 1;
		} else if (percentage < 0) {
			percentage = 0;
		}
		factor = /*0.62109375f **/ percentage /*+ 0.330078125f*/;
		/*if(factor > 0.95f)
		{
			factor = 1.0f; //eezstreet - mega hack
		}*/

		if (fcurrent < factor) {
			// Raise it
			fcurrent += (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent > factor) {
				// We passed it
				fcurrent = factor;
			}
		} else if (fcurrent > factor) {
			// Lower it
			fcurrent -= (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent < factor) {
				fcurrent = factor;
			}
		} else
		{
			//Stay the same
		}

		if (fcurrent != 0) {
			if (fcurrent < factor) {
				state = 1;
				// We're filling up, draw up to fcurrent solid, and up to factor translucent
				trap->R_SetColor(opacity);
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * fcurrent, focusItem->window.rect.h, 0, 0, fcurrent, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap->R_SetColor(fadecolor);
				trap->R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * fcurrent, focusItem->window.rect.y, focusItem->window.rect.w * (factor - fcurrent), focusItem->window.rect.h, fcurrent, 0, factor, 1, focusItem->window.background);
				trap->R_SetColor(NULL);
			} else if (fcurrent > factor) {
				state = 2;
				// We're draining, draw up to factor solid, and up to fcurrent translucent
				trap->R_SetColor(opacity);
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap->R_SetColor(fadecolor);
				trap->R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * factor, focusItem->window.rect.y, focusItem->window.rect.w * (fcurrent - factor), focusItem->window.rect.h, factor, 0, fcurrent, 1, focusItem->window.background);
				trap->R_SetColor(NULL);
			} else {
				state = 0;
				// Just solid
				trap->R_SetColor(opacity);
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
			}
		}
	}

	focusItem = Menu_FindItemByName(menuHUD, "healthtext");

	if (focusItem)
	{
		// Center and draw the text, apply glow if needed
		if (state == 1) {
			VectorCopy4(fillcolor, glowColor);
		} else if (state == 2) {
			VectorCopy4(draincolor, glowColor);
		} else {
			VectorCopy4(colorWhite, glowColor);
			if (healthAmt < 25) {
				// Glow the text from red to white
				float fade = 0.5f + cos((float)cg.time/250) *.5;
				glowColor[1] = glowColor[2] = fade;
			}
		}
		glowColor[3] *= cg.jkg_HUDOpacity;
		text = va("%i / %i", healthAmt, maxAmt);
		x = ((focusItem->window.rect.w/2) - (trap->R_Font_StrLenPixels(text, cgs.media.hudfont1, 0.6f) / 2)) + focusItem->window.rect.x;
		trap->R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			text,
			glowColor,
			cgs.media.hudfont1,
			-1,
			0.6f);
		trap->R_SetColor(NULL);
	}
}

/*
================
CG_DrawShield
================
*/
static void CG_DrawShield( menuDef_t *menuHUD )
{
	//vec4_t			calcColor;
	vec4_t			glowColor;
	playerState_t	*ps;
	int				shield, maxShield;
	itemDef_t		*focusItem;
	double			percentage, factor ; //quarterArmor;
	//int				i,currValue,inc;

	const char *text;
	int x;

	static double	fcurrent = 0;
	vec4_t	fadecolor = {1, 1, 1, 0.5f};
	static vec4_t	draincolor = {1, 0.4f, 0.4f, 1};
	static vec4_t	fillcolor = {0.4f, 1, 0.4f, 1};
	vec4_t	opacity;

	int state = 0;

	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	//ps = &cg.snap->ps;
	ps = &cg.predictedPlayerState;

	// Can we find the menu?
	if (!menuHUD)
	{
		return;
	}

	shield = ps->stats[STAT_SHIELD];
	maxShield = ps->stats[STAT_MAX_SHIELD];


	// TEST: just render the whole thing for now, we'll fix it later
	focusItem = Menu_FindItemByName(menuHUD, "shieldbar");

	if (focusItem)
	{
		percentage = (double)shield / (double)maxShield;
		if (percentage > 1) {
			percentage = 1;
		} else if (percentage < 0) {
			percentage = 0;
		}
		factor = /*0.6171875f **/ percentage /*+ 0.34375f*/;
		/*if(factor > 0.95f)
		{
			factor = 1.0f; //eezstreet - mega hack
		}*/
		// Fade our fcurrent to this factor
		if (fcurrent < factor) {
			// Raise it
			fcurrent += (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent > factor) {
				// We passed it
				fcurrent = factor;
			}
		} else if (fcurrent > factor) {
			// Lower it
			fcurrent -= (cg.frameDelta * .0003); // Go up 30% per second
			if (fcurrent < factor) {
				fcurrent = factor;
			}
		}

		if (fcurrent != 0) {
			if (fcurrent < factor) {
				state = 1;
				// We're filling up, draw up to fcurrent solid, and up to factor translucent
				trap->R_SetColor(opacity);
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * fcurrent, focusItem->window.rect.h, 0, 0, fcurrent, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap->R_SetColor(fadecolor);
				trap->R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * fcurrent, focusItem->window.rect.y, focusItem->window.rect.w * (factor - fcurrent), focusItem->window.rect.h, fcurrent, 0, factor, 1, focusItem->window.background);
				trap->R_SetColor(opacity);
			} else if (fcurrent > factor) {
				state = 2;
				// We're draining, draw up to factor solid, and up to fcurrent translucent
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
				fadecolor[3] *= cg.jkg_HUDOpacity;
				trap->R_SetColor(fadecolor);
				trap->R_DrawStretchPic(focusItem->window.rect.x + focusItem->window.rect.w * factor, focusItem->window.rect.y, focusItem->window.rect.w * (fcurrent - factor), focusItem->window.rect.h, factor, 0, fcurrent, 1, focusItem->window.background);
				trap->R_SetColor(opacity);
			} else {
				state = 0;
				// Just solid
				trap->R_SetColor(opacity);
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w * factor, focusItem->window.rect.h, 0, 0, factor, 1, focusItem->window.background);
			}
		}
	}


	if (!shield) {
		return;
	}

	focusItem = Menu_FindItemByName(menuHUD, "shieldtext");

	if (focusItem)
	{
		if (state == 1) {
			VectorCopy4(fillcolor, glowColor);
		} else if (state == 2) {
			VectorCopy4(draincolor, glowColor);
		} else {
			VectorCopy4(colorWhite, glowColor);
		}
		glowColor[3] *= cg.jkg_HUDOpacity;
		// Center and draw the text, positioning will be finetuned later on :P
		text = va("%i / %i", shield, maxShield);
		x = ((focusItem->window.rect.w/2) - (trap->R_Font_StrLenPixels(text, cgs.media.hudfont1, 0.6f) / 2)) + focusItem->window.rect.x;
		trap->R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			text,
			glowColor,
			cgs.media.hudfont1,
			-1,
			0.6f);
		trap->R_SetColor(NULL);
	}
}

static void CG_DrawTopLeftHUD ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;

    if (!menuHUD)
	{
	    return;
	}

	// Print background of the bars
	/*focusItem = Menu_FindItemByName(menuHUD, "barsbackground");
	if (focusItem)
	{
		trap->R_SetColor(opacity);	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}*/

	if (cg.predictedPlayerState.pm_type != PM_SPECTATOR)
	{
		CG_DrawShield(menuHUD);
		CG_DrawHealth(menuHUD);
		CG_DrawForcePower(menuHUD);
		JKG_DrawFiringMode(menuHUD);
		JKG_DrawAmmoType(menuHUD);

		playerState_t* ps;
		ps = &cg.predictedPlayerState;
		static vec4_t fullcolor = { 0.2f, 1, 0.3f, 1 };		//use this color when the resource is full (greenish)

		//if the UI element is supposed to be transparent/hidden, override
		if(opacity[3] < 1)
			fullcolor[3] = opacity[3];

		focusItem = Menu_FindItemByName(menuHUD, "frame");
		if (focusItem)
		{
			trap->R_SetColor(opacity);	
			CG_DrawPic( 
				focusItem->window.rect.x, 
				focusItem->window.rect.y, 
				focusItem->window.rect.w, 
				focusItem->window.rect.h, 
				focusItem->window.background 
				);			
		}

		focusItem = Menu_FindItemByName(menuHUD, "hudicon_shield");
		if (focusItem)
		{
			int	shield = ps->stats[STAT_SHIELD];
			int maxShield = ps->stats[STAT_MAX_SHIELD];

			if(shield > 0 && shield >= maxShield)
				trap->R_SetColor(fullcolor);
			else
				trap->R_SetColor(opacity);
			
			CG_DrawPic( 
				focusItem->window.rect.x, 
				focusItem->window.rect.y, 
				focusItem->window.rect.w, 
				focusItem->window.rect.h, 
				focusItem->window.background 
				);			
		}

		focusItem = Menu_FindItemByName(menuHUD, "hudicon_health");
		if (focusItem)
		{
			int	hp = ps->stats[STAT_HEALTH];
			int maxhp = ps->stats[STAT_MAX_HEALTH];

			if (hp > 0 && hp >= maxhp)
				trap->R_SetColor(fullcolor);
			else
				trap->R_SetColor(opacity);

			CG_DrawPic( 
				focusItem->window.rect.x, 
				focusItem->window.rect.y, 
				focusItem->window.rect.w, 
				focusItem->window.rect.h, 
				focusItem->window.background 
				);			
		}

		focusItem = Menu_FindItemByName(menuHUD, "hudicon_stamina");
		if (focusItem)
		{
			int	stamina = ps->forcePower;
			int maxstamina = ps->stats[STAT_MAX_STAMINA];

			if (stamina > 0 && stamina >= maxstamina)
				trap->R_SetColor(fullcolor);
			else
				trap->R_SetColor(opacity);

			CG_DrawPic( 
				focusItem->window.rect.x, 
				focusItem->window.rect.y, 
				focusItem->window.rect.w, 
				focusItem->window.rect.h, 
				focusItem->window.background 
				);			
		}
	}

	// Put in the avatar before we put on the frame
	/*focusItem = Menu_FindItemByName(menuHUD, "avatar");
	if (focusItem)
	{
		trap->R_SetColor(opacity);	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			cgs.media.avatar_placeholder
			);	
	}	*/	
	// Print frame

	// Print level
	/*focusItem = Menu_FindItemByName(menuHUD, "leveltext");
	if (focusItem)
	{
		const char *temp = va("%i", 0);
		int x = ((focusItem->window.rect.w/2) - (trap->R_Font_StrLenPixels(temp, MenuFontToHandle(1), 0.6f) / 2)) + focusItem->window.rect.x;
		//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		trap->R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			temp,
			opacity,//colorWhite,
			MenuFontToHandle(1),
			-1,
			0.6f);
	}*/

	/*focusItem = Menu_FindItemByName(menuHUD, "nametext");
	if (focusItem)
	{
		const char *temp = va("%s", Info_ValueForKey(CG_ConfigString(CS_PLAYERS + cg.snap->ps.clientNum), "n"));
		//CG_DrawRect(focusItem->window.rect.x,focusItem->window.rect.y,focusItem->window.rect.w,focusItem->window.rect.h,1,colorWhite);
		// To avoid rounding errors, we get the length at scale 1, then adjust it
		int x = ((focusItem->window.rect.w/2) - (((float)trap->R_Font_StrLenPixels(temp, MenuFontToHandle(1), 1) * 0.5f) / 2)) + focusItem->window.rect.x;
		//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		ChatBox_SetPaletteAlpha(opacity[3]);
		trap->R_Font_DrawString(
			x,
			focusItem->window.rect.y,
			temp,
			opacity,//colorWhite,
			MenuFontToHandle(1),
			-1,
			0.5f);
		ChatBox_SetPaletteAlpha(1);
	}*/
}

// Convert HSL to RGB
void HSL2RGB(float h, float s, float l, float *r, float *g, float *b) {
	double tr, tg, tb;
	double v;

	tr = l;
	tg = l;
	tb = l;
	v = (l <= 0.5f) ? (l * (1.0f + (l * 2))) : (l + l - l * l);
	if (v > 0) {
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m) / v;
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
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	16
#define STYLE_DROPSHADOW	0x80000000
static void CG_DrawFPS( float x, float y, float w, float h, int font, float textScale ) {
	char		*s;
	static unsigned short previousTimes[FPS_FRAMES];
	static unsigned short index;
	static int	previous, lastupdate;
	int		t, i, fps, total;
	unsigned short frameTime;
#ifdef _XBOX
	const int		xOffset = -40;
#else
	//const int		xOffset = 0;
#endif
	vec4_t	fpscolor;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap->Milliseconds();
	frameTime = t - previous;
	previous = t;
	if (t - lastupdate > 50)	//don't sample faster than this
	{
		lastupdate = t;
		previousTimes[index % FPS_FRAMES] = frameTime;
		index++;
	}
	// average multiple frames together to smooth changes out a bit
	total = 0;
	for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
		total += previousTimes[i];
	}
	if ( !total ) {
		total = 1;
	}
	fps = 1000 * FPS_FRAMES / total;

    s = va( "%i fps", fps );
    if (cg_drawFPS.integer == 2) {
        Q_strcat (s, 64, va ("\n%.3f mspf", (float)total / (float)FPS_FRAMES));
    }
	if (fps < 10) {
		VectorSet(fpscolor, 1, 0, 0);
	} else if (fps > 50) {
		VectorSet(fpscolor, 0, 1, 0);
	} else {
		int hue = (fps - 10) * 3; //(0 to 120)
		HSL2RGB((float)hue/360, 1, 0.5f, &fpscolor[0], &fpscolor[1], &fpscolor[2]);
	}
	fpscolor[3] = 1;

	//w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	trap->R_Font_DrawString(x, y, s, fpscolor, font | STYLE_DROPSHADOW, -1, textScale);
	//CG_DrawBigString( 635 - w + xOffset, y + 2, s, 1.0F);
}

static void CG_DrawMiniMap ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;

    if (!menuHUD)
	{
	    return;
	}	

	// Render the minimap
	// Use a default fixed radius of 500 units for now
	if (cgs.gametype != GT_DUEL) 
	{
		MiniMap_Render(menuHUD, 1500.0f);
	}

	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap->R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}

	//Render the credit display
	focusItem = Menu_FindItemByName(menuHUD, "credits");
	if (focusItem && cgs.gametype != GT_DUEL)
	{
		trap->R_SetColor(opacity);
		trap->R_Font_DrawString(focusItem->window.rect.x, focusItem->window.rect.y, va("Credits: %i", cg.predictedPlayerState.credits), opacity, cgDC.Assets.qhSmall3Font, -1, focusItem->textscale);
	}

	focusItem = Menu_FindItemByName(menuHUD, "smalltext");
	if(focusItem)
	{
		char buffer[1024];
		int mins, sec, msec;
		int numberItems = 0;
		buffer[0] = '\0';
		trap->R_SetColor(opacity);
		if(cg_drawTimer.integer == 1 || cg_drawTimer.integer == 3)
		{
			//Draw server time
			msec = cg.time - cgs.levelStartTime;
			sec = msec/1000;

			//Convert to mm:ss format
			mins = floor((float)sec/60);
			sec -= (mins * 60);

			strcat(buffer, va("Timer: %.2i:%.2i ", mins, sec));
			numberItems++;
		}
		if(cg_drawTimer.integer == 2 || cg_drawTimer.integer == 3)
		{
			//Add a slash
			if(numberItems > 0)
			{
				strcat(buffer, "/ ");
				numberItems++;
			}
			if(T_meridiem())
			{
				strcat(buffer, va("Clock: %.2i:%.2i PM ", T_hour(qfalse), T_minute()));
			}
			else
			{
				strcat(buffer, va("Clock: %.2i:%.2i AM ", T_hour(qfalse), T_minute()));
			}
		}

		//TODO: Add more shite

		//strcat(buffer, '\0');

		trap->R_Font_DrawString(focusItem->window.rect.x, focusItem->window.rect.y, buffer, opacity, cgDC.Assets.qhSmall3Font, -1, focusItem->textscale);
	}

	if(cg_drawFPS.integer > 0)
	{
		focusItem = Menu_FindItemByName(menuHUD, "fps");
		if(focusItem)
		{
			CG_DrawFPS(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, focusItem->iMenuFont, focusItem->textscale);
		}
	}
}

static void CG_DrawHotkeyBar ( menuDef_t *menuHUD, vec4_t opacity )
{
    itemDef_t *focusItem;
	int i;

    if (!menuHUD)
	{
	    return;
	}

	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap->R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}

	// Print background of the bars
	for (i=0; i<MAX_ACI_SLOTS; i++) {
		if (cg.playerACI[i] >= cg.playerInventory->size()) {
			cg.playerACI[i] = -1;
		}

		if (cg.playerACI[i] < 0) {
			continue;
		}

		itemInstance_t* playerItem = &(*cg.playerInventory)[cg.playerACI[i]];
		focusItem = Menu_FindItemByName(menuHUD, va("slot%i", i));
		if (focusItem)
		{
			vec4_t col = {0.11f, 0.11f, 0.11f, 1.0f};
			qhandle_t shader = cgs.media.whiteShader;	//dummy
			col[3] *= cg.jkg_HUDOpacity;
			if ( cg.playerACI[i] >= 0 && (*cg.playerInventory)[cg.playerACI[i]].id && (*cg.playerInventory)[cg.playerACI[i]].id->itemID )
			{
			    int weapon, variation;
				if(playerItem->id->itemType == ITEM_WEAPON)
				{
					if ( BG_GetWeaponByIndex (playerItem->id->weaponData.varID, &weapon, &variation) )
					{
						const weaponInfo_t *weaponInfo = CG_WeaponInfo (weapon, variation);
						shader = weaponInfo->hudIcon;
						col[0] = 1.0f;
						col[1] = 1.0f;
						col[2] = 1.0f;
				        
						/*trap->R_SetColor (colorTable[CT_MDGREY]);
						trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 0, 0, 1, 1, cgs.media.whiteShader);*/
						if(i == cg.weaponSelect)
						{
							trap->R_SetColor (opacity);
							//TODO: precache me!
							trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 0, 0, 1, 1, trap->R_RegisterShaderNoMip("gfx/jkghud/aciselect"));
						}
					}
				}
				else
				{
					col[0] = 1.0f;
					col[1] = 1.0f;
					col[2] = 1.0f;
					shader = trap->R_RegisterShaderNoMip(playerItem->id->visuals.itemIcon);
				}
			}
			if(shader != cgs.media.whiteShader)
			{
				trap->R_SetColor( col );
				trap->R_DrawStretchPic(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 0, 0, 1, 1, shader);
			}
			//CG_DrawRect(focusItem->window.rect.x, focusItem->window.rect.y, focusItem->window.rect.w, focusItem->window.rect.h, 1, colorWhite);
		}

		focusItem = Menu_FindItemByName(menuHUD, va("slotl%i", i));
		if (focusItem)
		{
			trap->R_Font_DrawString(focusItem->window.rect.x, focusItem->window.rect.y, va("%i", i), opacity, cgDC.Assets.qhSmallFont, -1, 0.4f);
		}

		focusItem = Menu_FindItemByName(menuHUD, va("slotq%i", i));
		if (focusItem && playerItem->id->maxStack > 1) {
			char* text = va("%i", playerItem->quantity);
			vec4_t color{ 0.52f, 0.65f, 0.96f, opacity[3] };
			trap->R_Font_DrawString(focusItem->window.rect.x - trap->R_Font_StrLenPixels(text, cgDC.Assets.qhSmallFont, 0.4f),
				focusItem->window.rect.y, text, color, cgDC.Assets.qhSmallFont, -1, 0.4f);
		}
	}

	focusItem = Menu_FindItemByName(menuHUD, "frame_overlay");
	if(focusItem)
	{
		trap->R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);	
	}
}

static void CG_DrawBottomRightHUD ( menuDef_t *menuHUD, centity_t *cent, vec4_t opacity )
{
	itemDef_t *focusItem;
	if (!menuHUD)
	{
	    return;
	}

	focusItem = Menu_FindItemByName(menuHUD, "framebg");
	if (focusItem)
	{
		vec4_t tmp;
		MAKERGBA(tmp,0,0,0, cg.jkg_HUDOpacity);
		trap->R_SetColor( tmp );	
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			cgs.media.whiteShader 
			);			
	}

	// Draw the bars (temp)
	
	CG_DrawJetpackCloak(menuHUD);
	
	focusItem = Menu_FindItemByName(menuHUD, "frame");
	if (focusItem)
	{
		trap->R_SetColor(opacity);
		CG_DrawPic( 
			focusItem->window.rect.x, 
			focusItem->window.rect.y, 
			focusItem->window.rect.w, 
			focusItem->window.rect.h, 
			focusItem->window.background 
			);			
	}

	// Draw ammo tics or saber style
	if ( cent->currentState.weapon == WP_SABER )
	{
		CG_DrawSaberStyle(cent,menuHUD);
	}
	else
	{
		CG_DrawAmmo(cent,menuHUD);
	}
}

/*
================
CG_DrawHUD
================
*/
void CG_DrawHUD(centity_t *cent)
{
	menuDef_t	*menuHUD = NULL;
	vec4_t	opacity		=	{1, 1, 1, 1};
	AdjustOpacityLevels();
	MAKERGBA(opacity, 1, 1, 1, cg.jkg_HUDOpacity);

	if (cg.captureFlagPercent > 0 && cg.captureFlagPercent < 100 && cg.capturingFlag)
	{// For attack/defence/scenario gametype flag captures...
		int x = 600;
		int y = 154;
		vec3_t color;

		if (cg.captureFlagPercent > 100)
			cg.captureFlagPercent = 100;

		FlagCaptureBar();

		VectorSet(color, 250, 100, 1);

		if (next_color_update < cg.time)
		{
			// Cycle writing color...
			if (color_forwards)
			{
				if (color_selector >= 250)
				{
					color_forwards = qfalse;
					color_selector--;
				}
				else
				{
					color_selector++;
				}
			}
			else
			{
				if (color_selector <= 50)
				{
					color_forwards = qtrue;
					color_selector++;
				}
				else
				{
					color_selector--;
				}
			}

			next_color_update = cg.time + 10;
		}

		color[0] = color_selector;
		color[1] = color_selector;
		color[2] = 1;

		if (cg.recaptureingFlag)
		{
			x-=15;
			UI_DrawScaledProportionalString(x, y, "Consolidating Control Point", UI_RIGHT|UI_DROPSHADOW, color, 0.4);
		}
		else
			UI_DrawScaledProportionalString(x, y, "Capturing Control Point", UI_RIGHT|UI_DROPSHADOW, color, 0.4);
	}
	else if (cg.captureFlagPercent >= 100 && cg.capturingFlag)
	{// For attack/defence/scenario gametype flag captures...
		int x = 600;
		int y = 154;
		vec3_t color;

		if (cg.captureFlagPercent > 100)
			cg.captureFlagPercent = 100;

		FlagCaptureBar();

		VectorSet(color, 250, 250, 0);

		UI_DrawScaledProportionalString(x, y, "^3Control Point Captured!", UI_RIGHT|UI_DROPSHADOW, color, 0.4);
	}

	if (cgs.gametype >= GT_TEAM)
	{	// tint the hud items based on team
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
			hudTintColor = redhudtint;
		else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
			hudTintColor = bluehudtint;
		else // If we're not on a team for whatever reason, leave things as they are.
			hudTintColor = colorTable[CT_WHITE];
	}
	else
	{	// tint the hud items white (dont' tint)
		hudTintColor = colorTable[CT_WHITE];
	}

	// Jedi Knight Galaxies -- Put in our own HUD components here

	// Render the top-left stuff
	menuHUD = Menus_FindByName("hud_topleft");
	CG_DrawTopLeftHUD (menuHUD, opacity);

	// Render the minimap
	if (cgs.gametype != GT_DUEL) {
	menuHUD = Menus_FindByName("hud_minimap");
	CG_DrawMiniMap (menuHUD, opacity);
	}

	// Render the hotkey bar
	if (cgs.gametype != GT_DUEL) {
		menuHUD = Menus_FindByName("hud_hotkey");
		CG_DrawHotkeyBar (menuHUD, opacity);
	}

	menuHUD = Menus_FindByName("hud_bottomright");
	CG_DrawBottomRightHUD (menuHUD, cent, opacity);
}
