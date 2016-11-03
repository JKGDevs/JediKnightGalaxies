#include "jkg_damagetypes.h"
#include "g_local.h"
#include "qcommon/q_shared.h"

typedef struct damageInstance_s
{
    gentity_t *attacker; // e.g. a player
    gentity_t *inflictor; // e.g. the player's rocket
    gentity_t *ignoreEnt;
    vec3_t direction;
    int methodOfDeath;
    int damageFlags;

	// Charge related
	int damageOverride;
} damageInstance_t;

typedef struct damageArea_s
{
    int         startTime;
    int         lastDamageTime;
    qboolean    active;
    vec3_t      origin;
    damageInstance_t context;
    
    const damageSettings_t *data;
} damageArea_t;

#define MAX_DAMAGE_SETTINGS (128)
static damageSettings_t damageSettings[MAX_DAMAGE_SETTINGS];
static unsigned int numDamageSettings;

// Maybe make a linked list of active damage settings? Though
// looping through 256 elements is hardly time-consuming
#define MAX_DAMAGE_AREAS (256)
static damageArea_t damageAreas[MAX_DAMAGE_AREAS];

static struct
{
    damageType_t damageType;
    int damageFlags;
    int debuffLifeTime;
    int damage;
    int damageInterval;
} damageTypeData[] = {
    { DT_DISINTEGRATE,  0,              0,          0,      0		},
    { DT_ELECTRIC,      0,              10000,      5,      500		},
    { DT_EXPLOSION,     DAMAGE_RADIUS,  0,          0,      0		},
    { DT_FIRE,          0,              10000,      2,      1000	},
    { DT_FREEZE,        0,              500,        2,      1000	},
    { DT_IMPLOSION,     DAMAGE_RADIUS,  0,          0,      0		},	// Not used.
    { DT_STUN,          0,              2000,       0,      0		},	// Not used.
    { DT_CARBONITE,     0,              4000,       2,      1000	},

	{ DT_BLASTER,		0,				0,			0,		0		},
	{ DT_SLUG,			0,				0,			0,		0		},	
	{ DT_ACP,			0,				0,			0,		0		},
	{ DT_PULSE,			0,				0,			0,		0		},
	{ DT_ION,			0,				0,			0,		0		},
	{ DT_SONIC,			0,				0,			0,		0		},
	{ DT_BLEED,			0,				10000,		1,		500		},
	{ DT_COLD,			0,				5000,		0,		0		}

};

void JKG_RemoveDamageType(gentity_t *ent, damageType_t type);

void JKG_InitDamageSystem ( void )
{
    memset (damageAreas, 0, sizeof (damageAreas));
    memset (damageSettings, 0, sizeof (damageSettings));
    numDamageSettings = 0;
}

static float CalculateDamageRadius ( const damageArea_t *area )
{
    int t = level.time - area->startTime;
    const radiusParams_t *dmgParams = &area->data->radiusParams;
    float stop = 0.0f;
    
    switch ( dmgParams->radiusFunc )
    {
        case RF_CONSTANT:
            return dmgParams->startRadius;
        break;
        
        case RF_LINEAR:
            return dmgParams->startRadius + ((float)t / (float)area->data->lifetime) * (dmgParams->endRadius - dmgParams->startRadius);
        break;
        
        case RF_NONLINEAR:
            stop = (float)area->data->lifetime * (dmgParams->generic1 * 0.01f);
            if ( t <= stop )
            {
                return dmgParams->startRadius;
            }
            else
            {
                t -= stop;
                return dmgParams->startRadius + ((float)t / (float)(area->data->lifetime - stop)) * (dmgParams->endRadius - dmgParams->startRadius);
            }
        break;
        
        case RF_WAVE:
            // FIXME: I R BORKED.
            stop = sin ((double)(t * dmgParams->generic1));
            stop = stop * 0.5f + 0.5f;
            return dmgParams->startRadius + stop * (dmgParams->endRadius - dmgParams->startRadius);
        break;
        
        case RF_CLAMP:
            stop = (float)area->data->lifetime * (dmgParams->generic1 * 0.01f);
            if ( t >= stop )
            {
                return dmgParams->endRadius;
            }
            else
            {
                return dmgParams->startRadius + ((float)t / stop) * (dmgParams->endRadius - dmgParams->startRadius);
            }
        break;
    }
    
    return 0.0f;
}

static float gaussian ( float x )
{
    // mean = 0, sigma squared = 0.5
    return expf (-2.0f * x * x) / sqrt (0.5f * M_PI);
}

static void SmallestVectorToBBox ( vec3_t out, const vec3_t position, const vec3_t absmin, const vec3_t absmax )
{
    int k;
    for ( k = 0; k < 3; k++ )
    {
        if ( position[k] < absmin[k] ) {
	        out[k] = absmin[k] - position[k];
        } else if ( position[k] > absmax[k] ) {
	        out[k] = position[k] - absmax[k];
        } else {
	        out[k] = 0.0f;
        }
    }
}

static int CalculateDamageForDistance ( const damageArea_t *area, const vec3_t playerMins, const vec3_t playerMaxs, const vec3_t playerOrigin, float damageRadius )
{
    int d; // = area->data->damage;
    vec3_t v;
    float distanceFromOrigin;
    float f = 0.0f;

	if(area->context.damageOverride != 0 && area->context.damageOverride != area->data->damage) {
		d = area->context.damageOverride;
	}
	else {
		d = area->data->damage;
	}
    
    SmallestVectorToBBox (v, area->origin, playerMins, playerMaxs);
    distanceFromOrigin = VectorLength (v);
    
    if ( distanceFromOrigin > damageRadius )
    {
        return 0;
    }
    
    switch ( area->data->radiusParams.damageFunc )
    {
        case DF_CONSTANT:
            return d;
        break;
        
        case DF_GAUSSIAN: // change to sigmoid?
            f = gaussian (distanceFromOrigin / damageRadius);
            f = Q_max (0.0f, Q_min (f, 1.0f));
            return d * f;
        break;
        
        case DF_LINEAR:
            return d * (1.0f - (distanceFromOrigin / damageRadius));
        break;
    }
    
    return d;
}

static void DebuffPlayer ( gentity_t *player, damageArea_t *area, int damage )
{
    vec3_t dir;
    int i = 0;
    int flags = 0;
    
    if ( !player->client )
    {
        return;
    }
    
    SmallestVectorToBBox (dir, area->origin, player->r.absmin, player->r.absmax);
    dir[2] += 24.0f; // Push the player up a bit to get some air time.
    VectorNormalize (dir);

    for ( i = 0; i < NUM_DAMAGE_TYPES; i++ )
    {
        damageType_t damageType = (damageType_t)i;
        
        if ( !(area->data->damageType & (1 << damageType)) )
        {
            continue;
        }
        
        player->client->damageTypeTime[i] = level.time;
		player->client->damageTypeOwner[i] = area->context.attacker;
        
        switch ( damageType )
        {
            case DT_STUN:
            case DT_CARBONITE:
                player->client->ps.freezeLegsAnim = player->client->ps.legsAnim;
                player->client->ps.freezeTorsoAnim = player->client->ps.torsoAnim;
                player->client->pmlock = player->client->pmfreeze = qtrue;
                VectorClear (player->client->ps.velocity);
            break;

			case DT_FREEZE:
				JKG_RemoveDamageType(player, DT_FIRE);		// Remove fire...
				JKG_RemoveDamageType(player, DT_COLD);		// And cold...
				player->client->ps.freezeLegsAnim = player->client->ps.legsAnim;
				player->client->ps.freezeTorsoAnim = player->client->ps.torsoAnim;
				player->client->pmlock = player->client->pmfreeze = qtrue;
				VectorClear(player->client->ps.velocity);
			break;

			case DT_COLD:
				// FIXME: change player speed
				JKG_RemoveDamageType(player, DT_FIRE);
			break;

			case DT_FIRE:
				// Fire effectively thaws the player
				JKG_RemoveDamageType(player, DT_COLD);
				JKG_RemoveDamageType(player, DT_FREEZE);
			break;
            
            case DT_EXPLOSION:
                flags |= DAMAGE_RADIUS;
            break;
            
            case DT_IMPLOSION:
                flags |= DAMAGE_RADIUS;
                VectorNegate (dir, dir);
            break;

			default:
			break;
        }
    }
    
    if ( damage > 0 )
    {
        // Save some information in case the player dies, and gets disintegrated.
        int legsAnim = player->client->ps.legsAnim;
        int torsoAnim = player->client->ps.torsoAnim;
        vec3_t viewAngles;
        VectorCopy (player->client->ps.viewangles, viewAngles);
        
        if ( !area->data->radial )
        {
            VectorCopy (area->context.direction, dir);
        }
        
        G_Damage (player, area->context.inflictor, area->context.attacker, dir, player->client->ps.origin, damage, flags | area->context.damageFlags, 0);
        if ( player->health <= 0 )
        {
            // Dead...
            if ( area->data->damageType & (1 << DT_DISINTEGRATE) )
            {
                // Clear all flags - should put this into its own function later
                player->client->ps.damageTypeFlags = 0;
                player->client->pmfreeze = qfalse;
                player->client->pmlock = qfalse;
                player->client->pmnomove = qfalse;
                
                player->client->ps.eFlags |= EF_DISINTEGRATION;
                player->client->ps.legsAnim = legsAnim;
                player->client->ps.torsoAnim = torsoAnim;
                player->r.contents = 0;
                VectorCopy (viewAngles, player->client->ps.viewangles);
                VectorClear (player->client->ps.lastHitLoc);
                VectorClear (player->client->ps.velocity);
            }
        }
    }
    
    player->client->ps.damageTypeFlags |= area->data->damageType;
    area->lastDamageTime = level.time;
}

qhandle_t JKG_RegisterDamageSettings ( const damageSettings_t *settings )
{
    qhandle_t handle = numDamageSettings;
    const damageSettings_t *data = &damageSettings[0];
    int i = 0;
    if ( numDamageSettings >= MAX_DAMAGE_SETTINGS )
    {
        trap->Print ("WARNING: Max number of damage type settings exceeded. Max is %d.\n", MAX_DAMAGE_SETTINGS);
        return 0;
    }
    
    for ( i = 0; i < numDamageSettings; i++, data++ )
    {
        if ( memcmp (data, settings, sizeof (*settings)) == 0 )
        {
            // We have an identical one already. Return a handle to this instead.
            return i + 1;
        }
    }
    
    damageSettings[numDamageSettings] = *settings;
    numDamageSettings++;
    
    return (handle + 1);
}

void JKG_RemoveDamageType ( gentity_t *ent, damageType_t type )
{
	if( type >= (damageType_t)MAX_DAMAGE_SETTINGS )
		return;

    switch ( type )
    {
        case DT_STUN:
        case DT_FREEZE:
        case DT_CARBONITE:
            ent->client->pmfreeze = qfalse;
            ent->client->pmlock = qfalse;
        break;

		default:
		break;
    }

    ent->client->ps.damageTypeFlags &= ~(1 << type);
    ent->client->damageTypeLastEffectTime[(int)type] = 0;
}

void JKG_DoPlayerDamageEffects ( gentity_t *ent )
{
    int i = 0;
    
    if ( ent->health <= 0 )
    {
        // dead, remove all damage types
        // and clear timers.
        if ( ent->client->ps.damageTypeFlags > 0 )
        {
            for ( i = 0; i < NUM_DAMAGE_TYPES; i++ )
            {
                damageType_t damageType = (damageType_t)i;
                JKG_RemoveDamageType (ent, damageType);
            }
        }
        
        return;
    }
    
    for ( i = 0; i < NUM_DAMAGE_TYPES; i++ )
    {
        damageType_t damageType = (damageType_t)i;

        if ( !(ent->client->ps.damageTypeFlags & (1 << damageType)) )
        {
            continue;
        }
        
        if ( (ent->client->damageTypeTime[i] + damageTypeData[i].debuffLifeTime) < level.time )
        {
            // Debuff lifetime has ended
            JKG_RemoveDamageType (ent, damageType);
            continue;
        }
        
        switch ( damageType )
        {
            case DT_FIRE:
            {
                vec3_t p;
                VectorCopy (ent->client->ps.origin, p);
                p[2] -= 12.0f; // Water about half way up your thighs will extinguish the flames.
                if ( trap->PointContents (p, ent->s.number) & CONTENTS_WATER )
                {
                    JKG_RemoveDamageType (ent, DT_FIRE);
                }
                else if ( (ent->client->damageTypeLastEffectTime[i] + 500) <= level.time )
                {
                    // FIXME: Need to know the attacker
                    // FIXME: Also need to add a new method of death.
                    ent->client->damageTypeLastEffectTime[i] = level.time;
					G_Damage (ent, ent->client->damageTypeOwner[damageType], ent->client->damageTypeOwner[damageType], vec3_origin, ent->client->ps.origin, 2, 0, 0);
                }
            }
            break;
            
            case DT_FREEZE:
                if ( (ent->client->damageTypeLastEffectTime[i] + 500) <= level.time )
                {
                    ent->client->damageTypeLastEffectTime[i] = level.time;
                    G_Damage (ent, ent->client->damageTypeOwner[damageType], ent->client->damageTypeOwner[damageType], vec3_origin, ent->client->ps.origin, 2, 0, 0);
                }
            break;

			default:
			break;
        }
    }
}

static damageArea_t *GetFreeDamageArea()
{
    int i = 0;
    damageArea_t *area = &damageAreas[0];
    while ( area->active && i < MAX_DAMAGE_AREAS )
    {
        area++;
        i++;
    }
    
    if ( i == MAX_DAMAGE_AREAS )
    {
        Com_Printf ("WARNING: no free damage areas. Splash damage will not be dealt.\n");
        return NULL;
    }
    else
    {
        return area;
    }
}

static void DamagePlayersInArea ( damageArea_t *area )
{
    float damageRadiusSquared = 0.0f;
    int entList[MAX_GENTITIES] = { 0 };
    int numEnts = 0;
    vec3_t areaMins, areaMaxs;
    gentity_t *ent = NULL;
    int j = 0;
    float damageRadius;

    if ( !area->active )
    {
        return;
    }
    
    if ( area->startTime > level.time )
    {
        // Delayed start. Doing do anything yet.
        return;
    }
    
    if ( (area->startTime + area->data->lifetime) < level.time )
    {
        // Area has decayed, set as inactive.
        area->active = qfalse;
        return;
    }
    
    if ( (area->lastDamageTime + area->data->damageDelay) > level.time )
    {
        // Too soon to try to damage players again.
        return;
    }
    
    damageRadius = CalculateDamageRadius (area);
    damageRadiusSquared = damageRadius * damageRadius;
    
    for ( j = 0; j < 3; j++ )
    {
        areaMins[j] = area->origin[j] - damageRadius;
        areaMaxs[j] = area->origin[j] + damageRadius;
    }
    
    numEnts = trap->EntitiesInBox (areaMins, areaMaxs, entList, MAX_GENTITIES);
    for ( j = 0; j < numEnts; j++ )
    {
        vec3_t playerOrigin;
        trace_t tr;
        int damage;
        vec3_t v;
        
        ent = &g_entities[entList[j]];
        if ( !ent->inuse || !ent->client )
        {
            continue;
        }
        
        if ( ent->s.eType != ET_NPC &&
            (ent->client->pers.connected != CON_CONNECTED || 
                ent->client->sess.sessionTeam == TEAM_SPECTATOR) )
        {
            continue;
        }
        
        if ( !ent->takedamage || ent == area->context.ignoreEnt )
        {
            continue;
        }
        
        if ( ent->health <= 0 )
        {
            continue;
        }

        // Check to make sure the player is within the radius.
        SmallestVectorToBBox (v, area->origin, ent->r.absmin, ent->r.absmax);
        if ( VectorLengthSquared (v) > damageRadiusSquared )
        {
            continue;
        }
        
        VectorCopy (ent->client->ps.origin, playerOrigin);
        if ( area->data->penetrationType != PT_SHIELD_ARMOR_BUILDING )
        {
            trap->Trace (&tr, area->origin, NULL, NULL, playerOrigin, -1, CONTENTS_SOLID, 0, 0, 0);
            if ( tr.fraction != 1.0f )
            {
                continue;
            }
        }
        
        // Check for armor etc
        // if 
        
        // Apply the damage and its effects.
        damage = CalculateDamageForDistance (area, ent->r.absmin, ent->r.absmax, playerOrigin, damageRadius);
		DebuffPlayer (ent, area, damage);
    }
}

static damageSettings_t *GetDamageSettingsForHandle ( qhandle_t handle )
{
    handle--;
    if ( handle < 0 || handle >= numDamageSettings )
    {
        trap->Print ("ERROR: Invalid damage area handle given.\n");
        return NULL;
    }
    
    return &damageSettings[handle];
}

//=========================================================
// JKG_ChargeDamageOverride
//---------------------------------------------------------
// Description: Calculates damage override, which is 
// often modified as a result of a charging weapon
//=========================================================
int JKG_ChargeDamageOverride( gentity_t *inflictor, bool bIsTraceline ) {
	int damage = 0;
	if(inflictor->s.generic1 != 0 || bIsTraceline) {
		weaponData_t *wp = GetWeaponData(inflictor->s.weapon, inflictor->s.weaponVariation);
		int firemode = inflictor->s.firingMode;
		if(wp->firemodes[firemode].chargeTime) {
			if(bIsTraceline) {
				damage = WP_GetWeaponDamage(inflictor, inflictor->s.firingMode);
			}
			else {
				int current = ( level.time - inflictor->s.generic1 ) / wp->firemodes[firemode].chargeTime;
				float maximum = wp->firemodes[firemode].chargeMaximum / wp->firemodes[firemode].chargeTime;

				if ( current > maximum ) current = maximum;

				damage = wp->firemodes[firemode].baseDamage * current * wp->firemodes[firemode].chargeMultiplier;
				if(damage < wp->firemodes[firemode].baseDamage) {
					damage = wp->firemodes[firemode].baseDamage;
				}
			}
		}
	}
	return damage;
}

//=========================================================
// JKG_DoDamage
//---------------------------------------------------------
// Description: This is a wrapper for the G_Damage
// function, which also does debuffs. It does _not_ create
// damage areas. It only does direct damage like with
// G_Damage.
//=========================================================
void JKG_DoDirectDamage ( qhandle_t handle, gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t origin, int dflags, int mod )
{
    damageSettings_t *data;
    damageArea_t area;
	int damage;
        
    if ( !targ->takedamage )
    {
        return;
    }
    
    if ( targ->health <= 0 )
    {
        return;
    }
    
    if ( !targ->client )
    {
        return;
    }
    
    data = GetDamageSettingsForHandle (handle);
    memset (&area, 0, sizeof (area));

	area.data = data;
	// The firing mode's base damage can lie! It doesn't account for dynamic damage amounts (ie weapon charging)
	area.context.damageOverride = JKG_ChargeDamageOverride(inflictor, inflictor == attacker);
	if(area.context.damageOverride != 0 && area.data->damage != area.context.damageOverride) {
		damage = area.context.damageOverride;
	}
	else {
		damage = data->damage;
	}

    area.active = qtrue;
    VectorCopy (dir, area.context.direction);
    area.context.ignoreEnt = NULL;
    area.context.attacker = attacker;
    area.context.damageFlags = dflags;
    area.context.inflictor = inflictor;
    area.context.methodOfDeath = mod;
    area.startTime = level.time;
    area.lastDamageTime = 0;
    VectorCopy (origin, area.origin);
    
	DebuffPlayer (targ, &area, damage);
}

//=========================================================
// JKG_DoSplashDamage
//---------------------------------------------------------
// Description: This is a _replacement_ function for
// G_RadiusDamage. It does all the same things, in addition
// to the debuffs.
//=========================================================
void JKG_DoSplashDamage ( qhandle_t handle, const vec3_t origin, gentity_t *inflictor, gentity_t *attacker, gentity_t *ignoreEnt, int mod )
{
    damageSettings_t *data = GetDamageSettingsForHandle (handle);
	bool bDoDamageOverride = false;

	if (inflictor != attacker) {
		bDoDamageOverride = true;
	}

    if ( !data->radial )
    {
        return;
    }
    
    if ( data->lifetime )
    {
        damageArea_t *area = GetFreeDamageArea();
        if ( !area )
        {
            return;
        }

        if ( !data->planar )
        {
            area->active = qtrue;
            area->data = data;
            area->lastDamageTime = 0;
            VectorCopy (origin, area->origin);
            VectorClear (area->context.direction);
            area->context.ignoreEnt = ignoreEnt;
            area->context.attacker = attacker;
            area->context.damageFlags = 0;
            area->context.inflictor = inflictor;
            area->context.methodOfDeath = mod;
            area->startTime = level.time + data->delay;

			if(bDoDamageOverride) {
				area->context.damageOverride = JKG_ChargeDamageOverride(inflictor, false);
			}
			else {
				area->context.damageOverride = area->data->damage;
			}
        }
        else
        {
            // check nearest surface. If can't find one, then don't create an area..
        }
    }
    else
    {
        // This is similar to the old style splash damage
        damageArea_t a;
        memset (&a, 0, sizeof (a));
        a.active = qtrue;
        a.data = data;
        a.lastDamageTime = 0;
        VectorCopy (origin, a.origin);
        VectorClear (a.context.direction);
        a.context.ignoreEnt = ignoreEnt;
        a.context.attacker = attacker;
        a.context.damageFlags = 0;
        a.context.inflictor = inflictor;
        a.context.methodOfDeath = mod;
        a.startTime = level.time;

		if(bDoDamageOverride) {
			a.context.damageOverride = JKG_ChargeDamageOverride(inflictor, false);
		}
		else {
			a.context.damageOverride = a.data->damage;
		}
        
        DamagePlayersInArea (&a);
    }
}

//=========================================================
// JKG_DoDamage
//---------------------------------------------------------
// Description: This is a wrapper for the G_Damage
// function, which also does debuffs and stuff. Might
// create a damage area instead, if the handle does
// splash damage.
//=========================================================
void JKG_DoDamage ( qhandle_t handle, gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, vec3_t dir, vec3_t origin, int dflags, int mod )
{
    damageSettings_t *data = GetDamageSettingsForHandle (handle);
    if ( data->radial )
    {
        JKG_DoSplashDamage (handle, origin, inflictor, attacker, NULL, mod);
    }

    JKG_DoDirectDamage (handle, targ, inflictor, attacker, dir, origin, dflags, mod);
}

//=========================================================
// JKG_DamagePlayers
//---------------------------------------------------------
// Description: To be called on every server frame update.
// This goes through all active areas and deals damage to
// players within those areas.
//=========================================================
void JKG_DamagePlayers ( void )
{
    int i;
    damageArea_t *area = &damageAreas[0];
    
    for ( i = 0; i < MAX_DAMAGE_AREAS; i++, area++ )
    {
        DamagePlayersInArea (area);
    }
}
