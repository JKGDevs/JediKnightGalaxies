// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "w_saber.h"
#include "qcommon/q_shared.h"
#include "jkg_damagetypes.h"

#define	MISSILE_PRESTEP_TIME	50

extern void laserTrapStick( gentity_t *ent, vec3_t endpos, vec3_t normal );
extern void NPC_Humanoid_Decloak( gentity_t *self );

extern qboolean FighterIsLanded( Vehicle_t *pVeh, playerState_t *parentPS );

/*
================
G_ReflectMissile

  Reflect the missile roughly back at it's owner
================
*/
float RandFloat(float min, float max);
void G_ReflectMissile( gentity_t *ent, gentity_t *missile, vec3_t forward ) 
{
	vec3_t	bounce_dir;
	int		i;
	float	speed;
	gentity_t	*owner = ent;
	int		isowner = 0;

	if ( ent->r.ownerNum )
	{
		owner = &g_entities[ent->r.ownerNum];
	}

	if (missile->r.ownerNum == ent->s.number)
	{ //the original owner is bouncing the missile, so don't try to bounce it back at him
		isowner = 1;
	}

	//save the original speed
	speed = VectorNormalize( missile->s.pos.trDelta );

	//if ( ent && owner && owner->NPC && owner->enemy && Q_stricmp( "Tavion", owner->NPC_type ) == 0 && Q_irand( 0, 3 ) )
	if ( &g_entities[missile->r.ownerNum] && missile->s.weapon != WP_SABER && missile->s.weapon != G2_MODEL_PART && !isowner )
	{//bounce back at them if you can
		VectorSubtract( g_entities[missile->r.ownerNum].r.currentOrigin, missile->r.currentOrigin, bounce_dir );
		VectorNormalize( bounce_dir );
	}
	else if (isowner)
	{ //in this case, actually push the missile away from me, and since we're giving boost to our own missile by pushing it, up the velocity
		vec3_t missile_dir;

		speed *= 1.5;

		VectorSubtract( missile->r.currentOrigin, ent->r.currentOrigin, missile_dir );
		VectorCopy( missile->s.pos.trDelta, bounce_dir );
		VectorScale( bounce_dir, DotProduct( forward, missile_dir ), bounce_dir );
		VectorNormalize( bounce_dir );
	}
	else
	{
		vec3_t missile_dir;

		VectorSubtract( ent->r.currentOrigin, missile->r.currentOrigin, missile_dir );
		VectorCopy( missile->s.pos.trDelta, bounce_dir );
		VectorScale( bounce_dir, DotProduct( forward, missile_dir ), bounce_dir );
		VectorNormalize( bounce_dir );
	}
	for ( i = 0; i < 3; i++ )
	{
		bounce_dir[i] += RandFloat( -0.2f, 0.2f );
	}

	VectorNormalize( bounce_dir );
	VectorScale( bounce_dir, speed, missile->s.pos.trDelta );
	missile->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( missile->r.currentOrigin, missile->s.pos.trBase );
	if ( missile->s.weapon != WP_SABER && missile->s.weapon != G2_MODEL_PART )
	{//you are mine, now!
		missile->r.ownerNum = ent->s.number;
	}
	if ( missile->s.weapon == WP_ROCKET_LAUNCHER )
	{//stop homing
		missile->think = 0;
		missile->nextthink = 0;
	}
}

void G_DeflectMissile( gentity_t *ent, gentity_t *missile, vec3_t forward ) 
{
	vec3_t	bounce_dir;
	int		i;
	float	speed;
	int		isowner = 0;
	vec3_t missile_dir;

	if (missile->r.ownerNum == ent->s.number)
	{ //the original owner is bouncing the missile, so don't try to bounce it back at him
		isowner = 1;
	}

	//save the original speed
	speed = VectorNormalize( missile->s.pos.trDelta );

	if (ent->client)
	{
		//VectorSubtract( ent->r.currentOrigin, missile->r.currentOrigin, missile_dir );
		AngleVectors(ent->client->ps.viewangles, missile_dir, 0, 0);
		VectorCopy(missile_dir, bounce_dir);
		//VectorCopy( missile->s.pos.trDelta, bounce_dir );
		VectorScale( bounce_dir, DotProduct( forward, missile_dir ), bounce_dir );
		VectorNormalize( bounce_dir );
	}
	else
	{
		VectorCopy(forward, bounce_dir);
		VectorNormalize(bounce_dir);
	}

	for ( i = 0; i < 3; i++ )
	{
		bounce_dir[i] += RandFloat( -1.0f, 1.0f );
	}

	VectorNormalize( bounce_dir );
	VectorScale( bounce_dir, speed, missile->s.pos.trDelta );
	missile->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( missile->r.currentOrigin, missile->s.pos.trBase );
	if ( missile->s.weapon != WP_SABER && missile->s.weapon != G2_MODEL_PART )
	{//you are mine, now!
		missile->r.ownerNum = ent->s.number;
	}
	if ( missile->s.weapon == WP_ROCKET_LAUNCHER )
	{//stop homing
		missile->think = 0;
		missile->nextthink = 0;
	}
}

/*
================
JKG_SaberDeflectMissile

Also handles stuff like force costs, etc
Only gets called when in projectile block mode

================
*/

void JKG_SaberDeflectMissile( gentity_t *ent, gentity_t *missile, vec3_t forward ) 
{
	vec3_t	bounce_dir;
	int		i;
	float	speed;
	gentity_t	*owner = ent;
	int		isowner = 0;

	if ( !(ent->client->ps.saberActionFlags & SAF_PROJBLOCKING) )
	{
		return;
	}

	if ( ent->r.ownerNum )
	{
		owner = &g_entities[ent->r.ownerNum];
	}

	if (missile->r.ownerNum == ent->s.number)
	{ //the original owner is bouncing the missile, so don't try to bounce it back at him
		isowner = 1;
	}

	//save the original speed
	speed = VectorNormalize( missile->s.pos.trDelta );

	//if ( ent && owner && owner->NPC && owner->enemy && Q_stricmp( "Tavion", owner->NPC_type ) == 0 && Q_irand( 0, 3 ) )
	if ( &g_entities[missile->r.ownerNum] && missile->s.weapon != WP_SABER && missile->s.weapon != G2_MODEL_PART && !isowner )
	{//bounce back at them if you can
		VectorSubtract( g_entities[missile->r.ownerNum].r.currentOrigin, missile->r.currentOrigin, bounce_dir );
		VectorNormalize( bounce_dir );
	}
	else if (isowner)
	{ //in this case, actually push the missile away from me, and since we're giving boost to our own missile by pushing it, up the velocity
		vec3_t missile_dir;

		speed *= 1.5;

		VectorSubtract( missile->r.currentOrigin, ent->r.currentOrigin, missile_dir );
		VectorCopy( missile->s.pos.trDelta, bounce_dir );
		VectorScale( bounce_dir, DotProduct( forward, missile_dir ), bounce_dir );
		VectorNormalize( bounce_dir );
	}
	else
	{
		vec3_t missile_dir;

		VectorSubtract( ent->r.currentOrigin, missile->r.currentOrigin, missile_dir );
		VectorCopy( missile->s.pos.trDelta, bounce_dir );
		VectorScale( bounce_dir, DotProduct( forward, missile_dir ), bounce_dir );
		VectorNormalize( bounce_dir );
	}
	for ( i = 0; i < 3; i++ )
	{
		float randomFloat = (level.time-ent->client->saberProjBlockTime)/2000.0f;
		float minimum;// = randomFloat/4;
		float result;
		int randomYesNo = Q_irand(0,1);

		if(randomFloat <= 0.05f)
		{
			randomFloat = 0;
		}
		else if(randomFloat > 0.8f)
		{
			randomFloat = 0.8f;
		}
		else if(randomFloat > 0.05)
		{
			randomFloat -= 0.05f;	// perfect blocks were WAY too hard to get before..this will help to make blocking easier
		}

		minimum = randomFloat / 4;

		// establish an effective minimum
		result = RandFloat(minimum, randomFloat);
		
		if(randomYesNo == 0)
		{
			bounce_dir[i] += result;
		}
		else
		{
			bounce_dir[i] -= result;
		}
	}

	VectorNormalize( bounce_dir );
	VectorScale( bounce_dir, speed, missile->s.pos.trDelta );
	missile->s.pos.trTime = level.time;		// move a bit on the very first frame
	VectorCopy( missile->r.currentOrigin, missile->s.pos.trBase );
	if ( missile->s.weapon != WP_SABER && missile->s.weapon != G2_MODEL_PART )
	{//you are mine, now!
		missile->r.ownerNum = ent->s.number;
	}
	if ( missile->s.weapon == WP_ROCKET_LAUNCHER )
	{//stop homing
		missile->think = 0;
		missile->nextthink = 0;
	}
}

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->flags & FL_BOUNCE_SHRAPNEL ) 
	{
		VectorScale( ent->s.pos.trDelta, 0.25f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_GRAVITY;

		// check for stop
		if ( trace->plane.normal[2] > 0.7 && ent->s.pos.trDelta[2] < 40 ) //this can happen even on very slightly sloped walls, so changed it from > 0 to > 0.7
		{
			G_SetOrigin( ent, trace->endpos );
			ent->nextthink = level.time + 100;
			return;
		}
	}
	else if ( ent->flags & FL_BOUNCE_HALF ) 
	{
		VectorScale( ent->s.pos.trDelta, 0.65f, ent->s.pos.trDelta );
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) 
		{
			G_SetOrigin( ent, trace->endpos );
			return;
		}
	}

	if (ent->s.weapon == WP_SABER)
	{
		G_Sound(ent, CHAN_BODY, G_SoundIndex(va("sound/weapons/saber/bounce%i.wav", Q_irand(1, 3))));
	}
	else if (ent->s.weapon == G2_MODEL_PART)
	{
		//Limb bounce sound?
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;

	if (ent->bounceCount != -5)
	{
		ent->bounceCount--;
	}
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

	ent->freeAfterEvent = qtrue;

	ent->takedamage = qfalse;
	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, 
				ent, ent->splashMethodOfDeath ) ) 
		{
			if (ent->parent)
			{
				g_entities[ent->parent->s.number].client->accuracy_hits++;
			}
			else if (ent->activator)
			{
				g_entities[ent->activator->s.number].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}

void G_RunStuckMissile( gentity_t *ent )
{
	if ( ent->takedamage )
	{
		if ( ent->s.groundEntityNum >= 0 && ent->s.groundEntityNum < ENTITYNUM_WORLD )
		{
			gentity_t *other = &g_entities[ent->s.groundEntityNum];

			if ( (!VectorCompare( vec3_origin, other->s.pos.trDelta ) && other->s.pos.trType != TR_STATIONARY) || 
				(!VectorCompare( vec3_origin, other->s.apos.trDelta ) && other->s.apos.trType != TR_STATIONARY) )
			{//thing I stuck to is moving or rotating now, kill me
				G_Damage( ent, other, other, NULL, NULL, 99999, 0, MOD_CRUSH );
				return;
			}
		}
	}
	// check think function
	G_RunThink( ent );
}

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


//-----------------------------------------------------------------------------
gentity_t *CreateMissile( vec3_t org, vec3_t dir, float vel, int life, 
							gentity_t *owner, qboolean altFire)
//-----------------------------------------------------------------------------
{
	gentity_t	*missile;

	missile = G_Spawn();
	
	missile->nextthink = level.time + life;
	missile->think = G_FreeEntity;
	missile->s.eType = ET_MISSILE;
	missile->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	missile->parent = owner;
	missile->r.ownerNum = owner->s.number;

	missile->s.firingMode = owner->s.firingMode;

	missile->s.pos.trType = TR_LINEAR;
	missile->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;	// NOTENOTE This is a Quake 3 addition over JK2
	missile->target_ent = NULL;

	SnapVector(org);
	VectorCopy( org, missile->s.pos.trBase );
	VectorScale( dir, vel, missile->s.pos.trDelta );
	VectorCopy( org, missile->r.currentOrigin);
	SnapVector(missile->s.pos.trDelta);

	return missile;
}

void G_MissileBounceEffect( gentity_t *ent, vec3_t org, vec3_t dir )
{
	//FIXME: have an EV_BOUNCE_MISSILE event that checks the s.weapon and does the appropriate effect
	switch( ent->s.weapon )
	{
	case WP_BOWCASTER:
		G_PlayEffectID( G_EffectIndex("bowcaster/deflect"), ent->r.currentOrigin, dir );
		break;
	case WP_BLASTER:
	case WP_BRYAR_PISTOL:
		G_PlayEffectID( G_EffectIndex("blaster/deflect"), ent->r.currentOrigin, dir );
		break;
	default:
		{
			gentity_t *te = G_TempEntity( org, EV_SABER_BLOCK );
			VectorCopy(org, te->s.origin);
			VectorCopy(dir, te->s.angles);
			te->s.eventParm = 0;
			te->s.weapon = 0;//saberNum
			te->s.legsAnim = 0;//bladeNum
		}
		break;
	}
}

static void G_GrenadeBounceEvent ( const gentity_t *ent )
{
    gentity_t *tent = G_TempEntity (ent->r.currentOrigin, EV_GRENADE_BOUNCE);
    VectorCopy (ent->r.currentOrigin, tent->s.origin);
    tent->s.number = ent->s.number;
    tent->s.weapon = ent->s.weapon;
    tent->s.weaponVariation = ent->s.weaponVariation;
    
    /*if ( ent->s.eFlags & EF_ALT_FIRING )
    {
        tent->s.eFlags |= EF_ALT_FIRING;
    }*/

	tent->s.firingMode = ent->s.firingMode;
}

/*
================
G_MissileImpact
================
*/
qboolean WP_SaberBlockNonRandom( gentity_t *self, gentity_t *other, vec3_t hitloc, qboolean missileBlock );
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;
	qboolean		isKnockedSaber = qfalse;

	other = &g_entities[trace->entityNum];

	// check for bounce
	if ( !other->takedamage &&
		(ent->bounceCount > 0 || ent->bounceCount == -5) &&
		( ent->flags & ( FL_BOUNCE | FL_BOUNCE_HALF ) ) ) {
		int originalBounceCount = ent->bounceCount;
		G_BounceMissile( ent, trace );
		if ( originalBounceCount != ent->bounceCount )
		{
		    G_GrenadeBounceEvent ((const gentity_t *)ent);
		}
		return;
	}
	else if (ent->neverFree && ent->s.weapon == WP_SABER && (ent->flags & FL_BOUNCE_HALF))
	{ //this is a knocked-away saber
		if (ent->bounceCount > 0 || ent->bounceCount == -5)
		{
		    	int originalBounceCount = ent->bounceCount;
		    	G_BounceMissile( ent, trace );
		    	G_GrenadeBounceEvent ((const gentity_t *)ent);
		    	if ( originalBounceCount != ent->bounceCount )
	        	{
	            		G_GrenadeBounceEvent ((const gentity_t *)ent);
	        	}
			return;
		}

		isKnockedSaber = qtrue;
	}
	
	// I would glom onto the FL_BOUNCE code section above, but don't feel like risking breaking something else
	if ( (!other->takedamage && (ent->bounceCount > 0 || ent->bounceCount == -5) && ( ent->flags&(FL_BOUNCE_SHRAPNEL) ) ) || ((trace->surfaceFlags&SURF_FORCEFIELD)&&!ent->splashDamage&&!ent->splashRadius&&(ent->bounceCount > 0 || ent->bounceCount == -5)) ) 
	{
		G_BounceMissile( ent, trace );

		if ( ent->bounceCount < 1 )
		{
			ent->flags &= ~FL_BOUNCE_SHRAPNEL;
		}
		return;
	}

	/*
	if ( !other->takedamage && ent->s.weapon == WP_THERMAL && !ent->alt_fire )
	{//rolling thermal det - FIXME: make this an eFlag like bounce & stick!!!
		//G_BounceRollMissile( ent, trace );
		if ( ent->owner && ent->owner->s.number == 0 ) 
		{
			G_MissileAddAlerts( ent );
		}
		//gi.linkentity( ent );
		return;
	}
	*/

	if ((other->r.contents & CONTENTS_LIGHTSABER) && !isKnockedSaber)
	{ //hit this person's saber, so..
		gentity_t *otherOwner = &g_entities[other->r.ownerNum];

		if (otherOwner->takedamage && otherOwner->client && otherOwner->client->ps.duelInProgress &&
			otherOwner->client->ps.duelIndex != ent->r.ownerNum)
		{
			goto killProj;
		}
	}
	else if (!isKnockedSaber)
	{
		if (other->takedamage && other->client && other->client->ps.duelInProgress &&
			other->client->ps.duelIndex != ent->r.ownerNum)
		{
			goto killProj;
		}
	}

	if (other->flags & FL_DMG_BY_HEAVY_WEAP_ONLY)
	{
		if (ent->methodOfDeath != MOD_REPEATER_ALT &&
			ent->methodOfDeath != MOD_ROCKET &&
			ent->methodOfDeath != MOD_FLECHETTE_ALT_SPLASH &&
			ent->methodOfDeath != MOD_ROCKET_HOMING &&
			ent->methodOfDeath != MOD_THERMAL &&
			ent->methodOfDeath != MOD_THERMAL_SPLASH &&
			ent->methodOfDeath != MOD_TRIP_MINE_SPLASH &&
			ent->methodOfDeath != MOD_TIMED_MINE_SPLASH &&
			ent->methodOfDeath != MOD_DET_PACK_SPLASH &&
			ent->methodOfDeath != MOD_VEHICLE &&
			ent->methodOfDeath != MOD_CONC &&
			ent->methodOfDeath != MOD_CONC_ALT &&
			ent->methodOfDeath != MOD_SABER &&
			ent->methodOfDeath != MOD_TURBLAST)
		{
			vec3_t fwd;

			if (trace)
			{
				VectorCopy(trace->plane.normal, fwd);
			}
			else
			{ //oh well
				AngleVectors(other->r.currentAngles, fwd, NULL, NULL);
			}

			G_DeflectMissile(other, ent, fwd);
			G_MissileBounceEffect(ent, ent->r.currentOrigin, fwd);
			return;
		}
	}

	if ((other->flags & FL_SHIELDED) &&
		ent->s.weapon != WP_ROCKET_LAUNCHER &&
		ent->s.weapon != WP_THERMAL &&
		ent->s.weapon != WP_TRIP_MINE &&
		ent->s.weapon != WP_DET_PACK &&
		ent->s.weapon != WP_EMPLACED_GUN &&
		ent->methodOfDeath != MOD_REPEATER_ALT &&
		ent->methodOfDeath != MOD_FLECHETTE_ALT_SPLASH && 
		ent->methodOfDeath != MOD_TURBLAST &&
		ent->methodOfDeath != MOD_VEHICLE &&
		ent->methodOfDeath != MOD_CONC &&
		ent->methodOfDeath != MOD_CONC_ALT &&
		!(ent->dflags&DAMAGE_HEAVY_WEAP_CLASS) )
	{// entity is shielded
		vec3_t fwd;

		if (other->client)
		{
			AngleVectors(other->client->ps.viewangles, fwd, NULL, NULL);
		}
		else
		{
			AngleVectors(other->r.currentAngles, fwd, NULL, NULL);
		}

		G_DeflectMissile(other, ent, fwd);
		G_MissileBounceEffect(ent, ent->r.currentOrigin, fwd);
		return;
	}

	// SABERFIXME: make this based on .wpn file? some conc rifles should be able to be deflected...
	if (other->takedamage && other->client &&
		ent->s.weapon != WP_ROCKET_LAUNCHER &&
		ent->s.weapon != WP_THERMAL &&
		ent->s.weapon != WP_TRIP_MINE &&
		ent->s.weapon != WP_DET_PACK &&
		ent->methodOfDeath != MOD_REPEATER_ALT &&
		ent->methodOfDeath != MOD_FLECHETTE_ALT_SPLASH &&
		ent->methodOfDeath != MOD_CONC &&
		ent->methodOfDeath != MOD_CONC_ALT &&
		other->client->saberBlockDebounce < level.time &&
		!isKnockedSaber &&
		WP_SaberCanBlock(other, ent->r.currentOrigin, 0, 0, qtrue, 0))
	{ //only block one projectile per 200ms (to prevent giant swarms of projectiles being blocked)
		vec3_t fwd;
		gentity_t *te;
		int otherDefLevel = other->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE];

		te = G_TempEntity( ent->r.currentOrigin, EV_SABER_BLOCK );
		VectorCopy(ent->r.currentOrigin, te->s.origin);
		VectorCopy(trace->plane.normal, te->s.angles);
		te->s.eventParm = 0;
		te->s.weapon = 0;//saberNum
		te->s.legsAnim = 0;//bladeNum

		/*if (other->client->ps.velocity[2] > 0 ||
			other->client->pers.cmd.forwardmove ||
			other->client->pers.cmd.rightmove)
			*/
		if (other->client->ps.velocity[2] > 0 ||
			other->client->pers.cmd.forwardmove < 0) //now we only do it if jumping or running backward. Should be able to full-on charge.
		{
			otherDefLevel -= 1;
			if (otherDefLevel < 0)
			{
				otherDefLevel = 0;
			}
		}

		AngleVectors(other->client->ps.viewangles, fwd, NULL, NULL);
		// SABERFIXME: Don't make this force based. This code is ugly as fuck anyway
		other->client->saberBlockDebounce = level.time + (350 - (otherDefLevel*100)); //200;

		//For jedi AI
		other->client->ps.saberEventFlags |= SEF_DEFLECTED;
		if ( other->client->ps.saberActionFlags & (1 << SAF_BLOCKING) && !(other->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING) ) )
		{
			goto killProj;
 		}
		else if ( other->client->ps.saberActionFlags & SAF_PROJBLOCKING )
		{
			JKG_SaberDeflectMissile(other, ent, fwd);
			other->client->saberProjBlockTime += 500; // give them a little bit of leeway --eez
		}
		return;
	}
	else if ((other->r.contents & CONTENTS_LIGHTSABER) && !isKnockedSaber)
	{ //hit this person's saber, so..
		gentity_t *otherOwner = &g_entities[other->r.ownerNum];

		if (otherOwner->takedamage && otherOwner->client &&
			ent->s.weapon != WP_ROCKET_LAUNCHER &&
			ent->s.weapon != WP_THERMAL &&
			ent->s.weapon != WP_TRIP_MINE &&
			ent->s.weapon != WP_DET_PACK &&
			ent->methodOfDeath != MOD_REPEATER_ALT &&
			ent->methodOfDeath != MOD_FLECHETTE_ALT_SPLASH &&
			ent->methodOfDeath != MOD_CONC &&
			ent->methodOfDeath != MOD_CONC_ALT /*&&
			otherOwner->client->ps.saberBlockTime < level.time*/)
		{ //for now still deflect even if saberBlockTime >= level.time because it hit the actual saber
			vec3_t fwd;
			gentity_t *te;
			int otherDefLevel = otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE];

			//in this case, deflect it even if we can't actually block it because it hit our saber
			//WP_SaberCanBlock(otherOwner, ent->r.currentOrigin, 0, 0, qtrue, 0);
			if (otherOwner->client && otherOwner->client->ps.weaponTime <= 0)
			{
				WP_SaberBlockNonRandom(otherOwner, NULL, ent->r.currentOrigin, qtrue);		// <-- ??? --eez
			}

			te = G_TempEntity( ent->r.currentOrigin, EV_SABER_BLOCK );
			VectorCopy(ent->r.currentOrigin, te->s.origin);
			VectorCopy(trace->plane.normal, te->s.angles);
			te->s.eventParm = 0;
			te->s.weapon = 0;//saberNum
			te->s.legsAnim = 0;//bladeNum

			/*if (otherOwner->client->ps.velocity[2] > 0 ||
				otherOwner->client->pers.cmd.forwardmove ||
				otherOwner->client->pers.cmd.rightmove)*/
			if (otherOwner->client->ps.velocity[2] > 0 ||
				otherOwner->client->pers.cmd.forwardmove < 0) //now we only do it if jumping or running backward. Should be able to full-on charge.
			{
				otherDefLevel -= 1;
				if (otherDefLevel < 0)
				{
					otherDefLevel = 0;
				}
			}

			AngleVectors(otherOwner->client->ps.viewangles, fwd, NULL, NULL);
			otherOwner->client->saberBlockDebounce = level.time + (350 - (otherDefLevel*100));//200;

			//For jedi AI
			otherOwner->client->ps.saberEventFlags |= SEF_DEFLECTED;

			// um...this code is a lil messed up, so i'll replace it with my own --eez
			if ( otherOwner->client->ps.saberActionFlags & (1 << SAF_BLOCKING) && !(otherOwner->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING )) )
			{
				goto killProj;
 			}
			else if ( otherOwner->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) )
			{
				// SABERFIXME: Write new function for this
				JKG_SaberDeflectMissile(otherOwner, ent, fwd);
				otherOwner->client->saberProjBlockTime += 500; // give them a little bit of leeway --eez
			}

			return;
		}
	}

	if( ent->genericValue10 )	// nerf this check in order to make grenade bouncing work --eez
	{
		vec3_t fwd;

		if (other->client)
		{
			AngleVectors(other->client->ps.viewangles, fwd, NULL, NULL);
		}
		else
		{
			AngleVectors(other->r.currentAngles, fwd, NULL, NULL);
		}
		VectorScale(ent->s.pos.trDelta, 0.2, ent->s.pos.trDelta);
		G_Damage(other, ent, ent->parent, fwd, ent->s.origin, ent->genericValue9, 0, MOD_THERMAL);
		G_DeflectMissile(other, ent, fwd);
		//G_MissileBounceEffect(ent, ent->r.currentOrigin, fwd);
		return;
	}

	// check for sticking
	if ( !other->takedamage && ( ent->s.eFlags & EF_MISSILE_STICK ) ) 
	{
		laserTrapStick( ent, trace->endpos, trace->plane.normal );
		G_AddEvent( ent, EV_MISSILE_STICK, 0 );
		return;
	}

	// impact damage
	if (other->takedamage && !isKnockedSaber) {
		// FIXME: wrong damage direction?
		weaponData_t *weapon = GetWeaponData (ent->s.weapon, ent->s.weaponVariation);
		weaponFireModeStats_t *fireMode = &weapon->firemodes[ent->s.firingMode];
		
		if ( ent->damage || fireMode->damageTypeHandle || fireMode->secondaryDmgHandle ) {
			vec3_t	velocity;

			hitClient = qtrue;
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;	// stepped on a grenade
			}

            if ( fireMode->damageTypeHandle )
            {
                JKG_DoDamage (fireMode->damageTypeHandle, other, ent, &g_entities[ent->r.ownerNum], velocity, ent->r.currentOrigin, 0, ent->methodOfDeath);
            }
            else if ( ent->damage )
            {
                G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity, ent->r.currentOrigin, ent->damage, 0, ent->methodOfDeath);
            }
            
            if ( fireMode->secondaryDmgHandle )
            {
                JKG_DoDamage (fireMode->secondaryDmgHandle, other, ent, &g_entities[ent->r.ownerNum], velocity, ent->r.currentOrigin, 0, ent->methodOfDeath);
            }

			if (other->client)
			{ //What I'm wondering is why this isn't in the NPC pain funcs. But this is what SP does, so whatever.
				class_t	npc_class = other->client->NPC_class;

				// If we are a robot and we aren't currently doing the full body electricity...
				if ( npc_class == CLASS_SEEKER || npc_class == CLASS_PROBE || npc_class == CLASS_MOUSE ||
					   npc_class == CLASS_GONK || npc_class == CLASS_R2D2 || npc_class == CLASS_R5D2 || npc_class == CLASS_REMOTE ||
					   npc_class == CLASS_MARK1 || npc_class == CLASS_MARK2 || //npc_class == CLASS_PROTOCOL ||//no protocol, looks odd
					   npc_class == CLASS_INTERROGATOR || npc_class == CLASS_ATST || npc_class == CLASS_SENTRY )
				{
					// special droid only behaviors
					if ( other->client->ps.electrifyTime < level.time + 100 )
					{
						// ... do the effect for a split second for some more feedback
						other->client->ps.electrifyTime = level.time + 450;
					}
					//FIXME: throw some sparks off droids,too
				}
			}
		}
	}
killProj:
	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if ( other->takedamage && other->client && !isKnockedSaber ) {
		{
			G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
			ent->s.otherEntityNum = other->s.number;
		}
	} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
		G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
	} else if (ent->s.weapon != G2_MODEL_PART && !isKnockedSaber) {
		G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
	}

	if (!isKnockedSaber)
	{
		ent->freeAfterEvent = qtrue;

		// change over to a normal entity right at the point of impact
		ent->s.eType = ET_GENERAL;
	}

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	ent->takedamage = qfalse;

	if (ent->s.weapon == G2_MODEL_PART)
	{
		ent->freeAfterEvent = qfalse; //it will free itself
	}

	if(ent->splashRadius && ent->splashDamage && !ent->genericValue10)
	{
		G_RadiusDamage(trace->endpos, &g_entities[ent->r.ownerNum], ent->splashDamage, ent->splashRadius, NULL, ent, ent->methodOfDeath);
	}

	trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin, groundSpot;
	trace_t		tr;
	int			passent;
	qboolean	isKnockedSaber = qfalse;
	vec3_t		difference;
	int			iLen;

	if (ent->neverFree && ent->s.weapon == WP_SABER && (ent->flags & FL_BOUNCE_HALF))
	{
		isKnockedSaber = qtrue;
		ent->s.pos.trType = TR_GRAVITY;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
	}
	else {
		// ignore interactions with the missile owner
		if ( (ent->r.svFlags&SVF_OWNERNOTSHARED) 
			&& (ent->s.eFlags&EF_JETPACK_ACTIVE) )
		{//A vehicle missile that should be solid to its owner
			//I don't care about hitting my owner
			passent = ent->s.number;
		}
		else
		{
			passent = ent->r.ownerNum;
		}
	}

	/* Get the new length we have just calculated */
	VectorSubtract( origin, ent->s.pos.trBase, difference );
	iLen = ( int ) VectorLength( difference );

	/* The length is larger then allowed, cut off the end position for the line trace */
	if ( ent->s.apos.trBase[0] && iLen >= ent->s.apos.trBase[0] )
	{
		vec3_t fwd;
		AngleVectors( ent->s.angles, fwd, NULL, NULL );
		VectorMA( ent->s.pos.trBase, ent->s.eventParm, fwd, origin );
	}

	// trace a line from the previous position to the current position
	if (d_projectileGhoul2Collision.integer)
	{
		trap_G2Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );

		if ( tr.fraction != 1.0 && tr.entityNum < ENTITYNUM_WORLD)
		{
			gentity_t *g2Hit = &g_entities[tr.entityNum];

			if (g2Hit->inuse && g2Hit->client && g2Hit->ghoul2)
			{ //since we used G2TRFLAG_GETSURFINDEX, tr.surfaceFlags will actually contain the index of the surface on the ghoul2 model we collided with.
				g2Hit->client->g2LastSurfaceHit = tr.surfaceFlags;
				g2Hit->client->g2LastSurfaceTime = level.time;
			}

			if (g2Hit->ghoul2)
			{
				tr.surfaceFlags = 0; //clear the surface flags after, since we actually care about them in here.
			}
		}
	}
	else
	{
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );
	}

	if ( tr.startsolid || tr.allsolid ) {
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
		tr.fraction = 0;
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	if (ent->passThroughNum && tr.entityNum == (ent->passThroughNum-1))
	{
		VectorCopy( origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
		goto passthrough;
	}

	trap_LinkEntity( ent );

	if (ent->s.weapon == G2_MODEL_PART && !ent->bounceCount)
	{
		vec3_t lowerOrg;
		trace_t trG;

		VectorCopy(ent->r.currentOrigin, lowerOrg);
		lowerOrg[2] -= 1;
		trap_Trace( &trG, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, lowerOrg, passent, ent->clipmask );

		VectorCopy(trG.endpos, groundSpot);

		if (!trG.startsolid && !trG.allsolid && trG.entityNum == ENTITYNUM_WORLD)
		{
			ent->s.groundEntityNum = trG.entityNum;
		}
		else
		{
			ent->s.groundEntityNum = ENTITYNUM_NONE;
		}
	}

	if ( tr.fraction != 1) {
		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// If grapple, reset owner
			if (ent->parent && ent->parent->client && ent->parent->client->hook == ent) {
				ent->parent->client->hook = NULL;
			}

			if ((ent->s.weapon == WP_SABER && ent->isSaberEntity) || isKnockedSaber)
			{
				G_RunThink( ent );
				return;
			}
			else if (ent->s.weapon != G2_MODEL_PART)
			{
				G_FreeEntity( ent );
				return;
			}
		}

#if 0 //will get stomped with missile impact event...
		if (ent->s.weapon > WP_NONE && ent->s.weapon < WP_NUM_WEAPONS &&
			(tr.entityNum < MAX_CLIENTS || g_entities[tr.entityNum].s.eType == ET_NPC))
		{ //player or NPC, try making a mark on him
			/*
			gentity_t *evEnt = G_TempEntity(ent->r.currentOrigin, EV_GHOUL2_MARK);

			evEnt->s.owner = tr.entityNum; //the entity the mark should be placed on
			evEnt->s.weapon = ent->s.weapon; //the weapon used (to determine mark type)
			VectorCopy(ent->r.currentOrigin, evEnt->s.origin); //the point of impact

			//origin2 gets the predicted trajectory-based position.
			BG_EvaluateTrajectory( &ent->s.pos, level.time, evEnt->s.origin2 );

			//If they are the same, there will be problems.
			if (VectorCompare(evEnt->s.origin, evEnt->s.origin2))
			{
				evEnt->s.origin2[2] += 2; //whatever, at least it won't mess up.
			}
			*/
			//ok, let's try adding it to the missile ent instead (tempents bad!)
			G_AddEvent(ent, EV_GHOUL2_MARK, 0);

			//copy current pos to s.origin, and current projected traj to origin2
			VectorCopy(ent->r.currentOrigin, ent->s.origin);
			BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->s.origin2 );

			//the index for whoever we are hitting
			ent->s.otherEntityNum = tr.entityNum;

			if (VectorCompare(ent->s.origin, ent->s.origin2))
			{
				ent->s.origin2[2] += 2.0f; //whatever, at least it won't mess up.
			}
		}
#else
		if (ent->s.weapon > WP_NONE && ent->s.weapon < WP_NUM_WEAPONS &&
			(tr.entityNum < MAX_CLIENTS || g_entities[tr.entityNum].s.eType == ET_NPC))
		{ //player or NPC, try making a mark on him
			//copy current pos to s.origin, and current projected traj to origin2
			VectorCopy(ent->r.currentOrigin, ent->s.origin);
			BG_EvaluateTrajectory( &ent->s.pos, level.time, ent->s.origin2 );

			if (VectorCompare(ent->s.origin, ent->s.origin2))
			{
				ent->s.origin2[2] += 2.0f; //whatever, at least it won't mess up.
			}
		}
#endif

		G_MissileImpact( ent, &tr );

		if (tr.entityNum == ent->s.otherEntityNum)
		{ //if the impact event other and the trace ent match then it's ok to do the g2 mark
			ent->s.trickedentindex = 1;
		}

		if ( ent->s.eType != ET_MISSILE && ent->s.weapon != G2_MODEL_PART )
		{
			return;		// exploded
		}
	}

passthrough:
	if ( ent->s.pos.trType == TR_STATIONARY && (ent->s.eFlags&EF_MISSILE_STICK) )
	{//stuck missiles should check some special stuff
		G_RunStuckMissile( ent );
		return;
	}

	if (ent->s.weapon == G2_MODEL_PART)
	{
		if (ent->s.groundEntityNum == ENTITYNUM_WORLD)
		{
			ent->s.pos.trType = TR_LINEAR;
			VectorClear(ent->s.pos.trDelta);
			ent->s.pos.trTime = level.time;

			VectorCopy(groundSpot, ent->s.pos.trBase);
			VectorCopy(groundSpot, ent->r.currentOrigin);

			if (ent->s.apos.trType != TR_STATIONARY)
			{
				ent->s.apos.trType = TR_STATIONARY;
				ent->s.apos.trTime = level.time;

				ent->s.apos.trBase[ROLL] = 0;
				ent->s.apos.trBase[PITCH] = 0;
			}
		}
	}

	/* We're still past the allowed range and have done damage, make the entity go boom! */
	if ( ent->s.apos.trBase[0] && iLen >= ent->s.apos.trBase[0] )
	{
	    gentity_t *te;	    
		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		
	    te = G_TempEntity (origin, EV_MISSILE_DIE);
	    VectorCopy (origin, te->s.origin);
	    AngleVectors (ent->s.angles, te->s.angles, NULL, NULL);
	    te->s.eFlags = ent->s.eFlags;
	    te->s.weapon = ent->s.weapon;
	    te->s.weaponVariation = ent->s.weaponVariation;
		
		G_FreeEntity (ent);
		
		return;
	}

	// check think function after bouncing
	G_RunThink( ent );
}


//=============================================================================



