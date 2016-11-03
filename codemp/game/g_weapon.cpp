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

// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"
#include "botlib/be_aas.h"
#include "bg_saga.h"
#include "ghoul2/G2.h"
#include "qcommon/q_shared.h"
#include "jkg_damagetypes.h"

static	vec3_t	forward, vright, up;
static	vec3_t	muzzle;

// Bryar Pistol
//--------
#define BRYAR_PISTOL_VEL			4600 //1600
#define BRYAR_PISTOL_DAMAGE			10
#define BRYAR_CHARGE_UNIT			200.0f	// bryar charging gives us one more unit every 200ms--if you change this, you'll have to do the same in bg_pmove
#define BRYAR_ALT_SIZE				1.0f

// E11 Blaster
//---------
#define BLASTER_SPREAD				1.6f//1.2f
#define BLASTER_VELOCITY			5800 //2300
#define BLASTER_DAMAGE				20

// Tenloss Disruptor
//----------
#define DISRUPTOR_MAIN_DAMAGE			30 //40
#define DISRUPTOR_MAIN_DAMAGE_SIEGE		50
#define DISRUPTOR_NPC_MAIN_DAMAGE_CUT	0.25f

#define DISRUPTOR_ALT_DAMAGE			100 //125
#define DISRUPTOR_NPC_ALT_DAMAGE_CUT	0.2f
#define DISRUPTOR_ALT_TRACES			3		// can go through a max of 3 damageable(sp?) entities
#define DISRUPTOR_CHARGE_UNIT			50.0f	// distruptor charging gives us one more unit every 50ms--if you change this, you'll have to do the same in bg_pmove

// Wookiee Bowcaster
//----------
#define	BOWCASTER_DAMAGE			50
#define	BOWCASTER_VELOCITY			1300
#define BOWCASTER_SPLASH_DAMAGE		25
#define BOWCASTER_SPLASH_RADIUS		40
#define BOWCASTER_SIZE				2

#define BOWCASTER_ALT_SPREAD		5.0f
#define BOWCASTER_VEL_RANGE			0.3f
#define BOWCASTER_CHARGE_UNIT		200.0f	// bowcaster charging gives us one more unit every 200ms--if you change this, you'll have to do the same in bg_pmove

// Heavy Repeater
//----------
#define REPEATER_SPREAD				1.15f//1.4f
#define	REPEATER_DAMAGE				8
#define	REPEATER_VELOCITY			4600//1600

#define REPEATER_ALT_SIZE				3	// half of bbox size
#define	REPEATER_ALT_DAMAGE				60
#define REPEATER_ALT_SPLASH_DAMAGE		60
#define REPEATER_ALT_SPLASH_RADIUS		128
#define REPEATER_ALT_SPLASH_RAD_SIEGE	80
#define	REPEATER_ALT_VELOCITY			1100

// DEMP2
//----------
#define	DEMP2_DAMAGE				35
#define	DEMP2_VELOCITY				1800
#define	DEMP2_SIZE					2		// half of bbox size

#define DEMP2_ALT_DAMAGE			8 //12		// does 12, 36, 84 at each of the 3 charge levels.
#define DEMP2_CHARGE_UNIT			700.0f	// demp2 charging gives us one more unit every 700ms--if you change this, you'll have to do the same in bg_weapons
#define DEMP2_ALT_RANGE				4096
#define DEMP2_ALT_SPLASHRADIUS		256

// Golan Arms Flechette
//---------
#define FLECHETTE_SHOTS				5
#define FLECHETTE_SPREAD			4.0f
#define FLECHETTE_DAMAGE			18//15
#define FLECHETTE_VEL				3500
#define FLECHETTE_SIZE				1
#define FLECHETTE_MINE_RADIUS_CHECK	256
#define FLECHETTE_ALT_DAMAGE		60
#define FLECHETTE_ALT_SPLASH_DAM	60
#define FLECHETTE_ALT_SPLASH_RAD	128

// Personal Rocket Launcher
//---------
#define	ROCKET_VELOCITY				1900 //900
#define	ROCKET_DAMAGE				200 //100
#define	ROCKET_SPLASH_DAMAGE		150 //100
#define	ROCKET_SPLASH_RADIUS		256 //160
#define ROCKET_SIZE					3
#define ROCKET_ALT_THINK_TIME		100

// Concussion Rifle
//---------
//primary
//man, this thing is too absurdly powerful. having to
//slash the values way down from sp.
#define	CONC_VELOCITY				3000
#define	CONC_DAMAGE					150 //150
#define	CONC_NPC_DAMAGE_EASY		40
#define	CONC_NPC_DAMAGE_NORMAL		80
#define	CONC_NPC_DAMAGE_HARD		100
#define	CONC_SPLASH_DAMAGE			150 //50
#define	CONC_SPLASH_RADIUS			300 //300
//alt
#define CONC_ALT_DAMAGE				100 //100 // Extreme damage on impact
#define CONC_ALT_NPC_DAMAGE_EASY	20
#define CONC_ALT_NPC_DAMAGE_MEDIUM	35
#define CONC_ALT_NPC_DAMAGE_HARD	50

// Stun Baton
//--------------
#define STUN_BATON_DAMAGE			20
#define STUN_BATON_ALT_DAMAGE		20
#define STUN_BATON_RANGE			8

// Melee
//--------------
#define MELEE_SWING1_DAMAGE			5
#define MELEE_SWING2_DAMAGE			6
#define MELEE_RANGE					8

// ATST Main Gun
//--------------
#define ATST_MAIN_VEL				4000	// 
#define ATST_MAIN_DAMAGE			25		// 
#define ATST_MAIN_SIZE				3		// make it easier to hit things

// ATST Side Gun
//---------------
#define ATST_SIDE_MAIN_DAMAGE				75
#define ATST_SIDE_MAIN_VELOCITY				1300
#define ATST_SIDE_MAIN_NPC_DAMAGE_EASY		30
#define ATST_SIDE_MAIN_NPC_DAMAGE_NORMAL	40
#define ATST_SIDE_MAIN_NPC_DAMAGE_HARD		50
#define ATST_SIDE_MAIN_SIZE					4
#define ATST_SIDE_MAIN_SPLASH_DAMAGE		10	// yeah, pretty small, either zero out or make it worth having?
#define ATST_SIDE_MAIN_SPLASH_RADIUS		16	// yeah, pretty small, either zero out or make it worth having?

#define ATST_SIDE_ALT_VELOCITY				1100
#define ATST_SIDE_ALT_NPC_VELOCITY			600
#define ATST_SIDE_ALT_DAMAGE				130

#define ATST_SIDE_ROCKET_NPC_DAMAGE_EASY	30
#define ATST_SIDE_ROCKET_NPC_DAMAGE_NORMAL	50
#define ATST_SIDE_ROCKET_NPC_DAMAGE_HARD	90

#define	ATST_SIDE_ALT_SPLASH_DAMAGE			130
#define	ATST_SIDE_ALT_SPLASH_RADIUS			200
#define ATST_SIDE_ALT_ROCKET_SIZE			5
#define ATST_SIDE_ALT_ROCKET_SPLASH_SCALE	0.5f	// scales splash for NPC's

const weaponFireModeStats_t *GetEntsCurrentFireMode ( const gentity_t *ent )
{
    const weaponData_t *weapon = GetWeaponData (ent->s.weapon, ent->s.weaponVariation);
	const weaponFireModeStats_t *fireMode = &weapon->firemodes[ent->s.firingMode];
	/*if ( ent->s.eFlags & EF_ALT_FIRING )
	{
	    fireMode = &weapon->firemodes[1];
	}*/
	
	return fireMode;
}

extern qboolean G_BoxInBounds( vec3_t point, vec3_t mins, vec3_t maxs, vec3_t boundsMins, vec3_t boundsMaxs );
extern void NPC_Humanoid_Decloak( gentity_t *self );

static void WP_FireEmplaced( gentity_t *ent, qboolean altFire );

void laserTrapStick( gentity_t *ent, vec3_t endpos, vec3_t normal );

void touch_NULL( gentity_t *ent, gentity_t *other, trace_t *trace )
{

}

qboolean WP_GetGrenadeBounce( gentity_t *ent, int firemode );
int WP_GetGrenadeBounceDamage( gentity_t *ent, int firemode );
extern void G_BounceMissile( gentity_t *ent, trace_t *trace );
void touch_GrenadeWithBouce( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	if( other->takedamage )
	{
		G_Damage( other, ent, ent->parent, NULL, ent->s.origin, ent->genericValue9, 0, MOD_THERMAL );
	}
	G_BounceMissile( ent, trace );
}

void laserTrapExplode( gentity_t *self );
void RocketDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);

static int GetClipIndex ( unsigned int weapon, unsigned int variation, unsigned int firemode )
{
    return BG_GetWeaponIndex (weapon, variation) + firemode;
}

int G_GetAmmoInCurrentClip ( const gentity_t *ent, unsigned int weapon, unsigned int variation, unsigned int firemode )
{
    int index = GetClipIndex (weapon, variation, firemode);
    if ( index >= (MAX_WEAPON_TABLE_SIZE * MAX_FIREMODES) )
    {
        return 0;
    }
    
    return ent->client->clipammo[index];
}

void G_SetAmmoInCurrentClip ( gentity_t *ent, unsigned int weapon, unsigned int variation, unsigned int firemode, unsigned int ammo )
{
    int index = GetClipIndex (weapon, variation, firemode);
    if ( index >= (MAX_WEAPON_TABLE_SIZE * MAX_FIREMODES) )
    {
        return;
    }

	if ( index >= 256 ) // TODO: use a global #define for this
		return;
    
    ent->client->clipammo[index] = ammo;
}

void W_TraceSetStart( gentity_t *ent, vec3_t start, vec3_t mins, vec3_t maxs )
{
	trace_t	tr;
	vec3_t	entMins;
	vec3_t	entMaxs;
	vec3_t	eyePoint;

	VectorAdd( ent->r.currentOrigin, ent->r.mins, entMins );
	VectorAdd( ent->r.currentOrigin, ent->r.maxs, entMaxs );

	if ( G_BoxInBounds( start, mins, maxs, entMins, entMaxs ) )
	{
		return;
	}

	if ( !ent->client )
	{
		return;
	}

	VectorCopy( ent->s.pos.trBase, eyePoint);
	eyePoint[2] += ent->client->ps.viewheight;
	
	trap->Trace( &tr, eyePoint, mins, maxs, start, ent->s.number, MASK_SOLID | CONTENTS_SHOTCLIP , 0, 0, 0);

	if ( tr.startsolid || tr.allsolid )
	{
		return;
	}

	if ( tr.fraction < 1.0f )
	{
		VectorCopy( tr.endpos, start );
	}
}

//---------------------------------------------------------
void WP_FireEmplacedMissile( gentity_t *ent, vec3_t start, vec3_t dir, qboolean altFire, gentity_t *ignore )
//---------------------------------------------------------
{
	int velocity	= BLASTER_VELOCITY;
	int	damage		= BLASTER_DAMAGE;
	gentity_t *missile;

	missile = CreateMissile( start, dir, velocity, 10000, ent, altFire );

	missile->classname = "emplaced_gun_proj";
	missile->s.weapon = WP_TURRET;//WP_EMPLACED_GUN;

	missile->activator = ignore;

	missile->damage = damage;
	missile->dflags = (DAMAGE_DEATH_KNOCKBACK|DAMAGE_HEAVY_WEAP_CLASS);
	missile->methodOfDeath = MOD_VEHICLE;
	missile->clipmask = MASK_SHOT | CONTENTS_LIGHTSABER;

	if (ignore)
	{
		missile->passThroughNum = ignore->s.number+1;
	}

	// we don't want it to bounce forever
	missile->bounceCount = 8;
}


int G_GetHitLocation(gentity_t *target, vec3_t ppoint);


static gentity_t *ent_list[MAX_GENTITIES];



//---------------------------------------------------------
void prox_mine_think( gentity_t *ent )
//---------------------------------------------------------
{
	int			count, i;
	qboolean	blow = qfalse;

	// if it isn't time to auto-explode, do a small proximity check
	if ( ent->delay > level.time )
	{
		count = G_RadiusList( ent->r.currentOrigin, FLECHETTE_MINE_RADIUS_CHECK, ent, qtrue, ent_list );

		for ( i = 0; i < count; i++ )
		{
			if ( ent_list[i]->client && ent_list[i]->health > 0 && ent->activator && ent_list[i]->s.number != ent->activator->s.number)
			{
				blow = qtrue;
				break;
			}
		}
	}
	else
	{
		// well, we must die now
		blow = qtrue;
	}

	if ( blow )
	{
		ent->think = laserTrapExplode;
		ent->nextthink = level.time + 200;
	}
	else
	{
		// we probably don't need to do this thinking logic very often...maybe this is fast enough?
		ent->nextthink = level.time + 500;
	}
}

//-----------------------------------------------------------------------------
static void WP_TraceSetStart( gentity_t *ent, vec3_t start, vec3_t mins, vec3_t maxs )
//-----------------------------------------------------------------------------
{
	//make sure our start point isn't on the other side of a wall
	trace_t	tr;
	vec3_t	entMins;
	vec3_t	entMaxs;

	VectorAdd( ent->r.currentOrigin, ent->r.mins, entMins );
	VectorAdd( ent->r.currentOrigin, ent->r.maxs, entMaxs );

	if ( G_BoxInBounds( start, mins, maxs, entMins, entMaxs ) )
	{
		return;
	}

	if ( !ent->client )
	{
		return;
	}

	trap->Trace( &tr, ent->client->ps.origin, mins, maxs, start, ent->s.number, MASK_SOLID|CONTENTS_SHOTCLIP , 0, 0, 0);

	if ( tr.startsolid || tr.allsolid )
	{
		return;
	}

	if ( tr.fraction < 1.0f )
	{
		VectorCopy( tr.endpos, start );
	}
}

void WP_ExplosiveDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->activator = attacker;
	laserTrapExplode(self);
}


/*
======================================================================

ROCKET LAUNCHER

======================================================================
*/

//---------------------------------------------------------
void rocketThink( gentity_t *ent )
//---------------------------------------------------------
{
	vec3_t newdir, targetdir, 
			up={0,0,1}, right; 
	vec3_t	org;
	float dot, dot2, dis;
	int i;
	float vel = (ent->spawnflags&1)?ent->speed:ROCKET_VELOCITY;

	if ( ent->genericValue1 && ent->genericValue1 < level.time )
	{//time's up, we're done, remove us
		if ( ent->genericValue2 )
		{//explode when die
			RocketDie( ent, &g_entities[ent->r.ownerNum], &g_entities[ent->r.ownerNum], 0, MOD_UNKNOWN );
		}
		else
		{//just remove when die
			G_FreeEntity( ent );
		}
		return;
	}
	if ( !ent->enemy 
		|| !ent->enemy->client 
		|| ent->enemy->health <= 0 
		|| ent->enemy->client->ps.powerups[PW_CLOAKED] )
	{//no enemy or enemy not a client or enemy dead or enemy cloaked
		if ( !ent->genericValue1  )
		{//doesn't have its own self-kill time
			ent->nextthink = level.time + 10000;
			ent->think = G_FreeEntity;
		}
		return;
	}

	if ( (ent->spawnflags&1) )
	{//vehicle rocket
		if ( ent->enemy->client && ent->enemy->client->NPC_class == CLASS_VEHICLE )
		{//tracking another vehicle
			if ( ent->enemy->client->ps.speed+4000 > vel )
			{
				vel = ent->enemy->client->ps.speed+4000;
			}
		}
	}

	if ( ent->enemy && ent->enemy->inuse )
	{	
		float newDirMult = ent->angle?ent->angle*2.0f:1.0f;
		float oldDirMult = ent->angle?(1.0f-ent->angle)*2.0f:1.0f;

		VectorCopy( ent->enemy->r.currentOrigin, org );
		org[2] += (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5f;

		VectorSubtract( org, ent->r.currentOrigin, targetdir );
		VectorNormalize( targetdir );

		// Now the rocket can't do a 180 in space, so we'll limit the turn to about 45 degrees.
		dot = DotProduct( targetdir, ent->movedir );
		if ( (ent->spawnflags&1) )
		{//vehicle rocket
			if ( ent->radius > -1.0f )
			{//can lose the lock if DotProduct drops below this number
				if ( dot < ent->radius )
				{//lost the lock!!!
					//HMM... maybe can re-lock on if they come in front again?
					/*
					//OR: should it stop trying to lock altogether?
					if ( ent->genericValue1 )
					{//have a timelimit, set next think to that
						ent->nextthink = ent->genericValue1;
						if ( ent->genericValue2 )
						{//explode when die
							ent->think = G_ExplodeMissile;
						}
						else
						{
							ent->think = G_FreeEntity;
						}
					}
					else
					{
						ent->think = NULL;
						ent->nextthink = -1;
					}
					*/
					return;
				}
			}
		}


		// a dot of 1.0 means right-on-target.
		if ( dot < 0.0f )
		{	
			// Go in the direction opposite, start a 180.
			CrossProduct( ent->movedir, up, right );
			dot2 = DotProduct( targetdir, right );

			if ( dot2 > 0 )
			{	
				// Turn 45 degrees right.
				VectorMA( ent->movedir, 0.4f*newDirMult, right, newdir );
			}
			else
			{	
				// Turn 45 degrees left.
				VectorMA( ent->movedir, -0.4f*newDirMult, right, newdir );
			}

			// Yeah we've adjusted horizontally, but let's split the difference vertically, so we kinda try to move towards it.
			newdir[2] = ( (targetdir[2]*newDirMult) + (ent->movedir[2]*oldDirMult) ) * 0.5;

			// let's also slow down a lot
			vel *= 0.5f;
		}
		else if ( dot < 0.70f )
		{	
			// Still a bit off, so we turn a bit softer
			VectorMA( ent->movedir, 0.5f*newDirMult, targetdir, newdir );
		}
		else
		{	
			// getting close, so turn a bit harder
			VectorMA( ent->movedir, 0.9f*newDirMult, targetdir, newdir );
		}

		// add crazy drunkenness
		for (i = 0; i < 3; i++ )
		{
			newdir[i] += crandom() * ent->random * 0.25f;
		}

		// decay the randomness
		ent->random *= 0.9f;

		if ( ent->enemy->client
			&& ent->enemy->client->ps.groundEntityNum != ENTITYNUM_NONE )
		{//tracking a client who's on the ground, aim at the floor...?
			// Try to crash into the ground if we get close enough to do splash damage
			dis = Distance( ent->r.currentOrigin, org );

			if ( dis < 128 )
			{
				// the closer we get, the more we push the rocket down, heh heh.
				newdir[2] -= (1.0f - (dis / 128.0f)) * 0.6f;
			}
		}

		VectorNormalize( newdir );

		VectorScale( newdir, vel * 0.5f, ent->s.pos.trDelta );
		VectorCopy( newdir, ent->movedir );
		SnapVector( ent->s.pos.trDelta );			// save net bandwidth
		VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;
	}

	ent->nextthink = level.time + ROCKET_ALT_THINK_TIME;	// Nothing at all spectacular happened, continue.
	return;
}

extern void G_ExplodeMissile( gentity_t *ent );
void RocketDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->die = 0;
	self->r.contents = 0;

	G_ExplodeMissile( self );

	self->think = G_FreeEntity;
	self->nextthink = level.time;
}

//---------------------------------------------------------
static void WP_FireRocket( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	int	damage	= ROCKET_DAMAGE;
	int	vel = ROCKET_VELOCITY;
	int dif = 0;
	float rTime;
	gentity_t *missile;

	if ( altFire )
	{
		vel *= 0.5f;
	}

	missile = CreateMissile( muzzle, forward, vel, 10000, ent, altFire );

	if (ent->client && ent->client->ps.rocketLockIndex != ENTITYNUM_NONE)
	{
		float lockTimeInterval = 1200.0f/16.0f;
		rTime = ent->client->ps.rocketLockTime;

		if (rTime == -1)
		{
			rTime = ent->client->ps.rocketLastValidTime;
		}
		dif = ( level.time - rTime ) / lockTimeInterval;

		if (dif < 0)
		{
			dif = 0;
		}

		//It's 10 even though it locks client-side at 8, because we want them to have a sturdy lock first, and because there's a slight difference in time between server and client
		if ( dif >= 10 && rTime != -1 )
		{
			missile->enemy = &g_entities[ent->client->ps.rocketLockIndex];

			if (missile->enemy && missile->enemy->client && missile->enemy->health > 0 && !OnSameTeam(ent, missile->enemy))
			{ //if enemy became invalid, died, or is on the same team, then don't seek it
				missile->angle = 0.5f;
				missile->think = rocketThink;
				missile->nextthink = level.time + ROCKET_ALT_THINK_TIME;
			}
		}

		ent->client->ps.rocketLockIndex = ENTITYNUM_NONE;
		ent->client->ps.rocketLockTime = 0;
		ent->client->ps.rocketTargetTime = 0;
	}

	missile->classname = "rocket_proj";
	missile->s.weapon = WP_ROCKET_LAUNCHER;

	// Make it easier to hit things
	VectorSet( missile->r.maxs, ROCKET_SIZE, ROCKET_SIZE, ROCKET_SIZE );
	VectorScale( missile->r.maxs, -1, missile->r.mins );

	missile->damage = damage;
	missile->dflags = DAMAGE_DEATH_KNOCKBACK;
	if (altFire)
	{
		missile->methodOfDeath = MOD_ROCKET_HOMING;
		missile->splashMethodOfDeath = MOD_ROCKET_HOMING_SPLASH;
	}
	else
	{
		missile->methodOfDeath = MOD_ROCKET;
		missile->splashMethodOfDeath = MOD_ROCKET_SPLASH;
	}
//===testing being able to shoot rockets out of the air==================================
	missile->health = 10;
	missile->takedamage = qtrue;
	missile->r.contents = MASK_SHOT;
	missile->die = RocketDie;
//===testing being able to shoot rockets out of the air==================================
	
	missile->clipmask = MASK_SHOT;
	missile->splashDamage = ROCKET_SPLASH_DAMAGE;
	missile->splashRadius = ROCKET_SPLASH_RADIUS;

	// we don't want it to ever bounce
	missile->bounceCount = 0;
}

/*
======================================================================

THERMAL DETONATOR

======================================================================
*/

#define TD_DAMAGE			300 //only do 70 on a direct impact
#define TD_SPLASH_RAD		386 //128
#define TD_SPLASH_DAM		200
#define TD_VELOCITY			900
#define TD_MIN_CHARGE		0.15f
#define TD_TIME				3000//6000
#define TD_ALT_TIME			3000

#define TD_ALT_DAMAGE		300//100
#define TD_ALT_SPLASH_RAD	386//128
#define TD_ALT_SPLASH_DAM	200//90
#define TD_ALT_VELOCITY		600
#define TD_ALT_MIN_CHARGE	0.15f
#define TD_ALT_TIME			3000

void thermalThinkStandard(gentity_t *ent);

//---------------------------------------------------------
void thermalDetonatorExplode( gentity_t *ent )
//---------------------------------------------------------
{
	if ( !ent->count )
	{
		//G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/thermal/warning.wav" ) );
		ent->count = 1;
		ent->genericValue5 = level.time + 500;
		ent->think = thermalThinkStandard;
		ent->nextthink = level.time;
		ent->r.svFlags |= SVF_BROADCAST;//so everyone hears/sees the explosion?
	}
	else
	{
		vec3_t	origin;
		vec3_t	dir={0,0,1};
	    const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (ent);

		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		origin[2] += 8;
		SnapVector( origin );
		G_SetOrigin( ent, origin );

		ent->s.eType = ET_GENERAL;
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );
		ent->freeAfterEvent = qtrue;
	    
        if ( fireMode->damageTypeHandle )
        {
            JKG_DoSplashDamage (fireMode->damageTypeHandle, ent->r.currentOrigin, ent, ent->parent, ent, ent->splashMethodOfDeath);
        }
        else if (G_RadiusDamage( ent->r.currentOrigin, ent->parent,  ent->splashDamage, ent->splashRadius, 
				ent, ent, ent->splashMethodOfDeath))
		{
			if(g_entities[ent->r.ownerNum].client)
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
        
        if ( fireMode->secondaryDmgHandle )
        {
            JKG_DoSplashDamage (fireMode->secondaryDmgHandle, ent->r.currentOrigin, ent, ent->parent, ent, ent->splashMethodOfDeath);
        }

		/*if (G_RadiusDamage( ent->r.currentOrigin, ent->parent,  ent->splashDamage, ent->splashRadius, 
				ent, ent, ent->splashMethodOfDeath))
		{
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}*/

		trap->LinkEntity( (sharedEntity_t *)ent );
	}
}

void thermalThinkStandard(gentity_t *ent)
{
	if ( ent->genericValue5 < level.time )
	{
		thermalDetonatorExplode( ent );
		return;
	}

	G_RunObject( ent );
	ent->nextthink = level.time;
}

//---------------------------------------------------------
gentity_t *WP_FireThermalDetonator( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;
	float chargeAmount = 1.0f; // default of full charge
	
	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();
	
	bolt->physicsObject = qtrue;

	bolt->classname = "thermal_detonator";
	bolt->think = thermalThinkStandard;
	bolt->nextthink = level.time;
	bolt->touch = touch_NULL;

	// How 'bout we give this thing a size...
	VectorSet( bolt->r.mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->r.maxs, 3.0f, 3.0f, 3.0f );
	bolt->clipmask = MASK_SHOT;

	W_TraceSetStart( ent, start, bolt->r.mins, bolt->r.maxs );//make sure our start point isn't on the other side of a wall

	if ( ent->client )
	{
		chargeAmount = level.time - ent->client->ps.weaponChargeTime;
	}

	// get charge amount
	chargeAmount = chargeAmount / ( float ) TD_VELOCITY;

	if ( chargeAmount > 1.0f )
	{
		chargeAmount = 1.0f;
	}
	else if ( chargeAmount < TD_MIN_CHARGE )
	{
		chargeAmount = TD_MIN_CHARGE;
	}

	// normal ones bounce, alt ones explode on impact
	bolt->genericValue5 = level.time + TD_TIME; // How long 'til she blows
	bolt->s.pos.trType = TR_GRAVITY;
	bolt->parent = ent;
	bolt->r.ownerNum = ent->s.number;
	VectorScale( dir, TD_VELOCITY * chargeAmount, bolt->s.pos.trDelta );

	if ( ent->health >= 0 )
	{
		bolt->s.pos.trDelta[2] += 120;
	}

	if ( !altFire )
	{
		bolt->flags |= FL_BOUNCE_HALF;
		bolt->flags |= FL_BOUNCE;
	}

	//bolt->s.loopSound = G_SoundIndex( "sound/weapons/thermal/thermloop.wav" );
	//bolt->s.loopIsSoundset = qfalse;

	bolt->damage = TD_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = TD_SPLASH_DAM;
	bolt->splashRadius = TD_SPLASH_RAD;

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_THERMAL;

	bolt->methodOfDeath = MOD_THERMAL;
	bolt->splashMethodOfDeath = MOD_THERMAL_SPLASH;

	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	VectorCopy( start, bolt->pos2 );

	bolt->bounceCount = -5;
	return bolt;
}


// JKG - used when a player is killed who was holding a primed thermal detonator
//---------------------------------------------------------
gentity_t *WP_DropThermalDetonator( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	gentity_t	*bolt;
	vec3_t		dir, start;
	
	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	bolt = G_Spawn();
	
	bolt->physicsObject = qtrue;

	bolt->classname = "thermal_detonator";
	bolt->think = thermalThinkStandard;
	bolt->nextthink = level.time;
	bolt->touch = touch_NULL;

	// How 'bout we give this thing a size...
	VectorSet( bolt->r.mins, -3.0f, -3.0f, -3.0f );
	VectorSet( bolt->r.maxs, 3.0f, 3.0f, 3.0f );
	bolt->clipmask = MASK_SHOT;

	W_TraceSetStart( ent, start, bolt->r.mins, bolt->r.maxs );//make sure our start point isn't on the other side of a wall

	// normal ones bounce, alt ones explode on impact
	bolt->genericValue5 = level.time + TD_TIME; // How long 'til she blows
	bolt->s.pos.trType = TR_GRAVITY;
	bolt->parent = ent;
	bolt->r.ownerNum = ent->s.number;

	if ( ent->health >= 0 )
	{
		bolt->s.pos.trDelta[2] += 120;
	}

	if ( !altFire )
	{
		bolt->flags |= FL_BOUNCE_HALF;
	}

	//bolt->s.loopSound = G_SoundIndex( "sound/weapons/thermal/thermloop.wav" );
	//bolt->s.loopIsSoundset = qfalse;

	bolt->damage = TD_DAMAGE;
	bolt->dflags = 0;
	bolt->splashDamage = TD_SPLASH_DAM;
	bolt->splashRadius = TD_SPLASH_RAD;

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weapon = WP_THERMAL;

	bolt->methodOfDeath = MOD_THERMAL;
	bolt->splashMethodOfDeath = MOD_THERMAL_SPLASH;

	bolt->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

	VectorCopy( start, bolt->pos2 );

	bolt->bounceCount = -5;

	return bolt;
}

gentity_t *WP_DropThermal( gentity_t *ent )
{
	AngleVectors( ent->client->ps.viewangles, forward, vright, up );
	return (WP_FireThermalDetonator( ent, qfalse ));
}


//---------------------------------------------------------
qboolean WP_LobFire( gentity_t *self, vec3_t start, vec3_t target, vec3_t mins, vec3_t maxs, int clipmask, 
				vec3_t velocity, qboolean tracePath, int ignoreEntNum, int enemyNum,
				float minSpeed, float maxSpeed, float idealSpeed, qboolean mustHit )
//---------------------------------------------------------
{ //for the galak mech NPC
	float	targetDist, shotSpeed, speedInc = 100, travelTime, impactDist, bestImpactDist = Q3_INFINITE;//fireSpeed, 
	vec3_t	targetDir, shotVel, failCase; 
	trace_t	trace;
	trajectory_t	tr;
	qboolean	blocked;
	int		elapsedTime, skipNum, timeStep = 500, hitCount = 0, maxHits = 7;
	vec3_t	lastPos, testPos;
	gentity_t	*traceEnt;
	
	if ( !idealSpeed )
	{
		idealSpeed = 300;
	}
	else if ( idealSpeed < speedInc )
	{
		idealSpeed = speedInc;
	}
	shotSpeed = idealSpeed;
	skipNum = (idealSpeed-speedInc)/speedInc;
	if ( !minSpeed )
	{
		minSpeed = 100;
	}
	if ( !maxSpeed )
	{
		maxSpeed = 900;
	}
	while ( hitCount < maxHits )
	{
		VectorSubtract( target, start, targetDir );
		targetDist = VectorNormalize( targetDir );

		VectorScale( targetDir, shotSpeed, shotVel );
		travelTime = targetDist/shotSpeed;
		shotVel[2] += travelTime * 0.5 * g_gravity.value;

		if ( !hitCount )		
		{//save the first (ideal) one as the failCase (fallback value)
			if ( !mustHit )
			{//default is fine as a return value
				VectorCopy( shotVel, failCase );
			}
		}

		if ( tracePath )
		{//do a rough trace of the path
			blocked = qfalse;

			VectorCopy( start, tr.trBase );
			VectorCopy( shotVel, tr.trDelta );
			tr.trType = TR_GRAVITY;
			tr.trTime = level.time;
			travelTime *= 1000.0f;
			VectorCopy( start, lastPos );
			
			//This may be kind of wasteful, especially on long throws... use larger steps?  Divide the travelTime into a certain hard number of slices?  Trace just to apex and down?
			for ( elapsedTime = timeStep; elapsedTime < floor(travelTime)+timeStep; elapsedTime += timeStep )
			{
				if ( (float)elapsedTime > travelTime )
				{//cap it
					elapsedTime = floor( travelTime );
				}
				BG_EvaluateTrajectory( &tr, level.time + elapsedTime, testPos );
				trap->Trace( &trace, lastPos, mins, maxs, testPos, ignoreEntNum, clipmask , 0, 0, 0);

				if ( trace.allsolid || trace.startsolid )
				{
					blocked = qtrue;
					break;
				}
				if ( trace.fraction < 1.0f )
				{//hit something
					if ( trace.entityNum == enemyNum )
					{//hit the enemy, that's perfect!
						break;
					}
					else if ( trace.plane.normal[2] > 0.7 && DistanceSquared( trace.endpos, target ) < 4096 )//hit within 64 of desired location, should be okay
					{//close enough!
						break;
					}
					else
					{//FIXME: maybe find the extents of this brush and go above or below it on next try somehow?
						impactDist = DistanceSquared( trace.endpos, target );
						if ( impactDist < bestImpactDist )
						{
							bestImpactDist = impactDist;
							VectorCopy( shotVel, failCase );
						}
						blocked = qtrue;
						//see if we should store this as the failCase
						if ( trace.entityNum < ENTITYNUM_WORLD )
						{//hit an ent
							traceEnt = &g_entities[trace.entityNum];
							if ( traceEnt && traceEnt->takedamage && !OnSameTeam( self, traceEnt ) )
							{//hit something breakable, so that's okay
								//we haven't found a clear shot yet so use this as the failcase
								VectorCopy( shotVel, failCase );
							}
						}
						break;
					}
				}
				if ( elapsedTime == floor( travelTime ) )
				{//reached end, all clear
					break;
				}
				else
				{
					//all clear, try next slice
					VectorCopy( testPos, lastPos );
				}
			}
			if ( blocked )
			{//hit something, adjust speed (which will change arc)
				hitCount++;
				shotSpeed = idealSpeed + ((hitCount-skipNum) * speedInc);//from min to max (skipping ideal)
				if ( hitCount >= skipNum )
				{//skip ideal since that was the first value we tested
					shotSpeed += speedInc;
				}
			}
			else
			{//made it!
				break;
			}
		}
		else
		{//no need to check the path, go with first calc
			break;
		}
	}

	if ( hitCount >= maxHits )
	{//NOTE: worst case scenario, use the one that impacted closest to the target (or just use the first try...?)
		VectorCopy( failCase, velocity );
		return qfalse;
	}
	VectorCopy( shotVel, velocity );
	return qtrue;
}

/*
======================================================================

LASER TRAP / TRIP MINE

======================================================================
*/
#define LT_DAMAGE			200
#define LT_SPLASH_RAD		348.0f//256.0f
#define LT_SPLASH_DAM		200
#define LT_VELOCITY			900.0f
#define LT_SIZE				1.5f
#define LT_ALT_TIME			4000		// Prox mines take some time to arm
#define	LT_ACTIVATION_DELAY	1000
#define	LT_DELAY_TIME		50

void laserTrapExplode( gentity_t *self )
{
	vec3_t v;
	gentity_t *te;
	self->takedamage = qfalse;

	if (self->activator)
	{
		//G_RadiusDamage( self->r.currentOrigin, self->activator, self->splashDamage, self->splashRadius, self, self, MOD_TRIP_MINE_SPLASH/*MOD_LT_SPLASH*/ );
		const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (self);
		if ( fireMode->damageTypeHandle )
		{
		    JKG_DoSplashDamage (fireMode->damageTypeHandle, self->r.currentOrigin, self, self->activator, self, MOD_TRIP_MINE_SPLASH);
		}
		else
		{
		    G_RadiusDamage( self->r.currentOrigin, self->activator, self->splashDamage, self->splashRadius, self, self, MOD_TRIP_MINE_SPLASH/*MOD_LT_SPLASH*/ );
		}
		
		if ( fireMode->secondaryDmgHandle )
		{
		    JKG_DoSplashDamage (fireMode->secondaryDmgHandle, self->r.currentOrigin, self, self->activator, self, MOD_TRIP_MINE_SPLASH);
		}

		if( self->enemy && self->enemy->client && !OnSameTeam(self->activator, self->enemy) && self->activator != self->enemy )
		{
			// Give us some credits, we're a good person etc
			self->enemy->client->ps.credits += 35;
			trap->SendServerCommand(self->enemy->s.number, "notify 1 \"Destroyed Enemy Equipment: +35 Credits\"");
		}
	}

	VectorCopy(self->s.pos.trDelta, v);
	//Explode outward from the surface

	if (self->s.time == -2)
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
	}
	te = G_TempEntity (self->r.currentOrigin, EV_EXPLOSIVE_BLOW);
	VectorCopy (self->r.currentOrigin, te->s.origin);
	VectorCopy (v, te->s.angles);
	te->s.weapon = self->s.weapon;
	te->s.weaponVariation = self->s.weaponVariation;
	te->s.firingMode = self->s.firingMode;

	self->think = G_FreeEntity;
	self->nextthink = level.time;
}

void laserTrapDelayedExplode( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	self->think = laserTrapExplode;
	self->nextthink = level.time + FRAMETIME;
	self->takedamage = qfalse;
	if ( attacker && !attacker->s.number )
	{
		//less damage when shot by player
		self->splashDamage /= 3;
		self->splashRadius /= 3;
	}
}

void touchLaserTrap( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	if (other && other->s.number < ENTITYNUM_WORLD)
	{ //just explode if we hit any entity. This way we don't have things happening like tripmines floating
	  //in the air after getting stuck to a moving door
		if ( ent->activator != other && !TeamFriendly( ent->activator->s.number, other->s.number ) )
		{
			ent->touch = 0;
			ent->nextthink = level.time + FRAMETIME;
			ent->think = laserTrapExplode;
			VectorCopy(trace->plane.normal, ent->s.pos.trDelta);
		}
	}
	else
	{
		ent->touch = 0;
		if (trace->entityNum != ENTITYNUM_NONE)
		{
			ent->enemy = &g_entities[trace->entityNum];
		}
		laserTrapStick(ent, trace->endpos, trace->plane.normal);
	}
}

void proxMineThink(gentity_t *ent)
{
	int i = 0;
	gentity_t *cl;
	gentity_t *owner = NULL;

	if (ent->r.ownerNum < ENTITYNUM_WORLD)
	{
		owner = &g_entities[ent->r.ownerNum];
	}

	//turn on the proximity mine and the cloaking device
	if ( !(ent->s.eFlags&EF_FIRING) )
	{//arm me
		//G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/laser_trap/warning.wav" ) );
		gentity_t *te = G_TempEntity (ent->r.currentOrigin, EV_EXPLOSIVE_ARM);
        te->s.weapon = WP_TRIP_MINE;
        te->s.weaponVariation = 0;
        te->s.eFlags |= EF_ALT_FIRING;
	        
		ent->s.eFlags |= EF_FIRING;
	}

	ent->nextthink = level.time;

	if ( !owner ||
		!owner->inuse ||
		!owner->client ||
		owner->client->pers.connected != CON_CONNECTED)
	{ //time to die!
		ent->think = laserTrapExplode;
		return;
	}

	while (i < MAX_CLIENTS)
	{ //eh, just check for clients, don't care about anyone else...
		cl = &g_entities[i];

		if (cl->inuse && cl->client && cl->client->pers.connected == CON_CONNECTED &&
			owner != cl && !TeamFriendly( cl->s.number, owner->s.number ) 
			&& cl->client->sess.sessionTeam != TEAM_SPECTATOR &&
			cl->client->tempSpectate < level.time && cl->health > 0 && !cl->client->noclip)
		{
			if (!OnSameTeam(owner, cl)/* || g_friendlyFire.integer*/)
			{ //not on the same team, or friendly fire is enabled
				// Nerfed the friendly fire check. Mines are hard enough to deal with in TFFA as it is --eez
				vec3_t v;

				VectorSubtract(ent->r.currentOrigin, cl->client->ps.origin, v);
				if (VectorLength(v) < (ent->splashRadius/2.0f))
				{
					// If he's moving too slow, dont trigger
					if (VectorLength(cl->client->ps.velocity) > 100) {
						ent->think = laserTrapExplode;
						return;
					}
				}
			}
		}
		i++;
	}
}

void laserTrapThink ( gentity_t *ent )
{
	gentity_t	*traceEnt;
	gentity_t	*owner = NULL;
	vec3_t		end;
	trace_t		tr;

	//just relink it every think
	trap->LinkEntity((sharedEntity_t *)ent);

	//turn on the beam effect
	if ( !(ent->s.eFlags&EF_FIRING) )
	{//arm me
		//G_Sound( ent, CHAN_WEAPON, G_SoundIndex( "sound/weapons/laser_trap/warning.wav" ) );
		gentity_t *te = G_TempEntity (ent->r.currentOrigin, EV_EXPLOSIVE_ARM);
        te->s.weapon = WP_TRIP_MINE;
        te->s.weaponVariation = 0;
	        
		ent->s.eFlags |= EF_FIRING;
	}
	ent->think = laserTrapThink;
	ent->nextthink = level.time + FRAMETIME;

	// Find the main impact point
	VectorMA ( ent->s.pos.trBase, 1024, ent->movedir, end );
	trap->Trace( &tr, ent->r.currentOrigin, NULL, NULL, end, ent->s.number, MASK_SHOT, 0, 0, 0);
	
	traceEnt = &g_entities[ tr.entityNum ];

	if (ent->r.ownerNum < ENTITYNUM_WORLD)
	{
		owner = &g_entities[ent->r.ownerNum];
	}

	ent->s.time = -1; //let all clients know to draw a beam from this guy

	if ( traceEnt->client || tr.startsolid )
	{
		if (traceEnt == owner || TeamFriendly( traceEnt->s.number, owner->s.number ) || (traceEnt->client && traceEnt->client->noclip)) {
			return;
		}
		//go boom
		ent->touch = 0;
		ent->nextthink = level.time + LT_DELAY_TIME;
		ent->think = laserTrapExplode;
	}
}

void laserTrapStick( gentity_t *ent, vec3_t endpos, vec3_t normal )
{
    gentity_t *tent = NULL;

    // JKG: Fix for floating trip mines
    VectorMA (endpos, -LT_SIZE, normal, endpos);
	G_SetOrigin( ent, endpos );
	VectorCopy( normal, ent->pos1 );

	VectorClear( ent->s.apos.trDelta );
	// This will orient the object to face in the direction of the normal
	VectorCopy( normal, ent->s.pos.trDelta );
	//VectorScale( normal, -1, ent->s.pos.trDelta );
	ent->s.pos.trTime = level.time;
	
	
	//This does nothing, cg_missile makes assumptions about direction of travel controlling angles
	vectoangles( normal, ent->s.apos.trBase );
	VectorClear( ent->s.apos.trDelta );
	ent->s.apos.trType = TR_STATIONARY;
	VectorCopy( ent->s.apos.trBase, ent->s.angles );
	VectorCopy( ent->s.angles, ent->r.currentAngles );
	
	
	tent = G_TempEntity( ent->r.currentOrigin, EV_MISSILE_MISS );
	tent->s.weapon = WP_TRIP_MINE;
	tent->s.weaponVariation = 0;
	tent->parent = ent;
	tent->r.ownerNum = ent->s.number;
	
	if ( ent->count )
	{//a tripwire
		//add draw line flag
		VectorCopy( normal, ent->movedir );
		ent->think = laserTrapThink;
		ent->nextthink = level.time + LT_ACTIVATION_DELAY;//delay the activation
		ent->touch = touch_NULL;
		//make it shootable
		ent->takedamage = qtrue;
		ent->health = 5;
		ent->die = laserTrapDelayedExplode;

		//shove the box through the wall
		VectorSet( ent->r.mins, -LT_SIZE*2, -LT_SIZE*2, -LT_SIZE*2 );
		VectorSet( ent->r.maxs, LT_SIZE*2, LT_SIZE*2, LT_SIZE*2 );

		//so that the owner can blow it up with projectiles
		ent->r.svFlags |= SVF_OWNERNOTSHARED;
	}
	else
	{
	    tent->s.eFlags |= EF_ALT_FIRING;
	
		ent->touch = touch_NULL;	// don't want teammates touching this and setting it off --eez
		ent->think = proxMineThink;//laserTrapExplode;
		ent->genericValue15 = 0; // Dont explode automatically //level.time + 30000; //auto-explode after 30 seconds.
		ent->nextthink = level.time + LT_ALT_TIME; // How long 'til she blows

		//make it shootable
		ent->takedamage = qtrue;
		ent->health = 5;
		ent->die = laserTrapDelayedExplode;

		//shove the box through the wall
		VectorSet( ent->r.mins, -LT_SIZE*2, -LT_SIZE*2, -LT_SIZE*2 );
		VectorSet( ent->r.maxs, LT_SIZE*2, LT_SIZE*2, LT_SIZE*2 );

		//so that the owner can blow it up with projectiles
		ent->r.svFlags |= SVF_OWNERNOTSHARED;

		if ( !(ent->s.eFlags&EF_FIRING) )
		{//arm me
		    	gentity_t *te = G_TempEntity (ent->r.currentOrigin, EV_EXPLOSIVE_ARM);
	        	te->s.weapon = WP_TRIP_MINE;
	        	te->s.weaponVariation = 0;
	        	te->s.eFlags |= EF_ALT_FIRING;	        
			ent->s.time = -1;
			ent->s.bolt2 = 1;
		}
	}
}

void TrapThink(gentity_t *ent)
{ //laser trap think
	ent->nextthink = level.time + 50;
	G_RunObject(ent);
}

void CreateLaserTrap( gentity_t *laserTrap, vec3_t start, gentity_t *owner )
{ //create a laser trap entity
	laserTrap->classname = "laserTrap";
	laserTrap->flags |= FL_BOUNCE_HALF;
	laserTrap->s.eFlags |= EF_MISSILE_STICK;
	laserTrap->splashDamage = LT_SPLASH_DAM;
	laserTrap->splashRadius = LT_SPLASH_RAD;
	laserTrap->damage = LT_DAMAGE;
	laserTrap->methodOfDeath = MOD_TRIP_MINE_SPLASH;
	laserTrap->splashMethodOfDeath = MOD_TRIP_MINE_SPLASH;
	laserTrap->s.eType = ET_GENERAL;
	laserTrap->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	laserTrap->s.weapon = WP_TRIP_MINE;
	laserTrap->s.weaponVariation = owner->s.weaponVariation;
	laserTrap->s.pos.trType = TR_GRAVITY;
	laserTrap->r.contents = MASK_SHOT;
	laserTrap->parent = owner;
	laserTrap->activator = owner;
	laserTrap->r.ownerNum = owner->s.number;
	laserTrap->s.owner = owner->s.number; // For selective hiding of the beam effect
	VectorSet( laserTrap->r.mins, -LT_SIZE, -LT_SIZE, -LT_SIZE );
	VectorSet( laserTrap->r.maxs, LT_SIZE, LT_SIZE, LT_SIZE );
	laserTrap->clipmask = MASK_SHOT;
	laserTrap->s.solid = 2;

	laserTrap->s.genericenemyindex = owner->s.number+MAX_GENTITIES;

	laserTrap->health = 1;

	laserTrap->s.time = 0;

	laserTrap->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( start, laserTrap->s.pos.trBase );
	SnapVector( laserTrap->s.pos.trBase );			// save net bandwidth
	
	SnapVector( laserTrap->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, laserTrap->r.currentOrigin);

	laserTrap->s.apos.trType = TR_GRAVITY;
	laserTrap->s.apos.trTime = level.time;
	laserTrap->s.apos.trBase[YAW] = rand()%360;
	laserTrap->s.apos.trBase[PITCH] = rand()%360;
	laserTrap->s.apos.trBase[ROLL] = rand()%360;

	if (rand()%10 < 5)
	{
		laserTrap->s.apos.trBase[YAW] = -laserTrap->s.apos.trBase[YAW];
	}

	VectorCopy( start, laserTrap->pos2 );
	laserTrap->touch = touchLaserTrap;
	laserTrap->think = TrapThink;
	laserTrap->nextthink = level.time + 50;
}

void WP_PlaceLaserTrap( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*laserTrap;
	gentity_t	*found = NULL;
	vec3_t		dir, start;
	int			trapcount = 0;
	int			foundLaserTraps[MAX_GENTITIES];
	int			trapcount_org;
	int			lowestTimeStamp;
	int			removeMe;
	int			i;

	foundLaserTraps[0] = ENTITYNUM_NONE;

	VectorCopy( forward, dir );
	VectorCopy( muzzle, start );

	laserTrap = G_Spawn();
	
	//limit to 10 placed at any one time
	//see how many there are now
	while ( (found = G_Find( found, FOFS(classname), "laserTrap" )) != NULL )
	{
		if ( found->parent != ent )
		{
			continue;
		}
		foundLaserTraps[trapcount++] = found->s.number;
	}
	//now remove first ones we find until there are only 9 left
	found = NULL;
	trapcount_org = trapcount;
	lowestTimeStamp = level.time;
	while ( trapcount > 9)
	{
		removeMe = -1;
		for ( i = 0; i < trapcount_org; i++ )
		{
			if ( foundLaserTraps[i] == ENTITYNUM_NONE )
			{
				continue;
			}
			found = &g_entities[foundLaserTraps[i]];
			if ( laserTrap && found->setTime < lowestTimeStamp )
			{
				removeMe = i;
				lowestTimeStamp = found->setTime;
			}
		}
		if ( removeMe != -1 )
		{
			//remove it... or blow it?
			if ( &g_entities[foundLaserTraps[removeMe]] == NULL )
			{
				break;
			}
			else
			{
				G_FreeEntity( &g_entities[foundLaserTraps[removeMe]] );
			}
			foundLaserTraps[removeMe] = ENTITYNUM_NONE;
			trapcount--;
		}
		else
		{
			break;
		}
	}

	//now make the new one
	CreateLaserTrap( laserTrap, start, ent );

	//set player-created-specific fields
	laserTrap->setTime = level.time;//remember when we placed it

	laserTrap->s.firingMode = ent->s.firingMode;

	if (!alt_fire)
	{//tripwire
		laserTrap->count = 1;
	}
	else
	{
	    laserTrap->s.eFlags |= EF_ALT_FIRING;
	}

	//move it
	laserTrap->s.pos.trType = TR_GRAVITY;

	if (alt_fire)
	{
		VectorScale( dir, 512, laserTrap->s.pos.trDelta );
	}
	else
	{
		VectorScale( dir, 256, laserTrap->s.pos.trDelta );
	}

	trap->LinkEntity((sharedEntity_t *)laserTrap);
}


/*
======================================================================

DET PACK

======================================================================
*/
void VectorNPos(vec3_t in, vec3_t out)
{
	if (in[0] < 0) { out[0] = -in[0]; } else { out[0] = in[0]; }
	if (in[1] < 0) { out[1] = -in[1]; } else { out[1] = in[1]; }
	if (in[2] < 0) { out[2] = -in[2]; } else { out[2] = in[2]; }
}

void DetPackBlow(gentity_t *self);

void charge_stick (gentity_t *self, gentity_t *other, trace_t *trace)
{
	gentity_t	*tent;

	if ( other 
		&& (other->flags&FL_BBRUSH)
		&& other->s.pos.trType == TR_STATIONARY
		&& other->s.apos.trType == TR_STATIONARY )
	{//a perfectly still breakable brush, let us attach directly to it!
		self->target_ent = other;//remember them when we blow up
	}
	else if ( other 
		&& other->s.number < ENTITYNUM_WORLD
		&& other->s.eType == ET_MOVER
		&& trace->plane.normal[2] > 0 )
	{//stick to it?
		self->s.groundEntityNum = other->s.number;
	}
	else if (other && other->s.number < ENTITYNUM_WORLD &&
		(other->client || !other->s.weapon))
	{ //hit another entity that is not stickable, "bounce" off
		vec3_t vNor, tN;

		VectorCopy(trace->plane.normal, vNor);
		VectorNormalize(vNor);
		VectorNPos(self->s.pos.trDelta, tN);
		self->s.pos.trDelta[0] += vNor[0]*(tN[0]*(((float)Q_irand(1, 10))*0.1));
		self->s.pos.trDelta[1] += vNor[1]*(tN[1]*(((float)Q_irand(1, 10))*0.1));
		self->s.pos.trDelta[2] += vNor[1]*(tN[2]*(((float)Q_irand(1, 10))*0.1));

		vectoangles(vNor, self->s.angles);
		vectoangles(vNor, self->s.apos.trBase);
		self->touch = charge_stick;
		return;
	}
	else if (other && other->s.number < ENTITYNUM_WORLD)
	{ //hit an entity that we just want to explode on (probably another projectile or something)
		vec3_t v;
		const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (self);

		self->touch = 0;
		self->think = 0;
		self->nextthink = 0;

		self->takedamage = qfalse;

		VectorClear(self->s.apos.trDelta);
		self->s.apos.trType = TR_STATIONARY;

		//G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
		
		if ( fireMode->damageTypeHandle )
		{
		    JKG_DoSplashDamage (fireMode->damageTypeHandle, self->r.currentOrigin, self, self->parent, self, MOD_DET_PACK_SPLASH);
		}
		else
		{
		    G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
		}
		
		if ( fireMode->secondaryDmgHandle )
		{
		    JKG_DoSplashDamage (fireMode->secondaryDmgHandle, self->r.currentOrigin, self, self->parent, self, MOD_DET_PACK_SPLASH);
		}
		
		VectorCopy(trace->plane.normal, v);
		VectorCopy(v, self->pos2);
		self->count = -1;
		G_PlayEffect(EFFECT_EXPLOSION_DETPACK, self->r.currentOrigin, v);

		self->think = G_FreeEntity;
		self->nextthink = level.time;
		return;
	}

	//if we get here I guess we hit hte world so we can stick to it

	self->touch = 0;
	self->think = DetPackBlow;
	self->nextthink = 0;

	VectorClear(self->s.apos.trDelta);
	self->s.apos.trType = TR_STATIONARY;

	self->s.pos.trType = TR_STATIONARY;
	// JKG: Make the detpack sit properly on the attached surface
	VectorNormalize(trace->plane.normal);
    VectorMA (trace->endpos, -2.0f, trace->plane.normal, self->r.currentOrigin);
    
	VectorCopy( self->r.currentOrigin, self->s.origin );
	VectorCopy( self->r.currentOrigin, self->s.pos.trBase );
	VectorClear( self->s.pos.trDelta );

	VectorClear( self->s.apos.trDelta );

	vectoangles(trace->plane.normal, self->s.angles);
	VectorCopy(self->s.angles, self->r.currentAngles );
	VectorCopy(self->s.angles, self->s.apos.trBase);

	VectorCopy(trace->plane.normal, self->pos2);
	self->count = -1;

	//G_Sound(self, CHAN_WEAPON, G_SoundIndex("sound/weapons/detpack/stick.wav"));
		
	tent = G_TempEntity( self->r.currentOrigin, EV_MISSILE_MISS );
	tent->s.weapon = WP_DET_PACK;
	tent->s.weaponVariation = 0;
	tent->parent = self;
	tent->r.ownerNum = self->s.number;

	//so that the owner can blow it up with projectiles
	self->r.svFlags |= SVF_OWNERNOTSHARED;
}

void DetPackBlow(gentity_t *self)
{
	vec3_t v;
	gentity_t *te;
	const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (self);

	self->pain = 0;
	self->die = 0;
	self->takedamage = qfalse;

	if ( self->target_ent )
	{//we were attached to something, do *direct* damage to it!
		//G_Damage( self->target_ent, self, &g_entities[self->r.ownerNum], v, self->r.currentOrigin, self->damage, 0, MOD_DET_PACK_SPLASH );
		if ( fireMode->damageTypeHandle )
		{
		    JKG_DoDirectDamage (fireMode->damageTypeHandle, self->target_ent, self, self->parent, vec3_origin, self->r.currentOrigin, 0, MOD_DET_PACK_SPLASH);
		}
		else
		{
		    G_Damage( self->target_ent, self, &g_entities[self->r.ownerNum], v, self->r.currentOrigin, self->damage, 0, MOD_DET_PACK_SPLASH );
		}
		
		if ( fireMode->secondaryDmgHandle )
		{
		    JKG_DoDirectDamage (fireMode->secondaryDmgHandle, self->target_ent, self, self->parent, vec3_origin, self->r.currentOrigin, 0, MOD_DET_PACK_SPLASH);
		}
	}
	
	//G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
	if ( fireMode->damageTypeHandle )
	{
	    JKG_DoSplashDamage (fireMode->damageTypeHandle, self->r.currentOrigin, self, self->parent, self, MOD_DET_PACK_SPLASH);
	}
	else
	{
	    G_RadiusDamage( self->r.currentOrigin, self->parent, self->splashDamage, self->splashRadius, self, self, MOD_DET_PACK_SPLASH );
	}
	
	if ( fireMode->secondaryDmgHandle )
	{
	    JKG_DoSplashDamage (fireMode->secondaryDmgHandle, self->r.currentOrigin, self, self->parent, self, MOD_DET_PACK_SPLASH);
	}
	v[0] = 0;
	v[1] = 0;
	v[2] = 1;

	if (self->count == -1)
	{
		VectorCopy(self->pos2, v);
	}

	//G_PlayEffect(EFFECT_EXPLOSION_DETPACK, self->r.currentOrigin, v);
	te = G_TempEntity (self->r.currentOrigin, EV_EXPLOSIVE_BLOW);
	VectorCopy (self->r.currentOrigin, te->s.origin);
	VectorCopy (v, te->s.angles);
	te->s.weapon = self->s.weapon;
	te->s.weaponVariation = self->s.weaponVariation;
	te->s.firingMode = self->parent->s.firingMode;

	self->think = G_FreeEntity;
	self->nextthink = level.time;

	// Right, now we gotta do the equipment stuff
	if( self->parent && self->enemy && self->parent->client && self->enemy->client)
	{
		if(self->parent != self->enemy && !OnSameTeam(self->parent, self->enemy))
		{
			self->enemy->client->ps.credits += 25;
			trap->SendServerCommand(self->enemy->s.number, "notify 1 \"Destroyed Enemy Equipment: +25 Credits\"");
		}
	}
}

void DetPackPain(gentity_t *self, gentity_t *attacker, int damage)
{
	self->think = DetPackBlow;
	self->nextthink = level.time + Q_irand(50, 100);
	self->takedamage = qfalse;
}

void DetPackDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->think = DetPackBlow;
	self->nextthink = level.time + Q_irand(50, 100);
	self->takedamage = qfalse;
}

void drop_charge (gentity_t *self, vec3_t start, vec3_t dir) 
{
	gentity_t	*bolt;
	int damage = WP_GetWeaponDamage (self, qfalse);

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->classname = "detpack";
	bolt->nextthink = level.time + FRAMETIME;
	bolt->think = G_RunObject;
	bolt->s.eType = ET_GENERAL;

	bolt->parent = self;
	bolt->r.ownerNum = self->s.number;
	bolt->damage = damage;
	bolt->splashDamage = damage;
	bolt->splashRadius = WP_GetWeaponSplashRange (self, qfalse);
	bolt->methodOfDeath = MOD_DET_PACK_SPLASH;
	bolt->splashMethodOfDeath = MOD_DET_PACK_SPLASH;
	bolt->clipmask = MASK_SHOT;
	bolt->s.solid = 2;
	bolt->r.contents = MASK_SHOT;
	bolt->touch = charge_stick;

	bolt->physicsObject = qtrue;

	bolt->s.genericenemyindex = self->s.number+MAX_GENTITIES;
	//rww - so client prediction knows we own this and won't hit it

	VectorSet( bolt->r.mins, -2, -2, -2 );
	VectorSet( bolt->r.maxs, 2, 2, 2 );

	bolt->health = 1;
	bolt->takedamage = qtrue;
	bolt->pain = DetPackPain;
	bolt->die = DetPackDie;

	bolt->s.weapon = WP_DET_PACK;
	bolt->s.weaponVariation = self->s.weaponVariation;

	bolt->setTime = level.time;

	G_SetOrigin(bolt, start);
	bolt->s.pos.trType = TR_GRAVITY;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale(dir, 300, bolt->s.pos.trDelta );
	bolt->s.pos.trTime = level.time;

	bolt->s.apos.trType = TR_GRAVITY;
	bolt->s.apos.trTime = level.time;
	bolt->s.apos.trBase[YAW] = rand()%360;
	bolt->s.apos.trBase[PITCH] = rand()%360;
	bolt->s.apos.trBase[ROLL] = rand()%360;

	if (rand()%10 < 5)
	{
		bolt->s.apos.trBase[YAW] = -bolt->s.apos.trBase[YAW];
	}

	vectoangles(dir, bolt->s.angles);
	VectorCopy(bolt->s.angles, bolt->s.apos.trBase);
	VectorSet(bolt->s.apos.trDelta, 300, 0, 0 );
	bolt->s.apos.trTime = level.time;

	trap->LinkEntity((sharedEntity_t *)bolt);
}

void BlowDetpacks(gentity_t *ent)
{
	gentity_t *found = NULL;

	if ( ent->client->ps.hasDetPackPlanted )
	{
		while ( (found = G_Find( found, FOFS(classname), "detpack") ) != NULL )
		{//loop through all ents and blow the crap out of them!
			if ( found->parent == ent )
			{
				VectorCopy( found->r.currentOrigin, found->s.origin );
				found->think = DetPackBlow;
				found->nextthink = level.time + 100 + random() * 200;
				//G_Sound( found, CHAN_BODY, G_SoundIndex("sound/weapons/detpack/warning.wav") );
			}
		}
		ent->client->ps.hasDetPackPlanted = qfalse;
	}
}

void WP_DropDetPack( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*found = NULL;
	int			trapcount = 0;
	int			foundDetPacks[MAX_GENTITIES] = {ENTITYNUM_NONE};
	int			trapcount_org;
	int			lowestTimeStamp;
	int			removeMe;
	int			i;

	if ( !ent || !ent->client )
	{
		return;
	}

	//limit to 10 placed at any one time
	//see how many there are now
	while ( (found = G_Find( found, FOFS(classname), "detpack" )) != NULL )
	{
		if ( found->parent != ent )
		{
			continue;
		}
		foundDetPacks[trapcount++] = found->s.number;
	}
	//now remove first ones we find until there are only 9 left
	found = NULL;
	trapcount_org = trapcount;
	lowestTimeStamp = level.time;
	while ( trapcount > 9 )
	{
		removeMe = -1;
		for ( i = 0; i < trapcount_org; i++ )
		{
			if ( foundDetPacks[i] == ENTITYNUM_NONE )
			{
				continue;
			}
			found = &g_entities[foundDetPacks[i]];
			if ( found->setTime < lowestTimeStamp )
			{
				removeMe = i;
				lowestTimeStamp = found->setTime;
			}
		}
		if ( removeMe != -1 )
		{
			//remove it... or blow it?
			if ( &g_entities[foundDetPacks[removeMe]] == NULL )
			{
				break;
			}
			else
			{
				G_FreeEntity( &g_entities[foundDetPacks[removeMe]] );
			}
			foundDetPacks[removeMe] = ENTITYNUM_NONE;
			trapcount--;
		}
		else
		{
			break;
		}
	}

	if ( alt_fire  )
	{
		BlowDetpacks(ent);
	}
	else
	{
		AngleVectors( ent->client->ps.viewangles, forward, vright, up );

		WP_CalculateMuzzlePoint( ent, forward, vright, up, muzzle );

		VectorNormalize( forward );
		VectorMA( muzzle, -4, forward, muzzle );
		drop_charge( ent, muzzle, forward );

		ent->client->ps.hasDetPackPlanted = qtrue;
	}
}

void WP_RecalculateTheFreakingMuzzleCrap( gentity_t *ent )
{
	// Nasty hack i embedded in order to keep the grenades from doing funky things
	WP_CalculateAngles( ent );
	AngleVectors( ent->client->ps.viewangles, forward, vright, up );
	WP_CalculateMuzzlePoint( ent, forward, vright, up, muzzle );
}



//---------------------------------------------------------
// FireStunBaton
//---------------------------------------------------------
void WP_FireStunBaton( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;
	vec3_t		muzzleStun;

	if (!ent->client)
	{
		VectorCopy(ent->r.currentOrigin, muzzleStun);
		muzzleStun[2] += 8;
	}
	else
	{
		VectorCopy(ent->client->ps.origin, muzzleStun);
		muzzleStun[2] += ent->client->ps.viewheight-6;
	}

	VectorMA(muzzleStun, 20.0f, forward, muzzleStun);
	VectorMA(muzzleStun, 4.0f, vright, muzzleStun);

	VectorMA( muzzleStun, STUN_BATON_RANGE, forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	trap->Trace( &tr, muzzleStun, mins, maxs, end, ent->s.number, MASK_SHOT , 0, 0, 0);

	if ( tr.entityNum >= ENTITYNUM_WORLD )
	{
		return;
	}

	tr_ent = &g_entities[tr.entityNum];

	if (tr_ent && tr_ent->takedamage && tr_ent->client)
	{ //see if either party is involved in a duel
		if (tr_ent->client->ps.duelInProgress &&
			tr_ent->client->ps.duelIndex != ent->s.number)
		{
			return;
		}

		if (ent->client &&
			ent->client->ps.duelInProgress &&
			ent->client->ps.duelIndex != tr_ent->s.number)
		{
			return;
		}
	}

	if ( tr_ent && tr_ent->takedamage )
	{
	    const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (ent);
		G_PlayEffect( EFFECT_STUNHIT, tr.endpos, tr.plane.normal );

		G_Sound( tr_ent, CHAN_WEAPON, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );
		//G_Damage( tr_ent, ent, ent, forward, tr.endpos, STUN_BATON_DAMAGE, (DAMAGE_NO_KNOCKBACK|DAMAGE_HALF_ABSORB), MOD_STUN_BATON );
		if ( fireMode->damageTypeHandle )
		{
		    JKG_DoDamage (fireMode->damageTypeHandle, tr_ent, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, MOD_STUN_BATON);
		}
		else
		{
		    G_Damage( tr_ent, ent, ent, forward, tr.endpos, STUN_BATON_DAMAGE, (DAMAGE_NO_KNOCKBACK|DAMAGE_HALF_ABSORB), MOD_STUN_BATON );
		}
		
		if ( fireMode->secondaryDmgHandle )
		{
		    JKG_DoDamage (fireMode->secondaryDmgHandle, tr_ent, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, MOD_STUN_BATON);
		}

		if (tr_ent->client)
		{ //if it's a player then use the shock effect
			if ( tr_ent->client->NPC_class == CLASS_VEHICLE )
			{//not on vehicles
				if ( !tr_ent->m_pVehicle
					|| tr_ent->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL 
					|| tr_ent->m_pVehicle->m_pVehicleInfo->type == VH_FLIER )
				{//can zap animals
					tr_ent->client->ps.electrifyTime = level.time + Q_irand( 3000, 4000 );
				}
			}
			else
			{
				tr_ent->client->ps.electrifyTime = level.time + 700;
			}
		}
	}
}


//---------------------------------------------------------
// FireMelee
//---------------------------------------------------------
void WP_FireMelee( gentity_t *ent, qboolean alt_fire )
{
	gentity_t	*tr_ent;
	trace_t		tr;
	vec3_t		mins, maxs, end;
	vec3_t		muzzlePunch;

	if ( ent->s.eType == ET_NPC 
		&& ent->s.weapon != WP_SABER 
		&& ent->enemy
		&& Distance(ent->enemy->r.currentOrigin, ent->r.currentOrigin) <= 72)
	{// UQ1: Added... At close range NPCs can hit you with their rifle butt...
	
	}
	else if (ent->client && ent->client->ps.torsoAnim == BOTH_MELEE2)
	{ //right
		if (ent->client->ps.brokenLimbs & (1 << BROKENLIMB_RARM))
		{
			return;
		}
	}
	else
	{ //left
		if (ent->client->ps.brokenLimbs & (1 << BROKENLIMB_LARM))
		{
			return;
		}
	}

	if (!ent->client)
	{
		VectorCopy(ent->r.currentOrigin, muzzlePunch);
		muzzlePunch[2] += 8;
	}
	else
	{
		VectorCopy(ent->client->ps.origin, muzzlePunch);
		muzzlePunch[2] += ent->client->ps.viewheight-6;
	}

	// Melee "improvements" - sap a little bit of stamina for each punch
	if (ent->client)
	{
		int threshold = bgConstants.staminaDrains.minPunchThreshold;
		if( threshold == 0 && ent->client->ps.forcePower <= 0 )
		{
			ent->client->ps.forcePower = 0;
		}
		else if (threshold && ent->client->ps.forcePower < threshold) {
			return;
		}
		ent->client->ps.forcePower -= bgConstants.staminaDrains.lossFromPunching;
	}

	VectorMA(muzzlePunch, 20.0f, forward, muzzlePunch);
	VectorMA(muzzlePunch, 4.0f, vright, muzzlePunch);

	VectorMA( muzzlePunch, MELEE_RANGE, forward, end );

	VectorSet( maxs, 6, 6, 6 );
	VectorScale( maxs, -1, mins );

	trap->Trace( &tr, muzzlePunch, mins, maxs, end, ent->s.number, MASK_SHOT , 0, 0, 0);

	if (tr.entityNum != ENTITYNUM_NONE)
	{ //hit something
		tr_ent = &g_entities[tr.entityNum];

		G_Sound( ent, CHAN_AUTO, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );

		if (tr_ent->takedamage && tr_ent->client)
		{ //special duel checks
			if (tr_ent->client->ps.duelInProgress &&
				tr_ent->client->ps.duelIndex != ent->s.number)
			{
				return;
			}

			if (ent->client &&
				ent->client->ps.duelInProgress &&
				ent->client->ps.duelIndex != tr_ent->s.number)
			{
				return;
			}
		}

		if ( tr_ent->takedamage )
		{ //damage them, do more damage if we're in the second right hook
			int dmg = 40; // UQ1: (NPCs) Hitting with rifle but does more damage...
			if (ent->s.weapon == WP_MELEE) {
				weaponData_t* wpData = GetWeaponData(WP_MELEE, 0);
				dmg = wpData->firemodes[0].baseDamage;
			}

			G_Damage( tr_ent, ent, ent, forward, tr.endpos, dmg, DAMAGE_NO_ARMOR, MOD_MELEE );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = (int)v[i];
		} else {
			v[i] = (int)v[i] + 1;
		}
	}
}



extern void G_MissileImpact( gentity_t *ent, trace_t *trace );
void WP_TouchVehMissile( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	trace_t	myTrace;
	memcpy( (void *)&myTrace, (void *)trace, sizeof(myTrace) );
	if ( other )
	{
		myTrace.entityNum = other->s.number;
	}
	G_MissileImpact( ent, &myTrace );
}

void WP_CalcVehMuzzle(gentity_t *ent, int muzzleNum)
{
	Vehicle_t *pVeh = ent->m_pVehicle;
	mdxaBone_t boltMatrix;
	vec3_t	vehAngles;

	assert(pVeh);

	if (pVeh->m_iMuzzleTime[muzzleNum] == level.time)
	{ //already done for this frame, don't need to do it again
		return;
	}
	//Uh... how about we set this, hunh...?  :)
	pVeh->m_iMuzzleTime[muzzleNum] = level.time;
	
	VectorCopy( ent->client->ps.viewangles, vehAngles );
	if ( pVeh->m_pVehicleInfo
		&& (pVeh->m_pVehicleInfo->type == VH_ANIMAL
			 ||pVeh->m_pVehicleInfo->type == VH_WALKER
			 ||pVeh->m_pVehicleInfo->type == VH_SPEEDER) )
	{
		vehAngles[PITCH] = vehAngles[ROLL] = 0;
	}

	trap->G2API_GetBoltMatrix_NoRecNoRot(ent->ghoul2, 0, pVeh->m_iMuzzleTag[muzzleNum], &boltMatrix, vehAngles,
		ent->client->ps.origin, level.time, NULL, ent->modelScale);
	BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, pVeh->m_vMuzzlePos[muzzleNum]);
	BG_GiveMeVectorFromMatrix(&boltMatrix, NEGATIVE_Y, pVeh->m_vMuzzleDir[muzzleNum]);
}

void WP_VehWeapSetSolidToOwner( gentity_t *self )
{
	self->r.svFlags |= SVF_OWNERNOTSHARED;
	if ( self->genericValue1 )
	{//expire after a time
		if ( self->genericValue2 )
		{//blow up when your lifetime is up
			self->think = G_ExplodeMissile;//FIXME: custom func?
		}
		else
		{//just remove yourself
			self->think = G_FreeEntity;//FIXME: custom func?
		}
		self->nextthink = level.time + self->genericValue1;
	}
}

#define VEH_HOMING_MISSILE_THINK_TIME		100
gentity_t *WP_FireVehicleWeapon( gentity_t *ent, vec3_t start, vec3_t dir, vehWeaponInfo_t *vehWeapon, qboolean alt_fire, qboolean isTurretWeap )
{
	gentity_t	*missile = NULL;

	//FIXME: add some randomness...?  Inherent inaccuracy stat of weapon?  Pilot skill?
	if ( !vehWeapon )
	{//invalid vehicle weapon
		return NULL;
	}
	else if ( vehWeapon->bIsProjectile )
	{//projectile entity
		vec3_t		mins, maxs;

		VectorSet( maxs, vehWeapon->fWidth/2.0f,vehWeapon->fWidth/2.0f,vehWeapon->fHeight/2.0f );
		VectorScale( maxs, -1, mins );

		//make sure our start point isn't on the other side of a wall
		WP_TraceSetStart( ent, start, mins, maxs );
		
		//FIXME: CUSTOM MODEL?
		//QUERY: alt_fire true or not?  Does it matter?
		missile = CreateMissile( start, dir, vehWeapon->fSpeed, 10000, ent, qfalse );

		missile->classname = "vehicle_proj";
		
		missile->s.genericenemyindex = ent->s.number+MAX_GENTITIES;
		missile->damage = vehWeapon->iDamage;
		missile->splashDamage = vehWeapon->iSplashDamage;
		missile->splashRadius = vehWeapon->fSplashRadius;

		//FIXME: externalize some of these properties?
		missile->dflags = DAMAGE_DEATH_KNOCKBACK;
		missile->clipmask = MASK_SHOT;
		//Maybe by checking flags...?
		if ( vehWeapon->bSaberBlockable )
		{
			missile->clipmask |= CONTENTS_LIGHTSABER;
		}
		/*
		if ( (vehWeapon->iFlags&VWF_KNOCKBACK) )
		{
			missile->dflags &= ~DAMAGE_DEATH_KNOCKBACK;
		}
		if ( (vehWeapon->iFlags&VWF_RADAR) )
		{
			missile->s.eFlags |= EF_RADAROBJECT;
		}
		*/
		// Make it easier to hit things
		VectorCopy( mins, missile->r.mins );
		VectorCopy( maxs, missile->r.maxs );
		//some slightly different stuff for things with bboxes
		if ( vehWeapon->fWidth || vehWeapon->fHeight )
		{//we assume it's a rocket-like thing
			missile->s.weapon = WP_ROCKET_LAUNCHER;//does this really matter?
			missile->methodOfDeath = MOD_VEHICLE;//MOD_ROCKET;
			missile->splashMethodOfDeath = MOD_VEHICLE;//MOD_ROCKET;// ?SPLASH;

			// we don't want it to ever bounce
			missile->bounceCount = 0;

			missile->mass = 10;
		}
		else
		{//a blaster-laser-like thing
			missile->s.weapon = WP_BLASTER;//does this really matter?
			missile->methodOfDeath = MOD_VEHICLE; //count as a heavy weap
			missile->splashMethodOfDeath = MOD_VEHICLE;// ?SPLASH;
			// we don't want it to bounce forever
			missile->bounceCount = 8;
		}
		
		if ( vehWeapon->bHasGravity )
		{//TESTME: is this all we need to do?
			missile->s.weapon = WP_THERMAL;//does this really matter?
			missile->s.pos.trType = TR_GRAVITY;
		}
		
		if ( vehWeapon->bIonWeapon )
		{//so it disables ship shields and sends them out of control
			missile->s.weapon = WP_DEMP2;
		}

		if ( vehWeapon->iHealth )
		{//the missile can take damage
			missile->health = vehWeapon->iHealth;
			missile->takedamage = qtrue;
			missile->r.contents = MASK_SHOT;
			missile->die = RocketDie;
		}

		//pilot should own this projectile on server if we have a pilot
		if (ent->m_pVehicle && ent->m_pVehicle->m_pPilot)
		{//owned by vehicle pilot
			missile->r.ownerNum = ent->m_pVehicle->m_pPilot->s.number;
		}
		else
		{//owned by vehicle?
			missile->r.ownerNum = ent->s.number;
		}

		//set veh as cgame side owner for purpose of fx overrides
		missile->s.owner = ent->s.number;
		/*if ( alt_fire )
		{//use the second weapon's iShotFX
			missile->s.eFlags |= EF_ALT_FIRING;
		}*/
		missile->s.firingMode = ent->s.firingMode;
		if ( isTurretWeap )
		{//look for the turret weapon info on cgame side, not vehicle weapon info
			missile->s.weapon = WP_TURRET;
		}
		if ( vehWeapon->iLifeTime )
		{//expire after a time
			if ( vehWeapon->bExplodeOnExpire )
			{//blow up when your lifetime is up
				missile->think = G_ExplodeMissile;//FIXME: custom func?
			}
			else
			{//just remove yourself
				missile->think = G_FreeEntity;//FIXME: custom func?
			}
			missile->nextthink = level.time + vehWeapon->iLifeTime;
		}
		missile->s.otherEntityNum2 = (vehWeapon-&g_vehWeaponInfo[0]);
		missile->s.eFlags |= EF_JETPACK_ACTIVE;
		//homing
		if ( vehWeapon->fHoming )
		{//homing missile
			if ( ent->client && ent->client->ps.rocketLockIndex != ENTITYNUM_NONE )
			{
				int dif = 0;
				float rTime;
				rTime = ent->client->ps.rocketLockTime;

				if (rTime == -1)
				{
					rTime = ent->client->ps.rocketLastValidTime;
				}

				if ( !vehWeapon->iLockOnTime )
				{//no minimum lock-on time
					dif = 10;//guaranteed lock-on
				}
				else
				{
					float lockTimeInterval = vehWeapon->iLockOnTime/16.0f;
					dif = ( level.time - rTime ) / lockTimeInterval;
				}

				if (dif < 0)
				{
					dif = 0;
				}

				//It's 10 even though it locks client-side at 8, because we want them to have a sturdy lock first, and because there's a slight difference in time between server and client
				if ( dif >= 10 && rTime != -1 )
				{
					missile->enemy = &g_entities[ent->client->ps.rocketLockIndex];

					if (missile->enemy && missile->enemy->client && missile->enemy->health > 0 && !OnSameTeam(ent, missile->enemy))
					{ //if enemy became invalid, died, or is on the same team, then don't seek it
						missile->spawnflags |= 1;//just to let it know it should be faster...
						missile->speed = vehWeapon->fSpeed;
						missile->angle = vehWeapon->fHoming;
						missile->radius = vehWeapon->fHomingFOV;
						//crap, if we have a lifetime, need to store that somewhere else on ent and have rocketThink func check it every frame...
						if ( vehWeapon->iLifeTime )
						{//expire after a time
							missile->genericValue1 = level.time + vehWeapon->iLifeTime;
							missile->genericValue2 = (int)(vehWeapon->bExplodeOnExpire);
						}
						//now go ahead and use the rocketThink func
						missile->think = rocketThink;//FIXME: custom func?
						missile->nextthink = level.time + VEH_HOMING_MISSILE_THINK_TIME;
						missile->s.eFlags |= EF_RADAROBJECT;//FIXME: externalize
						if ( missile->enemy->s.NPC_class == CLASS_VEHICLE )
						{//let vehicle know we've locked on to them
							missile->s.otherEntityNum = missile->enemy->s.number;
						}
					}
				}

				VectorCopy( dir, missile->movedir );
				missile->random = 1.0f;//FIXME: externalize?
			}
		}
		if ( !vehWeapon->fSpeed )
		{//a mine or something?
			//only do damage when someone touches us
			missile->s.weapon = WP_THERMAL;//does this really matter?
			G_SetOrigin( missile, start );
			missile->touch = WP_TouchVehMissile;
			missile->s.eFlags |= EF_RADAROBJECT;//FIXME: externalize
			//crap, if we have a lifetime, need to store that somewhere else on ent and have rocketThink func check it every frame...
			if ( vehWeapon->iLifeTime )
			{//expire after a time
				missile->genericValue1 = vehWeapon->iLifeTime;
				missile->genericValue2 = (int)(vehWeapon->bExplodeOnExpire);
			}
			//now go ahead and use the setsolidtoowner func
			missile->think = WP_VehWeapSetSolidToOwner;
			missile->nextthink = level.time + 3000;
		}
	}
	else
	{//traceline
		//FIXME: implement
	}

	return missile;
}

//custom routine to not waste tempents horribly -rww
void G_VehMuzzleFireFX( gentity_t *ent, gentity_t *broadcaster, int muzzlesFired )
{
	Vehicle_t *pVeh = ent->m_pVehicle;
	gentity_t *b;

	if (!pVeh)
	{
		return;
	}

	if (!broadcaster)
	{ //oh well. We will WASTE A TEMPENT.
		b = G_TempEntity( ent->client->ps.origin, EV_VEH_FIRE );
	}
	else
	{ //joy
		b = broadcaster;
	}

	//this guy owns it
	b->s.owner = ent->s.number;

	//this is the bitfield of all muzzles fired this time
	//NOTE: just need MAX_VEHICLE_MUZZLES bits for this... should be cool since it's currently 12 and we're sending it in 16 bits
	b->s.trickedentindex = muzzlesFired;

	if ( broadcaster )
	{ //add the event
		G_AddEvent( b, EV_VEH_FIRE, 0 );
	}
}

void G_EstimateCamPos( vec3_t viewAngles, vec3_t cameraFocusLoc, float viewheight, float thirdPersonRange, 
					  float thirdPersonHorzOffset, float vertOffset, float pitchOffset, 
					  int ignoreEntNum, vec3_t camPos )
{
	int		MASK_CAMERACLIP = (MASK_SOLID|CONTENTS_PLAYERCLIP);
	float	CAMERA_SIZE = 4;
	vec3_t	cameramins;
	vec3_t	cameramaxs;
	vec3_t	cameraFocusAngles, camerafwd, cameraup;
	vec3_t	cameraIdealTarget, cameraCurTarget;
	vec3_t	cameraIdealLoc, cameraCurLoc;
	vec3_t	diff;
	vec3_t	camAngles;
	vec3_t	viewaxis[3];
	trace_t	trace;

	VectorSet( cameramins, -CAMERA_SIZE, -CAMERA_SIZE, -CAMERA_SIZE );
	VectorSet( cameramaxs, CAMERA_SIZE, CAMERA_SIZE, CAMERA_SIZE );

	VectorCopy( viewAngles, cameraFocusAngles );
	cameraFocusAngles[PITCH] += pitchOffset;
	if ( !bg_fighterAltControl.integer )
	{//clamp view pitch
		cameraFocusAngles[PITCH] = AngleNormalize180( cameraFocusAngles[PITCH] );
		if (cameraFocusAngles[PITCH] > 80.0)
		{
			cameraFocusAngles[PITCH] = 80.0;
		}
		else if (cameraFocusAngles[PITCH] < -80.0)
		{
			cameraFocusAngles[PITCH] = -80.0;
		}
	}
	AngleVectors(cameraFocusAngles, camerafwd, NULL, cameraup);

	cameraFocusLoc[2] += viewheight;

	VectorCopy( cameraFocusLoc, cameraIdealTarget );
	cameraIdealTarget[2] += vertOffset;

	//NOTE: on cgame, this uses the thirdpersontargetdamp value, we ignore that here
	VectorCopy( cameraIdealTarget, cameraCurTarget );
	trap->Trace( &trace, cameraFocusLoc, cameramins, cameramaxs, cameraCurTarget, ignoreEntNum, MASK_CAMERACLIP , 0, 0, 0);
	if (trace.fraction < 1.0)
	{
		VectorCopy(trace.endpos, cameraCurTarget);
	}

	VectorMA(cameraIdealTarget, -(thirdPersonRange), camerafwd, cameraIdealLoc);
	//NOTE: on cgame, this uses the thirdpersoncameradamp value, we ignore that here
	VectorCopy( cameraIdealLoc, cameraCurLoc );
	trap->Trace(&trace, cameraCurTarget, cameramins, cameramaxs, cameraCurLoc, ignoreEntNum, MASK_CAMERACLIP, 0, 0, 0);
	if (trace.fraction < 1.0)
	{
		VectorCopy( trace.endpos, cameraCurLoc );
	}

	VectorSubtract(cameraCurTarget, cameraCurLoc, diff);
	{
		float dist = VectorNormalize(diff);
		//under normal circumstances, should never be 0.00000 and so on.
		if ( !dist || (diff[0] == 0 || diff[1] == 0) )
		{//must be hitting something, need some value to calc angles, so use cam forward
			VectorCopy( camerafwd, diff );
		}
	}

	vectoangles(diff, camAngles);

	if ( thirdPersonHorzOffset != 0.0f )
	{
		AnglesToAxis( camAngles, viewaxis );
		VectorMA( cameraCurLoc, thirdPersonHorzOffset, viewaxis[1], cameraCurLoc );
	}

	VectorCopy(cameraCurLoc, camPos);
}

void WP_GetVehicleCamPos( gentity_t *ent, gentity_t *pilot, vec3_t camPos )
{
	float thirdPersonHorzOffset = ent->m_pVehicle->m_pVehicleInfo->cameraHorzOffset;
	float thirdPersonRange = ent->m_pVehicle->m_pVehicleInfo->cameraRange;
	float pitchOffset = ent->m_pVehicle->m_pVehicleInfo->cameraPitchOffset;
	float vertOffset = ent->m_pVehicle->m_pVehicleInfo->cameraVertOffset;

	if ( ent->client->ps.hackingTime )
	{
		thirdPersonHorzOffset += (((float)ent->client->ps.hackingTime)/MAX_STRAFE_TIME) * -80.0f;
		thirdPersonRange += fabs(((float)ent->client->ps.hackingTime)/MAX_STRAFE_TIME) * 100.0f;
	}

	if ( ent->m_pVehicle->m_pVehicleInfo->cameraPitchDependantVertOffset )
	{
		if ( pilot->client->ps.viewangles[PITCH] > 0 )
		{
			vertOffset = 130+pilot->client->ps.viewangles[PITCH]*-10;
			if ( vertOffset < -170 )
			{
				vertOffset = -170;
			}
		}
		else if ( pilot->client->ps.viewangles[PITCH] < 0 )
		{
			vertOffset = 130+pilot->client->ps.viewangles[PITCH]*-5;
			if ( vertOffset > 130 )
			{
				vertOffset = 130;
			}
		}
		else
		{
			vertOffset = 30;
		}
		if ( pilot->client->ps.viewangles[PITCH] > 0 )
		{
			pitchOffset = pilot->client->ps.viewangles[PITCH]*-0.75;
		}
		else if ( pilot->client->ps.viewangles[PITCH] < 0 )
		{
			pitchOffset = pilot->client->ps.viewangles[PITCH]*-0.75;
		}
		else
		{
			pitchOffset = 0;
		}
	}

	//Control Scheme 3 Method:
	G_EstimateCamPos( ent->client->ps.viewangles, pilot->client->ps.origin, pilot->client->ps.viewheight, thirdPersonRange, 
		thirdPersonHorzOffset, vertOffset, pitchOffset, 
		pilot->s.number, camPos );
	/*
	//Control Scheme 2 Method:
	G_EstimateCamPos( ent->m_pVehicle->m_vOrientation, ent->r.currentOrigin, pilot->client->ps.viewheight, thirdPersonRange, 
		thirdPersonHorzOffset, vertOffset, pitchOffset, 
		pilot->s.number, camPos );
	*/
}

void WP_VehLeadCrosshairVeh( gentity_t *camTraceEnt, vec3_t newEnd, const vec3_t dir, const vec3_t shotStart, vec3_t shotDir )
{
	if ( camTraceEnt 
		&& camTraceEnt->client
		&& camTraceEnt->client->NPC_class == CLASS_VEHICLE )
	{//if the crosshair is on a vehicle, lead it
		float distAdjust = DotProduct( camTraceEnt->client->ps.velocity, dir );
		VectorMA( newEnd, distAdjust, dir, newEnd );
	}
	VectorSubtract( newEnd, shotStart, shotDir );
	VectorNormalize( shotDir );
}

#define MAX_XHAIR_DIST_ACCURACY	20000.0f
extern float g_cullDistance;
extern int BG_VehTraceFromCamPos( trace_t *camTrace, bgEntity_t *bgEnt, const vec3_t entOrg, const vec3_t shotStart, const vec3_t end, vec3_t newEnd, vec3_t shotDir, float bestDist );
qboolean WP_VehCheckTraceFromCamPos( gentity_t *ent, const vec3_t shotStart, vec3_t shotDir )
{
	//FIXME: only if dynamicCrosshair and dynamicCrosshairPrecision is on!
	if ( !ent 
		|| !ent->m_pVehicle 
		|| !ent->m_pVehicle->m_pVehicleInfo 
		|| !ent->m_pVehicle->m_pPilot//not being driven
		|| !((gentity_t*)ent->m_pVehicle->m_pPilot)->client//not being driven by a client...?!!!
		|| (ent->m_pVehicle->m_pPilot->s.number >= MAX_CLIENTS) )//being driven, but not by a real client, no need to worry about crosshair
	{
		return qfalse;
	}
	if ( (ent->m_pVehicle->m_pVehicleInfo->type == VH_FIGHTER && g_cullDistance > MAX_XHAIR_DIST_ACCURACY )
		|| ent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER)
	{
		//FIRST: simulate the normal crosshair trace from the center of the veh straight forward
		trace_t trace;
		vec3_t	dir, start, end;
		if ( ent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
		{//for some reason, the walker always draws the crosshair out from from the first muzzle point
			AngleVectors( ent->client->ps.viewangles, dir, NULL, NULL );
			VectorCopy( ent->r.currentOrigin, start );
			start[2] += ent->m_pVehicle->m_pVehicleInfo->height-DEFAULT_MINS_2-48;
		}
		else
		{
			vec3_t ang;
			if (ent->m_pVehicle->m_pVehicleInfo->type == VH_SPEEDER)
			{
				VectorSet(ang, 0.0f, ent->m_pVehicle->m_vOrientation[1], 0.0f);
			}
			else
			{
				VectorCopy(ent->m_pVehicle->m_vOrientation, ang);
			}
			AngleVectors( ang, dir, NULL, NULL );
			VectorCopy( ent->r.currentOrigin, start );
		}
		VectorMA( start, g_cullDistance, dir, end );
		trap->Trace( &trace, start, vec3_origin, vec3_origin, end, 
			ent->s.number, CONTENTS_SOLID|CONTENTS_BODY, 0, 0, 0 );

		if ( ent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER )
		{//just use the result of that one trace since walkers don't do the extra trace
			VectorSubtract( trace.endpos, shotStart, shotDir );
			VectorNormalize( shotDir );
			return qtrue;
		}
		else
		{//NOW do the trace from the camPos and compare with above trace
			trace_t	extraTrace;
			vec3_t	newEnd;
			int camTraceEntNum = BG_VehTraceFromCamPos( &extraTrace, (bgEntity_t *)ent, ent->r.currentOrigin, shotStart, end, newEnd, shotDir, (trace.fraction*g_cullDistance) );
			if ( camTraceEntNum )
			{
				WP_VehLeadCrosshairVeh( &g_entities[camTraceEntNum-1], newEnd, dir, shotStart, shotDir );
				return qtrue;
			}
		}
	}
	return qfalse;
}

//---------------------------------------------------------
void FireVehicleWeapon( gentity_t *ent, qboolean alt_fire ) 
//---------------------------------------------------------
{
	Vehicle_t *pVeh = ent->m_pVehicle;
	int muzzlesFired = 0;
	gentity_t *missile = NULL;
	vehWeaponInfo_t *vehWeapon = NULL;
	qboolean	clearRocketLockEntity = qfalse;
	
	if ( !pVeh )
	{
		return;
	}

	if (pVeh->m_iRemovedSurfaces)
	{ //can't fire when the thing is breaking apart
		return;
	}

	if (pVeh->m_pVehicleInfo->type == VH_WALKER &&
		ent->client->ps.electrifyTime > level.time)
	{ //don't fire while being electrocuted
		return;
	}

	// TODO?: If possible (probably not enough time), it would be nice if secondary fire was actually a mode switch/toggle
	// so that, for instance, an x-wing can have 4-gun fire, or individual muzzle fire. If you wanted a different weapon, you
	// would actually have to press the 2 key or something like that (I doubt I'd get a graphic for it anyways though). -AReis

	// If this is not the alternate fire, fire a normal blaster shot...
	if ( pVeh->m_pVehicleInfo &&
		(pVeh->m_pVehicleInfo->type != VH_FIGHTER || (pVeh->m_ulFlags&VEH_WINGSOPEN)) ) // NOTE: Wings open also denotes that it has already launched.
	{//fighters can only fire when wings are open
		int	weaponNum = 0, vehWeaponIndex = VEH_WEAPON_NONE;
		int	delay = 1000;
		qboolean aimCorrect = qfalse;
		qboolean linkedFiring = qfalse;

		if ( !alt_fire )
		{
			weaponNum = 0;
		}
		else
		{
			weaponNum = 1;
		}

		vehWeaponIndex = pVeh->m_pVehicleInfo->weapon[weaponNum].ID;

		if ( pVeh->weaponStatus[weaponNum].ammo <= 0 )
		{//no ammo for this weapon
			if ( pVeh->m_pPilot && pVeh->m_pPilot->s.number < MAX_CLIENTS )
			{// let the client know he's out of ammo
				int i;
				//but only if one of the vehicle muzzles is actually ready to fire this weapon
				for ( i = 0; i < MAX_VEHICLE_MUZZLES; i++ )
				{
					if ( pVeh->m_pVehicleInfo->weapMuzzle[i] != vehWeaponIndex )
					{//this muzzle doesn't match the weapon we're trying to use
						continue;
					}
					if ( pVeh->m_iMuzzleTag[i] != -1 
						&& pVeh->m_iMuzzleWait[i] < level.time )
					{//this one would have fired, send the no ammo message
						G_AddEvent( (gentity_t*)pVeh->m_pPilot, EV_NOAMMO, weaponNum );
						break;
					}
				}
			}
			return;
		}

		delay = pVeh->m_pVehicleInfo->weapon[weaponNum].delay;
		aimCorrect = pVeh->m_pVehicleInfo->weapon[weaponNum].aimCorrect;
		if ( pVeh->m_pVehicleInfo->weapon[weaponNum].linkable == 2//always linked
			|| ( pVeh->m_pVehicleInfo->weapon[weaponNum].linkable == 1//optionally linkable
				 && pVeh->weaponStatus[weaponNum].linked ) )//linked
		{//we're linking the primary or alternate weapons, so we'll do *all* the muzzles
			linkedFiring = qtrue;
		}

		if ( vehWeaponIndex <= VEH_WEAPON_BASE || vehWeaponIndex >= MAX_VEH_WEAPONS )
		{//invalid vehicle weapon
			return;
		}
		else
		{
			int i, numMuzzles = 0, numMuzzlesReady = 0, cumulativeDelay = 0, cumulativeAmmo = 0;
			qboolean sentAmmoWarning = qfalse;

			vehWeapon = &g_vehWeaponInfo[vehWeaponIndex];

			if ( pVeh->m_pVehicleInfo->weapon[weaponNum].linkable == 2 )
			{//always linked weapons don't accumulate delay, just use specified delay
				cumulativeDelay = delay;
			}
			//find out how many we've got for this weapon
			for ( i = 0; i < MAX_VEHICLE_MUZZLES; i++ )
			{
				if ( pVeh->m_pVehicleInfo->weapMuzzle[i] != vehWeaponIndex )
				{//this muzzle doesn't match the weapon we're trying to use
					continue;
				}
				if ( pVeh->m_iMuzzleTag[i] != -1 && pVeh->m_iMuzzleWait[i] < level.time )
				{
					numMuzzlesReady++;
				}
				if ( pVeh->m_pVehicleInfo->weapMuzzle[pVeh->weaponStatus[weaponNum].nextMuzzle] != vehWeaponIndex )
				{//Our designated next muzzle for this weapon isn't valid for this weapon (happens when ships fire for the first time)
					//set the next to this one
					pVeh->weaponStatus[weaponNum].nextMuzzle = i;
				}
				if ( linkedFiring )
				{
					cumulativeAmmo += vehWeapon->iAmmoPerShot;
					if ( pVeh->m_pVehicleInfo->weapon[weaponNum].linkable != 2 )
					{//always linked weapons don't accumulate delay, just use specified delay
						cumulativeDelay += delay;
					}
				}
				numMuzzles++;
			}

			if ( linkedFiring )
			{//firing all muzzles at once
				if ( numMuzzlesReady != numMuzzles )
				{//can't fire all linked muzzles yet
					return;
				}
				else 
				{//can fire all linked muzzles, check ammo
					if ( pVeh->weaponStatus[weaponNum].ammo < cumulativeAmmo )
					{//can't fire, not enough ammo
						if ( pVeh->m_pPilot && pVeh->m_pPilot->s.number < MAX_CLIENTS )
						{// let the client know he's out of ammo
							G_AddEvent( (gentity_t*)pVeh->m_pPilot, EV_NOAMMO, weaponNum );
						}
						return;
					}
				}
			}

			for ( i = 0; i < MAX_VEHICLE_MUZZLES; i++ )
			{
				if ( pVeh->m_pVehicleInfo->weapMuzzle[i] != vehWeaponIndex )
				{//this muzzle doesn't match the weapon we're trying to use
					continue;
				}
				if ( !linkedFiring
					&& i != pVeh->weaponStatus[weaponNum].nextMuzzle )
				{//we're only firing one muzzle and this isn't it
					continue;
				}

				// Fire this muzzle.
				if ( pVeh->m_iMuzzleTag[i] != -1 && pVeh->m_iMuzzleWait[i] < level.time )
				{
					vec3_t	start, dir;
					
					if ( pVeh->weaponStatus[weaponNum].ammo < vehWeapon->iAmmoPerShot )
					{//out of ammo!
						if ( !sentAmmoWarning )
						{
							sentAmmoWarning = qtrue;
							if ( pVeh->m_pPilot && pVeh->m_pPilot->s.number < MAX_CLIENTS )
							{// let the client know he's out of ammo
								G_AddEvent( (gentity_t*)pVeh->m_pPilot, EV_NOAMMO, weaponNum );
							}
						}
					}
					else
					{//have enough ammo to shoot
						//do the firing
						WP_CalcVehMuzzle(ent, i);
						VectorCopy( pVeh->m_vMuzzlePos[i], start );
						VectorCopy( pVeh->m_vMuzzleDir[i], dir );
						if ( WP_VehCheckTraceFromCamPos( ent, start, dir ) )
						{//auto-aim at whatever crosshair would be over from camera's point of view (if closer)
						}
						else if ( aimCorrect )
						{//auto-aim the missile at the crosshair if there's anything there
							trace_t trace;
							vec3_t	end;
							vec3_t	ang;
							vec3_t	fixedDir;

							if (pVeh->m_pVehicleInfo->type == VH_SPEEDER)
							{
								VectorSet(ang, 0.0f, pVeh->m_vOrientation[1], 0.0f);
							}
							else
							{
								VectorCopy(pVeh->m_vOrientation, ang);
							}
							AngleVectors( ang, fixedDir, NULL, NULL );
							VectorMA( ent->r.currentOrigin, 32768, fixedDir, end );
							//VectorMA( ent->r.currentOrigin, 8192, dir, end );
							trap->Trace( &trace, ent->r.currentOrigin, vec3_origin, vec3_origin, end, ent->s.number, MASK_SHOT , 0, 0, 0);
							if ( trace.fraction < 1.0f && !trace.allsolid && !trace.startsolid )
							{
								vec3_t newEnd;
								VectorCopy( trace.endpos, newEnd );
								WP_VehLeadCrosshairVeh( &g_entities[trace.entityNum], newEnd, fixedDir, start, dir );
							}
						}

						//play the weapon's muzzle effect if we have one
						//NOTE: just need MAX_VEHICLE_MUZZLES bits for this... should be cool since it's currently 12 and we're sending it in 16 bits
						muzzlesFired |= (1<<i);
												
						missile = WP_FireVehicleWeapon( ent, start, dir, vehWeapon, alt_fire, qfalse );
						if ( vehWeapon->fHoming )
						{//clear the rocket lock entity *after* all muzzles have fired
							clearRocketLockEntity = qtrue;
						}
					}

					if ( linkedFiring )
					{//we're linking the weapon, so continue on and fire all appropriate muzzles
						continue;
					}
					//else just firing one
					//take the ammo, set the next muzzle and set the delay on it
					if ( numMuzzles > 1 )
					{//more than one, look for it
						int nextMuzzle = pVeh->weaponStatus[weaponNum].nextMuzzle;
						while ( 1 )
						{
							nextMuzzle++;
							if ( nextMuzzle >= MAX_VEHICLE_MUZZLES )
							{
								nextMuzzle = 0;
							}
							if ( nextMuzzle == pVeh->weaponStatus[weaponNum].nextMuzzle )
							{//WTF?  Wrapped without finding another valid one!
								break;
							}
							if ( pVeh->m_pVehicleInfo->weapMuzzle[nextMuzzle] == vehWeaponIndex )
							{//this is the next muzzle for this weapon
								pVeh->weaponStatus[weaponNum].nextMuzzle = nextMuzzle;
								break;
							}
						}
					}//else, just stay on the one we just fired
					//set the delay on the next muzzle
					pVeh->m_iMuzzleWait[pVeh->weaponStatus[weaponNum].nextMuzzle] = level.time + delay;
					//take away the ammo
					pVeh->weaponStatus[weaponNum].ammo -= vehWeapon->iAmmoPerShot;
					//NOTE: in order to send the vehicle's ammo info to the client, we copy the ammo into the first 2 ammo slots on the vehicle NPC's client->ps.ammo array
					if ( pVeh->m_pParentEntity && ((gentity_t*)(pVeh->m_pParentEntity))->client )
					{
						((gentity_t*)(pVeh->m_pParentEntity))->client->ps.ammo = pVeh->weaponStatus[weaponNum].ammo;
					}
					//done!
					//we'll get in here again next frame and try the next muzzle...
					//return;
					goto tryFire;
				}
			}
			//we went through all the muzzles, so apply the cumulative delay and ammo cost
			if ( cumulativeAmmo )
			{//taking ammo one shot at a time
				//take the ammo
				pVeh->weaponStatus[weaponNum].ammo -= cumulativeAmmo;
				//NOTE: in order to send the vehicle's ammo info to the client, we copy the ammo into the first 2 ammo slots on the vehicle NPC's client->ps.ammo array
				if ( pVeh->m_pParentEntity && ((gentity_t*)(pVeh->m_pParentEntity))->client )
				{
					((gentity_t*)(pVeh->m_pParentEntity))->client->ps.ammo = pVeh->weaponStatus[weaponNum].ammo;
				}
			}
			if ( cumulativeDelay )
			{//we linked muzzles so we need to apply the cumulative delay now, to each of the linked muzzles
				for ( i = 0; i < MAX_VEHICLE_MUZZLES; i++ )
				{
					if ( pVeh->m_pVehicleInfo->weapMuzzle[i] != vehWeaponIndex )
					{//this muzzle doesn't match the weapon we're trying to use
						continue;
					}
					//apply the cumulative delay
					pVeh->m_iMuzzleWait[i] = level.time + cumulativeDelay;
				}
			}
		}
	}

tryFire:
	if ( clearRocketLockEntity )
	{//hmm, should probably clear that anytime any weapon fires?
		ent->client->ps.rocketLockIndex = ENTITYNUM_NONE;
		ent->client->ps.rocketLockTime = 0;
		ent->client->ps.rocketTargetTime = 0;
	}

	if ( vehWeapon && muzzlesFired > 0 )
	{
		G_VehMuzzleFireFX(ent, missile, muzzlesFired );
	}
}

/*
===============
FireWeapon
===============
*/
int BG_EmplacedView(vec3_t baseAngles, vec3_t angles, float *newYaw, float constraint);




//---------------------------------------------------------
static void WP_FireEmplaced( gentity_t *ent, qboolean altFire )
//---------------------------------------------------------
{
	vec3_t	dir, angs, gunpoint;
	vec3_t	right;
	gentity_t *gun;
	int side;

	if (!ent->client)
	{
		return;
	}

	if (!ent->client->ps.emplacedIndex)
	{ //shouldn't be using WP_EMPLACED_GUN if we aren't on an emplaced weapon
		return;
	}

	gun = &g_entities[ent->client->ps.emplacedIndex];

	if (!gun->inuse || gun->health <= 0)
	{ //gun was removed or killed, although we should never hit this check because we should have been forced off it already
		return;
	}

	VectorCopy(gun->s.origin, gunpoint);
	gunpoint[2] += 46;

	AngleVectors(ent->client->ps.viewangles, NULL, right, NULL);

	if (gun->genericValue10)
	{ //fire out of the right cannon side
		VectorMA(gunpoint, 10.0f, right, gunpoint);
		side = 0;
	}
	else
	{ //the left
		VectorMA(gunpoint, -10.0f, right, gunpoint);
		side = 1;
	}

	gun->genericValue10 = side;
	G_AddEvent(gun, EV_FIRE_WEAPON, side);

	vectoangles( forward, angs );

	AngleVectors( angs, dir, NULL, NULL );

	WP_FireEmplacedMissile( gun, gunpoint, dir, altFire, ent );
}

#define EMPLACED_CANRESPAWN 1

//----------------------------------------------------------

/*QUAKED emplaced_gun (0 0 1) (-30 -20 8) (30 20 60) CANRESPAWN

 count - if CANRESPAWN spawnflag, decides how long it is before gun respawns (in ms)
 constraint - number of degrees gun is constrained from base angles on each side (default 60.0)

 showhealth - set to 1 to show health bar on this entity when crosshair is over it
  
  teamowner - crosshair shows green for this team, red for opposite team
	0 - none
	1 - red
	2 - blue

  alliedTeam - team that can use this
	0 - any
	1 - red
	2 - blue

  teamnodmg - team that turret does not take damage from or do damage to
	0 - none
	1 - red
	2 - blue
*/
 
//----------------------------------------------------------
void emplaced_gun_use( gentity_t *self, gentity_t *other, trace_t *trace )
{
	vec3_t fwd1, fwd2;
	float dot;
	int oldWeapon;
	gentity_t *activator = other;
	float zoffset = 50;
	vec3_t anglesToOwner;
	vec3_t vLen;
	float ownLen;

	if ( self->health <= 0 )
	{ //gun is destroyed
		return;
	}

	if (self->activator)
	{ //someone is already using me
		return;
	}

	if (!activator->client)
	{
		return;
	}

	if (activator->client->ps.emplacedTime > level.time)
	{ //last use attempt still too recent
		return;
	}

	if (activator->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //don't use if busy doing something else
		return;
	}

	if (activator->client->ps.origin[2] > self->s.origin[2]+zoffset-8)
	{ //can't use it from the top
		return;
	}

	if (activator->client->ps.pm_flags & PMF_DUCKED)
	{ //must be standing
		return;
	}

	VectorSubtract(self->s.origin, activator->client->ps.origin, vLen);
	ownLen = VectorLength(vLen);

	if (ownLen > 64.0f)
	{ //must be within 64 units of the gun to use at all
		return;
	}

	// Let's get some direction vectors for the user
	AngleVectors( activator->client->ps.viewangles, fwd1, NULL, NULL );

	// Get the guns direction vector
	AngleVectors( self->pos1, fwd2, NULL, NULL );

	dot = DotProduct( fwd1, fwd2 );

	VectorSubtract(self->s.origin, activator->client->ps.origin, fwd1);
	VectorNormalize(fwd1);

	dot = DotProduct( fwd1, fwd2 );

	self->genericValue1 = 1;

	oldWeapon = activator->s.weapon;

	// swap the users weapon with the emplaced gun
	activator->client->ps.weapon = self->s.weapon;
	activator->client->ps.weaponstate = WEAPON_READY;
	activator->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_EMPLACED_GUN );

	activator->client->ps.emplacedIndex = self->s.number;

	self->s.emplacedOwner = activator->s.number;
	self->s.activeForcePass = NUM_FORCE_POWERS+1;

	// the gun will track which weapon we used to have
	self->s.weapon = oldWeapon;

	//user's new owner becomes the gun ent
	activator->r.ownerNum = self->s.number;
	self->activator = activator;

	VectorSubtract(self->r.currentOrigin, activator->client->ps.origin, anglesToOwner);
	vectoangles(anglesToOwner, anglesToOwner);
	return;
}

void emplaced_gun_realuse( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	emplaced_gun_use(self, other, NULL);
}

//----------------------------------------------------------
void emplaced_gun_pain( gentity_t *self, gentity_t *attacker, int damage )
{
	self->s.health = self->health;

	if ( self->health <= 0 )
	{
		//death effect.. for now taken care of on cgame
	}
	else
	{
		//if we have a pain behavior set then use it I guess
		G_ActivateBehavior( self, BSET_PAIN );
	}
}

#define EMPLACED_GUN_HEALTH 800

//----------------------------------------------------------
void emplaced_gun_update(gentity_t *self)
{
	vec3_t	smokeOrg, puffAngle;
	int oldWeap;
	float ownLen = 0;

	if (self->health < 1 && !self->genericValue5)
	{ //we are dead, set our respawn delay if we have one
		if (self->spawnflags & EMPLACED_CANRESPAWN)
		{
			self->genericValue5 = level.time + 4000 + self->count;
		}
	}
	else if (self->health < 1 && self->genericValue5 < level.time)
	{ //we are dead, see if it's time to respawn
		self->s.time = 0;
		self->genericValue4 = 0;
		self->genericValue3 = 0;
		self->health = EMPLACED_GUN_HEALTH*0.4;
		self->s.health = self->health;
	}

	if (self->genericValue4 && self->genericValue4 < 2 && self->s.time < level.time)
	{ //we have finished our warning (red flashing) effect, it's time to finish dying
		vec3_t explOrg;

		VectorSet( puffAngle, 0, 0, 1 );

		VectorCopy(self->r.currentOrigin, explOrg);
		explOrg[2] += 16;

		//just use the detpack explosion effect
		G_PlayEffect(EFFECT_EXPLOSION_DETPACK, explOrg, puffAngle);

		self->genericValue3 = level.time + Q_irand(2500, 3500);

		G_RadiusDamage(self->r.currentOrigin, self, self->splashDamage, self->splashRadius, self, NULL, MOD_UNKNOWN);

		self->s.time = -1;

		self->genericValue4 = 2;
	}

	if (self->genericValue3 > level.time)
	{ //see if we are freshly dead and should be smoking
		if (self->genericValue2 < level.time)
		{ //is it time yet to spawn another smoke puff?
			VectorSet( puffAngle, 0, 0, 1 );
			VectorCopy(self->r.currentOrigin, smokeOrg);

			smokeOrg[2] += 60;

			G_PlayEffect(EFFECT_SMOKE, smokeOrg, puffAngle);
			self->genericValue2 = level.time + Q_irand(250, 400);
		}
	}

	if (self->activator && self->activator->client && self->activator->inuse)
	{ //handle updating current user
		vec3_t vLen;
		VectorSubtract(self->s.origin, self->activator->client->ps.origin, vLen);
		ownLen = VectorLength(vLen);

		if (!(self->activator->client->pers.cmd.buttons & BUTTON_USE) && self->genericValue1)
		{
			self->genericValue1 = 0;
		}

		if (!(self->r.svFlags & SVF_BOT) && (self->activator->client->pers.cmd.buttons & BUTTON_USE) && !self->genericValue1)
		{
			self->activator->client->ps.emplacedIndex = 0;
			self->activator->client->ps.saberHolstered = 0;
			self->nextthink = level.time + 50;
			return;
		}
	}

	if (!(self->r.svFlags & SVF_BOT) && (self->activator && self->activator->client) &&
		(!self->activator->inuse || self->activator->client->ps.emplacedIndex != self->s.number || self->genericValue4 || ownLen > 64))
	{ //get the user off of me then
		self->activator->client->ps.stats[STAT_WEAPONS] &= ~(1<<WP_EMPLACED_GUN);

		oldWeap = self->activator->client->ps.weapon;
		self->activator->client->ps.weapon = self->s.weapon;
		self->s.weapon = oldWeap;
		self->activator->r.ownerNum = ENTITYNUM_NONE;
		self->activator->client->ps.emplacedTime = level.time + 1000;
		self->activator->client->ps.emplacedIndex = 0;
		self->activator->client->ps.saberHolstered = 0;
		self->activator = NULL;

		self->s.activeForcePass = 0;
	}
	else if (self->activator && self->activator->client)
	{ //make sure the user is using the emplaced gun weapon
		self->activator->client->ps.weapon = WP_EMPLACED_GUN;
		self->activator->client->ps.weaponstate = WEAPON_READY;
	}
	self->nextthink = level.time + 50;
}

//----------------------------------------------------------
void emplaced_gun_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{ //set us up to flash and then explode
	if (self->genericValue4)
	{
		return;
	}

	self->genericValue4 = 1;

	self->s.time = level.time + 3000;

	self->genericValue5 = 0;
}

void SP_emplaced_gun( gentity_t *ent )
{
	const char *name = "models/map_objects/mp/turret_chair.glm";
	vec3_t down;
	trace_t tr;

	//make sure our assets are precached
	RegisterItem( BG_FindItemForWeapon(WP_EMPLACED_GUN) );

	ent->r.contents = CONTENTS_SOLID;
	ent->s.solid = SOLID_BBOX;

	ent->genericValue5 = 0;

	VectorSet( ent->r.mins, -30, -20, 8 );
	VectorSet( ent->r.maxs, 30, 20, 60 );

	VectorCopy(ent->s.origin, down);

	down[2] -= 1024;

	trap->Trace(&tr, ent->s.origin, ent->r.mins, ent->r.maxs, down, ent->s.number, MASK_SOLID, 0, 0, 0);

	if (tr.fraction != 1 && !tr.allsolid && !tr.startsolid)
	{
		VectorCopy(tr.endpos, ent->s.origin);
	}

	ent->spawnflags |= 4; // deadsolid

	ent->health = EMPLACED_GUN_HEALTH;

	if (ent->spawnflags & EMPLACED_CANRESPAWN)
	{ //make it somewhat easier to kill if it can respawn
		ent->health *= 0.4;
	}

	ent->maxHealth = ent->health;
	G_ScaleNetHealth(ent);

	ent->genericValue4 = 0;

	ent->takedamage = qtrue;
	ent->pain = emplaced_gun_pain;
	ent->die = emplaced_gun_die;

	// being caught in this thing when it blows would be really bad.
	ent->splashDamage = 80;
	ent->splashRadius = 128;

	// amount of ammo that this little poochie has
	G_SpawnInt( "count", "600", &ent->count );

	G_SpawnFloat( "constraint", "60", &ent->s.origin2[0] );

	ent->s.modelindex = G_ModelIndex( (char *)name );
	ent->s.modelGhoul2 = 1;
	ent->s.g2radius = 110;

	//so the cgame knows for sure that we're an emplaced weapon
	ent->s.weapon = WP_EMPLACED_GUN;

	G_SetOrigin( ent, ent->s.origin );
	
	// store base angles for later
	VectorCopy( ent->s.angles, ent->pos1 );
	VectorCopy( ent->s.angles, ent->r.currentAngles );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->think = emplaced_gun_update;
	ent->nextthink = level.time + 50;

	ent->use = emplaced_gun_realuse;

	ent->r.svFlags |= SVF_PLAYER_USABLE;

	ent->s.pos.trType = TR_STATIONARY;

	ent->s.owner = MAX_CLIENTS+1;
	ent->s.shouldtarget = qtrue;
	//ent->s.teamowner = 0;

	trap->LinkEntity((sharedEntity_t *)ent);
}


//// LITTLE STUB
void FireWeapon( gentity_t *ent, int firingMode )
{
	WP_FireGenericWeapon( ent, firingMode );
}
//// END LITTLE STUB

/**************************************************
* WP_CalculateAngles
*
* Calculates the angles for the current weapon.
* This also handles emplaced weaponary and vehicle
* weaponary. The angles are calculated before the
* muzzle. It must be done in this order!
**************************************************/

void WP_CalculateAngles( gentity_t *ent )
{
	if ( ent->s.weapon == WP_EMPLACED_GUN && ent->client->ps.emplacedIndex)
	{
		gentity_t *emp = &g_entities[ent->client->ps.emplacedIndex];

		if ( emp->inuse )
		{
			float	fYaw;
			vec3_t	viewAngCap;

			VectorCopy( ent->client->ps.viewangles, viewAngCap );
			viewAngCap[PITCH] = ( viewAngCap[PITCH] > 40 ) ? 40 : viewAngCap[PITCH];

			if ( BG_EmplacedView( ent->client->ps.viewangles, emp->s.angles, &fYaw, emp->s.origin2[0] ))
			{
				viewAngCap[YAW] = fYaw;
			}

			AngleVectors( viewAngCap, forward, vright, up );
		}
		else
		{
			AngleVectors( ent->client->ps.viewangles, forward, vright, up );
		}
	}
	else if ( ent->s.number < MAX_CLIENTS && ent->client->ps.m_iVehicleNum && ent->s.weapon == WP_BLASTER )
	{
		vec3_t		 vehTurnAngles;
		gentity_t	*vehEnt = &g_entities[ent->client->ps.m_iVehicleNum];

		if ( vehEnt->inuse && vehEnt->client && vehEnt->m_pVehicle )
		{
			VectorCopy(vehEnt->m_pVehicle->m_vOrientation, vehTurnAngles);
			vehTurnAngles[PITCH] = ent->client->ps.viewangles[PITCH];
		}
		else
		{
			VectorCopy(ent->client->ps.viewangles, vehTurnAngles);
		}

		if ( ent->client->pers.cmd.rightmove > 0 )
		{ 
			vehTurnAngles[YAW] -= 90.0f;
		}
		else if ( ent->client->pers.cmd.rightmove < 0 )
		{
			vehTurnAngles[YAW] += 90.0f;
		}

		AngleVectors( vehTurnAngles, forward, vright, up );
	}
	else
	{
		AngleVectors( ent->client->ps.viewangles, forward, vright, up );
	}
}

/**************************************************
* WP_CalculateMuzzlePoint
*
* Calculates the muzzle point for the currently
* equiped weapon. Uses bolt information to calculate
* the proper muzzle.
**************************************************/

void WP_CalculateMuzzlePoint( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) 
{
	int			 iBolt;
	void		*pGhoul;
	mdxaBone_t   muzzleMatrix;

	/* HACK: stop vendors from firing at players */
	pGhoul		= ent->client->weaponGhoul2[0];
	if(! pGhoul )
		return;
	
	/* Temporarily attach a weapon model to the player, grab the position of the bolt at that moment, and then when we're done,
	 * pop the attached model off of the player. */
	pGhoul		= BG_GetWeaponGhoul2(ent->s.weapon, ent->s.weaponVariation);
	if( !pGhoul )
		return;
	trap->G2API_CopySpecificGhoul2Model(pGhoul, 0, ent->ghoul2, 1);
	trap->G2API_SetBoltInfo(pGhoul, 1, 0);
	iBolt		= trap->G2API_AddBolt( pGhoul, 0, "*flash" );	/* Use the *flash tag as muzzle point, it's rather accurate */

	/* These weapons dont have a proper muzzle bolt, don't crash the client but fake the muzzle */
	if ( pGhoul == NULL || iBolt == -1 )
	{
		VectorCopy( ent->r.currentOrigin, muzzlePoint );
		muzzlePoint[2] += ent->client->ps.viewheight;
	}
	else
	{
		/* Get the muzzle point by getting the bolt origin. Add the view height to compensate for the player height */
		vec3_t viewAngles, viewOrg;
		VectorClear( viewAngles );

		viewAngles[YAW] = ent->client->ps.viewangles[YAW];

		trap->G2API_GetBoltMatrix( pGhoul, 0, iBolt, &muzzleMatrix, ent->client->ps.viewangles, ent->r.currentOrigin, level.time, NULL, ent->modelScale );
		muzzlePoint[0] = muzzleMatrix.matrix[0][3];
		muzzlePoint[1] = muzzleMatrix.matrix[1][3];
		muzzlePoint[2] = muzzleMatrix.matrix[2][3] + ent->client->ps.viewheight;

		// Client could be rendering in first person, so put it just a little bit away from the camera so it doesn't clip into the player's view.

		AngleVectors( viewAngles, viewOrg, NULL, NULL );
		VectorScale( viewOrg, 5, viewOrg );

		VectorAdd( muzzlePoint, viewOrg, muzzlePoint );

		/* Snap the vector to save some bandwidth */
		SnapVector( muzzlePoint );
	}
}

/**************************************************
* WP_FireGenericMissile
*
* Fires a generic grenade for the currently equiped
* weapon. It utilizes the weapon table to get all
* the required information. Supports any missile
* type with any box size, bounce count, charge,
* gravity, range, speed, splash and more. Then it
* calculates specific grenade values to make it work.
**************************************************/

gentity_t *WP_FireGenericGrenade( gentity_t *ent, int firemode, vec3_t origin, vec3_t dir )
{
	gentity_t *bolt				 = WP_FireGenericMissile( ent, firemode, origin, dir );
	float fCharge				 = 0;
	float fSpeed				 = WP_GetWeaponSpeed( ent, firemode );		
	int	  iTime					 = GetWeaponData( ent->s.weapon, ent->s.weaponVariation )->weaponReloadTime;

	/* Rolling the grenade instead of throwing it, lower the speed considerably */
	if ( firemode ) fSpeed *= 0.5;

	/* Calculate the charge for this grenade */
	fCharge = (( ent->client ) ? ( level.time - ent->client->ps.weaponChargeTime ) : 1.0f ) / fSpeed;
	fCharge = ( fCharge > 1.0f ) ? 1.0f : (( fCharge < 0.15f ) ? 0.15f : fCharge );

	/* Make sure our grenade is not on the other side of a wall */
	W_TraceSetStart(ent, origin, bolt->r.mins, bolt->r.maxs);

	/* Set the physics object and make the grenade think (when to explode) */
	bolt->flags                  |= FL_BOUNCE_HALF | FL_BOUNCE;
	bolt->physicsObject			 = qtrue;
	bolt->think					 = thermalThinkStandard;
	bolt->nextthink				 = level.time;
	if(WP_GetGrenadeBounce( ent, firemode ))
	{
		bolt->parent				= ent;
		bolt->genericValue9			= WP_GetGrenadeBounceDamage ( ent, firemode );
		bolt->genericValue10		= qtrue;
		//bolt->touch					= touch_GrenadeWithBouce;
		bolt->touch					 = touch_NULL;
	}
	else
	{
		bolt->touch					 = touch_NULL;
	}
	bolt->genericValue5			 = ( ent->grenadeCookTime ) ? ent->grenadeCookTime : level.time + iTime;

	/* Scale the direction with the speed as the delta (movement starts) */
	VectorScale( dir, fSpeed * fCharge, bolt->s.pos.trDelta );
	if ( !firemode ) bolt->s.pos.trDelta[2] += 140.0f;

	/* Set the sounds (static for now) for this weapon */
	bolt->s.loopSound			 = G_SoundIndex( "sound/weapons/thermal/thermloop.wav" );
	bolt->s.loopIsSoundset		 = qfalse;
	
	/* Copy the base origin, snap the vector, and copy the current origin */
	VectorCopy( muzzle, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );
	VectorCopy( muzzle, bolt->r.currentOrigin );
	VectorCopy( muzzle, bolt->pos2 );
		
	// Make the player 'reload' the weapon
	//ent->client->ps.weaponstate = WEAPON_DROPPING;
	G_SetAnim (ent, NULL, SETANIM_TORSO, TORSO_DROPWEAP1, SETANIM_FLAG_OVERRIDE);

	return bolt;
}

/**************************************************
* WP_FireGenericMissile
*
* Fires a generic missile for the currently equiped
* weapon. It utilizes the weapon table to get all
* the required information. Supports any missile
* type with any box size, bounce count, charge,
* gravity, range, speed, splash and more.
**************************************************/

gentity_t *WP_FireGenericMissile( gentity_t *ent, int firemode, vec3_t origin, vec3_t dir )
{
	float		 fBoxSize			 = WP_GetWeaponBoxSize( ent, firemode );
	int			 iBounce			 = WP_GetWeaponBounce( ent, firemode );
	char		*zClassname			 = WP_GetWeaponClassname( ent, firemode );
	int			 iCharge			 = WP_GetWeaponCharge( ent, firemode );
	int			 iDamage			 = WP_GetWeaponDamage( ent, firemode );
	qboolean	 bGravity			 = WP_GetWeaponGravity( ent, firemode );
	int			 iMOD				 = WP_GetWeaponMOD( ent, firemode );
	int			 iSplashMOD			 = WP_GetWeaponSplashMOD( ent, firemode );
	float		 fRange				 = WP_GetWeaponRange( ent, firemode );
	float		 fSpeed				 = WP_GetWeaponSpeed( ent, firemode );
	float		 fSplashRange		 = WP_GetWeaponSplashRange( ent, firemode );
	gentity_t	*missile			 = NULL;

	/* Create the missile, fill in the name weapon, owner, methodOfDeath and such */
	missile							 = CreateMissile( origin, dir, fSpeed, 10000, ent, firemode /* FIXME: This will always be primary fire when firemodes is done */ );
	missile->classname				 = zClassname;
	missile->s.weapon				 = ent->s.weapon;
	missile->s.weaponVariation		 = ent->s.weaponVariation;
	missile->r.ownerNum				 = ent->s.number;
	missile->s.time					 = level.time; /* For client-side prediction */
	missile->parent					 = ent;
	missile->methodOfDeath			 = iMOD;

	/* Set the appropriate range for the bullet */
	if ( fRange >= 0.0f )
	{
		//missile->s.eventParm		 = fRange;
		// Xycaleth: casual hi-jacking of some random variable. It's so random you'd have
		// thought it wasn't out of place at all in Raven's code! :D
		missile->s.apos.trBase[0]   = fRange;
	}

	/* If this missile uses gravity, and if such give a slighty boost and set the movement type */
	if ( bGravity )
	{
		missile->s.pos.trType		 = TR_GRAVITY;
		missile->s.pos.trDelta[2]	+= 40.0f;
		missile->physicsObject		 = qtrue;
	}

	missile->s.firingMode = firemode;

	/* If charging, set the charge in the generic1 field so we render accordingly */
	if ( iCharge )
	{
		// Disabled for now. Its silly and causes shit for the grenades (otherwise overrule boxsize again)
		missile->s.generic1			 = iCharge;
//		fBoxSize					*= ( iCharge / 2 );
	}
		
	/* If this missile has a splash range, set the method of death, the damage and the radius */
	if ( fSplashRange )
	{
		missile->splashDamage		 = iDamage;
		missile->splashRadius		 = fSplashRange;
		missile->splashMethodOfDeath = iSplashMOD;
	}

	/* Set the ordinary hit damage, the damage flags and clip mask */
	missile->damage					 = iDamage;
	missile->dflags					 = DAMAGE_DEATH_KNOCKBACK;
	missile->clipmask				 = MASK_SHOT | CONTENTS_LIGHTSABER;

	/* If this missile can bounce, set the flag accordingly */
	if ( iBounce )
	{
		missile->bounceCount		 = iBounce;
		missile->flags				|= FL_BOUNCE;
		missile->flags				|= FL_BOUNCE_HALF;
	}

	/* Set the angles in the enity state and set the box size */
	vectoangles( dir, missile->s.angles );
	VectorSet( missile->r.maxs,  fBoxSize,  fBoxSize,  fBoxSize );
	VectorSet( missile->r.mins, -fBoxSize, -fBoxSize, -fBoxSize );
	return missile;
}

/**************************************************
* WP_FireGenericTraceLine
*
* Fires the generic trace line function, utilizes
* the weapon table for the weapon statistics. This
* function does an instant attack and renders using
* the DISRUPTOR events.
**************************************************/

void WP_FireGenericTraceLine( gentity_t *ent, int firemode )
{
	int			 iDamage		 = WP_GetWeaponDamage( ent, firemode );
	float		 fRange			 = WP_GetWeaponRange( ent, firemode );
	int			 iSkip			 = ent->s.number;

	gentity_t	*traceEnt, *tent;
	trace_t		 tr;
	vec3_t		 start, end;

	/* Client will use the weapon muzzle */
	if ( ent->client )
	{
		VectorCopy( ent->client->ps.origin, start );
		start[2] += ent->client->ps.viewheight;
	}
	/* NPC will use their own origin and use the eye coördinates */
	else
	{
		VectorCopy( ent->r.currentOrigin, start );
		start[2] += 24;
	}

	while( iDamage > 0 )
	{
		/* Set the range forward to the end */
		VectorMA( start, fRange, forward, end );

		/* Start the trace until we hit something */
		trap->Trace( &tr, start, NULL, NULL, end, iSkip, MASK_SHOT, 0, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, 3 );

		/* We have found an entity, check what it is */
		traceEnt = &g_entities[tr.entityNum];

		/* Ghoul tracing, used for dismemberment to indicate that blows off */
		if ( traceEnt->inuse && traceEnt->client )
		{
			/* Since we used G2TRFLAG_GETSURFINDEX, tr.surfaceFlags will actually contain the index of the surface on the ghoul2 model we collided with */
			if ( traceEnt->inuse && traceEnt->client && traceEnt->ghoul2 )
			{ 
				traceEnt->client->g2LastSurfaceHit	= tr.surfaceFlags;
				traceEnt->client->g2LastSurfaceTime	= level.time;
			}

			/* Clear the surface flags after, since we actually care about them in here */
			if ( traceEnt->ghoul2 )
			{
				tr.surfaceFlags = 0;
			}
		}

		/* We have hit a dueler or a Jedi who is able to dodge the shot completely (which is a bit sad really) */
		if (( traceEnt && traceEnt->client && traceEnt->client->ps.duelInProgress && traceEnt->client->ps.duelIndex != ent->s.number )
			|| NPC_Humanoid_DodgeEvasion(traceEnt, ent, &tr, G_GetHitLocation( traceEnt, tr.endpos )))
		{
			/* Calculate the vector difference to decrease the maximum range */
			vec3_t difference;
			VectorSubtract( tr.endpos, start, difference );

			/* Substract the range, set the skip entity and copy the end position as the start */
			fRange	= fRange - VectorNormalize( difference );
			iSkip	= tr.entityNum;
			VectorCopy( tr.endpos, start );
			continue;
		}
			
		/* Always render a shot beam from the muzzle to where we hit, the beam will continue if the shot continues */
		tent = G_TempEntity( tr.endpos, EV_WEAPON_TRACELINE );
		VectorCopy( muzzle, tent->s.origin2 );
		tent->s.eventParm = DirToByte (tr.plane.normal);
		tent->s.otherEntityNum = ENTITYNUM_NONE;
		tent->s.shouldtarget = qtrue;
		tent->s.owner = ent->s.number;
		tent->s.weapon = ent->client->ps.weapon;
		tent->s.weaponVariation = ent->client->ps.weaponVariation;
		tent->s.firingMode = firemode;
		if ( firemode )
		{
			tent->s.eFlags |= EF_ALT_FIRING;
		}

		/* The traced entity is a client with saber defense 3 so we must check if we can manage to block this attack */
		if ( traceEnt && traceEnt->client && traceEnt->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] >= FORCE_LEVEL_3 )
		{
			if ( WP_SaberCanBlock( traceEnt, tr.endpos, 0, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR, qtrue, 0 ))
			{
				gentity_t *te = NULL;

				/* Create a saber block event as the Jedi just blocked */
				te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
				VectorCopy( tr.endpos, te->s.origin );
				VectorCopy( tr.plane.normal, te->s.angles );

				/* If all the angles are 0, set one to 1 */
				if ( !te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2] )
				{
					te->s.angles[1] = 1;
				}

				/* Clear the event parameters */
				te->s.eventParm	= 0;
				te->s.weapon	= 0;
				te->s.legsAnim	= 0;
				break;
			}
		}

		/* We hit something where we can't draw an impact, let's bail */
		if ( tr.surfaceFlags & SURF_NOIMPACT )
		{
			break;
		}
			
		tent->s.otherEntityNum = tr.entityNum;

		/* This entity is capable of taking damage and is a client! Uses a different event */
		if ( traceEnt->takedamage && traceEnt->client )
		{
			//tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
			//tent->s.otherEntityNum = traceEnt->s.number;
			//tent->s.eventParm = DirToByte( tr.plane.normal );
			//tent->s.eFlags |= EF_ALT_FIRING;
		} 
		else 
		{
			/* This isn't a client but can take damage, is a mover or a glass brush. Smash it up! */
			if ( traceEnt->r.svFlags & SVF_GLASS_BRUSH || traceEnt->takedamage || traceEnt->s.eType == ET_MOVER )
			{
				if ( traceEnt->takedamage )
				{
					const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (ent);
					if ( fireMode->damageTypeHandle )
					{
					    JKG_DoDirectDamage (fireMode->damageTypeHandle, traceEnt, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR);
					}
					else
					{
					    G_Damage( traceEnt, ent, ent, forward, tr.endpos, iDamage, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR );
					}
					    
					if ( fireMode->secondaryDmgHandle )
					{
					    JKG_DoDirectDamage (fireMode->secondaryDmgHandle, traceEnt, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR);
					}
					//G_Damage( traceEnt, ent, ent, forward, tr.endpos, iDamage, DAMAGE_NO_KNOCKBACK, ( altFire ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR );
					//tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
					//tent->s.eventParm = DirToByte( tr.plane.normal );
				}
			}
			/* This always shows the sniper miss event, it's used for both primary and secondary fire. */
			else
			{
				//tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_SNIPER_MISS );
				//tent->s.eventParm = DirToByte( tr.plane.normal );
				break;
			}
		}

		/* Stop already, this thing is shielded for some reason */
		if ( traceEnt->flags & FL_SHIELDED )
		{
			break;
		}

		/* Okay, this client can take damage so prepare to disintegrate him (if we have enough damage) */
		if ( traceEnt->takedamage )
		{
			vec3_t	preAng;
			int		preHealth	= traceEnt->health;
			int		preLegs		= 0;
			int		preTorso	= 0;
			const weaponFireModeStats_t *fireMode = GetEntsCurrentFireMode (ent);

			/* Remember the legs/torse stance and client angles */
			if ( traceEnt->client )
			{
				preLegs		= traceEnt->client->ps.legsAnim;
				preTorso	= traceEnt->client->ps.torsoAnim;
				VectorCopy( traceEnt->client->ps.viewangles, preAng );
			}

			/* Throw the damage at the client, we'll be able to see if we're disintegrating him after this! */
			if ( fireMode->damageTypeHandle )
			{
			    JKG_DoDirectDamage (fireMode->damageTypeHandle, traceEnt, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR);
			}
			else
			{
			    G_Damage( traceEnt, ent, ent, forward, tr.endpos, iDamage, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR );
			}
			    
			if ( fireMode->secondaryDmgHandle )
			{
			    JKG_DoDirectDamage (fireMode->secondaryDmgHandle, traceEnt, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, ( firemode ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR);
			}
			//G_Damage( traceEnt, ent, ent, forward, tr.endpos, iDamage, DAMAGE_NO_KNOCKBACK, ( altFire ) ? MOD_DISRUPTOR_SNIPER : MOD_DISRUPTOR );

			/* This client had health, is now dead and we can disruptify this client! */
			/*if ( traceEnt->client && preHealth > 0 && traceEnt->health <= 0 && ( !ent || !ent->inuse || !ent->client || ent->s.eType != ET_NPC || ent->s.NPC_class != CLASS_VEHICLE || !ent->m_pVehicle || ent->m_pVehicle->m_pVehicleInfo->type == VH_ANIMAL ))
			{
				// JKG - Disintegration suppressor
				if (!traceEnt->client->noDisintegrate) {
					VectorCopy( preAng, traceEnt->client->ps.viewangles );
					VectorCopy( tr.endpos, traceEnt->client->ps.lastHitLoc );
					VectorClear( traceEnt->client->ps.velocity );

					traceEnt->client->ps.eFlags		|= EF_DISINTEGRATION;
					traceEnt->client->ps.legsAnim	 = preLegs;
					traceEnt->client->ps.torsoAnim	 = preTorso;
					traceEnt->r.contents			 = 0;
				}
			}*/

			/* Remove the amount of damage hit on this client from our trace, this way even normal disruptor shots can continue! */
			iDamage = iDamage - preHealth;

			/* Create the event to show that this was a successful hit! */
			if ( ent->s.weapon == WP_DISRUPTOR )
			{
				//tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
				//tent->s.eventParm = DirToByte( tr.plane.normal );
				//if ( traceEnt->client ) tent->s.weapon = 1;
			}
		}
		/* We hit an entity that can't take damage.. therefore, show the mark and stop tracing */
		else
		{
			/*tent = G_TempEntity( tr.endpos, EV_DISRUPTOR_HIT );
			tent->s.eventParm = DirToByte( tr.plane.normal );*/
			//tent->s.otherEntityNum = tr.entityNum;
			return;
		}

		/* Copy the end position and prepare for another trace! */
		VectorCopy( tr.endpos, start );
		iSkip = tr.entityNum;
	}
}

/**************************************************
* WP_GetWeaponAttackDisruption
*
* Gets the disruption bool for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

qboolean WP_GetWeaponIsHitscan( gentity_t* ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

    return thisWeaponData->firemodes[firemode].hitscan;
}
	
qboolean WP_IsWeaponGrenade ( const gentity_t *ent, int firemode )
{
	const weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	return thisWeaponData->firemodes[firemode].isGrenade;
}

/**************************************************
* WP_GetWeaponBoxSize
*
* Gets the box size for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

float WP_GetWeaponBoxSize( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if ( thisWeaponData->firemodes[firemode].boxSize )
	{
		return thisWeaponData->firemodes[firemode].boxSize;
	}

	return 1.0f;
}

/**************************************************
* WP_GetWeaponBounce
*
* Gets the bounce count for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

int WP_GetWeaponBounce( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if ( thisWeaponData->firemodes[firemode].bounceCount )
	{
		return thisWeaponData->firemodes[firemode].bounceCount;
	}

	return 0;
}

/**************************************************
* WP_GetWeaponCharge
*
* Gets the charge level for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information. This will
* calculate the charge tier level. This gets called
* from the WP_GetWeaponDamage function to formulate
* the charge into the weapon damage.
**************************************************/

int WP_GetWeaponCharge( gentity_t *ent, int firemode )
{
	weaponData_t	*thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if ( thisWeaponData->firemodes[firemode].chargeTime )
	{
		int current = ( level.time - ent->client->ps.weaponChargeTime ) / thisWeaponData->firemodes[firemode].chargeTime;
		float maximum = thisWeaponData->firemodes[firemode].chargeMaximum / thisWeaponData->firemodes[firemode].chargeTime;

		if ( current > maximum ) current = maximum;
		return current;
	}

	return 0;
}

/**************************************************
* WP_GetWeaponClassname
*
* Gets the weapon projectile class for the currently 
* selected weapon with the appropriate mode. This 
* references the weapon table for this information.
**************************************************/

char *WP_GetWeaponClassname( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	return thisWeaponData->firemodes[firemode].weaponClass;
}

/**************************************************
* WP_GetWeaponDamage
*
* Gets the weapon damage for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

int WP_GetWeaponDamage( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	int damage = 0;

	if ( thisWeaponData->firemodes[firemode].chargeTime )
	{
		int chargeDamage = thisWeaponData->firemodes[firemode].baseDamage * WP_GetWeaponCharge( ent, firemode ) * thisWeaponData->firemodes[firemode].chargeMultiplier;
		damage = ( thisWeaponData->firemodes[firemode].baseDamage > chargeDamage ) ? thisWeaponData->firemodes[firemode].baseDamage : chargeDamage;
	}
	else
	{
		damage = thisWeaponData->firemodes[firemode].baseDamage;
	}
		
	if ( thisWeaponData->firemodes[firemode].ammo )
	{
		damage *= thisWeaponData->firemodes[firemode].ammo->damageModifier;
	}
		
	return damage;
}

/**************************************************
* WP_CalculateSpread
*
* Calculate the spread of the weapon in pitch/yaw,
* given the gun's accuracy rating and pitch/yaw
* supplied as angles
**************************************************/

void WP_CalculateSpread( float *pitch, float *yaw, float accuracyRating, float modifier )
{
	float radius = (accuracyRating/2)*modifier;
	float pitchValue = (Q_irand(0,1) == 0) ? flrand(0,radius) : flrand(0,radius)*-1;
	float yawValue = (Q_irand(0,1) == 0) ? flrand(0,radius) : flrand(0,radius)*-1;

	*pitch = atan(pitchValue / 100)*5;
	*yaw = atan(yawValue / 100)*5;
}

/**************************************************
* WP_GetWeaponDirection
*
* Gets the splash damage for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/
static void WP_GetWeaponDirection( gentity_t *ent, int firemode, const vec3_t forward, vec3_t direction )
{

	weaponData_t	*thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	int				 fKnockBack		= ( float )thisWeaponData->firemodes[firemode].baseDamage;
	float			 fSpreadModifiers = 1.0f;
	vec3_t			 fAngles;
		
	/* Convert the forward to angle's to be able to modify it */
	vectoangles( forward, fAngles );

	if ( ent->client )
	{
		// Jumping				= Where are the bullets?!
		// Running				= Very Sloppy
		// Walking				= Slightly Sloppy
		// Crouching			= Slightly Accurate
		// Stationary			= Very Accurate
		// Stationary + Crouch	= Pinpoint Accuracy

		bool bIsCrouching	= ent->client->ps.pm_flags & PMF_DUCKED;
		bool bIsMoving		= ( ent->client->pers.cmd.forwardmove || ent->client->pers.cmd.rightmove || ent->client->pers.cmd.upmove > 0 );
		bool bIsWalking		= (ent->client->pers.cmd.buttons & BUTTON_WALKING) && !BG_IsSprinting (&ent->client->ps, &ent->client->pers.cmd, qfalse);
		bool bInIronsights  = (ent->client->pers.cmd.buttons & BUTTON_IRONSIGHTS);
		bool bIsInAir		= (ent->client->ps.groundEntityNum == ENTITYNUM_NONE);

		float fAccuracyRating =
			thisWeaponData->firemodes[firemode].weaponAccuracy.accuracyRating + ent->client->ps.stats[STAT_ACCURACY];

		int accuracyDrainDebounce = ( ent->client->accuracyDebounce > level.time ) ? 0 : thisWeaponData->firemodes[firemode].weaponAccuracy.msToDrainAccuracy;

		/* Client is in air, might be using a jetpack or something, add some slop! */
		if ( bIsInAir )
		{
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.inAirModifier;
			fKnockBack	*= 3.00f;
		}
		/* Client is currently running and not walking and/or crouching, a different slop value */
		else if ( !bIsCrouching && bIsMoving && !bIsWalking )
		{
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.crouchModifier;
			fKnockBack	*= 2.00f;
		}
		/* Client is walking around and is not crouching, somewhat better accuracy then running */
		else if ( bIsMoving && bIsWalking && !bIsCrouching )
		{
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.walkModifier;
			fKnockBack	*= 1.55f;
		}
		/* Client is crouching, even less spread. */
		else if ( bIsMoving && bIsWalking && bIsCrouching )
		{
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.crouchModifier;
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.walkModifier;
			fKnockBack	*= 1.35f;
		}
		/* Client is stationary and not crouching. Very good accuracy. */
		else if ( !bIsCrouching )
		{
			fKnockBack	*= 1.10f;
		}
		/* Client is crouching and stationary, pinpoint accuracy. Code for demonstrative purposes only! */
		else
		{
			fSpreadModifiers *= thisWeaponData->firemodes[firemode].weaponAccuracy.crouchModifier;
			fKnockBack	*= 1.0f;
		}
			
		/* The sights modifier is altered based on how far into scoping we are.
		 * This prevents "no-scoping" as seen in Call of Duty
		 */
		float phase = JKG_CalculateIronsightsPhase(&ent->client->ps, level.time, &ent->client->ironsightsBlend);
		float sightsDiff = thisWeaponData->firemodes[firemode].weaponAccuracy.sightsModifier - 1.0f;
		float adjustedSightsDiff = sightsDiff * phase;
		float finalSightsModifier = 1.0f + adjustedSightsDiff;

		/* Additional stabilising factor is in ironsights */
		if ( !bIsMoving && !bIsInAir &&
			bInIronsights )
		{
			fSpreadModifiers *= finalSightsModifier;
			fKnockBack  *= 0.8f;
		}
		else if( !bIsInAir && bInIronsights )
		{
			fSpreadModifiers *= finalSightsModifier;
		}

		/* We have some extra slop to add, so let's add it now */
		if( fAccuracyRating )
		{
			float pitch = 1.0f; // just putting in a dummy value here
			float yaw = 1.0f;	// just putting in a dummy value here

			WP_CalculateSpread( &pitch, &yaw, fAccuracyRating, fSpreadModifiers );

			fAngles[PITCH] += pitch;
			fAngles[YAW] += yaw;
		}

		/* We get more and more inaccurate every time we fire.
		If we're firing from an awkward position (such as being
		in mid-air), it takes us longer to recover from that shot. */
		accuracyDrainDebounce *= fSpreadModifiers;

		ent->client->accuracyDebounce = level.time + accuracyDrainDebounce;
		ent->client->ps.stats[STAT_ACCURACY] += thisWeaponData->firemodes[firemode].weaponAccuracy.accuracyRatingPerShot;
		if( ent->client->ps.stats[STAT_ACCURACY] > thisWeaponData->firemodes[firemode].weaponAccuracy.maxAccuracyAdd )
		{
			ent->client->ps.stats[STAT_ACCURACY] = thisWeaponData->firemodes[firemode].weaponAccuracy.maxAccuracyAdd;
		}
	}

	/* See if we have a decent knockback on this weapon! */
	if ( thisWeaponData->hasKnockBack )
	{
		vec3_t dir;

		/* Get the forward angle vector for this client */
		AngleVectors( ent->client->ps.viewangles, dir, NULL, NULL );
		VectorMA( ent->client->ps.velocity, -fKnockBack, dir, ent->client->ps.velocity );
		dir[YAW] = -dir[YAW];
			
		if ( fKnockBack > 280.0f )
		{
			Jetpack_Off( ent );
			G_Knockdown( ent, NULL, dir, fKnockBack, qtrue );
			G_Damage( ent, ent, ent, dir, ent->client->ps.origin, fKnockBack / 20, DAMAGE_NO_KNOCKBACK | DAMAGE_NO_ARMOR, MOD_UNKNOWN );
		}
	}

	/* Convert the angles back to a diretion and return the new value */
	AngleVectors( fAngles, direction, NULL, NULL );
}

/**************************************************
* WP_GetWeaponGravity
*
* Gets the gravity bool for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information. This will
* make projectiles be affected by gravity, such
* as grenades, rockets and slugs.
**************************************************/

qboolean WP_GetWeaponGravity( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	return thisWeaponData->firemodes[firemode].applyGravity;
}

/**************************************************
* WP_GetWeaponMOD
*
* Gets the weapon means of death for the currently 
* selected weapon with the appropriate mode. This 
* references the weapon table for this information.
**************************************************/

int WP_GetWeaponMOD( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	return thisWeaponData->firemodes[firemode].weaponMOD;
}

/**************************************************
* WP_GetWeaponSplashMOD
*
* Gets the splash means of deathe for the currently 
* selected weapon with the appropriate mode. This 
* references the weapon table for this information.
**************************************************/

int WP_GetWeaponSplashMOD( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	return thisWeaponData->firemodes[firemode].weaponSplashMOD;
}

/**************************************************
* GetWeaponRange
*
* Gets the weapon range for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

float WP_GetWeaponRange( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if ( thisWeaponData->firemodes[firemode].range )
	{
		return thisWeaponData->firemodes[firemode].range;
	}

	return WPR_M;
}

/**************************************************
* WP_GetWeaponShotCount
*
* Gets the shot count for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information. Recursive
* function keeps calling the fire weapon function
* to generate the required amount of shots.
**************************************************/

qboolean WP_GetWeaponShotCount( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	char *pShotCount = &thisWeaponData->firemodes[firemode].shotCount;

	/* This weapon has multiple shots, generate the fire events */
	if ( *pShotCount >= 2 )
	{
		/* Get the shot count and clear it to avoid recursive infinite loop */
		int iShotCount	= *pShotCount;
		int i			= 0;
		*pShotCount		= 0;

		/* Run through each and fire every shot count */
		for ( i = 0; i < iShotCount; i++ )
		{
			WP_FireGenericWeapon( ent, firemode );
		}

		/* Reset the table structure so the data has been preserved */
		*pShotCount	= iShotCount;
		return qtrue;
	}

	/* Nothing strange about it, let the original continue */
	return qfalse;
}


/**************************************************
* WP_GetWeaponSpeed
*
* Gets the projectile speed for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

float WP_GetWeaponSpeed( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if(!thisWeaponData)
	{
		// Some additional NULL checking here. You can never be too careful.
		return 5125.0f;
	}

	if ( thisWeaponData->firemodes[firemode].speed )
	{
		return thisWeaponData->firemodes[firemode].speed;
	}

	return 5125.0f;
}

/**************************************************
* WP_GetGrenadeBounce
*
* Gets the qboolean stating whether this weapon
* bounces off of living entities.
**************************************************/
qboolean WP_GetGrenadeBounce( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if(thisWeaponData)
		return thisWeaponData->firemodes[firemode].grenadeBounces;
		
	return qtrue;
}

/**************************************************
* WP_GetGrenadeBounceDamage
*
* Gets the damage that a grenade weapon does
* whenever it hits a living target.
**************************************************/
int WP_GetGrenadeBounceDamage( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if(thisWeaponData)
		return thisWeaponData->firemodes[firemode].grenadeBounceDMG;
		
	return 10;
}

/**************************************************
* WP_GetWeaponSplashRange
*
* Gets the splash damage for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

float WP_GetWeaponSplashRange( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );

	if(!thisWeaponData)
	{
		return 0.0f;
	}

	if ( thisWeaponData->firemodes[firemode].rangeSplash )
	{
		return thisWeaponData->firemodes[firemode].rangeSplash;
	}

	return 0.0f;
}

/**************************************************
* WP_FireGenericWeapon
*
* This is the main weapon fire routine, nearly every
* weapon in the game is routed through the weapon
* generic functions to use the weapon table. This
* in turn provides us with an accurate and simple
* place to get weapon information.
**************************************************/
extern void BG_SetTorsoAnimTimer(playerState_t *ps, int time );
void WP_FireGenericWeapon( gentity_t *ent, int firemode )
{
	BG_SetTorsoAnimTimer(&ent->client->ps, 200);

	if ( ent && ent->client && ent->client->NPC_class == CLASS_VEHICLE )
	{
		FireVehicleWeapon( ent, firemode );
		return;
	}
	else if ( ent->s.eType == ET_NPC 
		&& ent->s.weapon != WP_SABER 
		&& ent->enemy
		&& Distance(ent->enemy->r.currentOrigin, ent->r.currentOrigin) <= 72)
	{// UQ1: Added... At close range NPCs can hit you with their rifle butt...
		WP_FireMelee( ent, firemode );

		/* Reset the grenade cook timer, if any (with the proper weapon) */
		if ( ent->grenadeCookTime && ent->s.weapon == ent->grenadeWeapon && ent->s.weaponVariation == ent->grenadeVariation )
		{
			ent->grenadeCookTime = 0;
		}
		return;
	}
	else
	{
		if ( WP_GetWeaponShotCount( ent, firemode ))
		{
			return;
		}

		WP_CalculateAngles( ent );
		WP_CalculateMuzzlePoint( ent, forward, vright, up, muzzle );

		switch( ent->s.weapon )
		{
			case WP_NONE:
			{
				break;
			}

			case WP_STUN_BATON:
			{
				WP_FireStunBaton( ent, firemode );
				break;
			}

			case WP_MELEE:
			{
				WP_FireMelee( ent, firemode );
				break;
			}

			case WP_SABER:
			{
				break;
			}

			case WP_THERMAL:
			{
				vec3_t direction;
				WP_GetWeaponDirection (ent, firemode, forward, direction);
				WP_FireGenericGrenade( ent, firemode, muzzle, direction );
				break;
			}

			//// REPLACE ME
			case WP_TRIP_MINE:
			{
				WP_PlaceLaserTrap( ent, firemode );
				break;
			}

			case WP_DET_PACK:
			{
				WP_DropDetPack( ent, firemode );
				break;
			}

			case WP_EMPLACED_GUN:
			{
				if ( ent->client && ent->client->ewebIndex )
				{ 
					break;
				}

				WP_FireEmplaced( ent, !!firemode );
				break;
			}
			//// END REPLACE ME

			default:
			{
				if ( WP_GetWeaponIsHitscan( ent, firemode ))
				{
					ent->client->ps.torsoTimer += 100;
					WP_FireGenericTraceLine( ent, firemode );
				}
				else if ( WP_IsWeaponGrenade (ent, firemode) )
				{
					vec3_t direction;
					WP_GetWeaponDirection (ent, firemode, forward, direction);
					WP_FireGenericGrenade( ent, firemode, muzzle, direction );
				}
				else
				{
					vec3_t direction;
					WP_GetWeaponDirection (ent, firemode, forward, direction);

					ent->client->ps.torsoTimer += 100;
					WP_FireGenericMissile( ent, firemode, muzzle, direction );
				}
			}
		}

		/* Reset the grenade cook timer, if any (with the proper weapon) */
		if ( ent->grenadeCookTime && ent->s.weapon == ent->grenadeWeapon && ent->s.weaponVariation == ent->grenadeVariation )
		{
			ent->grenadeCookTime = 0;
		}
	}
}
