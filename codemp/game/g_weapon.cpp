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
#include "game/bg_public.h"
#include "game/bg_weapons.h"
#include "ghoul2/G2.h"
#include "qcommon/q_shared.h"
#include "jkg_damageareas.h"

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

#define MAX_PLACEABLE_CONSUME_WPNS			20		//default: 10 (used by tripmines and detonators)


const weaponFireModeStats_t *GetEntsCurrentFireMode( const gentity_t *ent, const weaponData_t* weapon )
{
	return &weapon->firemodes[ent->s.firingMode];
}

const weaponFireModeStats_t *GetEntsCurrentFireMode( const gentity_t *ent )
{
	const weaponData_t *weapon = GetWeaponData(ent->s.weapon, ent->s.weaponVariation);
	return GetEntsCurrentFireMode(ent, weapon);
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
		G_Damage( other, ent, ent->parent, NULL, ent->s.origin, ent->genericValue9, 0, JKG_GetMeansOfDamageIndex("MOD_EXPLOSION") );
	}
	G_BounceMissile( ent, trace );
}

void laserTrapExplode( gentity_t *self );

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
    
    return ent->client->clipammo[index][firemode];
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
    
    ent->client->clipammo[index][firemode] = ammo;
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
	missile->methodOfDeath = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");
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

void WP_ExplosiveDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod)
{
	self->activator = attacker;
	laserTrapExplode(self);
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
	    weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (ent);

		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		origin[2] += 8;
		SnapVector( origin );
		G_SetOrigin( ent, origin );

		ent->s.eType = ET_GENERAL;
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );
		ent->freeAfterEvent = qtrue;
	    

        JKG_DoSplashDamage (&fireMode->primary, ent->r.currentOrigin, ent, ent->parent, ent, ent->splashMethodOfDeath);
        
		if (fireMode->secondaryDmgPresent)
		{
			JKG_DoSplashDamage(&fireMode->secondary, ent->r.currentOrigin, ent, ent->parent, ent, ent->splashMethodOfDeath);
		}

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
#define LT_SPLASH_RAD		256.0f//256.0f
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
		weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (self);

		JKG_DoSplashDamage(&fireMode->primary, self->r.currentOrigin, self, self->activator, self, JKG_GetMeansOfDamageIndex("MOD_EXPLOSION"));
		
		if ( fireMode->secondaryDmgPresent )
		{
			JKG_DoSplashDamage(&fireMode->secondary, self->r.currentOrigin, self, self->activator, self, JKG_GetMeansOfDamageIndex("MOD_EXPLOSION"));
		}

		if( self->enemy && self->enemy->client && self->activator != self->enemy && 
			(!TeamFriendly(self->activator->s.number, self->enemy->s.number) || !OnSameTeam(self->activator, self->enemy)) )
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
		if ( ent->activator != other && ( !TeamFriendly( ent->activator->s.number, other->s.number ) || 
										  !OnSameTeam(ent->activator, other) ))
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
		//--futuza: checks what the trip mine should explode on, TeamFriendly() checks future Phase2+ 'parties'
		if (traceEnt == owner || TeamFriendly( traceEnt->s.number, owner->s.number ) || (traceEnt->client && traceEnt->client->noclip) || OnSameTeam(traceEnt, owner)) {
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
	laserTrap->methodOfDeath = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");
	laserTrap->splashMethodOfDeath = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");
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
	
	//limit to MAX_PLACEABLE_CONSUME_WPNS placed at any one time
	//see how many there are now
	while ( (found = G_Find( found, FOFS(classname), "laserTrap" )) != NULL )
	{
		if ( found->parent != ent )
		{
			continue;
		}
		foundLaserTraps[trapcount++] = found->s.number;
	}
	//now remove first ones we find until there are only max left
	found = NULL;
	trapcount_org = trapcount;
	lowestTimeStamp = level.time;
	while ( trapcount >= MAX_PLACEABLE_CONSUME_WPNS )
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
		weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (self);

		self->touch = 0;
		self->think = 0;
		self->nextthink = 0;

		self->takedamage = qfalse;

		VectorClear(self->s.apos.trDelta);
		self->s.apos.trType = TR_STATIONARY;

		JKG_DoSplashDamage(&fireMode->primary, self->r.currentOrigin, self, self->parent, self, JKG_GetMeansOfDamageIndex("MOD_EXPLOSION"));
		
		if ( fireMode->secondaryDmgPresent )
		{
			JKG_DoSplashDamage(&fireMode->secondary, self->r.currentOrigin, self, self->parent, self, JKG_GetMeansOfDamageIndex("MOD_EXPLOSION"));
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
	weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (self);
	int mod = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");

	self->pain = 0;
	self->die = 0;
	self->takedamage = qfalse;

	if ( self->target_ent )
	{//we were attached to something, do *direct* damage to it!
		JKG_DoDirectDamage(fireMode, self->target_ent, self, self->parent, vec3_origin, self->r.currentOrigin, 0, mod);
	}
	
	JKG_DoSplashDamage(&fireMode->primary, self->r.currentOrigin, self, self->parent, self, mod);
	
	if ( fireMode->secondaryDmgPresent )
	{
	    JKG_DoSplashDamage (&fireMode->secondary, self->r.currentOrigin, self, self->parent, self, mod);
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
	bolt->methodOfDeath = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");
	bolt->splashMethodOfDeath = JKG_GetMeansOfDamageIndex("MOD_EXPLOSION");
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

	//limit to MAX_PLACEABLE_CONSUME_WPNS placed at any one time
	//see how many there are now
	while ( (found = G_Find( found, FOFS(classname), "detpack" )) != NULL )
	{
		if ( found->parent != ent )
		{
			continue;
		}
		foundDetPacks[trapcount++] = found->s.number;
	}
	//now remove first ones we find until there are only the max left
	found = NULL;
	trapcount_org = trapcount;
	lowestTimeStamp = level.time;
	while ( trapcount >= MAX_PLACEABLE_CONSUME_WPNS )
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
	int mod = JKG_GetMeansOfDamageIndex("MOD_ELECTRICAL");

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
	    weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (ent);
		G_PlayEffect( EFFECT_STUNHIT, tr.endpos, tr.plane.normal );

		G_Sound( tr_ent, CHAN_WEAPON, G_SoundIndex( va("sound/weapons/melee/punch%d", Q_irand(1, 4)) ) );

		JKG_DoDamage(fireMode, tr_ent, ent, ent, forward, tr.endpos, DAMAGE_NO_KNOCKBACK, mod);

		if (tr_ent->client)
		{ //if it's a player then use the shock effect
			tr_ent->client->ps.electrifyTime = level.time + 700;
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

			G_Damage( tr_ent, ent, ent, forward, tr.endpos, dmg, DAMAGE_NO_SHIELD, MOD_MELEE );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

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

	VectorSubtract(self->s.origin, activator->client->ps.origin, fwd1);
	VectorNormalize(fwd1);

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
* WP_GetMaxRangeWithDecay
*
* Returns the maximum range of the weapon given 
* its range value, damage, and decayRate.
* damage == 0 when fired at this maxrange.
*
* If looking for maxrange when damage == 1, 
* subtract .01 from the result, or truncate the
* return type to an interger via cast.
* eg: static_cast<int>()
**************************************************/

//find the maximum effective range beyond a weapon's range (when damage == 0)
float WP_GetMaxRangeWithDecay(int damage, float range, float decayRate)
{
	//flip damage for heals
	if (damage < 0)
	{
		damage = -damage;
	}

	if (damage && range > 0 && decayRate > 0.0 && decayRate < 1.0)
	{
		if (decayRate < 0.0001)
			decayRate = 0.0001;

		float multiplier = static_cast<float>(std::log(1.0 / damage) / std::log(decayRate)); // --futuza: gcc has a stupid bug so we can't use logf(), see: https://stackoverflow.com/questions/55458487/stdexpf-and-stdlogf-not-recognized-by-gcc-7-2-0
		float maxRange = (range * multiplier) + range;

		//don't allow ranges beyond 10x
		if ((maxRange / range) > MAX_RANGE_MULTIPLIER)
			range = range * MAX_RANGE_MULTIPLIER;
		else
			range = maxRange;
	}

	return (range);
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
	bolt->s.ammoType			 = ent->s.ammoType;
	
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
	float		 fDecayRate			 = WP_GetWeaponDecayRate(ent, firemode);
	float		 fRange				 = WP_GetWeaponRange( ent, firemode );
	float		 fMaxRange			 = WP_GetMaxRangeWithDecay(iDamage, fRange, fDecayRate);
	float		 fSpeed				 = WP_GetWeaponSpeed( ent, firemode );
	float		 fSplashRange		 = WP_GetWeaponSplashRange( ent, firemode );
	gentity_t	*missile			 = NULL;

	/* Create the missile, fill in the name weapon, owner, methodOfDeath and such */
	missile							 = CreateMissile( origin, dir, fSpeed, 10000, ent, firemode /* FIXME: This will always be primary fire when firemodes is done */ );
	missile->classname				 = zClassname;
	missile->s.ammoType				 = ent->s.ammoType;
	missile->s.weapon				 = ent->s.weapon;
	missile->s.weaponVariation		 = ent->s.weaponVariation;
	missile->r.ownerNum				 = ent->s.number;
	missile->s.time					 = level.time; /* For client-side prediction */
	missile->parent					 = ent;
	missile->methodOfDeath			 = iMOD;

	/* Set the appropriate range for the bullet */
	if ( fRange >= 0.0f )
	{
		//if our maxRange is different, we need to calculate damage decay
		if (fRange != fMaxRange)
		{
			missile->range = fRange;
			missile->maxRange = fMaxRange;
			missile->decayRate = fDecayRate;
			missile->s.apos.trBase[0] = fMaxRange;
		}
		else
		{
			missile->range = fRange;
			missile->maxRange = fRange;
			missile->decayRate = 1;

			//missile->s.eventParm		 = fRange;
			// Xycaleth: casual hi-jacking of some random variable. It's so random you'd have
			// thought it wasn't out of place at all in Raven's code! :D
			missile->s.apos.trBase[0] = fRange;
		}
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
	float		 fMaxRange		 = fRange; //same as range, unless decayRate is set
	int			 iSkip			 = ent->s.number;
	float		 fDecayRate		 = WP_GetWeaponDecayRate(ent, firemode);

	gentity_t	*traceEnt, *tent;
	trace_t		 tr;
	vec3_t		 start, end;

	/* Client will use the weapon muzzle */
	if ( ent->client )
	{
		VectorCopy( ent->client->ps.origin, start );
		start[2] += ent->client->ps.viewheight;
	}
	/* NPC will use their own origin and use the eye coordinates */
	else
	{
		VectorCopy( ent->r.currentOrigin, start );
		start[2] += 24;
	}

	/* Increase search up to maxRange if decayRate is a factor */
	if (fDecayRate > 0.0 && fDecayRate < 1.0)
	{
		fMaxRange = WP_GetMaxRangeWithDecay(iDamage, fRange, fDecayRate);
	}

	while( iDamage > 0 )
	{
		/* Set the range forward to the end*/
		VectorMA(start, fMaxRange, forward, end);
			
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
			fMaxRange = fMaxRange - VectorNormalize( difference );
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
		tent->s.ammoType = ent->s.ammoType;
		if ( firemode )
		{
			tent->s.eFlags |= EF_ALT_FIRING;
		}

		/* The traced entity is a client with saber defense 3 so we must check if we can manage to block this attack */
		if ( traceEnt && traceEnt->client && traceEnt->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] >= FORCE_LEVEL_3 )
		{
			if ( WP_SaberCanBlock( traceEnt, tr.endpos, 0, WP_GetWeaponMOD(ent, firemode), qtrue, 0 ))
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
		} 
		else 
		{
			/* This isn't a client but can take damage, is a mover or a glass brush. Smash it up! */
			if ( traceEnt->r.svFlags & SVF_GLASS_BRUSH || traceEnt->takedamage || traceEnt->s.eType == ET_MOVER )
			{
				weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (ent);
				float distance = Distance(start, tr.endpos);  //measure distance from origin to impact point

				/* We're in range to do damage*/
				if (distance < fMaxRange)
				{
					damageDecay_t decay = {};
					decay.maxRange = fMaxRange;
					decay.recommendedRange = fRange;
					decay.distanceToDamageOrigin = distance;
					decay.decayRate = fDecayRate;

					JKG_DoDirectDamage(
						fireMode,
						traceEnt,
						ent,
						ent,
						forward,
						tr.endpos,
						DAMAGE_NO_KNOCKBACK,
						WP_GetWeaponMOD(ent, firemode),
						&decay);
				}
			}
			/* This always shows the sniper miss event, it's used for both primary and secondary fire. */
			else
			{
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
			weaponFireModeStats_t *fireMode = (weaponFireModeStats_t*)GetEntsCurrentFireMode (ent);
			float distance = Distance(start, tr.endpos);  //measure distance from origin to impact point

			/* Remember the legs/torse stance and client angles */
			if (traceEnt->client)
			{
				VectorCopy(traceEnt->client->ps.viewangles, preAng);
			}
			/* We're in range to do damage*/
			if (distance < fMaxRange)
			{
				damageDecay_t decay = {};
				decay.maxRange = fMaxRange;
				decay.recommendedRange = fRange;
				decay.distanceToDamageOrigin = distance;
				decay.decayRate = fDecayRate;

				JKG_DoDirectDamage(
					fireMode,
					traceEnt,
					ent,
					ent,
					forward,
					tr.endpos,
					DAMAGE_NO_KNOCKBACK,
					WP_GetWeaponMOD(ent, firemode),
					&decay);
			}

			/* Remove the amount of damage hit on this client from our trace, this way even normal disruptor shots can continue! */
			iDamage = iDamage - preHealth;
		}
		/* We hit an entity that can't take damage.. therefore, show the mark and stop tracing */
		else
		{
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

	if (ent->client)
	{
		if (ammoTable[ent->client->ps.ammoType].overrides.hitscan.first)
		{
			return ammoTable[ent->client->ps.ammoType].overrides.hitscan.second > 0;
		}
	}

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
	double boxSize = thisWeaponData->firemodes[firemode].boxSize;

	if (ent->client)
	{
		JKG_ApplyAmmoOverride(boxSize, ammoTable[ent->client->ps.ammoType].overrides.collisionSize);
	}

	if ( boxSize )
	{
		return boxSize;
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
	int bounceCount = thisWeaponData->firemodes[firemode].bounceCount;

	if (ent->client)
	{
		JKG_ApplyAmmoOverride(bounceCount, ammoTable[ent->client->ps.ammoType].overrides.bounces);
	}

	if ( bounceCount )
	{
		return bounceCount;
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
	int baseDamage = thisWeaponData->firemodes[firemode].baseDamage;

	if (ent->client)
	{
		JKG_ApplyAmmoOverride(baseDamage, ammoTable[ent->client->ps.ammoType].overrides.damage);
	}

	if ( thisWeaponData->firemodes[firemode].chargeTime )
	{
		int chargeDamage = baseDamage * WP_GetWeaponCharge( ent, firemode ) * thisWeaponData->firemodes[firemode].chargeMultiplier;
		damage = ( baseDamage > chargeDamage ) ? baseDamage : chargeDamage;
	}
	else
	{
		damage = baseDamage;
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
	vec3_t fAngles;
		
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
		// Rolling				= Random direction lol

		gclient_t *cl = ent->client;
		const bool bIsCrouching	= cl->ps.pm_flags & PMF_DUCKED;
		const bool bIsRolling = cl->ps.pm_flags & PMF_ROLLING;
		const bool bIsMoving	= ( cl->pers.cmd.forwardmove || cl->pers.cmd.rightmove || cl->pers.cmd.upmove > 0 );
		const bool bIsWalking	= (cl->pers.cmd.buttons & BUTTON_WALKING) && !BG_IsSprinting (&cl->ps, &cl->pers.cmd, qfalse);
		const bool bInIronsights  = (cl->pers.cmd.buttons & BUTTON_IRONSIGHTS);
		const bool bIsInAir		= (cl->ps.groundEntityNum == ENTITYNUM_NONE);

		const weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
		const weaponFireModeStats_t* fireMode = GetEntsCurrentFireMode(ent, thisWeaponData);
		const weaponAccuracyDetails_t* weaponAccuracy = &fireMode->weaponAccuracy;
		const ammo_t *currentAmmo = ammoTable + ent->s.ammoType;

		float fSpreadModifiers = 1.0f;
		double fKnockBack = ( double )fireMode->baseDamage;
		JKG_ApplyAmmoOverride(fKnockBack, currentAmmo->overrides.knockback);

		/* Client is in air, might be using a jetpack or something, add some slop! */
		if ( bIsInAir )
		{
			fSpreadModifiers *= weaponAccuracy->inAirModifier;
			fKnockBack	*= 3.00f;
		}
		/* Client is currently running and not walking and/or crouching, a different slop value */
		else if ( !bIsCrouching && bIsMoving && !bIsWalking )
		{
			fSpreadModifiers *= weaponAccuracy->crouchModifier;
			fKnockBack	*= 2.00f;
		}
		/* Client is walking around and is not crouching, somewhat better accuracy then running */
		else if ( bIsMoving && bIsWalking && !bIsCrouching )
		{
			fSpreadModifiers *= weaponAccuracy->walkModifier;
			fKnockBack	*= 1.55f;
		}
		/* Client is crouching, even less spread. */
		else if ( bIsMoving && bIsWalking && bIsCrouching )
		{
			fSpreadModifiers *= weaponAccuracy->crouchModifier;
			fSpreadModifiers *= weaponAccuracy->walkModifier;
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
			fSpreadModifiers *= weaponAccuracy->crouchModifier;
			fKnockBack	*= 1.0f;
		}

		/* Special case - client is rolling, shots should go wildly everywhere */
		if (bIsRolling)
		{
			fSpreadModifiers *= static_cast<float>(Q_irandSafe(50, 150));
			fKnockBack *= 3.0f;
		}
			
		/* The sights modifier is altered based on how far into scoping we are.
		 * This prevents "no-scoping" as seen in Call of Duty
		 */
		const float phase = JKG_CalculateIronsightsPhase(&cl->ps, level.time, &cl->ironsightsBlend);
		const float sightsDiff = weaponAccuracy->sightsModifier - 1.0f;
		const float adjustedSightsDiff = sightsDiff * phase;
		const float finalSightsModifier = 1.0f + adjustedSightsDiff;

		/* Additional stabilising factor is in ironsights */
		if ( !bIsMoving && !bIsInAir && bInIronsights )
		{
			fSpreadModifiers *= finalSightsModifier;
			fKnockBack  *= 0.8f;
		}
		else if( !bIsInAir && bInIronsights )
		{
			fSpreadModifiers *= finalSightsModifier;
		}

		/* We have some extra slop to add, so let's add it now */
		int nAccuracyBaseRating = weaponAccuracy->accuracyRating;
		JKG_ApplyAmmoOverride(nAccuracyBaseRating, currentAmmo->overrides.accuracyRatingBase);

		const int nAccuracyRating = nAccuracyBaseRating + cl->ps.stats[STAT_ACCURACY];
		if( nAccuracyRating )
		{
			float pitch = 1.0f; // just putting in a dummy value here
			float yaw = 1.0f;	// just putting in a dummy value here

			WP_CalculateSpread( &pitch, &yaw, nAccuracyRating, fSpreadModifiers );

			fAngles[PITCH] += pitch;
			fAngles[YAW] += yaw;
		}

		int accuracyDrainDebounce = ( cl->accuracyDebounce > level.time ) ? 0 : weaponAccuracy->msToDrainAccuracy;
		/* We get more and more inaccurate every time we fire.
		If we're firing from an awkward position (such as being
		in mid-air), it takes us longer to recover from that shot. */
		accuracyDrainDebounce *= fSpreadModifiers;

		cl->accuracyDebounce = level.time + accuracyDrainDebounce;

		int nAccuracyRatingPerShot = weaponAccuracy->accuracyRatingPerShot;
		JKG_ApplyAmmoOverride(nAccuracyRatingPerShot, currentAmmo->overrides.accuracyRatingPerShot);

		cl->ps.stats[STAT_ACCURACY] = Q_min(
			cl->ps.stats[STAT_ACCURACY] + nAccuracyRatingPerShot,
			weaponAccuracy->maxAccuracyAdd);

		/* See if we have a decent knockback on this weapon! */
		if ( thisWeaponData->hasKnockBack )
		{
			vec3_t dir;

			/* Get the forward angle vector for this client */
			AngleVectors( cl->ps.viewangles, dir, NULL, NULL );
			VectorMA( cl->ps.velocity, -fKnockBack, dir, cl->ps.velocity );
			dir[YAW] = -dir[YAW];
				
			if ( fKnockBack > 280.0f )
			{
				Jetpack_Off( ent );
				G_Knockdown( ent, NULL, dir, fKnockBack, qtrue );
				G_Damage( ent, ent, ent, dir, cl->ps.origin, fKnockBack / 20, DAMAGE_NO_KNOCKBACK | DAMAGE_NO_SHIELD, MOD_UNKNOWN );
			}
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
	int usesGravity = thisWeaponData->firemodes[firemode].applyGravity;

	JKG_ApplySimpleAmmoOverride(usesGravity, ammoTable[ent->s.ammoType].overrides.useGravity);

	return usesGravity > 0;
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
	int mod = thisWeaponData->firemodes[firemode].weaponMOD;

	if (ent->client)
	{
		JKG_ApplySimpleAmmoOverride(mod, ammoTable[ent->client->ps.ammoType].overrides.means);
	}

	return mod;
}

/**************************************************
* WP_GetWeaponSplashMOD
*
* Gets the splash means of death for the currently 
* selected weapon with the appropriate mode. This 
* references the weapon table for this information.
**************************************************/

int WP_GetWeaponSplashMOD( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	int mod = thisWeaponData->firemodes[firemode].weaponSplashMOD;

	if (ent->client)
	{
		JKG_ApplySimpleAmmoOverride(mod, ammoTable[ent->client->ps.ammoType].overrides.splashmeans);
	}

	return mod;
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
* GetWeaponDecayRate
*
* Gets the weapon's decayRate for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/
float WP_GetWeaponDecayRate(gentity_t* ent, int firemode)
{
	weaponData_t* thisWeaponData = GetWeaponData(ent->s.weapon, ent->s.weaponVariation);

	if (thisWeaponData->firemodes[firemode].decayRate)
	{
		return thisWeaponData->firemodes[firemode].decayRate;
	}

	return 0.25;
}


/**************************************************
* WP_GetWeaponShotCount
*
* Gets the shot count for the currently selected
* weapon with the appropriate mode. This references
* the weapon table for this information.
**************************************************/

int WP_GetWeaponShotCount( gentity_t *ent, int firemode )
{
	// This function used to be recursive, but that's horrendously bad...
	// Instead, we return the exact number of shots in one volley.

	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	int nShotCount = thisWeaponData->firemodes[firemode].shotCount;

	JKG_ApplyAmmoOverride(nShotCount, ammoTable[ent->s.ammoType].overrides.projectiles);

	return nShotCount;
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
	double fSpeed;

	if(!thisWeaponData)
	{
		// Some additional NULL checking here. You can never be too careful.
		return 5125.0f;
	}

	fSpeed = thisWeaponData->firemodes[firemode].speed;
	JKG_ApplyAmmoOverride(fSpeed, ammoTable[ent->s.ammoType].overrides.speed);

	if ( fSpeed )
	{
		return fSpeed;
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

double WP_GetWeaponSplashRange( gentity_t *ent, int firemode )
{
	weaponData_t *thisWeaponData = GetWeaponData( ent->s.weapon, ent->s.weaponVariation );
	double splashRange = thisWeaponData->firemodes[firemode].rangeSplash;

	if (ent->client)
	{
		JKG_ApplyAmmoOverride(splashRange, ammoTable[ent->client->ps.ammoType].overrides.splashRange);
	}

	return splashRange;
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
	int nShotCount;

	BG_SetTorsoAnimTimer(&ent->client->ps, 200);

	if ( ent->s.eType == ET_NPC 
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

	nShotCount = WP_GetWeaponShotCount(ent, firemode);

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
			for (int i = 0; i < nShotCount; i++)
			{
				WP_GetWeaponDirection(ent, firemode, forward, direction);
				WP_FireGenericGrenade(ent, firemode, muzzle, direction);
			}
			break;
		}

		//// REPLACE ME
		case WP_TRIP_MINE:
		{
			for (int i = 0; i < nShotCount; i++)
			{
				WP_PlaceLaserTrap(ent, firemode);
			}
			break;
		}

		case WP_DET_PACK:
		{
			for (int i = 0; i < nShotCount; i++)
			{
				WP_DropDetPack(ent, firemode);
			}
			break;
		}

		case WP_EMPLACED_GUN:
		{
			if ( ent->client && ent->client->ewebIndex )
			{ 
				break;
			}

			for (int i = 0; i < nShotCount; i++)
			{
				WP_FireEmplaced(ent, !!firemode);
			}
			break;
		}
		//// END REPLACE ME

		default:
			for(int i = 0; i < nShotCount; i++)
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
			break;
	}

	/* Reset the grenade cook timer, if any (with the proper weapon) */
	if ( ent->grenadeCookTime && ent->s.weapon == ent->grenadeWeapon && ent->s.weaponVariation == ent->grenadeVariation )
	{
		ent->grenadeCookTime = 0;
	}
}
