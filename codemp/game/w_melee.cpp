#include "g_local.h"
#include "bg_local.h"


#define STAFF_KICK_RANGE 16
extern void G_GetBoltPosition( gentity_t *self, int boltIndex, vec3_t pos, int modelIndex ); //NPC_utils.c

extern qboolean BG_InKnockDown( int anim );
qboolean G_KickDownable(gentity_t *ent)
{
	if (!d_saberKickTweak.integer)
	{
		return qtrue;
	}

	if (!ent || !ent->inuse || !ent->client)
	{
		return qfalse;
	}

	if (BG_InKnockDown(ent->client->ps.legsAnim) ||
		BG_InKnockDown(ent->client->ps.torsoAnim))
	{
		return qfalse;
	}

	if (ent->client->ps.weaponTime <= 0 &&
		ent->client->ps.weapon == WP_SABER &&
		ent->client->ps.groundEntityNum != ENTITYNUM_NONE)
	{
		return qfalse;
	}

	return qtrue;
}

void G_TossTheMofo(gentity_t *ent, vec3_t tossDir, float tossStr)
{
	if (!ent->inuse || !ent->client)
	{ //no good
		return;
	}

	if (ent->s.eType == ET_NPC && ent->s.NPC_class == CLASS_VEHICLE)
	{ //no, silly
		return;
	}

	VectorMA(ent->client->ps.velocity, tossStr, tossDir, ent->client->ps.velocity);
	ent->client->ps.velocity[2] = 200;
	if (ent->health > 0 && ent->client->ps.forceHandExtend != HANDEXTEND_KNOCKDOWN &&
		BG_KnockDownable(&ent->client->ps) &&
		G_KickDownable(ent))
	{ //if they are alive, knock them down I suppose
		ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
		ent->client->ps.forceHandExtendTime = level.time + 700;
		ent->client->ps.forceDodgeAnim = 0; //this toggles between 1 and 0, when it's 1 we should play the get up anim
		//ent->client->ps.quickerGetup = qtrue;
	}
}

//[KnockdownSys]
extern void G_ThrownDeathAnimForDeathAnim( gentity_t *hitEnt, vec3_t impactPoint );
//[/KnockdownSys]
gentity_t *G_KickTrace( gentity_t *ent, vec3_t kickDir, float kickDist, vec3_t kickEnd, int kickDamage, float kickPush )
{
	vec3_t	traceOrg, traceEnd, kickMins, kickMaxs;
	trace_t	trace;
	gentity_t	*hitEnt = NULL;
	VectorSet(kickMins, -2.0f, -2.0f, -2.0f);
	VectorSet(kickMaxs, 2.0f, 2.0f, 2.0f);
	//FIXME: variable kick height?
	if ( kickEnd && !VectorCompare( kickEnd, vec3_origin ) )
	{//they passed us the end point of the trace, just use that
		//this makes the trace flat
		VectorSet( traceOrg, ent->r.currentOrigin[0], ent->r.currentOrigin[1], kickEnd[2] );
		VectorCopy( kickEnd, traceEnd );
	}
	else
	{//extrude
		VectorSet( traceOrg, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]+ent->r.maxs[2]*0.5f );
		VectorMA( traceOrg, kickDist, kickDir, traceEnd );
	}

	if (d_saberKickTweak.integer)
	{
		trap->Trace( &trace, traceOrg, kickMins, kickMaxs, traceEnd, ent->s.number, MASK_SHOT, 0, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
	}
	else
	{
		trap->Trace( &trace, traceOrg, kickMins, kickMaxs, traceEnd, ent->s.number, MASK_SHOT, 0, 0, 0 );
	}

	//G_TestLine(traceOrg, traceEnd, 0x0000ff, 5000);
	if ( trace.fraction < 1.0f && !trace.startsolid && !trace.allsolid )
	{
		if (ent->client->jediKickTime > level.time)
		{
			if (trace.entityNum == ent->client->jediKickIndex)
			{ //we are hitting the same ent we last hit in this same anim, don't hit it again
				return NULL;
			}
		}
		ent->client->jediKickIndex = trace.entityNum;
		ent->client->jediKickTime = level.time + ent->client->ps.legsTimer;

		hitEnt = &g_entities[trace.entityNum];
		//FIXME: regardless of what we hit, do kick hit sound and impact effect
		//G_PlayEffect( "misc/kickHit", trace.endpos, trace.plane.normal );
		if ( ent->client->ps.torsoAnim == BOTH_A7_HILT )
		{
			G_Sound( ent, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/saber_slam" ) );
		}
		else
		{
			G_Sound( ent, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );
		}
		if ( hitEnt->inuse )
		{//we hit an entity
			//FIXME: don't hit same ent more than once per kick
			if ( hitEnt->takedamage )
			{//hurt it
				if (hitEnt->client)
				{
					hitEnt->client->ps.otherKiller = ent->s.number;
					hitEnt->client->ps.otherKillerDebounceTime = level.time + 10000;
					hitEnt->client->ps.otherKillerTime = level.time + 10000;
				}

				if (d_saberKickTweak.integer)
				{
					G_Damage( hitEnt, ent, ent, kickDir, trace.endpos, kickDamage*0.2f, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
				}
				else
				{
					G_Damage( hitEnt, ent, ent, kickDir, trace.endpos, kickDamage, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
				}
			}
			if (hitEnt->client  && hitEnt->takedamage && !(hitEnt->flags & FL_GODMODE)
				&& !(hitEnt->client->ps.pm_flags&PMF_TIME_KNOCKBACK) //not already flying through air?  Intended to stop multiple hits, but...
				&& G_CanBeEnemy(ent, hitEnt) )
			{//FIXME: this should not always work
				if ( hitEnt->health <= 0 )
				{//we kicked a dead guy
					//throw harder - FIXME: no matter how hard I push them, they don't go anywhere... corpses use less physics???
				//	G_Throw( hitEnt, kickDir, kickPush*4 );
					//see if we should play a better looking death on them
				//	G_ThrownDeathAnimForDeathAnim( hitEnt, trace.endpos );
					//[KnockdownSys]
					//reenabled SP code since the knockdown code now based on the SP code again.
					G_Throw( hitEnt, kickDir, kickPush*4 );
					//see if we should play a better looking death on them
					G_ThrownDeathAnimForDeathAnim( hitEnt, trace.endpos );
					//G_TossTheMofo(hitEnt, kickDir, kickPush*4.0f);
					//[/KnockdownSys]
				}
				else
				{
					
					G_Throw( hitEnt, kickDir, kickPush );
					if ( kickPush >= 75.0f && !Q_irand( 0, 2 ) )
					{
						G_Knockdown( hitEnt, ent, kickDir, 300, qtrue );
					}
					else
					{
						G_Knockdown( hitEnt, ent, kickDir, kickPush, qtrue );
					}					
				}
			}
		}
	}
	return (hitEnt);
}

void G_KickSomeMofos(gentity_t *ent)
{
	vec3_t	kickDir, kickEnd, fwdAngs;
	float animLength = BG_AnimLength( ent->localAnimIndex, (animNumber_t)ent->client->ps.legsAnim );
	float elapsedTime = (float)(animLength-ent->client->ps.legsTimer);
	float remainingTime = (animLength-elapsedTime);
	float kickDist = (ent->r.maxs[0]*1.5f)+STAFF_KICK_RANGE+8.0f;//fudge factor of 8
	int	  kickDamage = Q_irand(10, 15);//Q_irand( 3, 8 ); //since it can only hit a guy once now
	int	  kickPush = flrand( 50.0f, 100.0f );
	qboolean doKick = qfalse;
	renderInfo_t *ri = &ent->client->renderInfo;

	VectorSet(kickDir, 0.0f, 0.0f, 0.0f);
	VectorSet(kickEnd, 0.0f, 0.0f, 0.0f);
	VectorSet(fwdAngs, 0.0f, ent->client->ps.viewangles[YAW], 0.0f);

	//HMM... or maybe trace from origin to footRBolt/footLBolt?  Which one?  G2 trace?  Will do hitLoc, if so...
	if ( ent->client->ps.torsoAnim == BOTH_A7_HILT )
	{
		if ( elapsedTime >= 250 && remainingTime >= 250 )
		{//front
			doKick = qtrue;
			if ( ri->handRBolt != -1 )
			{//actually trace to a bolt
				G_GetBoltPosition( ent, ri->handRBolt, kickEnd, 0 );
				VectorSubtract( kickEnd, ent->client->ps.origin, kickDir );
				kickDir[2] = 0;//ah, flatten it, I guess...
				VectorNormalize( kickDir );
			}
			else
			{//guess
				AngleVectors( fwdAngs, kickDir, NULL, NULL );
			}
		}
	}
	else
	{
		switch ( ent->client->ps.legsAnim )
		{
		case BOTH_GETUP_BROLL_B:
		case BOTH_GETUP_BROLL_F:
		case BOTH_GETUP_FROLL_B:
		case BOTH_GETUP_FROLL_F:
			if ( elapsedTime >= 250 && remainingTime >= 250 )
			{//front
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->client->ps.origin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
			}
			break;
		case BOTH_A7_KICK_F_AIR:
		case BOTH_A7_KICK_B_AIR:
		case BOTH_A7_KICK_R_AIR:
		case BOTH_A7_KICK_L_AIR:
			if ( elapsedTime >= 100 && remainingTime >= 250 )
			{//air
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
			}
			break;
		case BOTH_A7_KICK_F:
			//FIXME: push forward?
			if ( elapsedTime >= 250 && remainingTime >= 250 )
			{//front
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
			}
			break;
		case BOTH_A7_KICK_B:
			//FIXME: push back?
			if ( elapsedTime >= 250 && remainingTime >= 250 )
			{//back
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
			}
			break;
		case BOTH_A7_KICK_R:
			//FIXME: push right?
			if ( elapsedTime >= 250 && remainingTime >= 250 )
			{//right
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
				}
			}
			break;
		case BOTH_A7_KICK_L:
			//FIXME: push left?
			if ( elapsedTime >= 250 && remainingTime >= 250 )
			{//left
				doKick = qtrue;
				if ( ri->footLBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footLBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
				}
				else
				{//guess
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
			}
			break;
		case BOTH_A7_KICK_S:
			kickPush = flrand( 75.0f, 125.0f );
			if ( ri->footRBolt != -1 )
			{//actually trace to a bolt
				if ( elapsedTime >= 550 
					&& elapsedTime <= 1050 )
				{
					doKick = qtrue;
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
					//NOTE: have to fudge this a little because it's not getting enough range with the anim as-is
					VectorMA( kickEnd, 8.0f, kickDir, kickEnd );
				}
			}
			else
			{//guess
				if ( elapsedTime >= 400 && elapsedTime < 500 )
				{//front
					doKick = qtrue;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
				else if ( elapsedTime >= 500 && elapsedTime < 600 )
				{//front-right?
					doKick = qtrue;
					fwdAngs[YAW] += 45;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
				else if ( elapsedTime >= 600 && elapsedTime < 700 )
				{//right
					doKick = qtrue;
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
				}
				else if ( elapsedTime >= 700 && elapsedTime < 800 )
				{//back-right?
					doKick = qtrue;
					fwdAngs[YAW] += 45;
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
				}
				else if ( elapsedTime >= 800 && elapsedTime < 900 )
				{//back
					doKick = qtrue;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
				else if ( elapsedTime >= 900 && elapsedTime < 1000 )
				{//back-left?
					doKick = qtrue;
					fwdAngs[YAW] += 45;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
				else if ( elapsedTime >= 1000 && elapsedTime < 1100 )
				{//left
					doKick = qtrue;
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
				else if ( elapsedTime >= 1100 && elapsedTime < 1200 )
				{//front-left?
					doKick = qtrue;
					fwdAngs[YAW] += 45;
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
			}
			break;
		case BOTH_A7_KICK_BF:
			kickPush = flrand( 75.0f, 125.0f );
			kickDist += 20.0f;
			if ( elapsedTime < 1500 )
			{//auto-aim!
	//			overridAngles = PM_AdjustAnglesForBFKick( ent, ucmd, fwdAngs, qboolean(elapsedTime<850) )?qtrue:overridAngles;
				//FIXME: if we haven't done the back kick yet and there's no-one there to
				//			kick anymore, go into some anim that returns us to our base stance
			}
			if ( ri->footRBolt != -1 )
			{//actually trace to a bolt
				if ( ( elapsedTime >= 750 && elapsedTime < 850 )
					|| ( elapsedTime >= 1400 && elapsedTime < 1500 ) )
				{//right, though either would do
					doKick = qtrue;
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
					//NOTE: have to fudge this a little because it's not getting enough range with the anim as-is
					VectorMA( kickEnd, 8, kickDir, kickEnd );
				}
			}
			else
			{//guess
				if ( elapsedTime >= 250 && elapsedTime < 350 )
				{//front
					doKick = qtrue;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
				}
				else if ( elapsedTime >= 350 && elapsedTime < 450 )
				{//back
					doKick = qtrue;
					AngleVectors( fwdAngs, kickDir, NULL, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
			}
			break;
		case BOTH_A7_KICK_RL:
			kickPush = flrand( 75.0f, 125.0f );
			kickDist += 10.0f;

			//ok, I'm tracing constantly on these things, they NEVER hit otherwise (in MP at least)

			//FIXME: auto aim at enemies on the side of us?
			//overridAngles = PM_AdjustAnglesForRLKick( ent, ucmd, fwdAngs, qboolean(elapsedTime<850) )?qtrue:overridAngles;
			//if ( elapsedTime >= 250 && elapsedTime < 350 )
			if (level.framenum&1)
			{//right
				doKick = qtrue;
				if ( ri->footRBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footRBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
					//NOTE: have to fudge this a little because it's not getting enough range with the anim as-is
					VectorMA( kickEnd, 8, kickDir, kickEnd );
				}
				else
				{//guess
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
				}
			}
			//else if ( elapsedTime >= 350 && elapsedTime < 450 )
			else
			{//left
				doKick = qtrue;
				if ( ri->footLBolt != -1 )
				{//actually trace to a bolt
					G_GetBoltPosition( ent, ri->footLBolt, kickEnd, 0 );
					VectorSubtract( kickEnd, ent->r.currentOrigin, kickDir );
					kickDir[2] = 0;//ah, flatten it, I guess...
					VectorNormalize( kickDir );
					//NOTE: have to fudge this a little because it's not getting enough range with the anim as-is
					VectorMA( kickEnd, 8, kickDir, kickEnd );
				}
				else
				{//guess
					AngleVectors( fwdAngs, NULL, kickDir, NULL );
					VectorScale( kickDir, -1, kickDir );
				}
			}
			break;
		}
	}

	if ( doKick )
	{
//		G_KickTrace( ent, kickDir, kickDist, kickEnd, kickDamage, kickPush );
		G_KickTrace( ent, kickDir, kickDist, NULL, kickDamage, kickPush );
	}
}

static QINLINE qboolean G_PrettyCloseIGuess(float a, float b, float tolerance)
{
    if ((a-b) < tolerance &&
		(a-b) > -tolerance)
	{
		return qtrue;
	}

	return qfalse;
}

void G_GrabSomeMofos(gentity_t *self)
{
	renderInfo_t *ri = &self->client->renderInfo;
	mdxaBone_t boltMatrix;
	vec3_t flatAng;
	vec3_t pos;
	vec3_t grabMins, grabMaxs;
	trace_t trace;

	if (!self->ghoul2 || ri->handRBolt == -1)
	{ //no good
		return;
	}

    VectorSet(flatAng, 0.0f, self->client->ps.viewangles[1], 0.0f);
	trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->handRBolt, &boltMatrix, flatAng, self->client->ps.origin,
		level.time, NULL, self->modelScale);
	BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, pos);

	VectorSet(grabMins, -4.0f, -4.0f, -4.0f);
	VectorSet(grabMaxs, 4.0f, 4.0f, 4.0f);

	//trace from my origin to my hand, if we hit anyone then get 'em
	trap->Trace( &trace, self->client->ps.origin, grabMins, grabMaxs, pos, self->s.number, MASK_SHOT, 0, G2TRFLAG_DOGHOULTRACE|G2TRFLAG_GETSURFINDEX|G2TRFLAG_THICK|G2TRFLAG_HITCORPSES, g_g2TraceLod.integer );
    
	if (trace.fraction != 1.0f &&
		trace.entityNum < ENTITYNUM_WORLD)
	{
		gentity_t *grabbed = &g_entities[trace.entityNum];

		if (grabbed->inuse && (grabbed->s.eType == ET_PLAYER || grabbed->s.eType == ET_NPC) &&
			grabbed->client && grabbed->health > 0 &&
			G_CanBeEnemy(self, grabbed) &&
			G_PrettyCloseIGuess(grabbed->client->ps.origin[2], self->client->ps.origin[2], 4.0f) &&
			(!BG_InGrappleMove(grabbed->client->ps.torsoAnim) || grabbed->client->ps.torsoAnim == BOTH_KYLE_GRAB) &&
			(!BG_InGrappleMove(grabbed->client->ps.legsAnim) || grabbed->client->ps.legsAnim == BOTH_KYLE_GRAB))
		{ //grabbed an active player/npc
			int tortureAnim = -1;
			int correspondingAnim = -1;

			if (self->client->pers.cmd.forwardmove > 0)
			{ //punch grab
				tortureAnim = BOTH_KYLE_PA_1;
				correspondingAnim = BOTH_PLAYER_PA_1;
			}
			else if (self->client->pers.cmd.forwardmove < 0)
			{ //knee-throw
				tortureAnim = BOTH_KYLE_PA_2;
				correspondingAnim = BOTH_PLAYER_PA_2;
			}

			if (tortureAnim == -1 || correspondingAnim == -1)
			{
				if (self->client->ps.torsoTimer < 300 && !self->client->grappleState)
				{ //you failed to grab anyone, play the "failed to grab" anim
					G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_MISS, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
					if (self->client->ps.torsoAnim == BOTH_KYLE_MISS)
					{ //providing the anim set succeeded..
						self->client->ps.weaponTime = self->client->ps.torsoTimer;
					}
				}
				return;
			}

			self->client->grappleIndex = grabbed->s.number;
			self->client->grappleState = 1;

			grabbed->client->grappleIndex = self->s.number;
			grabbed->client->grappleState = 20;

			//time to crack some heads
			G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, tortureAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			if (self->client->ps.torsoAnim == tortureAnim)
			{ //providing the anim set succeeded..
				self->client->ps.weaponTime = self->client->ps.torsoTimer;
			}

			G_SetAnim(grabbed, &grabbed->client->pers.cmd, SETANIM_BOTH, correspondingAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			if (grabbed->client->ps.torsoAnim == correspondingAnim)
			{ //providing the anim set succeeded..
				if (grabbed->client->ps.weapon == WP_SABER)
				{ //turn it off
					if (!grabbed->client->ps.saberHolstered)
					{
						grabbed->client->ps.saberHolstered = 2;
						if (grabbed->client->saber[0].soundOff)
						{
							G_Sound(grabbed, CHAN_AUTO, grabbed->client->saber[0].soundOff);
						}
						if (grabbed->client->saber[1].soundOff &&
							grabbed->client->saber[1].model[0])
						{
							G_Sound(grabbed, CHAN_AUTO, grabbed->client->saber[1].soundOff);
						}
					}
				}
				if (grabbed->client->ps.torsoTimer < self->client->ps.torsoTimer)
				{ //make sure they stay in the anim at least as long as the grabber
					grabbed->client->ps.torsoTimer = self->client->ps.torsoTimer;
				}
				grabbed->client->ps.weaponTime = grabbed->client->ps.torsoTimer;
			}
		}
	}

	if (self->client->ps.torsoTimer < 300 && !self->client->grappleState)
	{ //you failed to grab anyone, play the "failed to grab" anim
		G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_MISS, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		if (self->client->ps.torsoAnim == BOTH_KYLE_MISS)
		{ //providing the anim set succeeded..
			self->client->ps.weaponTime = self->client->ps.torsoTimer;
		}
	}
}

void JKG_GrappleUpdate( gentity_t *self )
{
	gentity_t *grappler = &g_entities[self->client->grappleIndex];

	if (!grappler->inuse || !grappler->client || grappler->client->grappleIndex != self->s.number ||
		!BG_InGrappleMove(grappler->client->ps.torsoAnim) || !BG_InGrappleMove(grappler->client->ps.legsAnim) ||
		!BG_InGrappleMove(self->client->ps.torsoAnim) || !BG_InGrappleMove(self->client->ps.legsAnim) ||
		!self->client->grappleState || !grappler->client->grappleState ||
		grappler->health < 1 || self->health < 1 ||
		!G_PrettyCloseIGuess(self->client->ps.origin[2], grappler->client->ps.origin[2], 4.0f))
	{
		self->client->grappleState = 0;
		if ((BG_InGrappleMove(self->client->ps.torsoAnim) && self->client->ps.torsoTimer > 100) ||
			(BG_InGrappleMove(self->client->ps.legsAnim) && self->client->ps.legsTimer > 100))
		{ //if they're pretty far from finishing the anim then shove them into another anim
			G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_MISS, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			if (self->client->ps.torsoAnim == BOTH_KYLE_MISS)
			{ //providing the anim set succeeded..
				self->client->ps.weaponTime = self->client->ps.torsoTimer;
			}
		}
	}
	else
	{
		vec3_t grapAng;

		VectorSubtract(grappler->client->ps.origin, self->client->ps.origin, grapAng);

		if (VectorLength(grapAng) > 64.0f)
		{ //too far away, break it off
			if ((BG_InGrappleMove(self->client->ps.torsoAnim) && self->client->ps.torsoTimer > 100) ||
				(BG_InGrappleMove(self->client->ps.legsAnim) && self->client->ps.legsTimer > 100))
			{
				self->client->grappleState = 0;

				G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_MISS, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
				if (self->client->ps.torsoAnim == BOTH_KYLE_MISS)
				{ //providing the anim set succeeded..
					self->client->ps.weaponTime = self->client->ps.torsoTimer;
				}
			}
		}
		else
		{
			vectoangles(grapAng, grapAng);
			SetClientViewAngle(self, grapAng);

			if (self->client->grappleState >= 20)
			{ //grapplee
				//try to position myself at the correct distance from my grappler
				float idealDist;
				vec3_t gFwd, idealSpot;
				trace_t trace;

				if (grappler->client->ps.torsoAnim == BOTH_KYLE_PA_1)
				{ //grab punch
					idealDist = 46.0f;
				}
				else
				{ //knee-throw
					idealDist = 34.0f;
				}

				AngleVectors(grappler->client->ps.viewangles, gFwd, 0, 0);
				VectorMA(grappler->client->ps.origin, idealDist, gFwd, idealSpot);

				trap->Trace(&trace, self->client->ps.origin, self->r.mins, self->r.maxs, idealSpot, self->s.number, self->clipmask, 0, 0, 0);
				if (!trace.startsolid && !trace.allsolid && trace.fraction == 1.0f)
				{ //go there
					G_SetOrigin(self, idealSpot);
					VectorCopy(idealSpot, self->client->ps.origin);
				}
			}
			else if (self->client->grappleState >= 1)
			{ //grappler
				if (grappler->client->ps.weapon == WP_SABER)
				{ //make sure their saber is shut off
					if (!grappler->client->ps.saberHolstered)
					{
						grappler->client->ps.saberHolstered = 2;
						if (grappler->client->saber[0].soundOff)
						{
							G_Sound(grappler, CHAN_AUTO, grappler->client->saber[0].soundOff);
						}
						if (grappler->client->saber[1].soundOff &&
							grappler->client->saber[1].model[0])
						{
							G_Sound(grappler, CHAN_AUTO, grappler->client->saber[1].soundOff);
						}
					}
				}

				//check for smashy events
				if (self->client->ps.torsoAnim == BOTH_KYLE_PA_1)
				{ //grab punch
                    if (self->client->grappleState == 1)
					{ //smack
						if (self->client->ps.torsoTimer < 3400)
						{
							int grapplerAnim = grappler->client->ps.torsoAnim;
							int grapplerTime = grappler->client->ps.torsoTimer;

							G_Damage(grappler, self, self, NULL, self->client->ps.origin, 10, 0, MOD_MELEE);
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );

							//it might try to put them into a pain anim or something, so override it back again
							if (grappler->health > 0)
							{
								grappler->client->ps.torsoAnim = grapplerAnim;
								grappler->client->ps.torsoTimer = grapplerTime;
								grappler->client->ps.legsAnim = grapplerAnim;
								grappler->client->ps.legsTimer = grapplerTime;
								grappler->client->ps.weaponTime = grapplerTime;
							}
							self->client->grappleState++;
						}
					}
					else if (self->client->grappleState == 2)
					{ //smack!
						if (self->client->ps.torsoTimer < 2550)
						{
							int grapplerAnim = grappler->client->ps.torsoAnim;
							int grapplerTime = grappler->client->ps.torsoTimer;

							G_Damage(grappler, self, self, NULL, self->client->ps.origin, 10, 0, MOD_MELEE);
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );

							//it might try to put them into a pain anim or something, so override it back again
							if (grappler->health > 0)
							{
								grappler->client->ps.torsoAnim = grapplerAnim;
								grappler->client->ps.torsoTimer = grapplerTime;
								grappler->client->ps.legsAnim = grapplerAnim;
								grappler->client->ps.legsTimer = grapplerTime;
								grappler->client->ps.weaponTime = grapplerTime;
							}
							self->client->grappleState++;
						}
					}
					else
					{ //SMACK!
						if (self->client->ps.torsoTimer < 1300)
						{
							vec3_t tossDir;

							G_Damage(grappler, self, self, NULL, self->client->ps.origin, 30, 0, MOD_MELEE);
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );

							self->client->grappleState = 0;

							VectorSubtract(grappler->client->ps.origin, self->client->ps.origin, tossDir);
							VectorNormalize(tossDir);
							VectorScale(tossDir, 500.0f, tossDir);
							tossDir[2] = 200.0f;

							VectorAdd(grappler->client->ps.velocity, tossDir, grappler->client->ps.velocity);

							if (grappler->health > 0)
							{ //if still alive knock them down
								grappler->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
								grappler->client->ps.forceHandExtendTime = level.time + 1300;
							}
						}
					}
				}
				else if (self->client->ps.torsoAnim == BOTH_KYLE_PA_2)
				{ //knee throw
                    if (self->client->grappleState == 1)
					{ //knee to the face
						if (self->client->ps.torsoTimer < 3200)
						{
							int grapplerAnim = grappler->client->ps.torsoAnim;
							int grapplerTime = grappler->client->ps.torsoTimer;

							G_Damage(grappler, self, self, NULL, self->client->ps.origin, 20, 0, MOD_MELEE);
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );

							//it might try to put them into a pain anim or something, so override it back again
							if (grappler->health > 0)
							{
								grappler->client->ps.torsoAnim = grapplerAnim;
								grappler->client->ps.torsoTimer = grapplerTime;
								grappler->client->ps.legsAnim = grapplerAnim;
								grappler->client->ps.legsTimer = grapplerTime;
								grappler->client->ps.weaponTime = grapplerTime;
							}
							self->client->grappleState++;
						}
					}
					else if (self->client->grappleState == 2)
					{ //smashed on the ground
						if (self->client->ps.torsoTimer < 2000)
						{
							//G_Damage(grappler, self, self, NULL, self->client->ps.origin, 10, 0, MOD_MELEE);
							//don't do damage on this one, it would look very freaky if they died
							G_EntitySound( grappler, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );
							self->client->grappleState++;
						}
					}
					else
					{ //and another smash
						if (self->client->ps.torsoTimer < 1000)
						{
							G_Damage(grappler, self, self, NULL, self->client->ps.origin, 30, 0, MOD_MELEE);
							//G_Sound( grappler, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/melee/punch%d", Q_irand( 1, 4 ) ) ) );

							//it might try to put them into a pain anim or something, so override it back again
							if (grappler->health > 0)
							{
								grappler->client->ps.torsoTimer = 1000;
								//G_SetAnim(grappler, &grappler->client->pers.cmd, SETANIM_BOTH, BOTH_GETUP3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
								grappler->client->grappleState = 0;
							}
							else
							{ //override death anim
								grappler->client->ps.torsoAnim = BOTH_DEADFLOP1;
								grappler->client->ps.legsAnim = BOTH_DEADFLOP1;
							}

							self->client->grappleState = 0;
						}
					}
				}
				else
				{ //?
				}
			}
		}
	}
}