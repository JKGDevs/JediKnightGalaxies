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

// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "ghoul2/G2.h"

#ifdef _GAME
#include "g_local.h" //ahahahahhahahaha@$!$!
#else
#include "../cgame/cg_local.h"
#endif

#define MAX_WEAPON_CHARGE_TIME 5000

// FIXME: remove extern, put in bg_local.h
extern qboolean BG_FullBodyTauntAnim( int anim );
// FIXME: move this func from bg_saber.cpp -> bg_pmove.cpp
extern float PM_WalkableGroundDistance(void);
extern qboolean PM_GroundSlideOkay( float zNormal );
// FIXME: remove extern, put in bg_local.h
extern saberInfo_t *BG_MySaber( int clientNum, int saberNum );

pmove_t		*pm;
pml_t		pml;

bgEntity_t *pm_entSelf = NULL;
bgEntity_t *pm_entVeh = NULL;

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_duckScale = 0.50f;
float	pm_swimScale = 0.50f;
float	pm_wadeScale = 0.70f;

float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 4.0f;
float	pm_flyaccelerate = 8.0f;

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;
#ifndef _MP_AIR_MOVEMENT
float pm_airDecelRate = 1.35f;	//Used for air decelleration away from current movement velocity
#endif

int		c_pmove = 0;

float forceSpeedLevels[4] = 
{
	1, //rank 0?
	1.25f,
	1.5f,
	1.75f
};

int forcePowerNeeded[NUM_FORCE_POWER_LEVELS][NUM_FORCE_POWERS] = 
{
	{ //nothing should be usable at rank 0..
		999,//FP_HEAL,//instant
		999,//FP_LEVITATION,//hold/duration
		999,//FP_SPEED,//duration
		999,//FP_PUSH,//hold/duration
		999,//FP_PULL,//hold/duration
		999,//FP_TELEPATHY,//instant
		999,//FP_GRIP,//hold/duration
		999,//FP_LIGHTNING,//hold/duration
		999,//FP_RAGE,//duration
		999,//FP_PROTECT,//duration
		999,//FP_ABSORB,//duration
		999,//FP_TEAM_HEAL,//instant
		999,//FP_TEAM_FORCE,//instant
		999,//FP_DRAIN,//hold/duration
		999,//FP_SEE,//duration
		999,//FP_SABER_OFFENSE,
		999,//FP_SABER_DEFENSE,
		999//FP_SABERTHROW,
		//NUM_FORCE_POWERS
	},
	{
		65,//FP_HEAL,//instant //was 25, but that was way too little
		10,//FP_LEVITATION,//hold/duration
		50,//FP_SPEED,//duration
		20,//FP_PUSH,//hold/duration
		20,//FP_PULL,//hold/duration
		20,//FP_TELEPATHY,//instant
		30,//FP_GRIP,//hold/duration
		1,//FP_LIGHTNING,//hold/duration
		50,//FP_RAGE,//duration
		50,//FP_PROTECT,//duration
		50,//FP_ABSORB,//duration
		50,//FP_TEAM_HEAL,//instant
		50,//FP_TEAM_FORCE,//instant
		20,//FP_DRAIN,//hold/duration
		20,//FP_SEE,//duration
		0,//FP_SABER_OFFENSE,
		2,//FP_SABER_DEFENSE,
		20//FP_SABERTHROW,
		//NUM_FORCE_POWERS
	},
	{
		60,//FP_HEAL,//instant
		10,//FP_LEVITATION,//hold/duration
		50,//FP_SPEED,//duration
		20,//FP_PUSH,//hold/duration
		20,//FP_PULL,//hold/duration
		20,//FP_TELEPATHY,//instant
		30,//FP_GRIP,//hold/duration
		1,//FP_LIGHTNING,//hold/duration
		50,//FP_RAGE,//duration
		25,//FP_PROTECT,//duration
		25,//FP_ABSORB,//duration
		33,//FP_TEAM_HEAL,//instant
		33,//FP_TEAM_FORCE,//instant
		20,//FP_DRAIN,//hold/duration
		20,//FP_SEE,//duration
		0,//FP_SABER_OFFENSE,
		1,//FP_SABER_DEFENSE,
		20//FP_SABERTHROW,
		//NUM_FORCE_POWERS
	},
	{
		50,//FP_HEAL,//instant //You get 5 points of health.. for 50 force points!
		10,//FP_LEVITATION,//hold/duration
		50,//FP_SPEED,//duration
		20,//FP_PUSH,//hold/duration
		20,//FP_PULL,//hold/duration
		20,//FP_TELEPATHY,//instant
		60,//FP_GRIP,//hold/duration
		1,//FP_LIGHTNING,//hold/duration
		50,//FP_RAGE,//duration
		10,//FP_PROTECT,//duration
		10,//FP_ABSORB,//duration
		25,//FP_TEAM_HEAL,//instant
		25,//FP_TEAM_FORCE,//instant
		20,//FP_DRAIN,//hold/duration
		20,//FP_SEE,//duration
		0,//FP_SABER_OFFENSE,
		0,//FP_SABER_DEFENSE,
		20//FP_SABERTHROW,
		//NUM_FORCE_POWERS
	}
};

float forceJumpHeight[NUM_FORCE_POWER_LEVELS] = 
{
	32,//normal jump (+stepheight+crouchdiff = 66)
	96,//(+stepheight+crouchdiff = 130)
	192,//(+stepheight+crouchdiff = 226)
	384//(+stepheight+crouchdiff = 418)
};

float forceJumpStrength[NUM_FORCE_POWER_LEVELS] = 
{
	JUMP_VELOCITY,//normal jump
	420,
	590,
	840
};

//rww - Get a pointer to the bgEntity by the index
// FIXME: belongs in bg_misc.cpp
bgEntity_t *PM_BGEntForNum( int num )
{
	bgEntity_t *ent;

	if (!pm)
	{
		assert(!"You cannot call PM_BGEntForNum outside of pm functions!");
		return NULL;
	}

	if (!pm->baseEnt)
	{
		assert(!"Base entity address not set");
		return NULL;
	}

	if (!pm->entSize)
	{
		assert(!"sizeof(ent) is 0, impossible (not set?)");
		return NULL;
	}

	assert(num >= 0 && num < MAX_GENTITIES);

    ent = (bgEntity_t *)((byte *)pm->baseEnt + pm->entSize*(num));

	return ent;
}

qboolean BG_KnockDownable(playerState_t *ps)
{
	if (!ps)
	{ //just for safety
		return qfalse;
	}

	if (ps->emplacedIndex)
	{ //using emplaced gun or eweb, can't be knocked down
		return qfalse;
	}

	//ok, I guess?
	return qtrue;
}

// Following couple things don't belong in the DLL namespace!
#ifdef _GAME
	#if !defined(MACOS_X) && !defined(__GCC__) && !defined(__GNUC__)
		typedef struct gentity_s gentity_t;
	#endif
#endif

static void PM_GroundTraceMissed( void );

/*
===============
PM_AddEvent
PM_AddEventWithParm

===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

void PM_AddEventWithParm( int newEvent, int parm ) 
{
	BG_AddPredictableEventToPlayerstate( newEvent, parm, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		return;
	}
	if ( pm->numtouch >= MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
==============
PM_ApplySpeedConstants

Adjusts movement speed based on values from constants.json
Only really used for WalkMove
==============
*/
void PM_ApplySpeedConstants()
{
	saberInfo_t	*saber;
	playerState_t* ps = pm->ps;
	usercmd_t* cmd = &pm->cmd;
	int svTime = pm->cmd.serverTime;
	float speedModifier = 1.0f;
	weaponData_t* weaponData = GetWeaponData(ps->weapon, ps->weaponVariation);

	//For prediction, always reset speed back to the last known server base speed
	//If we didn't do this, under lag we'd eventually dwindle speed down to 0 even though
	//that would not be the correct predicted value.
	ps->speed = ps->basespeed;

	if (ps->forceHandExtend == HANDEXTEND_DODGE)
	{
		ps->speed = 0;
	}

	if (ps->forceHandExtend == HANDEXTEND_KNOCKDOWN ||
		ps->forceHandExtend == HANDEXTEND_PRETHROWN ||
		ps->forceHandExtend == HANDEXTEND_POSTTHROWN)
	{
		ps->speed = 0;
	}

	for (int i = 0; i < MAX_ARMOR; i++) {
		int armor = ps->armor[i];
		if (armor--) {
			armorData_t* pArm = &armorTable[armor];
			speedModifier -= (1.0f - pArm->movemodifier);
		}
	}

	//check active buffs on player
	for (int i = 0; i < PLAYERBUFF_BITS; i++)
	{
		if (ps->buffsActive & (1 << i))
		{
			jkgBuff_t* pOtherBuff = &buffTable[ps->buffs[i].buffID];
			speedModifier -= (1.0f - pOtherBuff->passive.movemodifier_cur);
		}
	}

	if (cmd->forwardmove < 0 && !(cmd->buttons&BUTTON_WALKING) && pm->ps->groundEntityNum != ENTITYNUM_NONE)
	{//running backwards is slower than running forwards (like SP)
		speedModifier += bgConstants.backwardsSpeedModifier;
		if (ps->saberActionFlags & (1 << SAF_BLOCKING))
		{
			if (cmd->rightmove != 0)
			{
				speedModifier += bgConstants.backwardsDiagonalSpeedModifier;
			}
		}
	}

	if (cmd->rightmove != 0)
	{
		// strafing reduces speed significantly
		if (BG_IsSprinting(ps, cmd, true))
			speedModifier += bgConstants.strafeSpeedModifier / 4;
		else if (cmd->forwardmove != 0)
			speedModifier += bgConstants.strafeSpeedModifier / 1.5f;
		else
			speedModifier += bgConstants.strafeSpeedModifier;
	}

	if (!(cmd->buttons&BUTTON_WALKING) && !(cmd->buttons&BUTTON_IRONSIGHTS) && !BG_IsSprinting(ps, cmd, qtrue) && ps->weapon != WP_SABER)
	{
		speedModifier += bgConstants.baseSpeedModifier;
	}
	else if (cmd->buttons&BUTTON_WALKING || cmd->buttons&BUTTON_IRONSIGHTS)
	{
		speedModifier -= bgConstants.walkSpeedModifier;
	}

	if (ps->fd.forcePowersActive & (1 << FP_GRIP))
	{
		speedModifier -= 0.6f;
	}

	if (ps->fd.forcePowersActive & (1 << FP_SPEED))
	{
		speedModifier += 0.7f;
	}
	else if (ps->fd.forcePowersActive & (1 << FP_RAGE))
	{
		speedModifier += 0.3f;
	}
	else if (ps->fd.forceRageRecoveryTime > svTime)
	{
		speedModifier -= 0.25f;
	}

	if (ps->fd.forceGripCripple)
	{
		if (ps->fd.forcePowersActive & (1 << FP_RAGE))
		{
			speedModifier -= 0.1f;
		}
		else if (ps->fd.forcePowersActive & (1 << FP_SPEED))
		{ //force speed will help us escape
			speedModifier -= 0.2f;
		}
		else
		{
			speedModifier -= 0.8f;
		}
	}

	if (BG_SaberInAttack(ps->saberMove) && cmd->forwardmove < 0)
	{//if running backwards while attacking, don't run as fast.
		ps->speed *= SaberStances[ps->fd.saberAnimLevel].attackBackSpeedModifier;
	}
	else if (BG_SpinningSaberAnim(ps->legsAnim))
	{
		if (ps->fd.saberAnimLevel == FORCE_LEVEL_3)
		{
			speedModifier -= 0.7f;
		}
		else
		{
			speedModifier -= 0.5f;
		}
	}
	else if (ps->weapon == WP_SABER && BG_SaberInAttack(ps->saberMove))
	{//if attacking with saber while running, drop your speed
		speedModifier -= (1.0f - SaberStances[ps->fd.saberAnimLevel].attackSpeedModifier);
	}
	else if (ps->weapon == WP_SABER &&
		PM_SaberInTransition(ps->saberMove))
	{ //Now, we want to even slow down in transitions for level 3 (since it has chains and stuff now)
		if (cmd->forwardmove < 0)
		{
			speedModifier -= (1.0f - SaberStances[ps->fd.saberAnimLevel].attackSpeedModifier);
		}
		else
		{
			speedModifier -= (1.0f - SaberStances[ps->fd.saberAnimLevel].transitionSpeedModifier)*1.5;
		}
	}

	if (BG_InRoll(ps, ps->legsAnim) && ps->speed > 50)
	{ //can't roll unless you're able to move normally
		if ((ps->legsAnim) == BOTH_ROLL_B)
		{ //backwards roll is pretty fast, should also be slower
			if (ps->legsTimer > 800)
			{
				ps->speed = ps->legsTimer / 2.5f;
			}
			else
			{
				ps->speed = ps->legsTimer / 6.0f;//450;
			}
		}
		else
		{
			if (ps->legsTimer > 800)
			{
				ps->speed = ps->legsTimer / 1.5f;//450;
			}
			else
			{
				ps->speed = ps->legsTimer / 5.0f;//450;
			}
		}
		if (ps->speed > 600)
		{
			ps->speed = 600;
		}
		//Automatically slow down as the roll ends.

		if (ps->weaponstate == WEAPON_FIRING)
		{
			ps->weaponstate = WEAPON_ROLLFIRING;	//if we're trying to fire while rolling set our weapon state to spray and pray
		}
	}

	saber = BG_MySaber(ps->clientNum, 0);
	if (saber
		&& saber->moveSpeedScale != 1.0f)
	{
		ps->speed *= saber->moveSpeedScale;
	}
	saber = BG_MySaber(ps->clientNum, 1);
	if (saber
		&& saber->moveSpeedScale != 1.0f)
	{
		ps->speed *= saber->moveSpeedScale;
	}

	if (pm->ps->pm_flags & PMF_DUCKED) {
		speedModifier -= pm_duckScale;
	}

	speedModifier -= (1 - weaponData->speedModifier);
	if (ps->weaponstate == WEAPON_RELOADING && ps->weaponTime > 0) {
		speedModifier -= (1 - weaponData->reloadModifier);
	}

	if (speedModifier < bgConstants.minimumSpeedModifier)
	{
		speedModifier = bgConstants.minimumSpeedModifier;
	}

	ps->speed *= speedModifier;
}

/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	float	oldInZ;
	int		i;
	
	if ( (pm->ps->pm_flags&PMF_STUCK_TO_WALL) )
	{//no sliding!
		VectorCopy( in, out );
		return;
	}
	oldInZ = in[2];

	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
	if ( pm->stepSlideFix )
	{
		if ( pm->ps->clientNum < MAX_CLIENTS//normal player
			&& pm->ps->groundEntityNum != ENTITYNUM_NONE//on the ground
			&& normal[2] < MIN_WALK_NORMAL )//sliding against a steep slope
		{//if walking on the ground, don't slide up slopes that are too steep to walk on
			out[2] = oldInZ;
		}
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		if (pm->ps->pm_type == PM_SPECTATOR)
		{
			vel[2] = 0;
		}
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( (pm->ps->pm_type == PM_FREEZE || pm->ps->pm_type == PM_LOCK) || (
			pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) ) {
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR || pm->ps->pm_type == PM_FLOAT )
	{
		if (pm->ps->pm_type == PM_FLOAT)
		{ //almost no friction while floating
			drop += speed*0.1f*pml.frametime;
		}
		else
		{
			drop += speed*pm_spectatorfriction*pml.frametime;
		}
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel )
{
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
}

/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int		max;
	float	total;
	float	scale;
	int		umove = 0; //cmd->upmove;
			//don't factor upmove into scaling speed

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( umove ) > max ) {
		max = abs( umove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( (float)(cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + umove * umove) );
	scale = (float)pm->ps->speed * max / ( 127.0f * total );

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
	if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
		if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 0;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 2;
		} else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 3;
		} else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 4;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
			pm->ps->movementDir = 5;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
			pm->ps->movementDir = 6;
		} else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if ( pm->ps->movementDir == 2 ) {
			pm->ps->movementDir = 1;
		} else if ( pm->ps->movementDir == 6 ) {
			pm->ps->movementDir = 7;
		} 
	}
}

qboolean PM_ForceJumpingUp(void)
{
	if ( !(pm->ps->fd.forcePowersActive&(1<<FP_LEVITATION)) && pm->ps->fd.forceJumpCharge )
	{//already jumped and let go
		return qfalse;
	}

	if ( BG_InSpecialJump( pm->ps->legsAnim ) )
	{
		return qfalse;
	}

	if (BG_SaberInSpecial(pm->ps->saberMove))
	{
		return qfalse;
	}

	if (BG_SaberInSpecialAttack(pm->ps->legsAnim))
	{
		return qfalse;
	}

	if (!BG_CanUseFPNow(pm->gametype, pm->ps, pm->cmd.serverTime, FP_LEVITATION))
	{
		return qfalse;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE && //in air
		(pm->ps->pm_flags & PMF_JUMP_HELD) && //jumped
		pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 && //force-jump capable
		pm->ps->velocity[2] > 0 )//going up
	{
		return qtrue;
	}
	return qfalse;
}

static void PM_JumpForDir( void )
{
	int anim = BOTH_JUMP1;
	if ( pm->cmd.forwardmove > 0 ) 
	{
		anim = BOTH_JUMP1;
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} 
	else if ( pm->cmd.forwardmove < 0 )
	{
		anim = BOTH_JUMPBACK1;
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}
	else if ( pm->cmd.rightmove > 0 ) 
	{
		anim = BOTH_JUMPRIGHT1;
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	}
	else if ( pm->cmd.rightmove < 0 ) 
	{
		anim = BOTH_JUMPLEFT1;
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	}
	else
	{
		anim = BOTH_JUMP1;
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	}
	if(!BG_InDeathAnim(pm->ps->legsAnim))
	{
		PM_SetAnim(SETANIM_LEGS,anim,SETANIM_FLAG_OVERRIDE);
	}
}

void PM_SetPMViewAngle(playerState_t *ps, vec3_t angle, usercmd_t *ucmd)
{
	int			i;

	for (i=0 ; i<3 ; i++)
	{ // set the delta angle
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ps->delta_angles[i] = cmdAngle - ucmd->angles[i];
	}
	VectorCopy (angle, ps->viewangles);
}

qboolean PM_AdjustAngleForWallRun( playerState_t *ps, usercmd_t *ucmd, qboolean doMove )
{
	if (( (ps->legsAnim) == BOTH_WALL_RUN_RIGHT || (ps->legsAnim) == BOTH_WALL_RUN_LEFT ) && ps->legsTimer > 500 )
	{//wall-running and not at end of anim
		//stick to wall, if there is one
		vec3_t	fwd, rt, traceTo, mins, maxs, fwdAngles;
		trace_t	trace;
		float	dist, yawAdjust;

		VectorSet(mins, -15, -15, 0);
		VectorSet(maxs, 15, 15, 24);
		VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

		AngleVectors( fwdAngles, fwd, rt, NULL );
		if ( (ps->legsAnim) == BOTH_WALL_RUN_RIGHT )
		{
			dist = 128;
			yawAdjust = -90;
		}
		else
		{
			dist = -128;
			yawAdjust = 90;
		}
		VectorMA( ps->origin, dist, rt, traceTo );
		
		pm->trace( &trace, ps->origin, mins, maxs, traceTo, ps->clientNum, MASK_PLAYERSOLID );

		if ( trace.fraction < 1.0f 
			&& (trace.plane.normal[2] >= 0.0f && trace.plane.normal[2] <= 0.4f) )//&& ent->client->ps.groundEntityNum == ENTITYNUM_NONE )
		{
			trace_t	trace2;
			vec3_t traceTo2;
			vec3_t	wallRunFwd, wallRunAngles;
			
			VectorClear( wallRunAngles );
			wallRunAngles[YAW] = vectoyaw( trace.plane.normal )+yawAdjust;
			AngleVectors( wallRunAngles, wallRunFwd, NULL, NULL );

			VectorMA( pm->ps->origin, 32, wallRunFwd, traceTo2 );
			pm->trace( &trace2, pm->ps->origin, mins, maxs, traceTo2, pm->ps->clientNum, MASK_PLAYERSOLID );
			if ( trace2.fraction < 1.0f && DotProduct( trace2.plane.normal, wallRunFwd ) <= -0.999f )
			{//wall we can't run on in front of us
				trace.fraction = 1.0f;//just a way to get it to kick us off the wall below
			}
		} 

		if ( trace.fraction < 1.0f 
			&& (trace.plane.normal[2] >= 0.0f&&trace.plane.normal[2] <= 0.4f/*MAX_WALL_RUN_Z_NORMAL*/) )
		{//still a wall there
			if ( (ps->legsAnim) == BOTH_WALL_RUN_RIGHT )
			{
				ucmd->rightmove = 127;
			}
			else
			{
				ucmd->rightmove = -127;
			}
			if ( ucmd->upmove < 0 )
			{
				ucmd->upmove = 0;
			}
			//make me face perpendicular to the wall
			ps->viewangles[YAW] = vectoyaw( trace.plane.normal )+yawAdjust;

			PM_SetPMViewAngle(ps, ps->viewangles, ucmd);

			ucmd->angles[YAW] = ANGLE2SHORT( ps->viewangles[YAW] ) - ps->delta_angles[YAW];
			if ( doMove )
			{
				//push me forward
				float	zVel = ps->velocity[2];
				if ( ps->legsTimer > 500 )
				{//not at end of anim yet
					float speed = 125;
					if ( ucmd->forwardmove < 0 )
					{//slower
						speed = 100;
					}
					else if ( ucmd->forwardmove > 0 )
					{
						speed = 250;//running speed
					}
					VectorScale( fwd, speed, ps->velocity );
				}
				ps->velocity[2] = zVel;//preserve z velocity
				//pull me toward the wall, too
				VectorMA( ps->velocity, dist, rt, ps->velocity );
			}
			ucmd->forwardmove = 0;
			return qtrue;
		}
		else if ( doMove )
		{//stop it
			if ( (ps->legsAnim) == BOTH_WALL_RUN_RIGHT )
			{
				PM_SetAnim(SETANIM_BOTH, BOTH_WALL_RUN_RIGHT_STOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else if ( (ps->legsAnim) == BOTH_WALL_RUN_LEFT )
			{
				PM_SetAnim(SETANIM_BOTH, BOTH_WALL_RUN_LEFT_STOP, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
		}
	}

	return qfalse;
}

qboolean PM_AdjustAnglesForWallRunUpFlipAlt( usercmd_t *ucmd )
{
//	ucmd->angles[PITCH] = ANGLE2SHORT( pm->ps->viewangles[PITCH] ) - pm->ps->delta_angles[PITCH];
//	ucmd->angles[YAW] = ANGLE2SHORT( pm->ps->viewangles[YAW] ) - pm->ps->delta_angles[YAW];
	PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, ucmd);
	return qtrue;
}

qboolean PM_AdjustAngleForWallRunUp( playerState_t *ps, usercmd_t *ucmd, qboolean doMove )
{
	if ( ps->legsAnim == BOTH_FORCEWALLRUNFLIP_START )
	{//wall-running up
		//stick to wall, if there is one
		vec3_t	fwd, traceTo, mins, maxs, fwdAngles;
		trace_t	trace;
		float	dist = 128;

		VectorSet(mins, -15,-15,0);
		VectorSet(maxs, 15,15,24);
		VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

		AngleVectors( fwdAngles, fwd, NULL, NULL );
		VectorMA( ps->origin, dist, fwd, traceTo );
		pm->trace( &trace, ps->origin, mins, maxs, traceTo, ps->clientNum, MASK_PLAYERSOLID );
		if ( trace.fraction > 0.5f )
		{//hmm, some room, see if there's a floor right here
			trace_t	trace2;
			vec3_t	top, bottom;

			VectorCopy( trace.endpos, top );
			top[2] += (pm->mins[2]*-1) + 4.0f;
			VectorCopy( top, bottom );
			bottom[2] -= 64.0f;
			pm->trace( &trace2, top, pm->mins, pm->maxs, bottom, ps->clientNum, MASK_PLAYERSOLID );
			if ( !trace2.allsolid 
				&& !trace2.startsolid 
				&& trace2.fraction < 1.0f 
				&& trace2.plane.normal[2] > 0.7f )//slope we can stand on
			{//cool, do the alt-flip and land on whetever it is we just scaled up
				VectorScale( fwd, 100, pm->ps->velocity );
				pm->ps->velocity[2] += 400;
				PM_SetAnim(SETANIM_BOTH, BOTH_FORCEWALLRUNFLIP_ALT, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				pm->ps->pm_flags |= PMF_JUMP_HELD;
				PM_AddEvent(EV_JUMP);
				ucmd->upmove = 0;
				return qfalse;
			}
		}

		if (ps->legsTimer > 0 &&
			ucmd->forwardmove > 0 &&
			trace.fraction < 1.0f && 
			(trace.plane.normal[2] >= 0.0f&&trace.plane.normal[2]<=0.4f/*MAX_WALL_RUN_Z_NORMAL*/) )
		{//still a vertical wall there
			//make sure there's not a ceiling above us!
			trace_t	trace2;
			VectorCopy( ps->origin, traceTo );
			traceTo[2] += 64;
			pm->trace( &trace2, ps->origin, mins, maxs, traceTo, ps->clientNum, MASK_PLAYERSOLID );

			if (trace2.fraction >= 1.0f)
			{//all clear, keep going
				//FIXME: don't pull around 90 turns
				//FIXME: simulate stepping up steps here, somehow?
				ucmd->forwardmove = 127;
				if ( ucmd->upmove < 0 )
				{
					ucmd->upmove = 0;
				}
				//make me face the wall
				ps->viewangles[YAW] = vectoyaw( trace.plane.normal )+180;
				PM_SetPMViewAngle(ps, ps->viewangles, ucmd);

				ucmd->angles[YAW] = ANGLE2SHORT( ps->viewangles[YAW] ) - ps->delta_angles[YAW];
				if ( doMove )
				{
					//pull me toward the wall
					VectorScale( trace.plane.normal, -dist*trace.fraction, ps->velocity );
					//push me up
					if ( ps->legsTimer > 200 )
					{//not at end of anim yet
						ps->velocity[2] = 300.0f;//preserve z velocity
					}
				}
				ucmd->forwardmove = 0;
				return qtrue;
			}
		}
		//failed!
		if ( doMove )
		{//stop it
			VectorScale( fwd, -300.0f, ps->velocity );
			ps->velocity[2] += 200;

			PM_SetAnim(SETANIM_BOTH, BOTH_FORCEWALLRUNFLIP_END, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			ps->pm_flags |= PMF_JUMP_HELD;

			//FIXME do I need this in mp?
			PM_AddEvent(EV_JUMP);
			ucmd->upmove = 0;
		}
	}
	return qfalse;
}

#define	JUMP_OFF_WALL_SPEED	200.0f
//nice...
static float BG_ForceWallJumpStrength( void )
{
	return (forceJumpStrength[FORCE_LEVEL_3]/2.5f);
}

qboolean PM_AdjustAngleForWallJump( playerState_t *ps, usercmd_t *ucmd, qboolean doMove )
{
	if ( ( ( BG_InReboundJump( ps->legsAnim ) || BG_InReboundHold( ps->legsAnim ) )
			&& ( BG_InReboundJump( ps->torsoAnim ) || BG_InReboundHold( ps->torsoAnim ) ) )
		|| (pm->ps->pm_flags&PMF_STUCK_TO_WALL) )
	{//hugging wall, getting ready to jump off
		//stick to wall, if there is one
		vec3_t	checkDir, traceTo, mins, maxs, fwdAngles;
		trace_t	trace;
		float	dist = 128.0f, yawAdjust;

		VectorSet(mins, pm->mins[0],pm->mins[1],0);
		VectorSet(maxs, pm->maxs[0],pm->maxs[1],24);
		VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

		switch ( ps->legsAnim )
		{
		case BOTH_FORCEWALLREBOUND_RIGHT:
		case BOTH_FORCEWALLHOLD_RIGHT:
			AngleVectors( fwdAngles, NULL, checkDir, NULL );
			yawAdjust = -90;
			break;
		case BOTH_FORCEWALLREBOUND_LEFT:
		case BOTH_FORCEWALLHOLD_LEFT:
			AngleVectors( fwdAngles, NULL, checkDir, NULL );
			VectorScale( checkDir, -1, checkDir );
			yawAdjust = 90;
			break;
		case BOTH_FORCEWALLREBOUND_FORWARD:
		case BOTH_FORCEWALLHOLD_FORWARD:
			AngleVectors( fwdAngles, checkDir, NULL, NULL );
			yawAdjust = 180;
			break;
		case BOTH_FORCEWALLREBOUND_BACK:
		case BOTH_FORCEWALLHOLD_BACK:
			AngleVectors( fwdAngles, checkDir, NULL, NULL );
			VectorScale( checkDir, -1, checkDir );
			yawAdjust = 0;
			break;
		default:
			//WTF???
			pm->ps->pm_flags &= ~PMF_STUCK_TO_WALL;
			return qfalse;
			break;
		}
		if ( pm->debugMelee )
		{//uber-skillz
			if ( ucmd->upmove > 0 )
			{//hold on until you let go manually
				if ( BG_InReboundHold( ps->legsAnim ) )
				{//keep holding
					if ( ps->legsTimer < 150 )
					{
						ps->legsTimer = 150;
					}
				}
				else
				{//if got to hold part of anim, play hold anim
					if ( ps->legsTimer <= 300 )
					{
						ps->saberHolstered = 2;
						PM_SetAnim( SETANIM_BOTH, BOTH_FORCEWALLRELEASE_FORWARD+(ps->legsAnim-BOTH_FORCEWALLHOLD_FORWARD), SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
						ps->legsTimer = ps->torsoTimer = 150;
					}
				}
			}
		}
		VectorMA( ps->origin, dist, checkDir, traceTo );
		pm->trace( &trace, ps->origin, mins, maxs, traceTo, ps->clientNum, MASK_PLAYERSOLID );
		if ( //ucmd->upmove <= 0 && 
			ps->legsTimer > 100 &&
			trace.fraction < 1.0f && 
			fabs(trace.plane.normal[2]) <= 0.2f/*MAX_WALL_GRAB_SLOPE*/ )
		{//still a vertical wall there
			//FIXME: don't pull around 90 turns

			if ( ucmd->upmove < 0 )
			{
				ucmd->upmove = 0;
			}
			//align me to the wall
			ps->viewangles[YAW] = vectoyaw( trace.plane.normal )+yawAdjust;
			PM_SetPMViewAngle(ps, ps->viewangles, ucmd);

			ucmd->angles[YAW] = ANGLE2SHORT( ps->viewangles[YAW] ) - ps->delta_angles[YAW];
			if ( doMove )
			{
				//pull me toward the wall
				VectorScale( trace.plane.normal, -128.0f, ps->velocity );
			}
			ucmd->upmove = 0;
			ps->pm_flags |= PMF_STUCK_TO_WALL;
			return qtrue;
		}
		else if ( doMove 
			&& (ps->pm_flags&PMF_STUCK_TO_WALL))
		{//jump off
			//push off of it!
			ps->pm_flags &= ~PMF_STUCK_TO_WALL;
			ps->velocity[0] = ps->velocity[1] = 0;
			VectorScale( checkDir, -JUMP_OFF_WALL_SPEED, ps->velocity );
			ps->velocity[2] = BG_ForceWallJumpStrength();
			ps->pm_flags |= PMF_JUMP_HELD;
			ps->fd.forceJumpSound = 1; //this is a stupid thing, i should fix it.

			if (ps->origin[2] < ps->fd.forceJumpZStart)
			{
				ps->fd.forceJumpZStart = ps->origin[2];
			}
			//FIXME do I need this?

			BG_ForcePowerDrain( ps, FP_LEVITATION, 10 );
			//no control for half a second
			ps->pm_flags |= PMF_TIME_KNOCKBACK;
			ps->pm_time = 500;
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
			ucmd->upmove = 127;

			if ( BG_InReboundHold( ps->legsAnim ) )
			{//if was in hold pose, release now
				PM_SetAnim( SETANIM_BOTH, BOTH_FORCEWALLRELEASE_FORWARD+(ps->legsAnim-BOTH_FORCEWALLHOLD_FORWARD), SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else
			{
				PM_SetAnim(SETANIM_LEGS,BOTH_FORCEJUMP1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_RESTART);
			}
		}
	}
	ps->pm_flags &= ~PMF_STUCK_TO_WALL;
	return qfalse;
}

//[KnockdownSys]
//[SPPortCompete]
extern qboolean PM_InForceGetUp( playerState_t *ps );
int PM_MinGetUpTime( playerState_t *ps );
qboolean PM_AdjustAnglesForKnockdown( playerState_t *ps, usercmd_t *ucmd )
{//racc - adjusts the move cmd and angles for knockdown moves.
	if ( PM_InKnockDown( ps ) )
	{//being knocked down or getting up, can't do anything!
		if ( !PM_InForceGetUp( ps ) && (ps->legsTimer > PM_MinGetUpTime( ps ) 
			//KNOCKDOWNFIXME RAFIXME - impliment G_ControlledByPlayer?
			|| ps->clientNum >= MAX_CLIENTS
			//racc - don't move while using the non-force powered getups
			|| ps->legsAnim == BOTH_GETUP1
			|| ps->legsAnim == BOTH_GETUP2
			|| ps->legsAnim == BOTH_GETUP3
			|| ps->legsAnim == BOTH_GETUP4
			|| ps->legsAnim == BOTH_GETUP5) )
			//|| (ent->s.number >= MAX_CLIENTS&&!G_ControlledByPlayer(ent)) )
		{//can't get up yet
			ucmd->forwardmove = 0;
			ucmd->rightmove = 0;
		}
		
		if(ps->clientNum >= MAX_CLIENTS)
		{
			VectorClear( ps->moveDir );
		}
		//you can jump up out of a knockdown and you get get up into a crouch from a knockdown

		//allow players to continue to move unless they're actually on lying on the ground.
		if ( !PM_InForceGetUp( ps ) && BG_GetTorsoAnimPoint(ps, pm_entSelf->localAnimIndex) < .9f )
		{//can't turn unless in a force getup
			PM_SetPMViewAngle(ps, ps->viewangles, ucmd);
			ucmd->angles[PITCH] = ANGLE2SHORT( ps->viewangles[PITCH] ) - ps->delta_angles[PITCH];
			ucmd->angles[YAW] = ANGLE2SHORT( ps->viewangles[YAW] ) - ps->delta_angles[YAW];
			return qtrue;
		}
	}
	return qfalse;
}

//Set the height for when a force jump was started. If it's 0, nuge it up (slight hack to prevent holding jump over slopes)
void PM_SetForceJumpZStart(float value)
{
	pm->ps->fd.forceJumpZStart = value;
	if (!pm->ps->fd.forceJumpZStart)
	{
		pm->ps->fd.forceJumpZStart -= 0.1f;
	}
}

float forceJumpHeightMax[NUM_FORCE_POWER_LEVELS] = 
{
	66,//normal jump (32+stepheight(18)+crouchdiff(24) = 74)
	130,//(96+stepheight(18)+crouchdiff(24) = 138)
	226,//(192+stepheight(18)+crouchdiff(24) = 234)
	418//(384+stepheight(18)+crouchdiff(24) = 426)
};


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) 
{
	qboolean allowFlips = qtrue;
	
	if ( BG_IsSprinting (pm->ps, &pm->cmd, qtrue) )
	{
	    return qfalse;
	}

	if (pm->ps->clientNum >= MAX_CLIENTS)
	{
		bgEntity_t *pEnt = pm_entSelf;

		if (pEnt->s.eType == ET_NPC &&
			pEnt->s.NPC_class == CLASS_VEHICLE)
		{ //no!
			return qfalse;
		}
	}

	if (pm->ps->forceHandExtend == HANDEXTEND_KNOCKDOWN ||
		pm->ps->forceHandExtend == HANDEXTEND_PRETHROWN ||
		pm->ps->forceHandExtend == HANDEXTEND_POSTTHROWN)
	{
		return qfalse;
	}

	if(PM_InKnockDown(pm->ps))
	{
		return qfalse;
	}

	if (pm->ps->pm_type == PM_JETPACK)
	{ //there's no actual jumping while we jetpack
		return qfalse;
	}

	//Don't allow jump until all buttons are up
	if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return qfalse;		
	}

	if ( BG_InRoll( pm->ps, pm->ps->legsAnim ) ) 
	{//in knockdown
		return qfalse;		
	}

	// Don't allow jumping if we don't have the stamina for it
	if (pm->ps->forcePower < bgConstants.staminaDrains.minJumpThreshold) {
		return qfalse;
	}

	if ( pm->ps->weapon == WP_SABER )
	{
		saberInfo_t *saber1 = BG_MySaber( pm->ps->clientNum, 0 );
		saberInfo_t *saber2 = BG_MySaber( pm->ps->clientNum, 1 );
		if ( saber1
			&& (saber1->saberFlags&SFL_NO_FLIPS) )
		{
			allowFlips = qfalse;
		}
		if ( saber2
			&& (saber2->saberFlags&SFL_NO_FLIPS) )
		{
			allowFlips = qfalse;
		}
	}

	if (pm->ps->groundEntityNum != ENTITYNUM_NONE || pm->ps->origin[2] < pm->ps->fd.forceJumpZStart)
	{
		pm->ps->fd.forcePowersActive &= ~(1<<FP_LEVITATION);
	}

	if (pm->ps->fd.forcePowersActive & (1 << FP_LEVITATION))
	{ //Force jump is already active.. continue draining power appropriately until we land.
		if (pm->ps->fd.forcePowerDebounce[FP_LEVITATION] < pm->cmd.serverTime)
		{
			if ( pm->gametype == GT_DUEL 
				|| pm->gametype == GT_POWERDUEL )
			{//jump takes less power
				BG_ForcePowerDrain( pm->ps, FP_LEVITATION, 1 );
			}
			else
			{
				BG_ForcePowerDrain( pm->ps, FP_LEVITATION, 5 );
			}
			if (pm->ps->fd.forcePowerLevel[FP_LEVITATION] >= FORCE_LEVEL_2)
			{
				pm->ps->fd.forcePowerDebounce[FP_LEVITATION] = pm->cmd.serverTime + 300;
			}
			else
			{
				pm->ps->fd.forcePowerDebounce[FP_LEVITATION] = pm->cmd.serverTime + 200;
			}
		}
	}

	if (pm->ps->forceJumpFlip)
	{ //Forced jump anim
		int anim = BOTH_FORCEINAIR1;
		int	parts = SETANIM_BOTH;
		if ( allowFlips )
		{
			if ( pm->cmd.forwardmove > 0 )
			{
				anim = BOTH_FLIP_F;
			}
			else if ( pm->cmd.forwardmove < 0 )
			{
				anim = BOTH_FLIP_B;
			}
			else if ( pm->cmd.rightmove > 0 )
			{
				anim = BOTH_FLIP_R;
			}
			else if ( pm->cmd.rightmove < 0 )
			{
				anim = BOTH_FLIP_L;
			}
		}
		else
		{
			if ( pm->cmd.forwardmove > 0 )
			{
				anim = BOTH_FORCEINAIR1;
			}
			else if ( pm->cmd.forwardmove < 0 )
			{
				anim = BOTH_FORCEINAIRBACK1;
			}
			else if ( pm->cmd.rightmove > 0 )
			{
				anim = BOTH_FORCEINAIRRIGHT1;
			}
			else if ( pm->cmd.rightmove < 0 )
			{
				anim = BOTH_FORCEINAIRLEFT1;
			}
		}
		if ( pm->ps->weaponTime <= 0 )
		{//FIXME: really only care if we're in a saber attack anim...
			parts = SETANIM_LEGS;
		}

		PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		pm->ps->forceJumpFlip = qfalse;
		return qtrue;
	}
#if METROID_JUMP
	if ( pm->waterlevel < 2 ) 
	{
		if ( pm->gametype == GT_DUEL )
		{
		if ( pm->ps->gravity > 0 )
		{//can't do this in zero-G
			if ( PM_ForceJumpingUp() )
			{//holding jump in air
				float curHeight = pm->ps->origin[2] - pm->ps->fd.forceJumpZStart;
				//check for max force jump level and cap off & cut z vel
				if ( ( curHeight<=forceJumpHeight[0] ||//still below minimum jump height
						(pm->ps->forcePower && pm->cmd.upmove>=10) ) &&////still have force power available and still trying to jump up 
					curHeight < forceJumpHeight[pm->ps->fd.forcePowerLevel[FP_LEVITATION]] &&
					pm->ps->fd.forceJumpZStart)//still below maximum jump height
				{//can still go up
					if ( curHeight > forceJumpHeight[0] )
					{//passed normal jump height  *2?
						if ( !(pm->ps->fd.forcePowersActive&(1<<FP_LEVITATION)) )//haven't started forcejump yet
						{
							//start force jump
							pm->ps->fd.forcePowersActive |= (1<<FP_LEVITATION);
							pm->ps->fd.forceJumpSound = 1;
							//play flip
							if ((pm->cmd.forwardmove || pm->cmd.rightmove) && //pushing in a dir
								(pm->ps->legsAnim) != BOTH_FLIP_F &&//not already flipping
								(pm->ps->legsAnim) != BOTH_FLIP_B &&
								(pm->ps->legsAnim) != BOTH_FLIP_R &&
								(pm->ps->legsAnim) != BOTH_FLIP_L 
								&& allowFlips )
							{ 
								int anim = BOTH_FORCEINAIR1;
								int	parts = SETANIM_BOTH;

								if ( pm->cmd.forwardmove > 0 )
								{
									anim = BOTH_FLIP_F;
								}
								else if ( pm->cmd.forwardmove < 0 )
								{
									anim = BOTH_FLIP_B;
								}
								else if ( pm->cmd.rightmove > 0 )
								{
									anim = BOTH_FLIP_R;
								}
								else if ( pm->cmd.rightmove < 0 )
								{
									anim = BOTH_FLIP_L;
								}
								if ( pm->ps->weaponTime <= 0 )
								{
									parts = SETANIM_LEGS;
								}

								PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
							}
							else if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_1 )
							{
								vec3_t facingFwd, facingRight, facingAngles;
								int	anim = -1;
								float dotR, dotF;
								
								VectorSet(facingAngles, 0, pm->ps->viewangles[YAW], 0);

								AngleVectors( facingAngles, facingFwd, facingRight, NULL );
								dotR = DotProduct( facingRight, pm->ps->velocity );
								dotF = DotProduct( facingFwd, pm->ps->velocity );

								if ( fabs(dotR) > fabs(dotF) * 1.5f )
								{
									if ( dotR > 150 )
									{
										anim = BOTH_FORCEJUMPRIGHT1;
									}
									else if ( dotR < -150 )
									{
										anim = BOTH_FORCEJUMPLEFT1;
									}
								}
								else
								{
									if ( dotF > 150 )
									{
										anim = BOTH_FORCEJUMP1;
									}
									else if ( dotF < -150 )
									{
										anim = BOTH_FORCEJUMPBACK1;
									}
								}
								if ( anim != -1 )
								{
									int parts = SETANIM_BOTH;
									if ( pm->ps->weaponTime <= 0 )
									{//FIXME: really only care if we're in a saber attack anim...
										parts = SETANIM_LEGS;
									}

									PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
								}
							}
						}
						else
						{ //jump is already active (the anim has started)
							if ( pm->ps->legsTimer < 1 )
							{//not in the middle of a legsAnim
								int anim = (pm->ps->legsAnim);
								int newAnim = -1;
								switch ( anim )
								{
								case BOTH_FORCEJUMP1:
									newAnim = BOTH_FORCELAND1;//BOTH_FORCEINAIR1;
									break;
								case BOTH_FORCEJUMPBACK1:
									newAnim = BOTH_FORCELANDBACK1;//BOTH_FORCEINAIRBACK1;
									break;
								case BOTH_FORCEJUMPLEFT1:
									newAnim = BOTH_FORCELANDLEFT1;//BOTH_FORCEINAIRLEFT1;
									break;
								case BOTH_FORCEJUMPRIGHT1:
									newAnim = BOTH_FORCELANDRIGHT1;//BOTH_FORCEINAIRRIGHT1;
									break;
								}
								if ( newAnim != -1 )
								{
									int parts = SETANIM_BOTH;
									if ( pm->ps->weaponTime <= 0 )
									{
										parts = SETANIM_LEGS;
									}

									PM_SetAnim( parts, newAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
								}
							}
						}
					}

					//need to scale this down, start with height velocity (based on max force jump height) and scale down to regular jump vel
					if(pm->ps->fd.forcePowerLevel[FP_LEVITATION] > 0)
					{
						pm->ps->velocity[2] = (forceJumpHeight[pm->ps->fd.forcePowerLevel[FP_LEVITATION]]-curHeight)/forceJumpHeight[pm->ps->fd.forcePowerLevel[FP_LEVITATION]]*forceJumpStrength[pm->ps->fd.forcePowerLevel[FP_LEVITATION]];//JUMP_VELOCITY;
					}
					else
					{
						pm->ps->velocity[2] = (bgConstants.baseJumpHeight-curHeight)/(bgConstants.baseJumpHeight*bgConstants.baseJumpVelocity);
					}
					pm->ps->velocity[2] /= 10;
					//pm->ps->velocity[2] += JUMP_VELOCITY;
					pm->ps->velocity[2] += bgConstants.baseJumpTapVelocity;
					pm->ps->pm_flags |= PMF_JUMP_HELD;
				}
				else if ( curHeight > bgConstants.baseJumpTapHeight && curHeight < forceJumpHeight[pm->ps->fd.forcePowerLevel[FP_LEVITATION]] - bgConstants.baseJumpTapHeight )
				{//still have some headroom, don't totally stop it
					if ( pm->ps->velocity[2] > /*JUMP_VELOCITY*/ bgConstants.baseJumpTapVelocity )
					{
						pm->ps->velocity[2] = /*JUMP_VELOCITY*/ bgConstants.baseJumpTapVelocity;
					}
				}
				else
				{
					//pm->ps->velocity[2] = 0;
					//rww - changed for the sake of balance in multiplayer

					if ( pm->ps->velocity[2] > /*JUMP_VELOCITY*/ bgConstants.baseJumpTapHeight )
					{
						pm->ps->velocity[2] = /*JUMP_VELOCITY*/ bgConstants.baseJumpTapHeight;
					}
				}
				pm->cmd.upmove = 0;
				return qfalse;
			}
		}
		}
	}

#endif

	//Not jumping
	if ( pm->cmd.upmove < 10 && pm->ps->groundEntityNum != ENTITYNUM_NONE) {
		return qfalse;
	}

	// must wait for jump to be released
	if ( pm->ps->pm_flags & PMF_JUMP_HELD ) 
	{
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	if ( pm->ps->gravity <= 0 )
	{//in low grav, you push in the dir you're facing as long as there is something behind you to shove off of
		vec3_t	forward, back;
		trace_t	trace;

		AngleVectors( pm->ps->viewangles, forward, NULL, NULL );
		VectorMA( pm->ps->origin, -8, forward, back );
		pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, back, pm->ps->clientNum, pm->tracemask );

		if ( trace.fraction <= 1.0f )
		{
			VectorMA( pm->ps->velocity, bgConstants.baseJumpVelocity*2, forward, pm->ps->velocity );
			PM_SetAnim(SETANIM_LEGS,BOTH_FORCEJUMP1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_RESTART);
		}//else no surf close enough to push off of
		pm->cmd.upmove = 0;
	}
	else if ( pm->cmd.upmove > 0 && pm->waterlevel < 2 &&
		pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 &&
		!(pm->ps->pm_flags&PMF_JUMP_HELD) &&
		(pm->ps->weapon == WP_SABER || pm->ps->weapon == WP_MELEE) &&
		BG_CanUseFPNow(pm->gametype, pm->ps, pm->cmd.serverTime, FP_LEVITATION) )
	{
		qboolean allowWallRuns = qtrue;
		qboolean allowWallFlips = qtrue;
		allowFlips = qtrue;
		qboolean allowWallGrabs = qtrue;
		if ( pm->ps->weapon == WP_SABER )
		{
			saberInfo_t *saber1 = BG_MySaber( pm->ps->clientNum, 0 );
			saberInfo_t *saber2 = BG_MySaber( pm->ps->clientNum, 1 );
			if ( saber1
				&& (saber1->saberFlags&SFL_NO_WALL_RUNS) )
			{
				allowWallRuns = qfalse;
			}
			if ( saber2
				&& (saber2->saberFlags&SFL_NO_WALL_RUNS) )
			{
				allowWallRuns = qfalse;
			}
			if ( saber1
				&& (saber1->saberFlags&SFL_NO_WALL_FLIPS) )
			{
				allowWallFlips = qfalse;
			}
			if ( saber2
				&& (saber2->saberFlags&SFL_NO_WALL_FLIPS) )
			{
				allowWallFlips = qfalse;
			}
			if ( saber1
				&& (saber1->saberFlags&SFL_NO_FLIPS) )
			{
				allowFlips = qfalse;
			}
			if ( saber2
				&& (saber2->saberFlags&SFL_NO_FLIPS) )
			{
				allowFlips = qfalse;
			}
			if ( saber1
				&& (saber1->saberFlags&SFL_NO_WALL_GRAB) )
			{
				allowWallGrabs = qfalse;
			}
			if ( saber2
				&& (saber2->saberFlags&SFL_NO_WALL_GRAB) )
			{
				allowWallGrabs = qfalse;
			}
		}

		if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
		{//on the ground
			//check for left-wall and right-wall special jumps
			int anim = -1;
			float	vertPush = 0;
			if ( pm->cmd.rightmove > 0 && pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_1 )
			{//strafing right
				if ( pm->cmd.forwardmove > 0 )
				{//wall-run
					if ( allowWallRuns )
					{
						vertPush = forceJumpStrength[FORCE_LEVEL_2]/2.0f;
						anim = BOTH_WALL_RUN_RIGHT;
					}
				}
				else if ( pm->cmd.forwardmove == 0 )
				{//wall-flip
					if ( allowWallFlips )
					{
						vertPush = forceJumpStrength[FORCE_LEVEL_2]/2.25f;
						anim = BOTH_WALL_FLIP_RIGHT;
					}
				}
			}
			else if ( pm->cmd.rightmove < 0 && pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_1 )
			{//strafing left
				if ( pm->cmd.forwardmove > 0 )
				{//wall-run
					if ( allowWallRuns )
					{
						vertPush = forceJumpStrength[FORCE_LEVEL_2]/2.0f;
						anim = BOTH_WALL_RUN_LEFT;
					}
				}
				else if ( pm->cmd.forwardmove == 0 )
				{//wall-flip
					if ( allowWallFlips )
					{
						vertPush = forceJumpStrength[FORCE_LEVEL_2]/2.25f;
						anim = BOTH_WALL_FLIP_LEFT;
					}
				}
			}
			else if ( pm->cmd.forwardmove < 0 && !(pm->cmd.buttons&BUTTON_ATTACK) )
			{//backflip
				if ( allowFlips )
				{
					vertPush = bgConstants.baseJumpVelocity;
					anim = BOTH_FLIP_BACK1;
				}
			}

			vertPush += 128; //give them an extra shove

			if ( anim != -1 )
			{
				vec3_t fwd, right, traceto, mins, maxs, fwdAngles;
				vec3_t	idealNormal={0}, wallNormal={0};
				trace_t	trace;
				qboolean doTrace = qfalse;
				int contents = MASK_SOLID;

				VectorSet(mins, pm->mins[0],pm->mins[1],0);
				VectorSet(maxs, pm->maxs[0],pm->maxs[1],24);
				VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

				memset(&trace, 0, sizeof(trace)); //to shut the compiler up

				AngleVectors( fwdAngles, fwd, right, NULL );

				//trace-check for a wall, if necc.
				switch ( anim )
				{
				case BOTH_WALL_FLIP_LEFT:
					//NOTE: purposely falls through to next case!
				case BOTH_WALL_RUN_LEFT:
					doTrace = qtrue;
					VectorMA( pm->ps->origin, -16, right, traceto );
					break;

				case BOTH_WALL_FLIP_RIGHT:
					//NOTE: purposely falls through to next case!
				case BOTH_WALL_RUN_RIGHT:
					doTrace = qtrue;
					VectorMA( pm->ps->origin, 16, right, traceto );
					break;

				case BOTH_WALL_FLIP_BACK1:
					doTrace = qtrue;
					VectorMA( pm->ps->origin, 16, fwd, traceto );
					break;
				}

				if ( doTrace )
				{
					pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, contents );
					VectorCopy( trace.plane.normal, wallNormal );
					VectorNormalize( wallNormal );
					VectorSubtract( pm->ps->origin, traceto, idealNormal );
					VectorNormalize( idealNormal );
				}

				if ( !doTrace || (trace.fraction < 1.0f && (trace.entityNum < MAX_CLIENTS || DotProduct(wallNormal,idealNormal) > 0.7f)) )
				{//there is a wall there.. or hit a client
					if ( (anim != BOTH_WALL_RUN_LEFT 
							&& anim != BOTH_WALL_RUN_RIGHT
							&& anim != BOTH_FORCEWALLRUNFLIP_START) 
						|| (wallNormal[2] >= 0.0f&&wallNormal[2]<=0.4f/*MAX_WALL_RUN_Z_NORMAL*/) )
					{//wall-runs can only run on perfectly flat walls, sorry. 
						int parts;
						//move me to side
						if ( anim == BOTH_WALL_FLIP_LEFT )
						{
							pm->ps->velocity[0] = pm->ps->velocity[1] = 0;
							VectorMA( pm->ps->velocity, 150, right, pm->ps->velocity );
						}
						else if ( anim == BOTH_WALL_FLIP_RIGHT )
						{
							pm->ps->velocity[0] = pm->ps->velocity[1] = 0;
							VectorMA( pm->ps->velocity, -150, right, pm->ps->velocity );
						}
						else if ( anim == BOTH_FLIP_BACK1 
							|| anim == BOTH_FLIP_BACK2 
							|| anim == BOTH_FLIP_BACK3 
							|| anim == BOTH_WALL_FLIP_BACK1 )
						{
							pm->ps->velocity[0] = pm->ps->velocity[1] = 0;
							VectorMA( pm->ps->velocity, -150, fwd, pm->ps->velocity );
						}

						//up
						if ( vertPush )
						{
							pm->ps->velocity[2] = vertPush;
							pm->ps->fd.forcePowersActive |= (1 << FP_LEVITATION);
						}
						//animate me
						parts = SETANIM_LEGS;
						if ( anim == BOTH_BUTTERFLY_LEFT )
						{
							parts = SETANIM_BOTH;
							pm->cmd.buttons&=~BUTTON_ATTACK;
							pm->ps->saberMove = LS_NONE;
						}
						else if ( !pm->ps->weaponTime )
						{
							parts = SETANIM_BOTH;
						}
						PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
						if ( anim == BOTH_BUTTERFLY_LEFT )
						{
							pm->ps->weaponTime = pm->ps->torsoTimer;
						}
						PM_SetForceJumpZStart(pm->ps->origin[2]);//so we don't take damage if we land at same height
						pm->ps->pm_flags |= PMF_JUMP_HELD;
						pm->cmd.upmove = 0;
						pm->ps->fd.forceJumpSound = 1;
					}
				}
			}
		}
		else 
		{//in the air
			int legsAnim = pm->ps->legsAnim;

			if ( legsAnim == BOTH_WALL_RUN_LEFT || legsAnim == BOTH_WALL_RUN_RIGHT )
			{//running on a wall
				vec3_t right, traceto, mins, maxs, fwdAngles;
				trace_t	trace;
				int		anim = -1;

				VectorSet(mins, pm->mins[0], pm->mins[0], 0);
				VectorSet(maxs, pm->maxs[0], pm->maxs[0], 24);
				VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

				AngleVectors( fwdAngles, NULL, right, NULL );

				if ( legsAnim == BOTH_WALL_RUN_LEFT )
				{
					if ( pm->ps->legsTimer > 400 )
					{//not at the end of the anim
						float animLen = PM_AnimLength( 0, (animNumber_t)BOTH_WALL_RUN_LEFT );
						if ( pm->ps->legsTimer < animLen - 400 )
						{//not at start of anim
							VectorMA( pm->ps->origin, -16, right, traceto );
							anim = BOTH_WALL_RUN_LEFT_FLIP;
						}
					}
				}
				else if ( legsAnim == BOTH_WALL_RUN_RIGHT )
				{
					if ( pm->ps->legsTimer > 400 )
					{//not at the end of the anim
						float animLen = PM_AnimLength( 0, (animNumber_t)BOTH_WALL_RUN_RIGHT );
						if ( pm->ps->legsTimer < animLen - 400 )
						{//not at start of anim
							VectorMA( pm->ps->origin, 16, right, traceto );
							anim = BOTH_WALL_RUN_RIGHT_FLIP;
						}
					}
				}
				if ( anim != -1 )
				{
					pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, CONTENTS_SOLID|CONTENTS_BODY );
					if ( trace.fraction < 1.0f )
					{//flip off wall
						int parts = 0;

						if ( anim == BOTH_WALL_RUN_LEFT_FLIP )
						{
							pm->ps->velocity[0] *= 0.5f;
							pm->ps->velocity[1] *= 0.5f;
							VectorMA( pm->ps->velocity, 150, right, pm->ps->velocity );
						}
						else if ( anim == BOTH_WALL_RUN_RIGHT_FLIP )
						{
							pm->ps->velocity[0] *= 0.5f;
							pm->ps->velocity[1] *= 0.5f;
							VectorMA( pm->ps->velocity, -150, right, pm->ps->velocity );
						}
						parts = SETANIM_LEGS;
						if ( pm->ps->weaponTime <= 0 )
						{
							parts = SETANIM_BOTH;
						}
						PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
						pm->cmd.upmove = 0;
					}
				}
				if ( pm->cmd.upmove != 0 )
				{//jump failed, so don't try to do normal jump code, just return
					return qfalse;
				}
			}
			//NEW JKA
			else if ( pm->ps->legsAnim == BOTH_FORCEWALLRUNFLIP_START )
			{
				vec3_t fwd, traceto, mins, maxs, fwdAngles;
				trace_t	trace;
				int		anim = -1;
				float animLen;

				VectorSet(mins, pm->mins[0], pm->mins[0], 0.0f);
				VectorSet(maxs, pm->maxs[0], pm->maxs[0], 24.0f);
				//hmm, did you mean [1] and [1]?
				VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0.0f);
				AngleVectors( fwdAngles, fwd, NULL, NULL );

				assert(pm_entSelf); //null pm_entSelf would be a Bad Thing<tm>
				animLen = BG_AnimLength( pm_entSelf->localAnimIndex, BOTH_FORCEWALLRUNFLIP_START );
				if ( pm->ps->legsTimer < animLen - 400 )
				{//not at start of anim
					VectorMA( pm->ps->origin, 16, fwd, traceto );
					anim = BOTH_FORCEWALLRUNFLIP_END;
				}
				if ( anim != -1 )
				{
					pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, CONTENTS_SOLID|CONTENTS_BODY );
					if ( trace.fraction < 1.0f )
					{//flip off wall
						int parts = SETANIM_LEGS;

						pm->ps->velocity[0] *= 0.5f;
						pm->ps->velocity[1] *= 0.5f;
						VectorMA( pm->ps->velocity, -300, fwd, pm->ps->velocity );
						pm->ps->velocity[2] += 200;
						if ( pm->ps->weaponTime <= 0 )
						{//not attacking, set anim on both
							parts = SETANIM_BOTH;
						}
						PM_SetAnim( parts, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
						//FIXME: do damage to traceEnt, like above?
						//pm->ps->pm_flags |= PMF_JUMPING|PMF_SLOW_MO_FALL;
						//ha ha, so silly with your silly jumpy fally flags.
						pm->cmd.upmove = 0;
						PM_AddEvent( EV_JUMP );
					}
				}
				if ( pm->cmd.upmove != 0 )
				{//jump failed, so don't try to do normal jump code, just return
					return qfalse;
				}
			}
			else if ( pm->cmd.forwardmove > 0 //pushing forward
				&& pm->ps->fd.forceRageRecoveryTime < pm->cmd.serverTime	//not in a force Rage recovery period
				&& pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_1 
				&& PM_WalkableGroundDistance() <= 80 //unfortunately we do not have a happy ground timer like SP (this would use up more bandwidth if we wanted prediction workign right), so we'll just use the actual ground distance.
				&& (pm->ps->legsAnim == BOTH_JUMP1 || pm->ps->legsAnim == BOTH_INAIR1 ) )//not in a flip or spin or anything
			{//run up wall, flip backwards
				if ( allowWallRuns )
				{
					//FIXME: have to be moving... make sure it's opposite the wall... or at least forward?
					int wallWalkAnim = BOTH_WALL_FLIP_BACK1;
					int parts = SETANIM_LEGS;
					int contents = MASK_SOLID;//MASK_PLAYERSOLID;//CONTENTS_SOLID;
					if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_2 )
					{
						wallWalkAnim = BOTH_FORCEWALLRUNFLIP_START;
						parts = SETANIM_BOTH;
					}
					else
					{
						if ( pm->ps->weaponTime <= 0 )
						{
							parts = SETANIM_BOTH;
						}
					}

					vec3_t fwd, traceto, mins, maxs, fwdAngles;
					trace_t	trace;
					vec3_t	idealNormal;
					bgEntity_t *traceEnt;

					VectorSet(mins, pm->mins[0], pm->mins[1], 0.0f);
					VectorSet(maxs, pm->maxs[0], pm->maxs[1], 24.0f);
					VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0.0f);

					AngleVectors( fwdAngles, fwd, NULL, NULL );
					VectorMA( pm->ps->origin, 32, fwd, traceto );

					pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, contents );//FIXME: clip brushes too?
					VectorSubtract( pm->ps->origin, traceto, idealNormal );
					VectorNormalize( idealNormal );
					traceEnt = PM_BGEntForNum(trace.entityNum);
						
					if ( trace.fraction < 1.0f
						&&((trace.entityNum<ENTITYNUM_WORLD&&traceEnt&&traceEnt->s.solid!=SOLID_BMODEL)||DotProduct(trace.plane.normal,idealNormal)>0.7f) )
					{//there is a wall there
						pm->ps->velocity[0] = pm->ps->velocity[1] = 0;
						if ( wallWalkAnim == BOTH_FORCEWALLRUNFLIP_START )
						{
							pm->ps->velocity[2] = forceJumpStrength[FORCE_LEVEL_3]/2.0f;
						}
						else
						{
							VectorMA( pm->ps->velocity, -150, fwd, pm->ps->velocity );
							pm->ps->velocity[2] += 150.0f;
						}
						//animate me
						PM_SetAnim( parts, wallWalkAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);

						PM_SetForceJumpZStart(pm->ps->origin[2]);//so we don't take damage if we land at same height
						pm->cmd.upmove = 0;
						pm->ps->fd.forceJumpSound = 1;
						BG_ForcePowerDrain( pm->ps, FP_LEVITATION, 5 );

						pm->cmd.rightmove = pm->cmd.forwardmove= 0;
					}
				}
			}
			else if ( (!BG_InSpecialJump( legsAnim )//not in a special jump anim 
						||BG_InReboundJump( legsAnim )//we're already in a rebound
						||BG_InBackFlip( legsAnim ) )//a backflip (needed so you can jump off a wall behind you)
					&& pm->ps->velocity[2] > -1200 //not falling down very fast
					&& !(pm->ps->pm_flags&PMF_JUMP_HELD)//have to have released jump since last press
					&& (pm->cmd.forwardmove||pm->cmd.rightmove)//pushing in a direction
					&& pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_2//level 3 jump or better
					&& BG_CanUseFPNow(pm->gametype, pm->ps, pm->cmd.serverTime, FP_LEVITATION)
					&& (pm->ps->origin[2]-pm->ps->fd.forceJumpZStart) < (forceJumpHeightMax[FORCE_LEVEL_3]-(BG_ForceWallJumpStrength()/2.0f)) //can fit at least one more wall jump in (yes, using "magic numbers"... for now)
					)
			{//see if we're pushing at a wall and jump off it if so
				if ( allowWallGrabs )
				{
					//FIXME: make sure we have enough force power
					//FIXME: check  to see if we can go any higher
					//FIXME: limit to a certain number of these in a row?
					//FIXME: maybe don't require a ucmd direction, just check all 4?
					//FIXME: should stick to the wall for a second, then push off...
					vec3_t checkDir, traceto, mins, maxs, fwdAngles;
					trace_t	trace;
					vec3_t	idealNormal;
					int		anim = -1;

					VectorSet(mins, pm->mins[0], pm->mins[1], 0.0f);
					VectorSet(maxs, pm->maxs[0], pm->maxs[1], 24.0f);
					VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0.0f);

					if ( pm->cmd.rightmove )
					{
						if ( pm->cmd.rightmove > 0 )
						{
							anim = BOTH_FORCEWALLREBOUND_RIGHT;
							AngleVectors( fwdAngles, NULL, checkDir, NULL );
						}
						else if ( pm->cmd.rightmove < 0 )
						{
							anim = BOTH_FORCEWALLREBOUND_LEFT;
							AngleVectors( fwdAngles, NULL, checkDir, NULL );
							VectorScale( checkDir, -1, checkDir );
						}
					}
					else if ( pm->cmd.forwardmove > 0 )
					{
						anim = BOTH_FORCEWALLREBOUND_FORWARD;
						AngleVectors( fwdAngles, checkDir, NULL, NULL );
					}
					else if ( pm->cmd.forwardmove < 0 )
					{
						anim = BOTH_FORCEWALLREBOUND_BACK;
						AngleVectors( fwdAngles, checkDir, NULL, NULL );
						VectorScale( checkDir, -1, checkDir );
					}
					if ( anim != -1 )
					{//trace in the dir we're pushing in and see if there's a vertical wall there
						bgEntity_t *traceEnt;

						VectorMA( pm->ps->origin, 8, checkDir, traceto );
						pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, CONTENTS_SOLID );//FIXME: clip brushes too?
						VectorSubtract( pm->ps->origin, traceto, idealNormal );
						VectorNormalize( idealNormal );
						traceEnt = PM_BGEntForNum(trace.entityNum);
						if ( trace.fraction < 1.0f
							&&fabs(trace.plane.normal[2]) <= 0.2f/*MAX_WALL_GRAB_SLOPE*/
							&&((trace.entityNum<ENTITYNUM_WORLD&&traceEnt&&traceEnt->s.solid!=SOLID_BMODEL)||DotProduct(trace.plane.normal,idealNormal)>0.7f) )
						{//there is a wall there
							float dot = DotProduct( pm->ps->velocity, trace.plane.normal );
							if ( dot < 1.0f )
							{//can't be heading *away* from the wall!
								//grab it!
								PM_SetAnim( SETANIM_BOTH, anim, SETANIM_FLAG_RESTART|SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
								PM_AddEvent( EV_JUMP );//make sound for grab
								pm->ps->pm_flags |= PMF_STUCK_TO_WALL;
							}
						}
					}
				}
			}
		}
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
	{
		return qfalse;
	}
	if ( pm->cmd.upmove > 0 )
	{//no special jumps
		pm->ps->velocity[2] = bgConstants.baseJumpVelocity;
		PM_SetForceJumpZStart(pm->ps->origin[2]);//so we don't take damage if we land at same height
		pm->ps->pm_flags |= PMF_JUMP_HELD;
	}

	//Jumping
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;
	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	PM_SetForceJumpZStart(pm->ps->origin[2]);

	PM_AddEvent( EV_JUMP );

	//Set the animations
	if ( pm->ps->gravity > 0 && !BG_InSpecialJump( pm->ps->legsAnim ) )
	{
		PM_JumpForDir();

		// Drain stamina as well
		pm->ps->forcePower -= bgConstants.staminaDrains.lossFromJumping;
		if (pm->ps->forcePower < 0) {
			pm->ps->forcePower = 0;
		}
	}

	return qtrue;
}
/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time) {
		return qfalse;
	}

	// check for water jump
	if ( pm->waterlevel != 2 ) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pm->ps->origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( !(cont & CONTENTS_SOLID) ) {
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents (spot, pm->ps->clientNum );
	if ( cont & (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY) ) {
		return qfalse;
	}

	// jump out of water
	VectorScale (pml.forward, 200, pm->ps->velocity);
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
	// waterjump has no control, but falls
	JKG_CheckWaterRemoval(pm->ps);	// remove fire and other debuffs

	PM_StepSlideMove( qtrue );

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	if (pm->ps->velocity[2] < 0) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	if ( PM_CheckWaterJump() ) {
		PM_WaterJumpMove();
		return;
	}

	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	} else {
		for (i=0 ; i<3 ; i++)
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if ( wishspeed > pm->ps->speed * pm_swimScale ) {
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
			pm->ps->velocity, OVERCLIP );

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	JKG_CheckWaterRemoval(pm->ps);	// remove fire and other debuffs

	PM_SlideMove( qfalse );
}

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );
	
	if ( pm->ps->pm_type == PM_SPECTATOR && pm->cmd.buttons & BUTTON_IRONSIGHTS ) {
		//turbo boost
		scale *= 10;
	}

	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = pm->ps->speed * (pm->cmd.upmove/50.0f); //127.0f - increasing upward speed potential
	} else {
		for (i=0 ; i<3 ; i++) {
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove( qfalse );
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		accelerate;
	usercmd_t	cmd;
#ifdef _MP_AIR_MOVEMENT
	float		scale;
#endif

	if (pm->ps->pm_type != PM_SPECTATOR)
	{
		if (pm->ps->fd.forceJumpZStart &&
			pm->ps->forceJumpFlip)
		{
			PM_CheckJump();
		}
	}
	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
#ifdef _MP_AIR_MOVEMENT
	scale = PM_CmdScale( &cmd );
#endif

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for ( i = 0 ; i < 2 ; i++ )
	{
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
#ifdef _MP_AIR_MOVEMENT
	wishspeed *= scale;
#else

	if ( ( DotProduct (pm->ps->velocity, wishdir) ) < 0.0f )
	{//Encourage deceleration away from the current velocity
		wishspeed *= pm_airDecelRate;
	}
#endif

	accelerate = pm_airaccelerate;

	// not on ground, so little effect on velocity
	PM_Accelerate (wishdir, wishspeed, accelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if ( pml.groundPlane ) 
	{
		if ( !(pm->ps->pm_flags&PMF_STUCK_TO_WALL) )
		{//don't slide when stuck to a wall
			if ( PM_GroundSlideOkay( pml.groundTrace.plane.normal[2] ) )
			{
				PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
					pm->ps->velocity, OVERCLIP );
			}
		}
	}

	if ( (pm->ps->pm_flags&PMF_STUCK_TO_WALL) )
	{//no grav when stuck to wall
		PM_StepSlideMove( qfalse );
	}
	else
	{
		PM_StepSlideMove( qtrue );
	}
}

/*
===================
PM_JetpackMove

===================
*/
#define JETPACK_FLOAT_SPEED         128.0f //up movement speed
#define JETPACK_SINK_SPEED			192.0f //down movement speed
static void PM_JetpackMove(void) {
	// Pretty much blatantly copied from Tremulous, because they have good jetpacks
	float scale;
	float wishspeed;
	vec3_t wishvel;
	vec3_t wishdir;
	jetpackData_t* jet = &jetpackTable[pm->ps->jetpack - 1];
	qboolean forwardThrust = qfalse;
	qboolean thrustAllowed = jetpackTable[pm->ps->jetpack - 1].move.thrustAllowed;
	qboolean fwdThrustAllowed = jetpackTable[pm->ps->jetpack - 1].move.fwdThrustAllowed;

	PM_Friction();

	scale = PM_CmdScale(&pm->cmd);

	if (fwdThrustAllowed && (pm->cmd.buttons & BUTTON_SPRINT) && pm->cmd.forwardmove > 0) {
		forwardThrust = qtrue;
		pm->ps->eFlags |= EF_JETPACK_FLAMING;
		pm->cmd.upmove = 0;
		pm->cmd.rightmove = 0;
	}
	else if (pm->cmd.upmove > 0 && thrustAllowed)
	{
		pm->ps->eFlags |= EF_JETPACK_FLAMING; //going up
	}
	else
	{
		pm->ps->eFlags &= ~EF_JETPACK_FLAMING; //idling
	}

	for (int i = 0; i < 2; i++) {
		wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;

		// Modify the wishvel based on the values in the .jet files

		//are we thrusting?
		if (forwardThrust) {
			wishvel[i] *= jet->move.fwdThrustAmt;
		}
		else if (pm->ps->eFlags & EF_JETPACK_FLAMING) {
			if (i) {
				if (pm->cmd.forwardmove > 0) {
					wishvel[i] *= jet->move.thrustFwd;
				}
				else {
					wishvel[i] *= jet->move.thrustBack;
				}
			}
			else {
				wishvel[i] *= jet->move.thrustSide;
			}
		}
		else //no special movement?  Regular movement then.
		{
			if (pm->cmd.forwardmove > 0) 
			{
				wishvel[i] *= jet->move.forwardMove;
			}
			else 
			{
				wishvel[i] *= jet->move.backMove;
			}

			if (pm->cmd.rightmove > 0)
			{
				wishvel[i] *= jet->move.sideMove;
			}
		}
	}

	if (!forwardThrust) {
		if (pm->ps->eFlags & EF_JETPACK_FLAMING)
			wishvel[2] = JETPACK_FLOAT_SPEED * jet->move.thrustUp;

		else if (pm->cmd.upmove < 0)
			wishvel[2] = -JETPACK_SINK_SPEED * jet->move.downMove;

		else
			wishvel[2] = jet->move.hoverGravity;
	}
	else {
		wishvel[2] = ((pml.forward[2] * pm->cmd.forwardmove) + pml.right[2]) * jet->move.fwdThrustAmt;
	}

	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate(wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove(qfalse);

	PM_AnimateJetpack();
}

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed = 0.0f;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
		// begin swimming
		PM_WaterMove();
		return;
	}

	if (pm->ps->pm_type != PM_SPECTATOR)
	{
		if ( PM_CheckJump () ) {
			// jumped away
			if ( pm->waterlevel > 1 ) {
				PM_WaterMove();
			} else {
				PM_AirMove();
			}
			return;
		}
	}

	PM_ApplySpeedConstants();
	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale( &cmd );

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	// Get The WishVel And WishSpeed
	//-------------------------------  

	for ( i = 0 ; i < 3 ; i++ ) {
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if ( (pm->ps->pm_flags & PMF_ROLLING) && !BG_InRoll(pm->ps, pm->ps->legsAnim) &&
		!PM_InRollComplete(pm->ps, pm->ps->legsAnim))
	{
		if ( wishspeed > pm->ps->speed * pm_duckScale ) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if ( pm->waterlevel ) {
		float	waterScale;

		waterScale = pm->waterlevel / 3.0f;
		waterScale = 1.0f - ( 1.0f - pm_swimScale ) * waterScale;
		if ( wishspeed > pm->ps->speed * waterScale ) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK )
	{
		accelerate = pm_airaccelerate;
	}
	else
	{
		accelerate = pm_accelerate;
	}

// some speed modifiers --eez
	if( BG_IsSprinting( pm->ps, &pm->cmd, qtrue ) )
	{
		wishspeed *= bgConstants.sprintSpeedModifier;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK )
	{
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
		pm->ps->velocity, OVERCLIP );

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove( qfalse );
	
	// Xy: sprinting!
	if ( BG_IsSprinting (pm->ps, &pm->cmd, qtrue) && pm->ps->sprintDebounceTime <= pm->cmd.serverTime )
	{
	    BG_ForcePowerDrain (pm->ps, FP_ABSORB, 1);
	    pm->ps->sprintDebounceTime = pm->cmd.serverTime + 80;
	    if ( pm->ps->forcePower <= 0 )
	    {
	        pm->ps->sprintMustWait = 1;
	    }
	}
	else if ( pm->ps->sprintMustWait && pm->ps->forcePower >= bgConstants.staminaDrains.minSprintThreshold )
	{
	    pm->ps->sprintMustWait = 0;
	}
}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
	float	forward;

	if ( !pml.walking ) {
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if ( forward <= 0 ) {
		VectorClear (pm->ps->velocity);
	} else {
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5f;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );
	if (pm->cmd.buttons & BUTTON_ATTACK) {	//turbo boost
		scale *= 8;
	}
	if ( pm->cmd.buttons & BUTTON_SPRINT) {	//turbo boost
		scale *= 8;
	}

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void )
{
	if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) 
	{
		return 0;
	}
	return ( pml.groundTrace.surfaceFlags & MATERIAL_MASK );
}

extern qboolean PM_CanRollFromSoulCal( playerState_t *ps );
static int PM_TryRoll( void )
{
	trace_t	trace;
	int		anim = -1;
	vec3_t fwd, right, traceto, mins, maxs, fwdAngles;

	if (pm->ps->forcePower < bgConstants.staminaDrains.minRollThreshold) {
		// Not enough stamina to roll
		return 0;
	}

	if ( BG_SaberInSpecialAttack( pm->ps->torsoAnim ) 
		|| BG_SpinningSaberAnim( pm->ps->legsAnim ))
	{//special attacking or spinning
		if ( !PM_CanRollFromSoulCal( pm->ps ) )
		{//hehe
			return 0;
		}
	}

	// Deathspike: When the weapon is capable of being rolled with, perform the roll when we are in a movement up/down.
	if ( !GetWeaponData( pm->ps->weapon, pm->ps->weaponVariation )->hasRollAbility)
	{
		return 0;
	}

	if ( pm->ps->weapon == WP_SABER )
	{
		saberInfo_t *saber = BG_MySaber( pm->ps->clientNum, 0 );
		if ( saber
			&& (saber->saberFlags&SFL_NO_ROLLS) )
		{
			return 0;
		}
		saber = BG_MySaber( pm->ps->clientNum, 1 );
		if ( saber
			&& (saber->saberFlags&SFL_NO_ROLLS) )
		{
			return 0;
		}
	}

	VectorSet(mins, pm->mins[0],pm->mins[1],pm->mins[2]+STEPSIZE);
	VectorSet(maxs, pm->maxs[0],pm->maxs[1],pm->ps->crouchheight);

	VectorSet(fwdAngles, 0, pm->ps->viewangles[YAW], 0);

	AngleVectors( fwdAngles, fwd, right, NULL );

	if ( pm->cmd.forwardmove )
	{ //check forward/backward rolls
		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
		{
			anim = BOTH_ROLL_B;
			VectorMA( pm->ps->origin, -64, fwd, traceto );
		}
		else
		{
			anim = BOTH_ROLL_F;
			VectorMA( pm->ps->origin, 64, fwd, traceto );
		}
	}
	else if ( pm->cmd.rightmove > 0 )
	{ //right
		anim = BOTH_ROLL_R;
		VectorMA( pm->ps->origin, 64, right, traceto );
	}
	else if ( pm->cmd.rightmove < 0 )
	{ //left
		anim = BOTH_ROLL_L;
		VectorMA( pm->ps->origin, -64, right, traceto );
	}

	if ( anim != -1 )
	{ //We want to roll. Perform a trace to see if we can, and if so, send us into one.
		pm->trace( &trace, pm->ps->origin, mins, maxs, traceto, pm->ps->clientNum, CONTENTS_SOLID );
		if ( trace.fraction >= 1.0f )
		{
			pm->ps->saberMove = LS_NONE;
			return anim;
		}
	}
	return 0;
}

#ifdef _GAME
static void PM_CrashLandEffect( void )
{
	float delta;
	if ( pm->waterlevel )
	{
		return;
	}
	delta = fabs(pml.previous_velocity[2])/10;//VectorLength( pml.previous_velocity );?
	if ( delta >= 30 ) 
	{
		vec3_t bottom;
		int	effectID = -1;
		int material = (pml.groundTrace.surfaceFlags&MATERIAL_MASK);
		VectorSet( bottom, pm->ps->origin[0],pm->ps->origin[1],pm->ps->origin[2]+pm->mins[2]+1 );
		switch ( material )
		{
		case MATERIAL_MUD:
			effectID = EFFECT_LANDING_MUD;
			break;
		case MATERIAL_SAND:			
			effectID = EFFECT_LANDING_SAND;
			break;
		case MATERIAL_DIRT:
			effectID = EFFECT_LANDING_DIRT;
			break;
		case MATERIAL_SNOW:			
			effectID = EFFECT_LANDING_SNOW;
			break;
		case MATERIAL_GRAVEL:
			effectID = EFFECT_LANDING_GRAVEL;
			break;
		}

		if ( effectID != -1 )
		{
			G_PlayEffect( effectID, bottom, pml.groundTrace.plane.normal );
		}
	}
}
#endif
/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand( void ) {
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	qboolean	didRoll = qfalse;

	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if ( den < 0 ) {
		pm->ps->inAirAnim = qfalse;
		return;
	}
	t = (-b - sqrt( den ) ) / ( 2 * a );

	delta = vel + t * acc;
	delta = delta*delta * 0.0001f;

#ifdef _GAME
	PM_CrashLandEffect();
#endif
	// ducking while falling doubles damage
	if ( pm->ps->pm_flags & PMF_DUCKED ) {
		delta *= 2;
	}

	if (pm->ps->legsAnim == BOTH_A7_KICK_F_AIR ||
		pm->ps->legsAnim == BOTH_A7_KICK_B_AIR ||
		pm->ps->legsAnim == BOTH_A7_KICK_R_AIR ||
		pm->ps->legsAnim == BOTH_A7_KICK_L_AIR)
	{
		int landAnim = -1;
		switch ( pm->ps->legsAnim )
		{
		case BOTH_A7_KICK_F_AIR:
			landAnim = BOTH_FORCELAND1;
			break;
		case BOTH_A7_KICK_B_AIR:
			landAnim = BOTH_FORCELANDBACK1;
			break;
		case BOTH_A7_KICK_R_AIR:
			landAnim = BOTH_FORCELANDRIGHT1;
			break;
		case BOTH_A7_KICK_L_AIR:
			landAnim = BOTH_FORCELANDLEFT1;
			break;
		}
		if ( landAnim != -1 )
		{
			if ( pm->ps->torsoAnim == pm->ps->legsAnim )
			{
				PM_SetAnim(SETANIM_BOTH, landAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else
			{
				PM_SetAnim(SETANIM_LEGS, landAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
		}
	}
	else if (pm->ps->legsAnim == BOTH_FORCEJUMPLEFT1 ||
			pm->ps->legsAnim == BOTH_FORCEJUMPRIGHT1 ||
			pm->ps->legsAnim == BOTH_FORCEJUMPBACK1 ||
			pm->ps->legsAnim == BOTH_FORCEJUMP1)
	{
		int fjAnim;
		switch (pm->ps->legsAnim)
		{
		case BOTH_FORCEJUMPLEFT1:
			fjAnim = BOTH_LANDLEFT1;
			break;
		case BOTH_FORCEJUMPRIGHT1:
			fjAnim = BOTH_LANDRIGHT1;
			break;
		case BOTH_FORCEJUMPBACK1:
			fjAnim = BOTH_LANDBACK1;
			break;
		default:
			fjAnim = BOTH_LAND1;
			break;
		}
		PM_SetAnim(SETANIM_BOTH, fjAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	// decide which landing animation to use
	else if (!BG_InRoll(pm->ps, pm->ps->legsAnim) && pm->ps->inAirAnim && !pm->ps->m_iVehicleNum)
	{ //only play a land animation if we transitioned into an in-air animation while off the ground
		if (!BG_SaberInSpecial(pm->ps->saberMove) && !PM_InKnockDown(pm->ps))
		{
			if ( pm->ps->pm_flags & PMF_BACKWARDS_JUMP ) {
				PM_ForceLegsAnim( BOTH_LANDBACK1 );
			} else {
				PM_ForceLegsAnim( BOTH_LAND1 );
			}
		}
	}

	if (pm->ps->weapon != WP_SABER && pm->ps->weapon != WP_MELEE)
	{ //saber handles its own anims
		//This will push us back into our weaponready stance from the land anim.
		if ( pm->ps->zoomMode == 1 )
		{
			PM_StartTorsoAnim( GetWeaponData( pm->ps->weapon, pm->ps->weaponVariation )->anims.sights.torsoAnim );
		}
		else
		{
			if (pm->ps->weapon == WP_EMPLACED_GUN)
			{
				PM_StartTorsoAnim( BOTH_GUNSIT1 );
			}
			else
			{
				if ( !BG_IsSprinting (pm->ps, &pm->cmd, qtrue) )
				{
					if( pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK )
					{
						PM_StartTorsoAnim( GetWeaponData (pm->ps->weapon, pm->ps->weaponVariation)->anims.sights.torsoAnim );
					}
					else
					{
						PM_StartTorsoAnim( GetWeaponData (pm->ps->weapon, pm->ps->weaponVariation)->anims.ready.torsoAnim );
					}
				}
			}
		}
	}

	if (!BG_InSpecialJump(pm->ps->legsAnim) ||
		pm->ps->legsTimer < 1 ||
		(pm->ps->legsAnim) == BOTH_WALL_RUN_LEFT ||
		(pm->ps->legsAnim) == BOTH_WALL_RUN_RIGHT)
	{ //Only set the timer if we're in an anim that can be interrupted (this would not be, say, a flip)
		if (!BG_InRoll(pm->ps, pm->ps->legsAnim) && pm->ps->inAirAnim)
		{
			if (!BG_SaberInSpecial(pm->ps->saberMove) || pm->ps->weapon != WP_SABER)
			{
				if (pm->ps->legsAnim != BOTH_FORCELAND1 &&
					pm->ps->legsAnim != BOTH_FORCELANDBACK1 &&
					pm->ps->legsAnim != BOTH_FORCELANDRIGHT1 &&
					pm->ps->legsAnim != BOTH_FORCELANDLEFT1)
				{ //don't override if we have started a force land
					pm->ps->legsTimer = TIMER_LAND;
				}
			}
		}
	}

	pm->ps->inAirAnim = qfalse;

	if (pm->ps->m_iVehicleNum)
	{ //don't do fall stuff while on a vehicle
		return;
	}

	// never take falling damage if completely underwater
	if ( pm->waterlevel == 3 ) {
		return;
	}

	// reduce falling damage if there is standing water
	if ( pm->waterlevel == 2 ) {
		delta *= 0.25f;
	}
	if ( pm->waterlevel == 1 ) {
		delta *= 0.5f;
	}

	if ( delta < 1 ) {
		return;
	}

	if ( pm->ps->pm_flags & PMF_DUCKED ) 
	{
		if( delta >= 2 
			// && !PM_InOnGroundAnim( pm->ps->legsAnim )
			&& !PM_InKnockDown( pm->ps )
			&& !BG_InRoll(pm->ps, pm->ps->legsAnim) 
			&& pm->ps->forceHandExtend == HANDEXTEND_NONE )
		{//roll!
			int anim = PM_TryRoll();

			if (PM_InRollComplete(pm->ps, pm->ps->legsAnim))
			{
				anim = 0;
				pm->ps->legsTimer = 0;
				pm->ps->legsAnim = 0;
				PM_SetAnim(SETANIM_BOTH,BOTH_LAND1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				pm->ps->legsTimer = TIMER_LAND;
			}

			if ( anim )
			{//absorb some impact
				pm->ps->legsTimer = 0;
				delta /= 3; // /= 2 just cancels out the above delta *= 2 when landing while crouched, the roll itself should absorb a little damage
				pm->ps->legsAnim = 0;
				if (pm->ps->torsoAnim == BOTH_A7_SOULCAL)
				{ //get out of it on torso
					pm->ps->torsoTimer = 0;
				}
				PM_SetAnim(SETANIM_BOTH,anim,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				didRoll = qtrue;

				// Drain some stamina from rolling
				pm->ps->forcePower -= bgConstants.staminaDrains.lossFromRolling;
				if (pm->ps->forcePower < 0) {
					pm->ps->forcePower = 0;
				}

				// Check to see if we should remove roll related debuffs.
				JKG_CheckRollRemoval(pm->ps);
			}
		}
	}

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) )  {
		if (delta > 7)
		{
			int delta_send = (int)delta;

			if (delta_send > 600)
			{ //will never need to know any value above this
				delta_send = 600;
			}

			if (pm->ps->fd.forceJumpZStart)
			{
				if ((int)pm->ps->origin[2] >= (int)pm->ps->fd.forceJumpZStart)
				{ //was force jumping, landed on higher or same level as when force jump was started
					if (delta_send > 8)
					{
						delta_send = 8;
					}
				}
				else
				{
					if (delta_send > 8)
					{
						int dif = ((int)pm->ps->fd.forceJumpZStart - (int)pm->ps->origin[2]);
						int dmgLess = (forceJumpHeight[pm->ps->fd.forcePowerLevel[FP_LEVITATION]] - dif);

						if (dmgLess < 0)
						{
							dmgLess = 0;
						}

						delta_send -= (dmgLess*0.3f);

						if (delta_send < 8)
						{
							delta_send = 8;
						}

						//Com_Printf("Damage sub: %i\n", (int)((dmgLess*0.1f)));
					}
				}
			}

			if (didRoll)
			{ //Add the appropriate event..
				PM_AddEventWithParm( EV_ROLL, delta_send );
			}
			else
			{
				PM_AddEventWithParm( EV_FALL, delta_send );
			}
		}
		else
		{
			if (didRoll)
			{
				PM_AddEventWithParm( EV_ROLL, 0 );
			}
			else
			{
				PM_AddEventWithParm( EV_FOOTSTEP, PM_FootstepForSurface() );
			}
		}
	}

	// make sure velocity resets so we don't bounce back up again in case we miss the clear elsewhere
	pm->ps->velocity[2] = 0;

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
	int			i, j, k;
	vec3_t		point;

	if ( pm->debugLevel ) {
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if ( !trace->allsolid ) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25f;

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}

/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/

//[KnockdownSys]
qboolean PM_KnockDownAnimExtended( int anim );
//[/KnockdownSys]

static void PM_GroundTraceMissed( void ) {
	trace_t		trace;
	vec3_t		point;

	//rww - don't want to do this when handextend_choke, because you can be standing on the ground
	//while still holding your throat.
	if ( pm->ps->pm_type == PM_FLOAT ) 
	{
		//we're assuming this is because you're being choked
		int parts = SETANIM_LEGS;

		//rww - also don't use SETANIM_FLAG_HOLD, it will cause the legs to float around a bit before going into
		//a proper anim even when on the ground.
		PM_SetAnim(parts, BOTH_CHOKE3, SETANIM_FLAG_OVERRIDE);
	}
	else if ( pm->ps->pm_type == PM_JETPACK ) 
	{//jetpacking
		//rww - also don't use SETANIM_FLAG_HOLD, it will cause the legs to float around a bit before going into
		//a proper anim even when on the ground.
		//PM_SetAnim(SETANIM_LEGS,BOTH_FORCEJUMP1,SETANIM_FLAG_OVERRIDE);	
	}
	//If the anim is choke3, act like we just went into the air because we aren't in a float
	else if ( pm->ps->groundEntityNum != ENTITYNUM_NONE || (pm->ps->legsAnim) == BOTH_CHOKE3 ) 
	{
		// we just transitioned into freefall
		if ( pm->debugLevel ) {
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0f || pm->ps->pm_type == PM_FLOAT ) {
			if ( pm->ps->velocity[2] <= 0 && !(pm->ps->pm_flags&PMF_JUMP_HELD))
			{
				//PM_SetAnim(SETANIM_LEGS,BOTH_INAIR1,SETANIM_FLAG_OVERRIDE);
				PM_SetAnim(SETANIM_LEGS,BOTH_INAIR1,0);
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			}
			else if ( pm->cmd.forwardmove >= 0 ) 
			{
				PM_SetAnim(SETANIM_LEGS,BOTH_JUMP1,SETANIM_FLAG_OVERRIDE);
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} 
			else 
			{
				PM_SetAnim(SETANIM_LEGS,BOTH_JUMPBACK1,SETANIM_FLAG_OVERRIDE);
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}

			pm->ps->inAirAnim = qtrue;
		}
	}
	else if ( PM_KnockDownAnimExtended( pm->ps->legsAnim ) )
	{//no in-air anims when in knockdown anim
	}
	else if (!pm->ps->inAirAnim)
	{
		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy( pm->ps->origin, point );
		point[2] -= 64;

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if ( trace.fraction == 1.0f || pm->ps->pm_type == PM_FLOAT )
		{
			pm->ps->inAirAnim = qtrue;
		}
	}

	if (PM_InRollComplete(pm->ps, pm->ps->legsAnim))
	{ //Client won't catch an animation restart because it only checks frame against incoming frame, so if you roll when you land after rolling
	  //off of something it won't replay the roll anim unless we switch it off in the air. This fixes that.
		PM_SetAnim(SETANIM_BOTH,BOTH_INAIR1,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		pm->ps->inAirAnim = qtrue;
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
	vec3_t		point;
	trace_t		trace;
	float minNormal = (float)MIN_WALK_NORMAL;

	if ( pm->ps->clientNum >= MAX_CLIENTS)
	{
		minNormal = 0.0;//0.5;
	}

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25f;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if ( trace.allsolid ) {
		if ( !PM_CorrectAllSolid(&trace) )
			return;
	}

	if (pm->ps->pm_type == PM_FLOAT || pm->ps->pm_type == PM_JETPACK)
	{
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0f ) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		//[KnockdownSys]
		if(PM_InKnockDown( pm->ps ) )
		{//already in a knockdown
		}
		else if ( pm->cmd.forwardmove >= 0 ) {
		//if ( pm->cmd.forwardmove >= 0 ) {
		//[/KnockdownSys]
			PM_ForceLegsAnim( BOTH_JUMP1 );
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} else {
			PM_ForceLegsAnim( BOTH_JUMPBACK1 );
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	
	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < minNormal ) {
		if ( pm->debugLevel ) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
		// just hit the ground
		if ( pm->debugLevel ) {
			Com_Printf("%i:Land\n", c_pmove);
		}
		
		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		if ( pml.previous_velocity[2] < -200 ) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 1000;
		}
		
		
	}

	pm->ps->groundEntityNum = trace.entityNum;
	pm->ps->lastOnGround = pm->cmd.serverTime;

	PM_AddTouchEnt( trace.entityNum );	
}


/*
=============
PM_SetWaterLevel
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
			}
		}
	}

}

/*
==============
PM_CanStand

==============
*/
static qboolean PM_CanStand ( void )
{
    qboolean canStand = qtrue;
    float x, y;
    trace_t trace;

    const vec3_t lineMins = { -5.0f, -5.0f, -2.5f };
    const vec3_t lineMaxs = { 5.0f, 5.0f, 0.0f };

    for ( x = pm->mins[0] + 5.0f; canStand && x <= (pm->maxs[0] - 5.0f); x += 10.0f )
    {
        for ( y = pm->mins[1] + 5.0f; y <= (pm->maxs[1] - 5.0f); y += 10.0f )
        {
            vec3_t start = { x, y, pm->maxs[2] };
            vec3_t end = { x, y, (float)pm->ps->standheight };

            VectorAdd (start, pm->ps->origin, start);
            VectorAdd (end, pm->ps->origin, end);

            pm->trace (&trace, start, lineMins, lineMaxs, end, pm->ps->clientNum, pm->tracemask);
		    if ( trace.allsolid || trace.fraction < 1.0f )
		    {
			    canStand = qfalse;
			    break;
		    }
        }
    }
	
    return canStand;
}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/

// FIXME: remove
//[KnockdownSys]
qboolean PM_GettingUpFromKnockDown( float standheight, float crouchheight );
//[/KnockdownSys]

static void PM_CheckDuck (void)
{
	// No crouching in jetpack, doesnt do anything anyway
	if (pm->ps->pm_type == PM_JETPACK)
	{
		pm->ps->pm_flags &= ~PMF_DUCKED;
		pm->ps->pm_flags &= ~PMF_ROLLING;
	}

	if (pm->ps->clientNum < MAX_CLIENTS)
	{
		pm->mins[0] = -15;
		pm->mins[1] = -15;

		pm->maxs[0] = 15;
		pm->maxs[1] = 15;
	}

	if ( pm->ps->legsAnim == BOTH_JUMPATTACK6 )
	{
		//dynamically reduce bounding box to let character sail over heads of enemies
		if ( ( pm->ps->legsTimer >= 1450
				&& PM_AnimLength( 0, BOTH_JUMPATTACK6 ) - pm->ps->legsTimer >= 400 ) 
			||(pm->ps->legsTimer >= 400
				&& PM_AnimLength( 0, BOTH_JUMPATTACK6 ) - pm->ps->legsTimer >= 1100 ) )
		{//in a part of the anim that we're pretty much sideways in, raise up the mins
			pm->mins[2] = 0;
			pm->ps->pm_flags |= PMF_FIX_MINS;
		}
	}
	else
	{
		if ( (pm->ps->pm_flags&PMF_FIX_MINS) )// pm->mins[2] > DEFAULT_MINS_2 )
		{//drop the mins back down
			//do a trace to make sure it's okay
			trace_t	trace;
			vec3_t end, curMins, curMaxs;
	
			VectorSet( end, pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2]+MINS_Z ); 
			VectorSet( curMins, pm->mins[0], pm->mins[1], 0 ); 
			VectorSet( curMaxs, pm->maxs[0], pm->maxs[1], pm->ps->standheight ); 

			pm->trace( &trace, pm->ps->origin, curMins, curMaxs, end, pm->ps->clientNum, pm->tracemask );
			if ( !trace.allsolid && !trace.startsolid )
			{//should never start in solid
				if ( trace.fraction >= 1.0f )
				{//all clear
					//drop the bottom of my bbox back down
					pm->mins[2] = MINS_Z;
					pm->ps->pm_flags &= ~PMF_FIX_MINS;
				}
				else
				{//move me up so the bottom of my bbox will be where the trace ended, at least
					//need to trace up, too
					float updist = ((1.0f-trace.fraction) * -MINS_Z);
					end[2] = pm->ps->origin[2]+updist; 
					pm->trace( &trace, pm->ps->origin, curMins, curMaxs, end, pm->ps->clientNum, pm->tracemask );
					if ( !trace.allsolid && !trace.startsolid )
					{//should never start in solid
						if ( trace.fraction >= 1.0f )
						{//all clear
							//move me up
							pm->ps->origin[2] += updist;
							//drop the bottom of my bbox back down
							pm->mins[2] = MINS_Z;
							pm->ps->pm_flags &= ~PMF_FIX_MINS;
						}
						else
						{//crap, no room to expand, so just crouch us
							if ( pm->ps->legsAnim != BOTH_JUMPATTACK6
								|| pm->ps->legsTimer <= 200 )
							{//at the end of the anim, and we can't leave ourselves like this
								//so drop the maxs, put the mins back and move us up
								pm->maxs[2] += MINS_Z;
								pm->ps->origin[2] -= MINS_Z;
								pm->mins[2] = MINS_Z;
								//this way we'll be in a crouch when we're done
								if ( pm->ps->legsAnim == BOTH_JUMPATTACK6 )
								{
									pm->ps->legsTimer = pm->ps->torsoTimer = 0;
								}
								pm->ps->pm_flags |= PMF_DUCKED;
								//FIXME: do we need to set a crouch anim here?
								pm->ps->pm_flags &= ~PMF_FIX_MINS;
							}
						}
					}//crap, stuck
				}
			}//crap, stuck!
		}

		if ( !pm->mins[2] )
		{
			pm->mins[2] = MINS_Z;
		}
	}

	if (pm->ps->pm_type == PM_DEAD && pm->ps->clientNum < MAX_CLIENTS)
	{
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	if (BG_InRoll(pm->ps, pm->ps->legsAnim) && !BG_KickingAnim(pm->ps->legsAnim))
	{
		pm->maxs[2] = pm->ps->crouchheight; //CROUCH_MAXS_2;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
		pm->ps->pm_flags &= ~PMF_DUCKED;
		pm->ps->pm_flags |= PMF_ROLLING;
		return;
	}
	else if (pm->ps->pm_flags & PMF_ROLLING)
	{
		// Xycaleth's fix for crochjumping through roof
            	if ( PM_CanStand() )
            	{
               			pm->maxs[2] = pm->ps->standheight;
               			pm->ps->pm_flags &= ~PMF_ROLLING;
            	}
	}
	//[KnockdownSys]
	else if ( PM_GettingUpFromKnockDown( pm->ps->standheight, pm->ps->crouchheight ) )
	{//we're attempting to get up from a knockdown.
		pm->ps->viewheight = pm->ps->crouchheight + STANDARD_VIEWHEIGHT_OFFSET;
		return;
	}
	else if ( PM_InKnockDown( pm->ps ) )
	{//forced crouch
		pm->maxs[2] = pm->ps->crouchheight;
		pm->ps->viewheight = pm->ps->crouchheight + STANDARD_VIEWHEIGHT_OFFSET;
		pm->ps->pm_flags |= PMF_DUCKED;
		return;
	}
	//[/KnockdownSys]
	else if (pm->cmd.upmove < 0 ||
		pm->ps->forceHandExtend == HANDEXTEND_KNOCKDOWN ||
		pm->ps->forceHandExtend == HANDEXTEND_PRETHROWN ||
		pm->ps->forceHandExtend == HANDEXTEND_POSTTHROWN)
	{	// duck
		if (pm->ps->pm_type != PM_JETPACK) {
			pm->ps->pm_flags |= PMF_DUCKED;
		}
	} // stand up if possible 
	else if (pm->ps->pm_flags & PMF_DUCKED)
	{
		// Xycaleth's fix for crochjumping through roof
		if ( PM_CanStand() )
		{
			pm->maxs[2] = pm->ps->standheight;
			pm->ps->pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		pm->maxs[2] = pm->ps->crouchheight;//CROUCH_MAXS_2;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else if (pm->ps->pm_flags & PMF_ROLLING)
	{
		pm->maxs[2] = pm->ps->crouchheight;//CROUCH_MAXS_2;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
	else
	{
		pm->maxs[2] = pm->ps->standheight;//DEFAULT_MAXS_2;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}

/*
=================
PM_FootSlopeTrace

=================
*/
void PM_FootSlopeTrace( float *pDiff, float *pInterval )
{
	vec3_t	footLOrg, footROrg, footLBot, footRBot;
	vec3_t footLPoint, footRPoint;
	vec3_t footMins, footMaxs;
	vec3_t footLSlope, footRSlope;

	trace_t	trace;
	float	diff, interval;

	mdxaBone_t	boltMatrix;
	vec3_t		G2Angles;
	
	VectorSet(G2Angles, 0, pm->ps->viewangles[YAW], 0);

	interval = 4;//?

	trap->G2API_GetBoltMatrix( pm->ghoul2, 0, pm->g2Bolts_LFoot, 
			&boltMatrix, G2Angles, pm->ps->origin, pm->cmd.serverTime, 
					NULL, pm->modelScale );
	footLPoint[0] = boltMatrix.matrix[0][3];
	footLPoint[1] = boltMatrix.matrix[1][3];
	footLPoint[2] = boltMatrix.matrix[2][3];
	
	trap->G2API_GetBoltMatrix( pm->ghoul2, 0, pm->g2Bolts_RFoot, 
					&boltMatrix, G2Angles, pm->ps->origin, pm->cmd.serverTime, 
					NULL, pm->modelScale );
	footRPoint[0] = boltMatrix.matrix[0][3];
	footRPoint[1] = boltMatrix.matrix[1][3];
	footRPoint[2] = boltMatrix.matrix[2][3];

	//get these on the cgame and store it, save ourselves a ghoul2 construct skel call
	VectorCopy( footLPoint, footLOrg );
	VectorCopy( footRPoint, footROrg );

	//step 2: adjust foot tag z height to bottom of bbox+1
	footLOrg[2] = pm->ps->origin[2] + pm->mins[2] + 1;
	footROrg[2] = pm->ps->origin[2] + pm->mins[2] + 1;
	VectorSet( footLBot, footLOrg[0], footLOrg[1], footLOrg[2] - interval*10 );
	VectorSet( footRBot, footROrg[0], footROrg[1], footROrg[2] - interval*10 );

	//step 3: trace down from each, find difference
	VectorSet( footMins, -3, -3, 0 );
	VectorSet( footMaxs, 3, 3, 1 );

	pm->trace( &trace, footLOrg, footMins, footMaxs, footLBot, pm->ps->clientNum, pm->tracemask );
	VectorCopy( trace.endpos, footLBot );
	VectorCopy( trace.plane.normal, footLSlope );

	pm->trace( &trace, footROrg, footMins, footMaxs, footRBot, pm->ps->clientNum, pm->tracemask );
	VectorCopy( trace.endpos, footRBot );
	VectorCopy( trace.plane.normal, footRSlope );

	diff = footLBot[2] - footRBot[2];

	if ( pDiff != NULL )
	{
		*pDiff = diff;
	}
	if ( pInterval != NULL )
	{
		*pInterval = interval;
	}
}

/*
=================
PM_UsingIronSights
=================
*/
qboolean PM_UsingIronSights(const pmove_t *pm) {
	if (pm->ps->weapon == WP_THERMAL ||
			pm->ps->weapon == WP_TRIP_MINE ||
			pm->ps->weapon == WP_DET_PACK) {
		return qfalse;
	}

	return (pm->cmd.buttons & BUTTON_IRONSIGHTS) || pm->ps->zoomMode;
}

/*
===============
PM_Footsteps

===============
*/
static void PM_Footsteps( void ) {
	float		bobmove;
	int			old;
	int			setAnimFlags = 0;
	weaponData_t *wd = GetWeaponData(pm->ps->weapon, pm->ps->weaponVariation);

	//[Knockdown]
	if ( PM_InKnockDown( pm->ps ) )
	{//don't interrupt knockdowns with footstep stuff.
		return;
	}
	//[/Knockdown]

	if ( (PM_InSaberAnim( (pm->ps->legsAnim) ) && !BG_SpinningSaberAnim( (pm->ps->legsAnim) )) 
		|| (pm->ps->legsAnim) == BOTH_STAND1 
		|| (pm->ps->legsAnim) == BOTH_STAND1TO2 
		|| (pm->ps->legsAnim) == BOTH_STAND2TO1 
		|| (pm->ps->legsAnim) == BOTH_STAND2 
		|| (pm->ps->legsAnim) == BOTH_SABERFAST_STANCE
		|| (pm->ps->legsAnim) == BOTH_SABERSLOW_STANCE
		|| (pm->ps->legsAnim) == BOTH_BUTTON_HOLD
		|| (pm->ps->legsAnim) == BOTH_BUTTON_RELEASE
		|| (pm->ps->legsAnim) == BOTH_SPRINT
		|| PM_LandingAnim( (pm->ps->legsAnim) ) 
		|| PM_PainAnim( (pm->ps->legsAnim) ))
	{//legs are in a saber anim, and not spinning, be sure to override it
		setAnimFlags |= SETANIM_FLAG_OVERRIDE;
	}

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
		+  pm->ps->velocity[1] * pm->ps->velocity[1] );

	if (pm->ps->saberMove == LS_SPINATTACK)
	{
		PM_ContinueLegsAnim( pm->ps->torsoAnim );
	}
	else if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {

		// airborne leaves position in cycle intact, but doesn't advance
		if ( pm->waterlevel > 1 )
		{
			if (pm->xyspeed > 60)
			{
				PM_ContinueLegsAnim( BOTH_SWIMFORWARD );
			}
			else
			{
				PM_ContinueLegsAnim( BOTH_SWIM_IDLE1 );
			}
		}
		if(BG_IsSprinting (pm->ps, &pm->cmd, qtrue))
		{
			PM_ContinueLegsAnim( BOTH_SPRINT );
		}
		return;
	}
	// if not trying to move
	else if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
		if (  pm->xyspeed < 5 ) {
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if ( pm->ps->clientNum >= MAX_CLIENTS &&
				pm_entSelf &&
				pm_entSelf->s.NPC_class == CLASS_RANCOR )
			{
				if ( (pm->ps->eFlags2&EF2_USE_ALT_ANIM) )
				{//holding someone
					PM_ContinueLegsAnim( BOTH_STAND4 );
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND4);
				}
				else if ( (pm->ps->eFlags2&EF2_ALERTED) )
				{//have an enemy or have had one since we spawned
					PM_ContinueLegsAnim( BOTH_STAND2 );
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND2);
				}
				else
				{//just stand there
					PM_ContinueLegsAnim( BOTH_STAND1 );
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND1);
				}
			}
			else if ( pm->ps->clientNum >= MAX_CLIENTS &&
				pm_entSelf &&
				pm_entSelf->s.NPC_class == CLASS_WAMPA )
			{
				if ( (pm->ps->eFlags2&EF2_USE_ALT_ANIM) )
				{//holding a victim
					PM_ContinueLegsAnim( BOTH_STAND2 );
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND2);
				}
				else
				{//not holding a victim
					PM_ContinueLegsAnim( BOTH_STAND1 );
					//PM_SetAnim(pm,SETANIM_LEGS,BOTH_STAND1);
				}
			}
			else if ( (pm->ps->pm_flags & PMF_DUCKED) || (pm->ps->pm_flags & PMF_ROLLING) ) {
				if ((pm->ps->legsAnim) != BOTH_CROUCH1IDLE)
				{
					PM_SetAnim(SETANIM_LEGS, BOTH_CROUCH1IDLE, setAnimFlags);
				}
				else
				{
					PM_ContinueLegsAnim( BOTH_CROUCH1IDLE );
				}
			} else {
				if (pm->ps->weapon == WP_DISRUPTOR && pm->ps->zoomMode == 1)
				{
					///????  continue legs anim on a torso anim...??!!!
					//yeah.. the anim has a valid pose for the legs, it uses it (you can't move while using disruptor)
					PM_ContinueLegsAnim( TORSO_WEAPONREADY4 );
				}
				else
				{
					if (pm->ps->weapon == WP_SABER && BG_SabersOff( pm->ps ) )
					{
						if (!PM_AdjustStandAnimForSlope())
						{
							//PM_ContinueLegsAnim( BOTH_STAND1 );
							weaponData_t *wp = BG_GetWeaponDataByIndex( pm->ps->weaponId );
							PM_ContinueLegsAnim(PM_LegsSlopeBackTransition( wp->anims.ready.legsAnim ));
						}
					}
					else
					{
						if (pm->ps->weapon != WP_SABER || !PM_AdjustStandAnimForSlope())
						{
							if (pm->ps->weapon == WP_SABER)
							{
								PM_ContinueLegsAnim(PM_LegsSlopeBackTransition(PM_GetSaberStance()));
							}
							else
							{
								if( pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK )
									PM_ContinueLegsAnim(PM_LegsSlopeBackTransition(GetWeaponData (pm->ps->weapon, pm->ps->weaponVariation)->anims.sights.legsAnim));
								else
									PM_ContinueLegsAnim(PM_LegsSlopeBackTransition(GetWeaponData (pm->ps->weapon, pm->ps->weaponVariation)->anims.ready.legsAnim));
							}
						}
					}
				}
			}
		}
		return;
	}

	if (pm->ps->saberMove == LS_SPINATTACK)
	{
		bobmove = 0.2f;
		PM_ContinueLegsAnim( pm->ps->torsoAnim );
	}
	else if ( pm->ps->pm_flags & PMF_DUCKED )
	{
		int rolled = 0;

		bobmove = 0.5f;	// ducked characters bob much faster

		if ( ( BG_RunningAnim(pm->ps->legsAnim) 
			|| PM_CanRollFromSoulCal( pm->ps ) 
			|| pm->ps->saberActionFlags & (1 << SAF_BLOCKING) 
			|| pm->cmd.buttons & BUTTON_SPRINT
			|| (pm->cmd.buttons & BUTTON_IRONSIGHTS && !pm->ps->pm_flags & PMF_DUCKED )
			|| pm->cmd.buttons & BUTTON_WALKING )
				&& !BG_InRoll(pm->ps, pm->ps->legsAnim) )
			// simplified but more accurate at the same time
		{//roll!
			rolled = PM_TryRoll();
		}
		if ( !rolled )
		{ //if the roll failed or didn't attempt, do standard crouching anim stuff.
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
				if ((pm->ps->legsAnim) != BOTH_CROUCH1WALKBACK)
				{
					PM_SetAnim(SETANIM_LEGS, BOTH_CROUCH1WALKBACK, setAnimFlags);
				}
				else
				{
					PM_ContinueLegsAnim( BOTH_CROUCH1WALKBACK );
				}
			}
			else {
				if ((pm->ps->legsAnim) != BOTH_CROUCH1WALK)
				{
					PM_SetAnim(SETANIM_LEGS, BOTH_CROUCH1WALK, setAnimFlags);
				}
				else
				{
					PM_ContinueLegsAnim( BOTH_CROUCH1WALK );
				}
			}
		}
		else
		{ //otherwise send us into the roll
			pm->ps->legsTimer = 0;
			pm->ps->legsAnim = 0;
			PM_SetAnim(SETANIM_BOTH,rolled,SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			PM_AddEventWithParm( EV_ROLL, 0 );
			pm->maxs[2] = pm->ps->crouchheight;//CROUCH_MAXS_2;
			pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
			pm->ps->pm_flags &= ~PMF_DUCKED;
			pm->ps->pm_flags |= PMF_ROLLING;

			// Check to see if we should remove roll related debuffs.
			JKG_CheckRollRemoval(pm->ps);

			// Drain a little stamina from rolling
			pm->ps->forcePower -= bgConstants.staminaDrains.lossFromRolling;
			if (pm->ps->forcePower < 0) {
				pm->ps->forcePower = 0;
			}
		}
	}
	else if ((pm->ps->pm_flags & PMF_ROLLING) && !BG_InRoll(pm->ps, pm->ps->legsAnim) &&
		!PM_InRollComplete(pm->ps, pm->ps->legsAnim))
	{
		bobmove = 0.5f;	// ducked characters bob much faster

		if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
		{
			if ((pm->ps->legsAnim) != BOTH_CROUCH1WALKBACK)
			{
				PM_SetAnim(SETANIM_LEGS, BOTH_CROUCH1WALKBACK, setAnimFlags);
			}
			else
			{
				PM_ContinueLegsAnim( BOTH_CROUCH1WALKBACK );
			}
		}
		else
		{
			if ((pm->ps->legsAnim) != BOTH_CROUCH1WALK)
			{
				PM_SetAnim(SETANIM_LEGS, BOTH_CROUCH1WALK, setAnimFlags);
			}
			else
			{
				PM_ContinueLegsAnim( BOTH_CROUCH1WALK );
			}
		}
	}
	else
	{
		int desiredAnim = -1;

		if ((pm->ps->legsAnim == BOTH_FORCELAND1 ||
			pm->ps->legsAnim == BOTH_FORCELANDBACK1 ||
			pm->ps->legsAnim == BOTH_FORCELANDRIGHT1 ||
			pm->ps->legsAnim == BOTH_FORCELANDLEFT1) &&
			pm->ps->legsTimer > 0)
		{ //let it finish first
			bobmove = 0.2f;
		}
		else if ( BG_IsSprinting (pm->ps, &pm->cmd, qtrue) )
		{
		    bobmove = 0.5f;
		    // Xy: No need to handle NPCs for sprinting cause I'm lazy
		    if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
		    {
		        desiredAnim = pm->ps->saberHolstered > 1 ? BOTH_RUNBACK1 : BOTH_RUNBACK2;
		    }
		    else
		    {
		        // TODO: Need to replace these with the correct animations
				if( pm->ps->weapon == WP_SABER )
				{
					if( SaberStances[pm->ps->fd.saberAnimLevel].isStaffOnly && !BG_SabersOff(pm->ps) )
					{
						if ( pm->ps->saberHolstered > 1 )
						{//blades off
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
						}
						else if ( pm->ps->saberHolstered == 1 )
						{//1 blade on
							desiredAnim = BOTH_RUN2;
						}
						else
						{
							if (pm->ps->fd.forcePowersActive & (1<<FP_SPEED))
							{
								desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
							}
							else
							{
								desiredAnim = BOTH_RUN_STAFF;
							}
						}
					}
					else if( SaberStances[pm->ps->fd.saberAnimLevel].isDualsOnly && !BG_SabersOff(pm->ps) )
					{
						if ( pm->ps->saberHolstered > 1 )
						{//blades off
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
						}
						else if ( pm->ps->saberHolstered == 1 )
						{//1 saber on
							desiredAnim = BOTH_RUN2;
						}
						else
						{
							desiredAnim = BOTH_RUN_DUAL;
						}
					}
					else
					{
						desiredAnim = BOTH_SPRINT;
					}
				}
				else
				{
					if ( pm->ps->weapon == WP_MELEE || pm->ps->weapon == WP_NONE )
					{
						desiredAnim = BOTH_SPRINT;
					}
					else
					{
						desiredAnim = wd->anims.sprint.torsoAnim;
					}
				}
		    }
		}
		else if ( !(pm->cmd.buttons & BUTTON_WALKING) && // We are not holding the walk button
				!PM_UsingIronSights(pm) &&
				!(pm->ps->saberActionFlags & (1 << SAF_BLOCKING)) ) // not in block mode
		{ // Running, in other words.
			bobmove = 0.4f;	// faster speeds bob faster
			if ( pm->ps->clientNum >= MAX_CLIENTS &&
				pm_entSelf &&
				pm_entSelf->s.NPC_class == CLASS_WAMPA )
			{
				if ( (pm->ps->eFlags2&EF2_USE_ALT_ANIM) )
				{//full on run, on all fours
					desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
				}
				else
				{//regular, upright run
					desiredAnim = BOTH_RUN2;
				}
			}
			else if ( pm->ps->clientNum >= MAX_CLIENTS &&
				pm_entSelf &&
				pm_entSelf->s.NPC_class == CLASS_RANCOR )
			{//no run anims
				if ( (pm->ps->pm_flags&PMF_BACKWARDS_RUN) )
				{
					desiredAnim = BOTH_WALKBACK1;
				}
				else
				{
					desiredAnim = BOTH_WALK1;
				}
			}
#ifdef _GAME
			else if ( pm->ps->clientNum >= MAX_CLIENTS &&
				pm_entSelf &&
				pm_entSelf->s.NPC_class == CLASS_JAWA)
			{
				// Jawa has a special run animation :D
				desiredAnim = BOTH_RUN4;
				bobmove = 0.2f;
			} 
#endif
			else if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
			{
				if( SaberStances[pm->ps->fd.saberAnimLevel].isStaffOnly )
				{
					if ( pm->ps->saberHolstered > 1 ) 
					{//saber off
						desiredAnim = BOTH_RUNBACK1;
					}
					else
					{
						//desiredAnim = BOTH_RUNBACK_STAFF;
						//hmm.. stuff runback anim is pretty messed up for some reason.
						desiredAnim = BOTH_RUNBACK2;
					}
				}
				else if( SaberStances[pm->ps->fd.saberAnimLevel].isDualsOnly && !BG_SabersOff(pm->ps) )
				{
					if ( pm->ps->saberHolstered > 1 ) 
					{//sabers off
						desiredAnim = BOTH_RUNBACK1;
					}
					else
					{
						//desiredAnim = BOTH_RUNBACK_DUAL;
						//and so is the dual
						desiredAnim = BOTH_RUNBACK2;
					}
				}
				else
				{
					if ( pm->ps->saberHolstered ) 
					{//saber off
						desiredAnim = BOTH_RUNBACK1;
					}
					else
					{
						desiredAnim = BOTH_RUNBACK2;
					}
				}
			}
			else
			{
				if( pm->ps->weapon != WP_MELEE )
				{
					if( SaberStances[pm->ps->fd.saberAnimLevel].isStaffOnly )
					{
						if ( pm->ps->saberHolstered > 1 )
						{//blades off
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
						}
						else if ( pm->ps->saberHolstered == 1 )
						{//1 blade on
							desiredAnim = BOTH_RUN2;
						}
						else
						{
							if (pm->ps->fd.forcePowersActive & (1<<FP_SPEED))
							{
								desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
							}
							else
							{
								desiredAnim = BOTH_RUN_STAFF;
							}
						}
					}
					else if( SaberStances[pm->ps->fd.saberAnimLevel].isDualsOnly && !BG_SabersOff(pm->ps) )
					{
						if ( pm->ps->saberHolstered > 1 )
						{//blades off
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
						}
						else if ( pm->ps->saberHolstered == 1 )
						{//1 saber on
							desiredAnim = BOTH_RUN2;
						}
						else
						{
							desiredAnim = BOTH_RUN_DUAL;
						}
					}
					else
					{
						if ( pm->ps->saberHolstered || pm->ps->weapon == WP_MELEE || pm->ps->weapon == WP_NONE )  // JKG - Anim fix
						{//saber off
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
						}
						else
						{
							desiredAnim = BOTH_RUN2;
						}
					}
				}
				else
				{
					desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALERUN : BOTH_RUN1;
				}
			}
		}
		else
		{
			bobmove = 0.2f;	// walking bobs slow
			if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
			{
				if( SaberStances[pm->ps->fd.saberAnimLevel].isStaffOnly )
				{
					if ( pm->ps->saberHolstered > 1 )
					{
						desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALKBACK : BOTH_WALKBACK1;
					}
					else if ( pm->ps->saberHolstered )
					{
						desiredAnim = BOTH_WALKBACK2;
					}
					else
					{
						desiredAnim = BOTH_WALKBACK_STAFF;
					}
				}
				else if( SaberStances[pm->ps->fd.saberAnimLevel].isDualsOnly )
				{
					if ( pm->ps->saberHolstered > 1 )
					{
						desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALKBACK : BOTH_WALKBACK1;
					}
					else if ( pm->ps->saberHolstered )
					{
						desiredAnim = BOTH_WALKBACK2;
					}
					else
					{
						desiredAnim = BOTH_WALKBACK_DUAL;
					}
				}
				else
				{
					if ( pm->ps->weapon == WP_SABER && pm->ps->saberHolstered )
					{
						desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALKBACK : BOTH_WALKBACK1;
					}
					else if(pm->ps->weapon == WP_SABER && !pm->ps->saberHolstered)
					{
						desiredAnim = BOTH_WALKBACK2;
					}
					else
					{
						desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALKBACK : BOTH_WALKBACK1;
					}
				}
			}
			else
			{
				if ( pm->ps->weapon == WP_MELEE || pm->ps->weapon == WP_NONE )
				{
					desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALK : BOTH_WALK1;
				}
				else if ( BG_SabersOff( pm->ps ) )
				{
					desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALK : BOTH_WALK1;
				}
				else
				{
					if( SaberStances[pm->ps->fd.saberAnimLevel].isStaffOnly )
					{
						if ( pm->ps->saberHolstered > 1 )
						{
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALK : BOTH_WALK1;
						}
						else if ( pm->ps->saberHolstered )
						{
							desiredAnim = BOTH_WALK2;
						}
						else
						{
							desiredAnim = BOTH_WALK_STAFF;
						}
					}
					else if( SaberStances[pm->ps->fd.saberAnimLevel].isDualsOnly )
					{
						if ( pm->ps->saberHolstered > 1 )
						{
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALK : BOTH_WALK1;
						}
						else if ( pm->ps->saberHolstered )
						{
							desiredAnim = BOTH_WALK2;
						}
						else
						{
							desiredAnim = BOTH_WALK_DUAL;
						}
					}
					else
					{
						if ( pm->ps->saberHolstered )
						{
							desiredAnim = (pm->gender == GENDER_FEMALE) ? BOTH_FEMALEEWALK : BOTH_WALK1;
						}
						else
						{
							desiredAnim = BOTH_WALK2;
						}
					}
				}
			}
		}

		if (desiredAnim != -1)
		{
			int ires = PM_LegsSlopeBackTransition(desiredAnim);

			if(BG_IsSprinting(pm->ps, &pm->cmd, qtrue))
			{
				if(pm->ps->torsoAnim != desiredAnim && ires == desiredAnim)
				{
					PM_SetAnim(SETANIM_BOTH, desiredAnim, setAnimFlags);
				}
			}
			if ((pm->ps->legsAnim) != desiredAnim && ires == desiredAnim)
			{
				PM_SetAnim(SETANIM_LEGS, desiredAnim, setAnimFlags);
			}
			else
			{
				PM_ContinueLegsAnim(ires);
			}
		}
	}

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 )
	{
		pm->ps->footstepTime = pm->cmd.serverTime + 300;
		if ( pm->waterlevel == 1 ) {
			// splashing
			PM_AddEvent( EV_FOOTSPLASH );
		} else if ( pm->waterlevel == 2 ) {
			// wading / swimming at surface
			PM_AddEvent( EV_SWIM );
		} else if ( pm->waterlevel == 3 ) {
			// no sound when completely underwater
		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
#ifdef _GAME
	qboolean impact_splash = qfalse;
#endif
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
#ifdef _GAME
		if ( VectorLengthSquared( pm->ps->velocity ) > 40000 )
		{
			impact_splash = qtrue;
		}
#endif
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
#ifdef _GAME
		if ( VectorLengthSquared( pm->ps->velocity ) > 40000 )
		{
			impact_splash = qtrue;
		}
#endif
		PM_AddEvent( EV_WATER_LEAVE );
	}

#ifdef _GAME
	if ( impact_splash )
	{
		//play the splash effect
		trace_t	tr;
		vec3_t	start, end;


		VectorCopy( pm->ps->origin, start );
		VectorCopy( pm->ps->origin, end );

		// FIXME: set start and end better
		start[2] += 10;
		end[2] -= 40;

		pm->trace( &tr, start, vec3_origin, vec3_origin, end, pm->ps->clientNum, MASK_WATER );

		if ( tr.fraction < 1.0f )
		{
			if ( (tr.contents&CONTENTS_LAVA) )
			{
				G_PlayEffect( EFFECT_LAVA_SPLASH, tr.endpos, tr.plane.normal );
			}
			else if ( (tr.contents&CONTENTS_SLIME) )
			{
				G_PlayEffect( EFFECT_ACID_SPLASH, tr.endpos, tr.plane.normal );
			}
			else //must be water
			{
				G_PlayEffect( EFFECT_WATER_SPLASH, tr.endpos, tr.plane.normal );
			}
		}
	}
#endif

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}

/*
===============
PM_BeginWeaponChange
===============
*/
#ifdef _GAME
void G_PM_SwitchWeaponClip(playerState_t *ps, int newweapon, int newvariation, usercmd_t& cmd);
void G_PM_SwitchWeaponFiringMode(playerState_t *ps, int newweapon, int newvariation);
#endif
void PM_BeginWeaponChange( int weaponId, const weaponData_t* weaponData) {
    int weapon, variation;
	if(!BG_GetWeaponByIndex(pm->cmd.weapon, &weapon, &variation))
		return;

	/*if( pm->ps->clientNum >= MAX_CLIENTS )
	{
		if ( weaponId > 0 && pm->ps->clientNum, weaponId)
		{
			return;
		}
	}*/

	// Don't allow while in blocking mode for sabers, otherwise this fucks EVERYTHING up...
	if( pm->ps->weapon == WP_SABER && pm->ps->saberActionFlags & ( 1 << SAF_BLOCKING ) )
	{
		return;
	}

	// Don't allow while reloading.
	if ( pm->ps->weaponstate == WEAPON_DROPPING || pm->ps->weaponstate == WEAPON_RELOADING ) {
		return;
	}

	// Likewise, don't allow while sprinting.
	if( BG_IsSprinting( pm->ps, &pm->cmd, qtrue ) )
	{
		return;
	}

	// turn off any kind of zooming when weapon switching.
	if (pm->ps->zoomMode)
	{
		pm->ps->zoomMode = 0;
		pm->ps->zoomTime = pm->ps->commandTime;
	}

	//no switching while overheated
	if (pm->ps->overheated)
	{
		return;
	}

	// Is our weapon hot? (50%+ heat)
	if (pm->ps->heat > pm->ps->maxHeat / 2.0f)
	{
		PM_AddEvent(EV_OVERHEATED);

		//add weaponTime wait if weapon is hot
		pm->ps->weaponTime += (10 * (pm->ps->heat / pm->ps->maxHeat*100));

		if (pm->ps->heat > pm->ps->heatThreshold)
			pm->ps->weaponTime += 400; //extra penalty for switching while beyond threshold

		//In the future allow, but apply damage if we switch while overheated? possibly as a skill?
	}

    // Change of weapon
    PM_AddEventWithParm( EV_CHANGE_WEAPON, weaponId);
    
	pm->ps->weaponstate = WEAPON_DROPPING;
	if( pm->ps->weapon == WP_SABER )
	{
		if( pm->ps->saberHolstered == 2 && weapon == WP_SABER )
		{
			pm->ps->weaponTime += (weaponData->swapTime / 2); //150 by default
		}
		else if( pm->ps->saberHolstered == 2 && weapon == WP_MELEE )
		{
			// Switching from saber to melee
			PM_SetAnim(SETANIM_BOTH, BOTH_STAND2TO1, SETANIM_FLAG_OVERRIDE);
		}
		else if( pm->ps->saberHolstered == 2 && weapon != WP_SABER )
		{
			pm->ps->weaponTime += weaponData->swapTime; //300 by default
		}
		else
		{
			pm->ps->weaponTime += (weaponData->swapTime * 2); //600 by default
			PM_SetAnim(SETANIM_TORSO, TORSO_DROPWEAP1, SETANIM_FLAG_OVERRIDE);
		}
	}

	//if switching from a pistol or grenade, move faster
	else if (pm->ps->weapon == WP_BRYAR_PISTOL || pm->ps->weapon == WP_THERMAL)
	{
		pm->ps->weaponTime += (0.75 * weaponData->swapTime); //25% faster (225 default)
		PM_SetAnim(SETANIM_TORSO, TORSO_DROPWEAP1, SETANIM_FLAG_OVERRIDE);

	}

	else
	{
		pm->ps->weaponTime += weaponData->swapTime;
		
		if (weapon == WP_MELEE || weapon == WP_BRYAR_PISTOL || weapon == WP_THERMAL) //if switching to melee, a pistol, or grenade reduce switch time
		{
			pm->ps->weaponTime -= (0.25 * weaponData->swapTime); //25% (-75 default)
		}
		PM_SetAnim(SETANIM_TORSO, TORSO_DROPWEAP1, SETANIM_FLAG_OVERRIDE);
	}
}


/*
===============
PM_FinishWeaponChange
===============
*/

void PM_FinishWeaponChange( void ) {
	int		weapon;
	int     variation;

	//eezstreet edit
	if(!BG_GetWeaponByIndex(pm->cmd.weapon, &weapon, &variation))
		return;

#ifdef _GAME
	if( weapon != WP_MELEE )
		JKG_DoubleCheckWeaponChange( &pm->cmd, pm->ps ); // sometimes sabers get fucked up, time to implement a hacky solution :/
#endif

	if (weapon == WP_SABER)
	{
		//Stoiss add[SaberThrowSys]
		if (!pm->ps->saberEntityNum && pm->ps->saberInFlight)
		{//our saber is currently dropped.  Don't allow the dropped blade to be activated.
			if ( pm->ps->fd.saberAnimLevel == SS_DUAL )
			{//holding second saber, activate it.
				pm->ps->saberHolstered = 1;
				PM_SetSaberMove(LS_DRAW);
			}
			else
			{//not holding any sabers, make sure all our blades are all off.
				pm->ps->saberHolstered = 2;
			}
		}
		else if(!pm->ps->saberInFlight)
		{//have saber(s)
			if( pm->ps->weapon == WP_MELEE || pm->ps->weapon == WP_NONE )
			{
				// Don't do any sort of fancy unholstering if we're switching from melee->saber
				pm->ps->saberHolstered = 2;
			}
			PM_SetSaberMove(LS_DRAW);
		}
	}
	//Stoiss end[/SaberThrowSys]
	else if( weapon == WP_MELEE || weapon == WP_NONE )
	{
		PM_SetAnim(SETANIM_TORSO, BOTH_STAND9, SETANIM_FLAG_OVERRIDE);
	}
	else
	{
		//PM_StartTorsoAnim( TORSO_RAISEWEAP1);
		PM_SetAnim(SETANIM_TORSO, TORSO_RAISEWEAP1, SETANIM_FLAG_OVERRIDE);
	}

#ifdef _GAME	// Handle proper switching of the weapon in case we're using clips
	G_PM_SwitchWeaponClip(pm->ps, weapon, variation, pm->cmd);
	G_PM_SwitchWeaponFiringMode(pm->ps, weapon, variation);
#endif

    pm->ps->weaponId = pm->cmd.weapon;
	pm->ps->weapon = weapon;
	pm->ps->weaponVariation = variation;
	pm->ps->weaponstate = WEAPON_RAISING;

	pm->ps->weaponTime += 350;
	pm->ps->heat = 0.0f; // all weapon heat is eliminated when we switch weapons

	const weaponData_t *weaponData;
	weaponData = GetWeaponData(pm->ps->weapon, pm->ps->weaponVariation);
	pm->ps->maxHeat = weaponData->firemodes[pm->ps->firingMode].maxHeat; //set maxHeat to new weapon value
	pm->ps->heatThreshold = weaponData->firemodes[pm->ps->firingMode].heatThreshold;

	
	if (weapon == WP_SABER) //set saber crystal color
	{
		const saberCrystalData_t* crystal = JKG_GetSaberCrystal(weaponData->sab.defaultcrystal);
		if (crystal)
		{
			pm->ps->saberCrystal[0] = crystal->crystalID;
			//pm->ps->saberCrystal[1] = crystal->crystalID; //--futuza: additional sabers?  not sure how to handle that yet...
		}
	}
}

/*
======================
PM_DoChargedWeapons

======================
*/
static qboolean PM_DoChargedWeapons( void )
{
	qboolean	charging = qfalse;
	weaponData_t *thisWeaponData = GetWeaponData( pm->ps->weapon, pm->ps->weaponVariation );
		
	/* See if this weapon is chargeable with the primary fire */
	if ( pm->cmd.buttons & BUTTON_ATTACK )
	{
		if ( thisWeaponData->zoomType != ZOOM_NONE )
		{
			if ( thisWeaponData->firemodes[pm->ps->firingMode].chargeTime &&
				pm->ps->stats[STAT_AMMO] >= thisWeaponData->firemodes[pm->ps->firingMode].cost) 
			{
                   charging = qtrue;
		    }
		}
		/* Check the primary fire for a charged weapon; Even if we're using zoom, we must retain original specs */
		else if ( !charging && thisWeaponData->firemodes[pm->ps->firingMode].chargeTime && pm->ps->stats[STAT_AMMO] >= thisWeaponData->firemodes[pm->ps->firingMode].cost )
		{
			charging = qtrue;
		}
	}

	// set up the appropriate weapon state based on the button that's down.  
	//	Note that we ALWAYS return if charging is set ( meaning the buttons are still down )
	if ( charging )
	{
		weaponFireModeStats_t *weaponFireData = &GetWeaponData( pm->ps->weapon, pm->ps->weaponVariation )->firemodes[pm->ps->firingMode];	

		if ( pm->ps->weaponstate != WEAPON_CHARGING )
		{
			// charge isn't started, so do it now
			pm->ps->weaponstate = WEAPON_CHARGING;
			pm->ps->weaponChargeTime = pm->cmd.serverTime;
			pm->ps->weaponChargeSubtractTime = pm->cmd.serverTime + weaponFireData->chargeTime;

			BG_AddPredictableEventToPlayerstate(EV_WEAPON_CHARGE, (pm->ps->weapon << 8) | (pm->ps->weaponVariation & 0xFF), pm->ps);
		}

		// Jedi Knight Galaxies - handle clips
		if ( weaponFireData->clipSize ) 
		{
			if (( pm->cmd.serverTime - pm->ps->weaponChargeTime) < weaponFireData->chargeMaximum )
			{
				if (pm->ps->weaponChargeSubtractTime < pm->cmd.serverTime)
				{
					pm->ps->stats[STAT_AMMO] -= weaponFireData->chargeSubtract;
					pm->ps->weaponChargeSubtractTime = pm->cmd.serverTime + weaponFireData->chargeTime;
				}
			}
		} 
		else 
		{
			if ((pm->cmd.serverTime - pm->ps->weaponChargeTime) < weaponFireData->chargeMaximum)
			{
				if (pm->ps->weaponChargeSubtractTime < pm->cmd.serverTime)
				{
#ifdef _GAME
					gentity_t *Gself = &g_entities[pm->ps->clientNum];
					if (weaponFireData->useQuantity) 
					{
						BG_AdjustItemStackQuantity(Gself, pm->cmd.invensel, -weaponFireData->cost);
					}
					else 
					{
						Gself->client->ammoTable[pm->ps->ammoType] -= weaponFireData->cost;
					}
#endif
					pm->ps->stats[STAT_TOTALAMMO] -= weaponFireData->chargeSubtract;
					pm->ps->weaponChargeSubtractTime = pm->cmd.serverTime + weaponFireData->chargeTime;
				}
			}
		}

		return qtrue; // short-circuit rest of weapon code
	}

	// Only charging weapons should be able to set these states...so....
	//	let's see which fire mode we need to set up now that the buttons are up
	if ( pm->ps->weaponstate == WEAPON_CHARGING )
	{
		// weapon has a charge, so let us do an attack

		// dumb, but since we shoot a charged weapon on button-up, we need to repress this button for now
		pm->cmd.buttons |= BUTTON_ATTACK;
		pm->ps->eFlags |= EF_FIRING;
	}

	return qfalse; // continue with the rest of the weapon code
}

//perform player anim overrides while on vehicle.
extern int PM_irand_timesync(int val1, int val2);

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
// FIXME: remove extern
extern int PM_KickMoveForConditions(void);
static void PM_Weapon(void)
{
	int		addTime = 0;
	int amount;
	int		killAfterItem = 0;
	bgEntity_t* veh = NULL;
	// Jedi Knight Galaxies
	int doStdAnim;
	static qboolean jkg_didGrenadeCook[MAX_GENTITIES];
	const weaponData_t* weaponData;


#ifdef _GAME
	if (pm->ps->clientNum >= MAX_CLIENTS &&
		pm->ps->weapon == WP_NONE &&
		pm->cmd.weapon == WP_NONE &&
		pm_entSelf)
	{ //npc with no weapon
		gentity_t* gent = (gentity_t*)pm_entSelf;
		if (gent->inuse && gent->client &&
			gent->localAnimIndex < NUM_RESERVED_ANIMSETS)
		{ //humanoid
			pm->ps->torsoAnim = pm->ps->legsAnim;
			pm->ps->torsoTimer = pm->ps->legsTimer;
			return;
		}
	}
#endif

	if (!pm->ps->emplacedIndex &&
		pm->ps->weapon == WP_EMPLACED_GUN)
	{ //oh no!
		int i = 0;
		int weap = -1;

		while (i < WP_NUM_WEAPONS)
		{
			if ((pm->ps->stats[STAT_WEAPONS] & (1 << i)) && i != WP_NONE)
			{ //this one's good
				weap = i;
				break;
			}
			i++;
		}

		if (weap != -1)
		{
			pm->cmd.weapon = BG_GetWeaponIndexFromClass(weap, 0);
			pm->ps->weapon = weap;
			pm->ps->weaponVariation = 0;
			return;
		}
	}

	if (!PM_WeaponAnimate()) {
		return;
	}

	if (BG_InSpecialJump(pm->ps->legsAnim) ||
		BG_InRoll(pm->ps, pm->ps->legsAnim) ||
		PM_InRollComplete(pm->ps, pm->ps->legsAnim))
	{
		if (pm->ps->weaponstate != WEAPON_ROLLFIRING)
		{
			if (pm->ps->weaponTime < pm->ps->legsTimer)
			{
				pm->ps->weaponTime = pm->ps->legsTimer;
			}
		}
	}

	if (pm->ps->duelInProgress)
	{
		pm->cmd.weapon = BG_GetWeaponIndexFromClass(WP_SABER, 0);
		pm->ps->weapon = WP_SABER;
		pm->ps->weaponVariation = 1;

		if (pm->ps->duelTime >= pm->cmd.serverTime)
		{
			pm->cmd.upmove = 0;
			pm->cmd.forwardmove = 0;
			pm->cmd.rightmove = 0;
		}
	}

	if (pm->ps->weapon == WP_SABER && pm->ps->saberMove != LS_READY && pm->ps->saberMove != LS_NONE)
	{
		pm->cmd.weapon = BG_GetWeaponIndexFromClass(WP_SABER, pm->ps->weaponVariation); //don't allow switching out mid-attack
	}

	if (pm->ps->weapon == WP_SABER)
	{
		//rww - we still need the item stuff, so we won't return immediately
		PM_WeaponLightsaber();
		killAfterItem = 1;
	}
	else if (pm->ps->weapon != WP_EMPLACED_GUN)
	{
		if (!pm->ps->saberInFlight)
		{
			pm->ps->saberHolstered = 0;
		}
	}

	// don't allow attack until all buttons are up
	if (pm->ps->pm_flags & PMF_RESPAWNED) {
		return;
	}

	// ignore if spectator
	if (pm->ps->clientNum < MAX_CLIENTS && pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	// check for dead player
	if (pm->ps->stats[STAT_HEALTH] <= 0) {
		pm->ps->weapon = WP_NONE;
		pm->ps->weaponVariation = 0;
		return;
	}

	if (killAfterItem)
	{
		return;
	}

	// make weapon function
	if (pm->ps->weaponTime > 0) {
		pm->ps->weaponTime -= pml.msec;
	}

	if (pm->ps->duelInProgress && pm->ps->emplacedIndex)
	{
		pm->ps->emplacedIndex = 0;
		pm->ps->saberHolstered = 0;
	}

	if (pm->ps->weapon == WP_EMPLACED_GUN && pm->ps->emplacedIndex)
	{
		pm->cmd.weapon = BG_GetWeaponIndexFromClass(WP_EMPLACED_GUN, 0); //No switch for you!
		PM_StartTorsoAnim(BOTH_GUNSIT1);
	}

	amount = GetWeaponData(pm->ps->weapon, pm->ps->weaponVariation)->firemodes[pm->ps->firingMode].cost;
	weaponData = GetWeaponData(pm->ps->weapon, pm->ps->weaponVariation);
	// take an ammo away if not infinite

	// Jedi Knight Galaxies - Dont bitch about ammo unless we try to fire our weapon

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	// or if cooking a grenade
	if (pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING)
	{
		if ((pm->cmd.weapon != pm->ps->weaponId) && pm->ps->weaponstate != WEAPON_DROPPING)
		{
			if (jkg_didGrenadeCook[pm->ps->clientNum]) //needs a better way to check if we're currently cooking a nade on server, this doesn't really work. --futuza
				;
			else
			{
				PM_BeginWeaponChange(pm->cmd.weapon, weaponData);
			}
		}
	}


	if (pm->ps->weaponstate == WEAPON_FIRING && pm->ps->weapon >= WP_BRYAR_PISTOL && pm->ps->weapon <= WP_ROCKET_LAUNCHER) {
		// Ultra super duper special case - we are changing from a firing animation to transition into sights
		if (pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->torsoAnim == weaponData->anims.firing.torsoAnim) {
			PM_StartTorsoAnim(weaponData->anims.sights.torsoAnim);
		}
		else if (!(pm->ps->ironsightsTime & IRONSIGHTS_MSB) && pm->ps->torsoAnim == weaponData->anims.sightsFiring.torsoAnim) {
			PM_StartTorsoAnim(weaponData->anims.ready.torsoAnim);
		}
	}

	if (pm->ps->weaponTime > 0) {
		return;
	}

	// change weapon if time
	if (pm->ps->weaponstate == WEAPON_DROPPING) {
		PM_FinishWeaponChange();
		return;
	}

	if (pm->ps->weaponstate == WEAPON_RAISING || pm->ps->weaponstate == WEAPON_RELOADING)
	{
		pm->ps->weaponstate = WEAPON_READY;

		if (PM_CanSetWeaponAnims())
		{
			if (pm->ps->weapon == WP_SABER)
			{
				PM_StartTorsoAnim(PM_GetSaberStance());
			}
			else
			{
				if (pm->ps->weapon == WP_EMPLACED_GUN)
				{
					PM_StartTorsoAnim(BOTH_GUNSIT1);
				}
				else
				{
					if (pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK)
						PM_StartTorsoAnim(weaponData->anims.sights.torsoAnim);
					else
						PM_StartTorsoAnim(weaponData->anims.ready.torsoAnim);
				}
			}
		}
		return;
	}

	if (PM_CanSetWeaponAnims())
	{
		if (pm->ps->weaponstate == WEAPON_READY && pm->ps->weaponTime <= 0 &&
			(pm->ps->weapon >= WP_BRYAR_PISTOL || pm->ps->weapon == WP_STUN_BATON) &&
			pm->ps->torsoTimer <= 0 &&
			(((pm->ps->ironsightsTime & IRONSIGHTS_MSB) && pm->ps->torsoAnim != weaponData->anims.sights.torsoAnim) ||
				(!(pm->ps->ironsightsTime & IRONSIGHTS_MSB) && pm->ps->torsoAnim != weaponData->anims.ready.torsoAnim)) &&
			//(pm->ps->torsoAnim) != weaponData->anims.ready.torsoAnim &&
			pm->ps->torsoAnim != TORSO_WEAPONIDLE3 &&
			pm->ps->weapon != WP_EMPLACED_GUN)
		{
			if (!BG_IsSprinting(pm->ps, &pm->cmd, qtrue))
			{
				if (pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK)
					PM_StartTorsoAnim(weaponData->anims.sights.torsoAnim);
				else
					PM_StartTorsoAnim(weaponData->anims.ready.torsoAnim);
			}
		}
		else if (pm->ps->weapon == WP_MELEE || pm->ps->weapon == WP_NONE)
		{
			if (pm->ps->weaponTime <= 0 &&
				pm->ps->forceHandExtend == HANDEXTEND_NONE)
			{
				int desTAnim = pm->ps->legsAnim;
				if ((desTAnim == BOTH_STAND1 || desTAnim == BOTH_STAND2 || desTAnim == BOTH_STAND9) &&
					pm->ps->weapon == WP_MELEE)
				{ //remap the standard standing anims for melee stance
					if (desTAnim == BOTH_STAND9)
						desTAnim = BOTH_STAND9;
					else
						desTAnim = BOTH_STAND6;
				}

				if (!(pm->cmd.buttons & BUTTON_ATTACK) || pm->ps->weapon == WP_NONE)
				{ //don't do this while holding attack
					if (pm->ps->torsoAnim != desTAnim)
					{
						PM_StartTorsoAnim(desTAnim);
					}
				}
			}
		}
	}

	if (pm->ps->clientNum >= MAX_CLIENTS &&
		pm_entSelf &&
		pm_entSelf->s.NPC_class == CLASS_VEHICLE)
	{//we are a vehicle
		veh = pm_entSelf;
	}

	if (PM_DoChargedWeapons())
	{
		// JKG - Check for cookable grenades (you must press both to start its event)
		if (weaponData->hasCookAbility &&
			pm->ps->clientNum < MAX_CLIENTS &&
			jkg_didGrenadeCook[pm->ps->clientNum] == 0 &&
			(pm->cmd.buttons & BUTTON_ATTACK) &&
			(pm->cmd.buttons & BUTTON_IRONSIGHTS))
		{
			PM_AddEvent(EV_GRENADE_COOK);
			jkg_didGrenadeCook[pm->ps->clientNum] = 1;
		}

		// In some cases the charged weapon code may want us to short circuit the rest of the firing code
		return;
	}

	// JKG - Remove cook nade flag.
	if (jkg_didGrenadeCook[pm->ps->clientNum])
	{
		jkg_didGrenadeCook[pm->ps->clientNum] = 0;
	}

	// check for fire
	if (!(pm->cmd.buttons & BUTTON_ATTACK))
	{
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	if (pm->ps->weapon == 0)
	{
		return;
	}

	if (pm->ps->heat >= weaponData->firemodes[pm->ps->firingMode].maxHeat)
	{
		// Overheated, we can't fire
		if(!pm->ps->overheated)
			PM_AddEvent(EV_OVERHEATED);  //do a sound event

		pm->ps->overheated = true;
		pm->ps->shotsRemaining = 0;
		return;
	}

	if (pm->ps->heat >= weaponData->firemodes[pm->ps->firingMode].heatThreshold && pm->ps->overheated)
	{
		//need to cool down to heat threshold before we can fire again, still overheated
		return;
	}

	if (pm->ps->eFlags & EF_JETPACK_FLAMING && pm->cmd.upmove == 0)
	{
		// Sprinting forward while using jetpack, can't fire.
		return;
	}
	
	// JKG: Semi-automatic
	if ( pm->ps->shotsRemaining & SHOTS_TOGGLEBIT )
	{
		if ( weaponData->firemodes[pm->ps->firingMode].firingType == FT_SEMI )
		{
			return;
		}
		else if ( weaponData->firemodes[pm->ps->firingMode].firingType == FT_BURST )
		{
			pm->ps->shotsRemaining = weaponData->firemodes[pm->ps->firingMode].shotsPerBurst & ~SHOTS_TOGGLEBIT;
		}
	}

	/* Can't use zoom when the zoom mode is still locked */
	if (weaponData->zoomType != ZOOM_NONE && (pm->cmd.buttons & BUTTON_IRONSIGHTS) && !pm->ps->zoomLocked)
	{
		return;
	}

	/* Can't use zoom mode when you're using binoculars */
	if (weaponData->zoomType != ZOOM_NONE && (pm->cmd.buttons & BUTTON_IRONSIGHTS) && pm->ps->zoomMode == 2)
	{
		return;
	}

	if (pm->ps->weapon == WP_EMPLACED_GUN)
	{
		addTime = weaponData->firemodes[pm->ps->firingMode].delay;
		pm->ps->weaponTime += addTime;
		PM_AddEvent( EV_FIRE_WEAPON );
		return;
	}

	doStdAnim = 0;
#ifndef _GAME
	if (pm->ps->weapon == WP_MELEE)
#else //!_GAME
	if (pm->ps->weapon == WP_MELEE
		// UQ1: NPCs can hit you with their rifle butt at close range...
		|| (g_entities[pm->ps->clientNum].s.eType == ET_NPC 
			&& pm->ps->weapon != WP_SABER 
			&& g_entities[pm->ps->clientNum].enemy
			&& Distance(g_entities[pm->ps->clientNum].enemy->r.currentOrigin, g_entities[pm->ps->clientNum].r.currentOrigin) <= 72))
#endif //_GAME
	{ //special anims for standard melee attacks
		//Alternate between punches and use the anim length as weapon time.
		if (!pm->ps->m_iVehicleNum)
		{ //if riding a vehicle don't do this stuff at all
			if (pm->debugMelee &&
				(pm->cmd.buttons & BUTTON_ATTACK) &&
				(pm->cmd.buttons & BUTTON_IRONSIGHTS))
			{ //ok, grapple time
				return;
			}
			else if ( pm->cmd.buttons & BUTTON_IRONSIGHTS && pm->ps->forcePower >= bgConstants.staminaDrains.minKickThreshold )
			{ //kicks
				if (!BG_KickingAnim(pm->ps->torsoAnim) &&
					!BG_KickingAnim(pm->ps->legsAnim))
				{
					int kickMove = PM_KickMoveForConditions();
					if (kickMove == LS_HILT_BASH)
					{ //yeah.. no hilt to bash with!
						kickMove = LS_KICK_F;
					}

					if (kickMove != -1)
					{
						if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
						{//if in air, convert kick to an in-air kick
							float gDist = PM_GroundDistance();
							//let's only allow air kicks if a certain distance from the ground
							//it's silly to be able to do them right as you land.
							//also looks wrong to transition from a non-complete flip anim...
							if ((!BG_FlippingAnim( pm->ps->legsAnim ) || pm->ps->legsTimer <= 0) &&
								gDist > 64.0f && //strict minimum
								gDist > (-pm->ps->velocity[2])-64.0f //make sure we are high to ground relative to downward velocity as well
								)
							{
								switch ( kickMove )
								{
								case LS_KICK_F:
									kickMove = LS_KICK_F_AIR;
									break;
								case LS_KICK_B:
									kickMove = LS_KICK_B_AIR;
									break;
								case LS_KICK_R:
									kickMove = LS_KICK_R_AIR;
									break;
								case LS_KICK_L:
									kickMove = LS_KICK_L_AIR;
									break;
								default: //oh well, can't do any other kick move while in-air
									kickMove = -1;
									break;
								}
							}
							else
							{ //off ground, but too close to ground
								kickMove = -1;
							}
						}
					}

					if (kickMove != -1)
					{
						int kickAnim = SaberStances[pm->ps->fd.saberAnimLevel].moves[kickMove].anim;
//						int kickAnim = saberMoveData[kickMove].animToUse;

						if (kickAnim != -1)
						{
							PM_SetAnim(SETANIM_BOTH, kickAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
							if (pm->ps->legsAnim == kickAnim)
							{
								pm->ps->weaponTime = pm->ps->legsTimer;
								return;
							}
						}
					}
				}

				//if got here then no move to do so put torso into leg idle or whatever
				if (pm->ps->torsoAnim != pm->ps->legsAnim)
				{
					PM_SetAnim(SETANIM_BOTH, pm->ps->legsAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
					pm->ps->forcePower -= bgConstants.staminaDrains.lossFromKicking;
				}
				pm->ps->weaponTime = 0;
				return;
			}
			else if( pm->ps->forcePower >= bgConstants.staminaDrains.minPunchThreshold || pm->ps->torsoAnim == BOTH_MELEE1 )
			{ //just punch. We cannot start a melee combo if we are under 10 force power.
				int desTAnim = BOTH_MELEE1;
				if (pm->ps->torsoAnim == BOTH_MELEE1)
				{
#ifdef _GAME
				if ((pm->ps->weapon == WP_MELEE
					// UQ1: NPCs can hit you with their rifle butt at close range...
					|| (g_entities[pm->ps->clientNum].s.eType == ET_NPC 
						&& pm->ps->weapon != WP_SABER 
						&& g_entities[pm->ps->clientNum].enemy
						&& Distance(g_entities[pm->ps->clientNum].enemy->r.currentOrigin, g_entities[pm->ps->clientNum].r.currentOrigin) <= 72)))
#endif //_GAME
					desTAnim = BOTH_MELEE2;
				}
				PM_StartTorsoAnim( desTAnim );

				if (pm->ps->torsoAnim == desTAnim)
				{
					pm->ps->weaponTime = pm->ps->torsoTimer;
				}
			}
		}
	}
	else
	{
		doStdAnim = 1;
		//PM_StartTorsoAnim( WeaponAttackAnim[pm->ps->weapon] );
	}

	amount = weaponData->firemodes[pm->ps->firingMode].cost;
	JKG_ApplyAmmoOverride(amount, ammoTable[pm->ps->ammoType].overrides.ammocost);

	// take an ammo away if not infinite
	if ( (pm->ps->clientNum < MAX_CLIENTS || pm_entSelf->s.eType == ET_NPC) && weaponData->firemodes[pm->ps->firingMode].clipSize != -1 )
	{
		// enough energy to fire this weapon?
		// Jedi Knight Galaxies - Check clip also
		if ( weaponData->firemodes[pm->ps->firingMode].clipSize )
		{
			if ((pm->ps->stats[STAT_AMMO] - amount) >= 0) 
			{
				pm->ps->stats[STAT_AMMO] -= amount;
			}
			else	// Not enough energy
			{
				// Switch weapons
				if (pm->ps->weapon != WP_DET_PACK || !pm->ps->hasDetPackPlanted)
				{
					PM_AddEventWithParm( EV_NOAMMO, WP_NUM_WEAPONS+pm->ps->weapon );
					pm->ps->shotsRemaining = 0;
					pm->ps->weaponstate = WEAPON_READY; // Abort charging the weapon (if it is charging to begin with)
					if (pm->ps->weaponTime < 500)
					{
						pm->ps->weaponTime += 500;
					}
				}
				return;
			}
		} else {
			if ((pm->ps->stats[STAT_TOTALAMMO] - amount) >= 0)
			{
#ifdef _GAME
				gentity_t *Gself = &g_entities[pm->ps->clientNum];
				if (weaponData->firemodes[pm->ps->firingMode].useQuantity) {
					BG_AdjustItemStackQuantity(Gself, pm->cmd.invensel, -amount);
				}
				else {
					Gself->client->ammoTable[pm->ps->ammoType] -= amount;
				}
#endif
				pm->ps->stats[STAT_TOTALAMMO] -= amount;
			}
			else	// Not enough energy
			{
				// Switch weapons
				if (pm->ps->weapon != WP_DET_PACK || !pm->ps->hasDetPackPlanted)
				{
					PM_AddEventWithParm( EV_NOAMMO, WP_NUM_WEAPONS+pm->ps->weapon );
					pm->ps->shotsRemaining = 0;
					pm->ps->weaponstate = WEAPON_READY;		// Abort charging the weapon (if it is charging to begin with)
					if (pm->ps->weaponTime < 500)
					{
						pm->ps->weaponTime += 500;
					}
				}
				return;
			}
		}
	}

	// Increase heat
	pm->ps->heat += weaponData->firemodes[pm->ps->firingMode].heatGenerated;

	// Warn that we're about to overheat
	if (pm->ps->heat >= weaponData->firemodes[pm->ps->firingMode].heatThreshold)
	{
		PM_AddEvent(EV_HEATCRIT);  //this might be a bit too spammy, how to do less often? --futuza
	}

	// If we get here, we got the green light to fire the weapon
	pm->ps->weaponstate = WEAPON_FIRING;

	if (doStdAnim) {
		if( pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK )
		{
			PM_StartTorsoAnim( weaponData->anims.sightsFiring.torsoAnim );
		}
		else
		{
			PM_StartTorsoAnim( weaponData->anims.firing.torsoAnim );
		}
		pm->ps->torsoFlip = qtrue;
	}
	if ( pm->ps->weapon != WP_MELEE || !pm->ps->m_iVehicleNum )
	{
		PM_AddEvent( EV_FIRE_WEAPON );
	}

	switch ( weaponData->firemodes[pm->ps->firingMode].firingType )
	{
		case FT_AUTOMATIC:
			addTime = weaponData->firemodes[pm->ps->firingMode].delay;
			JKG_ApplyAmmoOverride(addTime, ammoTable[pm->ps->ammoType].overrides.fireDelay);
			break;
		        
		case FT_SEMI:
			addTime = weaponData->firemodes[pm->ps->firingMode].delay;
			JKG_ApplyAmmoOverride(addTime, ammoTable[pm->ps->ammoType].overrides.fireDelay);
			pm->ps->shotsRemaining = SHOTS_TOGGLEBIT;
			pm->ps->torsoTimer = addTime + 100;
			break;
		        
		case FT_BURST:
			if ( (pm->ps->shotsRemaining & ~SHOTS_TOGGLEBIT) == 1 )
			{
				addTime = weaponData->firemodes[pm->ps->firingMode].delay;
				pm->ps->shotsRemaining = SHOTS_TOGGLEBIT;
			}
			else
			{
				addTime = weaponData->firemodes[pm->ps->firingMode].burstFireDelay;
				pm->ps->shotsRemaining = (pm->ps->shotsRemaining - 1) & ~SHOTS_TOGGLEBIT;
			}
			JKG_ApplyAmmoOverride(addTime, ammoTable[pm->ps->ammoType].overrides.fireDelay);
			pm->ps->torsoTimer = addTime;
			break;		
	}

	if (pm->ps->fd.forcePowersActive & (1 << FP_RAGE))
	{
		addTime *= 0.75f;
	}
	else if (pm->ps->fd.forceRageRecoveryTime > pm->cmd.serverTime)
	{
		addTime *= 1.5f;
	}

	pm->ps->weaponTime += addTime;
}

/*
================
PM_Animate
================
*/

static void PM_Animate( void ) {
	if ( pm->cmd.buttons & BUTTON_GESTURE ) {
		if (pm->ps->m_iVehicleNum)
		{ //eh, fine, clear it
			if (pm->ps->forceHandExtendTime < pm->cmd.serverTime)
			{
				pm->ps->forceHandExtend = HANDEXTEND_NONE;
			}
		}

		if ( pm->ps->torsoTimer < 1 && pm->ps->forceHandExtend == HANDEXTEND_NONE &&
			pm->ps->legsTimer < 1 && pm->ps->weaponTime < 1 && pm->ps->saberLockTime < pm->cmd.serverTime) {

			pm->ps->forceHandExtend = HANDEXTEND_TAUNT;

			//FIXME: random taunt anims?
			pm->ps->forceDodgeAnim = BOTH_ENGAGETAUNT;

			pm->ps->forceHandExtendTime = pm->cmd.serverTime + 1000;
			
			//pm->ps->weaponTime = 100;

			PM_AddEvent( EV_TAUNT );
		}
	}
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if ( pm->ps->legsTimer > 0 ) {
		pm->ps->legsTimer -= pml.msec;
		if ( pm->ps->legsTimer < 0 ) {
			pm->ps->legsTimer = 0;
		}
	}

	if ( pm->ps->torsoTimer > 0 ) {
		pm->ps->torsoTimer -= pml.msec;
		if ( pm->ps->torsoTimer < 0 ) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
	short		temp;
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return;		// no view changes at all
	}

	// JKGalaxies
	if ( ps->pm_type == PM_LOCK || ps->pm_type == PM_NOMOVE ) {
		// No view changes
		return;
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
		return;		// no view changes at all
	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];

		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}

		ps->viewangles[i] = SHORT2ANGLE(temp);
	}
}

//-------------------------------------------
void PM_AdjustAttackStates( pmove_t *pmove )
//-------------------------------------------
{
	int amount = 0;
	qboolean overheated = qfalse;
	weaponData_t *weapon = GetWeaponData (pmove->ps->weapon, pmove->ps->weaponVariation);
	qboolean primFireDown;// = (pmove->cmd.buttons & BUTTON_ATTACK);
	
	// JKG: Keep attack button 'pressed' until no more shots
	// are remaining.
	if ( pmove->ps->shotsRemaining & ~SHOTS_TOGGLEBIT )
	{
	    if ( pmove->ps->eFlags & EF_FIRING )
	    {
		if ( weapon->firemodes[pmove->ps->firingMode].firingType == FT_BURST )
	        {
	            pmove->cmd.buttons |= BUTTON_ATTACK;
	        }
	    }
	}
	
	primFireDown = (pmove->cmd.buttons & BUTTON_ATTACK);
	
	// Iron sights!
	if ( (pmove->cmd.buttons & BUTTON_IRONSIGHTS || pmove->ps->isInSights) && 
		!(pmove->ps->sprintTime & SPRINT_MSB) && pmove->ps->ironsightsDebounceStart == 0 &&
		!(pmove->ps->pm_flags & PMF_ROLLING) )
	{
	    if ( !(pmove->ps->ironsightsTime & IRONSIGHTS_MSB) )
	    {
	        //Com_Printf ("%d: Bringing ironsights up\n", pmove->cmd.serverTime);
	        pmove->ps->ironsightsTime = pmove->cmd.serverTime | IRONSIGHTS_MSB;
	    }
	}
	else if ( pmove->ps->ironsightsTime & IRONSIGHTS_MSB )
	{
	    //Com_Printf ("%d: Bringing ironsights down\n", pmove->cmd.serverTime);
	    pmove->ps->ironsightsTime = pmove->cmd.serverTime & ~IRONSIGHTS_MSB;
	}

	else if( BG_IsSprinting (pmove->ps, &pmove->cmd, qtrue) && !(pmove->ps->ironsightsTime & IRONSIGHTS_MSB))
	{
	    if ( !(pmove->ps->sprintTime & SPRINT_MSB) )
	    {
	        //Com_Printf ("%d: Bringing ironsights up\n", pmove->cmd.serverTime);
	        pmove->ps->sprintTime = pmove->cmd.serverTime | SPRINT_MSB;
	    }
	}
	else if ( pmove->ps->sprintTime & SPRINT_MSB )
	{
		pmove->ps->sprintTime = pmove->cmd.serverTime & ~SPRINT_MSB;
	}

	// Check if we're in an ironsights transition...
	if( (pmove->ps->ironsightsTime & ~IRONSIGHTS_MSB)+ weapon->ironsightsTime > pmove->cmd.serverTime &&
		pmove->ps->weapon != WP_SABER && pmove->ps->weapon != WP_MELEE && pmove->ps->weapon != WP_NONE
		&& pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK)
	{	// We don't want saber or melee to trigger smoothness
		//Com_Printf("Ironsights transition\n");
		pmove->ps->sightsTransition = true;
	}
	else
	{
		pmove->ps->sightsTransition = false;
	}
	
	// get ammo usage
	if (primFireDown)
	{
		amount = pmove->ps->stats[STAT_AMMO] - weapon->firemodes[pmove->ps->firingMode].cost;
	}

	if ( weapon->zoomType != ZOOM_NONE )
	{
		weaponData_t *wp = GetWeaponData( pmove->ps->weapon, pmove->ps->weaponVariation );
		if ( pmove->ps->weaponstate == WEAPON_READY || pmove->ps->weaponstate == WEAPON_FIRING ||
			pmove->ps->weaponstate == WEAPON_IDLE || pmove->ps->weaponstate == WEAPON_CHARGING || // fix ADS charging bug --eez
			pmove->ps->weaponstate == WEAPON_ROLLFIRING) //you can now fire while rolling (good luck hitting anything)
	    	{
			if ( (pmove->cmd.buttons & BUTTON_IRONSIGHTS || pmove->ps->isInSights) &&
		            (pmove->ps->ironsightsTime & IRONSIGHTS_MSB) &&
					(((pmove->ps->ironsightsTime & ~IRONSIGHTS_MSB) + wp->ironsightsTime + 50) <= pmove->cmd.serverTime && pmove->ps->weapon != WP_SABER) )
		    	{
			    	// We just pressed the alt-fire key
			    	if ( !pmove->ps->zoomMode && pmove->ps->pm_type != PM_DEAD )
			    	{
					    // not already zooming, so do it now
					    pmove->ps->zoomMode = 1;
					    pmove->ps->zoomFov = weapon->startZoomFov;//cg_fov.value;
					    pmove->ps->zoomLocked = qfalse;
					    pmove->ps->zoomLockTime = pmove->cmd.serverTime + 50;
				    
					    PM_AddEvent (EV_ZOOM);
			    	}
			    	else if (pmove->ps->zoomLockTime < pmove->cmd.serverTime)
		        	{
			       	 	// Not pressing zoom any more
			        	if ( pmove->ps->zoomMode )
			        	{
				        	if (pmove->ps->zoomMode == 1 && !pmove->ps->zoomLocked && weapon->zoomType == ZOOM_CONTINUOUS)
				        	{ //approximate what level the client should be zoomed at based on how long zoom was held
					       		 pmove->ps->zoomFov = ((pmove->cmd.serverTime+50) - pmove->ps->zoomLockTime) * (0.001f * weapon->zoomTime) / (weapon->startZoomFov - weapon->endZoomFov);
					        	if (pmove->ps->zoomFov > weapon->startZoomFov)
					        	{
						        	pmove->ps->zoomFov = weapon->startZoomFov;
					        	}
					        	if (pmove->ps->zoomFov < weapon->endZoomFov)
					        	{
						        	pmove->ps->zoomFov = weapon->endZoomFov;
					        	}
				        	}
				        	// were zooming in, so now lock the zoom
				        	pmove->ps->zoomLocked = qtrue;
			        	}
		        	}
		    }
		    else if ( (!(pmove->cmd.buttons & BUTTON_IRONSIGHTS) && !pmove->ps->isInSights) &&
		                pmove->ps->zoomLockTime < pmove->cmd.serverTime && pmove->ps->weapon != WP_SABER )
		    {
		        	if (pmove->ps->zoomMode == 1 && pmove->ps->zoomLockTime < pmove->cmd.serverTime)
			    	{ //check for == 1 so we can't turn binoculars off with disruptor alt fire
				    	// already zooming, so must be wanting to turn it off
				    	pmove->ps->zoomMode = 0;
				   	pmove->ps->zoomTime = pmove->ps->commandTime;
				    	pmove->ps->zoomLocked = qfalse;
				    
				    	PM_AddEvent (EV_ZOOM);
			    	}
		    }		    
		    
		    if ( primFireDown )
		    {
			    // If we are zoomed, we should switch the ammo usage to the alt-fire, otherwise, we'll
			    //	just use whatever ammo was selected from above
			    if ( pmove->ps->zoomMode )
			    {
					amount = pmove->ps->stats[STAT_AMMO] - weapon->firemodes[pmove->ps->firingMode].cost;
			    }
		    }
		}		
	}
	
	// JKG: Burst fire
	if ( !(pmove->ps->shotsRemaining & ~SHOTS_TOGGLEBIT) &&
			(primFireDown && !(pmove->ps->eFlags & EF_FIRING)) )
	{
	    if ( pmove->ps->weaponTime <= 0 )
	    {
	    	if ( weapon->firemodes[pmove->ps->firingMode].firingType == FT_BURST )
            {
				//if we overheat/are overheated
				if (pmove->ps->heat >= weapon->firemodes[pmove->ps->firingMode].maxHeat || pm->ps->overheated)
				{
					// Overheated, we can't fire
					if (!pm->ps->overheated)
						PM_AddEvent(EV_OVERHEATED);  //do a sound event

					pm->ps->overheated = true;

					pmove->ps->eFlags &= ~EF_FIRING;
					pm->cmd.buttons &= ~BUTTON_ATTACK;
					primFireDown = qfalse;
				}

				else
				{
					pmove->ps->shotsRemaining = weapon->firemodes[pmove->ps->firingMode].shotsPerBurst & ~SHOTS_TOGGLEBIT;
				}
            }
        }
        else
        {
			pm->cmd.buttons &= ~BUTTON_ATTACK;
			primFireDown = qfalse;
        }
	}
	// set the firing flag for continuous beam weapons, saber will fire even if out of ammo
	if ( !(pmove->ps->pm_flags & PMF_RESPAWNED) && 
			pmove->ps->pm_type != PM_INTERMISSION && 
			primFireDown && 
			( amount >= 0 || pmove->ps->weapon == WP_SABER ) &&
			// JKG: No firing while sprinting
			!BG_IsSprinting (pmove->ps, &pm->cmd, qtrue) &&
			!(pmove->ps->heat >= weapon->firemodes[pmove->ps->firingMode].maxHeat || pm->ps->overheated)
		)
	{
		pmove->ps->eFlags &= ~EF_ALT_FIRING;

		// This flag should always get set, even when alt-firing
		pmove->ps->eFlags |= EF_FIRING;
	} 
	else
	{
		// Clear 'em out
		pmove->ps->eFlags &= ~EF_FIRING;
		if ( pmove->ps->shotsRemaining & SHOTS_TOGGLEBIT )
		{
		    pmove->ps->shotsRemaining = 0;
		}
	}
	
	// JKG: No firing while sprinting
    if ( BG_IsSprinting (pmove->ps, &pmove->cmd, qtrue) )
    {
        pmove->cmd.buttons &= ~BUTTON_ATTACK;
    }
	
	if ( weapon->zoomType != ZOOM_NONE )
	{
		if ( pmove->cmd.buttons & BUTTON_IRONSIGHTS && pmove->ps->zoomMode == 1 && pmove->ps->zoomLocked )
		{
			pmove->cmd.buttons &= ~BUTTON_IRONSIGHTS;
		}
	}
}

void PM_CmdForRoll( playerState_t *ps, int anim, usercmd_t *pCmd )
{
	switch ( (anim) )
	{
		case BOTH_ROLL_F:
			pCmd->forwardmove = 127;
			pCmd->rightmove = 0;
			break;
		case BOTH_ROLL_B:
			pCmd->forwardmove = -127;
			pCmd->rightmove = 0;
			break;
		case BOTH_ROLL_R:
			pCmd->forwardmove = 0;
			pCmd->rightmove = 127;
			break;
		case BOTH_ROLL_L:
			pCmd->forwardmove = 0;
			pCmd->rightmove = -127;
			break;
		case BOTH_GETUP_BROLL_R:
			pCmd->forwardmove = 0;
			pCmd->rightmove = 48;
			//NOTE: speed is 400
			break;

		case BOTH_GETUP_FROLL_R:
			if ( ps->legsTimer <= 250 )
			{//end of anim
				pCmd->forwardmove = pCmd->rightmove = 0;
			}
			else
			{
				pCmd->forwardmove = 0;
				pCmd->rightmove = 48;
				//NOTE: speed is 400
			}
			break;

	case BOTH_GETUP_BROLL_L:
		pCmd->forwardmove = 0;
		pCmd->rightmove = -48;
		//NOTE: speed is 400
		break;

	case BOTH_GETUP_FROLL_L:
		if ( ps->legsTimer <= 250 )
		{//end of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else
		{
			pCmd->forwardmove = 0;
			pCmd->rightmove = -48;
			//NOTE: speed is 400
		}
		break;

	case BOTH_GETUP_BROLL_B:
		if ( ps->torsoTimer <= 250 )
		{//end of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else if ( PM_AnimLength( 0, (animNumber_t)ps->legsAnim ) - ps->torsoTimer < 350 )
		{//beginning of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else
		{
			//FIXME: ramp down over length of anim
			pCmd->forwardmove = -64;
			pCmd->rightmove = 0;
			//NOTE: speed is 400
		}
		break;

	case BOTH_GETUP_FROLL_B:
		if ( ps->torsoTimer <= 100 )
		{//end of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else if ( PM_AnimLength( 0, (animNumber_t)ps->legsAnim ) - ps->torsoTimer < 200 )
		{//beginning of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else
		{
			//FIXME: ramp down over length of anim
			pCmd->forwardmove = -64;
			pCmd->rightmove = 0;
			//NOTE: speed is 400
		}
		break;

	case BOTH_GETUP_BROLL_F:
		if ( ps->torsoTimer <= 550 )
		{//end of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else if ( PM_AnimLength( 0, (animNumber_t)ps->legsAnim ) - ps->torsoTimer < 150 )
		{//beginning of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else
		{
			pCmd->forwardmove = 64;
			pCmd->rightmove = 0;
			//NOTE: speed is 400
		}
		break;

	case BOTH_GETUP_FROLL_F:
		if ( ps->torsoTimer <= 100 )
		{//end of anim
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		else
		{
			//FIXME: ramp down over length of anim
			pCmd->forwardmove = 64;
			pCmd->rightmove = 0;
			//NOTE: speed is 400
		}
		break;
	//[KnockdownSys]
	case BOTH_LK_DL_ST_T_SB_1_L:
		//kicked backwards
		if ( ps->legsTimer < 3050//at least 10 frames in
			&& ps->legsTimer > 550 )//at least 6 frames from end
		{//move backwards
			pCmd->forwardmove = -64;
			pCmd->rightmove = 0;
		}
		else
		{
			pCmd->forwardmove = pCmd->rightmove = 0;
		}
		break;
	//[/KnockdownSys]
	}
	pCmd->upmove = 0;
}

qboolean PM_SaberInTransition( int move );

static void PM_CmdForSaberMoves(usercmd_t *ucmd)
{
	//DUAL FORWARD+JUMP+ATTACK
	if ( ( pm->ps->legsAnim == BOTH_JUMPATTACK6
			&& pm->ps->saberMove == LS_JUMPATTACK_DUAL )
		|| ( pm->ps->legsAnim == BOTH_BUTTERFLY_FL1 
			&& pm->ps->saberMove == LS_JUMPATTACK_STAFF_LEFT ) 
		|| ( pm->ps->legsAnim == BOTH_BUTTERFLY_FR1 
			&& pm->ps->saberMove == LS_JUMPATTACK_STAFF_RIGHT )
		|| ( pm->ps->legsAnim == BOTH_BUTTERFLY_RIGHT
			&& pm->ps->saberMove == LS_BUTTERFLY_RIGHT ) 
		|| ( pm->ps->legsAnim == BOTH_BUTTERFLY_LEFT 
			&& pm->ps->saberMove == LS_BUTTERFLY_LEFT ) )
	{
		int aLen = PM_AnimLength(0, BOTH_JUMPATTACK6);

		ucmd->forwardmove = ucmd->rightmove = ucmd->upmove = 0;

		if ( pm->ps->legsAnim == BOTH_JUMPATTACK6 )
		{ //dual stance attack
			if ( pm->ps->legsTimer >= 100 //not at end
				&& (aLen - pm->ps->legsTimer) >= 250 ) //not in beginning
			{ //middle of anim
				//push forward
				ucmd->forwardmove = 127;
			}

			if ( (pm->ps->legsTimer >= 900 //not at end
					&& aLen - pm->ps->legsTimer >= 950 ) //not in beginning
				|| ( pm->ps->legsTimer >= 1600 
					&& aLen - pm->ps->legsTimer >= 400 ) ) //not in beginning
			{ //one of the two jumps
				if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
				{ //still on ground?
					if ( pm->ps->groundEntityNum >= MAX_CLIENTS )
					{
						//jump!
						pm->ps->velocity[2] = 250;//400;
						pm->ps->fd.forceJumpZStart = pm->ps->origin[2];//so we don't take damage if we land at same height
						//pm->ps->pm_flags |= PMF_JUMPING;
						//FIXME: NPCs yell?
						PM_AddEvent(EV_JUMP);
						//G_SoundOnEnt( ent, CHAN_BODY, "sound/weapons/force/jump.wav" );
					}
				}
				else
				{ //FIXME: if this is the second jump, maybe we should just stop the anim?
				}
			}
		}
		else
		{ //saberstaff attacks
			float lenMin = 1700.0f;
			float lenMax = 1800.0f;

			aLen = PM_AnimLength(0, (animNumber_t)pm->ps->legsAnim);

			if (pm->ps->legsAnim == BOTH_BUTTERFLY_LEFT)
			{
				lenMin = 1200.0f;
				lenMax = 1400.0f;
			}

			//FIXME: don't slide off people/obstacles?
			if ( pm->ps->legsAnim == BOTH_BUTTERFLY_RIGHT
				|| pm->ps->legsAnim == BOTH_BUTTERFLY_LEFT )
			{
				if ( pm->ps->legsTimer > 450 )
				{
					switch ( pm->ps->legsAnim )
					{
						case BOTH_BUTTERFLY_LEFT:
							ucmd->rightmove = -127;
							break;
						case BOTH_BUTTERFLY_RIGHT:
							ucmd->rightmove = 127;
							break;
						default:
							break;
					}
				}
			}
			else
			{
				if ( pm->ps->legsTimer >= 100 //not at end
					&& aLen - pm->ps->legsTimer >= 250 )//not in beginning
				{//middle of anim
					//push forward
					ucmd->forwardmove = 127;
				}
			}
			
			if ( pm->ps->legsTimer >= lenMin && pm->ps->legsTimer < lenMax )     
			{//one of the two jumps
				if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
				{//still on ground?
					//jump!
					if (pm->ps->legsAnim == BOTH_BUTTERFLY_LEFT)
					{
						pm->ps->velocity[2] = 350;
					}
					else
					{
						pm->ps->velocity[2] = 250;
					}
					pm->ps->fd.forceJumpZStart = pm->ps->origin[2];//so we don't take damage if we land at same height
					//pm->ps->pm_flags |= PMF_JUMPING;//|PMF_SLOW_MO_FALL;
					//FIXME: NPCs yell?
					PM_AddEvent(EV_JUMP);
					//G_SoundOnEnt( ent, CHAN_BODY, "sound/weapons/force/jump.wav" );
				}
				else
				{//FIXME: if this is the second jump, maybe we should just stop the anim?
				}
			}
		}

		if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
		{//can only turn when your feet hit the ground
			PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, ucmd);
		}
		//rwwFIXMEFIXME: Bother with bbox resizing like sp?
	}
	//STAFF BACK+JUMP+ATTACK
	else if (pm->ps->saberMove == LS_A_BACKFLIP_ATK &&
		pm->ps->legsAnim == BOTH_JUMPATTACK7)
	{
		int aLen = PM_AnimLength(0, BOTH_JUMPATTACK7);

		if ( pm->ps->legsTimer > 800 //not at end
			&& aLen - pm->ps->legsTimer >= 400 )//not in beginning
		{//middle of anim
			if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
			{//still on ground?
				vec3_t yawAngles, backDir;

				//push backwards some?
				VectorSet( yawAngles, 0, pm->ps->viewangles[YAW]+180, 0 );
				AngleVectors( yawAngles, backDir, 0, 0 );
				VectorScale( backDir, 100, pm->ps->velocity );

				//jump!
				pm->ps->velocity[2] = 300;
				pm->ps->fd.forceJumpZStart = pm->ps->origin[2]; //so we don't take damage if we land at same height
				//pm->ps->pm_flags |= PMF_JUMPING;//|PMF_SLOW_MO_FALL;

				//FIXME: NPCs yell?
				PM_AddEvent(EV_JUMP);
				//G_SoundOnEnt( ent, CHAN_BODY, "sound/weapons/force/jump.wav" );
				ucmd->upmove = 0; //clear any actual jump command
			}
		}
		ucmd->forwardmove = ucmd->rightmove = ucmd->upmove = 0;
	}
	//STAFF/DUAL SPIN ATTACK
	else if (pm->ps->saberMove == LS_SPINATTACK ||
		pm->ps->saberMove == LS_SPINATTACK_DUAL)
	{
		ucmd->forwardmove = ucmd->rightmove = ucmd->upmove = 0;
		//lock their viewangles during these attacks.
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, ucmd);
	}
}

/*
================
BG_IsSprinting

================
*/
qboolean BG_IsSprinting(const playerState_t *ps, const usercmd_t *cmd, qboolean PMOVE)
{
	if (!(cmd->buttons & BUTTON_SPRINT) && !ps->isSprinting)
	{
		return qfalse;
	}

	if (ps->saberActionFlags & (1 << SAF_BLOCKING))
	{
		return qfalse;
	}

	if (ps->pm_type == PM_JETPACK)
	{
		return qfalse;
	}

	if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return qfalse;
	}

	if ((ps->ironsightsTime & IRONSIGHTS_MSB) && ps->weapon != WP_THERMAL && ps->weapon != WP_TRIP_MINE && ps->weapon != WP_DET_PACK)
	{	// Can't be sprinting while using iron sights (grenades do not count!)
		return qfalse;
	}

	//Can't sprint while cooking grenade
	if ((ps->weapon == WP_THERMAL || ps->weapon == WP_TRIP_MINE || ps->weapon == WP_DET_PACK) && (cmd->buttons & BUTTON_ATTACK))
	{
		return qfalse;
	}

	if (ps->weaponstate == WEAPON_RELOADING)
	{
		return false; // Cannot sprint while reloading.
	}


	// Only when on the ground.

	if (PMOVE)
	{
		if (!pml.groundPlane && (pm->cmd.serverTime - 350) > pm->cmd.serverTime)
		{
			return qfalse;
		}
		if (ps->pm_type == PM_FLOAT)
		{
			return qfalse;
		}
		if (fabsf(ps->velocity[2]) > 158.0f)
		{
			return qfalse;
		}
	}
#ifdef _GAME
	else if (fabsf(ps->velocity[2]) > 158.0f)
	{
		return qfalse;
	}
#endif


	if (BG_KnockdownAnim(ps->torsoAnim) || BG_RollingAnim(ps->torsoAnim) ||
		BG_SwimmingAnim(ps->torsoAnim))
	{
		return qfalse;
	}

	if (cmd->upmove < 0)
	{
		return qfalse;	// can't be done while crouching (fixme: use something else here like sneaking?)
	}

	if (cmd->forwardmove <= 0)
	{
		return qfalse; // MUST be moving, and moving forward.
	}

	if (ps->sprintMustWait)
	{
		return qfalse;
	}

	return qtrue;
}

/*
================
PmoveSingle

================
*/
void PmoveSingle (pmove_t *pmove) {
	qboolean stiffenedUp = qfalse;
	qboolean noAnimate = qfalse;

	pm = pmove;

	//set up these "global" bg ents
	pm_entSelf = PM_BGEntForNum(pm->ps->clientNum);

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if (pm->ps->pm_type == PM_FLOAT)
	{ //You get no control over where you go in grip movement
		stiffenedUp = qtrue;
	}
	else if (pm->ps->eFlags & EF_DISINTEGRATION)
	{
		stiffenedUp = qtrue;
	}
	else if ( BG_SaberLockBreakAnim( pm->ps->legsAnim )
		|| BG_SaberLockBreakAnim( pm->ps->torsoAnim ) 
		|| pm->ps->saberLockTime >= pm->cmd.serverTime )
	{//can't move or turn
		stiffenedUp = qtrue;
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
	}
	else if ( pm->ps->saberMove == LS_A_BACK || pm->ps->saberMove == LS_A_BACK_CR ||
		pm->ps->saberMove == LS_A_BACKSTAB || pm->ps->saberMove == LS_A_FLIP_STAB ||
		pm->ps->saberMove == LS_A_FLIP_SLASH || pm->ps->saberMove == LS_A_JUMP_T__B_ ||
		pm->ps->saberMove == LS_DUAL_LR || pm->ps->saberMove == LS_DUAL_FB || pm->ps->saberMove == LS_A_BACKFLIP_ATK)
	{
		if (pm->ps->legsAnim == BOTH_JUMPFLIPSTABDOWN ||
			pm->ps->legsAnim == BOTH_JUMPFLIPSLASHDOWN1)
		{ //flipover medium stance attack
			if (pm->ps->legsTimer < 1600 && pm->ps->legsTimer > 900)
			{
				pm->ps->viewangles[YAW] += pml.frametime*90.0f;
				PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
			}
		}
		stiffenedUp = qtrue;
	}
	else if ((pm->ps->legsAnim) == (BOTH_A2_STABBACK1) ||
		(pm->ps->legsAnim) == (BOTH_ATTACK_BACK) ||
		(pm->ps->legsAnim) == (BOTH_CROUCHATTACKBACK1) ||
		(pm->ps->legsAnim) == (BOTH_FORCELEAP2_T__B_) ||
		(pm->ps->legsAnim) == (BOTH_JUMPFLIPSTABDOWN) ||
		(pm->ps->legsAnim) == (BOTH_JUMPFLIPSLASHDOWN1))
	{
		stiffenedUp = qtrue;
	}
	else if (pm->ps->legsAnim == BOTH_ROLL_STAB)
	{
		stiffenedUp = qtrue;
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
	}
	else if (pm->ps->heldByClient)
	{
		stiffenedUp = qtrue;
	}
	else if (BG_KickMove(pm->ps->saberMove) || BG_KickingAnim(pm->ps->legsAnim))
	{
		stiffenedUp = qtrue;
	}
	else if (BG_InGrappleMove(pm->ps->torsoAnim))
	{
		stiffenedUp = qtrue;
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
	}
	else if ( pm->ps->saberMove == LS_STABDOWN_DUAL ||
			pm->ps->saberMove == LS_STABDOWN_STAFF ||
			pm->ps->saberMove == LS_STABDOWN)
	{//FIXME: need to only move forward until we bump into our target...?
		if (pm->ps->legsTimer < 800)
		{ //freeze movement near end of anim
			stiffenedUp = qtrue;
			PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
		}
		else
		{ //force forward til then
            pm->cmd.rightmove = 0;
			pm->cmd.upmove = 0;
			pm->cmd.forwardmove = 64;
		}
	}
	else if (pm->ps->saberMove == LS_PULL_ATTACK_STAB ||
		pm->ps->saberMove == LS_PULL_ATTACK_SWING)
	{
		stiffenedUp = qtrue;
	}
	else if ( BG_FullBodyTauntAnim( pm->ps->legsAnim )
		&& BG_FullBodyTauntAnim( pm->ps->torsoAnim ) )
	{
		if ( (pm->cmd.buttons&BUTTON_ATTACK)
			|| (pm->cmd.buttons&BUTTON_FORCEPOWER)
			|| (pm->cmd.buttons&BUTTON_FORCEGRIP)
			|| (pm->cmd.buttons&BUTTON_FORCE_LIGHTNING)
			|| (pm->cmd.buttons&BUTTON_FORCE_DRAIN)
			|| pm->cmd.upmove )
		{//stop the anim
			if ( pm->ps->legsAnim == BOTH_MEDITATE
				&& pm->ps->torsoAnim == BOTH_MEDITATE )
			{
				PM_SetAnim( SETANIM_BOTH, BOTH_MEDITATE_END, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else
			{
				pm->ps->legsTimer = pm->ps->torsoTimer = 0;
			}
			if ( pm->ps->forceHandExtend == HANDEXTEND_TAUNT )
			{
				pm->ps->forceHandExtend = 0;
			}
		}
		else 
		{
			if ( pm->ps->legsAnim == BOTH_MEDITATE )
			{
				if ( pm->ps->legsTimer < 100 )
				{
					pm->ps->legsTimer = 100;
				}
			}
			if ( pm->ps->torsoAnim == BOTH_MEDITATE )
			{
				if ( pm->ps->torsoTimer < 100 )
				{
					pm->ps->legsTimer = 100;
				}
				pm->ps->forceHandExtend = HANDEXTEND_TAUNT;
				pm->ps->forceHandExtendTime = pm->cmd.serverTime + 100;
			}
			if ( pm->ps->legsTimer > 0 || pm->ps->torsoTimer > 0 )
			{
				stiffenedUp = qtrue;
				PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
				pm->cmd.rightmove = 0;
				pm->cmd.upmove = 0;
				pm->cmd.forwardmove = 0;
				pm->cmd.buttons = 0;
			}
		}
	}
	else if ( pm->ps->legsAnim == BOTH_MEDITATE_END 
		&& pm->ps->legsTimer > 0 )
	{
		stiffenedUp = qtrue;
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
        pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
		pm->cmd.forwardmove = 0;
		pm->cmd.buttons = 0;
	}
	else if (pm->ps->legsAnim == BOTH_FORCELAND1 ||
		pm->ps->legsAnim == BOTH_FORCELANDBACK1 ||
		pm->ps->legsAnim == BOTH_FORCELANDRIGHT1 ||
		pm->ps->legsAnim == BOTH_FORCELANDLEFT1) 
	{ //can't move while in a force land
		stiffenedUp = qtrue;
	}

	if ( pm->ps->saberMove == LS_A_LUNGE )
	{//can't move during lunge
		pm->cmd.rightmove = pm->cmd.upmove = 0;
		if ( pm->ps->legsTimer > 500 )
		{
			pm->cmd.forwardmove = 127;
		}
		else
		{
			pm->cmd.forwardmove = 0;
		}
	}

	if ( pm->ps->saberMove == LS_A_JUMP_T__B_ )
	{//can't move during leap
		if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
		{//hit the ground
			pm->cmd.forwardmove = 0;
		}
		pm->cmd.rightmove = pm->cmd.upmove = 0;
	}

	if (pm->ps->emplacedIndex)
	{
		if (pm->cmd.forwardmove < 0 || PM_GroundDistance() > 32.0f)
		{
			pm->ps->emplacedIndex = 0;
			pm->ps->saberHolstered = 0;
		}
		else
		{
			stiffenedUp = qtrue;
		}
	}

	/* When charging such weapon, do not allow any jump */
	if ( GetWeaponData( pm->ps->weapon, pm->ps->weaponVariation )->zoomType != ZOOM_NONE && pm->ps->zoomMode == 1 )
	{
		pm->cmd.upmove = pm->cmd.upmove < 0 ? pm->cmd.upmove : 0;
	}

	if (stiffenedUp)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->ps->fd.forceGripCripple)
	{ //don't let attack or alt attack if being gripped I guess
		pm->cmd.buttons &= ~BUTTON_ATTACK;
		pm->cmd.buttons &= ~BUTTON_IRONSIGHTS;
	}

	if ( BG_InRoll( pm->ps, pm->ps->legsAnim ) )
	{ //can't roll unless you're able to move normally
		PM_CmdForRoll( pm->ps, pm->ps->legsAnim, &pm->cmd );
	}
	
	// Xy: Go back to walking speed when ironsights are up
	if ( pm->ps->ironsightsTime & IRONSIGHTS_MSB && pm->ps->weapon != WP_THERMAL && pm->ps->weapon != WP_TRIP_MINE && pm->ps->weapon != WP_DET_PACK )
	{
		pm->cmd.forwardmove = Q_min (Q_max (pm->cmd.forwardmove, -bgConstants.ironsightsMoveSpeed), bgConstants.ironsightsMoveSpeed);
		pm->cmd.rightmove = Q_min (Q_max (pm->cmd.rightmove, -bgConstants.ironsightsMoveSpeed), bgConstants.ironsightsMoveSpeed);			// neither should this...
	}

	if ( pm->ps->saberActionFlags & (1 << SAF_BLOCKING) && !(pm->cmd.buttons & BUTTON_WALKING) )
	{
		pm->cmd.forwardmove = Q_min (Q_max (pm->cmd.forwardmove, -bgConstants.blockingModeMoveSpeed), bgConstants.blockingModeMoveSpeed);	// this shouldn't be zero... o.o
		pm->cmd.rightmove = Q_min (Q_max (pm->cmd.rightmove, -bgConstants.blockingModeMoveSpeed), bgConstants.blockingModeMoveSpeed);
	}

	PM_CmdForSaberMoves(&pm->cmd);

	if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}
	
	// Walking AND sprinting = sprinting!
	if ( (pm->cmd.buttons & BUTTON_WALKING) && BG_IsSprinting (pm->ps, &pm->cmd, qtrue) )
	{
	    pm->cmd.buttons &= ~BUTTON_WALKING;
	    if ( pm->cmd.forwardmove > 0 )
	    {
	        pm->cmd.forwardmove = 127;
	    }
	    else if ( pm->cmd.forwardmove < 0 )
	    {
	        pm->cmd.forwardmove = -128;
	    }
	    
	    if ( pm->cmd.rightmove > 0 )
	    {
	        pm->cmd.rightmove = 127;
	    }
	    else if ( pm->cmd.rightmove < 0 )
	    {
	        pm->cmd.rightmove = -128;
	    }
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if ( abs( pm->cmd.forwardmove ) > bgConstants.walkingSpeed || abs( pm->cmd.rightmove ) > bgConstants.walkingSpeed ) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}

	// In certain situations, we may want to control which attack buttons are pressed and what kind of functionality
	//	is attached to them
	PM_AdjustAttackStates( pm );

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && 
		!( pm->cmd.buttons & BUTTON_ATTACK ) ) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = Com_Clampi(1, 200, pmove->cmd.serverTime - pm->ps->commandTime);
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001f;

	if ( pm->ps->legsAnim == BOTH_FORCEWALLRUNFLIP_ALT &&
		pm->ps->legsTimer > 0 )
	{
		vec3_t vFwd, fwdAng;
		VectorSet(fwdAng, 0.0f, pm->ps->viewangles[YAW], 0.0f);

		AngleVectors( fwdAng, vFwd, NULL, NULL );
		if ( pm->ps->groundEntityNum == ENTITYNUM_NONE )
		{
			float savZ = pm->ps->velocity[2];
			VectorScale( vFwd, 100, pm->ps->velocity );
			pm->ps->velocity[2] = savZ;
		}
		pm->cmd.forwardmove = pm->cmd.rightmove = pm->cmd.upmove = 0;
		PM_AdjustAnglesForWallRunUpFlipAlt( &pm->cmd );
	}

	PM_AdjustAngleForWallJump( pm->ps, &pm->cmd, qtrue );
	PM_AdjustAngleForWallRunUp( pm->ps, &pm->cmd, qtrue );
	PM_AdjustAngleForWallRun( pm->ps, &pm->cmd, qtrue );

	//[KnockdownSys]
	PM_AdjustAnglesForKnockdown( pm->ps, &pm->cmd );
	//[/KnockdownSys]

	if (pm->ps->saberMove == LS_A_JUMP_T__B_ || pm->ps->saberMove == LS_A_LUNGE ||
		pm->ps->saberMove == LS_A_BACK_CR || pm->ps->saberMove == LS_A_BACK ||
		pm->ps->saberMove == LS_A_BACKSTAB)
	{
		PM_SetPMViewAngle(pm->ps, pm->ps->viewangles, &pm->cmd);
	}

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->cmd.upmove < 10 && !(pm->ps->pm_flags & PMF_STUCK_TO_WALL)) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if ( pm->cmd.forwardmove < 0 ) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if ( pm->ps->pm_type >= PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->ps->saberLockTime >= pm->cmd.serverTime)
	{
		pm->cmd.upmove = 0;
		pm->cmd.forwardmove = 0;//50;
		pm->cmd.rightmove = 0;//*= 0.1f;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_CheckDuck ();
		if (!pm->noSpecMove)
		{
			PM_FlyMove ();
		}
		PM_DropTimers ();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		if (pm->ps->clientNum < MAX_CLIENTS)
		{
			PM_NoclipMove ();
			PM_DropTimers ();
			return;
		}
	}

	if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck ();

	// set groundentity
	PM_GroundTrace();

	// Jedi Knight Galaxies, PM_LOCK
	if (pm->ps->pm_type == PM_FREEZE || pm->ps->pm_type == PM_LOCK) {
		PM_Friction();
		if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
		{
			if (!(pm->ps->eFlags & EF_JETPACK_ACTIVE))
			{
				// if not in a jetpack, scale downward velocity by gravity
				pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
			}
			PM_SlideMove(!(pm->ps->eFlags & EF_JETPACK_ACTIVE));
		}
		PM_GroundTrace();
		return;		// no movement at all
	}

	if ( pm->ps->groundEntityNum != ENTITYNUM_NONE )
	{//on ground
		pm->ps->fd.forceJumpZStart = 0;
	}

	if ( pm->ps->pm_type == PM_DEAD ) {
		PM_DeadMove ();
	}

	PM_DropTimers();

	if (pm->ps->pm_type == PM_JETPACK) {
		PM_JetpackMove();
	}
	else if (pm->ps->pm_type == PM_FLOAT) {
		PM_FlyMove();
	}
	else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
		PM_WaterJumpMove();
	}
	else if (pm->waterlevel > 1) {
		PM_WaterMove();
	}
	else if (pml.walking) {
		PM_WalkMove();
	}
	else {
		PM_AirMove();
	}

	if (!noAnimate)
	{
		PM_Animate();
	}

	// set groundentity
	PM_GroundTrace();
	PM_SetWaterLevel();
	if (pm->cmd.forcesel != (byte)-1 && (pm->ps->fd.forcePowersKnown & (1 << pm->cmd.forcesel)))
	{
		pm->ps->fd.forcePowerSelected = pm->cmd.forcesel;
	}

	PM_Weapon();

	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap velocity to integer coordinates to save network bandwidth
	if ( !pm->pmove_float )
		trap->SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;
	qboolean	locked = qfalse;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	if (pmove->ps->fallingToDeath)
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.buttons = 0;
	}
	if (pmove->ps->pm_type == PM_NOMOVE) {
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
		pmove->cmd.buttons = 0;
		locked = qtrue;
		pmove->ps->pm_type = PM_NORMAL;		// Hack, i know, but this way we can still do the normal pmove stuff
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			// If its above 1000 we got a huge desync
			if ( msec > 66 && msec < 800 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		
		PmoveSingle( pmove );
		
		if ( pmove->ps->pm_flags & PMF_JUMP_HELD ) {
			pmove->cmd.upmove = 20;
		}
	}
	if (locked) {
		pmove->ps->pm_type = PM_NOMOVE;		// Restore it
	}
}


#define PLAYER_KNOCKDOWN_HOLD_EXTRA_TIME 0
//[KnockdownSys]
int PM_MinGetUpTime( playerState_t *ps )
{//racc - returns the animation timer level at which you're allowed to get up from a knockdown.  
	//IE the larger this number, the faster you can get up.
	//bgEntity_t *pEnt = pm_entSelf;
	if ( ps->legsAnim == BOTH_PLAYER_PA_3_FLY
			|| ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L
			|| ps->legsAnim == BOTH_RELEASED )
	{//special cases
		return 200;
	}
	//else if ( pEnt->s.NPC_class == CLASS_ALORA )
	//{//alora springs up very quickly from knockdowns!
	//	return 1000;
	//}
	//CoOpFixMe RAFIXME - impliment G_ControlledByPlayer
	else if ( ps->clientNum < MAX_CLIENTS )
	//else if ( (ps->clientNum < MAX_CLIENTS||G_ControlledByPlayer(ent)) )
	{//player can get up faster based on his/her force jump skill
		int getUpTime = PLAYER_KNOCKDOWN_HOLD_EXTRA_TIME;
		if ( ps->fd.forcePowerLevel[FP_LEVITATION] >= FORCE_LEVEL_3 )
		{
			return (getUpTime+400);//750
		}
		else if ( ps->fd.forcePowerLevel[FP_LEVITATION] == FORCE_LEVEL_2 )
		{
			return (getUpTime+200);//500
		}
		else if ( ps->fd.forcePowerLevel[FP_LEVITATION] == FORCE_LEVEL_1 )
		{
			return (getUpTime+100);//250
		}
		else
		{
			return getUpTime;
		}
	}
	return 200;
}


qboolean PM_InAttackRoll( int anim )
{//racc - anim in a melee attack roll.
	switch ( anim )
	{
	case BOTH_GETUP_BROLL_B:
	case BOTH_GETUP_BROLL_F:
	case BOTH_GETUP_FROLL_B:
	case BOTH_GETUP_FROLL_F:
		return qtrue;
	}
	return qfalse;
}


qboolean PM_CheckRollSafety( int anim, float testDist ) 
{//racc - requires PM access!  
	//This function checks to see if there's an obstruction that would
	//block the movement for the given roll anim.
	vec3_t	forward, right, testPos, angles;
	trace_t	trace;
	int		contents = (CONTENTS_SOLID|CONTENTS_BOTCLIP);

	if ( pm->ps->clientNum < MAX_CLIENTS )
	{//player
		contents |= CONTENTS_PLAYERCLIP;
	}
	else
	{//NPC
		contents |= CONTENTS_MONSTERCLIP;
	}
	if ( PM_InAttackRoll( pm->ps->legsAnim ) )
	{//we don't care if people are in the way, we'll knock them down!
		contents &= ~CONTENTS_BODY;
	}
	angles[PITCH] = angles[ROLL] = 0;
	angles[YAW] = pm->ps->viewangles[YAW];//Add ucmd.angles[YAW]?
	AngleVectors( angles, forward, right, NULL );

	switch ( anim )
	{
	case BOTH_GETUP_BROLL_R:
	case BOTH_GETUP_FROLL_R:
		VectorMA( pm->ps->origin, testDist, right, testPos );
		break;
	case BOTH_GETUP_BROLL_L:
	case BOTH_GETUP_FROLL_L:
		VectorMA( pm->ps->origin, -testDist, right, testPos );
		break;
	case BOTH_GETUP_BROLL_F:
	case BOTH_GETUP_FROLL_F:
		VectorMA( pm->ps->origin, testDist, forward, testPos );
		break;
	case BOTH_GETUP_BROLL_B:
	case BOTH_GETUP_FROLL_B:
		VectorMA( pm->ps->origin, -testDist, forward, testPos );
		break;
	default://FIXME: add normal rolls?  Make generic for any forced-movement anim?
		return qtrue;
		break;
	}

	pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, testPos, pm->ps->clientNum, contents );
	if ( trace.fraction < 1.0f 
		|| trace.allsolid
		|| trace.startsolid )
	{//inside something or will hit something
		return qfalse;
	}
	return qtrue;
}


extern qboolean BG_StabDownAnim( int anim );
qboolean PM_GoingToAttackDown( playerState_t *ps )
{//racc - is the given ps in an animation that is about to attack the ground?
	if ( BG_StabDownAnim( ps->torsoAnim )//stabbing downward
		|| ps->saberMove == LS_A_LUNGE//lunge
		|| ps->saberMove == LS_A_JUMP_T__B_//death from above
		|| ps->saberMove == LS_A_T2B//attacking top to bottom
		|| ps->saberMove == LS_S_T2B//starting at attack downward
		|| (PM_SaberInTransition( ps->saberMove ) && SaberStances[pm->ps->fd.saberAnimLevel].moves[ps->saberMove].startingQuadrant == Q_T) )//transitioning to a top to bottom attack
	{
		return qtrue;
	}
	return qfalse;
}


qboolean PM_LockedAnim( int anim );
qboolean PM_CrouchGetup( float crouchheight )
{//racc - recover from our current knockdown state into a crouch.  
	//This should work as long as we're in a known knockdown state.
	int	anim = -1;
	pm->maxs[2] = crouchheight;
	pm->ps->viewheight = crouchheight + STANDARD_VIEWHEIGHT_OFFSET;

	switch ( pm->ps->legsAnim )
	{
	case BOTH_KNOCKDOWN1:
	case BOTH_KNOCKDOWN2:
	case BOTH_KNOCKDOWN4:
	case BOTH_RELEASED:
	case BOTH_PLAYER_PA_3_FLY:
		anim = BOTH_GETUP_CROUCH_B1;
		break;
	case BOTH_KNOCKDOWN3:
	case BOTH_KNOCKDOWN5:
	case BOTH_LK_DL_ST_T_SB_1_L:
		anim = BOTH_GETUP_CROUCH_F1;
		break;
	}
	if ( anim == -1 )
	{//WTF? stay down?
		pm->ps->legsTimer = 100;//hold this anim for another 10th of a second
		return qfalse;
	}
	else
	{//get up into crouch anim
		if ( PM_LockedAnim( pm->ps->torsoAnim ) )
		{//need to be able to override this anim
			pm->ps->torsoTimer = 0;
		}
		if ( PM_LockedAnim( pm->ps->legsAnim ) )
		{//need to be able to override this anim
			pm->ps->legsTimer = 0;
		}
		PM_SetAnim( SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
		//RAFIXME - Impliment saberBounceMove?
		pm->ps->saberMove = LS_READY;//don't finish whatever saber anim you may have been in
		//pm->ps->saberMove = pm->ps->saberBounceMove = LS_READY;//don't finish whatever saber anim you may have been in
		pm->ps->saberBlocked = BLOCKED_NONE;
		return qtrue;
	}
}


extern qboolean PM_LockedAnim( int anim );
qboolean PM_CheckRollGetup( void )
{//racc - try getting up from a knockdown by using a getup roll move.
#ifdef _GAME
	gentity_t* self = &g_entities[pm->ps->clientNum];
#endif
	if ( pm->ps->legsAnim == BOTH_KNOCKDOWN1
		|| pm->ps->legsAnim == BOTH_KNOCKDOWN2
		|| pm->ps->legsAnim == BOTH_KNOCKDOWN3
		|| pm->ps->legsAnim == BOTH_KNOCKDOWN4
		|| pm->ps->legsAnim == BOTH_KNOCKDOWN5 
		|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L
		|| pm->ps->legsAnim == BOTH_PLAYER_PA_3_FLY
		|| pm->ps->legsAnim == BOTH_RELEASED )
	{//lying on back or front
		
		if ( (pm->ps->clientNum < MAX_CLIENTS //player
				//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED)) //can't do roll getups while fatigued.
				&& ( pm->cmd.rightmove //pressing left or right
					|| (pm->cmd.forwardmove &&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) ) ) //or pressing fwd/back and have force jump.
#ifdef _GAME
				|| ( (pm->ps->clientNum >= MAX_CLIENTS) 
					&& self->NPC //an NPC
#endif
		/* //KNOCKDOWNFIXME RAFIXME - Impliment PM_ControlledByPlayer?
		if ( ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer()) && ( pm->cmd.rightmove || (pm->cmd.forwardmove&&pm->ns->forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) ) )//player pressing left or right
			|| ( (pm->ps->clientNum >= MAX_CLIENTS&&!PM_ControlledByPlayer()) && pm->gent->NPC//an NPC
		*/
#ifdef _GAME
					&& pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0//have at least force jump 1
					&& self->enemy //I have an enemy
					&& self->enemy->client//a client
					&& self->enemy->enemy == self//he's mad at me!
					&& (PM_GoingToAttackDown( &self->enemy->client->ps )||!Q_irand(0,2))//he's attacking downward! (or we just feel like doing it this time)
					&& (/*(self->client&&self->client->NPC_class==CLASS_ALORA)||*/Q_irand( 0, RANK_CAPTAIN )<self->NPC->rank) ) //higher rank I am, more likely I am to roll away!
#endif
				)
		{//roll away!
			int anim;
			qboolean forceGetUp = qfalse;
			if ( pm->cmd.forwardmove > 0 )
			{
				if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3 
					|| pm->ps->legsAnim == BOTH_KNOCKDOWN5 
					|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
				{
					anim = BOTH_GETUP_FROLL_F;
				}
				else
				{
					anim = BOTH_GETUP_BROLL_F;
				}
				forceGetUp = qtrue;
			}
			else if ( pm->cmd.forwardmove < 0 )
			{
				if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3
					|| pm->ps->legsAnim == BOTH_KNOCKDOWN5
					|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
				{
					anim = BOTH_GETUP_FROLL_B;
				}
				else
				{
					anim = BOTH_GETUP_BROLL_B;
				}
				forceGetUp = qtrue;
			}
			else if ( pm->cmd.rightmove > 0 )
			{
				if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3 
					|| pm->ps->legsAnim == BOTH_KNOCKDOWN5
					|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
				{
					anim = BOTH_GETUP_FROLL_R;
				}
				else
				{
					anim = BOTH_GETUP_BROLL_R;
				}
			}
			else if ( pm->cmd.rightmove < 0 )
			{
				if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3 
					|| pm->ps->legsAnim == BOTH_KNOCKDOWN5
					|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
				{
					anim = BOTH_GETUP_FROLL_L;
				}
				else
				{
					anim = BOTH_GETUP_BROLL_L;
				}
			}
			else
			{//racc - If no move, then randomly select a roll move.  This only only works for NPCs.
				if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3
					|| pm->ps->legsAnim == BOTH_KNOCKDOWN5
					|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
				{//on your front
					anim = PM_irand_timesync( BOTH_GETUP_FROLL_B, BOTH_GETUP_FROLL_R );
				}
				else
				{
					anim = PM_irand_timesync( BOTH_GETUP_BROLL_B, BOTH_GETUP_BROLL_R );
				}
			}
			//RAFIXME - Impliment PM_ControlledByPlayer?
			if ( pm->ps->clientNum >= MAX_CLIENTS )
			//if ( (pm->ps->clientNum >= MAX_CLIENTS&&!PM_ControlledByPlayer()) )
			{//racc - NPCs do roll safety checks to make sure they can safely roll in that direction.
				if ( !PM_CheckRollSafety( anim, 64 ) )
				{//oops, try other one
					if ( pm->ps->legsAnim == BOTH_KNOCKDOWN3
						|| pm->ps->legsAnim == BOTH_KNOCKDOWN5
						|| pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
					{
						if ( anim == BOTH_GETUP_FROLL_R )
						{
							anim = BOTH_GETUP_FROLL_L;
						}
						else if ( anim == BOTH_GETUP_FROLL_F )
						{
							anim = BOTH_GETUP_FROLL_B;
						}
						else if ( anim == BOTH_GETUP_FROLL_B )
						{
							anim = BOTH_GETUP_FROLL_F;
						}
						else
						{
							anim = BOTH_GETUP_FROLL_L;
						}
						if ( !PM_CheckRollSafety( anim, 64 ) )
						{//neither side is clear, screw it
							return qfalse;
						}
					}
					else
					{
						if ( anim == BOTH_GETUP_BROLL_R )
						{
							anim = BOTH_GETUP_BROLL_L;
						}
						else if ( anim == BOTH_GETUP_BROLL_F )
						{
							anim = BOTH_GETUP_BROLL_B;
						}
						else if ( anim == BOTH_GETUP_FROLL_B )
						{
							anim = BOTH_GETUP_BROLL_F;
						}
						else
						{
							anim = BOTH_GETUP_BROLL_L;
						}
						if ( !PM_CheckRollSafety( anim, 64 ) )
						{//neither side is clear, screw it
							return qfalse;
						}
					}
				}
			}
			pm->cmd.rightmove = pm->cmd.forwardmove = 0;
			if ( PM_LockedAnim( pm->ps->torsoAnim ) )
			{//need to be able to override this anim
				pm->ps->torsoTimer = 0;
			}
			if ( PM_LockedAnim( pm->ps->legsAnim ) )
			{//need to be able to override this anim
				pm->ps->legsTimer = 0;
			}
			PM_SetAnim( SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			pm->ps->weaponTime = pm->ps->torsoTimer - 300;//don't attack until near end of this anim
			//RAFIXME - impliment saberBounceMove?
			pm->ps->saberMove = LS_READY;//don't finish whatever saber anim you may have been in
			//pm->ps->saberMove = pm->ps->saberBounceMove = LS_READY;//don't finish whatever saber anim you may have been in //SP Version
			pm->ps->saberBlocked = BLOCKED_NONE;
			if ( forceGetUp )
			{
#ifdef _GAME
				if ( self && self->client && self->client->playerTeam == NPCTEAM_ENEMY 
					&& self->NPC && self->NPC->blockedSpeechDebounceTime < level.time
					&& !Q_irand( 0, 1 ) )
				{//racc - evil NPCs sometimes taunt when they use the force to jump up from a knockdown.
					PM_AddEvent( Q_irand( EV_COMBAT1, EV_COMBAT3 ) );
					self->NPC->blockedSpeechDebounceTime = level.time + 1000;
				}
				G_PreDefSound(self->client->ps.origin, PDSOUND_FORCEJUMP);
				//G_SoundOnEnt( pm->gent, CHAN_BODY, "sound/weapons/force/jump.wav" ); //SP version
#endif
				//launch off ground?
				pm->ps->weaponTime = 300;//just to make sure it's cleared
			}
			return qtrue;
		}
	}
	return qfalse;
}


qboolean PM_GettingUpFromKnockDown( float standheight, float crouchheight )
{//racc - attempt to get up from a knockdown if we can/need to.
	//bgEntity_t *pEnt = pm_entSelf;
	int legsAnim = pm->ps->legsAnim;
	if ( legsAnim == BOTH_KNOCKDOWN1
		||legsAnim == BOTH_KNOCKDOWN2
		||legsAnim == BOTH_KNOCKDOWN3 
		||legsAnim == BOTH_KNOCKDOWN4
		||legsAnim == BOTH_KNOCKDOWN5 
		||legsAnim == BOTH_PLAYER_PA_3_FLY
		||legsAnim == BOTH_LK_DL_ST_T_SB_1_L
		||legsAnim == BOTH_RELEASED )
	{//in a knockdown
		int minTimeLeft = PM_MinGetUpTime( pm->ps );
		if ( pm->ps->legsTimer <= minTimeLeft )
		{//if only a quarter of a second left, allow roll-aways
			if ( PM_CheckRollGetup() )
			{//racc - decided to use a getup roll.
				pm->cmd.rightmove = pm->cmd.forwardmove = 0;
				return qtrue;
			}
		}
#ifdef _GAME
		if ( TIMER_Exists( &g_entities[pm->ps->clientNum], "noGetUpStraight" ) ) 
		{//racc - check for a npc don't-getup-right-now timer for this NPC.
			if ( !TIMER_Done2( &g_entities[pm->ps->clientNum], "noGetUpStraight", qtrue ) )
			{//not allowed to do straight get-ups for another few seconds
				if ( pm->ps->legsTimer <= minTimeLeft )
				{//hold it for a bit
					pm->ps->legsTimer = minTimeLeft+1;
				}
			}
		}
#endif
		if ( !pm->ps->legsTimer	 //our knockdown is over
			|| (pm->ps->legsTimer <= minTimeLeft //or we're strong enough to get up earlier.
				&& (pm->cmd.upmove>0 /*|| (pEnt->s.NPC_class==CLASS_ALORA)*/)) ) //and we're trying to get up
		{//done with the knockdown - FIXME: somehow this is allowing an *instant* getup...???
			//FIXME: if trying to crouch (holding button?), just get up into a crouch?
			if ( pm->cmd.upmove < 0 )
			{
				return PM_CrouchGetup( crouchheight );
			}
			else
			{
				trace_t	trace;
				// try to stand up
				pm->maxs[2] = standheight;
				pm->trace( &trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
				if ( !trace.allsolid )
				{//stand up
					int	anim = BOTH_GETUP1;
					qboolean forceGetUp = qfalse;
					pm->maxs[2] = standheight;
					pm->ps->viewheight = standheight + STANDARD_VIEWHEIGHT_OFFSET;
					//NOTE: the force power checks will stop fencers and grunts from getting up using force jump
					switch ( pm->ps->legsAnim )
					{
					case BOTH_KNOCKDOWN1:
						//RAFIXME - Impliment PM_ControlledByPlayer?
						if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 //has force jump
							//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED))  //player isn't fatigued.
							&& ( (pm->ps->clientNum < MAX_CLIENTS && pm->cmd.upmove > 0) //is a player trying to jump
								|| pm->ps->clientNum >= MAX_CLIENTS) ) //an NPC doesn't have to give a command to do this.
						//if ( (pm->ps->clientNum&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) || ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer())&&pm->cmd.upmove>0&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) )//FORCE_LEVEL_1) )FORCE_LEVEL_1) )
						{
							anim = PM_irand_timesync( BOTH_FORCE_GETUP_B1, BOTH_FORCE_GETUP_B6 );//NOTE: BOTH_FORCE_GETUP_B5 takes soe steps forward at end
							forceGetUp = qtrue;
						}
						else
						{
							anim = BOTH_GETUP1;
						}
						break;
					case BOTH_KNOCKDOWN2:
					case BOTH_PLAYER_PA_3_FLY:
						//RAFIXME - impliment PM_ControlledByPlayer?
						if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 //has force jump
							//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED))  //player isn't fatigued.
							&& ( (pm->ps->clientNum < MAX_CLIENTS && pm->cmd.upmove > 0) //is a player trying to jump
								|| pm->ps->clientNum >= MAX_CLIENTS) ) //an NPC doesn't have to give a command to do this.
						//if ( (pm->ps->clientNum&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) || ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer())&&pm->cmd.upmove>0&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) )//FORCE_LEVEL_1) )FORCE_LEVEL_1) )
						{
							anim = PM_irand_timesync( BOTH_FORCE_GETUP_B1, BOTH_FORCE_GETUP_B6 );//NOTE: BOTH_FORCE_GETUP_B5 takes soe steps forward at end
							forceGetUp = qtrue;
						}
						else
						{
							anim = BOTH_GETUP2;
						}
						break;
					case BOTH_KNOCKDOWN3:
						//RAFIXME - impliment PM_ControlledByPlayer?
						if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 //has force jump
							//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED))  //player isn't fatigued.
							&& ( (pm->ps->clientNum < MAX_CLIENTS && pm->cmd.upmove > 0) //is a player trying to jump
								|| pm->ps->clientNum >= MAX_CLIENTS) ) //an NPC doesn't have to give a command to do this.
						//if ( (pm->ps->clientNum&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) || ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer())&&pm->cmd.upmove>0&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) )//FORCE_LEVEL_1) )
						{
							anim = PM_irand_timesync( BOTH_FORCE_GETUP_F1, BOTH_FORCE_GETUP_F2 );
							forceGetUp = qtrue;
						}
						else
						{
							anim = BOTH_GETUP3;
						}
						break;
					case BOTH_KNOCKDOWN4:
					case BOTH_RELEASED:
						//RAFIXME - impliment PM_ControlledByPlayer?
						if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 //has force jump
							//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED))  //player isn't fatigued.
							&& ( (pm->ps->clientNum < MAX_CLIENTS && pm->cmd.upmove > 0) //is a player trying to jump
								|| pm->ps->clientNum >= MAX_CLIENTS) ) //an NPC doesn't have to give a command to do this.
						//if ( (pm->ps->clientNum&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) || ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer())&&pm->cmd.upmove>0&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) )//FORCE_LEVEL_1) )FORCE_LEVEL_1) )
						{
							anim = PM_irand_timesync( BOTH_FORCE_GETUP_B1, BOTH_FORCE_GETUP_B6 );//NOTE: BOTH_FORCE_GETUP_B5 takes soe steps forward at end
							forceGetUp = qtrue;
						}
						else
						{
							anim = BOTH_GETUP4;
						}
						break;
					case BOTH_KNOCKDOWN5:
					case BOTH_LK_DL_ST_T_SB_1_L:
						//RAFIXME - impliment PM_ControlledByPlayer?
						if ( pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0 //has force jump
							//&& !(pm->ps->userInt3 & (1 << FLAG_FATIGUED))  //player isn't fatigued.
							&& ( (pm->ps->clientNum < MAX_CLIENTS && pm->cmd.upmove > 0) //is a player trying to jump
								|| pm->ps->clientNum >= MAX_CLIENTS) ) //an NPC doesn't have to give a command to do this.
						//if ( (pm->ps->clientNum&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) || ((pm->ps->clientNum < MAX_CLIENTS||PM_ControlledByPlayer())&&pm->cmd.upmove>0&&pm->ps->fd.forcePowerLevel[FP_LEVITATION] > FORCE_LEVEL_0) )//FORCE_LEVEL_1) )FORCE_LEVEL_1) )
						{
							anim = PM_irand_timesync( BOTH_FORCE_GETUP_F1, BOTH_FORCE_GETUP_F2 );
							forceGetUp = qtrue;
						}
						else
						{
							anim = BOTH_GETUP5;
						}
						break;
					}
					//Com_Printf( "getupanim = %s\n", animTable[anim].name );
					if ( forceGetUp )
					{//racc - using the Force to get up.
#ifdef _GAME
						gentity_t *self = &g_entities[pm->ps->clientNum];
						if ( self && self->client && self->client->playerTeam == NPCTEAM_ENEMY 
							&& self->NPC && self->NPC->blockedSpeechDebounceTime < level.time
							&& !Q_irand( 0, 1 ) )
						{//racc - enemy bots talk a little smack if they
							PM_AddEvent( Q_irand( EV_COMBAT1, EV_COMBAT3 ) );
							self->NPC->blockedSpeechDebounceTime = level.time + 1000;
						}
						G_PreDefSound(self->client->ps.origin, PDSOUND_FORCEJUMP);
						//G_SoundOnEnt( pm->gent, CHAN_BODY, "sound/weapons/force/jump.wav" ); //SP version
#endif

						//launch off ground?
						pm->ps->weaponTime = 300;//just to make sure it's cleared
					}
					if ( PM_LockedAnim( pm->ps->torsoAnim ) )
					{//need to be able to override this anim
						pm->ps->torsoTimer = 0;
					}
					if ( PM_LockedAnim( pm->ps->legsAnim ) )
					{//need to be able to override this anim
						pm->ps->legsTimer = 0;
					}
					PM_SetAnim( SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					//RAFIXME - impliment saberBounceMove?
					pm->ps->saberMove = LS_READY;//don't finish whatever saber anim you may have been in
					//pm->ps->saberMove = pm->ps->saberBounceMove = LS_READY;//don't finish whatever saber anim you may have been in
					pm->ps->saberBlocked = BLOCKED_NONE;
					return qtrue;
				}
				else
				{
					return PM_CrouchGetup( crouchheight );
				}
			}
		}
		else
		{//racc - not ready to getup yet.  Just set the movement.
			if ( pm->ps->legsAnim == BOTH_LK_DL_ST_T_SB_1_L )
			{//racc - apprenently this move has a special cmd for it.
				PM_CmdForRoll( pm->ps, pm->ps->legsAnim, &pm->cmd );
			}
			else
			{
				pm->cmd.rightmove = pm->cmd.forwardmove = 0;
			}
		}
	}
	return qfalse;
}
//[/KnockdownSys]
