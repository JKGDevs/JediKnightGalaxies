/*
===========================================================================
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

#include "g_local.h"
#include "bg_local.h"
#include "w_saber.h"
#include "ai_main.h"
#include "ghoul2/G2.h"

#define SABER_BOX_SIZE 16.0f
extern bot_state_t *botstates[MAX_CLIENTS];
extern qboolean InFront( vec3_t spot, vec3_t from, vec3_t fromAngles, float threshHold );
extern void G_TestLine(vec3_t start, vec3_t end, int color, int time);

int saberSpinSound = 0;

//would be cleaner if these were renamed to BG_ and proto'd in a header.
qboolean PM_SaberInTransition( int move );
qboolean PM_SaberInDeflect( int move );
qboolean PM_SaberInBrokenParry( int move );
qboolean PM_SaberInBounce( int move );
qboolean BG_SaberInReturn( int move );
qboolean BG_InKnockDownOnGround( playerState_t *ps );
qboolean BG_StabDownAnim( int anim );
qboolean BG_SabersOff( playerState_t *ps );
qboolean BG_SaberInTransitionAny( int move );
qboolean BG_SaberInAttackPure( int move );
qboolean WP_SaberBladeDoTransitionDamage( saberInfo_t *saber, int bladeNum );

void WP_SaberAddG2Model( gentity_t *saberent, const char *saberModel, qhandle_t saberSkin );
void WP_SaberRemoveG2Model( gentity_t *saberent );

/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///
///																																///
///																																///
///													JEDI KNIGHT GALAXIES														///
///											EXHAUSTIVE LIGHTSABER COMBAT SYSTEM													///
///																																///
///						System designed by Pande, eezstreet. Visuals designed by BlackResuru. (c) 2012 JKG						///
///																																///
///																																///
/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///

// Returns qtrue if we should disarm the attacker, returns qfalse if we should stagger the defender
unsigned char JKG_GetBPNeededForBlock( gentity_t *blocker, int attackerSaberMove, int attackerSaberStyle );
qboolean JKG_ShouldDisarm(gentity_t *defender, gentity_t *attacker)
{
	int BPneeded;
	if(!defender->client)
	{
		return qfalse;
	}
	if(!attacker->client)
	{
		return qfalse;
	}

	if(!(defender->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) ) )
	{
		return qfalse; // No, because the person isn't projectile blocking
	}

	if( defender->client->ps.blockPoints < 40 )
	{
		return qfalse; // Defender has crap BP
	}

	if( attacker->client->ps.forcePower >= 35 )
	{
		return qfalse; // Attacker has too much force power to be a candidate for this
	}

	// Alright, let's establish how much BP is actually drained from this move first.
	BPneeded = JKG_GetBPNeededForBlock(defender, attacker->client->ps.saberMove, attacker->client->ps.fd.saberAnimLevel);

	if( BPneeded > 0 )
	{
		return qfalse; // Not a perfect block, so, no.
	}

	if( (level.time - defender->client->saberProjBlockTime) > 6000 )
	{
		return qfalse; // Didn't block fast enough for it to count, soz.
	}

	// kk, we can disarm the guy :D
	// will add a random factor here in case it is felt that this happens way too often
	return qtrue;
}

float RandFloat(float min, float max) {
	//JAC: Fixed an issue where linux was producing undesired results due to not using RAND_MAX, which is platform-dependant
	int randActual = rand();
	float randMax = 32768.0f;
#ifdef _WIN32
	if ( g_randFix.integer == 2 )
		randActual = (randActual<<16)|randActual;
#elif defined(__GCC__)
	if ( g_randFix.integer == 1 )
		randMax = RAND_MAX;
#endif
	return ((randActual * (max - min)) / randMax) + min;
}

#ifdef DEBUG_SABER_BOX
void	G_DebugBoxLines(vec3_t mins, vec3_t maxs, int duration)
{
	vec3_t start;
	vec3_t end;

	float x = maxs[0] - mins[0];
	float y = maxs[1] - mins[1];

	// top of box
	VectorCopy(maxs, start);
	VectorCopy(maxs, end);
	start[0] -= x;
	G_TestLine(start, end, 0x00000ff, duration);
	end[0] = start[0];
	end[1] -= y;
	G_TestLine(start, end, 0x00000ff, duration);
	start[1] = end[1];
	start[0] += x;
	G_TestLine(start, end, 0x00000ff, duration);
	G_TestLine(start, maxs, 0x00000ff, duration);
	// bottom of box
	VectorCopy(mins, start);
	VectorCopy(mins, end);
	start[0] += x;
	G_TestLine(start, end, 0x00000ff, duration);
	end[0] = start[0];
	end[1] += y;
	G_TestLine(start, end, 0x00000ff, duration);
	start[1] = end[1];
	start[0] -= x;
	G_TestLine(start, end, 0x00000ff, duration);
	G_TestLine(start, mins, 0x00000ff, duration);
}
#endif

//general check for performing certain attacks against others
qboolean G_CanBeEnemy( gentity_t *self, gentity_t *enemy )
{
	if (!self->inuse || !enemy->inuse || !self->client || !enemy->client)
	{
		return qfalse;
	}

	if (self->client->ps.duelInProgress && self->client->ps.duelIndex != enemy->s.number)
	{ //dueling but not with this person
		return qfalse;
	}

	if (enemy->client->ps.duelInProgress && enemy->client->ps.duelIndex != self->s.number)
	{ //other guy dueling but not with me
		return qfalse;
	}

	if (g_gametype.integer < GT_TEAM)
	{ //ok, sure
		return qtrue;
	}

	if (g_friendlyFire.integer)
	{ //if ff on then can inflict damage normally on teammates
		return qtrue;
	}

	if (OnSameTeam(self, enemy))
	{ //ff not on, don't hurt teammates
		return qfalse;
	}

	return qtrue;
}

//This function gets the attack power which is used to decide broken parries,
//knockaways, and numerous other things. It is not directly related to the
//actual amount of damage done, however. -rww
static QINLINE int G_SaberAttackPower(gentity_t *ent, qboolean attacking)
{
	return 0;
}

void WP_DeactivateSaber( gentity_t *self, qboolean clearLength )
{
	if ( !self || !self->client )
	{
		return;
	}
	//keep my saber off!
	if ( !self->client->ps.saberHolstered )
	{
		self->client->ps.saberHolstered = 2;
		//Doens't matter ATM
		if (self->client->saber[0].soundOff)
		{
			G_Sound(self, CHAN_WEAPON, self->client->saber[0].soundOff);
		}

		if (self->client->saber[1].soundOff &&
			self->client->saber[1].model[0])
		{
			G_Sound(self, CHAN_WEAPON, self->client->saber[1].soundOff);
		}

	}
}

void WP_ActivateSaber( gentity_t *self )
{
	if ( !self || !self->client )
	{
		return;
	}

	if (self->NPC &&
		self->client->ps.forceHandExtend == HANDEXTEND_JEDITAUNT &&
		(self->client->ps.forceHandExtendTime - level.time) > 200)
	{ //if we're an NPC and in the middle of a taunt then stop it
		self->client->ps.forceHandExtend = HANDEXTEND_NONE;
		self->client->ps.forceHandExtendTime = 0;
	}
	else if (self->client->ps.fd.forceGripCripple)
	{ //can't activate saber while being gripped
		return;
	}
	//Stoiss add: OJP stance Fix
	//NPC code calls this all the time, only whip out saber if it's not already turned on.
	if ( self->client->ps.saberHolstered == 2)
	//if ( self->client->ps.saberHolstered )
	{
		self->client->ps.saberHolstered = 0;
		if (self->client->saber[0].soundOn)
		{
			G_Sound(self, CHAN_WEAPON, self->client->saber[0].soundOn);
		}

		if (self->client->saber[1].soundOn)
		{
			G_Sound(self, CHAN_WEAPON, self->client->saber[1].soundOn);
		}
	}
}

#define PROPER_THROWN_VALUE 999 //Ah, well.. 

void SaberUpdateSelf(gentity_t *ent)
{
	if (ent->r.ownerNum == ENTITYNUM_NONE)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	if (!g_entities[ent->r.ownerNum].inuse ||
		!g_entities[ent->r.ownerNum].client/* ||
		g_entities[ent->r.ownerNum].client->sess.sessionTeam == TEAM_SPECTATOR*/)
	{
		ent->think = G_FreeEntity;
		ent->nextthink = level.time;
		return;
	}

	if (g_entities[ent->r.ownerNum].client->ps.saberInFlight && g_entities[ent->r.ownerNum].health > 0)
	{ //let The Master take care of us now (we'll get treated like a missile until we return)
		ent->nextthink = level.time;
		ent->genericValue5 = PROPER_THROWN_VALUE;
		return;
	}

	ent->genericValue5 = 0;

	if (g_entities[ent->r.ownerNum].client->ps.weapon != WP_SABER ||
		(g_entities[ent->r.ownerNum].client->ps.pm_flags & PMF_FOLLOW) ||
		//RWW ADDED 7-19-03 BEGIN
		g_entities[ent->r.ownerNum].client->sess.sessionTeam == TEAM_SPECTATOR ||
		g_entities[ent->r.ownerNum].client->tempSpectate >= level.time ||
		//RWW ADDED 7-19-03 END
		g_entities[ent->r.ownerNum].health < 1 ||
		BG_SabersOff( &g_entities[ent->r.ownerNum].client->ps ) ||
		(!g_entities[ent->r.ownerNum].client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] && g_entities[ent->r.ownerNum].s.eType != ET_NPC))
	{ //owner is not using saber, spectating, dead, saber holstered, or has no attack level
		ent->r.contents = 0;
		ent->clipmask = 0;
	}
	else
	{ //Standard contents (saber is active)
#ifdef DEBUG_SABER_BOX
		if (g_saberDebugBox.integer == 1|| g_saberDebugBox.integer == 4)
		{
			vec3_t dbgMins;
			vec3_t dbgMaxs;

			VectorAdd( ent->r.currentOrigin, ent->r.mins, dbgMins );
			VectorAdd( ent->r.currentOrigin, ent->r.maxs, dbgMaxs );

			G_DebugBoxLines(dbgMins, dbgMaxs, (10.0f/(float)sv_fps.integer)*100);
		}
#endif
		if (ent->r.contents != CONTENTS_LIGHTSABER)
		{
			if ((level.time - g_entities[ent->r.ownerNum].client->lastSaberStorageTime) <= 200)
			{ //Only go back to solid once we're sure our owner has updated recently
				ent->r.contents = CONTENTS_LIGHTSABER;
				ent->clipmask = MASK_PLAYERSOLID | CONTENTS_LIGHTSABER;
			}
		}
		else
		{
			ent->r.contents = CONTENTS_LIGHTSABER;
			ent->clipmask = MASK_PLAYERSOLID | CONTENTS_LIGHTSABER;
		}
	}

	trap->LinkEntity((sharedEntity_t *)ent);

	ent->nextthink = level.time;
}

void SaberGotHit( gentity_t *self, gentity_t *other, trace_t *trace )
{
	gentity_t *own = &g_entities[self->r.ownerNum];

	if (!own || !own->client)
	{
		return;
	}

	//Do something here..? Was handling projectiles here, but instead they're now handled in their own functions.
}

qboolean BG_SuperBreakLoseAnim( int anim );

static QINLINE void SetSaberBoxSize(gentity_t *saberent)
{
	gentity_t *owner = NULL;
	vec3_t saberOrg, saberTip;
	int i;
	int j = 0;
	int k = 0;
	qboolean dualSabers = qfalse;
	qboolean alwaysBlock[MAX_SABERS][MAX_BLADES];
	qboolean forceBlock = qfalse;

	assert(saberent && saberent->inuse);

	if (saberent->r.ownerNum < MAX_CLIENTS && saberent->r.ownerNum >= 0)
	{
		owner = &g_entities[saberent->r.ownerNum];
	}
	else if (saberent->r.ownerNum >= 0 && saberent->r.ownerNum < ENTITYNUM_WORLD &&
		g_entities[saberent->r.ownerNum].s.eType == ET_NPC)
	{
		owner = &g_entities[saberent->r.ownerNum];
	}

	if (!owner || !owner->inuse || !owner->client)
	{
		assert(!"Saber with no owner?");
		return;
	}

	if ( owner->client->saber[1].model[0] )
	{
		dualSabers = qtrue;
	}

	if ( PM_SaberInBrokenParry(owner->client->ps.saberMove)
		|| BG_SuperBreakLoseAnim( owner->client->ps.torsoAnim ) )
	{ //let swings go right through when we're in this state
		for ( i = 0; i < MAX_SABERS; i++ )
		{
			if ( i > 0 && !dualSabers )
			{//not using a second saber, set it to not blocking
				for ( j = 0; j < MAX_BLADES; j++ )
				{
					alwaysBlock[i][j] = qfalse;
				}
			}
			else
			{
				if ( (owner->client->saber[i].saberFlags2&SFL2_ALWAYS_BLOCK) )
				{
					for ( j = 0; j < owner->client->saber[i].numBlades; j++ )
					{
						alwaysBlock[i][j] = qtrue;
						forceBlock = qtrue;
					}
				}
				if ( owner->client->saber[i].bladeStyle2Start > 0 )
				{
					for ( j = owner->client->saber[i].bladeStyle2Start; j < owner->client->saber[i].numBlades; j++ )
					{
						if ( (owner->client->saber[i].saberFlags2&SFL2_ALWAYS_BLOCK2) )
						{
							alwaysBlock[i][j] = qtrue;
							forceBlock = qtrue;
						}
						else
						{
							alwaysBlock[i][j] = qfalse;
						}
					}
				}
			}
		}
		if ( !forceBlock )
		{//no sabers/blades to FORCE to be on, so turn off blocking altogether
			VectorSet( saberent->r.mins, 0, 0, 0 );
			VectorSet( saberent->r.maxs, 0, 0, 0 );
#ifndef FINAL_BUILD
			if (g_saberDebugPrint.integer > 1)
			{
				Com_Printf("Client %i in broken parry, saber box 0\n", owner->s.number);
			}
#endif
			return;
		}
	}

	if ((level.time - owner->client->lastSaberStorageTime) > 200 ||
		(level.time - owner->client->saber[j].blade[k].storageTime) > 100)
	{ //it's been too long since we got a reliable point storage, so use the defaults and leave.
		VectorSet( saberent->r.mins, -SABER_BOX_SIZE, -SABER_BOX_SIZE, -SABER_BOX_SIZE );
		VectorSet( saberent->r.maxs, SABER_BOX_SIZE, SABER_BOX_SIZE, SABER_BOX_SIZE );
		return;
	}

	if ( dualSabers
		|| owner->client->saber[0].numBlades > 1 )
	{//dual sabers or multi-blade saber
		if ( owner->client->ps.saberHolstered > 1 )
		{//entirely off
			//no blocking at all
			VectorSet( saberent->r.mins, 0, 0, 0 );
			VectorSet( saberent->r.maxs, 0, 0, 0 );
			return;
		}
	}
	else
	{//single saber
		if ( owner->client->ps.saberHolstered )
		{//off
			//no blocking at all
			VectorSet( saberent->r.mins, 0, 0, 0 );
			VectorSet( saberent->r.maxs, 0, 0, 0 );
			return;
		}
	}
	//Start out at the saber origin, then go through all the blades and push out the extents
	//for each blade, then set the box relative to the origin.
	VectorCopy(saberent->r.currentOrigin, saberent->r.mins);
	VectorCopy(saberent->r.currentOrigin, saberent->r.maxs);

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < MAX_SABERS; j++)
		{
			if (!owner->client->saber[j].model[0])
			{
				break;
			}
			if ( dualSabers
				&& owner->client->ps.saberHolstered == 1 
				&& j == 1 )
			{ //this mother is holstered, get outta here.
				j++;
				continue;
			}
			for (k = 0; k < owner->client->saber[j].numBlades; k++)
			{
				if ( k > 0 )
				{//not the first blade
					if ( !dualSabers )
					{//using a single saber
						if ( owner->client->saber[j].numBlades > 1 )
						{//with multiple blades
							if( owner->client->ps.saberHolstered == 1 )
							{//all blades after the first one are off
								break;
							}
						}
					}
				}
				if ( forceBlock )
				{//only do blocking with blades that are marked to block
					if ( !alwaysBlock[j][k] )
					{//this blade shouldn't be blocking
						continue;
					}
				}
				//VectorMA(owner->client->saber[j].blade[k].muzzlePoint, owner->client->saber[j].blade[k].lengthMax*0.5f, owner->client->saber[j].blade[k].muzzleDir, saberOrg);
				VectorCopy(owner->client->saber[j].blade[k].muzzlePoint, saberOrg);
				VectorMA(owner->client->saber[j].blade[k].muzzlePoint, owner->client->saber[j].blade[k].lengthMax, owner->client->saber[j].blade[k].muzzleDir, saberTip);

				if (saberOrg[i] < saberent->r.mins[i])
				{
					saberent->r.mins[i] = saberOrg[i];
				}
				if (saberTip[i] < saberent->r.mins[i])
				{
					saberent->r.mins[i] = saberTip[i];
				}

				if (saberOrg[i] > saberent->r.maxs[i])
				{
					saberent->r.maxs[i] = saberOrg[i];
				}
				if (saberTip[i] > saberent->r.maxs[i])
				{
					saberent->r.maxs[i] = saberTip[i];
				}

				//G_TestLine(saberOrg, saberTip, 0x0000ff, 50);
			}
		}
	}

	VectorSubtract(saberent->r.mins, saberent->r.currentOrigin, saberent->r.mins);
	VectorSubtract(saberent->r.maxs, saberent->r.currentOrigin, saberent->r.maxs);
}

void WP_SaberInitBladeData( gentity_t *ent )
{
	gentity_t *saberent = NULL;
	gentity_t *checkEnt;
	int i = 0;

	while (i < level.num_entities)
	{ //make sure there are no other saber entities floating around that think they belong to this client.
		checkEnt = &g_entities[i];

		if (checkEnt->inuse && checkEnt->neverFree &&
			checkEnt->r.ownerNum == ent->s.number &&
			checkEnt->classname && checkEnt->classname[0] &&
			!Q_stricmp(checkEnt->classname, "lightsaber"))
		{
			if (saberent)
			{ //already have one
				checkEnt->neverFree = qfalse;
				checkEnt->think = G_FreeEntity;
				checkEnt->nextthink = level.time;
			}
			else
			{ //hmm.. well then, take it as my own.
				//free the bitch but don't issue a kg2 to avoid overflowing clients.
				checkEnt->s.modelGhoul2 = 0;
				G_FreeEntity(checkEnt);

				//now init it manually and reuse this ent slot.
				G_InitGentity(checkEnt);
				saberent = checkEnt;
			}
		}

		i++;
	}

	//We do not want the client to have any real knowledge of the entity whatsoever. It will only
	//ever be used on the server.
	if (!saberent)
	{ //ok, make one then
		saberent = G_Spawn();
	}
	ent->client->ps.saberEntityNum = ent->client->saberStoredIndex = saberent->s.number;
	saberent->classname = "lightsaber";
	
	saberent->neverFree = qtrue; //the saber being removed would be a terrible thing.

	saberent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	saberent->r.ownerNum = ent->s.number;

	saberent->clipmask = MASK_PLAYERSOLID | CONTENTS_LIGHTSABER;
	saberent->r.contents = CONTENTS_LIGHTSABER;

	SetSaberBoxSize(saberent);

	saberent->mass = 10;

	saberent->s.eFlags |= EF_NODRAW;
	saberent->r.svFlags |= SVF_NOCLIENT;

	saberent->s.modelGhoul2 = 1;
	//should we happen to be removed (we belong to an NPC and he is removed) then
	//we want to attempt to remove our g2 instance on the client in case we had one.

	saberent->touch = SaberGotHit;

	saberent->think = SaberUpdateSelf;
	saberent->genericValue5 = 0;
	saberent->nextthink = level.time + 50;

	saberSpinSound = G_SoundIndex("sound/weapons/saber/saberspin.wav");
}

#define LOOK_DEFAULT_SPEED	0.15f
#define LOOK_TALKING_SPEED	0.15f	

static QINLINE qboolean G_CheckLookTarget( gentity_t *ent, vec3_t	lookAngles, float *lookingSpeed )
{
	//FIXME: also clamp the lookAngles based on the clamp + the existing difference between
	//		headAngles and torsoAngles?  But often the tag_torso is straight but the torso itself
	//		is deformed to not face straight... sigh...

	//Now calc head angle to lookTarget, if any
	if ( ent->client->renderInfo.lookTarget >= 0 && ent->client->renderInfo.lookTarget < ENTITYNUM_WORLD )
	{
		vec3_t	lookDir, lookOrg, eyeOrg;
		int i;

		if ( ent->client->renderInfo.lookMode == LM_ENT )
		{
			gentity_t	*lookCent = &g_entities[ent->client->renderInfo.lookTarget];
			if ( lookCent )
			{
				if ( lookCent != ent->enemy )
				{//We turn heads faster than headbob speed, but not as fast as if watching an enemy
					*lookingSpeed = LOOK_DEFAULT_SPEED;
				}

				//FIXME: Ignore small deltas from current angles so we don't bob our head in synch with theirs?

				/*
				if ( ent->client->renderInfo.lookTarget == 0 && !cg.renderingThirdPerson )//!cg_thirdPerson.integer )
				{//Special case- use cg.refdef.vieworg if looking at player and not in third person view
					VectorCopy( cg.refdef.vieworg, lookOrg );
				}
				*/ //No no no!
				if ( lookCent->client )
				{
					VectorCopy( lookCent->client->renderInfo.eyePoint, lookOrg );
				}
				else if ( lookCent->inuse && !VectorCompare( lookCent->r.currentOrigin, vec3_origin ) )
				{
					VectorCopy( lookCent->r.currentOrigin, lookOrg );
				}
				else
				{//at origin of world
					return qfalse;
				}
				//Look in dir of lookTarget
			}
		}
		else if ( ent->client->renderInfo.lookMode == LM_INTEREST && ent->client->renderInfo.lookTarget > -1 && ent->client->renderInfo.lookTarget < MAX_INTEREST_POINTS )
		{
			VectorCopy( level.interestPoints[ent->client->renderInfo.lookTarget].origin, lookOrg );
		}
		else
		{
			return qfalse;
		}

		VectorCopy( ent->client->renderInfo.eyePoint, eyeOrg );

		VectorSubtract( lookOrg, eyeOrg, lookDir );

		vectoangles( lookDir, lookAngles );

		for ( i = 0; i < 3; i++ )
		{
			lookAngles[i] = AngleNormalize180( lookAngles[i] );
			ent->client->renderInfo.eyeAngles[i] = AngleNormalize180( ent->client->renderInfo.eyeAngles[i] );
		}
		AnglesSubtract( lookAngles, ent->client->renderInfo.eyeAngles, lookAngles );
		return qtrue;
	}

	return qfalse;
}

//rww - attempted "port" of the SP version which is completely client-side and
//uses illegal gentity access. I am trying to keep this from being too
//bandwidth-intensive.
//This is primarily droid stuff I guess, I'm going to try to handle all humanoid
//NPC stuff in with the actual player stuff if possible.
void NPC_SetBoneAngles(gentity_t *ent, char *bone, vec3_t angles);
static QINLINE void G_G2NPCAngles(gentity_t *ent, vec3_t legs[3], vec3_t angles)
{
	char *craniumBone = "cranium";
	char *thoracicBone = "thoracic"; //only used by atst so doesn't need a case
	qboolean looking = qfalse;
	vec3_t viewAngles;
	vec3_t lookAngles;

	if ( ent->client )
	{
		if ( (ent->client->NPC_class == CLASS_PROBE ) 
			|| (ent->client->NPC_class == CLASS_R2D2 ) 
			|| (ent->client->NPC_class == CLASS_R5D2) 
			|| (ent->client->NPC_class == CLASS_ATST) )
		{
			vec3_t	trailingLegsAngles;

			VectorCopy( ent->client->ps.viewangles, angles );
			angles[PITCH] = 0;

			VectorCopy( ent->client->ps.viewangles, viewAngles );
	//			viewAngles[YAW] = viewAngles[ROLL] = 0;
			viewAngles[PITCH] *= 0.5;
			VectorCopy( viewAngles, lookAngles );

			lookAngles[1] = 0;

			if ( ent->client->NPC_class == CLASS_ATST )
			{//body pitch
				NPC_SetBoneAngles(ent, thoracicBone, lookAngles);
				//BG_G2SetBoneAngles( cent, ent, ent->thoracicBone, lookAngles, BONE_ANGLES_POSTMULT,POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, cgs.model_draw);
			}

			VectorCopy( viewAngles, lookAngles );

			if ( ent && ent->client && ent->client->NPC_class == CLASS_ATST )
			{
				//CG_ATSTLegsYaw( cent, trailingLegsAngles );
				AnglesToAxis( trailingLegsAngles, legs );
			}
			else
			{
				//FIXME: this needs to properly set the legs.yawing field so we don't erroneously play the turning anim, but we do play it when turning in place
				/*
				if ( angles[YAW] == cent->pe.legs.yawAngle )
				{
					cent->pe.legs.yawing = qfalse;
				}
				else
				{
					cent->pe.legs.yawing = qtrue;
				}

				cent->pe.legs.yawAngle = angles[YAW];
				if ( ent->client )
				{
					ent->client->renderInfo.legsYaw = angles[YAW];
				}
				AnglesToAxis( angles, legs );
				*/
			}

	//			if ( ent && ent->client && ent->client->NPC_class == CLASS_ATST )
	//			{
	//				looking = qfalse;
	//			}
	//			else
			{	//look at lookTarget!
				//FIXME: snaps to side when lets go of lookTarget... ?
				float	lookingSpeed = 0.3f;
				looking = G_CheckLookTarget( ent, lookAngles, &lookingSpeed );
				lookAngles[PITCH] = lookAngles[ROLL] = 0;//droids can't pitch or roll their heads
				if ( looking )
				{//want to keep doing this lerp behavior for a full second after stopped looking (so don't snap)
					ent->client->renderInfo.lookingDebounceTime = level.time + 1000;
				}
			}
			if ( ent->client->renderInfo.lookingDebounceTime > level.time )
			{	//adjust for current body orientation
				vec3_t	oldLookAngles;

				lookAngles[YAW] -= 0;//ent->client->ps.viewangles[YAW];//cent->pe.torso.yawAngle;
				//lookAngles[YAW] -= cent->pe.legs.yawAngle;

				//normalize
				lookAngles[YAW] = AngleNormalize180( lookAngles[YAW] );

				//slowly lerp to this new value
				//Remember last headAngles
				VectorCopy( ent->client->renderInfo.lastHeadAngles, oldLookAngles );
				if( VectorCompare( oldLookAngles, lookAngles ) == qfalse )
				{
					//FIXME: This clamp goes off viewAngles,
					//but really should go off the tag_torso's axis[0] angles, no?
					lookAngles[YAW] = oldLookAngles[YAW]+(lookAngles[YAW]-oldLookAngles[YAW])*0.4f;
				}
				//Remember current lookAngles next time
				VectorCopy( lookAngles, ent->client->renderInfo.lastHeadAngles );
			}
			else
			{//Remember current lookAngles next time
				VectorCopy( lookAngles, ent->client->renderInfo.lastHeadAngles );
			}
			if ( ent->client->NPC_class == CLASS_ATST )
			{
				VectorCopy( ent->client->ps.viewangles, lookAngles );
				lookAngles[0] = lookAngles[2] = 0;
				lookAngles[YAW] -= trailingLegsAngles[YAW];
			}
			else
			{
				lookAngles[PITCH] = lookAngles[ROLL] = 0;
				lookAngles[YAW] -= ent->client->ps.viewangles[YAW];
			}

			NPC_SetBoneAngles(ent, craniumBone, lookAngles);
			//BG_G2SetBoneAngles( cent, ent, ent->craniumBone, lookAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, cgs.model_draw); 
			//return;
		}
		else//if ( (ent->client->NPC_class == CLASS_GONK ) || (ent->client->NPC_class == CLASS_INTERROGATOR) || (ent->client->NPC_class == CLASS_SENTRY) )
		{
		//	VectorCopy( ent->client->ps.viewangles, angles );
		//	AnglesToAxis( angles, legs );
			//return;
		}
	}
}

static QINLINE void G_G2PlayerAngles( gentity_t *ent, vec3_t legs[3], vec3_t legsAngles)
{
	qboolean tPitching = qfalse,
			 tYawing = qfalse,
			 lYawing = qfalse;
	float tYawAngle = ent->client->ps.viewangles[YAW],
		  tPitchAngle = 0,
		  lYawAngle = ent->client->ps.viewangles[YAW];

	int ciLegs = ent->client->ps.legsAnim;
	int ciTorso = ent->client->ps.torsoAnim;

	vec3_t turAngles;
	vec3_t lerpOrg, lerpAng;

	if (ent->s.eType == ET_NPC && ent->client)
	{ //sort of hacky, but it saves a pretty big load off the server
		int i = 0;
		gentity_t *clEnt;

		//If no real clients are in the same PVS then don't do any of this stuff, no one can see him anyway!
		while (i < MAX_CLIENTS)
		{
			clEnt = &g_entities[i];

			if (clEnt && clEnt->inuse && clEnt->client &&
				trap->InPVS(clEnt->client->ps.origin, ent->client->ps.origin))
			{ //this client can see him
				break;
			}

			i++;
		}

		if (i == MAX_CLIENTS)
		{ //no one can see him, just return
			return;
		}
	}

	VectorCopy(ent->client->ps.origin, lerpOrg);
	VectorCopy(ent->client->ps.viewangles, lerpAng);

	if (ent->localAnimIndex < NUM_RESERVED_ANIMSETS)
	{ //don't do these things on non-humanoids
		vec3_t lookAngles;
		entityState_t *emplaced = NULL;

		if (ent->client->ps.hasLookTarget)
		{
			VectorSubtract(g_entities[ent->client->ps.lookTarget].r.currentOrigin, ent->client->ps.origin, lookAngles);
			vectoangles(lookAngles, lookAngles);
			ent->client->lookTime = level.time + 1000;
		}
		else
		{
			VectorCopy(ent->client->ps.origin, lookAngles);
		}
		lookAngles[PITCH] = 0;

		if (ent->client->ps.emplacedIndex)
		{
			emplaced = &g_entities[ent->client->ps.emplacedIndex].s;
		}

		BG_G2PlayerAngles(ent->ghoul2, ent->client->renderInfo.motionBolt, &ent->s, level.time, lerpOrg, lerpAng, legs,
			legsAngles, &tYawing, &tPitching, &lYawing, &tYawAngle, &tPitchAngle, &lYawAngle, FRAMETIME, turAngles,
			ent->modelScale, ciLegs, ciTorso, &ent->client->corrTime, lookAngles, ent->client->lastHeadAngles,
			ent->client->lookTime, emplaced, NULL, ent->client->ps.saberActionFlags);

		if (ent->client->ps.heldByClient && ent->client->ps.heldByClient <= MAX_CLIENTS)
		{ //then put our arm in this client's hand
			//is index+1 because index 0 is valid.
			int heldByIndex = ent->client->ps.heldByClient-1;
			gentity_t *other = &g_entities[heldByIndex];
			int lHandBolt = 0;

			if (other && other->inuse && other->client && other->ghoul2)
			{
				lHandBolt = trap->G2API_AddBolt(other->ghoul2, 0, "*l_hand");
			}
			else
			{ //they left the game, perhaps?
				ent->client->ps.heldByClient = 0;
				return;
			}

			if (lHandBolt)
			{
				mdxaBone_t boltMatrix;
				vec3_t boltOrg;
				vec3_t tAngles;

				VectorCopy(other->client->ps.viewangles, tAngles);
				tAngles[PITCH] = tAngles[ROLL] = 0;

				trap->G2API_GetBoltMatrix(other->ghoul2, 0, lHandBolt, &boltMatrix, tAngles, other->client->ps.origin, level.time, 0, other->modelScale);
				boltOrg[0] = boltMatrix.matrix[0][3];
				boltOrg[1] = boltMatrix.matrix[1][3];
				boltOrg[2] = boltMatrix.matrix[2][3];

				BG_IK_MoveArm(ent->ghoul2, lHandBolt, level.time, &ent->s, ent->client->ps.torsoAnim/*BOTH_DEAD1*/, boltOrg, &ent->client->ikStatus,
					ent->client->ps.origin, ent->client->ps.viewangles, ent->modelScale, 500, qfalse);
			}
		}
		else if (ent->client->ikStatus)
		{ //make sure we aren't IKing if we don't have anyone to hold onto us.
			int lHandBolt = 0;

			if (ent && ent->inuse && ent->client && ent->ghoul2)
			{
				lHandBolt = trap->G2API_AddBolt(ent->ghoul2, 0, "*l_hand");
			}
			else
			{ //This shouldn't happen, but just in case it does, we'll have a failsafe.
				ent->client->ikStatus = qfalse;
			}

			if (lHandBolt)
			{
				BG_IK_MoveArm(ent->ghoul2, lHandBolt, level.time, &ent->s,
					ent->client->ps.torsoAnim/*BOTH_DEAD1*/, vec3_origin, &ent->client->ikStatus, ent->client->ps.origin, ent->client->ps.viewangles, ent->modelScale, 500, qtrue);
			}
		}
	}
	else if (ent->NPC)
	{ //an NPC not using a humanoid skeleton, do special angle stuff.
		G_G2NPCAngles(ent, legs, legsAngles);
	}
}

static QINLINE qboolean SaberAttacking( gentity_t *self )
{
	if ( PM_SaberInParry(self->client->ps.saberMove) )
		return qfalse;
	if ( PM_SaberInBrokenParry(self->client->ps.saberMove) )
		return qfalse;
	if ( PM_SaberInDeflect(self->client->ps.saberMove) )
		return qfalse;
	if ( PM_SaberInBounce(self->client->ps.saberMove) )
		return qfalse;
	if ( PM_SaberInKnockaway(self->client->ps.saberMove) )
		return qfalse;

	//if we're firing and not blocking, then we're attacking.
	if (BG_SaberInAttack(self->client->ps.saberMove))
		if (self->client->ps.weaponstate == WEAPON_FIRING && self->client->ps.saberBlocked == BLOCKED_NONE)
			return qtrue;

	if ( BG_SaberInSpecial( self->client->ps.saberMove ) )
		return qtrue;

	return qfalse;
}

typedef enum
{
	LOCK_FIRST = 0,
	LOCK_TOP = LOCK_FIRST,
	LOCK_DIAG_TR,
	LOCK_DIAG_TL,
	LOCK_DIAG_BR,
	LOCK_DIAG_BL,
	LOCK_R,
	LOCK_L,
	LOCK_RANDOM
} sabersLockMode_t;

#define LOCK_IDEAL_DIST_TOP 32.0f
#define LOCK_IDEAL_DIST_CIRCLE 48.0f

#define SABER_HITDAMAGE 35
qboolean WP_SaberBlockNonRandom( gentity_t *self, gentity_t *other, vec3_t hitloc, qboolean missileBlock );

int G_SaberLockAnim( int attackerSaberStyle, int defenderSaberStyle, int topOrSide, int lockOrBreakOrSuperBreak, int winOrLose )
{
	int baseAnim = -1;
	if ( lockOrBreakOrSuperBreak == SABERLOCK_LOCK )
	{//special case: if we're using the same style and locking
		if ( attackerSaberStyle == defenderSaberStyle 
			|| (!SaberStances[attackerSaberStyle].isDualsOnly&&!SaberStances[attackerSaberStyle].isStaffOnly&&!SaberStances[defenderSaberStyle].isDualsOnly&&!SaberStances[defenderSaberStyle].isStaffOnly) )
		{//using same style
			if ( winOrLose == SABERLOCK_LOSE )
			{//you want the defender's stance...
				if( SaberStances[defenderSaberStyle].isDualsOnly )
				{
					if ( topOrSide == SABERLOCK_TOP )
					{
						baseAnim = BOTH_LK_DL_DL_T_L_2;
					}
					else
					{
						baseAnim = BOTH_LK_DL_DL_S_L_2;
					}
				}
				else if( SaberStances[defenderSaberStyle].isStaffOnly )
				{
					if ( topOrSide == SABERLOCK_TOP )
					{
						baseAnim = BOTH_LK_ST_ST_T_L_2;
					}
					else
					{
						baseAnim = BOTH_LK_ST_ST_S_L_2;
					}
				}
				else
				{
					if ( topOrSide == SABERLOCK_TOP )
					{
						baseAnim = BOTH_LK_S_S_T_L_2;
					}
					else
					{
						baseAnim = BOTH_LK_S_S_S_L_2;
					}
				}
			}
		}
	}
	if ( baseAnim == -1 )
	{
		if( SaberStances[attackerSaberStyle].isDualsOnly )
		{
			if( SaberStances[defenderSaberStyle].isDualsOnly )
			{
				baseAnim = BOTH_LK_DL_DL_S_B_1_L;
			}
			else if( SaberStances[defenderSaberStyle].isStaffOnly )
			{
				baseAnim = BOTH_LK_DL_ST_S_B_1_L;
			}
			else
			{
				baseAnim = BOTH_LK_DL_S_S_B_1_L;
			}
		}
		else if( SaberStances[attackerSaberStyle].isStaffOnly )
		{
			if( SaberStances[defenderSaberStyle].isDualsOnly )
			{
				baseAnim = BOTH_LK_ST_DL_S_B_1_L;
			}
			else if( SaberStances[defenderSaberStyle].isStaffOnly )
			{
				baseAnim = BOTH_LK_ST_ST_S_B_1_L;
			}
			else
			{
				baseAnim = BOTH_LK_ST_S_S_B_1_L;
			}
		}
		else
		{
			if( SaberStances[defenderSaberStyle].isDualsOnly )
			{
				baseAnim = BOTH_LK_S_DL_S_B_1_L;
			}
			else if( SaberStances[defenderSaberStyle].isStaffOnly )
			{
				baseAnim = BOTH_LK_S_ST_S_B_1_L;
			}
			else
			{
				baseAnim = BOTH_LK_S_S_S_B_1_L;
			}
		}
		//side lock or top lock?
		if ( topOrSide == SABERLOCK_TOP )
		{
			baseAnim += 5;
		}
		//lock, break or superbreak?
		if ( lockOrBreakOrSuperBreak == SABERLOCK_LOCK )
		{
			baseAnim += 2;
		}
		else 
		{//a break or superbreak
			if ( lockOrBreakOrSuperBreak == SABERLOCK_SUPERBREAK )
			{
				baseAnim += 3;
			}
			//winner or loser?
			if ( winOrLose == SABERLOCK_WIN )
			{
				baseAnim += 1;
			}
		}
	}
	return baseAnim;
}

extern qboolean BG_CheckIncrementLockAnim( int anim, int winOrLose ); //bg_saber.c
#define LOCK_IDEAL_DIST_JKA 46.0f//all of the new saberlocks are 46.08 from each other because Richard Lico is da MAN

static QINLINE qboolean WP_SabersCheckLock2( gentity_t *attacker, gentity_t *defender, sabersLockMode_t lockMode )
{
	int		attAnim, defAnim = 0;
	float	attStart = 0.5f, defStart = 0.5f;
	float	idealDist = 48.0f;
	vec3_t	attAngles, defAngles, defDir;
	vec3_t	newOrg;
	vec3_t	attDir;
	float	diff = 0;
	trace_t trace;

	//MATCH ANIMS
	if ( lockMode == LOCK_RANDOM )
	{
		lockMode = (sabersLockMode_t)Q_irand( (int)LOCK_FIRST, (int)(LOCK_RANDOM)-1 );
	}
	if ( !SaberStances[attacker->client->ps.fd.saberAnimLevel].isDualsOnly &&
		 !SaberStances[attacker->client->ps.fd.saberAnimLevel].isStaffOnly &&
		 !SaberStances[defender->client->ps.fd.saberAnimLevel].isDualsOnly &&
		 !SaberStances[defender->client->ps.fd.saberAnimLevel].isStaffOnly)
	{//2 single sabers?  Just do it the old way...
		switch ( lockMode )
		{
		case LOCK_TOP:
			attAnim = BOTH_BF2LOCK;
			defAnim = BOTH_BF1LOCK;
			attStart = defStart = 0.5f;
			idealDist = LOCK_IDEAL_DIST_TOP;
			break;
		case LOCK_DIAG_TR:
			attAnim = BOTH_CCWCIRCLELOCK;
			defAnim = BOTH_CWCIRCLELOCK;
			attStart = defStart = 0.5f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		case LOCK_DIAG_TL:
			attAnim = BOTH_CWCIRCLELOCK;
			defAnim = BOTH_CCWCIRCLELOCK;
			attStart = defStart = 0.5f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		case LOCK_DIAG_BR:
			attAnim = BOTH_CWCIRCLELOCK;
			defAnim = BOTH_CCWCIRCLELOCK;
			attStart = defStart = 0.85f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		case LOCK_DIAG_BL:
			attAnim = BOTH_CCWCIRCLELOCK;
			defAnim = BOTH_CWCIRCLELOCK;
			attStart = defStart = 0.85f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		case LOCK_R:
			attAnim = BOTH_CCWCIRCLELOCK;
			defAnim = BOTH_CWCIRCLELOCK;
			attStart = defStart = 0.75f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		case LOCK_L:
			attAnim = BOTH_CWCIRCLELOCK;
			defAnim = BOTH_CCWCIRCLELOCK;
			attStart = defStart = 0.75f;
			idealDist = LOCK_IDEAL_DIST_CIRCLE;
			break;
		default:
			return qfalse;
			break;
		}
	}
	else
	{//use the new system
		idealDist = LOCK_IDEAL_DIST_JKA;//all of the new saberlocks are 46.08 from each other because Richard Lico is da MAN
		if ( lockMode == LOCK_TOP )
		{//top lock
			attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_TOP, SABERLOCK_LOCK, SABERLOCK_WIN );
			defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_TOP, SABERLOCK_LOCK, SABERLOCK_LOSE );
			attStart = defStart = 0.5f;
		}
		else
		{//side lock
			switch ( lockMode )
			{
			case LOCK_DIAG_TR:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				attStart = defStart = 0.5f;
				break;
			case LOCK_DIAG_TL:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				attStart = defStart = 0.5f;
				break;
			case LOCK_DIAG_BR:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				if ( BG_CheckIncrementLockAnim( attAnim, SABERLOCK_WIN ) )
				{
					attStart = 0.85f;//move to end of anim
				}
				else
				{
					attStart = 0.15f;//start at beginning of anim
				}
				if ( BG_CheckIncrementLockAnim( defAnim, SABERLOCK_LOSE ) )
				{
					defStart = 0.85f;//start at end of anim
				}
				else
				{
					defStart = 0.15f;//start at beginning of anim
				}
				break;
			case LOCK_DIAG_BL:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				if ( BG_CheckIncrementLockAnim( attAnim, SABERLOCK_WIN ) )
				{
					attStart = 0.85f;//move to end of anim
				}
				else
				{
					attStart = 0.15f;//start at beginning of anim
				}
				if ( BG_CheckIncrementLockAnim( defAnim, SABERLOCK_LOSE ) )
				{
					defStart = 0.85f;//start at end of anim
				}
				else
				{
					defStart = 0.15f;//start at beginning of anim
				}
				break;
			case LOCK_R:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				if ( BG_CheckIncrementLockAnim( attAnim, SABERLOCK_WIN ) )
				{
					attStart = 0.75f;//move to end of anim
				}
				else
				{
					attStart = 0.25f;//start at beginning of anim
				}
				if ( BG_CheckIncrementLockAnim( defAnim, SABERLOCK_LOSE ) )
				{
					defStart = 0.75f;//start at end of anim
				}
				else
				{
					defStart = 0.25f;//start at beginning of anim
				}
				break;
			case LOCK_L:
				attAnim = G_SaberLockAnim( attacker->client->ps.fd.saberAnimLevel, defender->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_WIN );
				defAnim = G_SaberLockAnim( defender->client->ps.fd.saberAnimLevel, attacker->client->ps.fd.saberAnimLevel, SABERLOCK_SIDE, SABERLOCK_LOCK, SABERLOCK_LOSE );
				//attacker starts with advantage
				if ( BG_CheckIncrementLockAnim( attAnim, SABERLOCK_WIN ) )
				{
					attStart = 0.75f;//move to end of anim
				}
				else
				{
					attStart = 0.25f;//start at beginning of anim
				}
				if ( BG_CheckIncrementLockAnim( defAnim, SABERLOCK_LOSE ) )
				{
					defStart = 0.75f;//start at end of anim
				}
				else
				{
					defStart = 0.25f;//start at beginning of anim
				}
				break;
			default:
				return qfalse;
				break;
			}
		}
	}

	G_SetAnim(attacker, NULL, SETANIM_BOTH, attAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	attacker->client->ps.saberLockFrame = bgAllAnims[attacker->localAnimIndex].anims[attAnim].firstFrame+(bgAllAnims[attacker->localAnimIndex].anims[attAnim].numFrames*attStart);

	G_SetAnim(defender, NULL, SETANIM_BOTH, defAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	defender->client->ps.saberLockFrame = bgAllAnims[defender->localAnimIndex].anims[defAnim].firstFrame+(bgAllAnims[defender->localAnimIndex].anims[defAnim].numFrames*defStart);

	attacker->client->ps.saberLockHits = 0;
	defender->client->ps.saberLockHits = 0;

	attacker->client->ps.saberLockAdvance = qfalse;
	defender->client->ps.saberLockAdvance = qfalse;

	VectorClear( attacker->client->ps.velocity );
	VectorClear( defender->client->ps.velocity );
	attacker->client->ps.saberLockTime = defender->client->ps.saberLockTime = level.time + 10000;
	attacker->client->ps.saberLockEnemy = defender->s.number;
	defender->client->ps.saberLockEnemy = attacker->s.number;
	attacker->client->ps.weaponTime = defender->client->ps.weaponTime = Q_irand( 1000, 3000 );//delay 1 to 3 seconds before pushing

	VectorSubtract( defender->r.currentOrigin, attacker->r.currentOrigin, defDir );
	VectorCopy( attacker->client->ps.viewangles, attAngles );
	attAngles[YAW] = vectoyaw( defDir );
	SetClientViewAngle( attacker, attAngles );
	defAngles[PITCH] = attAngles[PITCH]*-1;
	defAngles[YAW] = AngleNormalize180( attAngles[YAW] + 180);
	defAngles[ROLL] = 0;
	SetClientViewAngle( defender, defAngles );
	
	//MATCH POSITIONS
	diff = VectorNormalize( defDir ) - idealDist;//diff will be the total error in dist
	//try to move attacker half the diff towards the defender
	VectorMA( attacker->r.currentOrigin, diff*0.5f, defDir, newOrg );

	trap->Trace( &trace, attacker->r.currentOrigin, attacker->r.mins, attacker->r.maxs, newOrg, attacker->s.number, attacker->clipmask , 0, 0, 0);
	if ( !trace.startsolid && !trace.allsolid )
	{
		G_SetOrigin( attacker, trace.endpos );
		if (attacker->client)
		{
			VectorCopy(trace.endpos, attacker->client->ps.origin);
		}
		trap->LinkEntity( (sharedEntity_t *)attacker );
	}
	//now get the defender's dist and do it for him too
	VectorSubtract( attacker->r.currentOrigin, defender->r.currentOrigin, attDir );
	diff = VectorNormalize( attDir ) - idealDist;//diff will be the total error in dist
	//try to move defender all of the remaining diff towards the attacker
	VectorMA( defender->r.currentOrigin, diff, attDir, newOrg );
	trap->Trace( &trace, defender->r.currentOrigin, defender->r.mins, defender->r.maxs, newOrg, defender->s.number, defender->clipmask , 0, 0, 0);
	if ( !trace.startsolid && !trace.allsolid )
	{
		if (defender->client)
		{
			VectorCopy(trace.endpos, defender->client->ps.origin);
		}
		G_SetOrigin( defender, trace.endpos );
		trap->LinkEntity( (sharedEntity_t *)defender );
	}

	//DONE!
	return qtrue;
}

qboolean WP_SabersCheckLock( gentity_t *ent1, gentity_t *ent2 )
{
	float dist;
	qboolean	ent1BlockingPlayer = qfalse;
	qboolean	ent2BlockingPlayer = qfalse;

	if ( g_debugSaberLocks.integer )
	{
		WP_SabersCheckLock2( ent1, ent2, LOCK_RANDOM );
		return qtrue;
	}
	//for now.. it's not fair to the lone duelist.
	//we need dual saber lock animations.
	if (level.gametype == GT_POWERDUEL)
	{
		return qfalse;
	}

	if (!g_saberLocking.integer)
	{
		return qfalse;
	}

	if (!ent1->client || !ent2->client)
	{
		return qfalse;
	}

	if (ent1->s.eType == ET_NPC ||
		ent2->s.eType == ET_NPC)
	{ //if either ents is NPC, then never let an NPC lock with someone on the same playerTeam
		if (ent1->client->playerTeam == ent2->client->playerTeam)
		{
			return qfalse;
		}
	}

	if (!ent1->client->ps.saberEntityNum ||
		!ent2->client->ps.saberEntityNum ||
		ent1->client->ps.saberInFlight ||
		ent2->client->ps.saberInFlight)
	{ //can't get in lock if one of them has had the saber knocked out of his hand
		return qfalse;
	}

	if ( fabs( ent1->r.currentOrigin[2]-ent2->r.currentOrigin[2] ) > 16 )
	{
		return qfalse;
	}
	if ( ent1->client->ps.groundEntityNum == ENTITYNUM_NONE ||
		ent2->client->ps.groundEntityNum == ENTITYNUM_NONE )
	{
		return qfalse;
	}
	dist = DistanceSquared(ent1->r.currentOrigin,ent2->r.currentOrigin);
	if ( dist < 64 || dist > 6400 )
	{//between 8 and 80 from each other
		return qfalse;
	}

	if (BG_InSpecialJump(ent1->client->ps.legsAnim))
	{
		return qfalse;
	}
	if (BG_InSpecialJump(ent2->client->ps.legsAnim))
	{
		return qfalse;
	}

	if (BG_InRoll(&ent1->client->ps, ent1->client->ps.legsAnim))
	{
		return qfalse;
	}
	if (BG_InRoll(&ent2->client->ps, ent2->client->ps.legsAnim))
	{
		return qfalse;
	}

	if (ent1->client->ps.forceHandExtend != HANDEXTEND_NONE ||
		ent2->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return qfalse;
	}

	if ((ent1->client->ps.pm_flags & PMF_DUCKED) ||
		(ent2->client->ps.pm_flags & PMF_DUCKED))
	{
		return qfalse;
	}

	if ( (ent1->client->saber[0].saberFlags&SFL_NOT_LOCKABLE)
		|| (ent2->client->saber[0].saberFlags&SFL_NOT_LOCKABLE) )
	{
		return qfalse;
	}
	if ( ent1->client->saber[1].model[0] &&
			!ent1->client->ps.saberHolstered &&
			(ent1->client->saber[1].saberFlags&SFL_NOT_LOCKABLE) )
	{
		return qfalse;
	}
	if ( ent2->client->saber[1].model[0] &&
			!ent2->client->ps.saberHolstered &&
			(ent2->client->saber[1].saberFlags&SFL_NOT_LOCKABLE) )
	{
		return qfalse;
	}

	if (!InFront( ent1->client->ps.origin, ent2->client->ps.origin, ent2->client->ps.viewangles, 0.4f ))
	{
		return qfalse;
	}
	if (!InFront( ent2->client->ps.origin, ent1->client->ps.origin, ent1->client->ps.viewangles, 0.4f ))
	{
		return qfalse;
	}

	//T to B lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A2_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A3_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A4_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A5_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A6_T__B_ ||
		ent1->client->ps.torsoAnim == BOTH_A7_T__B_)
	{//ent1 is attacking top-down
		return WP_SabersCheckLock2( ent1, ent2, LOCK_TOP );
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A2_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A3_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A4_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A5_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A6_T__B_ ||
		ent2->client->ps.torsoAnim == BOTH_A7_T__B_)
	{//ent2 is attacking top-down
		return WP_SabersCheckLock2( ent2, ent1, LOCK_TOP );
	}

	if ( ent1->s.number >= 0 && ent1->s.number < MAX_CLIENTS &&
		ent1->client->ps.saberBlocking == BLK_WIDE && ent1->client->ps.weaponTime <= 0 )
	{
		ent1BlockingPlayer = qtrue;
	}
	if ( ent2->s.number >= 0 && ent2->s.number < MAX_CLIENTS &&
		ent2->client->ps.saberBlocking == BLK_WIDE && ent2->client->ps.weaponTime <= 0 )
	{
		ent2BlockingPlayer = qtrue;
	}

	//TR to BL lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A5_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A6_TR_BL ||
		ent1->client->ps.torsoAnim == BOTH_A7_TR_BL)
	{//ent1 is attacking diagonally
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TR );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A6_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A7_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TL )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TR );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A2_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A3_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A4_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A5_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A6_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_A7_BR_TL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_BL );
		}
		return qfalse;
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A5_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A6_TR_BL ||
		ent2->client->ps.torsoAnim == BOTH_A7_TR_BL)
	{//ent2 is attacking diagonally
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TR );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A6_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A7_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TL )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TR );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A2_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A3_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A4_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A5_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A6_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_A7_BR_TL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_BL );
		}
		return qfalse;
	}

	//TL to BR lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A5_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A6_TL_BR ||
		ent1->client->ps.torsoAnim == BOTH_A7_TL_BR)
	{//ent1 is attacking diagonally
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TL );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A6_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A7_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TR )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_TL );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A2_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A3_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A4_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A5_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A6_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_A7_BL_TR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent2 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent1, ent2, LOCK_DIAG_BR );
		}
		return qfalse;
	}

	if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A5_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A6_TL_BR ||
		ent2->client->ps.torsoAnim == BOTH_A7_TL_BR)
	{//ent2 is attacking diagonally
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TL );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A6_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A7_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TR )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_TL );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A2_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A3_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A4_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A5_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A6_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_A7_BL_TR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent1 is attacking in the opposite diagonal
			return WP_SabersCheckLock2( ent2, ent1, LOCK_DIAG_BR );
		}
		return qfalse;
	}
	//L to R lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A2__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A3__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A4__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A5__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A6__L__R ||
		ent1->client->ps.torsoAnim == BOTH_A7__L__R)
	{//ent1 is attacking l to r
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_L );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A6_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_A7_TL_BR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TR ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent2 is attacking or blocking on the r
			return WP_SabersCheckLock2( ent1, ent2, LOCK_L );
		}
		return qfalse;
	}
	if ( ent2->client->ps.torsoAnim == BOTH_A1__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A2__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A3__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A4__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A5__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A6__L__R ||
		ent2->client->ps.torsoAnim == BOTH_A7__L__R)
	{//ent2 is attacking l to r
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_L );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A2_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A3_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A4_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A5_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A6_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_A7_TL_BR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TR ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BL )
		{//ent1 is attacking or blocking on the r
			return WP_SabersCheckLock2( ent2, ent1, LOCK_L );
		}
		return qfalse;
	}
	//R to L lock
	if ( ent1->client->ps.torsoAnim == BOTH_A1__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A2__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A3__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A4__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A5__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A6__R__L ||
		ent1->client->ps.torsoAnim == BOTH_A7__R__L)
	{//ent1 is attacking r to l
		if ( ent2BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent1, ent2, LOCK_R );
		}
		if ( ent2->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A6_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_A7_TR_BL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_TL ||
			ent2->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent2 is attacking or blocking on the l
			return WP_SabersCheckLock2( ent1, ent2, LOCK_R );
		}
		return qfalse;
	}
	if ( ent2->client->ps.torsoAnim == BOTH_A1__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A2__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A3__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A4__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A5__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A6__R__L ||
		ent2->client->ps.torsoAnim == BOTH_A7__R__L)
	{//ent2 is attacking r to l
		if ( ent1BlockingPlayer )
		{//player will block this anyway
			return WP_SabersCheckLock2( ent2, ent1, LOCK_R );
		}
		if ( ent1->client->ps.torsoAnim == BOTH_A1_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A2_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A3_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A4_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A5_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A6_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_A7_TR_BL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_TL ||
			ent1->client->ps.torsoAnim == BOTH_P1_S1_BR )
		{//ent1 is attacking or blocking on the l
			return WP_SabersCheckLock2( ent2, ent1, LOCK_R );
		}
		return qfalse;
	}
	if ( !Q_irand( 0, 10 ) )
	{
		return WP_SabersCheckLock2( ent1, ent2, LOCK_RANDOM );
	}
	return qfalse;
}

static QINLINE int G_GetParryForBlock(int block)
{
	switch (block)
	{
		case BLOCKED_UPPER_RIGHT:
			return LS_PARRY_UR;
			break;
		case BLOCKED_UPPER_RIGHT_PROJ:
			return LS_REFLECT_UR;
			break;
		case BLOCKED_UPPER_LEFT:
			return LS_PARRY_UL;
			break;
		case BLOCKED_UPPER_LEFT_PROJ:
			return LS_REFLECT_UL;
			break;
		case BLOCKED_LOWER_RIGHT:
			return LS_PARRY_LR;
			break;
		case BLOCKED_LOWER_RIGHT_PROJ:
			return LS_REFLECT_LR;
			break;
		case BLOCKED_LOWER_LEFT:
			return LS_PARRY_LL;
			break;
		case BLOCKED_LOWER_LEFT_PROJ:
			return LS_REFLECT_LL;
			break;
		case BLOCKED_TOP:
			return LS_PARRY_UP;
			break;
		case BLOCKED_TOP_PROJ:
			return LS_REFLECT_UP;
			break;
		default:
			break;
	}

	return LS_NONE;
}

int PM_SaberBounceForAttack( int stance, int move );
int PM_SaberDeflectionForQuad( int quad );

extern stringID_table_t animTable[MAX_ANIMATIONS+1];
static QINLINE qboolean WP_GetSaberDeflectionAngle( gentity_t *attacker, gentity_t *defender, float saberHitFraction )
{
	qboolean animBasedDeflection = qtrue;
	int attSaberLevel, defSaberLevel;

	if ( !attacker || !attacker->client || !attacker->ghoul2 )
	{
		return qfalse;
	}
	if ( !defender || !defender->client || !defender->ghoul2 )
	{
		return qfalse;
	}

	if ((level.time - attacker->client->lastSaberStorageTime) > 500)
	{ //last update was too long ago, something is happening to this client to prevent his saber from updating
		return qfalse;
	}
	if ((level.time - defender->client->lastSaberStorageTime) > 500)
	{ //ditto
		return qfalse;
	}

	attSaberLevel = G_SaberAttackPower(attacker, SaberAttacking(attacker));
	defSaberLevel = G_SaberAttackPower(defender, SaberAttacking(defender));

	if ( animBasedDeflection )
	{
		//Hmm, let's try just basing it off the anim
		int attQuadStart = SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attacker->client->ps.saberMove].startingQuadrant;
		int attQuadEnd = SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attacker->client->ps.saberMove].endQuadrant;
		int defQuad = SaberStances[defender->client->ps.fd.saberAnimLevel].moves[defender->client->ps.saberMove].endQuadrant;
		int quadDiff = fabs((float)(defQuad-attQuadStart));

		if ( defender->client->ps.saberMove == LS_READY )
		{
			//FIXME: we should probably do SOMETHING here...
			//I have this return qfalse here in the hopes that
			//the defender will pick a parry and the attacker
			//will hit the defender's saber again.
			//But maybe this func call should come *after*
			//it's decided whether or not the defender is
			//going to parry.
			return qfalse;
		}

		//reverse the left/right of the defQuad because of the mirrored nature of facing each other in combat
		switch ( defQuad )
		{
		case Q_BR:
			defQuad = Q_BL;
			break;
		case Q_R:
			defQuad = Q_L;
			break;
		case Q_TR:
			defQuad = Q_TL;
			break;
		case Q_TL:
			defQuad = Q_TR;
			break;
		case Q_L:
			defQuad = Q_R;
			break;
		case Q_BL:
			defQuad = Q_BR;
			break;
		}

		if ( quadDiff > 4 )
		{//wrap around so diff is never greater than 180 (4 * 45)
			quadDiff = 4 - (quadDiff - 4);
		}
		//have the quads, find a good anim to use
		if ( (!quadDiff || (quadDiff == 1 && Q_irand(0,1))) //defender pretty much stopped the attack at a 90 degree angle
			&& (defSaberLevel == attSaberLevel || Q_irand( 0, defSaberLevel-attSaberLevel ) >= 0) )//and the defender's style is stronger
		{
			//bounce straight back
#ifndef FINAL_BUILD
			int attMove = attacker->client->ps.saberMove;
#endif
			attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.fd.saberAnimLevel, attacker->client->ps.saberMove );
#ifndef FINAL_BUILD
			if (g_saberDebugPrint.integer)
			{
				Com_Printf( "attack %s vs. parry %s bounced to %s\n", 
					animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attMove].anim].name, 
					animTable[SaberStances[defender->client->ps.fd.saberAnimLevel].moves[defender->client->ps.saberMove].anim].name,
					animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attacker->client->ps.saberMove].anim].name );
			}
#endif
			attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
			return qfalse;
		}
		else
		{//attack hit at an angle, figure out what angle it should bounce off att
			int newQuad;
			quadDiff = defQuad - attQuadEnd;
			//add half the diff of between the defense and attack end to the attack end
			if ( quadDiff > 4 )
			{
				quadDiff = 4 - (quadDiff - 4);
			}
			else if ( quadDiff < -4 )
			{
				quadDiff = -4 + (quadDiff + 4);
			}
			newQuad = attQuadEnd + ceil( ((float)quadDiff)/2.0f );
			if ( newQuad < Q_BR )
			{//less than zero wraps around
				newQuad = Q_B + newQuad;
			}
			if ( newQuad == attQuadStart )
			{//never come off at the same angle that we would have if the attack was not interrupted
				if ( Q_irand(0, 1) )
				{
					newQuad--;
				}
				else
				{
					newQuad++;
				}
				if ( newQuad < Q_BR )
				{
					newQuad = Q_B;
				}
				else if ( newQuad > Q_B )
				{
					newQuad = Q_BR;
				}
			}
			if ( newQuad == defQuad )
			{//bounce straight back
#ifndef FINAL_BUILD
				int attMove = attacker->client->ps.saberMove;
#endif
				attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.fd.saberAnimLevel, attacker->client->ps.saberMove );
#ifndef FINAL_BUILD
				if (g_saberDebugPrint.integer)
				{
					Com_Printf( "attack %s vs. parry %s bounced to %s\n", 
						animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attMove].anim].name, 
						animTable[SaberStances[defender->client->ps.fd.saberAnimLevel].moves[defender->client->ps.saberMove].anim].name,
						animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attacker->client->ps.saberMove].anim].name );
				}
#endif
				attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				return qfalse;
			}
			//else, pick a deflection
			else
			{
#ifndef FINAL_BUILD
				int attMove = attacker->client->ps.saberMove;
#endif
				attacker->client->ps.saberMove = PM_SaberDeflectionForQuad( newQuad );
#ifndef FINAL_BUILD
				if (g_saberDebugPrint.integer)
				{
					Com_Printf( "attack %s vs. parry %s deflected to %s\n", 
						animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attMove].anim].name, 
						animTable[SaberStances[defender->client->ps.fd.saberAnimLevel].moves[defender->client->ps.saberMove].anim].name,
						animTable[SaberStances[attacker->client->ps.fd.saberAnimLevel].moves[attacker->client->ps.saberMove].anim].name );
				}
#endif
				attacker->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
				return qtrue;
			}
		}
	}
	else
	{ //old math-based method (probably broken)
		vec3_t	att_HitDir, def_BladeDir, temp;
		float	hitDot;

		VectorCopy(attacker->client->lastSaberBase_Always, temp);

		AngleVectors(attacker->client->lastSaberDir_Always, att_HitDir, 0, 0);

		AngleVectors(defender->client->lastSaberDir_Always, def_BladeDir, 0, 0);

		//now compare
		hitDot = DotProduct( att_HitDir, def_BladeDir );
		if ( hitDot < 0.25f && hitDot > -0.25f )
		{//hit pretty much perpendicular, pop straight back
			attacker->client->ps.saberMove = PM_SaberBounceForAttack( attacker->client->ps.fd.saberAnimLevel, attacker->client->ps.saberMove );
			attacker->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
			return qfalse;
		}
		else 
		{//a deflection
			vec3_t	att_Right, att_Up, att_DeflectionDir;
			float	swingRDot, swingUDot;

			//get the direction of the deflection
			VectorScale( def_BladeDir, hitDot, att_DeflectionDir );
			//get our bounce straight back direction
			VectorScale( att_HitDir, -1.0f, temp );
			//add the bounce back and deflection
			VectorAdd( att_DeflectionDir, temp, att_DeflectionDir );
			//normalize the result to determine what direction our saber should bounce back toward
			VectorNormalize( att_DeflectionDir );

			//need to know the direction of the deflectoin relative to the attacker's facing
			VectorSet( temp, 0, attacker->client->ps.viewangles[YAW], 0 );//presumes no pitch!
			AngleVectors( temp, NULL, att_Right, att_Up );
			swingRDot = DotProduct( att_Right, att_DeflectionDir );
			swingUDot = DotProduct( att_Up, att_DeflectionDir );

			if ( swingRDot > 0.25f )
			{//deflect to right
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_TR;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_BR;
				}
				else
				{//deflect horizontally
					attacker->client->ps.saberMove = LS_D1__R;
				}
			}
			else if ( swingRDot < -0.25f )
			{//deflect to left
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_TL;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_BL;
				}
				else
				{//deflect horizontally
					attacker->client->ps.saberMove = LS_D1__L;
				}
			}
			else
			{//deflect in middle
				if ( swingUDot > 0.25f )
				{//deflect to top
					attacker->client->ps.saberMove = LS_D1_T_;
				}
				else if ( swingUDot < -0.25f )
				{//deflect to bottom
					attacker->client->ps.saberMove = LS_D1_B_;
				}
				else
				{//deflect horizontally?  Well, no such thing as straight back in my face, so use top
					if ( swingRDot > 0 )
					{
						attacker->client->ps.saberMove = LS_D1_TR;
					}
					else if ( swingRDot < 0 )
					{
						attacker->client->ps.saberMove = LS_D1_TL;
					}
					else
					{
						attacker->client->ps.saberMove = LS_D1_T_;
					}
				}
			}

			attacker->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			return qtrue;
		}
	}
}

int G_KnockawayForParry( int move )
{
	//FIXME: need actual anims for this
	//FIXME: need to know which side of the saber was hit!  For now, we presume the saber gets knocked away from the center
	switch ( move )
	{
	case LS_PARRY_UP:
		return LS_K1_T_;//push up
		break;
	case LS_PARRY_UR:
	default://case LS_READY:
		return LS_K1_TR;//push up, slightly to right
		break;
	case LS_PARRY_UL:
		return LS_K1_TL;//push up and to left
		break;
	case LS_PARRY_LR:
		return LS_K1_BR;//push down and to left
		break;
	case LS_PARRY_LL:
		return LS_K1_BL;//push down and to right
		break;
	}
}

#define SABER_NONATTACK_DAMAGE 1

//For strong attacks, we ramp damage based on the point in the attack animation
static QINLINE int G_GetAttackDamage(gentity_t *self, int minDmg, int maxDmg, float multPoint)
{
	int speedDif = 0;
	int totalDamage = maxDmg;
	float peakPoint = 0;
	float attackAnimLength = bgAllAnims[self->localAnimIndex].anims[self->client->ps.torsoAnim].numFrames * fabs((float)(bgAllAnims[self->localAnimIndex].anims[self->client->ps.torsoAnim].frameLerp));
	float currentPoint = 0;
	float damageFactor = 0;
	float animSpeedFactor = 1.0f;

	//Be sure to scale by the proper anim speed just as if we were going to play the animation
	BG_SaberStartTransAnim(self->s.number, self->client->ps.fd.saberAnimLevel, self->client->ps.weapon, self->client->ps.torsoAnim, &animSpeedFactor,
		self->client->ps.brokenLimbs, SaberStances[self->client->ps.fd.saberAnimLevel].moves[self->client->ps.saberMove].animspeedscale, 
		self->client->ps.saberSwingSpeed, self->client->ps.saberMove);
	speedDif = attackAnimLength - (attackAnimLength * animSpeedFactor);
	attackAnimLength += speedDif;
	peakPoint = attackAnimLength;
	peakPoint -= attackAnimLength*multPoint;

	//we treat torsoTimer as the point in the animation (closer it is to attackAnimLength, closer it is to beginning)
	currentPoint = self->client->ps.torsoTimer;

	damageFactor = (float)((currentPoint/peakPoint));
	if (damageFactor > 1)
	{
		damageFactor = (2.0f - damageFactor);
	}

	totalDamage *= damageFactor;
	if (totalDamage < minDmg)
	{
		totalDamage = minDmg;
	}
	if (totalDamage > maxDmg)
	{
		totalDamage = maxDmg;
	}

	//Com_Printf("%i\n", totalDamage);

	return totalDamage;
}

//Get the point in the animation and return a percentage of the current point in the anim between 0 and the total anim length (0.0f - 1.0f)
static QINLINE float G_GetAnimPoint(gentity_t *self)
{
	int speedDif = 0;
	float attackAnimLength = bgAllAnims[self->localAnimIndex].anims[self->client->ps.torsoAnim].numFrames * fabs((float)(bgAllAnims[self->localAnimIndex].anims[self->client->ps.torsoAnim].frameLerp));
	float currentPoint = 0;
	float animSpeedFactor = 1.0f;
	float animPercentage = 0;

	//Be sure to scale by the proper anim speed just as if we were going to play the animation
	BG_SaberStartTransAnim(self->s.number, self->client->ps.fd.saberAnimLevel, self->client->ps.weapon, self->client->ps.torsoAnim, &animSpeedFactor,
		self->client->ps.brokenLimbs, SaberStances[self->client->ps.fd.saberAnimLevel].moves[self->client->ps.saberMove].animspeedscale, 
		self->client->ps.saberSwingSpeed, self->client->ps.saberMove);
	speedDif = attackAnimLength - (attackAnimLength * animSpeedFactor);
	attackAnimLength += speedDif;

	currentPoint = self->client->ps.torsoTimer;

	animPercentage = currentPoint/attackAnimLength;

	//Com_Printf("%f\n", animPercentage);

	return animPercentage;
}

static QINLINE qboolean G_ClientIdleInWorld(gentity_t *ent)
{
	if (ent->s.eType == ET_NPC)
	{
		return qfalse;
	}

	if (!ent->client->pers.cmd.upmove &&
		!ent->client->pers.cmd.forwardmove &&
		!ent->client->pers.cmd.rightmove &&
		!(ent->client->pers.cmd.buttons & BUTTON_GESTURE) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCEGRIP) &&
		!(ent->client->pers.cmd.buttons & BUTTON_IRONSIGHTS) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCEPOWER) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCE_LIGHTNING) &&
		!(ent->client->pers.cmd.buttons & BUTTON_FORCE_DRAIN) &&
		!(ent->client->pers.cmd.buttons & BUTTON_ATTACK))
	{
		return qtrue;
	}

	return qfalse;
}

static QINLINE qboolean G_G2TraceCollide(trace_t *tr, vec3_t lastValidStart, vec3_t lastValidEnd, vec3_t traceMins, vec3_t traceMaxs)
{ //Hit the ent with the normal trace, try the collision trace.
	G2Trace_t		G2Trace;
	gentity_t		*g2Hit;
	vec3_t			angles;
	int				tN = 0;
	float			fRadius = 0;

	if (!g_entities[tr->entityNum].inuse /*||
		(g_entities[tr->entityNum].s.eFlags & EF_DEAD)*/)
	{ //don't do perpoly on corpses.
		return qfalse;
	}

	if (traceMins[0] ||
		traceMins[1] ||
		traceMins[2] ||
		traceMaxs[0] ||
		traceMaxs[1] ||
		traceMaxs[2])
	{
		fRadius=(traceMaxs[0]-traceMins[0])/2.0f;
	}

	memset (&G2Trace, 0, sizeof(G2Trace));

	while (tN < MAX_G2_COLLISIONS)
	{
		G2Trace[tN].mEntityNum = -1;
		tN++;
	}
	g2Hit = &g_entities[tr->entityNum];

	if (g2Hit && g2Hit->inuse && g2Hit->ghoul2)
	{
		vec3_t g2HitOrigin;

		angles[ROLL] = angles[PITCH] = 0;

		if (g2Hit->client)
		{
			VectorCopy(g2Hit->client->ps.origin, g2HitOrigin);
			angles[YAW] = g2Hit->client->ps.viewangles[YAW];
		}
		else
		{
			VectorCopy(g2Hit->r.currentOrigin, g2HitOrigin);
			angles[YAW] = g2Hit->r.currentAngles[YAW];
		}

		trap->G2API_CollisionDetect ( G2Trace, g2Hit->ghoul2, angles, g2HitOrigin, level.time, g2Hit->s.number, lastValidStart, lastValidEnd, g2Hit->modelScale, 0, g_g2TraceLod.integer, fRadius );

		if (G2Trace[0].mEntityNum != g2Hit->s.number)
		{
			tr->fraction = 1.0f;
			tr->entityNum = ENTITYNUM_NONE;
			tr->startsolid = 0;
			tr->allsolid = 0;
			return qfalse;
		}
		else
		{ //The ghoul2 trace result matches, so copy the collision position into the trace endpos and send it back.
			VectorCopy(G2Trace[0].mCollisionPosition, tr->endpos);
			VectorCopy(G2Trace[0].mCollisionNormal, tr->plane.normal);
			
			if (g2Hit->client)
			{
				g2Hit->client->g2LastSurfaceHit = G2Trace[0].mSurfaceIndex;
				g2Hit->client->g2LastSurfaceTime = level.time;
				//[BugFix12]
				g2Hit->client->g2LastSurfaceModel = G2Trace[0].mModelIndex;
				//[/BugFix12]

			}
			return qtrue;
		}
	}

	return qfalse;
}

static QINLINE qboolean G_SaberInBackAttack(int move)
{
	switch (move)
	{
	case LS_A_BACK:
	case LS_A_BACK_CR:
	case LS_A_BACKSTAB:
		return qtrue;
	}

	return qfalse;
}

qboolean saberCheckKnockdown_Thrown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);
qboolean saberCheckKnockdown_Smashed(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other, int damage);
qboolean saberCheckKnockdown_BrokenParry(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);


typedef struct saberFace_s
{
	vec3_t v1;
	vec3_t v2;
	vec3_t v3;
} saberFace_t;

//build faces around blade for collision checking -rww
static QINLINE void G_BuildSaberFaces(vec3_t base, vec3_t tip, float radius, vec3_t fwd,
										  vec3_t right, int *fNum, saberFace_t **fList)
{
	static saberFace_t faces[12];
	int i = 0;
	float *d1 = NULL, *d2 = NULL;
	vec3_t invFwd;
	vec3_t invRight;

	VectorCopy(fwd, invFwd);
	VectorInverse(invFwd);
	VectorCopy(right, invRight);
	VectorInverse(invRight);

	while (i < 8)
	{
		//yeah, this part is kind of a hack, but eh
		if (i < 2)
		{ //"left" surface
			d1 = &fwd[0];
			d2 = &invRight[0];
		}
		else if (i < 4)
		{ //"right" surface
			d1 = &fwd[0];
			d2 = &right[0];
		}
		else if (i < 6)
		{ //"front" surface
			d1 = &right[0];
			d2 = &fwd[0];
		}
		else if (i < 8)
		{ //"back" surface
			d1 = &right[0];
			d2 = &invFwd[0];
		}

		//first triangle for this surface
		VectorMA(base, radius/2.0f, d1, faces[i].v1);
		VectorMA(faces[i].v1, radius/2.0f, d2, faces[i].v1);

		VectorMA(tip, radius/2.0f, d1, faces[i].v2);
		VectorMA(faces[i].v2, radius/2.0f, d2, faces[i].v2);

		VectorMA(tip, -radius/2.0f, d1, faces[i].v3);
		VectorMA(faces[i].v3, radius/2.0f, d2, faces[i].v3);

		i++;

		//second triangle for this surface
		VectorMA(tip, -radius/2.0f, d1, faces[i].v1);
		VectorMA(faces[i].v1, radius/2.0f, d2, faces[i].v1);

		VectorMA(base, radius/2.0f, d1, faces[i].v2);
		VectorMA(faces[i].v2, radius/2.0f, d2, faces[i].v2);

		VectorMA(base, -radius/2.0f, d1, faces[i].v3);
		VectorMA(faces[i].v3, radius/2.0f, d2, faces[i].v3);

		i++;
	}

	//top surface
	//face 1
	VectorMA(tip, radius/2.0f, fwd, faces[i].v1);
	VectorMA(faces[i].v1, -radius/2.0f, right, faces[i].v1);

	VectorMA(tip, radius/2.0f, fwd, faces[i].v2);
	VectorMA(faces[i].v2, radius/2.0f, right, faces[i].v2);

	VectorMA(tip, -radius/2.0f, fwd, faces[i].v3);
	VectorMA(faces[i].v3, -radius/2.0f, right, faces[i].v3);

	i++;

	//face 2
	VectorMA(tip, radius/2.0f, fwd, faces[i].v1);
	VectorMA(faces[i].v1, radius/2.0f, right, faces[i].v1);

	VectorMA(tip, -radius/2.0f, fwd, faces[i].v2);
	VectorMA(faces[i].v2, -radius/2.0f, right, faces[i].v2);

	VectorMA(tip, -radius/2.0f, fwd, faces[i].v3);
	VectorMA(faces[i].v3, radius/2.0f, right, faces[i].v3);

	i++;

	//bottom surface
	//face 1
	VectorMA(base, radius/2.0f, fwd, faces[i].v1);
	VectorMA(faces[i].v1, -radius/2.0f, right, faces[i].v1);

	VectorMA(base, radius/2.0f, fwd, faces[i].v2);
	VectorMA(faces[i].v2, radius/2.0f, right, faces[i].v2);

	VectorMA(base, -radius/2.0f, fwd, faces[i].v3);
	VectorMA(faces[i].v3, -radius/2.0f, right, faces[i].v3);

	i++;

	//face 2
	VectorMA(base, radius/2.0f, fwd, faces[i].v1);
	VectorMA(faces[i].v1, radius/2.0f, right, faces[i].v1);

	VectorMA(base, -radius/2.0f, fwd, faces[i].v2);
	VectorMA(faces[i].v2, -radius/2.0f, right, faces[i].v2);

	VectorMA(base, -radius/2.0f, fwd, faces[i].v3);
	VectorMA(faces[i].v3, radius/2.0f, right, faces[i].v3);

	i++;

	//yeah.. always going to be 12 I suppose.
	*fNum = i;
	*fList = &faces[0];
}

//collision utility function -rww
static QINLINE void G_SabCol_CalcPlaneEq(vec3_t x, vec3_t y, vec3_t z, float *planeEq)
{
	planeEq[0] = x[1]*(y[2]-z[2]) + y[1]*(z[2]-x[2]) + z[1]*(x[2]-y[2]);
	planeEq[1] = x[2]*(y[0]-z[0]) + y[2]*(z[0]-x[0]) + z[2]*(x[0]-y[0]);
	planeEq[2] = x[0]*(y[1]-z[1]) + y[0]*(z[1]-x[1]) + z[0]*(x[1]-y[1]);
	planeEq[3] = -(x[0]*(y[1]*z[2] - z[1]*y[2]) + y[0]*(z[1]*x[2] - x[1]*z[2]) + z[0]*(x[1]*y[2] - y[1]*x[2]) );
}

//collision utility function -rww
static QINLINE int G_SabCol_PointRelativeToPlane(vec3_t pos, float *side, float *planeEq)
{
	*side = planeEq[0]*pos[0] + planeEq[1]*pos[1] + planeEq[2]*pos[2] + planeEq[3];

	if (*side > 0.0f)
	{
		return 1;
	}
	else if (*side < 0.0f)
	{
		return -1;
	}

	return 0;
}

//do actual collision check using generated saber "faces"
static QINLINE qboolean G_SaberFaceCollisionCheck(int fNum, saberFace_t *fList, vec3_t atkStart,
											 vec3_t atkEnd, vec3_t atkMins, vec3_t atkMaxs, vec3_t impactPoint)
{
	static float planeEq[4];
	static float side, side2, dist;
	static vec3_t dir;
	static vec3_t point;
	int i = 0;

	if (VectorCompare(atkMins, vec3_origin) && VectorCompare(atkMaxs, vec3_origin))
	{
		VectorSet(atkMins, -1.0f, -1.0f, -1.0f);
		VectorSet(atkMaxs, 1.0f, 1.0f, 1.0f);
	}

	VectorSubtract(atkEnd, atkStart, dir);

	while (i < fNum)
	{
		G_SabCol_CalcPlaneEq(fList->v1, fList->v2, fList->v3, planeEq);

		if (G_SabCol_PointRelativeToPlane(atkStart, &side, planeEq) !=
			G_SabCol_PointRelativeToPlane(atkEnd, &side2, planeEq))
		{ //start/end points intersect with the plane
			static vec3_t extruded;
			static vec3_t minPoint, maxPoint;
			static vec3_t planeNormal;
			static int facing;

			VectorCopy(&planeEq[0], planeNormal);
			side2 = planeNormal[0]*dir[0] + planeNormal[1]*dir[1] + planeNormal[2]*dir[2];

			dist = side/side2;
			VectorMA(atkStart, -dist, dir, point);

			VectorAdd(point, atkMins, minPoint);
			VectorAdd(point, atkMaxs, maxPoint);

			//point is now the point at which we intersect on the plane.
			//see if that point is within the edges of the face.
            VectorMA(fList->v1, -2.0f, planeNormal, extruded);
			G_SabCol_CalcPlaneEq(fList->v1, fList->v2, extruded, planeEq);
			facing = G_SabCol_PointRelativeToPlane(point, &side, planeEq);

			if (facing < 0)
			{ //not intersecting.. let's try with the mins/maxs and see if they interesect on the edge plane
				facing = G_SabCol_PointRelativeToPlane(minPoint, &side, planeEq);
				if (facing < 0)
				{
					facing = G_SabCol_PointRelativeToPlane(maxPoint, &side, planeEq);
				}
			}

			if (facing >= 0)
			{ //first edge is facing...
				VectorMA(fList->v2, -2.0f, planeNormal, extruded);
				G_SabCol_CalcPlaneEq(fList->v2, fList->v3, extruded, planeEq);
				facing = G_SabCol_PointRelativeToPlane(point, &side, planeEq);

				if (facing < 0)
				{ //not intersecting.. let's try with the mins/maxs and see if they interesect on the edge plane
					facing = G_SabCol_PointRelativeToPlane(minPoint, &side, planeEq);
					if (facing < 0)
					{
						facing = G_SabCol_PointRelativeToPlane(maxPoint, &side, planeEq);
					}
				}

				if (facing >= 0)
				{ //second edge is facing...
					VectorMA(fList->v3, -2.0f, planeNormal, extruded);
					G_SabCol_CalcPlaneEq(fList->v3, fList->v1, extruded, planeEq);
					facing = G_SabCol_PointRelativeToPlane(point, &side, planeEq);

					if (facing < 0)
					{ //not intersecting.. let's try with the mins/maxs and see if they interesect on the edge plane
						facing = G_SabCol_PointRelativeToPlane(minPoint, &side, planeEq);
						if (facing < 0)
						{
							facing = G_SabCol_PointRelativeToPlane(maxPoint, &side, planeEq);
						}
					}

					if (facing >= 0)
					{ //third edge is facing.. success
						VectorCopy(point, impactPoint);
						return qtrue;
					}
				}
			}
		}

		i++;
		fList++;
	}
	
	//did not hit anything
	return qfalse;
}

float WP_SaberBladeLength( saberInfo_t *saber )
{//return largest length
	int	i;
	float len = 0.0f;
	for ( i = 0; i < saber->numBlades; i++ )
	{
		if ( saber->blade[i].lengthMax > len )
		{
			len = saber->blade[i].lengthMax; 
		}
	}
	return len;
}

float WP_SaberLength( gentity_t *ent )
{//return largest length
	if ( !ent || !ent->client )
	{
		return 0.0f;
	}
	else
	{
		int i;
		float len, bestLen = 0.0f;
		for ( i = 0; i < MAX_SABERS; i++ )
		{
			len = WP_SaberBladeLength( &ent->client->saber[i] );
			if ( len > bestLen )
			{
				bestLen = len;
			}
		}
		return bestLen;
	}
}
int WPDEBUG_SaberColor( saber_colors_t saberColor )
{
	switch( (int)(saberColor) )
	{
		case SABER_RED:
			return 0x000000ff;
			break;
		case SABER_ORANGE:
			return 0x000088ff;
			break;
		case SABER_YELLOW:
			return 0x0000ffff;
			break;
		case SABER_GREEN:
			return 0x0000ff00;
			break;
		case SABER_BLUE:
			return 0x00ff0000;
			break;
		case SABER_PURPLE:
			return 0x00ff00ff;
			break;
		default:
			return 0x00ffffff;//white
			break;
	}
}
/*
WP_SabersIntersect

Breaks the two saber paths into 2 tris each and tests each tri for the first saber path against each of the other saber path's tris

FIXME: subdivide the arc into a consistant increment
FIXME: test the intersection to see if the sabers really did intersect (weren't going in the same direction and/or passed through same point at different times)?
*/
extern qboolean tri_tri_intersect(vec3_t V0,vec3_t V1,vec3_t V2,vec3_t U0,vec3_t U1,vec3_t U2);
#define SABER_EXTRAPOLATE_DIST 16.0f
qboolean WP_SabersIntersect( gentity_t *ent1, int ent1SaberNum, int ent1BladeNum, gentity_t *ent2, qboolean checkDir )
{
	vec3_t	saberBase1, saberTip1, saberBaseNext1, saberTipNext1;
	vec3_t	saberBase2, saberTip2, saberBaseNext2, saberTipNext2;
	int		ent2SaberNum = 0, ent2BladeNum = 0;
	vec3_t	dir;

	if ( !ent1 || !ent2 )
	{
		return qfalse;
	}
	if ( !ent1->client || !ent2->client )
	{
		return qfalse;
	}
	if ( BG_SabersOff( &ent1->client->ps )
		|| BG_SabersOff( &ent2->client->ps ) )
	{
		return qfalse;
	}

	for ( ent2SaberNum = 0; ent2SaberNum < MAX_SABERS; ent2SaberNum++ )
	{
		if ( ent2->client->saber[ent2SaberNum].type != SABER_NONE )
		{
			for ( ent2BladeNum = 0; ent2BladeNum < ent2->client->saber[ent2SaberNum].numBlades; ent2BladeNum++ )
			{
				if ( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].lengthMax > 0 )
				{//valid saber and this blade is on
					//if ( ent1->client->saberInFlight )
					{
						VectorCopy( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePointOld, saberBase1 );
						VectorCopy( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePoint, saberBaseNext1 );

						VectorSubtract( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePoint, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePointOld, dir );
						VectorNormalize( dir );
						VectorMA( saberBaseNext1, SABER_EXTRAPOLATE_DIST, dir, saberBaseNext1 );

						VectorMA( saberBase1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].lengthMax+SABER_EXTRAPOLATE_DIST, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzleDirOld, saberTip1 );
						VectorMA( saberBaseNext1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].lengthMax+SABER_EXTRAPOLATE_DIST, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzleDir, saberTipNext1 );

						VectorSubtract( saberTipNext1, saberTip1, dir );
						VectorNormalize( dir );
						VectorMA( saberTipNext1, SABER_EXTRAPOLATE_DIST, dir, saberTipNext1 );
					}
					/*
					else
					{
						VectorCopy( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePoint, saberBase1 );
						VectorCopy( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzlePointNext, saberBaseNext1 );
						VectorMA( saberBase1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].lengthMax, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzleDir, saberTip1 );
						VectorMA( saberBaseNext1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].lengthMax, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzleDirNext, saberTipNext1 );
					}
					*/

					//if ( ent2->client->saberInFlight )
					{
						VectorCopy( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePointOld, saberBase2 );
						VectorCopy( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePoint, saberBaseNext2 );

						VectorSubtract( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePoint, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePointOld, dir );
						VectorNormalize( dir );
						VectorMA( saberBaseNext2, SABER_EXTRAPOLATE_DIST, dir, saberBaseNext2 );

						VectorMA( saberBase2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].lengthMax+SABER_EXTRAPOLATE_DIST, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzleDirOld, saberTip2 );
						VectorMA( saberBaseNext2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].lengthMax+SABER_EXTRAPOLATE_DIST, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzleDir, saberTipNext2 );

						VectorSubtract( saberTipNext2, saberTip2, dir );
						VectorNormalize( dir );
						VectorMA( saberTipNext2, SABER_EXTRAPOLATE_DIST, dir, saberTipNext2 );
					}
					/*
					else
					{
						VectorCopy( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePoint, saberBase2 );
						VectorCopy( ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzlePointNext, saberBaseNext2 );
						VectorMA( saberBase2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].lengthMax, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzleDir, saberTip2 );
						VectorMA( saberBaseNext2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].lengthMax, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzleDirNext, saberTipNext2 );
					}
					*/
					if ( checkDir )
					{//check the direction of the two swings to make sure the sabers are swinging towards each other
						vec3_t saberDir1, saberDir2;
						float dot = 0.0f;

						VectorSubtract( saberTipNext1, saberTip1, saberDir1 );
						VectorSubtract( saberTipNext2, saberTip2, saberDir2 );
						VectorNormalize( saberDir1 );
						VectorNormalize( saberDir2 );
						if ( DotProduct( saberDir1, saberDir2 ) > 0.6f )
						{//sabers moving in same dir, probably didn't actually hit
							continue;
						}
						//now check orientation of sabers, make sure they're not parallel or close to it
						dot = DotProduct( ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].muzzleDir, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].muzzleDir );
						if ( dot > 0.9f || dot < -0.9f )
						{//too parallel to really block effectively?
							continue;
						}
					}

#ifdef DEBUG_SABER_BOX
					if ( g_saberDebugBox.integer == 2 || g_saberDebugBox.integer == 4 )
					{
						G_TestLine(saberBase1, saberTip1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].color, 500);
						G_TestLine(saberTip1, saberTipNext1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].color, 500);
						G_TestLine(saberTipNext1, saberBase1, ent1->client->saber[ent1SaberNum].blade[ent1BladeNum].color, 500);

						G_TestLine(saberBase2, saberTip2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].color, 500);
						G_TestLine(saberTip2, saberTipNext2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].color, 500);
						G_TestLine(saberTipNext2, saberBase2, ent2->client->saber[ent2SaberNum].blade[ent2BladeNum].color, 500);
					}
#endif
					if ( tri_tri_intersect( saberBase1, saberTip1, saberBaseNext1, saberBase2, saberTip2, saberBaseNext2 ) )
					{
						return qtrue;
					}
					if ( tri_tri_intersect( saberBase1, saberTip1, saberBaseNext1, saberBase2, saberTip2, saberTipNext2 ) )
					{
						return qtrue;
					}
					if ( tri_tri_intersect( saberBase1, saberTip1, saberTipNext1, saberBase2, saberTip2, saberBaseNext2 ) )
					{
						return qtrue;
					}
					if ( tri_tri_intersect( saberBase1, saberTip1, saberTipNext1, saberBase2, saberTip2, saberTipNext2 ) )
					{
						return qtrue;
					}
				}
			}
		}
	}
	return qfalse;
}

static QINLINE int G_PowerLevelForSaberAnim( gentity_t *ent, int saberNum, qboolean mySaberHit )
{
	if ( !ent || !ent->client || saberNum >= MAX_SABERS )
	{
		return FORCE_LEVEL_0;
	}
	else
	{
		int anim = ent->client->ps.torsoAnim;
		int	animTimer = ent->client->ps.torsoTimer;
		int	animTimeElapsed = BG_AnimLength( ent->localAnimIndex, (animNumber_t)anim ) - animTimer;
		saberInfo_t *saber = &ent->client->saber[saberNum];
		if ( anim >= BOTH_A1_T__B_ && anim <= BOTH_D1_B____ )
		{
			//FIXME: these two need their own style
			if ( saber->type == SABER_LANCE )
			{
				return FORCE_LEVEL_4;
			}
			else if ( saber->type == SABER_TRIDENT )
			{
				return FORCE_LEVEL_3;
			}
			return FORCE_LEVEL_1;
		}
		if ( anim >= BOTH_A2_T__B_ && anim <= BOTH_D2_B____ )
		{
			return FORCE_LEVEL_2;
		}
		if ( anim >= BOTH_A3_T__B_ && anim <= BOTH_D3_B____ )
		{
			return FORCE_LEVEL_3;
		}
		if ( anim >= BOTH_A4_T__B_ && anim <= BOTH_D4_B____ )
		{//desann
			return FORCE_LEVEL_4;
		}
		if ( anim >= BOTH_A5_T__B_ && anim <= BOTH_D5_B____ )
		{//tavion
			return FORCE_LEVEL_2;
		}
		if ( anim >= BOTH_A6_T__B_ && anim <= BOTH_D6_B____ )
		{//dual
			return FORCE_LEVEL_2;
		}
		if ( anim >= BOTH_A7_T__B_ && anim <= BOTH_D7_B____ )
		{//staff
			return FORCE_LEVEL_2;
		}
		if ( anim >= BOTH_P1_S1_T_ && anim <= BOTH_H1_S1_BR )
		{//parries, knockaways and broken parries
			return FORCE_LEVEL_1;//FIXME: saberAnimLevel?
		}
		switch ( anim )
		{
		case BOTH_A2_STABBACK1:
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimer < 450 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 400 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_ATTACK_BACK:
			if ( animTimer < 500 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_CROUCHATTACKBACK1:
			if ( animTimer < 800 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_BUTTERFLY_LEFT:
		case BOTH_BUTTERFLY_RIGHT:
		case BOTH_BUTTERFLY_FL1:
		case BOTH_BUTTERFLY_FR1:
			//FIXME: break up?
			return FORCE_LEVEL_3;
			break;
		case BOTH_FJSS_TR_BL:
		case BOTH_FJSS_TL_BR:
			//FIXME: break up?
			return FORCE_LEVEL_3;
			break;
		case BOTH_K1_S1_T_:	//# knockaway saber top
		case BOTH_K1_S1_TR:	//# knockaway saber top right
		case BOTH_K1_S1_TL:	//# knockaway saber top left
		case BOTH_K1_S1_BL:	//# knockaway saber bottom left
		case BOTH_K1_S1_B_:	//# knockaway saber bottom
		case BOTH_K1_S1_BR:	//# knockaway saber bottom right
			//FIXME: break up?
			return FORCE_LEVEL_3;
			break;
		case BOTH_LUNGE2_B__T_:
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimer < 400 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 150 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_FORCELEAP2_T__B_:
			if ( animTimer < 400 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 550 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_VS_ATR_S:
		case BOTH_VS_ATL_S:
		case BOTH_VT_ATR_S:
		case BOTH_VT_ATL_S:
			return FORCE_LEVEL_3;//???
			break;
		case BOTH_JUMPFLIPSLASHDOWN1:
			if ( animTimer <= 1000 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 600 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_JUMPFLIPSTABDOWN:
			if ( animTimer <= 1300 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed <= 300 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_JUMPATTACK6:
			/*
			if (pm->ps)
			{
				if ( ( pm->ps->legsAnimTimer >= 1450
						&& BG_AnimLength( g_entities[ps->clientNum].client->clientInfo.animFileIndex, BOTH_JUMPATTACK6 ) - pm->ps->legsAnimTimer >= 400 ) 
					||(pm->ps->legsAnimTimer >= 400
						&& BG_AnimLength( g_entities[ps->clientNum].client->clientInfo.animFileIndex, BOTH_JUMPATTACK6 ) - pm->ps->legsAnimTimer >= 1100 ) )
				{//pretty much sideways
					return FORCE_LEVEL_3;
				}
			}
			*/
			if ( ( animTimer >= 1450
					&& animTimeElapsed >= 400 )
				||(animTimer >= 400
					&& animTimeElapsed >= 1100 ) )
			{//pretty much sideways
				return FORCE_LEVEL_3;
			}
			return FORCE_LEVEL_0;
			break;
		case BOTH_JUMPATTACK7:
			if ( animTimer <= 1200 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 200 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_SPINATTACK6:
			if ( animTimeElapsed <= 200 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_2;//FORCE_LEVEL_3;
			break;
		case BOTH_SPINATTACK7:
			if ( animTimer <= 500 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 500 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_2;//FORCE_LEVEL_3;
			break;
		case BOTH_FORCELONGLEAP_ATTACK:
			if ( animTimeElapsed <= 200 )
			{//1st four frames of anim
				return FORCE_LEVEL_3;
			}
			break;
		/*
		case BOTH_A7_KICK_F://these kicks attack, too
		case BOTH_A7_KICK_B:
		case BOTH_A7_KICK_R:
		case BOTH_A7_KICK_L:
			//FIXME: break up
			return FORCE_LEVEL_3;
			break;
		*/
		case BOTH_STABDOWN:
			if ( animTimer <= 900 )
			{//end of anim
				return FORCE_LEVEL_3;
			}
			break;
		case BOTH_STABDOWN_STAFF:
			if ( animTimer <= 850 )
			{//end of anim
				return FORCE_LEVEL_3;
			}
			break;
		case BOTH_STABDOWN_DUAL:
			if ( animTimer <= 900 )
			{//end of anim
				return FORCE_LEVEL_3;
			}
			break;
		case BOTH_A6_SABERPROTECT:
			if ( animTimer < 650 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 250 )
			{//start of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A7_SOULCAL:
			if ( animTimer < 650 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 600 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A1_SPECIAL:
			if ( animTimer < 600 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 200 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A2_SPECIAL:
			if ( animTimer < 300 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 200 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A3_SPECIAL:
			if ( animTimer < 700 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 200 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_FLIP_ATTACK7:
			return FORCE_LEVEL_3;
			break;
		case BOTH_PULL_IMPALE_STAB:
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimer < 1000 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_PULL_IMPALE_SWING:
			if ( animTimer < 500 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 650 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_ALORA_SPIN_SLASH:
			if ( animTimer < 900 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 250 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A6_FB:
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimer < 250 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 250 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A6_LR:	
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimer < 250 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 250 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_3;
			break;
		case BOTH_A7_HILT:
			return FORCE_LEVEL_0;
			break;
	//===SABERLOCK SUPERBREAKS START===========================================================================
		case BOTH_LK_S_DL_T_SB_1_W:
			if ( animTimer < 700 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_S_ST_S_SB_1_W:
			if ( animTimer < 300 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_S_DL_S_SB_1_W:
		case BOTH_LK_S_S_S_SB_1_W:
			if ( animTimer < 700 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 400 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_S_ST_T_SB_1_W:
		case BOTH_LK_S_S_T_SB_1_W:
			if ( animTimer < 150 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 400 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_DL_DL_T_SB_1_W:
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_DL_DL_S_SB_1_W:
		case BOTH_LK_DL_ST_S_SB_1_W:
			if ( animTimeElapsed < 1000 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_DL_ST_T_SB_1_W:
			if ( animTimer < 950 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 650 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_DL_S_S_SB_1_W:
			if ( saberNum != 0 )
			{//only right hand saber does damage in this suberbreak
				return FORCE_LEVEL_0;
			}
			if ( animTimer < 900 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 450 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_DL_S_T_SB_1_W:
			if ( saberNum != 0 )
			{//only right hand saber does damage in this suberbreak
				return FORCE_LEVEL_0;
			}
			if ( animTimer < 250 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 150 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_ST_DL_S_SB_1_W:
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_ST_DL_T_SB_1_W:
			//special suberbreak - doesn't kill, just kicks them backwards
			return FORCE_LEVEL_0;
			break;
		case BOTH_LK_ST_ST_S_SB_1_W:
		case BOTH_LK_ST_S_S_SB_1_W:
			if ( animTimer < 800 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 350 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			return FORCE_LEVEL_5;
			break;
		case BOTH_LK_ST_ST_T_SB_1_W:
		case BOTH_LK_ST_S_T_SB_1_W:
			return FORCE_LEVEL_5;
			break;
	//===SABERLOCK SUPERBREAKS START===========================================================================
		case BOTH_HANG_ATTACK:
			//FIME: break up
			if ( animTimer < 1000 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else if ( animTimeElapsed < 250 )
			{//beginning of anim
				return FORCE_LEVEL_0;
			}
			else
			{//sweet spot
				return FORCE_LEVEL_5;
			}
			break;
		case BOTH_ROLL_STAB:
			if ( mySaberHit )
			{//someone else hit my saber, not asking for damage level, but defense strength
				return FORCE_LEVEL_1;
			}
			if ( animTimeElapsed > 400 )
			{//end of anim
				return FORCE_LEVEL_0;
			}
			else
			{
				return FORCE_LEVEL_3;
			}
			break;
		}
		return FORCE_LEVEL_0;
	}
}

#define MAX_SABER_VICTIMS 16
static int		victimEntityNum[MAX_SABER_VICTIMS];
static qboolean victimHitEffectDone[MAX_SABER_VICTIMS];
static float	totalDmg[MAX_SABER_VICTIMS];
static vec3_t	dmgDir[MAX_SABER_VICTIMS];
static vec3_t	dmgSpot[MAX_SABER_VICTIMS];
static qboolean dismemberDmg[MAX_SABER_VICTIMS];
static int saberKnockbackFlags[MAX_SABER_VICTIMS];
static int numVictims = 0;
void WP_SaberClearDamage( void )
{
	int ven;
	for ( ven = 0; ven < MAX_SABER_VICTIMS; ven++ )
	{
		victimEntityNum[ven] = ENTITYNUM_NONE;
	}
	memset( victimHitEffectDone, 0, sizeof( victimHitEffectDone ) );
	memset( totalDmg, 0, sizeof( totalDmg ) );
	memset( dmgDir, 0, sizeof( dmgDir ) );
	memset( dmgSpot, 0, sizeof( dmgSpot ) );
	memset( dismemberDmg, 0, sizeof( dismemberDmg ) );
	memset( saberKnockbackFlags, 0, sizeof( saberKnockbackFlags ) );
	numVictims = 0;
}

void WP_SaberDamageAdd( gentity_t *inflictor, int trVictimEntityNum, vec3_t trDmgDir, vec3_t trDmgSpot, 
					   int trDmg, qboolean doDismemberment, int knockBackFlags, int saberNum )
{
	if ( trVictimEntityNum < 0 || trVictimEntityNum >= ENTITYNUM_WORLD )
	{
		return;
	}
	
	if ( trDmg )
	{//did some damage to something
		int curVictim = 0;
		int i;

		for ( i = 0; i < numVictims; i++ )
		{
			if ( victimEntityNum[i] == trVictimEntityNum )
			{//already hit this guy before
				curVictim = i;
				break;
			}
		}
		if ( i == numVictims )
		{//haven't hit his guy before
			if ( numVictims + 1 >= MAX_SABER_VICTIMS )
			{//can't add another victim at this time
				return;
			}
			//add a new victim to the list
			curVictim = numVictims;
			victimEntityNum[numVictims++] = trVictimEntityNum;
		}

		trDmg *= inflictor->client->saber[saberNum].extraDamage;
		totalDmg[curVictim] += trDmg;
		if ( VectorCompare( dmgDir[curVictim], vec3_origin ) )
		{
			VectorCopy( trDmgDir, dmgDir[curVictim] );
		}
		if ( VectorCompare( dmgSpot[curVictim], vec3_origin ) )
		{
			VectorCopy( trDmgSpot, dmgSpot[curVictim] );
		}
		if ( doDismemberment )
		{
			dismemberDmg[curVictim] = qtrue;
		}
		saberKnockbackFlags[curVictim] |= knockBackFlags;
	}
}

void WP_SaberApplyDamage( gentity_t *self )
{
	int i;
	if ( !numVictims )
	{
		return;
	}
	for ( i = 0; i < numVictims; i++ )
	{
		gentity_t *victim = NULL;
		int dflags = 0;

		victim = &g_entities[victimEntityNum[i]];

// nmckenzie: SABER_DAMAGE_WALLS
		if ( !victim->client )
		{
			totalDmg[i] *= g_saberWallDamageScale.value;
		}

		if ( !dismemberDmg[i] )
		{//don't do dismemberment!
			dflags |= DAMAGE_NO_DISMEMBER;
		}
		dflags |= saberKnockbackFlags[i];

		G_Damage( victim, self, self, dmgDir[i], dmgSpot[i], totalDmg[i], dflags, MOD_SABER );
	}
}


static void WP_SaberDoHit( gentity_t *self, int saberNum, int bladeNum )
{
	int i;
	if ( !numVictims )
	{
		return;
	}
	for ( i = 0; i < numVictims; i++ )
	{
		gentity_t *te = NULL, *victim = NULL;
		qboolean isDroid = qfalse;

		if ( victimHitEffectDone[i] )
		{
			continue;
		}

		victimHitEffectDone[i] = qtrue;

		victim = &g_entities[victimEntityNum[i]];

		if ( victim->client )
		{
			class_t npc_class = victim->client->NPC_class;

			if ( npc_class == CLASS_SEEKER || npc_class == CLASS_PROBE || npc_class == CLASS_MOUSE || npc_class == CLASS_REMOTE ||
					npc_class == CLASS_GONK || npc_class == CLASS_R2D2 || npc_class == CLASS_R5D2 ||
					npc_class == CLASS_PROTOCOL || npc_class == CLASS_MARK1 || npc_class == CLASS_MARK2 ||
					npc_class == CLASS_INTERROGATOR || npc_class == CLASS_ATST || npc_class == CLASS_SENTRY )  
			{ //don't make "blood" sparks for droids.
				isDroid = qtrue;
			}
		}

		te = G_TempEntity( dmgSpot[i], EV_SABER_HIT );
		if ( te )
		{
			te->s.otherEntityNum = victimEntityNum[i];
			te->s.otherEntityNum2 = self->s.number;
			te->s.weapon = saberNum;
			te->s.legsAnim = bladeNum;

			VectorCopy(dmgSpot[i], te->s.origin);
			//VectorCopy(tr.plane.normal, te->s.angles);
			VectorScale( dmgDir[i], -1, te->s.angles);
			
			if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
			{ //don't let it play with no direction
				te->s.angles[1] = 1;
			}

			if (!isDroid && (victim->client || victim->s.eType == ET_NPC ||
				victim->s.eType == ET_BODY))
			{
				if ( totalDmg[i] < 5 )
				{
					te->s.eventParm = 3;
				}
				else if (totalDmg[i] < 20 )
				{
					te->s.eventParm = 2;
				}
				else
				{
					te->s.eventParm = 1;
				}
			}
			else
			{
				if ( !(self->client->saber[saberNum].saberFlags2&SFL2_NO_CLASH_FLARE) )
				{
					if (totalDmg[i] > SABER_NONATTACK_DAMAGE)
					{ //I suppose I could tie this into the saberblock event, but I'm tired of adding flags to that thing.
						gentity_t *teS = G_TempEntity( te->s.origin, EV_SABER_CLASHFLARE );
						VectorCopy(te->s.origin, teS->s.origin);
					}
					te->s.eventParm = 0;
				}
			}
		}
	}
}

extern qboolean G_EntIsBreakable( int entityNum );
void WP_SaberRadiusDamage( gentity_t *ent, vec3_t point, float radius, int damage, float knockBack )
{
	if ( !ent || !ent->client )
	{
		return;
	}
	else if ( radius <= 0.0f || (damage <= 0 && knockBack <= 0) )
	{
		return;
	}
	else
	{
		vec3_t		mins, maxs, entDir;
		int			radiusEnts[128];
		gentity_t	*radiusEnt = NULL;
		int			numEnts, i;
		float		dist;

		//Setup the bbox to search in
		for ( i = 0; i < 3; i++ )
		{
			mins[i] = point[i] - radius;
			maxs[i] = point[i] + radius;
		}

		//Get the number of entities in a given space
		numEnts = trap->EntitiesInBox( mins, maxs, radiusEnts, 128 );

		for ( i = 0; i < numEnts; i++ )
		{
			radiusEnt = &g_entities[radiusEnts[i]];
			if ( !radiusEnt->inuse )
			{
				continue;
			}
			
			if ( radiusEnt == ent )
			{//Skip myself
				continue;
			}
			
			if ( radiusEnt->client == NULL )
			{//must be a client
				if ( G_EntIsBreakable( radiusEnt->s.number ) )
				{//damage breakables within range, but not as much
					G_Damage( radiusEnt, ent, ent, vec3_origin, radiusEnt->r.currentOrigin, 10, 0, MOD_MELEE );
				}
				continue;
			}

			if ( (radiusEnt->client->ps.eFlags2&EF2_HELD_BY_MONSTER) )
			{//can't be one being held
				continue;
			}
			
			VectorSubtract( radiusEnt->r.currentOrigin, point, entDir );
			dist = VectorNormalize( entDir );
			if ( dist <= radius )
			{//in range
				if ( damage > 0 )
				{//do damage
					int points = ceil((float)damage*dist/radius);
					G_Damage( radiusEnt, ent, ent, vec3_origin, radiusEnt->r.currentOrigin, points, DAMAGE_NO_KNOCKBACK, MOD_MELEE );
				}
				if ( knockBack > 0 )
				{//do knockback
					if ( radiusEnt->client
						&& radiusEnt->client->NPC_class != CLASS_RANCOR
						&& radiusEnt->client->NPC_class != CLASS_ATST
						&& !(radiusEnt->flags&FL_NO_KNOCKBACK) )//don't throw them back
					{
						float knockbackStr = knockBack*dist/radius;
						entDir[2] += 0.1f;
						VectorNormalize( entDir );
						G_Throw( radiusEnt, entDir, knockbackStr );
						if ( radiusEnt->health > 0 )
						{//still alive
							if ( knockbackStr > 50 )
							{//close enough and knockback high enough to possibly knock down
								if ( dist < (radius*0.5f)
									|| radiusEnt->client->ps.groundEntityNum != ENTITYNUM_NONE )
								{//within range of my fist or within ground-shaking range and not in the air
									//[KnockdownSys]
									//ported multi-direction knockdowns from SP.
									//racc - FYI, this appears to be code pulled from Tavion's spector slam, not sure
									//why it's in the saber radius damage stuff.
									G_Knockdown( radiusEnt, ent, entDir, 500, qtrue );
									//G_Knockdown( radiusEnt );//, ent, entDir, 500, qtrue );
									//[/KnockdownSys]
								}
							}
						}
					}
				}
			}
		}
	}
}

static qboolean saberDoClashEffect = qfalse;
static vec3_t saberClashPos = {0};
static vec3_t saberClashNorm = {0};
static int saberClashEventParm = 1;
void WP_SaberDoClash( gentity_t *self, int saberNum, int bladeNum )
{
	if ( saberDoClashEffect )
	{
		gentity_t *te = G_TempEntity( saberClashPos, EV_SABER_BLOCK );
		VectorCopy(saberClashPos, te->s.origin);
		VectorCopy(saberClashNorm, te->s.angles);
		te->s.eventParm = saberClashEventParm;
		te->s.otherEntityNum2 = self->s.number;
		te->s.weapon = saberNum;
		te->s.legsAnim = bladeNum;
	}
}

void WP_SaberBounceSound( gentity_t *ent, int saberNum, int bladeNum )
{
	int index = 1;
	if ( !ent || !ent->client )
	{
		return;
	}
	index = Q_irand( 1, 9 );
	if ( ent->client->saber[saberNum].bounceSound[0] )
	{
		G_Sound( ent, CHAN_AUTO, ent->client->saber[saberNum].bounceSound[Q_irand( 0, 2 )] );
	}
	else if ( ent->client->saber[saberNum].blockSound[0] )
	{
		G_Sound( ent, CHAN_AUTO, ent->client->saber[saberNum].blockSound[Q_irand( 0, 2 )] );
	}
	else
	{	
		G_Sound( ent, CHAN_AUTO, G_SoundIndex( va( "sound/weapons/saber/saberblock%d.wav", index ) ) );
	}
}

// Watch out! This assumes that the attacker and the blocker are both valid clients
unsigned char JKG_GetBPNeededForBlock( gentity_t *blocker, int attackerSaberMove, int attackerSaberStyle )
{
	qboolean iForward = (blocker->client->pers.cmd.forwardmove > 0);
	qboolean iBackward = (blocker->client->pers.cmd.forwardmove < 0);
	qboolean iRight = (blocker->client->pers.cmd.rightmove > 0);
	qboolean iLeft = (blocker->client->pers.cmd.rightmove < 0);
	int numPlanesWrong = 0;			// the BP loss is based on the number of planes wrong, and the style

	if(iForward)
	{
		// Going forward
		if(iLeft)
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 2;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 1;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 0;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 2;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 1;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 0;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 1;
					break;
			}
		}
		else if(iRight)
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 0;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 1;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 2;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 0;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 1;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 2;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 1;
					break;
			}
		}
		else
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 1;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 2;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 1;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 1;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 2;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 1;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 0;
					break;
			}
		}
	}
	else if(iBackward)
	{
		// Going backward
		if(iLeft)
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 0;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 1;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 2;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 0;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 1;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 2;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 1;
					break;
			}
		}
		else if(iRight)
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 2;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 1;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 0;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 2;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 1;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 0;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 1;
					break;
			}
		}
		else
		{
			switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 1;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 2;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 1;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 1;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 2;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 1;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 0;
					break;
			}
		}
	}
	else if(iRight)
	{
		switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 1;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 2;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 1;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 1;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 2;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 1;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 0;
					break;
			}
	}
	else if(iLeft)
	{
		switch(attackerSaberMove)
			{
				case LS_A_TL2BR:	// Top left to bottom right
					numPlanesWrong = 1;
					break;
				case LS_A_L2R:		// Left to right
					numPlanesWrong = 2;
					break;
				case LS_A_BL2TR:	// Bottom left to top right
					numPlanesWrong = 1;
					break;
				case LS_A_BR2TL:	// Bottom right to top left
					numPlanesWrong = 1;
					break;
				case LS_A_R2L:		// Right to left
					numPlanesWrong = 2;
					break;
				case LS_A_TR2BL:	// Top right to bottom left
					numPlanesWrong = 1;
					break;
				case LS_A_T2B:		// Top to bottom
					numPlanesWrong = 0;
					break;
			}
	}
	else
	{
		// Standing still I guess
		switch(attackerSaberMove)
		{
			case LS_A_TL2BR:	// Top left to bottom right
				numPlanesWrong = 1;
				break;
			case LS_A_L2R:		// Left to right
				numPlanesWrong = 0;
				break;
			case LS_A_BL2TR:	// Bottom left to top right
				numPlanesWrong = 1;
				break;
			case LS_A_BR2TL:	// Bottom right to top left
				numPlanesWrong = 1;
				break;
			case LS_A_R2L:		// Right to left
				numPlanesWrong = 0;
				break;
			case LS_A_TR2BL:	// Top right to bottom left
				numPlanesWrong = 1;
				break;
			case LS_A_T2B:		// Top to bottom
				numPlanesWrong = 2;
				break;
		}
	}

	//return 30; // for now

	return SaberStances[attackerSaberStyle].BPdrain*numPlanesWrong;
}

static qboolean saberHitWall = qfalse;
static qboolean saberHitSaber = qfalse;
static float saberHitFraction = 1.0f;
//rww - MP version of the saber damage function. This is where all the things like blocking, triggering a parry,
//triggering a broken parry, doing actual damage, etc. are done for the saber. It doesn't resemble the SP
//version very much, but functionality is (hopefully) about the same.
//This is a large function. I feel sort of bad inlining it. But it does get called tons of times per frame.
qboolean BG_SuperBreakWinAnim( int anim );

void DebounceSaberImpact(gentity_t *self, gentity_t *otherSaberer, 
						 int rSaberNum, int rBladeNum, int sabimpactentitynum)
{//this function adds the nessicary debounces for saber impacts so that we can do
	//consistant damamge to players
	/*
	self = player of the checksaberdamage.
	otherSaberer = the other saber owner if we hit their saber as well.
	rSaberNum = the saber number with the impact
	rBladeNum = the blade number on the saber with the impact
	sabimpactentitynum = number of the entity that we hit 
	(if a saber was hit, it's owner is used for this value)
	*/

	//add basic saber impact debounce
	self->client->sabimpact[rSaberNum][rBladeNum].EntityNum = sabimpactentitynum;
	self->client->sabimpact[rSaberNum][rBladeNum].Debounce = level.time;

	if(otherSaberer)
	{//we hit an enemy saber so update our sabimpact data with that info for us and the enemy.
		self->client->sabimpact[rSaberNum][rBladeNum].SaberNum = self->client->lastSaberCollided;
		self->client->sabimpact[rSaberNum][rBladeNum].BladeNum = self->client->lastBladeCollided;

		//Also add this impact to the otherowner so he doesn't do do his behavior rolls twice.
		otherSaberer->client->sabimpact[self->client->lastSaberCollided][self->client->lastBladeCollided].EntityNum = self->client->ps.saberEntityNum;
		otherSaberer->client->sabimpact[self->client->lastSaberCollided][self->client->lastBladeCollided].Debounce = level.time;
		otherSaberer->client->sabimpact[self->client->lastSaberCollided][self->client->lastBladeCollided].SaberNum = rSaberNum;
		otherSaberer->client->sabimpact[self->client->lastSaberCollided][self->client->lastBladeCollided].BladeNum = rBladeNum;
	}
	else
	{//blank out the saber blade impact stuff since we didn't hit another guy's saber
		self->client->sabimpact[rSaberNum][rBladeNum].SaberNum = -1;
		self->client->sabimpact[rSaberNum][rBladeNum].BladeNum = -1;
	}
}


/*
====================================
CheckSaberDamage

This function gets called whenever we are to damage a player in lightsaber combat.
This doesn't actually apply the damage however, so we use this for blocking
detection and stuff like that.
====================================
*/

void DebounceSaberImpact(gentity_t *self, gentity_t *otherSaberer, 
						 int rSaberNum, int rBladeNum, int sabimpactentitynum);
static QINLINE qboolean CheckSaberDamage(gentity_t *self, int rSaberNum, int rBladeNum, vec3_t saberStart, vec3_t saberEnd, qboolean doInterpolate, int trMask, qboolean extrapolate )
{
	static trace_t tr;
	static vec3_t dir;
	static vec3_t saberTrMins, saberTrMaxs;
	static vec3_t lastValidStart;
	static vec3_t lastValidEnd;
	static int selfSaberLevel;
	static int otherSaberLevel;
	int dmg = 0;
	int attackStr = 0;
	float saberBoxSize = 0;
	qboolean idleDamage = qfalse;
	qboolean didHit = qfalse;
	qboolean sabersClashed = qfalse;
	qboolean unblockable = qfalse;
	qboolean didDefense = qfalse;
	qboolean didOffense = qfalse;
	qboolean saberTraceDone = qfalse;
	qboolean otherUnblockable = qfalse;
	qboolean tryDeflectAgain = qfalse;
	qboolean	saberHitWall = qfalse;
	//boxscale for blockbox
	float boxScale = 10;
	
	int sabimpactdebounce;
	int sabimpactentitynum;

	gentity_t *otherOwner = NULL; //gentity_t *otherOwner;

	if (BG_SabersOff( &self->client->ps ))
	{
		return qfalse;
	}

	selfSaberLevel = G_SaberAttackPower(self, SaberAttacking(self));
	//Add the standard radius into the box size
	saberBoxSize += (self->client->saber[rSaberNum].blade[rBladeNum].radius*0.5f);
	VectorSet(saberTrMins, -saberBoxSize*boxScale, -saberBoxSize*boxScale, -saberBoxSize*boxScale);
	VectorSet(saberTrMaxs, saberBoxSize*boxScale, saberBoxSize*boxScale, saberBoxSize*boxScale);
	boxScale *= 0.3;

	if (self->client->ps.weaponTime <= 0)
	{ //if not doing any attacks or anything, just use point traces.
		VectorClear(saberTrMins);
		VectorClear(saberTrMaxs);
	}
	else
	{
		VectorSet(saberTrMins, -saberBoxSize*boxScale, -saberBoxSize*boxScale, -saberBoxSize*boxScale);
		VectorSet(saberTrMaxs, saberBoxSize*boxScale, saberBoxSize*boxScale, saberBoxSize*boxScale);
		boxScale *= 0.3;
	}

	//saber impact debouncer stuff
	if(idleDamage)
	{
		sabimpactdebounce = g_saberDmgDelay_Idle.integer;
	}
	else
	{
		sabimpactdebounce = g_saberDmgDelay_Wound.integer;
	}

	if(otherOwner)
	{
		sabimpactentitynum = otherOwner->client->ps.saberEntityNum;
	}
	else
	{
		sabimpactentitynum = tr.entityNum;
	}

	if(self->client->sabimpact[rSaberNum][rBladeNum].EntityNum == sabimpactentitynum
		&& ((level.time - self->client->sabimpact[rSaberNum][rBladeNum].Debounce) < sabimpactdebounce))
	{//the impact debounce for this entity isn't up yet.
		if( self->client->sabimpact[rSaberNum][rBladeNum].BladeNum != -1 
			|| self->client->sabimpact[rSaberNum][rBladeNum].SaberNum != -1)
		{//the last impact was a saber on this entity
			if(otherOwner)
			{
				if (self->client->sabimpact[rSaberNum][rBladeNum].BladeNum == self->client->lastBladeCollided
					&& self->client->sabimpact[rSaberNum][rBladeNum].SaberNum == self->client->lastSaberCollided)
				{
					return qtrue;
				}
			}
		}
		else
		{//last impact was this saber.
			return qtrue;
		}
	}//Stoiss end
	//end saber impact debouncer stuff

	while (!saberTraceDone)
	{
		// do not interpolate (or, use SP style attacks) --eez
		// Need to figure out what the difference is though.
		vec3_t saberEndExtrapolated;
		if ( extrapolate )
		{//extrapolate 16
			vec3_t diff;
			VectorSubtract( saberEnd, saberStart, diff );
			VectorNormalize( diff );
			VectorMA( saberStart, SABER_EXTRAPOLATE_DIST, diff, saberEndExtrapolated );
		}
		else
		{
			VectorCopy( saberEnd, saberEndExtrapolated );
		}
		trap->Trace(&tr, saberStart, saberTrMins, saberTrMaxs, saberEndExtrapolated, self->s.number, trMask, 0, 0, 0);

		VectorCopy(saberStart, lastValidStart);
		VectorCopy(saberEndExtrapolated, lastValidEnd);
		/*
		if ( tr.allsolid || tr.startsolid )
		{
			if ( tr.entityNum == ENTITYNUM_NONE )
			{
				qboolean whah = qtrue;
			}
			Com_Printf( "saber trace start/all solid - ent is %d\n", tr.entityNum );
		}
		*/
		if (tr.entityNum < MAX_CLIENTS)
		{
			G_G2TraceCollide(&tr, lastValidStart, lastValidEnd, saberTrMins, saberTrMaxs);
		}
		else if (tr.entityNum < ENTITYNUM_WORLD)
		{
			gentity_t *trHit = &g_entities[tr.entityNum];

			if (trHit->inuse && trHit->ghoul2)
			{ //hit a non-client entity with a g2 instance
				G_G2TraceCollide(&tr, lastValidStart, lastValidEnd, saberTrMins, saberTrMaxs);
			}
		}

		saberTraceDone = qtrue;
	}

	//eez:
	if ( self->client->saberAttackWound < level.time																									// saber damage debounce
		&& (SaberAttacking(self)																														// attacking with saber?
			|| BG_SuperBreakWinAnim(self->client->ps.torsoAnim)
			|| (self->client->ps.saberInFlight&&rSaberNum==0)
			|| (WP_SaberBladeDoTransitionDamage( &self->client->saber[rSaberNum], rBladeNum )&&BG_SaberInTransitionAny(self->client->ps.saberMove)))
	   )
	{ //this animation is that of the last attack movement, and so it should do full damage					// eezstreet note: this code here is what pissed off a lot of JK2 1.02 players. FIXME?
		float fDmg = 0.0f;
		if ( self->client->ps.saberInFlight )
		{
			gentity_t *saberEnt = &g_entities[self->client->ps.saberEntityNum];
			if ( !saberEnt 
				|| !saberEnt->s.saberInFlight )
			{//does less damage on the way back
				fDmg = 1.0f;
				attackStr = FORCE_LEVEL_0;
			}
			else
			{
				fDmg = 2.5f*self->client->ps.fd.forcePowerLevel[FP_SABERTHROW];
				attackStr = FORCE_LEVEL_1;
			}
		}
		else
		{
			attackStr = G_PowerLevelForSaberAnim( self, rSaberNum, qfalse );
			if ( g_saberRealisticCombat.integer )
			{
				switch ( attackStr )
				{
				default:
				case FORCE_LEVEL_3:
					fDmg = 10.0f;
					break;
				case FORCE_LEVEL_2:
					fDmg = 5.0f;
					break;
				case FORCE_LEVEL_1:
				case FORCE_LEVEL_0:
					fDmg = 2.5f;
					break;
				}
			}
			else
			{
				if ( self->client->ps.torsoAnim == BOTH_SPINATTACK6
					|| self->client->ps.torsoAnim == BOTH_SPINATTACK7 )
				{//too easy to do, lower damage
					fDmg = 2.5f;
				}
				else
				{
					fDmg = 2.5f * (float)attackStr;
				}
			}
		}
		if ( g_saberRealisticCombat.integer > 1 )
		{//always do damage, and lots of it
			if ( g_saberRealisticCombat.integer > 2 )
			{//always do damage, and lots of it
				fDmg = 25.0f;
			}
			else if ( fDmg > 0.1f )
			{//only do super damage if we would have done damage according to normal rules
				fDmg = 25.0f;
			}
		}
		/*
		if ( dmg > 0.1f )
		{
			if ( (self->client->ns.forcePowersActive&(1<<FP_RAGE)) )
			{//add some damage if raged
				dmg += self->client->ns.forcePowerLevel[FP_RAGE] * 5.0f;
			}
			else if ( self->client->ps.forceRageRecoveryTime )
			{//halve it if recovering
				dmg *= 0.5f;
			}
		}
		*/
		if ( level.gametype != GT_DUEL
			&& level.gametype != GT_POWERDUEL )
		{//in faster-paced games, sabers do more damage
			fDmg *= 2.0f;
		}
		if ( fDmg )
		{//the longer the trace, the more damage it does
			//FIXME: in SP, we only use the part of the trace that's actually *inside* the hit ent...
			float traceLength = Distance( saberEnd, saberStart );
			if ( tr.fraction >= 1.0f )
			{//allsolid?
				dmg = ceil( fDmg*traceLength*0.1f*0.33f );
			}
			else
			{//fractional hit, the sooner you hit in the trace, the more damage you did
				dmg = ceil( fDmg*traceLength*(1.0f-tr.fraction)*0.1f*0.33f );//(1.0f-tr.fraction) isn't really accurate, but kind of simulates what we have in SP
			}
#ifdef DEBUG_SABER_BOX
			if ( g_saberDebugBox.integer == 3 || g_saberDebugBox.integer == 4 )
			{
				G_TestLine( saberStart, saberEnd, 0x0000ff, 50 );
			}
#endif
		}
		/*
		if ( dmg )
		{
			Com_Printf("CL %i SABER DMG: %i, anim %s, torsoTimer %i\n", self->s.number, dmg, animTable[self->client->ps.torsoAnim].name, self->client->ps.torsoTimer );
		}
		*/
		if ( self->client->ps.torsoAnim == BOTH_A1_SPECIAL
			|| self->client->ps.torsoAnim == BOTH_A2_SPECIAL
			|| self->client->ps.torsoAnim == BOTH_A3_SPECIAL )
		{//parry/block/break-parry bonus for single-style kata moves
			attackStr++;
		}
		if ( BG_SuperBreakWinAnim( self->client->ps.torsoAnim ) )
		{
			trMask &= ~CONTENTS_LIGHTSABER;
		}
		
	}
	else if (self->client->saberAttackWound < level.time &&
		self->client->saberIdleWound < level.time)
	{ //just touching, do minimal damage and only check for it every 200ms (mainly to cut down on network traffic for hit events)
		if ( (self->client->saber[0].saberFlags2&SFL2_NO_IDLE_EFFECT) )
		{//no idle damage or effects
			return qtrue;//true cause even though we didn't get a hit, we don't want to do those extra traces because the debounce time says not to.
		}
		trMask &= ~CONTENTS_LIGHTSABER;
		dmg = SABER_NONATTACK_DAMAGE;
		idleDamage = qtrue;
	}
	else
	{
		return qtrue; //true cause even though we didn't get a hit, we don't want to do those extra traces because the debounce time says not to.
	}

	if (BG_SaberInSpecial(self->client->ps.saberMove))
	{
		unblockable = qtrue;
		self->client->ps.saberBlocked = 0;
	}

	if (!dmg)
	{
		if (tr.entityNum < MAX_CLIENTS ||
			(g_entities[tr.entityNum].inuse && (g_entities[tr.entityNum].r.contents & CONTENTS_LIGHTSABER)))
		{
			return qtrue;
		}
		return qfalse;
	}

	if (dmg > SABER_NONATTACK_DAMAGE)
	{
		dmg *= g_saberDamageScale.value;

		//see if this specific saber has a damagescale
		if ( self->client->saber[rSaberNum].damageScale != 1.0f )
		{
			dmg = ceil( (float)dmg*self->client->saber[rSaberNum].damageScale );
		}

		if ((self->client->ps.brokenLimbs & (1 << BROKENLIMB_RARM)) ||
			(self->client->ps.brokenLimbs & (1 << BROKENLIMB_LARM)))
		{ //weaken it if an arm is broken
			dmg *= 0.3;
			if (dmg <= SABER_NONATTACK_DAMAGE)
			{
				dmg = SABER_NONATTACK_DAMAGE+1;
			}
		}
	}

#ifndef FINAL_BUILD
	if (g_saberDebugPrint.integer > 2 && dmg > 1)
	{
		Com_Printf("CL %i SABER DMG: %i\n", self->s.number, dmg);
	}
#endif

	VectorSubtract(saberEnd, saberStart, dir);
	VectorNormalize(dir);

	if (tr.entityNum == ENTITYNUM_WORLD ||
		g_entities[tr.entityNum].s.eType == ET_TERRAIN)
	{ //register this as a wall hit for jedi AI
        self->client->ps.saberEventFlags |= SEF_HITWALL;
		saberHitWall = qtrue;
	}

	if (saberHitWall 
		&& (self->client->saber[rSaberNum].saberFlags & SFL_BOUNCE_ON_WALLS)
		&& (BG_SaberInAttackPure( self->client->ps.saberMove ) //only in a normal attack anim
			|| self->client->ps.saberMove == LS_A_JUMP_T__B_ ) //or in the strong jump-fwd-attack "death from above" move
		)
	{ //then bounce off
		/*
		qboolean onlyIfNotSpecial = qfalse;
		qboolean skipIt = qfalse;
		if (tr.plane.normal[2] >= 0.8f ||
			tr.plane.normal[2] <= -0.8f ||
			VectorCompare(tr.plane.normal, vec3_origin))
		{
			if ((tr.plane.normal[2] >= 0.8f || VectorCompare(tr.plane.normal, vec3_origin)) &&
				self->client->ps.viewangles[PITCH] <= 30.0f &&
				self->client->pers.cmd.upmove >= 0)
			{ //don't hit the ground if we are not looking down a lot/crouched
				skipIt = qtrue;
			}
			else
			{
				onlyIfNotSpecial = qtrue;
			}
		}
		if (!skipIt && (!onlyIfNotSpecial || !BG_SaberInSpecial(self->client->ps.saberMove)))
		*/
		{
			gentity_t *te = NULL;
			
			qboolean pre = saberDoClashEffect;

			VectorCopy( tr.endpos, saberClashPos );
			VectorCopy( tr.plane.normal, saberClashNorm );
			saberClashEventParm = 1;
			saberDoClashEffect = qtrue;
			WP_SaberDoClash( self, rSaberNum, rBladeNum );
			saberDoClashEffect = pre;
			

			self->client->ps.saberMove = BG_BrokenParryForAttack(self->client->ps.saberMove, self->client->ps.fd.saberAnimLevel);
			self->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;
			if (self->client->ps.torsoAnim == self->client->ps.legsAnim)
			{ //set anim now on both parts
				int anim = SaberStances[self->client->ps.fd.saberAnimLevel].moves[self->client->ps.saberMove].anim;
				G_SetAnim(self, &self->client->pers.cmd, SETANIM_BOTH, anim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}

			//do bounce sound & force feedback
			WP_SaberBounceSound( self, rSaberNum, rBladeNum );
			//do hit effect
			te = G_TempEntity( tr.endpos, EV_SABER_HIT );
			te->s.otherEntityNum = ENTITYNUM_NONE;//we didn't hit anyone in particular
			te->s.otherEntityNum2 = self->s.number;//send this so it knows who we are
			te->s.weapon = rSaberNum;
			te->s.legsAnim = rBladeNum;
			VectorCopy(tr.endpos, te->s.origin);
			VectorCopy(tr.plane.normal, te->s.angles);
			if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
			{ //don't let it play with no direction
				te->s.angles[1] = 1;
			}
			//do radius damage/knockback, if any
			WP_SaberRadiusDamage( self, tr.endpos, self->client->saber[rSaberNum].splashRadius, self->client->saber[rSaberNum].splashDamage, self->client->saber[rSaberNum].splashKnockback );
	
			return qtrue;
		}
	}

	//rww - I'm saying || tr.startsolid here, because otherwise your saber tends to skip positions and go through
	//people, and the compensation traces start in their bbox too. Which results in the saber passing through people
	//when you visually cut right through them. Which sucks.

	if ((tr.fraction != 1 || tr.startsolid) &&
		g_entities[tr.entityNum].takedamage &&
		(g_entities[tr.entityNum].health > 0 || !(g_entities[tr.entityNum].s.eFlags & EF_DISINTEGRATION)) &&
		tr.entityNum != self->s.number &&
		g_entities[tr.entityNum].inuse)
	{//hit something that had health and takes damage
		if (idleDamage &&
			g_entities[tr.entityNum].client &&
			OnSameTeam(self, &g_entities[tr.entityNum]) &&
			!g_friendlySaber.integer)
		{
			return qfalse;
		}

		if (g_entities[tr.entityNum].client &&
			g_entities[tr.entityNum].client->ps.duelInProgress &&
			g_entities[tr.entityNum].client->ps.duelIndex != self->s.number)
		{
			return qfalse;
		}

		if (g_entities[tr.entityNum].client &&
			self->client->ps.duelInProgress &&
			self->client->ps.duelIndex != g_entities[tr.entityNum].s.number)
		{
			return qfalse;
		}

		if ( BG_StabDownAnim( self->client->ps.torsoAnim )
			&& g_entities[tr.entityNum].client 
			&& !BG_InKnockDownOnGround( &g_entities[tr.entityNum].client->ps ) )
		{//stabdowns only damage people who are actually on the ground...
			return qfalse;
		}

		if ( WP_SaberCanBlock( &g_entities[tr.entityNum], tr.endpos, 0, MOD_SABER, qfalse, attackStr ) && 
			(g_entities[tr.entityNum].client->saberBlockDebounce < level.time) )
		{
			// Block, damn it! But only if we have enough BP...
			unsigned char BPneeded;
			otherOwner = &g_entities[tr.entityNum];

			// Figure out how much BP to get from that.
			BPneeded = JKG_GetBPNeededForBlock(&g_entities[tr.entityNum], self->client->ps.saberMove, self->client->ps.fd.saberAnimLevel);

			// TODO: make this based off of style and all that jazz...
			if(otherOwner->client->ps.blockPoints >= BPneeded)
			{
				saberDoClashEffect = qtrue;
				VectorCopy( tr.endpos, saberClashPos );
				VectorCopy( tr.plane.normal, saberClashNorm );
				saberClashEventParm = 1;
				if(otherOwner->client->saberSaberBlockDebounce < level.time)
				{
					// BP grace time
					otherOwner->client->ps.blockPoints -= BPneeded;
					if(otherOwner->client->ps.blockPoints < 0)
					{
						// rollover warning
						otherOwner->client->ps.blockPoints = 0;
					}
					// Don't recreate MB2's inconsistent method of draining BP, aka, drain a static amount per swing
					otherOwner->client->saberSaberBlockDebounce = level.time + self->client->ps.torsoTimer - 50; // like a 50ms grace period
					if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
					{
						tryDeflectAgain = qtrue; //Failed the deflect, try it again if we can if the guy we're smashing goes into a parry and we don't break it
					}
					else
					{
						int preMove = otherOwner->client->ps.saberMove;

						otherOwner->client->ps.saberMove = G_GetParryForBlock(otherOwner->client->ps.saberBlocked);
						WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction);
						otherOwner->client->ps.saberMove = preMove;
					}
				}
				goto blockStuff;
			}
		}

		self->client->saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;

		didHit = qtrue;

		//damage the thing we hit
		qboolean doDismemberment = qfalse;
		int	knockbackFlags = 0;

		if (g_entities[tr.entityNum].client)
		{ //not a "jedi", so make them suffer more
			if ( dmg > SABER_NONATTACK_DAMAGE )
			{ //don't bother increasing just for idle touch damage
				dmg *= 1.5;
			}
		}

		if (self->s.eType == ET_NPC &&
			g_entities[tr.entityNum].client &&
			self->client->playerTeam == g_entities[tr.entityNum].client->playerTeam &&
			dmg != SABER_NONATTACK_DAMAGE)
		{ //Oops. Since he's an NPC, we'll be forgiving and cut the damage down.
			dmg *= 0.2f;
		}

		//store the damage, we'll apply it later
		if ( !(self->client->saber[rSaberNum].saberFlags2&SFL2_NO_DISMEMBERMENT) )
		{
			doDismemberment = qtrue;
		}

		WP_SaberDamageAdd( self, tr.entityNum, dir, tr.endpos, dmg, doDismemberment, knockbackFlags, rSaberNum );

		if (g_entities[tr.entityNum].client)
		{
			//Let jedi AI know if it hit an enemy
			if ( self->enemy && self->enemy == &g_entities[tr.entityNum] )
			{
				self->client->ps.saberEventFlags |= SEF_HITENEMY;
			}
			else
			{
                self->client->ps.saberEventFlags |= SEF_HITOBJECT;
			}
		}
	}
	else if ((tr.fraction != 1 || tr.startsolid) &&
		(g_entities[tr.entityNum].r.contents & CONTENTS_LIGHTSABER) &&
		g_entities[tr.entityNum].r.contents != -1 &&
		g_entities[tr.entityNum].inuse)
	{ //saber clash
		otherOwner = &g_entities[g_entities[tr.entityNum].r.ownerNum];

		if (!otherOwner->inuse || !otherOwner->client)
		{
			return qfalse;
		}

		if ( otherOwner 
			&& otherOwner->client 
			&& otherOwner->client->ps.saberInFlight )
		{//don't do extra collision checking vs sabers in air
		}
		else
		{//hit an in-hand saber, do extra collision check against it
			if ( !WP_SabersIntersect( self, rSaberNum, rBladeNum, otherOwner, qfalse ) )
			{//sabers did not actually intersect
				return qfalse;
			}
		}

		if (OnSameTeam(self, otherOwner) &&
			!g_friendlySaber.integer)
		{
			return qfalse;
		}

		/*if ((self->s.eType == ET_NPC || otherOwner->s.eType == ET_NPC) && //just make sure one of us is an npc
			self->client->playerTeam == otherOwner->client->playerTeam)
		{ //don't hit your teammate's sabers if you are an NPC. It can be rather annoying.
			return qfalse;
		}*/ //Wrong

		if (otherOwner->client->ps.duelInProgress &&
			otherOwner->client->ps.duelIndex != self->s.number)
		{
			return qfalse;
		}

		if (self->client->ps.duelInProgress &&
			self->client->ps.duelIndex != otherOwner->s.number)
		{
			return qfalse;
		}

		if ( g_debugSaberLocks.integer )
		{
			WP_SabersCheckLock2( self, otherOwner, LOCK_RANDOM );
			return qtrue;
		}

		didHit = qtrue;
		self->client->saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;

		if (dmg <= SABER_NONATTACK_DAMAGE)
		{
			self->client->saberIdleWound = level.time + g_saberDmgDelay_Idle.integer;
		}

		saberDoClashEffect = qtrue;
		VectorCopy( tr.endpos, saberClashPos );
		VectorCopy( tr.plane.normal, saberClashNorm );
		saberClashEventParm = 1;

		sabersClashed = qtrue;
		saberHitSaber = qtrue;
		saberHitFraction = tr.fraction;

		if (saberCheckKnockdown_Smashed(&g_entities[tr.entityNum], otherOwner, self, dmg))
		{ //smashed it out of the air
			return qfalse;
		}

		//is this my thrown saber?
		if ( self->client->ps.saberEntityNum
			&& self->client->ps.saberInFlight
			&& rSaberNum == 0
			&& saberCheckKnockdown_Smashed( &g_entities[self->client->ps.saberEntityNum], self, otherOwner, dmg))
		{ //they smashed it out of the air
			return qfalse;
		}

blockStuff:
		otherUnblockable = qfalse;

		if (otherOwner && otherOwner->client && otherOwner->client->ps.saberInFlight)
		{
			return qfalse;
		}

		//this is a thrown saber, don't do any fancy saber-saber collision stuff
		if ( self->client->ps.saberEntityNum
			&& self->client->ps.saberInFlight
			&& rSaberNum == 0 )
		{
			return qfalse;
		}

		otherSaberLevel = G_SaberAttackPower(otherOwner, SaberAttacking(otherOwner));

		if (dmg > SABER_NONATTACK_DAMAGE && !unblockable && !otherUnblockable)
		{
			int lockFactor = g_saberLockFactor.integer;

			if (sabersClashed && Q_irand(1, 20) <= lockFactor)
			{
				if (!G_ClientIdleInWorld(otherOwner))
				{
					if (WP_SabersCheckLock(self, otherOwner))
					{
						self->client->ps.saberBlocked = BLOCKED_NONE;
						otherOwner->client->ps.saberBlocked = BLOCKED_NONE;
						return didHit;
					}
				}
			}
		}

		if (!otherOwner || !otherOwner->client)
		{
			return didHit;
		}

		if (BG_SaberInSpecial(otherOwner->client->ps.saberMove))
		{
			otherUnblockable = qtrue;
			otherOwner->client->ps.saberBlocked = 0;
		}

		// this guy is slick as shit. he blocked an attack with no help!
		if( !unblockable &&
			dmg > SABER_NONATTACK_DAMAGE && 
			!PM_SaberInBounce(self->client->ps.saberMove) &&
			 //!PM_SaberInParry(self->client->ps.saberMove) &&
			 !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
			 !BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
			 !PM_SaberInBounce(otherOwner->client->ps.saberMove)// &&
			 //!PM_SaberInDeflect(self->client->ps.saberMove) &&
			 /*!PM_SaberInReflect(self->client->ps.saberMove)*/ )
		{
			//otherOwner->client->ns.blockPoints += 20;
			WP_SaberBlockNonRandom( self, otherOwner, tr.endpos, qfalse );
			if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
			{
				tryDeflectAgain = qtrue; //Failed the deflect, try it again if we can if the guy we're smashing goes into a parry and we don't break it
			}
			else
			{
				int preMove = otherOwner->client->ps.saberMove;

				otherOwner->client->ps.saberMove = G_GetParryForBlock(otherOwner->client->ps.saberBlocked);
				WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction);
				otherOwner->client->ps.saberMove = preMove;
				return qfalse;
			}
		}

		if ( sabersClashed &&
			dmg > SABER_NONATTACK_DAMAGE &&
			 //selfSaberLevel < FORCE_LEVEL_3 &&
			 !PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
			 !PM_SaberInParry(self->client->ps.saberMove) &&
			 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
			 !BG_SaberInSpecial(self->client->ps.saberMove) &&
			 !PM_SaberInBounce(self->client->ps.saberMove) &&
			 !PM_SaberInDeflect(self->client->ps.saberMove) &&
			 !PM_SaberInReflect(self->client->ps.saberMove) &&
			 !unblockable )
		{
			//if (Q_irand(1, 10) <= 6)
			if (1) //for now, just always try a deflect. (deflect func can cause bounces too)
			{
				if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
				{
					tryDeflectAgain = qtrue; //Failed the deflect, try it again if we can if the guy we're smashing goes into a parry and we don't break it
				}
				else
				{
					self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
					didOffense = qtrue;
				}
			}
			else
			{
				self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				didOffense = qtrue;

#ifndef FINAL_BUILD
				if (g_saberDebugPrint.integer)
				{
					Com_Printf("Client %i clashed into client %i's saber, did BLOCKED_ATK_BOUNCE\n", self->s.number, otherOwner->s.number);
				}
#endif
			}
		}

		if ( ((selfSaberLevel < FORCE_LEVEL_3 && ((tryDeflectAgain && Q_irand(1, 10) <= 3) || (!tryDeflectAgain && Q_irand(1, 10) <= 7))) || (Q_irand(1, 10) <= 1 && otherSaberLevel >= FORCE_LEVEL_3))
			&& !PM_SaberInBounce(self->client->ps.saberMove)

			&& !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove)
			&& !BG_SaberInSpecial(otherOwner->client->ps.saberMove)
			&& !PM_SaberInBounce(otherOwner->client->ps.saberMove)
			&& !PM_SaberInDeflect(otherOwner->client->ps.saberMove)
			&& !PM_SaberInReflect(otherOwner->client->ps.saberMove)

			&& (otherSaberLevel > FORCE_LEVEL_2 || ( otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] >= 3 && Q_irand(0, otherSaberLevel) )) 
			&& !unblockable
			&& !otherUnblockable
			&& dmg > SABER_NONATTACK_DAMAGE
			&& !didOffense) //don't allow the person we're attacking to do this if we're making an unblockable attack
		{//knockaways can make fast-attacker go into a broken parry anim if the ent is using fast or med. In MP, we also randomly decide this for level 3 attacks.
			//Going to go ahead and let idle damage do simple knockaways. Looks sort of good that way.
			//turn the parry into a knockaway
			if (self->client->ps.saberEntityNum) //make sure he has his saber still
			{
				saberCheckKnockdown_BrokenParry(&g_entities[self->client->ps.saberEntityNum], self, otherOwner);
			}

			if (!PM_SaberInParry(otherOwner->client->ps.saberMove))
			{
				// SABERFIXME: is this some kind of knockaway code or what?
				WP_SaberBlockNonRandom(otherOwner, self, tr.endpos, qfalse);
				otherOwner->client->ps.saberMove = BG_KnockawayForParry( otherOwner->client->ps.saberBlocked );
				otherOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			}
			else
			{
				otherOwner->client->ps.saberMove = G_KnockawayForParry(otherOwner->client->ps.saberMove); //BG_KnockawayForParry( otherOwner->client->ps.saberBlocked );
				otherOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
			}
	
			//make them (me) go into a broken parry
			self->client->ps.saberMove = BG_BrokenParryForAttack( self->client->ps.saberMove, self->client->ps.fd.saberAnimLevel );
			self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

#ifndef FINAL_BUILD
			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i sent client %i into a reflected attack with a knockaway\n", otherOwner->s.number, self->s.number);
			}
#endif

			didDefense = qtrue;
		}
		else if ((selfSaberLevel > FORCE_LEVEL_2 || unblockable) && //if we're doing a special attack, we can send them into a broken parry too (MP only)
				 ( otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] < selfSaberLevel || (otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] == selfSaberLevel && (Q_irand(1, 10) >= otherSaberLevel*1.5 || unblockable)) ) &&
				 PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInParry(self->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
				 !PM_SaberInBounce(self->client->ps.saberMove) &&
				 dmg > SABER_NONATTACK_DAMAGE &&
				 !didOffense &&
				 !otherUnblockable)
		{ //they are in a parry, and we are slamming down on them with a move of equal or greater force than their defense, so send them into a broken parry.. unless they are already in one.
			if (otherOwner->client->ps.saberEntityNum) //make sure he has his saber still
			{
				saberCheckKnockdown_BrokenParry(&g_entities[otherOwner->client->ps.saberEntityNum], otherOwner, self);
			}

#ifndef FINAL_BUILD
			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i sent client %i into a broken parry\n", self->s.number, otherOwner->s.number);
			}
#endif

			otherOwner->client->ps.saberMove = BG_BrokenParryForParry( otherOwner->client->ps.saberMove );
			otherOwner->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;
			otherOwner->client->ps.saberSwingSpeed = 1.0f;

			didDefense = qtrue;
		}
		else if ((selfSaberLevel > FORCE_LEVEL_2) && //if we're doing a special attack, we can send them into a broken parry too (MP only)
				 //( otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] < selfSaberLevel || (otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] == selfSaberLevel && (Q_irand(1, 10) >= otherSaberLevel*3 || unblockable)) ) &&
				 otherSaberLevel >= FORCE_LEVEL_3 &&
				 PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				 !PM_SaberInParry(self->client->ps.saberMove) &&
				 !PM_SaberInBrokenParry(self->client->ps.saberMove) &&
				 !PM_SaberInBounce(self->client->ps.saberMove) &&
				 !PM_SaberInDeflect(self->client->ps.saberMove) &&
				 !PM_SaberInReflect(self->client->ps.saberMove) &&
				 dmg > SABER_NONATTACK_DAMAGE &&
				 !didOffense &&
				 !unblockable)
		{ //they are in a parry, and we are slamming down on them with a move of equal or greater force than their defense, so send them into a broken parry.. unless they are already in one.
#ifndef FINAL_BUILD
			if (g_saberDebugPrint.integer)
			{
				Com_Printf("Client %i bounced off of client %i's saber\n", self->s.number, otherOwner->s.number);
			}
#endif

			if (!tryDeflectAgain)
			{
				if (!WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction))
				{
					tryDeflectAgain = qtrue;
				}
			}

			didOffense = qtrue;
		}
		else if (SaberAttacking(otherOwner) && dmg > SABER_NONATTACK_DAMAGE && !BG_SaberInSpecial(otherOwner->client->ps.saberMove) && !didOffense && !otherUnblockable)
		{ //they were attacking and our saber hit their saber, make them bounce. But if they're in a special attack, leave them.
			if (!PM_SaberInBounce(self->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(self->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&

				!PM_SaberInReflect(self->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				int attackAdv, defendStr = G_PowerLevelForSaberAnim( otherOwner, 0, qtrue ), attackBonus = 0;
				if ( otherOwner->client->ps.torsoAnim == BOTH_A1_SPECIAL
					|| otherOwner->client->ps.torsoAnim == BOTH_A2_SPECIAL
					|| otherOwner->client->ps.torsoAnim == BOTH_A3_SPECIAL )
				{//parry/block/break-parry bonus for single-style kata moves
					defendStr++;
				}
				defendStr += Q_irand(0, otherOwner->client->saber[0].parryBonus );
				if ( otherOwner->client->saber[1].model[0] && !otherOwner->client->ps.saberHolstered )
				{
					defendStr += Q_irand(0, otherOwner->client->saber[1].parryBonus );
				}

#ifndef FINAL_BUILD
				if (g_saberDebugPrint.integer)
				{
					Com_Printf("Client %i and client %i bounced off of each other's sabers\n", self->s.number, otherOwner->s.number);
				}
#endif

				attackBonus = Q_irand(0, self->client->saber[0].breakParryBonus );
				if ( self->client->saber[1].model[0] && !self->client->ps.saberHolstered )
				{
					attackBonus += Q_irand(0, self->client->saber[1].breakParryBonus );
				}
				attackAdv = (attackStr+attackBonus+self->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])-(defendStr+otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE]);
				
				if ( attackAdv > 1 ) 
				{//I won, he should knockaway
					otherOwner->client->ps.saberMove = BG_BrokenParryForAttack( otherOwner->client->ps.saberMove, otherOwner->client->ps.fd.saberAnimLevel );
					otherOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
				}
				else if ( attackAdv > 0 )
				{//I won, he should bounce, I should continue
					otherOwner->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				}
				else if ( attackAdv < 1 )
				{//I lost, I get knocked away
					self->client->ps.saberMove = BG_BrokenParryForAttack( self->client->ps.saberMove, self->client->ps.fd.saberAnimLevel );
					self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;
				}
				else if ( attackAdv < 0 )
				{//I lost, I bounce off
					self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				}
				else
				{//even, both bounce off
					self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
					otherOwner->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
				}

				didOffense = qtrue;
			}
		}
		
		if (!didDefense && dmg <= SABER_NONATTACK_DAMAGE && !otherUnblockable) //with perpoly, it looks pretty weird to have clash flares coming off the guy's face and whatnot
		{
			if (!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				WP_SaberBlockNonRandom(otherOwner, self, tr.endpos, qfalse);
				otherOwner->client->ps.saberEventFlags |= SEF_PARRIED;
			}
		}
		else if (!didDefense && dmg > SABER_NONATTACK_DAMAGE && !otherUnblockable) //if not more than idle damage, don't even bother blocking.
		{ //block
			if (!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
				!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
				!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
				!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
				!PM_SaberInReflect(otherOwner->client->ps.saberMove))
			{
				qboolean crushTheParry = qfalse;

				if (unblockable)
				{ //It's unblockable. So send us into a broken parry immediately.
					crushTheParry = qtrue;
				}

				if (!SaberAttacking(otherOwner))
				{
					/*int otherIdleStr = otherOwner->client->ps.fd.saberAnimLevel;
					if ( otherIdleStr == SS_DUAL
						|| otherIdleStr == SS_STAFF )
					{
						otherIdleStr = SS_SHII_CHO;
					}*/
					//int otherIdleStr = 1; // don't bother with this code yet. this is the stagger code --eez

					WP_SaberBlockNonRandom(otherOwner, self, tr.endpos, qfalse);
					otherOwner->client->ps.saberEventFlags |= SEF_PARRIED;
					self->client->ps.saberEventFlags |= SEF_BLOCKED;

					/*if ( attackStr+self->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > otherIdleStr+otherOwner->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE] )
					{
						crushTheParry = qtrue;
					}
					else
					{*/
					//	tryDeflectAgain = qtrue;
					//}

					// SABERFIXME: change this stagger algo, it's so ugly that it makes me cry --eez
					if( JKG_ShouldDisarm( otherOwner, self ) )
					{
						if (self->client->ps.saberEntityNum)
						{
							if(saberCheckKnockdown_BrokenParry(&g_entities[self->client->ps.saberEntityNum], self, otherOwner))
							{
								return qtrue;
							}
						}
					}
					if(otherOwner->client->ps.blockPoints < 30 && Q_irand(1,2) == 1)	// quick, small change, since stagger barely ever happened anyway --eez
					{
						// stagger him
						crushTheParry = qtrue;
					}
					else
					{
						tryDeflectAgain = qtrue;
					}
				}
				else if (selfSaberLevel > otherSaberLevel ||
					(selfSaberLevel == otherSaberLevel && Q_irand(1, 10) <= 2))
				{ //they are attacking, and we managed to make them break
					//Give them a parry, so we can later break it.
					WP_SaberBlockNonRandom(otherOwner, self, tr.endpos, qfalse);
					crushTheParry = qtrue;

					if (otherOwner->client->ps.saberEntityNum) //make sure he has his saber still
					{
						saberCheckKnockdown_BrokenParry(&g_entities[otherOwner->client->ps.saberEntityNum], otherOwner, self);
					}

#ifndef FINAL_BUILD
					if (g_saberDebugPrint.integer)
					{
						Com_Printf("Client %i forced client %i into a broken parry with a stronger attack\n", self->s.number, otherOwner->s.number);
					}
#endif
				}
				else
				{ //They are attacking, so are we, and obviously they have an attack level higher than or equal to ours
					if (selfSaberLevel == otherSaberLevel)
					{ //equal level, try to bounce off each other's sabers
						if (!didOffense &&
							!PM_SaberInParry(self->client->ps.saberMove) &&
							!PM_SaberInBrokenParry(self->client->ps.saberMove) &&
							!BG_SaberInSpecial(self->client->ps.saberMove) &&
							!PM_SaberInBounce(self->client->ps.saberMove) &&
							!PM_SaberInDeflect(self->client->ps.saberMove) &&
							!PM_SaberInReflect(self->client->ps.saberMove) &&
							!unblockable)
						{
							self->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
							didOffense = qtrue;
						}
						if (!didDefense &&
							!PM_SaberInParry(otherOwner->client->ps.saberMove) &&
							!PM_SaberInBrokenParry(otherOwner->client->ps.saberMove) &&
							!BG_SaberInSpecial(otherOwner->client->ps.saberMove) &&
							!PM_SaberInBounce(otherOwner->client->ps.saberMove) &&
							!PM_SaberInDeflect(otherOwner->client->ps.saberMove) &&
							!PM_SaberInReflect(otherOwner->client->ps.saberMove) &&
							!unblockable)
						{
							otherOwner->client->ps.saberBlocked = BLOCKED_ATK_BOUNCE;
						}
#ifndef FINAL_BUILD
						if (g_saberDebugPrint.integer)
						{
							Com_Printf("Equal attack level bounce/deflection for clients %i and %i\n", self->s.number, otherOwner->s.number);
						}
#endif

						self->client->ps.saberEventFlags |= SEF_DEFLECTED;
						otherOwner->client->ps.saberEventFlags |= SEF_DEFLECTED;
					}
					else if ((level.time - otherOwner->client->lastSaberStorageTime) < 500 && !unblockable) //make sure the stored saber data is updated
					{ //They are higher, this means they can actually smash us into a broken parry
						//Using reflected anims instead now
						self->client->ps.saberMove = BG_BrokenParryForAttack(self->client->ps.saberMove, self->client->ps.fd.saberAnimLevel);
						self->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;

						if (self->client->ps.saberEntityNum) //make sure he has his saber still
						{
							saberCheckKnockdown_BrokenParry(&g_entities[self->client->ps.saberEntityNum], self, otherOwner);
						}

#ifndef FINAL_BUILD
						if (g_saberDebugPrint.integer)
						{
							Com_Printf("Client %i hit client %i's stronger attack, was forced into a broken parry\n", self->s.number, otherOwner->s.number);
						}
#endif

						otherOwner->client->ps.saberEventFlags &= ~SEF_BLOCKED;

						didOffense = qtrue;
					}
				}

				if (crushTheParry && PM_SaberInParry(G_GetParryForBlock(otherOwner->client->ps.saberBlocked)))
				{ //This means that the attack actually hit our saber, and we went to block it.
				  //But, one of the above cases says we actually can't. So we will be smashed into a broken parry instead.
					otherOwner->client->ps.saberMove = BG_BrokenParryForParry( G_GetParryForBlock(otherOwner->client->ps.saberBlocked) );
					otherOwner->client->ps.saberBlocked = BLOCKED_PARRY_BROKEN;
					otherOwner->client->ps.saberSwingSpeed = 1.0f;

					otherOwner->client->ps.saberEventFlags &= ~SEF_PARRIED;
					self->client->ps.saberEventFlags &= ~SEF_BLOCKED;

#ifndef FINAL_BUILD
					if (g_saberDebugPrint.integer)
					{
						Com_Printf("Client %i broke through %i's parry with a special or stronger attack\n", self->s.number, otherOwner->s.number);
					}
#endif
				}
				else if (PM_SaberInParry(G_GetParryForBlock(otherOwner->client->ps.saberBlocked)) && !didOffense && tryDeflectAgain)
				{ //We want to try deflecting again because the other is in the parry and we haven't made any new moves
					int preMove = otherOwner->client->ps.saberMove;

					otherOwner->client->ps.saberMove = G_GetParryForBlock(otherOwner->client->ps.saberBlocked);
					WP_GetSaberDeflectionAngle(self, otherOwner, tr.fraction);
					otherOwner->client->ps.saberMove = preMove;
				}
			}
		}

		self->client->saberAttackWound = level.time + g_saberDmgDelay_Wound.integer;
	}

	return didHit;
}

QINLINE int VectorCompare2( const vec3_t v1, const vec3_t v2 ) {
	if ( v1[0] > v2[0]+0.0001f || v1[0] < v2[0]-0.0001f
		|| v1[1] > v2[1]+0.0001f || v1[1] < v2[1]-0.0001f
		|| v1[2] > v2[2]+0.0001f || v1[2] < v2[2]-0.0001f ) {
		return 0;
	}			
	return 1;
}

#define MAX_SABER_SWING_INC 0.33f
void G_SPSaberDamageTraceLerped( gentity_t *self, int saberNum, int bladeNum, vec3_t baseNew, vec3_t endNew, int clipmask )
{
	vec3_t baseOld, endOld;
	vec3_t mp1, mp2;
	vec3_t md1, md2;

	if ( (level.time-self->client->saber[saberNum].blade[bladeNum].trail.lastTime) > 100 )
	{//no valid last pos, use current
		VectorCopy(baseNew, baseOld);
		VectorCopy(endNew, endOld);
	}
	else
	{//trace from last pos
		VectorCopy( self->client->saber[saberNum].blade[bladeNum].trail.base, baseOld );
		VectorCopy( self->client->saber[saberNum].blade[bladeNum].trail.tip, endOld );
	}

	VectorCopy( baseOld, mp1 );
	VectorCopy( baseNew, mp2 );
	VectorSubtract( endOld, baseOld, md1 );
	VectorNormalize( md1 );
	VectorSubtract( endNew, baseNew, md2 );
	VectorNormalize( md2 );

	saberHitWall = qfalse;
	saberHitSaber = qfalse;
	saberHitFraction = 1.0f;
	if ( VectorCompare2( baseOld, baseNew ) && VectorCompare2( endOld, endNew ) )
	{//no diff
		CheckSaberDamage( self, saberNum, bladeNum, baseNew, endNew, qfalse, clipmask, qfalse );
	}
	else
	{//saber moved, lerp
		float step = 8, stepsize = 8;//aveLength, 
		vec3_t	ma1, ma2, md2ang, curBase1, curBase2;
		int	xx;
		vec3_t curMD1, curMD2;//, mdDiff, dirDiff;
		float dirInc, curDirFrac;
		vec3_t baseDiff, bladePointOld, bladePointNew;
		qboolean extrapolate = qtrue;

		//do the trace at the base first
		VectorCopy( baseOld, bladePointOld );
		VectorCopy( baseNew, bladePointNew );
		CheckSaberDamage( self, saberNum, bladeNum, bladePointOld, bladePointNew, qfalse, clipmask, qtrue );

		//if hit a saber, shorten rest of traces to match
		if ( saberHitFraction < 1.0f )
		{
			//adjust muzzleDir...
			vec3_t ma1, ma2;
			vectoangles( md1, ma1 );
			vectoangles( md2, ma2 );
			for ( xx = 0; xx < 3; xx++ )
			{
				md2ang[xx] = LerpAngle( ma1[xx], ma2[xx], saberHitFraction );
			}
			AngleVectors( md2ang, md2, NULL, NULL );
			//shorten the base pos
			VectorSubtract( mp2, mp1, baseDiff );
			VectorMA( mp1, saberHitFraction, baseDiff, baseNew );
			VectorMA( baseNew, self->client->saber[saberNum].blade[bladeNum].lengthMax, md2, endNew );
		}

		//If the angle diff in the blade is high, need to do it in chunks of 33 to avoid flattening of the arc
		if ( BG_SaberInAttack( self->client->ps.saberMove ) 
			|| BG_SaberInSpecialAttack( self->client->ps.torsoAnim ) 
			|| BG_SpinningSaberAnim( self->client->ps.torsoAnim ) 
			|| BG_InSpecialJump( self->client->ps.torsoAnim ) )
			//|| (g_timescale->value<1.0f&&BG_SaberInTransitionAny( ent->client->ps.saberMove )) )
		{
			curDirFrac = DotProduct( md1, md2 );
		}
		else
		{
			curDirFrac = 1.0f;
		}
		//NOTE: if saber spun at least 180 degrees since last damage trace, this is not reliable...!
		if ( fabs(curDirFrac) < 1.0f - MAX_SABER_SWING_INC )
		{//the saber blade spun more than 33 degrees since the last damage trace
			curDirFrac = dirInc = 1.0f/((1.0f - curDirFrac)/MAX_SABER_SWING_INC);
		}
		else
		{
			curDirFrac = 1.0f;
			dirInc = 0.0f;
		}
		//qboolean hit_saber = qfalse;

		vectoangles( md1, ma1 );
		vectoangles( md2, ma2 );

		//VectorSubtract( md2, md1, mdDiff );
		VectorCopy( md1, curMD2 );
		VectorCopy( baseOld, curBase2 );

		while ( 1 )
		{
			VectorCopy( curMD2, curMD1 );
			VectorCopy( curBase2, curBase1 );
			if ( curDirFrac >= 1.0f )
			{
				VectorCopy( md2, curMD2 );
				VectorCopy( baseNew, curBase2 );
			}
			else
			{
				for ( xx = 0; xx < 3; xx++ )
				{
					md2ang[xx] = LerpAngle( ma1[xx], ma2[xx], curDirFrac );
				}
				AngleVectors( md2ang, curMD2, NULL, NULL );
				//VectorMA( md1, curDirFrac, mdDiff, curMD2 );
				VectorSubtract( baseNew, baseOld, baseDiff );
				VectorMA( baseOld, curDirFrac, baseDiff, curBase2 );
			}
			// Move up the blade in intervals of stepsize
			for ( step = stepsize; step <= self->client->saber[saberNum].blade[bladeNum].lengthMax /*&& step < self->client->saber[saberNum].blade[bladeNum].lengthOld*/; step += stepsize )
			{
				VectorMA( curBase1, step, curMD1, bladePointOld );
				VectorMA( curBase2, step, curMD2, bladePointNew );
				
				if ( step+stepsize >= self->client->saber[saberNum].blade[bladeNum].lengthMax )
				{
					extrapolate = qfalse;
				}
				//do the damage trace
				CheckSaberDamage( self, saberNum, bladeNum, bladePointOld, bladePointNew, qfalse, clipmask, extrapolate );
				/*
				if ( WP_SaberDamageForTrace( ent->s.number, bladePointOld, bladePointNew, baseDamage, curMD2, 
					qfalse, entPowerLevel, ent->client->ps.saber[saberNum].type, qtrue,
					saberNum, bladeNum ) )
				{
					hit_wall = qtrue;
				}
				*/

				//if hit a saber, shorten rest of traces to match
				if ( saberHitFraction < 1.0f )
				{
					vec3_t curMA1, curMA2;
					//adjust muzzle endpoint
					VectorSubtract( mp2, mp1, baseDiff );
					VectorMA( mp1, saberHitFraction, baseDiff, baseNew );
					VectorMA( baseNew, self->client->saber[saberNum].blade[bladeNum].lengthMax, curMD2, endNew );
					//adjust muzzleDir...
					vectoangles( curMD1, curMA1 );
					vectoangles( curMD2, curMA2 );
					for ( xx = 0; xx < 3; xx++ )
					{
						md2ang[xx] = LerpAngle( curMA1[xx], curMA2[xx], saberHitFraction );
					}
					AngleVectors( md2ang, curMD2, NULL, NULL );
					saberHitSaber = qtrue;
				}
				if (saberHitWall)
				{
					break;
				}
			}
			if ( saberHitWall || saberHitSaber )
			{
				break;
			}
			if ( curDirFrac >= 1.0f )
			{
				break;
			}
			else
			{
				curDirFrac += dirInc;
				if ( curDirFrac >= 1.0f )
				{
					curDirFrac = 1.0f;
				}
			}
		}

		//do the trace at the end last
		//Special check- adjust for length of blade not being a multiple of 12
		/*
		aveLength = (ent->client->ps.saber[saberNum].blade[bladeNum].lengthOld + ent->client->ps.saber[saberNum].blade[bladeNum].length)/2;
		if ( step > aveLength )
		{//less dmg if the last interval was not stepsize
			tipDmgMod = (stepsize-(step-aveLength))/stepsize;
		}
		//NOTE: since this is the tip, we do not extrapolate the extra 16
		if ( WP_SaberDamageForTrace( ent->s.number, endOld, endNew, tipDmgMod*baseDamage, md2, 
			qfalse, entPowerLevel, ent->client->ps.saber[saberNum].type, qfalse,
			saberNum, bladeNum ) )
		{
			hit_wall = qtrue;
		}
		*/
	}
}

qboolean BG_SaberInTransitionAny( int move );

qboolean WP_ForcePowerUsable( gentity_t *self, forcePowers_t forcePower );
qboolean InFOV3( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
qboolean NPC_Humanoid_WaitingAmbush( gentity_t *self );
void NPC_Humanoid_Ambush( gentity_t *self );
evasionType_t NPC_Humanoid_SaberBlockGo( gentity_t *self, usercmd_t *cmd, vec3_t pHitloc, vec3_t phitDir, gentity_t *incoming, float dist );
void NPC_SetLookTarget( gentity_t *self, int entNum, int clearTime );
// eez -- this function handles all of the projectile stuff involving sabers
void WP_SaberStartMissileBlockCheck( gentity_t *self, usercmd_t *ucmd  )
{
	float		dist;
	gentity_t	*ent, *incoming = NULL;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	int			i, e;
	float		closestDist, radius = 256;
	vec3_t		forward, dir, missile_dir, fwdangles = {0};
	trace_t		trace;
	vec3_t		traceTo, entDir;
	float		dot1, dot2;
	float		lookTDist = -1;
	gentity_t	*lookT = NULL;
	qboolean	doFullRoutine = qtrue;

	//keep this updated even if we don't get below
	if ( !(self->client->ps.eFlags2&EF2_HELD_BY_MONSTER) )
	{//lookTarget is set by and to the monster that's holding you, no other operations can change that
		self->client->ps.hasLookTarget = qfalse;
	}

	if ( self->client->ps.weapon != WP_SABER && self->client->NPC_class != CLASS_BOBAFETT )
	{
		doFullRoutine = qfalse;
	}
	else if ( self->client->ps.saberInFlight )
	{
		doFullRoutine = qfalse;
	}
	else if ( self->client->ps.fd.forcePowersActive&(1<<FP_LIGHTNING) )
	{//can't block while zapping
		doFullRoutine = qfalse;
	}
	else if ( self->client->ps.fd.forcePowersActive&(1<<FP_DRAIN) )
	{//can't block while draining
		doFullRoutine = qfalse;
	}
	else if ( self->client->ps.fd.forcePowersActive&(1<<FP_PUSH) )
	{//can't block while shoving
		doFullRoutine = qfalse;
	}
	else if ( self->client->ps.fd.forcePowersActive&(1<<FP_GRIP) )
	{//can't block while gripping (FIXME: or should it break the grip?  Pain should break the grip, I think...)
		doFullRoutine = qfalse;
	}
	
	if (self->client->ps.weaponTime > 0)
	{ //don't autoblock while busy with stuff
		return;
	}

	if ( (self->client->saber[0].saberFlags&SFL_NOT_ACTIVE_BLOCKING) )
	{//can't actively block with this saber type
		return;
	}

	if ( self->health <= 0 )
	{//dead don't try to block (NOTE: actual deflection happens in missile code)
		return;
	}
	if ( PM_InKnockDown( &self->client->ps ) )
	{//can't block when knocked down
		return;
	}

	if ( BG_SabersOff( &self->client->ps ) && self->client->NPC_class != CLASS_BOBAFETT )
	{
		if ( self->s.eType != ET_NPC )
		{//player doesn't auto-activate
			doFullRoutine = qfalse;
		}
	}

	if ( self->s.eType == ET_PLAYER )
	{//don't do this if already attacking!
		if ( ucmd->buttons & BUTTON_ATTACK 
			|| BG_SaberInAttack( self->client->ps.saberMove )
			|| BG_SaberInSpecialAttack( self->client->ps.torsoAnim )
			|| BG_SaberInTransitionAny( self->client->ps.saberMove ))
		{
			doFullRoutine = qfalse;
		}

		// don't block if we aren't holding the block button --eez
		if( !(self->client->ps.saberActionFlags & (1 << SAF_BLOCKING)) )
		{
			doFullRoutine = qfalse;
		}
	}

	if ( self->client->ps.fd.forcePowerDebounce[FP_SABER_DEFENSE] > level.time )
	{//can't block while gripping (FIXME: or should it break the grip?  Pain should break the grip, I think...)
		doFullRoutine = qfalse;
	}

	fwdangles[1] = self->client->ps.viewangles[1];
	AngleVectors( fwdangles, forward, NULL, NULL );

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = self->r.currentOrigin[i] - radius;
		maxs[i] = self->r.currentOrigin[i] + radius;
	}

	numListedEntities = trap->EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	closestDist = radius;

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = &g_entities[entityList[ e ]];

		if (ent == self)
			continue;

		//as long as we're here I'm going to get a looktarget too, I guess. -rww
		if (self->s.eType == ET_PLAYER &&
			ent->client &&
			(ent->s.eType == ET_NPC || ent->s.eType == ET_PLAYER) &&
			!OnSameTeam(ent, self) &&
			ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
			!(ent->client->ps.pm_flags & PMF_FOLLOW) &&
			ent->health > 0)
		{ //seems like a valid enemy to look at.
			vec3_t vecSub;
			float vecLen;

			VectorSubtract(self->client->ps.origin, ent->client->ps.origin, vecSub);
			vecLen = VectorLength(vecSub);

			if (lookTDist == -1 || vecLen < lookTDist)
			{
				trace_t tr;
				vec3_t myEyes;

				VectorCopy(self->client->ps.origin, myEyes);
				myEyes[2] += self->client->ps.viewheight;

				trap->Trace(&tr, myEyes, NULL, NULL, ent->client->ps.origin, self->s.number, MASK_PLAYERSOLID, 0, 0, 0);

				if (tr.fraction == 1.0f || tr.entityNum == ent->s.number)
				{ //we have a clear line of sight to him, so it's all good.
					lookT = ent;
					lookTDist = vecLen;
				}
			}
		}

		if (!doFullRoutine)
		{ //don't care about the rest then
			continue;
		}

		if (ent->r.ownerNum == self->s.number)
			continue;
		if ( !(ent->inuse) )
			continue;
		if ( ent->s.eType != ET_MISSILE && !(ent->s.eFlags&EF_MISSILE_STICK) )
		{//not a normal projectile
			gentity_t *pOwner;

			if (ent->r.ownerNum < 0 || ent->r.ownerNum >= ENTITYNUM_WORLD)
			{ //not going to be a client then.
				continue;
			}
				
			pOwner = &g_entities[ent->r.ownerNum];

			if (!pOwner->inuse || !pOwner->client)
			{
				continue; //not valid cl owner
			}

			if (!pOwner->client->ps.saberEntityNum ||
				!pOwner->client->ps.saberInFlight ||
				pOwner->client->ps.saberEntityNum != ent->s.number)
			{ //the saber is knocked away and/or not flying actively, or this ent is not the cl's saber ent at all
				continue;
			}

			//If we get here then it's ok to be treated as a thrown saber, I guess.
		}
		else
		{
			if ( ent->s.pos.trType == TR_STATIONARY && self->s.eType == ET_PLAYER )
			{//nothing you can do with a stationary missile if you're the player
				continue;
			}
		}

		//see if they're in front of me
		VectorSubtract( ent->r.currentOrigin, self->r.currentOrigin, dir );
		dist = VectorNormalize( dir );
		//FIXME: handle detpacks, proximity mines and tripmines
		if ( ent->s.weapon == WP_THERMAL )
		{//thermal detonator!
			if ( self->NPC && dist < ent->splashRadius )
			{
				if ( dist < ent->splashRadius && 
					ent->nextthink < level.time + 600 && 
					ent->count && 
					self->client->ps.groundEntityNum != ENTITYNUM_NONE && 
						(ent->s.pos.trType == TR_STATIONARY||
						ent->s.pos.trType == TR_INTERPOLATE||
						(dot1 = DotProduct( dir, forward )) < SABER_REFLECT_MISSILE_CONE||
						!WP_ForcePowerUsable( self, FP_PUSH )) )
				{//TD is close enough to hurt me, I'm on the ground and the thing is at rest or behind me and about to blow up, or I don't have force-push so force-jump!
					//FIXME: sometimes this might make me just jump into it...?
					self->client->ps.fd.forceJumpCharge = 480;
				}
				else if ( self->client->NPC_class != CLASS_BOBAFETT )
				{//FIXME: check forcePushRadius[NPC->client->ns.forcePowerLevel[FP_PUSH]]
					ForceThrow( self, qfalse );
				}
			}
			continue;
		}
		else if ( ent->splashDamage && ent->splashRadius )
		{//exploding missile
			//FIXME: handle tripmines and detpacks somehow... 
			//			maybe do a force-gesture that makes them explode?  
			//			But what if we're within it's splashradius?
			if ( self->s.eType == ET_PLAYER )
			{//players don't auto-handle these at all
				continue;
			}
			else 
			{
				//if ( ent->s.pos.trType == TR_STATIONARY && (ent->s.eFlags&EF_MISSILE_STICK) 
				//	&& 	self->client->NPC_class != CLASS_BOBAFETT )
				if (0) //Maybe handle this later?
				{//a placed explosive like a tripmine or detpack
					if ( InFOV3( ent->r.currentOrigin, self->client->renderInfo.eyePoint, self->client->ps.viewangles, 90, 90 ) )
					{//in front of me
						if ( G_ClearLOS4( self, ent ) )
						{//can see it
							vec3_t throwDir;
							//make the gesture
							ForceThrow( self, qfalse );
							//take it off the wall and toss it
							ent->s.pos.trType = TR_GRAVITY;
							ent->s.eType = ET_MISSILE;
							ent->s.eFlags &= ~EF_MISSILE_STICK;
							ent->flags |= FL_BOUNCE_HALF;
							AngleVectors( ent->r.currentAngles, throwDir, NULL, NULL );
							VectorMA( ent->r.currentOrigin, ent->r.maxs[0]+4, throwDir, ent->r.currentOrigin );
							VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
							VectorScale( throwDir, 300, ent->s.pos.trDelta );
							ent->s.pos.trDelta[2] += 150;
							VectorMA( ent->s.pos.trDelta, 800, dir, ent->s.pos.trDelta );
							ent->s.pos.trTime = level.time;		// move a bit on the very first frame
							VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
							ent->r.ownerNum = self->s.number;
							// make it explode, but with less damage
							ent->splashDamage /= 3;
							ent->splashRadius /= 3;
							//ent->think = WP_Explode;
							ent->nextthink = level.time + Q_irand( 500, 3000 );
						}
					}
				}
				else if ( dist < ent->splashRadius && 
				self->client->ps.groundEntityNum != ENTITYNUM_NONE && 
					(DotProduct( dir, forward ) < SABER_REFLECT_MISSILE_CONE||
					!WP_ForcePowerUsable( self, FP_PUSH )) )
				{//NPCs try to evade it
					self->client->ps.fd.forceJumpCharge = 480;
				}
				else if ( self->client->NPC_class != CLASS_BOBAFETT )
				{//else, try to force-throw it away
					//FIXME: check forcePushRadius[NPC->client->ns.forcePowerLevel[FP_PUSH]]
					ForceThrow( self, qfalse );
				}
			}
			//otherwise, can't block it, so we're screwed
			continue;
		}

		if ( ent->s.weapon != WP_SABER )
		{//only block shots coming from behind
			if ( (dot1 = DotProduct( dir, forward )) < SABER_REFLECT_MISSILE_CONE )
				continue;
		}
		else if ( self->s.eType == ET_PLAYER )
		{//player never auto-blocks thrown sabers
			continue;
		}//NPCs always try to block sabers coming from behind!

		//see if they're heading towards me
		VectorCopy( ent->s.pos.trDelta, missile_dir );
		VectorNormalize( missile_dir );
		if ( (dot2 = DotProduct( dir, missile_dir )) > 0 )
			continue;

		//FIXME: must have a clear trace to me, too...
		if ( dist < closestDist )
		{
			VectorCopy( self->r.currentOrigin, traceTo );
			traceTo[2] = self->r.absmax[2] - 4;
			trap->Trace( &trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, traceTo, ent->s.number, ent->clipmask , 0, 0, 0);
			if ( trace.allsolid || trace.startsolid || (trace.fraction < 1.0f && trace.entityNum != self->s.number && trace.entityNum != self->client->ps.saberEntityNum) )
			{//okay, try one more check
				VectorNormalize2( ent->s.pos.trDelta, entDir );
				VectorMA( ent->r.currentOrigin, radius, entDir, traceTo );
				trap->Trace( &trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, traceTo, ent->s.number, ent->clipmask , 0, 0, 0);
				if ( trace.allsolid || trace.startsolid || (trace.fraction < 1.0f && trace.entityNum != self->s.number && trace.entityNum != self->client->ps.saberEntityNum) )
				{//can't hit me, ignore it
					continue;
				}
			}
			if ( self->s.eType == ET_NPC )
			{//An NPC
				if ( self->NPC && !self->enemy && ent->r.ownerNum != ENTITYNUM_NONE )
				{
					gentity_t *owner = &g_entities[ent->r.ownerNum];
					if ( owner->health >= 0 && (!owner->client || owner->client->playerTeam != self->client->playerTeam) )
					{
						G_SetEnemy( self, owner );
					}
				}
			}
			//FIXME: if NPC, predict the intersection between my current velocity/path and the missile's, see if it intersects my bounding box (+/-saberLength?), don't try to deflect unless it does?
			closestDist = dist;
			incoming = ent;
		}
	}

	if (self->s.eType == ET_NPC && self->localAnimIndex < NUM_RESERVED_ANIMSETS)
	{ //humanoid NPCs don't set angles based on server angles for looking, unlike other NPCs
		if (self->client && self->client->renderInfo.lookTarget < ENTITYNUM_WORLD)
		{
			lookT = &g_entities[self->client->renderInfo.lookTarget];
		}
	}

	if (lookT)
	{ //we got a looktarget at some point so we'll assign it then.
		if ( !(self->client->ps.eFlags2&EF2_HELD_BY_MONSTER) )
		{//lookTarget is set by and to the monster that's holding you, no other operations can change that
			self->client->ps.hasLookTarget = qtrue;
			self->client->ps.lookTarget = lookT->s.number;
		}
	}

	if (!doFullRoutine)
	{ //then we're done now
		return;
	}

	if ( incoming )
	{
		if ( self->NPC /*&& !G_ControlledByPlayer( self )*/ )
		{
			if ( NPC_Humanoid_WaitingAmbush( self ) )
			{
				NPC_Humanoid_Ambush( self );
			}
			if ( NPC_Humanoid_SaberBlockGo( self, &self->NPC->last_ucmd, NULL, NULL, incoming, 0.0f ) != EVASION_NONE )
			{//make sure to turn on your saber if it's not on
				if ( self->client->NPC_class != CLASS_BOBAFETT )
				{
					//self->client->ps.SaberActivate();
					WP_ActivateSaber(self);
				}
			}
		}
		else//player
		{
			gentity_t *owner = &g_entities[incoming->r.ownerNum];

			WP_SaberBlockNonRandom( self, NULL, incoming->r.currentOrigin, qtrue );
			if ( owner && owner->client && (!self->enemy || self->enemy->s.weapon != WP_SABER) )//keep enemy jedi over shooters
			{
				self->enemy = owner;
				//NPC_SetLookTarget( self, owner->s.number, level.time+1000 );
				//player looktargetting done differently
			}
		}
	}
}

#define MIN_SABER_SLICE_DISTANCE 50

#define MIN_SABER_SLICE_RETURN_DISTANCE 30

#define SABER_THROWN_HIT_DAMAGE 30
#define SABER_THROWN_RETURN_HIT_DAMAGE 5

void thrownSaberTouch (gentity_t *saberent, gentity_t *other, trace_t *trace);

static QINLINE qboolean CheckThrownSaberDamaged(gentity_t *saberent, gentity_t *saberOwner, gentity_t *ent, int dist, int returning, qboolean noDCheck)
{
	vec3_t vecsub;
	float veclen;
	gentity_t *te;

	if (saberOwner && saberOwner->client && saberOwner->client->saberAttackWound > level.time)
	{
		return qfalse;
	}

	if (ent && ent->client && ent->inuse && ent->s.number != saberOwner->s.number &&
		ent->health > 0 && ent->takedamage &&
		trap->InPVS(ent->client->ps.origin, saberent->r.currentOrigin) &&
		ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
		(ent->client->pers.connected || ent->s.eType == ET_NPC))
	{ //hit a client
		if (ent->inuse && ent->client &&
			ent->client->ps.duelInProgress &&
			ent->client->ps.duelIndex != saberOwner->s.number)
		{
			return qfalse;
		}

		if (ent->inuse && ent->client &&
			saberOwner->client->ps.duelInProgress &&
			saberOwner->client->ps.duelIndex != ent->s.number)
		{
			return qfalse;
		}

		VectorSubtract(saberent->r.currentOrigin, ent->client->ps.origin, vecsub);
		veclen = VectorLength(vecsub);

		if (veclen < dist)
		{ //within range
			trace_t tr;

			trap->Trace(&tr, saberent->r.currentOrigin, NULL, NULL, ent->client->ps.origin, saberent->s.number, MASK_SHOT, 0, 0, 0);

			if (tr.fraction == 1 || tr.entityNum == ent->s.number)
			{ //Slice them
				if (WP_SaberCanBlock(ent, tr.endpos, 0, MOD_SABER, qfalse, 999))
				{ //they blocked it
					WP_SaberBlockNonRandom(ent, saberOwner, tr.endpos, qfalse);

					te = G_TempEntity( tr.endpos, EV_SABER_BLOCK );
					VectorCopy(tr.endpos, te->s.origin);
					VectorCopy(tr.plane.normal, te->s.angles);
					if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
					{
						te->s.angles[1] = 1;
					}
					te->s.eventParm = 1;
					te->s.weapon = 0;//saberNum
					te->s.legsAnim = 0;//bladeNum

					if (saberCheckKnockdown_Thrown(saberent, saberOwner, &g_entities[tr.entityNum]))
					{ //it was knocked out of the air
						return qfalse;
					}

					if (!returning)
					{ //return to owner if blocked
						thrownSaberTouch(saberent, saberent, NULL);
					}

					saberOwner->client->saberAttackWound = level.time + 500;
					return qfalse;
				}
				else
				{ //a good hit
					vec3_t dir;
					int dflags = 0;

					VectorSubtract(tr.endpos, saberent->r.currentOrigin, dir);
					VectorNormalize(dir);

					if (!dir[0] && !dir[1] && !dir[2])
					{
						dir[1] = 1;
					}

					if ( (saberOwner->client->saber[0].saberFlags2&SFL2_NO_DISMEMBERMENT) )
					{
						dflags |= DAMAGE_NO_DISMEMBER;
					}

					G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, saberent->damage, dflags, MOD_SABER);

					te = G_TempEntity( tr.endpos, EV_SABER_HIT );
					te->s.otherEntityNum = ent->s.number;
					te->s.otherEntityNum2 = saberOwner->s.number;
					te->s.weapon = 0;//saberNum
					te->s.legsAnim = 0;//bladeNum
					VectorCopy(tr.endpos, te->s.origin);
					VectorCopy(tr.plane.normal, te->s.angles);
					if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
					{
						te->s.angles[1] = 1;
					}

					te->s.eventParm = 1;

					if (!returning)
					{ //return to owner if blocked
						thrownSaberTouch(saberent, saberent, NULL);
					}
				}

				saberOwner->client->saberAttackWound = level.time + 500;
			}
		}
	}
	else if (ent && !ent->client && ent->inuse && ent->takedamage && ent->health > 0 && ent->s.number != saberOwner->s.number &&
		ent->s.number != saberent->s.number && (noDCheck ||trap->InPVS(ent->r.currentOrigin, saberent->r.currentOrigin)))
	{ //hit a non-client

		if (noDCheck)
		{
			veclen = 0;
		}
		else
		{
			VectorSubtract(saberent->r.currentOrigin, ent->r.currentOrigin, vecsub);
			veclen = VectorLength(vecsub);
		}

		if (veclen < dist)
		{
			trace_t tr;
			vec3_t entOrigin;

			if (ent->s.eType == ET_MOVER)
			{
				VectorSubtract( ent->r.absmax, ent->r.absmin, entOrigin );
				VectorMA( ent->r.absmin, 0.5, entOrigin, entOrigin );
				VectorAdd( ent->r.absmin, ent->r.absmax, entOrigin );
				VectorScale( entOrigin, 0.5f, entOrigin );
			}
			else
			{
				VectorCopy(ent->r.currentOrigin, entOrigin);
			}

			trap->Trace(&tr, saberent->r.currentOrigin, NULL, NULL, entOrigin, saberent->s.number, MASK_SHOT, 0, 0, 0);

			if (tr.fraction == 1 || tr.entityNum == ent->s.number)
			{
				vec3_t dir;
				int dflags = 0;

				VectorSubtract(tr.endpos, entOrigin, dir);
				VectorNormalize(dir);

				if ( (saberOwner->client->saber[0].saberFlags2&SFL2_NO_DISMEMBERMENT) )
				{
					dflags |= DAMAGE_NO_DISMEMBER;
				}

				if (ent->s.eType == ET_NPC)
				{ //an animent
					G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, 40, dflags, MOD_SABER);
				}
				else
				{
					G_Damage(ent, saberOwner, saberOwner, dir, tr.endpos, 5, dflags, MOD_SABER);
				}

				te = G_TempEntity( tr.endpos, EV_SABER_HIT );
				te->s.otherEntityNum = ENTITYNUM_NONE; //don't do this for throw damage
				//te->s.otherEntityNum = ent->s.number;
				te->s.otherEntityNum2 = saberOwner->s.number;//actually, do send this, though - for the overridden per-saber hit effects/sounds
				te->s.weapon = 0;//saberNum
				te->s.legsAnim = 0;//bladeNum
				VectorCopy(tr.endpos, te->s.origin);
				VectorCopy(tr.plane.normal, te->s.angles);
				if (!te->s.angles[0] && !te->s.angles[1] && !te->s.angles[2])
				{
					te->s.angles[1] = 1;
				}

				if ( ent->s.eType == ET_MOVER )
				{
					if ( saberOwner
						&& saberOwner->client
						&& (saberOwner->client->saber[0].saberFlags2&SFL2_NO_CLASH_FLARE) ) 
					{//don't do clash flare - NOTE: assumes same is true for both sabers if using dual sabers!
						G_FreeEntity( te );//kind of a waste, but...
					}
					else
					{
						//I suppose I could tie this into the saberblock event, but I'm tired of adding flags to that thing.
						gentity_t *teS = G_TempEntity( te->s.origin, EV_SABER_CLASHFLARE );
						VectorCopy(te->s.origin, teS->s.origin);

						te->s.eventParm = 0;
					}
				}
				else
				{
					te->s.eventParm = 1;
				}

				if (!returning)
				{ //return to owner if blocked
					thrownSaberTouch(saberent, saberent, NULL);
				}

				saberOwner->client->saberAttackWound = level.time + 500;
			}
		}
	}

	return qtrue;
}

static QINLINE void saberCheckRadiusDamage(gentity_t *saberent, int returning)
{ //we're going to cheat and damage players within the saber's radius, just for the sake of doing things more "efficiently" (and because the saber entity has no server g2 instance)
	int i = 0;
	int dist = 0;
	gentity_t *ent;
	gentity_t *saberOwner = &g_entities[saberent->r.ownerNum];

	if (returning && returning != 2)
	{
		dist = MIN_SABER_SLICE_RETURN_DISTANCE;
	}
	else
	{
		dist = MIN_SABER_SLICE_DISTANCE;
	}

	if (!saberOwner || !saberOwner->client)
	{
		return;
	}

	if (saberOwner->client->saberAttackWound > level.time)
	{
		return;
	}

	while (i < level.num_entities)
	{
		ent = &g_entities[i];

		CheckThrownSaberDamaged(saberent, saberOwner, ent, dist, returning, qfalse);

		i++;
	}
}

#define THROWN_SABER_COMP

static QINLINE void saberMoveBack( gentity_t *ent, qboolean goingBack ) 
{
	vec3_t		origin, oldOrg;

	ent->s.pos.trType = TR_LINEAR;

	VectorCopy( ent->r.currentOrigin, oldOrg );
	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	//Get current angles?
	BG_EvaluateTrajectory( &ent->s.apos, level.time, ent->r.currentAngles );

	//compensation test code..
#ifdef THROWN_SABER_COMP
	if (!goingBack && ent->s.pos.trType != TR_GRAVITY)
	{ //acts as a fallback in case touch code fails, keeps saber from going through things between predictions
		float originalLength = 0;
		int iCompensationLength = 32;
		trace_t tr;
		vec3_t mins, maxs;
		vec3_t calcComp, compensatedOrigin;
		VectorSet( mins, -24.0f, -24.0f, -8.0f );
		VectorSet( maxs, 24.0f, 24.0f, 8.0f );

		VectorSubtract(origin, oldOrg, calcComp);
		originalLength = VectorLength(calcComp);

		VectorNormalize(calcComp);

		compensatedOrigin[0] = oldOrg[0] + calcComp[0]*(originalLength+iCompensationLength);		
		compensatedOrigin[1] = oldOrg[1] + calcComp[1]*(originalLength+iCompensationLength);
		compensatedOrigin[2] = oldOrg[2] + calcComp[2]*(originalLength+iCompensationLength);

		trap->Trace(&tr, oldOrg, mins, maxs, compensatedOrigin, ent->r.ownerNum, MASK_PLAYERSOLID, 0, 0, 0);

		if ((tr.fraction != 1 || tr.startsolid || tr.allsolid) && tr.entityNum != ent->r.ownerNum && !(g_entities[tr.entityNum].r.contents & CONTENTS_LIGHTSABER))
		{
			VectorClear(ent->s.pos.trDelta);

			//Unfortunately doing this would defeat the purpose of the compensation. We will have to settle for a jerk on the client.
			//VectorCopy( origin, ent->r.currentOrigin );

			//we'll skip the dist check, since we don't really care about that (we just hit it physically)
			CheckThrownSaberDamaged(ent, &g_entities[ent->r.ownerNum], &g_entities[tr.entityNum], 256, 0, qtrue);

			if (ent->s.pos.trType == TR_GRAVITY)
			{ //got blocked and knocked away in the damage func
				return;
			}

			tr.startsolid = 0;
			if (tr.entityNum == ENTITYNUM_NONE)
			{ //eh, this is a filthy lie. (obviously it had to hit something or it wouldn't be in here, so we'll say it hit the world)
				tr.entityNum = ENTITYNUM_WORLD;
			}
			thrownSaberTouch(ent, &g_entities[tr.entityNum], &tr);
			return;
		}
	}
#endif

	VectorCopy( origin, ent->r.currentOrigin );
}

void SaberBounceSound( gentity_t *self, gentity_t *other, trace_t *trace )
{
	VectorCopy(self->r.currentAngles, self->s.apos.trBase);
	self->s.apos.trBase[PITCH] = 90;
}

void DeadSaberThink(gentity_t *saberent)
{
	if (saberent->speed < level.time)
	{
		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	G_RunObject(saberent);
}

void MakeDeadSaber(gentity_t *ent)
{	//spawn a "dead" saber entity here so it looks like the saber fell out of the air.
	//This entity will remove itself after a very short time period.
	vec3_t startorg;
	vec3_t startang;
	gentity_t *saberent;
	gentity_t *owner = NULL;

	saberent = G_Spawn();

	VectorCopy(ent->r.currentOrigin, startorg);
	VectorCopy(ent->r.currentAngles, startang);

	saberent->classname = "deadsaber";
			
	saberent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	saberent->r.ownerNum = ent->s.number;

	saberent->clipmask = MASK_PLAYERSOLID;
	saberent->r.contents = CONTENTS_TRIGGER;//0;

	VectorSet( saberent->r.mins, -3.0f, -3.0f, -1.5f );
	VectorSet( saberent->r.maxs, 3.0f, 3.0f, 1.5f );

	saberent->touch = SaberBounceSound;

	saberent->think = DeadSaberThink;
	saberent->nextthink = level.time;

	VectorCopy(startorg, saberent->s.pos.trBase);
	VectorCopy(startang, saberent->s.apos.trBase);

	VectorCopy(startorg, saberent->s.origin);
	VectorCopy(startang, saberent->s.angles);

	VectorCopy(startorg, saberent->r.currentOrigin);
	VectorCopy(startang, saberent->r.currentAngles);

	saberent->s.apos.trType = TR_GRAVITY;
	saberent->s.apos.trDelta[0] = Q_irand(200, 800);
	saberent->s.apos.trDelta[1] = Q_irand(200, 800);
	saberent->s.apos.trDelta[2] = Q_irand(200, 800);
	saberent->s.apos.trTime = level.time-50;

	saberent->s.pos.trType = TR_GRAVITY;
	saberent->s.pos.trTime = level.time-50;
	saberent->flags = FL_BOUNCE_HALF;
	if (ent->r.ownerNum >= 0 && ent->r.ownerNum < ENTITYNUM_WORLD)
	{
		owner = &g_entities[ent->r.ownerNum];

		if (owner->inuse && owner->client &&
			owner->client->saber[0].model[0])
		{
			WP_SaberAddG2Model( saberent, owner->client->saber[0].model, owner->client->saber[0].skin );
		}
		else
		{
			//WP_SaberAddG2Model( saberent, NULL, 0 );
			//argh!!!!
			G_FreeEntity(saberent);
			return;
		}
	}

	saberent->s.modelGhoul2 = 1;
	saberent->s.g2radius = 20;

	saberent->s.eType = ET_MISSILE;
	saberent->s.weapon = WP_SABER;

	saberent->speed = level.time + 4000;

	saberent->bounceCount = 12;

	//fall off in the direction the real saber was headed
	VectorCopy(ent->s.pos.trDelta, saberent->s.pos.trDelta);

	saberMoveBack(saberent, qtrue);
	saberent->s.pos.trType = TR_GRAVITY;

	trap->LinkEntity((sharedEntity_t *)saberent);	
}

#define MAX_LEAVE_TIME 20000

void saberReactivate(gentity_t *saberent, gentity_t *saberOwner);
void saberBackToOwner(gentity_t *saberent);

void DownedSaberThink(gentity_t *saberent)
{
	gentity_t *saberOwn = NULL;
	qboolean notDisowned = qfalse;
	qboolean pullBack = qfalse;

	saberent->nextthink = level.time;

	if (saberent->r.ownerNum == ENTITYNUM_NONE)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	saberOwn = &g_entities[saberent->r.ownerNum];

	if (!saberOwn ||
		!saberOwn->inuse ||
		!saberOwn->client ||
		saberOwn->client->sess.sessionTeam == TEAM_SPECTATOR ||
		(saberOwn->client->ps.pm_flags & PMF_FOLLOW))
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (saberOwn->client->ps.saberEntityNum)
	{
		if (saberOwn->client->ps.saberEntityNum == saberent->s.number)
		{ //owner shouldn't have this set if we're thinking in here. Must've fallen off a cliff and instantly respawned or something.
			notDisowned = qtrue;
		}
		else
		{ //This should never happen, but just in case..
			assert(!"ULTRA BAD THING");
			MakeDeadSaber(saberent);

			saberent->think = G_FreeEntity;
			saberent->nextthink = level.time;
			return;
		}
	}

	if (notDisowned || saberOwn->health < 1 || !saberOwn->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
	{ //He's dead, just go back to our normal saber status
		saberOwn->client->ps.saberEntityNum = saberOwn->client->saberStoredIndex;

		//MakeDeadSaber(saberent); //spawn a dead saber on top of where we are now. The "bodyqueue" method.
		//Actually this will get taken care of when the thrown saber func sees we're dead.

#ifdef _DEBUG
		if (saberOwn->client->saberStoredIndex != saberent->s.number)
		{ //I'm paranoid.
			assert(!"Bad saber index!!!");
		}
#endif

		saberReactivate(saberent, saberOwn);

		if (saberOwn->health < 1)
		{
			saberOwn->client->ps.saberInFlight = qfalse;
			MakeDeadSaber(saberent);
		}

		saberent->touch = SaberGotHit;
		saberent->think = SaberUpdateSelf;
		saberent->genericValue5 = 0;
		saberent->nextthink = level.time;

		saberent->r.svFlags |= (SVF_NOCLIENT);
		//saberent->r.contents = CONTENTS_LIGHTSABER;
		saberent->s.loopSound = 0;
		saberent->s.loopIsSoundset = qfalse;

		if (saberOwn->health > 0)
		{ //only set this if he's alive. If dead we want to reflect the lack of saber on the corpse, as he died with his saber out.
			saberOwn->client->ps.saberInFlight = qfalse;
			WP_SaberRemoveG2Model( saberent );
		}
		saberOwn->client->ps.saberEntityState = 0;
		saberOwn->client->ps.saberThrowDelay = level.time + 500;
		saberOwn->client->ps.saberCanThrow = qfalse;

		return;
	}

	if (saberOwn->client->saberKnockedTime < level.time && (saberOwn->client->pers.cmd.buttons & BUTTON_ATTACK))
	{ //He wants us back
		pullBack = qtrue;
	}
	else if ((level.time - saberOwn->client->saberKnockedTime) > MAX_LEAVE_TIME)
	{ //Been sitting around for too long, go back no matter what he wants.
		pullBack = qtrue;
	}

	if (pullBack)
	{ //Get going back to the owner.
		saberOwn->client->ps.saberEntityNum = saberOwn->client->saberStoredIndex;

#ifdef _DEBUG
		if (saberOwn->client->saberStoredIndex != saberent->s.number)
		{ //I'm paranoid.
			assert(!"Bad saber index!!!");
		}
#endif
		saberReactivate(saberent, saberOwn);

		saberent->touch = SaberGotHit;

		saberent->think = saberBackToOwner;
		saberent->speed = 0;
		saberent->genericValue5 = 0;
		saberent->nextthink = level.time;

		saberent->r.contents = CONTENTS_LIGHTSABER;

		G_Sound( saberOwn, CHAN_BODY, G_SoundIndex( "sound/weapons/force/pull.wav" ) );
		if (saberOwn->client->saber[0].soundOn)
		{
			G_Sound( saberent, CHAN_BODY, saberOwn->client->saber[0].soundOn );
		}
		if (saberOwn->client->saber[1].soundOn)
		{
			G_Sound( saberOwn, CHAN_BODY, saberOwn->client->saber[1].soundOn );
		}

		return;
	}

	G_RunObject(saberent);
	saberent->nextthink = level.time;
}

void saberReactivate(gentity_t *saberent, gentity_t *saberOwner)
{
	saberent->s.saberInFlight = qtrue;

	saberent->s.apos.trType = TR_LINEAR;
	saberent->s.apos.trDelta[0] = 0;
	saberent->s.apos.trDelta[1] = 800;
	saberent->s.apos.trDelta[2] = 0;

	saberent->s.pos.trType = TR_LINEAR;
	saberent->s.eType = ET_GENERAL;
	saberent->s.eFlags = 0;

	saberent->parent = saberOwner;

	saberent->genericValue5 = 0;

	SetSaberBoxSize(saberent);

	saberent->touch = thrownSaberTouch;

	saberent->s.weapon = WP_SABER;

	saberOwner->client->ps.saberEntityState = 1;

	trap->LinkEntity((sharedEntity_t *)saberent);
}

//[SaberThrowSys]
qboolean BG_CrouchAnim( int anim );
void DrownedSaberTouch( gentity_t *self, gentity_t *other, trace_t *trace )
{//similar to SaberBounceSound but the saber's owners can also pick up their saber by crouching or rolling over it
	VectorCopy(self->r.currentAngles, self->s.apos.trBase);
	self->s.apos.trBase[PITCH] = 90;

	//be able to pick up a dead saber by crouching/rolling over it while on the ground or by catching it mid-air.
	if(other->s.number != self->r.ownerNum || !other || !other->client)
	{//not our owner, or our owner is bad, ignore touch
		return;
	}

	if(self->s.pos.trType == TR_STATIONARY //saber isn't bouncing around.
		//and in a roll or crouching
		&& (BG_InRoll(&other->client->ps, other->client->ps.legsAnim) || BG_CrouchAnim(other->client->ps.legsAnim)))
	{//racc - picked up the saber.
		//SABERSYSRAFIXME - this could be shorter as I simply copy/pasted the entire 
		//downed saber -> returning -> caught sequence.
		other->client->ps.saberEntityNum = other->client->saberStoredIndex;

#ifdef _DEBUG
		if (other->client->saberStoredIndex != self->s.number)
		{ //I'm paranoid.
			assert(!"Bad saber index!!!");
		}
#endif
		saberReactivate(self, other);

		self->r.contents = CONTENTS_LIGHTSABER;

		G_Sound( self, CHAN_AUTO, G_SoundIndex( "sound/weapons/saber/saber_catch.wav" ) );

		other->client->ps.saberInFlight = qfalse;
		other->client->ps.saberEntityState = 0;
		other->client->ps.saberCanThrow = qfalse;
		other->client->ps.saberThrowDelay = level.time + 300;

		if(other->client->ps.forceHandExtend == HANDEXTEND_SABERPULL)
		{
			//stop holding hand out if we still are.
			other->client->ps.forceHandExtend = HANDEXTEND_NONE;
			other->client->ps.forceHandExtendTime = level.time;
		}

		self->touch = SaberGotHit;

		self->think = SaberUpdateSelf;
		self->genericValue5 = 0;
		self->nextthink = level.time + 50;
		WP_SaberRemoveG2Model( self );

		//auto reactive the blade
		other->client->ps.saberHolstered = 2;

		if (other->client->saber[0].soundOn)
		{//make activation noise if we have one.
			G_Sound(other, CHAN_WEAPON, other->client->saber[0].soundOn);
		}
	}
}
//[/SaberThrowSys]

#define SABER_RETRIEVE_DELAY 3000 //3 seconds for now. This will leave you nice and open if you lose your saber.

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other)
{
	/*saberOwner->client->ps.saberEntityNum = 0; //still stored in client->saberStoredIndex
	saberOwner->client->saberKnockedTime = level.time + SABER_RETRIEVE_DELAY;

	saberent->clipmask = MASK_SOLID;
	saberent->r.contents = CONTENTS_TRIGGER;//0;

	VectorSet( saberent->r.mins, -3.0f, -3.0f, -1.5f );
	VectorSet( saberent->r.maxs, 3.0f, 3.0f, 1.5f );

	saberent->s.apos.trType = TR_GRAVITY;
	saberent->s.apos.trDelta[0] = Q_irand(200, 800);
	saberent->s.apos.trDelta[1] = Q_irand(200, 800);
	saberent->s.apos.trDelta[2] = Q_irand(200, 800);
	saberent->s.apos.trTime = level.time-50;

	saberent->s.pos.trType = TR_GRAVITY;
	saberent->s.pos.trTime = level.time-50;
	saberent->flags |= FL_BOUNCE_HALF;

	WP_SaberAddG2Model( saberent, saberOwner->client->saber[0].model, saberOwner->client->saber[0].skin );

	saberent->s.modelGhoul2 = 1;
	saberent->s.g2radius = 20;

	saberent->s.eType = ET_MISSILE;
	saberent->s.weapon = WP_SABER;

	saberent->speed = level.time + 4000;

	saberent->bounceCount = -5;//8;

	saberMoveBack(saberent, qtrue);
	saberent->s.pos.trType = TR_GRAVITY;

	saberent->s.loopSound = 0; //kill this in case it was spinning.
	saberent->s.loopIsSoundset = qfalse;

	saberent->r.svFlags &= ~(SVF_NOCLIENT); //make sure the client is getting updates on where it is and such.

	saberent->touch = SaberBounceSound;
	saberent->think = DownedSaberThink;
	saberent->nextthink = level.time;

	if (saberOwner != other)
	{ //if someone knocked it out of the air and it wasn't turned off, go in the direction they were facing.
		if (other->inuse && other->client)
		{
			vec3_t otherFwd;
			float deflectSpeed = 200;

			AngleVectors(other->client->ps.viewangles, otherFwd, 0, 0);

			saberent->s.pos.trDelta[0] = otherFwd[0]*deflectSpeed;
			saberent->s.pos.trDelta[1] = otherFwd[1]*deflectSpeed;
			saberent->s.pos.trDelta[2] = otherFwd[2]*deflectSpeed;
		}
	}

	trap->LinkEntity((sharedEntity_t *)saberent);

	if (saberOwner->client->saber[0].soundOff)
	{
		G_Sound( saberent, CHAN_BODY, saberOwner->client->saber[0].soundOff );
	}

	if (saberOwner->client->saber[1].soundOff &&
		saberOwner->client->saber[1].model[0])
	{
		G_Sound( saberOwner, CHAN_BODY, saberOwner->client->saber[1].soundOff );
	}*/
		//[BugFix23]
	//trace stuct used for determining if it's safe to spawn at current location
	trace_t		tr;  
	//[/BugFix23]
	saberOwner->client->ps.saberEntityNum = 0; //still stored in client->saberStoredIndex
	saberOwner->client->saberKnockedTime = level.time + SABER_RETRIEVE_DELAY;

	saberent->clipmask = MASK_SOLID;

	if(saberOwner->client->ps.fd.saberAnimLevel != SS_DUAL)
		saberent->r.contents = CONTENTS_TRIGGER;//0;

	VectorSet( saberent->r.mins, -3.0f, -3.0f, -1.5f );
	VectorSet( saberent->r.maxs, 3.0f, 3.0f, 1.5f );

	//[BugFix23]
	//perform a trace before attempting to spawn at currently location.
	//unfortunately, it's a fairly regular occurance that current saber location
	//(normally at the player's right hand) could result in the saber being stuck 
	//in the the map and then freaking out.
	trap->Trace(&tr, saberent->r.currentOrigin, saberent->r.mins, saberent->r.maxs,
		saberent->r.currentOrigin, saberent->s.number, saberent->clipmask, 0, 0, 0);
	if(tr.startsolid || tr.fraction != 1)
	{//bad position, try popping our origin up a bit
		saberent->r.currentOrigin[2] += 20;
		G_SetOrigin(saberent, saberent->r.currentOrigin);
		trap->Trace(&tr, saberent->r.currentOrigin, saberent->r.mins, saberent->r.maxs,
			saberent->r.currentOrigin, saberent->s.number, saberent->clipmask, 0, 0, 0);
		if(tr.startsolid || tr.fraction != 1)
		{//still no luck, try using our owner's origin
			G_SetOrigin(saberent, saberOwner->client->ps.origin); 
			
			//since this is our last chance, we don't care if this works or not.
		}
	}
	//[/BugFix23]

	saberent->s.apos.trType = TR_GRAVITY;
	saberent->s.apos.trDelta[0] = Q_irand(200, 800);
	saberent->s.apos.trDelta[1] = Q_irand(200, 800);
	saberent->s.apos.trDelta[2] = Q_irand(200, 800);
	saberent->s.apos.trTime = level.time-50;

	saberent->s.pos.trType = TR_GRAVITY;
	saberent->s.pos.trTime = level.time-50;
	saberent->flags |= FL_BOUNCE_HALF;

	WP_SaberAddG2Model( saberent, saberOwner->client->saber[0].model, saberOwner->client->saber[0].skin );

	saberent->s.modelGhoul2 = 1;
	saberent->s.g2radius = 20;

	saberent->s.eType = ET_MISSILE;
	saberent->s.weapon = WP_SABER;

	saberent->speed = level.time + 0;//4000

	saberent->bounceCount = -5;//8;
	//[SaberThrowSys]
	//don't try to move during the frame of our creation.
	//saberMoveBack(saberent, qtrue);

	//make sure that this saber has it's owner associated with it so we can render blades on this dropped saber properly.
	saberent->s.owner = saberOwner->s.number;
	//[/SaberThrowSys]
	saberent->s.pos.trType = TR_GRAVITY;

	saberent->s.loopSound = 0; //kill this in case it was spinning.
	saberent->s.loopIsSoundset = qfalse;

	saberent->r.svFlags &= ~(SVF_NOCLIENT); //make sure the client is getting updates on where it is and such.

	//[SaberThrowSys]
	//created new function so we could have dropped sabers return then they are manually grabbed by their owners.
	saberent->touch = DrownedSaberTouch;
	//saberent->touch = SaberBounceSound;
	//[/SaberThrowSys]
	saberent->think = DownedSaberThink;
	saberent->nextthink = level.time;

	if (saberOwner != other)
	{ //if someone knocked it out of the air and it wasn't turned off, go in the direction they were facing.
		if (other->inuse && other->client)
		{
			vec3_t otherFwd;
			float deflectSpeed = 200;

			AngleVectors(other->client->ps.viewangles, otherFwd, 0, 0);

			saberent->s.pos.trDelta[0] = otherFwd[0]*deflectSpeed;
			saberent->s.pos.trDelta[1] = otherFwd[1]*deflectSpeed;
			saberent->s.pos.trDelta[2] = otherFwd[2]*deflectSpeed;
		}
	}

	trap->LinkEntity((sharedEntity_t *)saberent);

	if (saberOwner->client->saber[0].soundOff)
	{
		G_Sound( saberent, CHAN_BODY, saberOwner->client->saber[0].soundOff );
	}

	if (saberOwner->client->saber[1].soundOff &&
		saberOwner->client->saber[1].model[0])
	{
		G_Sound( saberOwner, CHAN_BODY, saberOwner->client->saber[1].soundOff );
	}

	//[SaberThrowSys]
	//properly holster the saber so the blade turns off
	if(saberOwner->client->ps.fd.saberAnimLevel == SS_DUAL)
	{//only switch off one blade if player is in the dual styley.
		saberOwner->client->ps.saberHolstered = 1;
	}
	else
	{
		saberOwner->client->ps.saberHolstered = 2;
	}
	//[/SaberThrowSys]
}

//sort of a silly macro I guess. But if I change anything in here I'll probably want it to be everywhere.
#define SABERINVALID (!saberent || !saberOwner || !other || !saberent->inuse || !saberOwner->inuse || !other->inuse || !saberOwner->client || !other->client || !saberOwner->client->ps.saberEntityNum || saberOwner->client->ps.saberLockTime > (level.time-100))

void WP_SaberRemoveG2Model( gentity_t *saberent )
{
	if ( saberent->ghoul2 )
	{
		trap->G2API_RemoveGhoul2Models( &saberent->ghoul2 );
	}
}

void WP_SaberAddG2Model( gentity_t *saberent, const char *saberModel, qhandle_t saberSkin )
{
	WP_SaberRemoveG2Model( saberent );
	if ( saberModel && saberModel[0] )
	{
		saberent->s.modelindex = G_ModelIndex(saberModel);
	}
	else
	{
		saberent->s.modelindex = G_ModelIndex( "models/weapons2/saber/saber_w.glm" );
	}
	//FIXME: use customSkin?
	trap->G2API_InitGhoul2Model( &saberent->ghoul2, saberModel, saberent->s.modelindex, saberSkin, 0, 0, 0 );
}

//Make the saber go flying directly out of the owner's hand in the specified direction
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity)
{
	if (!saberent || !saberOwner ||
		!saberent->inuse || !saberOwner->inuse ||
		!saberOwner->client)
	{
		return qfalse;
	}

	if (!saberOwner->client->ps.saberEntityNum)
	{ //already gone
		return qfalse;
	}

	if ((level.time - saberOwner->client->lastSaberStorageTime) > 50)
	{ //must have a reasonably updated saber base pos
		return qfalse;
	}

	if (saberOwner->client->ps.saberLockTime > (level.time-100))
	{
		return qfalse;
	}
	if ( (saberOwner->client->saber[0].saberFlags&SFL_NOT_DISARMABLE) )
	{
		return qfalse;
	}

	saberOwner->client->ps.saberInFlight = qtrue;
	saberOwner->client->ps.saberEntityState = 1;

	saberent->s.saberInFlight = qfalse;//qtrue;

	saberent->s.pos.trType = TR_LINEAR;
	saberent->s.eType = ET_GENERAL;
	saberent->s.eFlags = 0;

	WP_SaberAddG2Model( saberent, saberOwner->client->saber[0].model, saberOwner->client->saber[0].skin );

	saberent->s.modelGhoul2 = 127;

	saberent->parent = saberOwner;

	saberent->damage = SABER_THROWN_HIT_DAMAGE;
	saberent->methodOfDeath = MOD_SABER;
	saberent->splashMethodOfDeath = MOD_SABER;
	saberent->s.solid = 2;
	saberent->r.contents = CONTENTS_LIGHTSABER;

	saberent->genericValue5 = 0;

	VectorSet( saberent->r.mins, -24.0f, -24.0f, -8.0f );
	VectorSet( saberent->r.maxs, 24.0f, 24.0f, 8.0f );

	saberent->s.genericenemyindex = saberOwner->s.number+1024;
	saberent->s.weapon = WP_SABER;

	saberent->genericValue5 = 0;

	G_SetOrigin(saberent, saberOwner->client->lastSaberBase_Always); //use this as opposed to the right hand bolt,
	//because I don't want to risk reconstructing the skel again to get it here. And it isn't worth storing.
	saberKnockDown(saberent, saberOwner, saberOwner);
	VectorCopy(velocity, saberent->s.pos.trDelta); //override the velocity on the knocked away saber.
	
	return qtrue;
}

//Called at the result of a circle lock duel - the loser gets his saber tossed away and is put into a reflected attack anim
qboolean saberCheckKnockdown_DuelLoss(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other)
{
	vec3_t dif;
	float totalDistance = 1;
	float distScale = 6.5f;
	qboolean validMomentum = qtrue;
	int	disarmChance = 1;

	if (SABERINVALID)
	{
		return qfalse;
	}

	VectorClear(dif);

	if (!other->client->olderIsValid || (level.time - other->client->lastSaberStorageTime) >= 200)
	{ //see if the spots are valid
		validMomentum = qfalse;
	}

	if (validMomentum)
	{
		//Get the difference 
		VectorSubtract(other->client->lastSaberBase_Always, other->client->olderSaberBase, dif);
		totalDistance = VectorNormalize(dif);

		if (!totalDistance)
		{ //fine, try our own
			if (!saberOwner->client->olderIsValid || (level.time - saberOwner->client->lastSaberStorageTime) >= 200)
			{
				validMomentum = qfalse;
			}

			if (validMomentum)
			{
				VectorSubtract(saberOwner->client->lastSaberBase_Always, saberOwner->client->olderSaberBase, dif);
				totalDistance = VectorNormalize(dif);
			}
		}

		if (validMomentum)
		{
			if (!totalDistance)
			{ //try the difference between the two blades
				VectorSubtract(saberOwner->client->lastSaberBase_Always, other->client->lastSaberBase_Always, dif);
				totalDistance = VectorNormalize(dif);
			}

			if (totalDistance)
			{ //if we still have no difference somehow, just let it fall to the ground when the time comes.
				if (totalDistance < 20)
				{
					totalDistance = 20;
				}
				VectorScale(dif, totalDistance*distScale, dif);
			}
		}
	}

	saberOwner->client->ps.saberMove = LS_V1_BL; //rwwFIXMEFIXME: Ideally check which lock it was exactly and use the proper anim (same goes for the attacker)
	saberOwner->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	disarmChance += saberOwner->client->saber[0].extraDisarmChance;

	if ( Q_irand( 0, disarmChance ) >= 10)
	{
		return saberKnockOutOfHand(saberent, saberOwner, dif);
	}
	else
	{
		return qfalse;
	}
}

//Called when we want to try knocking the saber out of the owner's hand upon them going into a broken parry.
//Also called on reflected attacks.
qboolean saberCheckKnockdown_BrokenParry(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other)
{
	qboolean doKnock = qfalse;
	int	disarmChance = 1;

	if (SABERINVALID)
	{
		return qfalse;
	}

	if (!other->client->olderIsValid || (level.time - other->client->lastSaberStorageTime) >= 200)
	{ //if we don't know which way to throw the saber based on momentum between saber positions, just don't throw it
		return qfalse;
	}

	disarmChance += saberOwner->client->saber[0].extraDisarmChance;

	if( Q_irand(0, disarmChance) >= 10 )
	{
		doKnock = qtrue;
	}

	if (doKnock)
	{
		vec3_t dif;
		float totalDistance;
		float distScale = 6.5f;

		VectorSubtract(other->client->lastSaberBase_Always, other->client->olderSaberBase, dif);
		totalDistance = VectorNormalize(dif);

		if (!totalDistance)
		{ //fine, try our own
			if (!saberOwner->client->olderIsValid || (level.time - saberOwner->client->lastSaberStorageTime) >= 200)
			{ //if we don't know which way to throw the saber based on momentum between saber positions, just don't throw it
				return qfalse;
			}

			VectorSubtract(saberOwner->client->lastSaberBase_Always, saberOwner->client->olderSaberBase, dif);
			totalDistance = VectorNormalize(dif);
		}

		if (!totalDistance)
		{ //...forget it then.
			return qfalse;
		}

		if (totalDistance < 20)
		{
			totalDistance = 20;
		}
		 
		VectorScale(dif, totalDistance*distScale, dif);

		return saberKnockOutOfHand(saberent, saberOwner, dif);
	}

	return qfalse;
}

qboolean BG_InExtraDefenseSaberMove( int move );

//Called upon an enemy actually slashing into a thrown saber
qboolean saberCheckKnockdown_Smashed(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other, int damage)
{
	if (SABERINVALID)
	{
		return qfalse;
	}

	if (!saberOwner->client->ps.saberInFlight)
	{ //can only do this if the saber is already actually in flight
		return qfalse;
	}

	if ( other
		&& other->inuse
		&& other->client 
		&& BG_InExtraDefenseSaberMove( other->client->ps.saberMove ) )
	{ //make sure the blow was strong enough
		saberKnockDown(saberent, saberOwner, other);
		return qtrue;
	}

	if (damage > 10)
	{ //make sure the blow was strong enough
		saberKnockDown(saberent, saberOwner, other);
		return qtrue;
	}

	return qfalse;
}

//Called upon blocking a thrown saber. If the throw level compared to the blocker's defense level
//is inferior, or equal and a random factor is met, then the saber will be tossed to the ground.
qboolean saberCheckKnockdown_Thrown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other)
{
	int throwLevel = 0;
	int defenLevel = 0;
	qboolean tossIt = qfalse;

	if (SABERINVALID)
	{
		return qfalse;
	}

	defenLevel = other->client->ps.fd.forcePowerLevel[FP_SABER_DEFENSE];
	throwLevel = saberOwner->client->ps.fd.forcePowerLevel[FP_SABERTHROW];

	if (defenLevel > throwLevel)
	{
		tossIt = qtrue;
	}
	else if (defenLevel == throwLevel && Q_irand(1, 10) <= 4)
	{
		tossIt = qtrue;
	}
	//otherwise don't

	if (tossIt)
	{
		saberKnockDown(saberent, saberOwner, other);
		return qtrue;
	}

	return qfalse;
}

void saberBackToOwner(gentity_t *saberent)
{
	gentity_t *saberOwner = &g_entities[saberent->r.ownerNum];
	vec3_t dir;
	float ownerLen;

	if (saberent->r.ownerNum == ENTITYNUM_NONE)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (!saberOwner->inuse ||
		!saberOwner->client ||
		saberOwner->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (saberOwner->health < 1 || !saberOwner->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
	{ //He's dead, just go back to our normal saber status
		saberent->touch = SaberGotHit;
		saberent->think = SaberUpdateSelf;
		saberent->genericValue5 = 0;
		saberent->nextthink = level.time;

		if (saberOwner->client &&
			saberOwner->client->saber[0].soundOff)
		{
			G_Sound(saberent, CHAN_AUTO, saberOwner->client->saber[0].soundOff);
		}
		MakeDeadSaber(saberent);

		saberent->r.svFlags |= (SVF_NOCLIENT);
		saberent->r.contents = CONTENTS_LIGHTSABER;
		SetSaberBoxSize(saberent);
		saberent->s.loopSound = 0;
		saberent->s.loopIsSoundset = qfalse;
		WP_SaberRemoveG2Model( saberent );

		saberOwner->client->ps.saberInFlight = qfalse;
		saberOwner->client->ps.saberEntityState = 0;
		saberOwner->client->ps.saberThrowDelay = level.time + 500;
		saberOwner->client->ps.saberCanThrow = qfalse;

		return;
	}

	//make sure this is set alright
	assert(saberOwner->client->ps.saberEntityNum == saberent->s.number ||
		saberOwner->client->saberStoredIndex == saberent->s.number);
	saberOwner->client->ps.saberEntityNum = saberent->s.number;

	saberent->r.contents = CONTENTS_LIGHTSABER;

	VectorSubtract(saberent->pos1, saberent->r.currentOrigin, dir);

	ownerLen = VectorLength(dir);

	if (saberent->speed < level.time)
	{
		float baseSpeed = 900;

		VectorNormalize(dir);

		saberMoveBack(saberent, qtrue);
		VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

		if (saberOwner->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //allow players with high saber throw rank to control the return speed of the saber
			baseSpeed = 900;

			saberent->speed = level.time;
		}
		else
		{
			baseSpeed = 700;
			saberent->speed = level.time + 50;
		}

		//Gradually slow down as it approaches, so it looks smoother coming into the hand.
		if (ownerLen < 64)
		{
			VectorScale(dir, baseSpeed-600, saberent->s.pos.trDelta );
		}
		else if (ownerLen < 128)
		{
			VectorScale(dir, baseSpeed-400, saberent->s.pos.trDelta );
		}
		else if (ownerLen < 256)
		{
			VectorScale(dir, baseSpeed-200, saberent->s.pos.trDelta );
		}
		else
		{
			VectorScale(dir, baseSpeed, saberent->s.pos.trDelta );
		}

		saberent->s.pos.trTime = level.time;
	}
	
	//I'm just doing this now. I don't really like the spin on the way back. And it does weird stuff with the new saber-knocked-away code.
	if (saberOwner->client->ps.saberEntityNum == saberent->s.number)
	{
		if ( !(saberOwner->client->saber[0].saberFlags&SFL_RETURN_DAMAGE)
			|| saberOwner->client->ps.saberHolstered )
		{
			saberent->s.saberInFlight = qfalse;
		}
		saberent->s.loopSound = saberOwner->client->saber[0].soundLoop;
		saberent->s.loopIsSoundset = qfalse;

		if (ownerLen <= 32)
		{
			G_Sound( saberent, CHAN_AUTO, G_SoundIndex( "sound/weapons/saber/saber_catch.wav" ) );

			saberOwner->client->ps.saberInFlight = qfalse;
			saberOwner->client->ps.saberEntityState = 0;
			saberOwner->client->ps.saberCanThrow = qfalse;
			saberOwner->client->ps.saberThrowDelay = level.time + 100;

			saberent->touch = SaberGotHit;

			saberent->think = SaberUpdateSelf;
			saberent->genericValue5 = 0;
			saberent->nextthink = level.time + 50;
			WP_SaberRemoveG2Model( saberent );

			return;
		}

		if (!saberent->s.saberInFlight)
		{
			saberCheckRadiusDamage(saberent, 1);
		}
		else
		{
			saberCheckRadiusDamage(saberent, 2);
		}

		saberMoveBack(saberent, qtrue);
	}

	saberent->nextthink = level.time;
}

void saberFirstThrown(gentity_t *saberent);

void thrownSaberTouch (gentity_t *saberent, gentity_t *other, trace_t *trace)
{
	gentity_t *hitEnt = other;

	if (other && other->s.number == saberent->r.ownerNum)
	{
		return;
	}
	VectorClear(saberent->s.pos.trDelta);
	saberent->s.pos.trTime = level.time;

	saberent->s.apos.trType = TR_LINEAR;
	saberent->s.apos.trDelta[0] = 0;
	saberent->s.apos.trDelta[1] = 0;
	saberent->s.apos.trDelta[2] = 0;

	//VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

	saberent->think = saberBackToOwner;
	saberent->nextthink = level.time + 50;

	if (other && other->r.ownerNum < MAX_CLIENTS &&
		(other->r.contents & CONTENTS_LIGHTSABER) &&
		g_entities[other->r.ownerNum].client &&
		g_entities[other->r.ownerNum].inuse)
	{
		hitEnt = &g_entities[other->r.ownerNum];
	}

	//we'll skip the dist check, since we don't really care about that (we just hit it physically)
	CheckThrownSaberDamaged(saberent, &g_entities[saberent->r.ownerNum], hitEnt, 256, 0, qtrue);

	saberent->speed = 0;
}

#define SABER_MAX_THROW_DISTANCE 700

void saberFirstThrown(gentity_t *saberent)
{
	vec3_t		vSub;
	float		vLen;
	gentity_t	*saberOwn = &g_entities[saberent->r.ownerNum];

	if (saberent->r.ownerNum == ENTITYNUM_NONE)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (!saberOwn ||
		!saberOwn->inuse ||
		!saberOwn->client ||
		saberOwn->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		MakeDeadSaber(saberent);

		saberent->think = G_FreeEntity;
		saberent->nextthink = level.time;
		return;
	}

	if (saberOwn->health < 1 || !saberOwn->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE])
	{ //He's dead, just go back to our normal saber status
		saberent->touch = SaberGotHit;
		saberent->think = SaberUpdateSelf;
		saberent->genericValue5 = 0;
		saberent->nextthink = level.time;

		if (saberOwn->client &&
			saberOwn->client->saber[0].soundOff)
		{
			G_Sound(saberent, CHAN_AUTO, saberOwn->client->saber[0].soundOff);
		}
		MakeDeadSaber(saberent);

		saberent->r.svFlags |= (SVF_NOCLIENT);
		saberent->r.contents = CONTENTS_LIGHTSABER;
		SetSaberBoxSize(saberent);
		saberent->s.loopSound = 0;
		saberent->s.loopIsSoundset = qfalse;
		WP_SaberRemoveG2Model( saberent );

		saberOwn->client->ps.saberInFlight = qfalse;
		saberOwn->client->ps.saberEntityState = 0;
		saberOwn->client->ps.saberThrowDelay = level.time + 500;
		saberOwn->client->ps.saberCanThrow = qfalse;

		return;
	}

	if ((level.time - saberOwn->client->ps.saberDidThrowTime) > 500)
	{
		if ((level.time - saberOwn->client->ps.saberDidThrowTime) > 6000)
		{ //if it's out longer than 6 seconds, return it
			thrownSaberTouch(saberent, saberent, NULL);
			goto runMin;
		}
	}
	
	if (!BG_CanUseFPNow(level.gametype, &saberOwn->client->ps, level.time, FP_SABERTHROW))
	{
		thrownSaberTouch(saberent, saberent, NULL);
		goto runMin;
	}

	VectorSubtract(saberOwn->client->ps.origin, saberent->r.currentOrigin, vSub);
	vLen = VectorLength(vSub);

	if (vLen >= (SABER_MAX_THROW_DISTANCE*saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW]))
	{
		thrownSaberTouch(saberent, saberent, NULL);
		goto runMin;
	}

	if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_2 &&
		saberent->speed < level.time)
	{ //if owner is rank 3 in saber throwing, the saber goes where he points
		vec3_t fwd, traceFrom, traceTo, dir;
		trace_t tr;

		AngleVectors(saberOwn->client->ps.viewangles, fwd, 0, 0);

		VectorCopy(saberOwn->client->ps.origin, traceFrom);
		traceFrom[2] += saberOwn->client->ps.viewheight;

		VectorCopy(traceFrom, traceTo);
		traceTo[0] += fwd[0]*4096;
		traceTo[1] += fwd[1]*4096;
		traceTo[2] += fwd[2]*4096;

		saberMoveBack(saberent, qfalse);
		VectorCopy(saberent->r.currentOrigin, saberent->s.pos.trBase);

		if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //if highest saber throw rank, we can direct the saber toward players directly by looking at them
			trap->Trace(&tr, traceFrom, NULL, NULL, traceTo, saberOwn->s.number, MASK_PLAYERSOLID, 0, 0, 0);
		}
		else
		{
			trap->Trace(&tr, traceFrom, NULL, NULL, traceTo, saberOwn->s.number, MASK_SOLID, 0, 0, 0);
		}

		VectorSubtract(tr.endpos, saberent->r.currentOrigin, dir);

		VectorNormalize(dir);

		VectorScale(dir, 500, saberent->s.pos.trDelta );
		saberent->s.pos.trTime = level.time;

		if (saberOwn->client->ps.fd.forcePowerLevel[FP_SABERTHROW] >= FORCE_LEVEL_3)
		{ //we'll treat them to a quicker update rate if their throw rank is high enough
			saberent->speed = level.time + 100;
		}
		else
		{
			saberent->speed = level.time + 400;
		}
	}

runMin:

	saberCheckRadiusDamage(saberent, 0);
	G_RunObject(saberent);
}

void UpdateClientRenderBolts(gentity_t *self, vec3_t renderOrigin, vec3_t renderAngles)
{
	mdxaBone_t boltMatrix;
	renderInfo_t *ri = &self->client->renderInfo;

	if (!self->ghoul2)
	{
		VectorCopy(self->client->ps.origin, ri->headPoint);
		VectorCopy(self->client->ps.origin, ri->handRPoint);
		VectorCopy(self->client->ps.origin, ri->handLPoint);
		VectorCopy(self->client->ps.origin, ri->torsoPoint);
		VectorCopy(self->client->ps.origin, ri->crotchPoint);
		VectorCopy(self->client->ps.origin, ri->footRPoint);
		VectorCopy(self->client->ps.origin, ri->footLPoint);
	}
	else
	{
		//head
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->headBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->headPoint[0] = boltMatrix.matrix[0][3];
		ri->headPoint[1] = boltMatrix.matrix[1][3];
		ri->headPoint[2] = boltMatrix.matrix[2][3];

		//right hand
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->handRBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->handRPoint[0] = boltMatrix.matrix[0][3];
		ri->handRPoint[1] = boltMatrix.matrix[1][3];
		ri->handRPoint[2] = boltMatrix.matrix[2][3];

		//left hand
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->handLBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->handLPoint[0] = boltMatrix.matrix[0][3];
		ri->handLPoint[1] = boltMatrix.matrix[1][3];
		ri->handLPoint[2] = boltMatrix.matrix[2][3];

		//chest
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->torsoBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->torsoPoint[0] = boltMatrix.matrix[0][3];
		ri->torsoPoint[1] = boltMatrix.matrix[1][3];
		ri->torsoPoint[2] = boltMatrix.matrix[2][3];

		//crotch
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->crotchBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->crotchPoint[0] = boltMatrix.matrix[0][3];
		ri->crotchPoint[1] = boltMatrix.matrix[1][3];
		ri->crotchPoint[2] = boltMatrix.matrix[2][3];

		//right foot
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->footRBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->footRPoint[0] = boltMatrix.matrix[0][3];
		ri->footRPoint[1] = boltMatrix.matrix[1][3];
		ri->footRPoint[2] = boltMatrix.matrix[2][3];

		//left foot
		trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->footLBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
		ri->footLPoint[0] = boltMatrix.matrix[0][3];
		ri->footLPoint[1] = boltMatrix.matrix[1][3];
		ri->footLPoint[2] = boltMatrix.matrix[2][3];
	}

	self->client->renderInfo.boltValidityTime = level.time;
}

static void UpdateClientRenderinfo(gentity_t *self, vec3_t renderOrigin, vec3_t renderAngles)
{
	renderInfo_t *ri = &self->client->renderInfo;
	if ( ri->mPCalcTime < level.time )
	{
		//We're just going to give rough estimates on most of this stuff,
		//it's not like most of it matters.

	#if 0 //#if 0'd since it's a waste setting all this to 0 each frame.
		//Should you wish to make any of this valid then feel free to do so.
		ri->headYawRangeLeft = ri->headYawRangeRight = ri->headPitchRangeUp = ri->headPitchRangeDown = 0;
		ri->torsoYawRangeLeft = ri->torsoYawRangeRight = ri->torsoPitchRangeUp = ri->torsoPitchRangeDown = 0;

		ri->torsoFpsMod = ri->legsFpsMod = 0;

		VectorClear(ri->customRGB);
		ri->customAlpha = 0;
		ri->renderFlags = 0;
		ri->lockYaw = 0;

		VectorClear(ri->headAngles);
		VectorClear(ri->torsoAngles);

		//VectorClear(ri->eyeAngles);

		ri->legsYaw = 0;
	#endif

		if (self->ghoul2 &&
			self->ghoul2 != ri->lastG2)
		{ //the g2 instance changed, so update all the bolts.
			//rwwFIXMEFIXME: Base on skeleton used? Assuming humanoid currently.
			ri->lastG2 = self->ghoul2;

			if (self->localAnimIndex < NUM_RESERVED_ANIMSETS)
			{
				ri->headBolt = trap->G2API_AddBolt(self->ghoul2, 0, "*head_eyes");
				ri->handRBolt = trap->G2API_AddBolt(self->ghoul2, 0, "*r_hand");
				ri->handLBolt = trap->G2API_AddBolt(self->ghoul2, 0, "*l_hand");
				ri->torsoBolt = trap->G2API_AddBolt(self->ghoul2, 0, "thoracic");
				ri->crotchBolt = trap->G2API_AddBolt(self->ghoul2, 0, "pelvis");
				ri->footRBolt = trap->G2API_AddBolt(self->ghoul2, 0, "*r_leg_foot");
				ri->footLBolt = trap->G2API_AddBolt(self->ghoul2, 0, "*l_leg_foot");
				ri->motionBolt = trap->G2API_AddBolt(self->ghoul2, 0, "Motion");
			}
			else
			{
				ri->headBolt = -1;
				ri->handRBolt = -1;
				ri->handLBolt = -1;
				ri->torsoBolt = -1;
				ri->crotchBolt = -1;
				ri->footRBolt = -1;
				ri->footLBolt = -1;
				ri->motionBolt = -1;
			}

			ri->lastG2 = self->ghoul2;
		}

		VectorCopy( self->client->ps.viewangles, self->client->renderInfo.eyeAngles );

		//we'll just say the legs/torso are whatever the first frame of our current anim is.
		ri->torsoFrame = bgAllAnims[self->localAnimIndex].anims[self->client->ps.torsoAnim].firstFrame;
		ri->legsFrame = bgAllAnims[self->localAnimIndex].anims[self->client->ps.legsAnim].firstFrame;
		if (g_debugServerSkel.integer)
		{	//Alright, I was doing this, but it's just too slow to do every frame.
			//From now on if we want this data to be valid we're going to have to make a verify call for it before
			//accessing it. I'm only doing this now if we want to debug the server skel by drawing lines from bolt
			//positions every frame.
			mdxaBone_t boltMatrix;

			if (!self->ghoul2)
			{
				VectorCopy(self->client->ps.origin, ri->headPoint);
				VectorCopy(self->client->ps.origin, ri->handRPoint);
				VectorCopy(self->client->ps.origin, ri->handLPoint);
				VectorCopy(self->client->ps.origin, ri->torsoPoint);
				VectorCopy(self->client->ps.origin, ri->crotchPoint);
				VectorCopy(self->client->ps.origin, ri->footRPoint);
				VectorCopy(self->client->ps.origin, ri->footLPoint);
			}
			else
			{
				//head
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->headBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->headPoint[0] = boltMatrix.matrix[0][3];
				ri->headPoint[1] = boltMatrix.matrix[1][3];
				ri->headPoint[2] = boltMatrix.matrix[2][3];

				//right hand
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->handRBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->handRPoint[0] = boltMatrix.matrix[0][3];
				ri->handRPoint[1] = boltMatrix.matrix[1][3];
				ri->handRPoint[2] = boltMatrix.matrix[2][3];

				//left hand
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->handLBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->handLPoint[0] = boltMatrix.matrix[0][3];
				ri->handLPoint[1] = boltMatrix.matrix[1][3];
				ri->handLPoint[2] = boltMatrix.matrix[2][3];

				//chest
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->torsoBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->torsoPoint[0] = boltMatrix.matrix[0][3];
				ri->torsoPoint[1] = boltMatrix.matrix[1][3];
				ri->torsoPoint[2] = boltMatrix.matrix[2][3];

				//crotch
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->crotchBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->crotchPoint[0] = boltMatrix.matrix[0][3];
				ri->crotchPoint[1] = boltMatrix.matrix[1][3];
				ri->crotchPoint[2] = boltMatrix.matrix[2][3];

				//right foot
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->footRBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->footRPoint[0] = boltMatrix.matrix[0][3];
				ri->footRPoint[1] = boltMatrix.matrix[1][3];
				ri->footRPoint[2] = boltMatrix.matrix[2][3];

				//left foot
				trap->G2API_GetBoltMatrix(self->ghoul2, 0, ri->footLBolt, &boltMatrix, renderAngles, renderOrigin, level.time, NULL, self->modelScale);
				ri->footLPoint[0] = boltMatrix.matrix[0][3];
				ri->footLPoint[1] = boltMatrix.matrix[1][3];
				ri->footLPoint[2] = boltMatrix.matrix[2][3];
			}

			//Now draw the skel for debug
			G_TestLine(ri->headPoint, ri->torsoPoint, 0x000000ff, 50);
			G_TestLine(ri->torsoPoint, ri->handRPoint, 0x000000ff, 50);
			G_TestLine(ri->torsoPoint, ri->handLPoint, 0x000000ff, 50);
			G_TestLine(ri->torsoPoint, ri->crotchPoint, 0x000000ff, 50);
			G_TestLine(ri->crotchPoint, ri->footRPoint, 0x000000ff, 50);
			G_TestLine(ri->crotchPoint, ri->footLPoint, 0x000000ff, 50);
		}

		//muzzle point calc (we are going to be cheap here)
		VectorCopy(ri->muzzlePoint, ri->muzzlePointOld);
		VectorCopy(self->client->ps.origin, ri->muzzlePoint);
		VectorCopy(ri->muzzleDir, ri->muzzleDirOld);
		AngleVectors(self->client->ps.viewangles, ri->muzzleDir, 0, 0);
		ri->mPCalcTime = level.time;

		VectorCopy(self->client->ps.origin, ri->eyePoint);
		ri->eyePoint[2] += self->client->ps.viewheight;
	}
}

static bool QINLINE JKG_CanWeThrowSaber( gentity_t *self )
{
	if (self->client->ps.saberThrowDelay < level.time)
	{
		if ( (self->client->saber[0].saberFlags&SFL_NOT_THROWABLE) )
		{//cant throw it normally!
			if ( (self->client->saber[0].saberFlags&SFL_SINGLE_BLADE_THROWABLE) )
			{//but can throw it if only have 1 blade on
				if ( self->client->saber[0].numBlades > 1
					&& self->client->ps.saberHolstered == 1 )
				{//have multiple blades and only one blade on
					return true;
				}
				else
				{//multiple blades on, can't throw
					return false;
				}
			}
			else
			{//never can throw it
				return false;
			}
		}
		else
		{//can throw it!
			return true;
		}
	}
	return self->client->ps.saberCanThrow;
}

void WP_SaberPositionUpdate( gentity_t *self, usercmd_t *ucmd )
{ //rww - keep the saber position as updated as possible on the server so that we can try to do realistic-looking contact stuff
  //Note that this function also does the majority of working in maintaining the server g2 client instance (updating angles/anims/etc)
	gentity_t *mySaber = NULL;
	mdxaBone_t	boltMatrix;
	vec3_t properAngles, properOrigin;
	vec3_t boltAngles, boltOrigin;
	vec3_t end;
	vec3_t legAxis[3];
	vec3_t addVel;
	vec3_t rawAngles;
	float fVSpeed = 0;
	int returnAfterUpdate = 0;
	float animSpeedScale = 1.0f;
	int saberNum;
	qboolean clientOverride = false;
	int rSaberNum = 0;
	int rBladeNum = 0;

#ifdef _DEBUG
	if (g_disableServerG2.integer)
	{
		return;
	}
#endif

	if (self && self->inuse && self->client)
	{
		if (self->client->saberCycleQueue)
		{
			self->client->ps.fd.saberDrawAnimLevel = self->client->saberCycleQueue;
		}
		else
		{
			self->client->ps.fd.saberDrawAnimLevel = self->client->ps.fd.saberAnimLevel;
		}
	}

	if (self &&
		self->inuse &&
		self->client &&
		self->client->saberCycleQueue &&
		(self->client->ps.weaponTime <= 0 || self->health < 1))
	{ //we cycled attack levels while we were busy, so update now that we aren't (even if that means we're dead)
		self->client->ps.fd.saberAnimLevel = self->client->saberCycleQueue;
		self->client->saberCycleQueue = 0;
	}

	if (!self ||
		!self->inuse ||
		!self->client ||
		!self->ghoul2 ||
		!g2SaberInstance)
	{
		return;
	}

	if (BG_KickingAnim(self->client->ps.legsAnim))
	{ //do some kick traces and stuff if we're in the appropriate anim
		G_KickSomeMofos(self);
	}
	else if (self->client->ps.torsoAnim == BOTH_KYLE_GRAB)
	{ //try to grab someone
		G_GrabSomeMofos(self);
	}
	else if (self->client->grappleState)
	{
		JKG_GrappleUpdate(self);
	}

	//If this is a listen server (client+server running on same machine),
	//then lets try to steal the skeleton/etc data off the client instance
	//for this entity to save us processing time.
	clientOverride = trap->G2API_OverrideServer(self->ghoul2);

	saberNum = self->client->ps.saberEntityNum;

	if (!saberNum)
	{
		saberNum = self->client->saberStoredIndex;
	}

	if (!saberNum)
	{
		returnAfterUpdate = 1;
		goto nextStep;
	}

	mySaber = &g_entities[saberNum];

	if (self->health < 1)
	{ //we don't want to waste precious CPU time calculating saber positions for corpses. But we want to avoid the saber ent position lagging on spawn, so..
		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //Since we haven't got a bolt position, place it on top of the player origin.
			VectorCopy(self->client->ps.origin, mySaber->r.currentOrigin);
		}
	}

	if ( BG_SuperBreakWinAnim( self->client->ps.torsoAnim ) )
	{
		self->client->ps.weaponstate = WEAPON_FIRING;
	}

	if (self->client->ps.weapon != WP_SABER ||
		self->client->ps.weaponstate == WEAPON_RAISING ||
		self->client->ps.weaponstate == WEAPON_DROPPING ||
		self->health < 1)
	{
		if (!self->client->ps.saberInFlight)
		{
			returnAfterUpdate = 1;
		}
	}

	self->client->ps.saberCanThrow = JKG_CanWeThrowSaber(self);
nextStep:
	if (self->client->ps.fd.forcePowersActive & (1 << FP_RAGE))
	{
		animSpeedScale = 2;
	}

	// eez: factor in different lightsaber moves here
	if( self->client->ps.saberMove >= LS_A_TL2BR &&
		self->client->ps.saberMove <= LS_A_BACKSTAB )
	{
		animSpeedScale = 0.8;
	}
	
	VectorCopy(self->client->ps.origin, properOrigin);

	//try to predict the origin based on velocity so it's more like what the client is seeing
	VectorCopy(self->client->ps.velocity, addVel);
	VectorNormalize(addVel);

	if (self->client->ps.velocity[0] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[0]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[0];
	}
	if (self->client->ps.velocity[1] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[1]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[1];
	}
	if (self->client->ps.velocity[2] < 0)
	{
		fVSpeed += (-self->client->ps.velocity[2]);
	}
	else
	{
		fVSpeed += self->client->ps.velocity[2];
	}

	//fVSpeed *= 0.08;
	fVSpeed *= 1.6f/sv_fps.value;

	//Cap it off at reasonable values so the saber box doesn't go flying ahead of us or
	//something if we get a big speed boost from something.
	if (fVSpeed > 70)
	{
		fVSpeed = 70;
	}
	if (fVSpeed < -70)
	{
		fVSpeed = -70;
	}

	properOrigin[0] += addVel[0]*fVSpeed;
	properOrigin[1] += addVel[1]*fVSpeed;
	properOrigin[2] += addVel[2]*fVSpeed;

	properAngles[0] = 0;
	properAngles[1] = self->client->ps.viewangles[YAW];
	properAngles[2] = 0;

	AnglesToAxis( properAngles, legAxis );

	UpdateClientRenderinfo(self, properOrigin, properAngles);

	if (!clientOverride)
	{ //if we get the client instance we don't need to do this
		G_G2PlayerAngles( self, legAxis, properAngles );
	}

	if (returnAfterUpdate && saberNum)
	{ //We don't even need to do GetBoltMatrix if we're only in here to keep the g2 server instance in sync
		//but keep our saber entity in sync too, just copy it over our origin.

		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //Since we haven't got a bolt position, place it on top of the player origin.
			VectorCopy(self->client->ps.origin, mySaber->r.currentOrigin);
		}

		goto finalUpdate;
	}

	if (returnAfterUpdate)
	{
		goto finalUpdate;
	}

	//We'll get data for blade 0 first no matter what it is and stick them into
	//the constant ("_Always") values. Later we will handle going through each blade.
	trap->G2API_GetBoltMatrix(self->ghoul2, 1, 0, &boltMatrix, properAngles, properOrigin, level.time, NULL, self->modelScale);
	BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, boltOrigin);
	BG_GiveMeVectorFromMatrix(&boltMatrix, NEGATIVE_Y, boltAngles);

	//immediately store these values so we don't have to recalculate this again
	if (self->client->lastSaberStorageTime && (level.time - self->client->lastSaberStorageTime) < 200)
	{ //alright
		VectorCopy(self->client->lastSaberBase_Always, self->client->olderSaberBase);
		self->client->olderIsValid = qtrue;
	}
	else
	{
		self->client->olderIsValid = qfalse;
	}

	VectorCopy(boltOrigin, self->client->lastSaberBase_Always);
	VectorCopy(boltAngles, self->client->lastSaberDir_Always);
	self->client->lastSaberStorageTime = level.time;

	VectorCopy(boltAngles, rawAngles);

	VectorMA( boltOrigin, self->client->saber[0].blade[0].lengthMax, boltAngles, end );

	if (self->client->ps.saberEntityNum)
	{
		//I guess it's good to keep the position updated even when contents are 0
		if (mySaber && ((mySaber->r.contents & CONTENTS_LIGHTSABER) || mySaber->r.contents == 0) && !self->client->ps.saberInFlight)
		{ //place it roughly in the middle of the saber..
			VectorMA( boltOrigin, self->client->saber[0].blade[0].lengthMax, boltAngles, mySaber->r.currentOrigin );
		}
	}

	boltAngles[YAW] = self->client->ps.viewangles[YAW];

	if (self->client->ps.saberInFlight)
	{ //do the thrown-saber stuff
		gentity_t *saberent = &g_entities[saberNum];

		if (saberent)
		{
			if (!self->client->ps.saberEntityState && self->client->ps.saberEntityNum)
			{
				vec3_t startorg, startang, dir;

				VectorCopy(boltOrigin, saberent->r.currentOrigin);

				VectorCopy(boltOrigin, startorg);
				VectorCopy(boltAngles, startang);

				startang[0] = 90;
				//Instead of this we'll sort of fake it and slowly tilt it down on the client via
				//a perframe method (which doesn't actually affect where or how the saber hits)

				saberent->r.svFlags &= ~(SVF_NOCLIENT);
				VectorCopy(startorg, saberent->s.pos.trBase);
				VectorCopy(startang, saberent->s.apos.trBase);

				VectorCopy(startorg, saberent->s.origin);
				VectorCopy(startang, saberent->s.angles);

				saberent->s.saberInFlight = qtrue;



				saberent->s.pos.trType = TR_LINEAR;

				saberent->s.apos.trDelta[0] = SaberStances[self->client->ps.fd.saberAnimLevel].saberThrowPitch;
				saberent->s.apos.trDelta[1] = SaberStances[self->client->ps.fd.saberAnimLevel].saberThrowYaw;
				saberent->s.apos.trDelta[2] = SaberStances[self->client->ps.fd.saberAnimLevel].saberThrowRoll;

				saberent->s.eType = ET_GENERAL;
				saberent->s.eFlags = 0;

				WP_SaberAddG2Model( saberent, self->client->saber[0].model, self->client->saber[0].skin );

				saberent->s.modelGhoul2 = 127;

				saberent->parent = self;

				self->client->ps.saberEntityState = 1;

				//Projectile stuff:
				AngleVectors(self->client->ps.viewangles, dir, NULL, NULL);

				saberent->nextthink = level.time + FRAMETIME;
				saberent->think = saberFirstThrown;

				saberent->damage = SABER_THROWN_HIT_DAMAGE;
				saberent->methodOfDeath = MOD_SABER;
				saberent->splashMethodOfDeath = MOD_SABER;
				saberent->s.solid = 2;
				saberent->r.contents = CONTENTS_LIGHTSABER;

				saberent->genericValue5 = 0;

				VectorSet( saberent->r.mins, SABERMINS_X, SABERMINS_Y, SABERMINS_Z );
				VectorSet( saberent->r.maxs, SABERMAXS_X, SABERMAXS_Y, SABERMAXS_Z );

				saberent->s.genericenemyindex = self->s.number+1024;

				saberent->touch = thrownSaberTouch;

				saberent->s.weapon = WP_SABER;

				VectorScale(dir, 900, saberent->s.pos.trDelta );
				saberent->s.pos.trTime = level.time;

				if ( self->client->saber[0].spinSound )
				{
					saberent->s.loopSound = self->client->saber[0].spinSound;
				}
				else
				{
							saberent->s.loopSound = saberSpinSound;
				}
				saberent->s.loopIsSoundset = qfalse;

				self->client->ps.saberDidThrowTime = level.time;

				self->client->dangerTime = level.time;
				self->client->ps.eFlags &= ~EF_INVULNERABLE;
				self->client->invulnerableTimer = 0;

				trap->LinkEntity((sharedEntity_t *)saberent);
			}
			else if (self->client->ps.saberEntityNum) //only do this stuff if your saber is active and has not been knocked out of the air.
			{
				VectorCopy(boltOrigin, saberent->pos1);
				trap->LinkEntity((sharedEntity_t *)saberent);

				if (saberent->genericValue5 == PROPER_THROWN_VALUE)
				{ //return to the owner now, this is a bad state to be in for here..
					saberent->genericValue5 = 0;
					saberent->think = SaberUpdateSelf;
					saberent->nextthink = level.time;
					WP_SaberRemoveG2Model( saberent );
					
					self->client->ps.saberInFlight = qfalse;
					self->client->ps.saberEntityState = 0;
					self->client->ps.saberThrowDelay = level.time + 500;
					self->client->ps.saberCanThrow = qfalse;
				}
			}
		}
	}

	/*
	if (self->client->ps.saberInFlight)
	{ //if saber is thrown then only do the standard stuff for the left hand saber
		rSaberNum = 1;
	}
	*/

	if (!BG_SabersOff(&self->client->ps))
	{
		gentity_t *saberent = &g_entities[saberNum];

		if (!self->client->ps.saberInFlight && saberent)
		{
			saberent->r.svFlags |= (SVF_NOCLIENT);
			saberent->r.contents = CONTENTS_LIGHTSABER;
			SetSaberBoxSize(saberent);
			saberent->s.loopSound = 0;
			saberent->s.loopIsSoundset = qfalse;
		}

		if (self->client->ps.saberLockTime > level.time && self->client->ps.saberEntityNum)
		{
			if (self->client->saberIdleWound < level.time)
			{
				gentity_t *te;
				vec3_t dir;
				te = G_TempEntity( g_entities[saberNum].r.currentOrigin, EV_SABER_BLOCK );
				VectorSet( dir, 0, 1, 0 );
				VectorCopy(g_entities[saberNum].r.currentOrigin, te->s.origin);
				VectorCopy(dir, te->s.angles);
				te->s.eventParm = 1;
				te->s.weapon = 0;//saberNum
				te->s.legsAnim = 0;//bladeNum

				self->client->saberIdleWound = level.time + Q_irand(400, 600);
			}

			while (rSaberNum < MAX_SABERS)
			{
				rBladeNum = 0;
				while (rBladeNum < self->client->saber[rSaberNum].numBlades)
				{ //Don't bother updating the bolt for each blade for this, it's just a very rough fallback method for during saberlocks
					VectorCopy(boltOrigin, self->client->saber[saberNum].blade[rBladeNum].trail.base);
					VectorCopy(end, self->client->saber[saberNum].blade[rBladeNum].trail.tip);
					self->client->saber[saberNum].blade[rBladeNum].trail.lastTime = level.time;

					rBladeNum++;
				}

				rSaberNum++;
			}
			self->client->hasCurrentPosition = qtrue;

			self->client->ps.saberBlocked = BLOCKED_NONE;

			goto finalUpdate;
		}

		//reset it in case we used it for cycling before
		rSaberNum = rBladeNum = 0;

		if (self->client->ps.saberInFlight)
		{ //if saber is thrown then only do the standard stuff for the left hand saber
			if (!self->client->ps.saberEntityNum)
			{ //however, if saber is not in flight but rather knocked away, our left saber is off, and thus we may do nothing.
				rSaberNum = 1;//was 2?
			}
			else
			{//thrown saber still in flight, so do damage
				rSaberNum = 0;//was 1?
			}
		}

		WP_SaberClearDamage();
		saberDoClashEffect = qfalse;

		//Now cycle through each saber and each blade on the saber and do damage traces.
		while (rSaberNum < MAX_SABERS)
		{
			if (!self->client->saber[rSaberNum].model[0])
			{
				rSaberNum++;
				continue;
			}

			/*
			if (rSaberNum == 0 && (self->client->ps.brokenLimbs & (1 << BROKENLIMB_RARM)))
			{ //don't do saber 0 is the right arm is broken
				rSaberNum++;
				continue;
			}
			*/
			//for now I'm keeping a broken right arm swingable, it will just look and act damaged
			//but still be useable
			
			if (rSaberNum == 1 && (self->client->ps.brokenLimbs & (1 << BROKENLIMB_LARM)))
			{ //don't to saber 1 if the left arm is broken
				break;
			}
			if (rSaberNum > 0 &&
					self->client->saber[1].model[0] &&
					self->client->ps.saberHolstered == 1 )
			{ //don't to saber 2 if it's off
				break;
			}
			rBladeNum = 0;
			while (rBladeNum < self->client->saber[rSaberNum].numBlades)
			{
				//update muzzle data for the blade
				VectorCopy(self->client->saber[rSaberNum].blade[rBladeNum].muzzlePoint, self->client->saber[rSaberNum].blade[rBladeNum].muzzlePointOld);
				VectorCopy(self->client->saber[rSaberNum].blade[rBladeNum].muzzleDir, self->client->saber[rSaberNum].blade[rBladeNum].muzzleDirOld);

				if ( rBladeNum > 0 //more than one blade
					&& (!self->client->saber[1].model[0])//not using dual blades
					&& self->client->saber[rSaberNum].numBlades > 1//using a multi-bladed saber
					&& self->client->ps.saberHolstered == 1 )//
				{ //don't to extra blades if they're off
					break;
				}
				//get the new data
				//then update the bolt pos/dir. rBladeNum corresponds to the bolt index because blade bolts are added in order.
				if ( rSaberNum == 0 && self->client->ps.saberInFlight )
				{
					if ( !self->client->ps.saberEntityNum )
					{//dropped it... shouldn't get here, but...
						//assert(0);
						//FIXME: It's getting here a lot actually....
						rSaberNum++;
						rBladeNum = 0;
						continue;
					}
					else
					{
						gentity_t *saberEnt = &g_entities[self->client->ps.saberEntityNum];
						vec3_t saberOrg, saberAngles;
						if ( !saberEnt 
							|| !saberEnt->inuse
							|| !saberEnt->ghoul2 )
						{//wtf?
							rSaberNum++;
							rBladeNum = 0;
							continue;
						}
						if ( saberent->s.saberInFlight )
						{//spinning
							BG_EvaluateTrajectory( &saberEnt->s.pos, level.time+50, saberOrg );
							BG_EvaluateTrajectory( &saberEnt->s.apos, level.time+50, saberAngles );
						}
						else
						{//coming right back
							vec3_t saberDir;
							BG_EvaluateTrajectory( &saberEnt->s.pos, level.time, saberOrg );
							VectorSubtract( self->r.currentOrigin, saberOrg, saberDir );
							vectoangles( saberDir, saberAngles );
						}
						trap->G2API_GetBoltMatrix(saberEnt->ghoul2, 0, rBladeNum, &boltMatrix, saberAngles, saberOrg, level.time, NULL, self->modelScale);
						BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, self->client->saber[rSaberNum].blade[rBladeNum].muzzlePoint);
						BG_GiveMeVectorFromMatrix(&boltMatrix, NEGATIVE_Y, self->client->saber[rSaberNum].blade[rBladeNum].muzzleDir);
						VectorCopy( self->client->saber[rSaberNum].blade[rBladeNum].muzzlePoint, boltOrigin );
						VectorMA( boltOrigin, self->client->saber[rSaberNum].blade[rBladeNum].lengthMax, self->client->saber[rSaberNum].blade[rBladeNum].muzzleDir, end );
					}

				}
				else
				{
					trap->G2API_GetBoltMatrix(self->ghoul2, rSaberNum+1, rBladeNum, &boltMatrix, properAngles, properOrigin, level.time, NULL, self->modelScale);
					BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, self->client->saber[rSaberNum].blade[rBladeNum].muzzlePoint);
					BG_GiveMeVectorFromMatrix(&boltMatrix, NEGATIVE_Y, self->client->saber[rSaberNum].blade[rBladeNum].muzzleDir);
					VectorCopy( self->client->saber[rSaberNum].blade[rBladeNum].muzzlePoint, boltOrigin );
					VectorMA( boltOrigin, self->client->saber[rSaberNum].blade[rBladeNum].lengthMax, self->client->saber[rSaberNum].blade[rBladeNum].muzzleDir, end );
				}

				self->client->saber[rSaberNum].blade[rBladeNum].storageTime = level.time;

				G_SPSaberDamageTraceLerped( self, rSaberNum, rBladeNum, boltOrigin, end, (MASK_PLAYERSOLID|CONTENTS_LIGHTSABER|MASK_SHOT) );

				VectorCopy(boltOrigin, self->client->saber[rSaberNum].blade[rBladeNum].trail.base);
				VectorCopy(end, self->client->saber[rSaberNum].blade[rBladeNum].trail.tip);
				self->client->saber[rSaberNum].blade[rBladeNum].trail.lastTime = level.time;
				//VectorCopy(boltOrigin, self->client->lastSaberBase);
				//VectorCopy(end, self->client->lastSaberTip);
				self->client->hasCurrentPosition = qtrue;

				//do hit effects
				WP_SaberDoHit( self, rSaberNum, rBladeNum );
				WP_SaberDoClash( self, rSaberNum, rBladeNum );

				rBladeNum++;
			}

			rSaberNum++;
		}
		
		WP_SaberApplyDamage( self );
		//NOTE: doing one call like this after the 2 loops above is a bit cheaper, tempentity-wise... but won't use the correct saber and blade numbers...
		//now actually go through and apply all the damage we did
		//WP_SaberDoHit( self, 0, 0 );
		//WP_SaberDoClash( self, 0, 0 );

		if (mySaber && mySaber->inuse)
		{
			trap->LinkEntity((sharedEntity_t *)mySaber);
		}

		if (!self->client->ps.saberInFlight)
		{
			self->client->ps.saberEntityState = 0;
		}
	}
finalUpdate:
	if (clientOverride)
	{ //if we get the client instance we don't even need to bother setting anims and stuff
		return;
	}

	G_UpdateClientAnims(self, animSpeedScale);
}

int WP_MissileBlockForBlock( int saberBlock )
{
	switch( saberBlock )
	{
	case BLOCKED_UPPER_RIGHT:
		return BLOCKED_UPPER_RIGHT_PROJ;
		break;
	case BLOCKED_UPPER_LEFT:
		return BLOCKED_UPPER_LEFT_PROJ;
		break;
	case BLOCKED_LOWER_RIGHT:
		return BLOCKED_LOWER_RIGHT_PROJ;
		break;
	case BLOCKED_LOWER_LEFT:
		return BLOCKED_LOWER_LEFT_PROJ;
		break;
	case BLOCKED_TOP:
		return BLOCKED_TOP_PROJ;
		break;
	}
	return saberBlock;
}

// Check and see if we're properly blocking stuff --eez
qboolean JKG_SaberBlockIsGood( gentity_t *self, int saberBlock )
{
	switch( saberBlock )
	{
		case BLOCKED_UPPER_RIGHT:
			// moving right = deduct 10 BP
			if ( self->client->pers.cmd.rightmove > 0 )
			{
				return qfalse;
			}
			break;
		case BLOCKED_UPPER_LEFT:
			// moving left = deduct 10 BP
			if ( self->client->pers.cmd.rightmove < 0 )
			{
				return qfalse;
			}
			break;
		/*case BLOCKED_TOP:
			// moving back = deduct 10 BP
			break;*/
		case BLOCKED_LOWER_LEFT:
			// moving left = deduct 10 BP
			if ( self->client->pers.cmd.rightmove < 0 )
			{
				return qfalse;
			}
			break;
		case BLOCKED_LOWER_RIGHT:
			// moving right = deduct 10 BP
			if ( self->client->pers.cmd.rightmove > 0 )
			{
				return qfalse;
			}
			break;
	}
	return qtrue;
}

qboolean JKG_PlayersMovingInOppositeDirections( int p1fwd, int p2fwd, int p1right, int p2right )
{
	// Returns true if players are moving in opposite directions --eez
	if( p1fwd != 0 )
	{
		if( (p2fwd < 0 && p1fwd < 0) || (p2fwd > 0 && p1fwd > 0) )
		{
			if( p1right < 0 && p2right < 0 )
			{
				return qfalse;
			}
			else if( p1right > 0 && p2right > 0 )
			{
				return qfalse;
			}
		}
	}
	
	if( p1right != 0 )
	{
		if( (p1right < 0 && p2right < 0) || (p1right > 0 && p2right > 0) )
		{
			return qfalse;
		}
	}
	return qtrue;
}

qboolean WP_SaberBlockNonRandom( gentity_t *self, gentity_t *other, vec3_t hitloc, qboolean missileBlock )
{
	vec3_t diff, fwdangles={0,0,0}, right;
	vec3_t clEye;
	float rightdot;
	float zdiff;
	int desiredBlocked;

	VectorCopy(self->client->ps.origin, clEye);
	clEye[2] += self->client->ps.viewheight;

	VectorSubtract( hitloc, clEye, diff );
	diff[2] = 0;
	VectorNormalize( diff );

	fwdangles[1] = self->client->ps.viewangles[1];
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff);
	zdiff = hitloc[2] - clEye[2];
	
	if ( zdiff > 0 )
	{
		if ( rightdot > 0.3 )
		{
			desiredBlocked = BLOCKED_UPPER_RIGHT;
		}
		else if ( rightdot < -0.3 )
		{
			desiredBlocked = BLOCKED_UPPER_LEFT;
		}
		else
		{
			desiredBlocked = BLOCKED_TOP;
		}
	}
	else if ( zdiff > -20 )//20 )
	{
		if ( zdiff < -10 )//30 )
		{//hmm, pretty low, but not low enough to use the low block, so we need to duck
			
		}
		if ( rightdot > 0.1 )
		{
			desiredBlocked = BLOCKED_UPPER_RIGHT;
		}
		else if ( rightdot < -0.1 )
		{
			desiredBlocked = BLOCKED_UPPER_LEFT;
		}
		else
		{
			desiredBlocked = BLOCKED_TOP;
		}
	}
	else
	{
		if ( rightdot >= 0 )
		{
			desiredBlocked = BLOCKED_LOWER_RIGHT;
		}
		else
		{
			desiredBlocked = BLOCKED_LOWER_LEFT;
		}
	}

	if ( missileBlock )
	{
		// handle missile block stuff here --eez

		if(self->s.eType == ET_PLAYER)
		{
			if(!(self->client->ps.saberActionFlags & (1 << SAF_PROJBLOCKING) ))
			{
				// Not projectile blocking? use a slow block --eez
				return qtrue;		// we blocked this
			}

			/*if((level.time - self->client->saberProjBlockTime) < 2000)
			{
				// don't do a fast autoblock if we just press the button, this prevents crazy annomalies from happening --eez
				self->client->ps.saberBlocked = self->client->ps.saberMove;
				return qtrue;		// we blocked this
			}*/
		}
		self->client->ps.saberBlocked = WP_MissileBlockForBlock( desiredBlocked );
		return qtrue;	// always block if we're dealing with projectiles
	}
	else
	{
		int BPneeded = JKG_GetBPNeededForBlock( self, other->client->ps.saberMove, other->client->ps.fd.saberAnimLevel );
		if(self->client->ps.blockPoints >= BPneeded)
		{
			/*if(BPneeded < 15)
			{
				other->client->ns.forcePower -= BPneeded-15;
			}*/
			self->client->ps.blockPoints -= BPneeded;
			self->client->ps.saberBlocked = desiredBlocked;
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}
	return qtrue;
}

// FIXME: remove
void WP_SaberBlock( gentity_t *playerent, vec3_t hitloc, qboolean missileBlock )
{
	vec3_t diff, fwdangles={0,0,0}, right;
	float rightdot;
	float zdiff;

	VectorSubtract(hitloc, playerent->client->ps.origin, diff);
	VectorNormalize(diff);

	fwdangles[1] = playerent->client->ps.viewangles[1];
	// Ultimately we might care if the shot was ahead or behind, but for now, just quadrant is fine.
	AngleVectors( fwdangles, NULL, right, NULL );

	rightdot = DotProduct(right, diff) + RandFloat(-0.2f,0.2f);
	zdiff = hitloc[2] - playerent->client->ps.origin[2] + Q_irand(-8,8);
	
	// Figure out what quadrant the block was in.
	if (zdiff > 24)
	{	// Attack from above
		if (Q_irand(0,1))
		{
			playerent->client->ps.saberBlocked = BLOCKED_TOP;
		}
		else
		{
			playerent->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
		}
	}
	else if (zdiff > 13)
	{	// The upper half has three viable blocks...
		if (rightdot > 0.25)
		{	// In the right quadrant...
			if (Q_irand(0,1))
			{
				playerent->client->ps.saberBlocked = BLOCKED_UPPER_LEFT;
			}
			else
			{
				playerent->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
			}
		}
		else
		{
			switch(Q_irand(0,3))
			{
			case 0:
				playerent->client->ps.saberBlocked = BLOCKED_UPPER_RIGHT;
				break;
			case 1:
			case 2:
				playerent->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
				break;
			case 3:
				playerent->client->ps.saberBlocked = BLOCKED_TOP;
				break;
			}
		}
	}
	else
	{	// The lower half is a bit iffy as far as block coverage.  Pick one of the "low" ones at random.
		if (Q_irand(0,1))
		{
			playerent->client->ps.saberBlocked = BLOCKED_LOWER_RIGHT;
		}
		else
		{
			playerent->client->ps.saberBlocked = BLOCKED_LOWER_LEFT;
		}
	}

	if ( missileBlock )
	{
		if(playerent->s.eType == ET_PLAYER)
		{
			if(!(playerent->client->ps.saberActionFlags & (1 << SAF_BLOCKING)))
			{
				return;
			}
		}
		playerent->client->ps.saberBlocked = WP_MissileBlockForBlock( playerent->client->ps.saberBlocked );
	}
}
// not toching the mouse :P under here we have the fuction there handle our block on proj blocking and blocking saber
// int? should be qboolean methinks
qboolean WP_SaberCanBlock(gentity_t *self, vec3_t point, int dflags, int mod, qboolean projectile, int attackStr)
{
	qboolean thrownSaber = qfalse;
	float blockFactor = 0;

	if (!self || !self->client || !point)
	{
		return qfalse;
	}

	if (attackStr == 999)
	{
		attackStr = 0;
		thrownSaber = qtrue;
	}

	if (BG_SaberInAttack(self->client->ps.saberMove))
	{
		return qfalse;
	}

	if (PM_InSaberAnim(self->client->ps.torsoAnim) && !self->client->ps.saberBlocked &&
		self->client->ps.saberMove != LS_READY && self->client->ps.saberMove != LS_NONE)
	{
		if ( self->client->ps.saberMove < LS_PARRY_UP || self->client->ps.saberMove > LS_REFLECT_LL )
		{
			return qfalse;
		}
	}

	if (PM_SaberInBrokenParry(self->client->ps.saberMove))
	{
		return qfalse;
	}

	if (!self->client->ps.saberEntityNum)
	{ //saber is knocked away
		return qfalse;
	}

	if (BG_SabersOff( &self->client->ps ))
	{
		return qfalse;
	}
	// here we start as a ET = Player blocking projectile 
	if ( self->s.eType == ET_PLAYER )
	{
		if( projectile )
		{// holding Block when pressing and use proj blocking to deflect shoots with
			if( !(self->client->ps.saberActionFlags & (1 << SAF_BLOCKING)) && !(self->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING ) ) )
			{
				// Forget it.
				return qfalse;
			}
		}
		else
		{// Repeat the blocking i guess ?
			if( !(self->client->ps.saberActionFlags & (1 << SAF_BLOCKING)) )
			{
				return qfalse;
			}
		}
	}

	if (self->client->ps.weapon != WP_SABER)
	{
		return qfalse;
	}

	if (self->client->ps.weaponstate == WEAPON_RAISING)
	{
		return qfalse;
	}

	if (self->client->ps.saberInFlight)
	{
		return qfalse;
	}

	if ((self->client->pers.cmd.buttons & BUTTON_ATTACK)/* &&
		(projectile || attackStr == FORCE_LEVEL_3)*/)
	{ //don't block when the player is trying to slash, if it's a projectile or he's doing a very strong attack
		return qfalse;
	}

	if (SaberAttacking(self))
	{ //attacking, can't block now
		return qfalse;
	}
	//Stoiss add BlockAnim			  //LS_READY	
	if (self->client->ps.saberMove != LS_READY &&
		!self->client->ps.saberBlocking &&
		!(self->client->ps.saberActionFlags & (1 << SAF_BLOCKING)))
	{
		return qfalse;
	}

	if (self->client->saberBlockDebounce >= level.time)
	{
		return qfalse;
	}

	if (self->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return qfalse;
	}

	blockFactor = 0.3f;

	if (thrownSaber)
	{
		blockFactor -= 0.25f;
	}

	if (attackStr)
	{ //blocking a saber, not a projectile.
		blockFactor -= 0.25f;
	}

	if (!InFront( point, self->client->ps.origin, self->client->ps.viewangles, 0.2f )) //orig 0.2f
	{
		return qfalse;
	}

	if (projectile)
	{
		// k, do force draining stuff now --eez
		if(self->s.eType == ET_PLAYER)
		{
			qboolean projBlockMode = self->client->ps.saberActionFlags & ( 1 << SAF_PROJBLOCKING );
			int forcePowerUsed = 10; // FIXME: make this gun-dependant

			// shouldn't be able to block period if we aren't in block mode --eez
			if( !(self->client->ps.saberActionFlags & (1 << SAF_BLOCKING)) && !(self->client->pers.cmd.buttons & BUTTON_IRONSIGHTS) )
			{
				return qfalse;
			}


			// modify force power used by how fast we activated projectile block mode, if we did --eez
			if( projBlockMode )
			{
				if((level.time - self->client->saberProjBlockTime) < 2000)
				{
					// good --eez
					if((level.time - self->client->saberProjBlockTime) < 1000)
					{
						// better --eez
						if((level.time - self->client->saberProjBlockTime) < 500)
						{
							// excellent --eez
							forcePowerUsed /= 5;
						}
						else
						{
							forcePowerUsed /= 3;
						}
					}
					else
					{
						forcePowerUsed /= 2;
					}
				}
			}

			if(self->client->ps.forcePower < forcePowerUsed)
			{
				self->client->ps.forcePower = 0;
				return qfalse;
			}
			else
			{
				self->client->ps.forcePower -= forcePowerUsed;
			}
			return qtrue;
		}
		else
		{
			// FIXME:
			// just block the shot. jedi NPCs should be tough eh? --eez
			WP_SaberBlockNonRandom(self, NULL, point, projectile);
		}
	}
	return qtrue;
}

//[SaberThrowSys]
void SaberBallisticsTouch(gentity_t *saberent, gentity_t *other, trace_t *trace)
{//touch function for sabers in ballistics mode
	gentity_t *saberOwn = &g_entities[saberent->r.ownerNum];

	if (other && other->s.number == saberent->r.ownerNum)
	{//racc - we hit our owner, just ignore it.
		return;
	}

	if(saberent->s.eFlags & EF_MISSILE_STICK)
	{
		trap->Print("this isn't good\n");
	}

	//knock the saber down after impact.
	saberKnockDown(saberent, saberOwn, other);
}
//This is the bounce count used for ballistic sabers.  We watch this value to know
//when to make the switch to a simple dropped saber.
#define BALLISTICSABER_BOUNCECOUNT 10


void SaberBallisticsThink(gentity_t *saberEnt)
{//think function for sabers in ballistics mode
	//G_RunObject(saberEnt);

	saberEnt->nextthink = level.time;

	if(saberEnt->s.eFlags & EF_MISSILE_STICK)
	{//we're stuck in something
		float ownerLen;
		vec3_t dir;
		gentity_t *saberOwner = &g_entities[saberEnt->r.ownerNum];

		//racc - Apprenently pos1 is the hand bolt.
		VectorSubtract(saberOwner->r.currentOrigin, saberEnt->r.currentOrigin, dir);

		ownerLen = VectorLength(dir);

		if (ownerLen <= 32)
		{//racc - picked up the saber.
			G_Sound( saberEnt, CHAN_AUTO, G_SoundIndex( "sound/weapons/saber/saber_catch.wav" ) );

			saberEnt->s.eFlags &= ~EF_MISSILE_STICK;
			saberReactivate(saberEnt, saberOwner);

			saberEnt->speed = 0;
			saberEnt->genericValue5 = 0;
			saberEnt->nextthink = level.time;

			saberEnt->r.contents = CONTENTS_LIGHTSABER;

			saberOwner->client->ps.saberInFlight = qfalse;
			saberOwner->client->ps.saberEntityState = 0;
			saberOwner->client->ps.saberCanThrow = qfalse;
			saberOwner->client->ps.saberThrowDelay = level.time + 300;

			saberEnt->touch = SaberGotHit;

			saberEnt->think = SaberUpdateSelf;
			saberEnt->genericValue5 = 0;
			saberEnt->nextthink = level.time + 50;
			WP_SaberRemoveG2Model( saberEnt );

			saberOwner->client->ps.saberEntityNum = saberOwner->client->saberStoredIndex;
			return;
		}
		// Commenting out for now until I figure out what to do about saber throw --eez
		/*else if ( saberOwner->client->saberKnockedTime < level.time && 
			((saberOwner->client->buttons & BUTTON_ALT_ATTACK && 
			saberOwner->client->ps.fd.forcePowerSelected == FP_SABERTHROW) 
			|| saberOwner->client->buttons & BUTTON_ATTACK
			|| saberOwner->client->ps.forceHandExtend == HANDEXTEND_SABERPULL) )
		{//we want to pull the saber back.
			// need an if player in front of in straight line
			// need to set it up so the thing wiggles before freeing
			//ForceThrow(saberOwner, qtrue); //play effect to unstick.
			saberEnt->s.eFlags &= ~EF_MISSILE_STICK;

			saberReactivate(saberEnt, saberOwner);

			saberEnt->touch = SaberGotHit;

			saberEnt->think = saberBackToOwner;
			saberEnt->speed = 0;
			saberEnt->genericValue5 = 0;
			saberEnt->nextthink = level.time;

			saberEnt->r.contents = CONTENTS_LIGHTSABER;
		}*/
		else if ((level.time - saberOwner->client->saberKnockedTime) > MAX_LEAVE_TIME)
		{//We left it too long.  Just have it turn off and fall to the ground. 
			VectorClear(saberEnt->s.pos.trDelta);
			VectorClear(saberEnt->s.apos.trDelta);
			saberEnt->speed = 0;
			//saberEnt->s.eFlags &= ~EF_MISSILE_STICK;
			saberKnockDown(saberEnt, saberOwner, saberOwner);
		}
	}
	else
	{//flying thru the air
		if(saberEnt->bounceCount != BALLISTICSABER_BOUNCECOUNT)
		{//we've hit an object and bounced off it, go to dead saber mode
			gentity_t *saberOwn = &g_entities[saberEnt->r.ownerNum];
			saberKnockDown(saberEnt, saberOwn, saberOwn);
		}
		//Bug fix: sabers were falling through certain solid objects
		else
			G_RunObject(saberEnt);
	}
}

void thrownSaberBallistics(gentity_t *saberEnt, gentity_t *saberOwn, qboolean stuck)
{//this function converts the saber from thrown saber that's being held on course by
	//the force into a saber that's just ballastically moving.

	if(stuck)
	{//lightsaber is stuck in something, just hang there.
		VectorClear(saberEnt->s.pos.trDelta);
		VectorClear(saberEnt->s.apos.trDelta);

		//set the sticky eflag
		saberEnt->s.eFlags = EF_MISSILE_STICK;

		//don't move at all.
		saberEnt->s.pos.trType = TR_STATIONARY;
		saberEnt->s.apos.trType = TR_STATIONARY;

		//set the force retrieve timer so we'll know when to pull it out 
		//of the object with the force/turn off saber.
		saberOwn->client->saberKnockedTime = level.time + SABER_RETRIEVE_DELAY;

		//no more loop sound!
		saberEnt->s.loopSound = saberOwn->client->saber[0].soundLoop;
		saberEnt->s.loopIsSoundset = qfalse;

		//don't actually bounce on impact with walls.
		saberEnt->bounceCount = 0;
	}
	else
	{//otherwise, just move by normal ballistic physics
		//spin just like we were in our saber throw.
		saberEnt->s.apos.trType = TR_LINEAR;
		saberEnt->s.apos.trDelta[0] = 0;
		saberEnt->s.apos.trDelta[1] = 800;
		saberEnt->s.apos.trDelta[2] = 0;

		//but now gravity has an effect.
		saberEnt->s.pos.trType = TR_GRAVITY;

		//clear the entity flags
		saberEnt->s.eFlags = 0;

		saberEnt->bounceCount = BALLISTICSABER_BOUNCECOUNT;
	}
	
	//set up for saber style bouncing
	saberEnt->flags |= FL_BOUNCE_HALF;


	//set traqec timers and initial positions
	saberEnt->s.apos.trTime = level.time;
	VectorCopy(saberEnt->r.currentAngles, saberEnt->s.apos.trBase);

	saberEnt->s.pos.trTime = level.time;
	VectorCopy(saberEnt->r.currentOrigin, saberEnt->s.pos.trBase);

	//let the player know that they've lost control of the saber.
	saberOwn->client->ps.saberEntityNum = 0;

	//set the approprate function pointer stuff
	saberEnt->think = SaberBallisticsThink;
	saberEnt->touch = SaberBallisticsTouch;
	saberEnt->nextthink = level.time + FRAMETIME;

	trap->LinkEntity((sharedEntity_t *)saberEnt);

	//add the saber model to our gentity ghoul2 instance
	WP_SaberAddG2Model( saberEnt, saberOwn->client->saber[0].model, 
							saberOwn->client->saber[0].skin );

	saberEnt->s.modelGhoul2 = 1;
	saberEnt->s.g2radius = 20;

	saberEnt->s.eType = ET_MISSILE;
	saberEnt->s.weapon = WP_SABER;
}
//[/SaberThrowSys]

void JKG_NetworkSaberCrystals( playerState_t *ps, std::size_t invId, int weaponId )
{
	// I'm being extra cautious with these checks for a reason...we originally networked stuff
	// going by the weaponId because the stuff sent from the ACI was very inaccurate at times.

	// Network what saber crystals we have
	if( !ps )
		return;

	if( ps->weaponstate == WEAPON_DROPPING )
	{
		// Semi-hack, we want to play the sound when we're turning off the saber.
		if( ps->clientNum < 0 || ps->clientNum >= MAX_CLIENTS )
		{
			return;
		}

		gentity_t *ent2 = &g_entities[ps->clientNum];

		if( ent2->client->didSaberOffSound )
			return;
		else if( ent2->client->saber[0].soundOff && !ps->saberHolstered )
		{
			// If we're switching to another saber, we use the quick sound. If we're switching to a gun, use the slow sound.
			int weapon, variation;
			gentity_t *te = G_TempEntity( ent2->r.currentOrigin, EV_SABER_HOLSTER );
			te->s.eventParm = ps->clientNum;
			if( BG_GetWeaponByIndex( weaponId, &weapon, &variation ) )
			{
				if( weapon == WP_SABER )
				{
					te->s.eFlags = 1;
				}
			}
			ent2->client->didSaberOffSound = true;
			return;
		}
	}
	else if( ps->weaponstate != WEAPON_RAISING )
		return;

	int entNum = ps->clientNum;

	if( entNum < 0 || entNum >= MAX_CLIENTS || invId < 0 )
	{
		// GTFO maron
		return;
	}

	gentity_t *ent = &g_entities[entNum];
	ent->client->didSaberOffSound = false;
	if( invId >= ent->inventory->size() )
	{
		// Not quite valid since it's bigger.
		return;
	}

	itemInstance_t *itm = &(*ent->inventory)[invId];
	if( !itm->id )
	{
		// NOPE.avi
		return;
	}

	if( itm->id->itemType != ITEM_WEAPON )
	{
		// NOPE_2.0_.avi
		return;
	}

	if( itm->id->weaponData.weapon != WP_SABER )
	{
		// still-NOPE.avi
		return;
	}

	if( itm->id->weaponData.varID != weaponId )
	{
		// ultimate way of checking to be ABSOLUTELY SURE --eez
		return;
	}

	// ok go
	//ps->saberCrystal[0] = itm->calc1;	// FIXME: need stuff for akimbo...
}

void JKG_DoubleCheckWeaponChange( usercmd_t *cmd, playerState_t *ps )
{
	std::size_t invSel = cmd->invensel;

	gentity_t *ent = &g_entities[ps->clientNum];

	if( ent->inventory->size() <= invSel || invSel < 0 )
	{
		cmd->weapon = 0;
	} else {
		cmd->weapon = (*ent->inventory)[invSel].id->weaponData.varID;
	}
}
