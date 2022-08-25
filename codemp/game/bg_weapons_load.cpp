#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_damage.h"
#include <json/cJSON.h>
#include "../cgame/animtable.h"

#if defined(_GAME)
	#include "g_local.h"
#elif defined(_CGAME)
	#include "cgame/cg_local.h"
#elif defined(IN_UI)
	#include "ui/ui_local.h"
#endif

static int fmLoadCounter;

static void BG_ParseWeaponStatsFlags ( weaponData_t *weaponData, const char *flagStr )
{
    if ( Q_stricmp (flagStr, "cookable") == 0 )
    {
        weaponData->hasCookAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "knockback") == 0 )
    {
        weaponData->hasKnockBack = qtrue;
    }
    else if ( Q_stricmp (flagStr, "roll") == 0 )
    {
        weaponData->hasRollAbility = qtrue;
    }
    else if ( Q_stricmp (flagStr, "zoom") == 0 )
    {
        weaponData->zoomType = ZOOM_CONTINUOUS;
    }
    else if ( Q_stricmp (flagStr, "togglezoom") == 0 )
    {
        weaponData->zoomType = ZOOM_TOGGLE;
    }
}

static void BG_ParseFireModeFiringType ( weaponFireModeStats_t *fireMode, const char *firingTypeStr )
{
    if ( Q_stricmp (firingTypeStr, "auto") == 0 )
    {
        fireMode->firingType = FT_AUTOMATIC;
    }
    else if ( Q_stricmp (firingTypeStr, "semi") == 0 )
    {
        fireMode->firingType = FT_SEMI;
    }
    else if ( Q_stricmp (firingTypeStr, "burst") == 0 )
    {
        fireMode->firingType = FT_BURST;
    }
}

#ifdef _GAME
#include "jkg_damageareas.h"

static int BG_GetDamageFlagFromString(const char* szDamageFlagString)
{
	if (!Q_stricmp(szDamageFlagString, "DAMAGE_RADIUS"))
	{
		return DAMAGE_RADIUS;
	}
	else if (!Q_stricmp(szDamageFlagString, "DAMAGE_NO_SHIELD"))
	{
		return DAMAGE_NO_SHIELD;
	}
	else if (!Q_stricmp(szDamageFlagString, "DAMAGE_NO_KNOCKBACK"))
	{
		return DAMAGE_NO_KNOCKBACK;
	}
	else if (!Q_stricmp(szDamageFlagString, "DAMAGE_NO_PROTECTION"))
	{
		return DAMAGE_NO_PROTECTION;
	}
	else if (!Q_stricmp(szDamageFlagString, "DAMAGE_NO_HIT_LOC"))
	{
		return DAMAGE_NO_HIT_LOC;
	}
	else if (!Q_stricmp(szDamageFlagString, "DAMAGE_NO_DISMEMBER"))
	{
		return DAMAGE_NO_DISMEMBER;
	}
	return DAMAGE_NORMAL;
}

static void BG_ParseDamage ( weaponFireModeStats_t *fireModeStats, cJSON *damageNode, qboolean secondary )
{
    if ( !damageNode )
    {
        return;
    }
    
    if ( !secondary && cJSON_IsNumber (damageNode) )
    {
        fireModeStats->baseDamage = (short)cJSON_ToIntegerOpt (damageNode, 0);
		fireModeStats->primary.damage = fireModeStats->baseDamage;
    }
	else if (secondary && cJSON_IsNumber(damageNode))
	{
		fireModeStats->secondary.damage = cJSON_ToIntegerOpt(damageNode, 0);
		fireModeStats->secondaryDmgPresent = qtrue;
	}
    else if ( cJSON_IsObject (damageNode) )
    {
        damageSettings_t darea;
        cJSON *node = NULL;
        
        memset (&darea, 0, sizeof (darea));
        
        node = cJSON_GetObjectItem (damageNode, "damageradius");
        if ( node )
        {
            cJSON *child = NULL;
            const char *s = NULL;
            
            darea.radial = qtrue;
            
            child = cJSON_GetObjectItem (node, "start");
            darea.radiusParams.startRadius = (float)cJSON_ToNumber (child);
            
            child = cJSON_GetObjectItem (node, "end");
            darea.radiusParams.endRadius = (float)cJSON_ToNumber (child);
            
            child = cJSON_GetObjectItem (node, "parm");
            darea.radiusParams.generic1 = cJSON_ToInteger (child);
            
            child = cJSON_GetObjectItem (node, "falloff");
            s = cJSON_ToStringOpt (child, "constant");
            if ( Q_stricmp (s, "constant") == 0 )
                darea.radiusParams.damageFunc = DF_CONSTANT;
            else if ( Q_stricmp (s, "linear") == 0 )
                darea.radiusParams.damageFunc = DF_LINEAR;
            else if ( Q_stricmp (s, "gaussian") == 0 )
                darea.radiusParams.damageFunc = DF_GAUSSIAN;
            else
            {
                darea.radiusParams.damageFunc = DF_CONSTANT;
                Com_Printf ("Unknown damage falloff type used: %s. Defaulting to linear falloff.\n", s);
            }
                
            child = cJSON_GetObjectItem (node, "function");
            s = cJSON_ToStringOpt (child, "constant");
            if ( Q_stricmp (s, "linear") == 0 )
                darea.radiusParams.radiusFunc = RF_LINEAR;
            else if ( Q_stricmp (s, "nonlinear") == 0 )
                darea.radiusParams.radiusFunc = RF_NONLINEAR;
            else if ( Q_stricmp (s, "clamp") == 0 )
                darea.radiusParams.radiusFunc = RF_CLAMP;
            else if ( Q_stricmp (s, "wave") == 0 )
                darea.radiusParams.radiusFunc = RF_WAVE;
            else if ( Q_stricmp (s, "constant") == 0 )
                darea.radiusParams.radiusFunc = RF_CONSTANT;
            else
            {
                darea.radiusParams.radiusFunc = RF_CONSTANT;
                Com_Printf ("Unknown radius function used: %s; Defaulting to constant radius.\n", s);
            }
        }
        
        node = cJSON_GetObjectItem (damageNode, "duration");
        darea.lifetime = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "delay");
        darea.delay = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "damage");
        darea.damage = cJSON_ToIntegerOpt (node, 0);
        fireModeStats->baseDamage = darea.damage;
        
        node = cJSON_GetObjectItem (damageNode, "damagedelay");
        darea.damageDelay = cJSON_ToIntegerOpt (node, 0);
        
        node = cJSON_GetObjectItem (damageNode, "penetration");
        switch ( cJSON_ToIntegerOpt (node, 0) )
        {
            default:
            case 0: darea.penetrationType = PT_NONE; break;
			// not used
            //case 1: darea.penetrationType = PT_SHIELD; break;
            //case 2: darea.penetrationType = PT_SHIELD_ARMOR; break;
            case 3: darea.penetrationType = PT_WALLS; break;
        }

		node = cJSON_GetObjectItem(damageNode, "flags");
		for (int i = 0; i < cJSON_GetArraySize(node); i++)
		{
			darea.damageFlags |= BG_GetDamageFlagFromString(cJSON_ToStringOpt(cJSON_GetArrayItem(node, i), ""));
		}

		node = cJSON_GetObjectItem(damageNode, "buffs");
		if (!node)
		{
			darea.numberDebuffs = 0;
		}
		else
		{
			darea.numberDebuffs = cJSON_GetArraySize(node);
			for (int i = 0; i < darea.numberDebuffs; i++)
			{
				cJSON* arrayNode = cJSON_GetArrayItem(node, i);
				cJSON* childNode = cJSON_GetObjectItem(arrayNode, "buff");
				darea.debuffs[i].debuff = JKG_ResolveBuffName(cJSON_ToStringOpt(childNode, ""));

				childNode = cJSON_GetObjectItem(arrayNode, "duration");
				darea.debuffs[i].duration = cJSON_ToIntegerOpt(childNode, 2000);

				childNode = cJSON_GetObjectItem(arrayNode, "intensity");
				darea.debuffs[i].intensity = cJSON_ToNumberOpt(childNode, 1.0);
			}
		}

		if (secondary)
		{
			fireModeStats->secondary = darea;
			fireModeStats->secondaryDmgPresent = qtrue;
		}
		else
		{
			fireModeStats->primary = darea;
		}
    }
}
#endif

static void BG_ParseWeaponFireMode ( weaponFireModeStats_t *fireModeStats, cJSON *fireModeNode )
{
    cJSON *node;
    const char *str = NULL;
    
    if ( fireModeNode == NULL )
    {
        return;
    }

    node = cJSON_GetObjectItem (fireModeNode, "damage");
#ifdef _GAME
    BG_ParseDamage (fireModeStats, node, qfalse);
    
    node = cJSON_GetObjectItem (fireModeNode, "secondarydamage");
    BG_ParseDamage (fireModeStats, node, qtrue);
#else
	fireModeStats->baseDamage = cJSON_ToInteger (node);
#endif
    
    node = cJSON_GetObjectItem (fireModeNode, "grenade");
    fireModeStats->isGrenade = (qboolean)cJSON_ToBooleanOpt (node, 0);

	node = cJSON_GetObjectItem (fireModeNode, "grenadeBounces");
    fireModeStats->grenadeBounces = (qboolean)cJSON_ToBooleanOpt (node, 1);

	node = cJSON_GetObjectItem (fireModeNode, "grenadeBounceDMG");
    fireModeStats->grenadeBounceDMG = (char)cJSON_ToIntegerOpt (node, 10);
    
    node = cJSON_GetObjectItem (fireModeNode, "ballistic");
    fireModeStats->applyGravity = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "bounces");
    fireModeStats->bounceCount = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "hitscan");
    fireModeStats->hitscan = (char)cJSON_ToBooleanOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectiles");
    fireModeStats->shotCount = (char)cJSON_ToIntegerOpt (node, 1);
    
    node = cJSON_GetObjectItem (fireModeNode, "collisionsize");
    fireModeStats->boxSize = (float)cJSON_ToNumberOpt (node, 0.0);

	node = cJSON_GetObjectItem (fireModeNode, "ammocost");
    fireModeStats->cost = (char)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxchargetime");
    fireModeStats->chargeMaximum = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedamage");
    fireModeStats->chargeMultiplier = (float)cJSON_ToNumberOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "chargedelay");
    fireModeStats->chargeTime = (short)cJSON_ToIntegerOpt (node, 0);

	node = cJSON_GetObjectItem (fireModeNode, "chargesubtract");
	fireModeStats->chargeSubtract = cJSON_ToIntegerOpt(node, fireModeStats->cost);
    
    node = cJSON_GetObjectItem (fireModeNode, "firingtype");
    BG_ParseFireModeFiringType (fireModeStats, cJSON_ToStringOpt (node, "auto"));
    
    // 0 means fully automatic, otherwise one burst will fire weapon n times.
    node = cJSON_GetObjectItem (fireModeNode, "shotsperburst");
    fireModeStats->shotsPerBurst = (char)cJSON_ToIntegerOpt (node, 0);
    
    // 0 means infinite delay (semi-automatic), otherwise n milliseconds between rounds in a burst.
    node = cJSON_GetObjectItem (fireModeNode, "burstshotdelay");
    fireModeStats->burstFireDelay = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "firedelay");
    fireModeStats->delay = (short)cJSON_ToIntegerOpt (node, 0);
    
    node = cJSON_GetObjectItem (fireModeNode, "range");
    fireModeStats->range = (float)cJSON_ToIntegerOpt (node, WPR_M);

    node = cJSON_GetObjectItem(fireModeNode, "decayRate");
    fireModeStats->decayRate = (float)cJSON_ToNumberOpt(node, 0.25);
    if (fireModeStats->decayRate > 1) //check for invalid decayRates
        fireModeStats->decayRate = 1;
    if (fireModeStats->decayRate < 0)
        fireModeStats->decayRate = 0;

    node = cJSON_GetObjectItem (fireModeNode, "splashrange");
    fireModeStats->rangeSplash = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "recoil");
    fireModeStats->recoil = (float)cJSON_ToNumberOpt (node, 0.0);

	node = cJSON_GetObjectItem(fireModeNode, "heatGenerated");
	fireModeStats->heatGenerated = (float)cJSON_ToNumberOpt(node, 0.0f);

	node = cJSON_GetObjectItem(fireModeNode, "maxHeat");
	fireModeStats->maxHeat = (unsigned int)cJSON_ToNumberOpt(node, 100);

    node = cJSON_GetObjectItem(fireModeNode, "heatThreshold");
    fireModeStats->heatThreshold = (unsigned int)cJSON_ToNumberOpt(node, (fireModeStats->maxHeat * 0.75)); //defaults to 75% of maxHeat

    node = cJSON_GetObjectItem(fireModeNode, "reloadClearHeat");
    fireModeStats->reloadClearHeat = (unsigned int)cJSON_ToNumberOpt(node, 50); //default is 50% (0 = no heat cleared, 100% of heat cleared)
    if (fireModeStats->reloadClearHeat > 100)
        fireModeStats->reloadClearHeat = 100;
    if (fireModeStats->reloadClearHeat < 0)
        fireModeStats->reloadClearHeat = 0;

	node = cJSON_GetObjectItem(fireModeNode, "clipSize");
	fireModeStats->clipSize = (unsigned int)cJSON_ToIntegerOpt(node, 0);

	node = cJSON_GetObjectItem (fireModeNode, "accuracy");
	if( node )
	{
		cJSON *child = NULL;

		child = cJSON_GetObjectItem( node, "accuracyRating" );
		fireModeStats->weaponAccuracy.accuracyRating = (float)cJSON_ToNumberOpt( child, 32.0f );

		child = cJSON_GetObjectItem( node, "crouchModifier" );
		fireModeStats->weaponAccuracy.crouchModifier = (float)cJSON_ToNumberOpt( child, 0.8f );

		child = cJSON_GetObjectItem( node, "runModifier" );
		fireModeStats->weaponAccuracy.runModifier = (float)cJSON_ToNumberOpt( child, 2.0f );

		child = cJSON_GetObjectItem( node, "sightsModifier" );
		fireModeStats->weaponAccuracy.sightsModifier = (float)cJSON_ToNumberOpt( child, 0.2f );

		child = cJSON_GetObjectItem( node, "walkModifier" );
		fireModeStats->weaponAccuracy.walkModifier = (float)cJSON_ToNumberOpt( child, 1.55f );

		child = cJSON_GetObjectItem( node, "inAirModifier" );
		fireModeStats->weaponAccuracy.inAirModifier = (float)cJSON_ToNumberOpt( child, 3.0f );

		child = cJSON_GetObjectItem( node, "accuracyRatingPerShot" );
		fireModeStats->weaponAccuracy.accuracyRatingPerShot = (int)cJSON_ToNumberOpt( child, 2 );

		child = cJSON_GetObjectItem( node, "msToDrainAccuracy" );
		fireModeStats->weaponAccuracy.msToDrainAccuracy = (int)cJSON_ToNumberOpt( child, 200 );

		child = cJSON_GetObjectItem( node, "maxAccuracyAdd" );
		fireModeStats->weaponAccuracy.maxAccuracyAdd = (int)cJSON_ToNumberOpt( child, 128 );
	}
	else
	{
		fireModeStats->weaponAccuracy.accuracyRating = 32.0f;
		fireModeStats->weaponAccuracy.crouchModifier = 0.8f;
		fireModeStats->weaponAccuracy.runModifier = 2.0f;
		fireModeStats->weaponAccuracy.sightsModifier = 0.2f;
		fireModeStats->weaponAccuracy.walkModifier = 1.55f;
		fireModeStats->weaponAccuracy.inAirModifier = 3.0f;
		fireModeStats->weaponAccuracy.accuracyRatingPerShot = 2;
		fireModeStats->weaponAccuracy.msToDrainAccuracy = 200;
		fireModeStats->weaponAccuracy.maxAccuracyAdd = 128;
	}
    
    node = cJSON_GetObjectItem (fireModeNode, "projectilespeed");
    fireModeStats->speed = (float)cJSON_ToNumberOpt (node, 0.0);
    
    node = cJSON_GetObjectItem (fireModeNode, "projectileclass");
    str = cJSON_ToStringOpt (node, "unknown_proj");
    Q_strncpyz (fireModeStats->weaponClass, str, sizeof (fireModeStats->weaponClass));
    
    node = cJSON_GetObjectItem (fireModeNode, "meansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
	fireModeStats->weaponMOD = JKG_GetMeansOfDamageIndex(str);

    if (fireModeStats->weaponMOD == JKG_GetMeansOfDamageIndex("MOD_ACP") )
    {
        node = cJSON_GetObjectItem(fireModeNode, "ACPRatio");
        fireModeStats->ACPRatio = (float)cJSON_ToNumberOpt(node, 0.5f);
    }
    
    node = cJSON_GetObjectItem (fireModeNode, "splashmeansofdeath");
    str = cJSON_ToStringOpt (node, "MOD_UNKNOWN");
	fireModeStats->weaponSplashMOD = JKG_GetMeansOfDamageIndex(str);

	node = cJSON_GetObjectItem(fireModeNode, "useQuantity");
	fireModeStats->useQuantity = cJSON_ToBooleanOpt(node, qfalse);

	if (!fireModeStats->useQuantity) {
		node = cJSON_GetObjectItem(fireModeNode, "ammoBase");
		fireModeStats->ammoBase = BG_GetAmmo(cJSON_ToStringOpt(node, "AMMO_NONE"));

		node = cJSON_GetObjectItem(fireModeNode, "ammoDefault");
		fireModeStats->ammoDefault = BG_GetAmmo(cJSON_ToStringOpt(node, "AMMO_NONE"));
	}
}

//=========================================================
// BG_ParseWeaponStats
//---------------------------------------------------------
// Description:
// Parses the stats block of a weapons file.
//=========================================================
static void BG_ParseWeaponStats ( weaponData_t *weaponData, cJSON *statsNode )
{
    cJSON *node;
    const char *flags[4];
    
    if ( statsNode == NULL )
    {
        return;
    }

    node = cJSON_GetObjectItem (statsNode, "reloadtime");
    weaponData->weaponReloadTime = (unsigned short)cJSON_ToIntegerOpt (node, 0);

    node = cJSON_GetObjectItem (statsNode, "flags");
    if ( node != NULL )
    {
        int numFlags = cJSON_ReadStringArray (node, 4, flags);
        int i;

        for ( i = 0; i < numFlags; i++ )
        {
            BG_ParseWeaponStatsFlags (weaponData, flags[i]);
        }
    }

    node = cJSON_GetObjectItem (statsNode, "speed");
    weaponData->speedModifier = (float)cJSON_ToNumberOpt (node, 1.0);
        
    node = cJSON_GetObjectItem (statsNode, "reloadmodifier");
    weaponData->reloadModifier = (float)cJSON_ToNumberOpt (node, 1.0);

    node = cJSON_GetObjectItem(statsNode, "swaptime");
    weaponData->swapTime = cJSON_ToNumberOpt(node, 300);
    
    node = cJSON_GetObjectItem (statsNode, "startzoomfov");
    weaponData->startZoomFov = (float)cJSON_ToNumberOpt (node, 50.0);
    
    node = cJSON_GetObjectItem (statsNode, "endzoomfov");
    weaponData->endZoomFov = (float)cJSON_ToNumberOpt (node, weaponData->zoomType == ZOOM_TOGGLE ? weaponData->startZoomFov : 5.0);
    
    node = cJSON_GetObjectItem (statsNode, "zoomtime");
    weaponData->zoomTime = cJSON_ToIntegerOpt (node, 2000);

	node = cJSON_GetObjectItem (statsNode, "ironsightsTime");
	weaponData->ironsightsTime = cJSON_ToIntegerOpt (node, IRONSIGHTS_TIME);
}

static void BG_ParseAnimationObject ( cJSON *animationNode, int *torsoAnimation, int *legsAnimation )
{
    cJSON *node = cJSON_GetObjectItem (animationNode, "torso");
    const char *anim = cJSON_ToStringOpt (node, "BOTH_STAND1");
    
    *torsoAnimation = GetIDForString (animTable, anim);
    
    node = cJSON_GetObjectItem (animationNode, "legs");
    if ( node == NULL )
    {
        *legsAnimation = *torsoAnimation;
        return;
    }
    
    anim = cJSON_ToString (node);
    *legsAnimation = GetIDForString (animTable, anim);
}

static void BG_ParseWeaponPlayerAnimations ( weaponData_t *weaponData, cJSON *playerAnimNode )
{
    cJSON *node;
    
    node = cJSON_GetObjectItem (playerAnimNode, "ready");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.ready.torsoAnim, &weaponData->anims.ready.legsAnim);
	}
    
    node = cJSON_GetObjectItem (playerAnimNode, "firing");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.firing.torsoAnim, &weaponData->anims.firing.legsAnim);
	}
    
    node = cJSON_GetObjectItem (playerAnimNode, "reload");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.reload.torsoAnim, &weaponData->anims.reload.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "backwardWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.backwardWalk.torsoAnim, &weaponData->anims.backwardWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "forwardWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.forwardWalk.torsoAnim, &weaponData->anims.forwardWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "crouchWalk");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.crouchWalk.torsoAnim, &weaponData->anims.crouchWalk.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "crouchWalkBack");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.crouchWalkBack.torsoAnim, &weaponData->anims.crouchWalkBack.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "jump");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.jump.torsoAnim, &weaponData->anims.jump.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "land");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.land.torsoAnim, &weaponData->anims.land.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "run");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.run.torsoAnim, &weaponData->anims.run.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sprint");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.sprint.torsoAnim, &weaponData->anims.sprint.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sprintStyle");
	if(node)
	{
		weaponData->firstPersonSprintStyle = cJSON_ToNumber(node);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sights");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.sights.torsoAnim, &weaponData->anims.sights.legsAnim);
	}

	node = cJSON_GetObjectItem (playerAnimNode, "sightsFiring");
	if(node)
	{
		BG_ParseAnimationObject (node, &weaponData->anims.sightsFiring.torsoAnim, &weaponData->anims.sightsFiring.legsAnim);
	}
}

static void ReadString ( cJSON *parent, const char *field, char *dest, size_t destSize )
{
    cJSON *node = NULL;
    const char *str = NULL;
    
    node = cJSON_GetObjectItem (parent, field);
    str = cJSON_ToString (node);
    if ( str && str[0] )
    {
        int len = strlen (str);
        if ( (len + 1) > destSize )
        {
            Com_Printf (S_COLOR_YELLOW "WARNING: %s is too long (%d chars). Max length for this string is %d.\n", str, len, destSize - 1);
        }
        Q_strncpyz (dest, str, destSize);
    }
	else
	{
		dest[0] = '\0';
	}
}

#ifdef _CGAME

static void BG_ParseVisualsFireMode ( weaponVisualFireMode_t *fireMode, cJSON *fireModeNode, int numFireModes )
{
    cJSON *node = NULL;
    cJSON *child = NULL;
    qboolean isGrenade = qfalse;
    qboolean isBlaster = qfalse;
    qboolean isTripmine = qfalse;
    qboolean isDetpack = qfalse;

    ReadString (fireModeNode, "type", fireMode->type, sizeof (fireMode->type));
    isGrenade = (qboolean)(Q_stricmp (fireMode->type, "grenade") == 0);
    isBlaster = (qboolean)(Q_stricmp (fireMode->type, "blaster") == 0);
    isTripmine = (qboolean)(Q_stricmp (fireMode->type, "tripmine") == 0);
    isDetpack = (qboolean)(Q_stricmp (fireMode->type, "detpack") == 0);

	ReadString (fireModeNode, "displayName", fireMode->displayName, sizeof(fireMode->displayName));
	Q_strncpyz(fireMode->displayName, CG_GetStringEdString2(fireMode->displayName), sizeof(fireMode->displayName));

	ReadString (fireModeNode, "crosshairShader", fireMode->crosshairShader, MAX_QPATH);
	ReadString (fireModeNode, "switchToSound", fireMode->switchToSound, MAX_QPATH);
	if(!fireMode->switchToSound[0])
	{
		if(fmLoadCounter == 0)
		{
			Com_sprintf(fireMode->switchToSound, MAX_QPATH, "sound/weapons/common/click%i.wav", numFireModes);
		}
		else
		{
			Com_sprintf(fireMode->switchToSound, MAX_QPATH, "sound/weapons/common/click1.wav");
		}
	}

	node = cJSON_GetObjectItem (fireModeNode, "animType");
    fireMode->animType = (firingModeAnim_t)cJSON_ToInteger (node);

	fireMode->overrideIndicatorFrame = -1;											// set this as default since 0 is still a valid frame
	node = cJSON_GetObjectItem (fireModeNode, "overrideIndicatorFrame");
	if(node)
	{
		fireMode->overrideIndicatorFrame = cJSON_ToInteger (node);
	}

	node = cJSON_GetObjectItem (fireModeNode, "displayExplosive");
	fireMode->displayExplosive = (qboolean)cJSON_ToBooleanOpt(node, qfalse);
    
    // TODO: Need to tie this to the table in cg_weapons.c somehow...
    // Weapon Render
    node = cJSON_GetObjectItem (fireModeNode, "muzzlelightintensity");
    fireMode->weaponRender.generic.muzzleLightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "muzzlelightcolor", fireMode->weaponRender.generic.muzzleLightColor, sizeof (fireMode->weaponRender.generic.muzzleLightColor));
    ReadString (fireModeNode, "chargingfx", fireMode->weaponRender.generic.chargingEffect, sizeof (fireMode->weaponRender.generic.chargingEffect));    
    ReadString (fireModeNode, "muzzlefx", fireMode->weaponRender.generic.muzzleEffect, sizeof (fireMode->weaponRender.generic.muzzleEffect));
    
    // Weapon Fire
    child = cJSON_GetObjectItem (fireModeNode, "firesound");
    if ( child )
    {
        if ( cJSON_IsArray (child) )
        {
            const char *fireSounds[8];
            int numFireSounds = cJSON_ReadStringArray (child, 8, fireSounds);
            int i;
            
            for ( i = 0; i < numFireSounds; i++ )
            {
                Q_strncpyz (fireMode->weaponFire.generic.fireSound[i], fireSounds[i], sizeof (fireMode->weaponFire.generic.fireSound[i]));
            }
        }
        else
        {
            const char *s = cJSON_ToString (child);
            if ( s ) Q_strncpyz (fireMode->weaponFire.generic.fireSound[0], s, sizeof (fireMode->weaponFire.generic.fireSound[0]));
        }
    }
    
    // Traceline Render
    ReadString (fireModeNode, "tracelineshader", fireMode->tracelineRender.generic.tracelineShader, sizeof (fireMode->tracelineRender.generic.tracelineShader));
    
    node = cJSON_GetObjectItem (fireModeNode, "minsize");
    fireMode->tracelineRender.generic.minSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "maxsize");
    fireMode->tracelineRender.generic.maxSize = (float)cJSON_ToNumber (node);
    
    node = cJSON_GetObjectItem (fireModeNode, "lifetime");
    fireMode->tracelineRender.generic.lifeTime = cJSON_ToInteger (node);
    
    // Weapon Charge
    ReadString (fireModeNode, "chargingsound", fireMode->weaponCharge.chargingSound, sizeof (fireMode->weaponCharge.chargingSound));
    
    // Projectile render
    ReadString (fireModeNode, "projectilemodel", fireMode->projectileRender.generic.projectileModel, sizeof (fireMode->projectileRender.generic.projectileModel));
    ReadString (fireModeNode, "projectilefx", fireMode->projectileRender.generic.projectileEffect, sizeof (fireMode->projectileRender.generic.projectileEffect));
    ReadString (fireModeNode, "runsound", fireMode->projectileRender.generic.runSound, sizeof (fireMode->projectileRender.generic.runSound));
    
    node = cJSON_GetObjectItem (fireModeNode, "lightintensity");
    fireMode->projectileRender.generic.lightIntensity = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "lightcolor", fireMode->projectileRender.generic.lightColor, sizeof (fireMode->projectileRender.generic.lightColor));
  
    ReadString (fireModeNode, "deathfx", fireMode->projectileRender.generic.deathEffect, sizeof (fireMode->projectileRender.generic.deathEffect));
    
    // Projectile miss event
    child = cJSON_GetObjectItem (fireModeNode, "miss");
    if ( isTripmine || isDetpack )
    {
        ReadString (child, "sticksound", fireMode->projectileMiss.explosive.stickSound, sizeof (fireMode->projectileMiss.explosive.stickSound));
    }
    else
    {
        ReadString (child, "impactfx", fireMode->projectileMiss.generic.impactEffect, sizeof (fireMode->projectileMiss.generic.impactEffect));
        ReadString (child, "shockwavefx", fireMode->projectileMiss.grenade.shockwaveEffect, sizeof (fireMode->projectileMiss.grenade.shockwaveEffect));
    }
    
    // Projectile hit event
    child = cJSON_GetObjectItem (fireModeNode, "hit");
    ReadString (child, "impactfx", fireMode->projectileHitPlayer.generic.impactEffect, sizeof (fireMode->projectileHitPlayer.generic.impactEffect));
	ReadString(child, "shieldimpactfx", fireMode->projectileHitPlayer.generic.impactShieldEffect, sizeof(fireMode->projectileHitPlayer.generic.impactShieldEffect));	//note this is incomplete --Futuza
    ReadString (child, "shockwavefx", fireMode->projectileHitPlayer.grenade.shockwaveEffect, sizeof (fireMode->projectileHitPlayer.grenade.shockwaveEffect));
    
    // Projectile deflected event
    ReadString (fireModeNode, "deflectedfx", fireMode->projectileDeflected.generic.deflectEffect, sizeof (fireMode->projectileDeflected.generic.deflectEffect));
    
    // Grenade bounce event
    child = cJSON_GetObjectItem (fireModeNode, "bouncesound");
    if ( child )
    {
        const char *bounceSounds[2];
        int numBounceSounds = cJSON_ReadStringArray (child, 2, bounceSounds);
        int i;
        
        for ( i = 0; i < numBounceSounds; i++ )
        {
            Q_strncpyz (fireMode->grenadeBounce.grenade.bounceSound[i], bounceSounds[i], sizeof (fireMode->grenadeBounce.grenade.bounceSound[i]));
        }
    }
    
    // Explosive render
    ReadString (fireModeNode, "g2model", fireMode->explosiveRender.detpack.g2Model, sizeof (fireMode->explosiveRender.detpack.g2Model));
    
    node = cJSON_GetObjectItem (fireModeNode, "g2radius");
    fireMode->explosiveRender.detpack.g2Radius = (float)cJSON_ToNumber (node);
    
    ReadString (fireModeNode, "linefx", fireMode->explosiveRender.tripmine.lineEffect, sizeof (fireMode->explosiveRender.tripmine.lineEffect));
    
    // Explosive blow event
    ReadString (fireModeNode, "explodefx", fireMode->explosiveBlow.generic.explodeEffect, sizeof (fireMode->explosiveBlow.generic.explodeEffect));
    
    // Explosive armed event
    ReadString (fireModeNode, "armsound", fireMode->explosiveArm.armSound, sizeof (fireMode->explosiveArm.armSound));

	fmLoadCounter++;
}
#endif
static void BG_ParseVisuals ( weaponData_t *weaponData, cJSON *visualsNode )
{
    weaponVisual_t *weaponVisuals = &weaponData->visuals;

    ReadString (visualsNode, "worldmodel", weaponVisuals->world_model, sizeof (weaponVisuals->world_model));
#ifdef _CGAME
    ReadString (visualsNode, "viewmodel", weaponVisuals->view_model, sizeof (weaponVisuals->view_model));
    ReadString (visualsNode, "hudicon", weaponVisuals->icon, sizeof (weaponVisuals->icon));
    ReadString (visualsNode, "hudnaicon", weaponVisuals->icon_na, sizeof (weaponVisuals->icon_na));
    ReadString (visualsNode, "selectsound", weaponVisuals->selectSound, sizeof (weaponVisuals->selectSound));
    
    cJSON *child = cJSON_GetObjectItem (visualsNode, "indicators");
    if ( child )
    {
        int numGroupIndicators = 0;
        int i;
        char *shaders[3];
        cJSON *subchild = cJSON_GetObjectItem (child, "ammo");
        if ( subchild )
        {
            weaponVisuals->indicatorType = IND_NORMAL;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, const_cast<const char **>(shaders));
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        else if ( (subchild = cJSON_GetObjectItem (child, "leds")) != NULL )
        {
            weaponVisuals->indicatorType = IND_GRENADE;
            numGroupIndicators = cJSON_ReadStringArray (subchild, 3, const_cast<const char **>(shaders));
            for ( i = 0; i < numGroupIndicators; i++ )
            {
                Q_strncpyz (weaponVisuals->groupedIndicatorShaders[i], shaders[i], sizeof (weaponVisuals->groupedIndicatorShaders[i]));
            }
        }
        
        ReadString (child, "firemode", weaponVisuals->firemodeIndicatorShader, sizeof (weaponVisuals->firemodeIndicatorShader));
    }
    
    ReadString (visualsNode, "gunposition", weaponVisuals->gunPosition, sizeof (weaponVisuals->gunPosition));
    ReadString (visualsNode, "ironsightsPosition", weaponVisuals->ironsightsPosition, sizeof (weaponVisuals->ironsightsPosition));
    
    cJSON *node = cJSON_GetObjectItem (visualsNode, "ironsightsFov");
    weaponVisuals->ironsightsFov = (float)cJSON_ToNumberOpt (node, 0.0);
    
    // Scope toggle
    child = cJSON_GetObjectItem (visualsNode, "scope");
    ReadString (child, "startsound", weaponVisuals->scopeStartSound, sizeof (weaponVisuals->scopeStartSound));
    ReadString (child, "stopsound", weaponVisuals->scopeStopSound, sizeof (weaponVisuals->scopeStopSound));
    
    // Scope zoom
    node = cJSON_GetObjectItem (child, "looptime");
    weaponVisuals->scopeSoundLoopTime = cJSON_ToInteger (node);
    
    ReadString (child, "loopsound", weaponVisuals->scopeLoopSound, sizeof (weaponVisuals->scopeLoopSound));
    
    // Scope render
    ReadString (child, "mask", weaponVisuals->scopeShader, sizeof (weaponVisuals->scopeShader));

	//Barrel count
	node = cJSON_GetObjectItem (visualsNode, "barrelCount");
	weaponVisuals->barrelCount = cJSON_ToIntegerOpt(node, 0);
    
    /*node = cJSON_GetObjectItem (visualsNode, "primary");
    BG_ParseVisualsFireMode (&weaponVisuals->primary, node);
    
    node = cJSON_GetObjectItem (visualsNode, "secondary");
    BG_ParseVisualsFireMode (&weaponVisuals->secondary, node);*/

	for ( int i = 0; i < weaponData->numFiringModes; i++ )
	{
		node = cJSON_GetObjectItem (visualsNode, va("firemode%i", i) );
		if(node)
		{
			BG_ParseVisualsFireMode(&weaponVisuals->visualFireModes[i], node, weaponData->numFiringModes);
		}
	}
#endif
}

static void BG_ParseWPNSaberData ( weaponData_t *wp, cJSON *json )
{
	memset( &wp->sab, 0, sizeof( wp->sab ) );

	ReadString( json, "hilt", wp->sab.hiltname, sizeof( wp->sab.hiltname ) );
	ReadString( json, "crystal", wp->sab.defaultcrystal, sizeof( wp->sab.defaultcrystal ) );
}

extern stringID_table_t WPTable[]; // From bg_saga.c

#define MAX_WEAPON_FILE_LENGTH (16384) // 16kb should be enough, 4kb apparently wasn't!
static qboolean BG_ParseWeaponFile ( const char *weaponFilePath )
{
    cJSON *json = NULL;
    cJSON *jsonNode = NULL;
    char error[MAX_STRING_CHARS];
    const char *str = NULL;
    int weapon;
	int i;
    
    char weaponFileData[MAX_WEAPON_FILE_LENGTH];
    fileHandle_t f;
    int fileLen = trap->FS_Open (weaponFilePath, &f, FS_READ);
    
    weaponData_t weaponData;

	fmLoadCounter = 0;
    
    if ( !f || fileLen == -1 )
    {
        Com_Printf (S_COLOR_RED "%s: failed to read the weapon file. File is unreadable or is empty.\n", weaponFilePath);
        return qfalse;
    }
    
    if ( (fileLen + 1) >= MAX_WEAPON_FILE_LENGTH )
    {
        trap->FS_Close (f);
        Com_Printf (S_COLOR_RED "%s: file too big (%d bytes, maximum is %d).\n", weaponFilePath, fileLen, MAX_WEAPON_FILE_LENGTH - 1);
        
        return qfalse;
    }
    
    trap->FS_Read (&weaponFileData, fileLen, f);
    weaponFileData[fileLen] = '\0';
    
    trap->FS_Close (f);
    
    json = cJSON_ParsePooled (weaponFileData, error, sizeof (error));
    if ( json == NULL )
    {
        Com_Printf (S_COLOR_RED "%s: %s\n", weaponFilePath, error);
        
        return qfalse;
    }
    
    BG_InitializeWeaponData (&weaponData);
    
    ReadString (json, "classname", weaponData.classname, sizeof (weaponData.classname));
    
    jsonNode = cJSON_GetObjectItem (json, "type");
    str = cJSON_ToString (jsonNode);
    weapon = GetIDForString (WPTable, str);
    weaponData.weaponBaseIndex = weapon;
    
    jsonNode = cJSON_GetObjectItem (json, "variation");
    weapon = cJSON_ToNumber (jsonNode);
    weaponData.weaponModIndex = weapon;

    jsonNode = cJSON_GetObjectItem (json, "stats");
    BG_ParseWeaponStats (&weaponData, jsonNode);

	// Special awesome saber stuff...
	if( weaponData.weaponBaseIndex == WP_SABER )
	{
		jsonNode = cJSON_GetObjectItem (json, "saberdata");
		BG_ParseWPNSaberData( &weaponData, jsonNode );
	}
    
	weaponData.numFiringModes = 0;
	for(i = 0; i < MAX_FIREMODES; i++)
	{
		jsonNode = cJSON_GetObjectItem (json, va("firemode%i", i));
		if(jsonNode != NULL)
		{
			BG_ParseWeaponFireMode (&weaponData.firemodes[i], jsonNode);
			weaponData.numFiringModes++;
		}
	}
    
    jsonNode = cJSON_GetObjectItem (json, "playeranims");
    BG_ParseWeaponPlayerAnimations (&weaponData, jsonNode);
    
    jsonNode = cJSON_GetObjectItem (json, "name");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.displayName, str, sizeof (weaponData.displayName));
    
#ifdef _CGAME
    // TODO: Maybe we can turn this into a loop somehow? It's just turning into a stupidly long list.

    jsonNode = cJSON_GetObjectItem (json, "weaponanims");
    // TODO
    
    jsonNode = cJSON_GetObjectItem (json, "description");
    str = cJSON_ToString (jsonNode);
    Q_strncpyz (weaponData.visuals.description, str, sizeof (weaponData.visuals.description));
#endif
    jsonNode = cJSON_GetObjectItem (json, "visual");
    BG_ParseVisuals (&weaponData, jsonNode);
    

    /*if ( weaponData.zoomType != ZOOM_NONE )
    {
        // If we have zoom mode, then copy over the data from the primary to the secondary
        // so it's as if we haven't changed fire modes at all! Ingenious! (And also temporary)
        weaponData.firemodes[1] = weaponData.firemodes[0];
    }*/
    
    BG_AddWeaponData (&weaponData);
    
    cJSON_Delete (json);
    
    return qtrue;
}

#if 0

#define WEAPON_DATA_CACHE_VERSION (1)
typedef struct weaponDataCacheHeader_s
{
    int version;
    int numWeapons;
} weaponDataCacheHeader_t;

static qboolean LoadWeaponCacheFile ( weaponData_t *weaponDataTable, int *numLoadedWeapons, int *numWeapons )
{
    fileHandle_t f;
    int fileLen;
    
    char *buffer = NULL;
    char *p = NULL;
    weaponDataCacheHeader_t *header = NULL;
    int i;
    
    fileLen = trap->FS_Open ("ext_data/weapons/weapcache.bin", &f, FS_READ);
    if ( fileLen == -1 || !f )
    {
        return qfalse;
    }
    
    buffer = (char *)malloc (fileLen + 1);
    trap->FS_Read ((void *)buffer, fileLen, f);
    trap->FS_Close (f);
    buffer[fileLen] = '\0';
    
    header = (weaponDataCacheHeader_t *)&buffer[0];
    if ( header->version != WEAPON_DATA_CACHE_VERSION )
    {
        free (buffer);
        return qfalse;
    }
    
    p = buffer + sizeof (weaponDataCacheHeader_t);
    memcpy (weaponDataTable, p, header->numWeapons * sizeof (weaponData_t));
    *numLoadedWeapons = header->numWeapons;
    
    memset (numWeapons, 0, sizeof (int) * MAX_WEAPONS);
    for ( i = 0; i < *numLoadedWeapons; i++ )
    {
        numWeapons[weaponDataTable[i].weaponBaseIndex]++;
    }
    
    free (buffer);
    
    return qtrue;
}

static void WriteWeaponCacheFile ( weaponData_t *weaponDataTable, int *numLoadedWeapons )
{
    fileHandle_t f;
    char *buffer;
    char *p;
    weaponDataCacheHeader_t *header = NULL;
    int bufferSize;
    
    trap->FS_Open ("ext_data/weapons/weapcache.bin", &f, FS_WRITE);
    if ( !f )
    {
        return;
    }
    
    bufferSize = sizeof (weaponDataCacheHeader_t) + sizeof (weaponData_t) * *numLoadedWeapons;
    buffer = (char *)malloc (bufferSize);
    p = buffer;
    header = (weaponDataCacheHeader_t *)p;
    header->version = WEAPON_DATA_CACHE_VERSION;
    header->numWeapons = *numLoadedWeapons;
    
    p += sizeof (weaponDataCacheHeader_t);
    memcpy (p, weaponDataTable, sizeof (weaponData_t) * *numLoadedWeapons);
    
    trap->FS_Write ((void *)buffer, bufferSize, f);
    trap->FS_Close (f);
    
    free (buffer);
}

#endif

qboolean BG_LoadWeapons ( weaponData_t *weaponDataTable, unsigned int *numLoadedWeapons, unsigned int *numWeapons )
{
    int i;
    char weaponFiles[8192];
    int numFiles = Q_FSGetFileListSorted ("ext_data/weapons", ".wpn", weaponFiles, sizeof (weaponFiles));
    const char *weaponFile = weaponFiles;
    int successful = 0;
    int failed = 0;
    
    int t = trap->Milliseconds();
    
    Com_Printf ("------- Weapon Initialization -------\n");
    
    //if ( !LoadWeaponCacheFile (weaponDataTable, numLoadedWeapons, numWeapons) )
    {
        for ( i = 0; i < numFiles; i++ )
        {
            if ( BG_ParseWeaponFile (va ("ext_data/weapons/%s", weaponFile)) )
            {
                successful++;
            }
            else
            {
                failed++;
            }
            
            weaponFile += strlen (weaponFile) + 1;
        }
        
        //WriteWeaponCacheFile (weaponDataTable, numLoadedWeapons);
    }
    /*else
    {
        successful = *numLoadedWeapons;
    }*/
    
    Com_Printf ("Successfully loaded %d weapons, failed to load %d weapons.\n", successful, failed);
    Com_Printf ("Took %d milliseconds.\n", trap->Milliseconds() - t);
    Com_Printf ("-------------------------------------\n");
    
    return (qboolean)(successful > 0);
}

// Cloned from CG. Serverside needs Ghoul 2 instances too, in order to calculate muzzle position.
#if defined(_CGAME) || defined(_GAME)

void BG_InitWeaponG2Instances(void) {
	unsigned int i = 0;
	unsigned int j;
	
	memset(g2WeaponInstances, 0, sizeof(g2WeaponInstances));
	
	for ( i = 0; i <= LAST_USEABLE_WEAPON; i++ )
	{
	    unsigned int numVariations = BG_NumberOfWeaponVariations (i);
	    for ( j = 0; j < numVariations; j++ )
	    {
	        const weaponData_t *weaponData = GetWeaponData (i, j);
			const int id = BG_GetWeaponIndex(i, j);

			if( weaponData && weaponData->classname[0] &&			// stop with these goddamn asserts...
				weaponData->visuals.world_model[0])		// hard to code with all this background noise going on --eez
			{
				void *ghoul2 = NULL;
	        
				trap->G2API_InitGhoul2Model (&g2WeaponInstances[id].ghoul2, weaponData->visuals.world_model, 0, 0, 0, 0, 0);
	        
				ghoul2 = g2WeaponInstances[id].ghoul2;
	        
				if ( trap->G2_HaveWeGhoul2Models (ghoul2) )
				{
					trap->G2API_SetBoltInfo (ghoul2, 0, 0);
					trap->G2API_AddBolt (ghoul2, 0, i == WP_SABER ? "*blade1" : "*flash");
	            
					g2WeaponInstances[id].weaponNum = i;
					g2WeaponInstances[id].weaponVariation = j;
				}
			}
	    }
	}
}

void BG_ShutdownWeaponG2Instances(void) {
	unsigned int i = 0;
	unsigned int j;
	unsigned int id = 0;
	
	for ( i = 0; i <= LAST_USEABLE_WEAPON; i++ )
	{
	    unsigned int numVariations = BG_NumberOfWeaponVariations (i);
	    for ( j = 0; j < numVariations; j++ )
	    {
			if(g2WeaponInstances[id].ghoul2 && trap->G2_HaveWeGhoul2Models(g2WeaponInstances[id].ghoul2)) {
				trap->G2API_CleanGhoul2Models(&g2WeaponInstances[id].ghoul2);
				g2WeaponInstances[id].ghoul2 = NULL;
			}
			id++;
	    }
	}
}

void *BG_GetWeaponGhoul2 ( int weaponNum, int weaponVariation )
{
	const int id = BG_GetWeaponIndex(weaponNum, weaponVariation);
	return g2WeaponInstances[id].ghoul2;
}

#endif
