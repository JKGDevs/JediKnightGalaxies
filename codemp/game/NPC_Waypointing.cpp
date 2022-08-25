#include "g_local.h"
#include "ai_main.h"

#ifdef __DOMINANCE_NPC__

extern gNPC_t		*NPCInfo;
extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int FindCloseList(int wpNum);
extern void ClearRoute( int Route[MAX_WPARRAY_SIZE] );
extern void AddtoRoute( int wpNum, int Route[MAX_WPARRAY_SIZE] );
extern qboolean OpenListEmpty(void);
extern int FindOpenList(int wpNum);
extern void AddCloseList( int openListpos );
extern void RemoveFirstOpenList( void );
extern float VectorDistanceNoHeight ( vec3_t v1, vec3_t v2 );
extern qboolean NPC_FacePosition( vec3_t position, qboolean doPitch );
extern qboolean NPC_Humanoid_ClearPathToSpot( vec3_t dest, int impactEntNum );

extern void NPC_SetAnim(gentity_t *ent, int setAnimParts, int anim, int setAnimFlags);
extern qboolean PM_InKnockDown( playerState_t *ps );

extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);
extern int ASTAR_FindPath(int from, int to, int *pathlist);
extern int GetNearestWP(vec3_t org, int badwp);

extern qboolean JKG_CheckRoutingFrom( int wp );
extern qboolean JKG_CheckBelowWaypoint( int wp );

extern qboolean InFOV3( vec3_t spot, vec3_t from, vec3_t fromAngles, int hFOV, int vFOV );
extern void G_UcmdMoveForDir( gentity_t *self, usercmd_t *cmd, vec3_t dir );
extern qboolean DOM_Jedi_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );
extern void PM_SetPMViewAngle(playerState_t *ps, vec3_t angle, usercmd_t *ucmd);

extern visibility_t NPC_CheckVisibility ( gentity_t *ent, int flags );
extern gentity_t *NPC_PickEnemyExt( qboolean checkAlerts );
extern void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime );

extern int DOM_GetRandomCloseWP(vec3_t org, int badwp, int unused);

//extern qboolean NPC_IsCoverpointFor ( int thisWP, int forWP );
extern qboolean NPC_IsCoverpointFor ( int thisWP, gentity_t *enemy );

#define CHECK_PVS		1
#define CHECK_360		2
#define CHECK_FOV		4
#define CHECK_SHOOT		8
#define CHECK_VISRANGE	16

// Conversations...
extern void NPC_NPCConversation();
extern void NPC_FindConversationPartner();
extern void NPC_StormTrooperConversation();

extern int			num_cover_spots;
extern int			cover_nodes[MAX_WPARRAY_SIZE];

extern gentity_t *NPC;
extern usercmd_t ucmd;

const char* NPCObjectiveList[] = {
	// True goals, multiple copies to make them more common...
	"misc_control_point",
	"flag",

	// Useable entities...
	"emplaced_gun",
	"misc_turret",
	"misc_turretG2",
	"misc_weapon_shooter",
	"misc_security_panel",
	"misc_ammo_floor_unit",
	"misc_shield_floor_unit",
	"misc_model_shield_power_converter",
	"misc_model_ammo_power_converter",
	"misc_model_health_power_converter",
	"func_usable",
	"func_button",

	// Lifts, doors, etc...
	"func_plat",
	"func_door",
	"func_useable",

	"trigger_door",
	"trigger_multiple",
	"target_speaker",
	"target_scriptrunner",
	"fx_runner",

	// Nav goals...
	"item_botroam",
	"point_combat",
	"NPC_goal",
	"waypoint_navgoal",
	"waypoint_navgoal_8",
	"waypoint_navgoal_4",
	"waypoint_navgoal_2",
	"waypoint_navgoal_1",
	"misc_siege_item",
	"info_siege_objective",

	// Ammo, guns, etc...
	"misc_model_gun_rack",
	"misc_model_ammo_rack",
	"ammo_blaster",
	"ammo_all",
	"ammo_thermal",
	"ammo_tripmine",
	"ammo_detpack",
	"ammo_force",
	"ammo_blaster",
	"ammo_powercell",
	"ammo_metallic_bolts",
	"ammo_rockets",
	"ammo_all",
	"weapon_stun_baton", 
	"weapon_melee", 
	"weapon_saber", 
	"weapon_bryar_pistol", 
	"weapon_bryar_pistol_old", 
	"weapon_blaster_pistol", 
	"weapon_concussion_rifle", 
	"weapon_bryar_pistol", 
	"weapon_blaster", 
	"weapon_disruptor",
	"weapon_bowcaster",
	"weapon_repeater", 
	"weapon_demp2", 
	"weapon_flechette", 
	"weapon_rocket_launcher",
	"weapon_thermal",
	"weapon_trip_mine", 
	"weapon_det_pack", 
	"weapon_emplaced", 
	"weapon_turretwp", 

	// Players...
	"player",

	"",
};

int NPCObjectives[MAX_GENTITIES];
int num_NPC_objectives = 0;

int next_goal_update = -1;

void Update_NPC_Goal_Lists ( void )
{// Create arrays containing up to date valid goal entity numbers...
	int loop = 0;

	// Initialize the max values...
	num_NPC_objectives = 0;

	for (loop = 0;loop < MAX_GENTITIES;loop++)
	{
		int nameNum = 0;

		for (nameNum = 0; NPCObjectiveList[nameNum][0] != '\0' ;nameNum++)
		{
			if (!Q_stricmp( g_entities[loop].classname, NPCObjectiveList[nameNum] ))
			{
				NPCObjectives[num_NPC_objectives] = loop;
				num_NPC_objectives++;
				break;
			}
		}
	}

	num_NPC_objectives--;

	if (num_NPC_objectives < 0)
		num_NPC_objectives = 0;

	trap->Print("NPC WAYPOINTING DEBUG: There are currently %i NPC objectives.\n", num_NPC_objectives);
}

int NPC_Find_Goal_EntityNum ( int ignoreEnt, int ignoreEnt2, vec3_t current_org, int teamNum )
{// Return standard goals only for soldiers...
	if (next_goal_update < level.time)
	{// Update new goal lists.. Unique1
		Update_NPC_Goal_Lists();
		next_goal_update = level.time + 10000;
	}

	if (num_NPC_objectives > 0)
	{
		int choice = NPCObjectives[Q_irand(0, num_NPC_objectives)];
	
		if (choice != ignoreEnt && choice != ignoreEnt2)
			return choice;
	}

	return -1;
}

extern qboolean JKG_CheckBelowWaypoint( int wp );
extern qboolean JKG_CheckRoutingFrom( int wp );

int NPC_FindGoal( gentity_t *NPC )
{
	/*
	int wp = -1;
	gentity_t *ent = NULL;

	ent = &g_entities[NPC_Find_Goal_EntityNum( -1, -1, NPC->r.currentOrigin, NPC->s.teamowner )];

	if (!ent || ent->s.teamowner == NPC->s.teamowner)
	{
		//trap->Print("Failed to find a goal entity!\n");
		//return -1;

		return irand(0, gWPNum-1); // UQ1: Will try using random waypoints...
	}

	//trap->Print("NPC_FindGoal: Found a goal entity %s (%i).\n", ent->classname, ent->s.number);

	wp = DOM_GetBestWaypoint(ent->s.origin, NPC->s.number, -1);

	return wp;
	*/




	// UQ1: Will just use random waypoints (wanderring) for now - as our maps have few entities to be used as goals...
	//return irand(0, gWPNum-1); // UQ1: Will try using random waypoints...
	



	/*int waypoint = irand(0, gWPNum-1);
	int tries = 0;

	while (gWPArray[waypoint]->inuse == qfalse || !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint ))
	{
		gWPArray[waypoint]->inuse = qfalse; // set it bad!

		if (tries >= 2)
		{
			return -1; // Try again on next check...
		}

		// Find a new one... This is probably a bad waypoint...
		waypoint = irand(0, gWPNum-1);
		tries++;
	}*/

	int waypoint = irand(0, gWPNum-1);

	if (gWPArray[waypoint]->inuse == qfalse /*|| !JKG_CheckBelowWaypoint(waypoint) || !JKG_CheckRoutingFrom( waypoint )*/)
	{
		gWPArray[waypoint]->inuse = qfalse; // set it bad!
		return -1; // Try again on next check...
	}

	return waypoint;
}

extern gentity_t *WARZONE_FindGoalForTeam( int TEAM );

int NPC_FindWarzoneGoal( gentity_t *NPC )
{
	int waypoint = -1;
	gentity_t *goal = WARZONE_FindGoalForTeam( NPC->client->playerTeam );

	if (!goal) return -1;

	waypoint = DOM_GetRandomCloseWP(goal->s.origin, NPC->s.number, -1);

	if (waypoint <= 0 || gWPArray[waypoint]->inuse == qfalse)
	{
		return -1; // Try again on next check...
	}

	return waypoint;
}

#ifndef WAYPOINT_NONE
#define WAYPOINT_NONE -1
#endif //WAYPOINT_NONE

#define PATH_OK 0
#define PATH_BLOCKED 1

typedef struct wpneighborInfo_s
{
	int flags;
} wpneighborInfo_t;

typedef struct wpobjectinfo_s
{
	wpneighborInfo_t neighbors[MAX_NEIGHBOR_SIZE];
} wpobjectInfo_t;

wpobjectInfo_t *gWPArrayInfo[MAX_WPARRAY_SIZE];

qboolean WPFlagsInitialized = qfalse;

void InitWPLinkFlags ( void )
{// To be called externally to block bad points ingame...
	int wp = 0;
	int neighbor = 0;

	WPFlagsInitialized = qtrue;

	for (wp = 0; wp < gWPNum;wp++)
	{
		for (neighbor = 0; neighbor < MAX_NEIGHBOR_SIZE;neighbor++)
		{
			gWPArrayInfo[wp] = (wpobjectInfo_t *)malloc(sizeof(wpobjectInfo_t));
			gWPArrayInfo[wp]->neighbors[neighbor].flags = PATH_OK;
		}
	}
}

void MarkWPLinkBad ( gentity_t *NPC, int wp )
{// To be called externally to block bad points ingame...
	if (!WPFlagsInitialized)
		InitWPLinkFlags();
}

//Find the ideal (shortest) route between the start wp and the end wp
//badwp is for situations where you need to recalc a path when you dynamically discover
//that a wp is bad (door locked, blocked, etc).
//doRoute = actually set botRoute
//float FindIdealPathtoWP(bot_state_t *bs, int start, int end, int badwp, bot_route_t Route)

#define NODE_INVALID -1
#define MAX_NODELINKS 32
#define MAX_PATHLIST_NODES MAX_WPARRAY_SIZE //4096

extern int BOT_GetFCost ( gentity_t *bot, int to, int num, int parentNum, float *gcost );
extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);

/*
===========================================================================
UpdategWPArray
Called every frame to update the unreachable/objective flags for gWPArray,
so that the NPCs are aware an obstruction has been removed, etc
===========================================================================
*/
void UpdategWPArray( void )
{
	int i, j;

	for (i = 0; i < gWPNum; i++)		//loop through all the gWPArray
	{
		//loop through all the links from this node
		for (j = 0; j < gWPArray[i]->neighbornum; j++)
		{
			if (gWPArrayInfo[i]->neighbors[j].flags & PATH_BLOCKED)	//if the path is blocked
			{
				trace_t tr;
				vec3_t start, end;

				VectorCopy(gWPArray[i]->origin, start);
				VectorCopy(gWPArray[gWPArray[i]->neighbors[j].num]->origin, end);
				
				//do a trace to see if there is still anything blocking the path
				trap->Trace(&tr, start, NULL, NULL, end, ENTITYNUM_NONE, MASK_DEADSOLID, 0, 0, 0);

				//if the path is open, clear the 'blocked' flag
				if (tr.fraction == 1.0)
					gWPArrayInfo[i]->neighbors[j].flags &= ~PATH_BLOCKED;
			}
		}
	}
}

//find a given wpNum on the given route and return it's address.  return -1 if not on route.
//use wpNum = -1 to find the last wp on route.
int NPC_FindOnRoute( int wpNum, int Route[MAX_WPARRAY_SIZE] )
{
	int i;

	for( i=0; i < MAX_WPARRAY_SIZE; i++ )
	{
		//trap->Print("Checking %i = %i.\n", wpNum, Route[i]);

		if( wpNum == Route[i] )
		{//Success!
			return i;
		}
	}

	//Couldn't find it
	return -1;
}


/*///////////////////////////////////////////////////
NPCGetNextNode
if the NPC has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

short int NPCGetNextNode(gentity_t *NPC)
{
	/*short*/ int node = WAYPOINT_NONE;
	int	temp = 0;

	//we should never call this in NPCSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (NPC->longTermGoal == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (NPC->pathsize <= 0)	//if the NPC is at the end of his path, this shouldn't have been called
	{
		NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	//node = NPC->pathlist[NPC->pathsize-1];	//pathlist is in reverse order
	//NPC->pathsize--;	//mark that we've moved another node
	temp = NPC_FindOnRoute( NPC->wpCurrent, NPC->pathlist );
	node = NPC->pathlist[temp-1];
	//trap->Print("Node found at pos %i in array.\n",temp);
	return node;
}

/*
===========================================================================
MyVisible
This is just a copy of the built-in 'visible' function
but accounting for the players' viewheight.  This lets the
bots see players slightly more often and in a more realistic way
===========================================================================
*/
qboolean MyVisible (gentity_t *self, gentity_t *other)
{
	vec3_t selfView, otherView;		//basically stores the locations of the players' eyes
	trace_t		tr;					//used in performing the collision info
	gentity_t	*traceEnt;			//holds the entity returned by the trace function

	if (other->s.eType != ET_NPC && other->s.eType != ET_PLAYER)
		return qfalse;

	if (!other->client)
		return qfalse;

	VectorCopy(self->r.currentOrigin, selfView);		//copy the bot's origin to this variable

	selfView[2] += self->client->ps.viewheight;			//add the bot's viewheight	

	VectorCopy(other->r.currentOrigin, otherView);		//copy the target's origin

	otherView[2] += other->client->ps.viewheight;		//add the target's viewheight

	//check if a shot from the bot's viewheight to the player's viewheight would hit
	trap->Trace (&tr, selfView, NULL, NULL, otherView, self->s.number, MASK_SHOT, 0, 0, 0);

	traceEnt = &g_entities[tr.entityNum];		//set traceEnt to the entity the shot would hit

	if (traceEnt == other)		//if it would hit the player, return true
	{
		return qtrue;
	}

	return qfalse;				//it would hit something other than the player (or nothing) so return false

}

void NPC_FixBotWaypointNeighbors ( void )
{// Because JKA's waypoint neighbors don't include the nodes above and below... Grrr...
	int i = 0;

	if (gWPNum <= 0)
		return;

	trap->Print("^1*** ^3DominancE^5: Repairing waypoint database...\n");

	for (i = 0; i < gWPNum && gWPArray[i]->neighbornum < 32; i++)
	{
		vec3_t eye;

		VectorCopy(gWPArray[i]->origin, eye);
		eye[2]+=32;

		if (i - 1 < 0)
		{
			//if (OrgVisible(eye, gWPArray[i+1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i+1;
			}
		}
		else if (i + 1 >= gWPNum)
		{
			//if (OrgVisible(eye, gWPArray[i-1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i-1;
			}
		}
		else
		{
			//if (OrgVisible(eye, gWPArray[i+1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i+1;
			}
			
			//if (OrgVisible(eye, gWPArray[i-1]->origin, ENTITYNUM_NONE))
			{
				gWPArray[i]->neighbornum++;
				gWPArray[i]->neighbors[gWPArray[i]->neighbornum].num = i-1;
			}
		}
	}

	trap->Print("^1*** ^3DominancE^5: Waypoint database repaired...\n");
}

//===========================================================================
// Routine      : NPC_MOVEMENT_ReachableBy
// Description  : Determine if a blocked goal vector is reachable.. Returns how to get there...
#define NOT_REACHABLE -1
#define REACHABLE 0
#define REACHABLE_JUMP 1
#define REACHABLE_DUCK 2
#define REACHABLE_STRAFE_LEFT 3
#define REACHABLE_STRAFE_RIGHT 4

static vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
static vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

int NPC_MOVEMENT_ReachableBy(gentity_t *NPC, vec3_t goal)
{
    trace_t trace;
    vec3_t v, v2, eyes, mins, maxs, Org, forward, right, up, rightOrigin, leftOrigin;

	// First check direct movement...
	VectorCopy(NPC->r.currentOrigin, Org);

    VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	trap->Trace(&trace, Org, v, NPC->r.maxs, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE, 0, 0, 0);

	//if (trace.entityNum > 0)
	//	trap->Print("Hit entity %i (%s).\n", trace.entityNum, g_entities[trace.entityNum].classname);

	//if (g_entities[trace.entityNum].s.eType == ET_NPC)
	//	trap->Print("Hit an NPC!\n");

    if (trace.fraction == 1.0) {
        return REACHABLE; // Yes we can see it
	}

	// Now look for jump access...
	VectorAdd( Org, playerMins, mins );
	VectorAdd( Org, playerMaxs, maxs );

	VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	VectorCopy(Org, eyes);
	eyes[2]+=32;
	trap->Trace(&trace, eyes, v, NPC->r.maxs, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE, 0, 0, 0);
    if (trace.fraction == 1.0) {
        return REACHABLE_JUMP; // Yes we can see it
	}

	// Now look for crouch access...
	//CROUCH_VIEWHEIGHT ??
	VectorCopy(NPC->r.mins,v);
    v[2] += 18; // Stepsize
	VectorCopy(NPC->r.maxs,v2);
    v2[2] *= 0.5; // Stepsize
	trap->Trace(&trace, Org, v, v2, goal, NPC->s.number, MASK_SOLID|MASK_OPAQUE, 0, 0, 0);
    if (trace.fraction == 1.0) {
        return REACHABLE_DUCK; // Yes we can see it
	}

	// Now look for strafe access... Left or Right...
	AngleVectors( NPC->client->ps.viewangles, forward, right, up );
	
	VectorMA( NPC->r.currentOrigin, 64, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -64, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 201;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 201;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	// And strafing a bit further away...
	VectorMA( NPC->r.currentOrigin, 128, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -128, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 401;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 401;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	// And strafing a bit further away...
	VectorMA( NPC->r.currentOrigin, 192, right, rightOrigin );
	VectorMA( NPC->r.currentOrigin, -192, right, leftOrigin );

	if (OrgVisible(leftOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_left_timer = level.time + 601;
		VectorCopy(leftOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_LEFT; // Yes we can see it
	}

	if (OrgVisible(rightOrigin, gWPArray[NPC->wpCurrent]->origin, NPC->s.number))
	{
		NPC->bot_strafe_right_timer = level.time + 601;
		VectorCopy(rightOrigin, NPC->bot_strafe_target_position);
		return REACHABLE_STRAFE_RIGHT; // Yes we can see it
	}

	return NOT_REACHABLE;
}

extern int CheckForFunc(vec3_t org, int ignore);

qboolean NPC_WaitForFunc ( gentity_t *NPC )
{
	gentity_t *test = NULL;
	vec3_t size, center, pos1, pos2, origin;
	int i;

	for (i = 0;i < MAX_GENTITIES;i++)
	{
		test = &g_entities[i];

		if (!test)
			continue;

		if (Q_stricmp( "func_plat", test->classname ) &&  Q_stricmp( "func_door", test->classname ))
			continue;

		VectorCopy( NPC->r.currentOrigin, origin );

		//find center, pos1, pos2
		if ( VectorCompare( vec3_origin, test->s.origin ) )
		{//does not have an origin brush, so pos1 & pos2 are relative to world origin, need to calc center
			VectorSubtract( test->r.absmax, test->r.absmin, size );
			VectorMA( test->r.absmin, 0.5, size, center );
			if ( (test->spawnflags&1) && test->moverState == MOVER_POS1 )
			{//if at pos1 and started open, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
				VectorSubtract( center, test->pos1, center );
			}
			else if ( !(test->spawnflags&1) && test->moverState == MOVER_POS2 )
			{//if at pos2, make sure we get the center where it *started* because we're going to add back in the relative values pos1 and pos2
				VectorSubtract( center, test->pos2, center );
			}
			VectorAdd( center, test->pos1, pos1 );
			VectorAdd( center, test->pos2, pos2 );
		}
		else
		{//actually has an origin, pos1 and pos2 are absolute
			VectorCopy( test->r.currentOrigin, center );
			VectorCopy( test->pos1, pos1 );
			VectorCopy( test->pos2, pos2 );
		}

		if(Distance(origin, center) > 256/*192*//*400*/)
		{//too far away
			continue;
		}

		if (NPC->npc_mover_start_pos == MOVER_POS1 && test->moverState == MOVER_POS2)
		{// Ready to get off the lift...
			return qfalse;
		}

		if (NPC->npc_mover_start_pos == MOVER_POS2 && test->moverState == MOVER_POS1)
		{// Ready to get off the lift...
			return qfalse;
		}

		if (test->moverState == MOVER_1TO2 || test->moverState == MOVER_2TO1)
		{// If it's moving, then always wait!
			return qtrue;
		}

		if ( Distance( pos1, NPC->r.currentOrigin ) < Distance( pos2, NPC->r.currentOrigin ) )
		{//pos1 is closer
			if ( test->moverState == MOVER_POS1 )
			{//at the closest pos
				return qtrue;
			}
		}
		else
		{//pos2 is closer
			if ( test->moverState == MOVER_POS2 )
			{//at closest pos
				return qtrue;
			}
		}
		//we have a winner!
		break;
	}

	if (test && NPC->npc_mover_start_pos < 0)
	{// Mark the position we got on the mover at!
		NPC->npc_mover_start_pos = test->moverState;
	}
	else
	{
		NPC->npc_mover_start_pos = -1;
	}

	return qfalse;
}

/*///////////////////////////////////////////////////
NPC_GetNextNode
if the bot has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

int NPC_GetNextNode(gentity_t *NPC)
{
	short int node = WAYPOINT_NONE;

	//we should never call this in BOTSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (NPC->longTermGoal == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (NPC->pathsize <= 0)	//if the bot is at the end of his path, this shouldn't have been called
	{
		//NPC->longTermGoal = WAYPOINT_NONE;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	node = NPC->pathlist[NPC->pathsize-1];	//pathlist is in reverse order
	NPC->pathsize--;	//mark that we've moved another node

	if (NPC->pathsize <= 0)
	{
		if (NPC->wpCurrent < 0)
		{
			NPC->wpCurrent = NPC->longTermGoal;
		}
		else
		{
			node = NPC->longTermGoal;
		}
	}
	return node;
}

void NPC_SelectMoveAnimation()
{
	if (NPC->client->ps.crouchheight <= 0)
		NPC->client->ps.crouchheight = CROUCH_MAXS_2;

	if (NPC->client->ps.standheight <= 0)
		NPC->client->ps.standheight = DEFAULT_MAXS_2;

	/*
	if (NPC->client->ps.pm_flags & PMF_DUCKED && NPC->r.maxs[2] > NPC->client->ps.crouchheight)
	{
		VectorCopy(playerMins, NPC->r.mins);
		VectorCopy(playerMaxs, NPC->r.maxs);

		NPC->r.maxs[2] = NPC->client->ps.crouchheight;
		trap->LinkEntity(NPC);
	}
	else if (!(NPC->client->ps.pm_flags & PMF_DUCKED) && NPC->r.maxs[2] < NPC->client->ps.standheight)
	{
		VectorCopy(playerMins, NPC->r.mins);
		VectorCopy(playerMaxs, NPC->r.maxs);

		NPC->r.maxs[2] = NPC->client->ps.standheight;
		trap->LinkEntity(NPC);
	}
	*/

	if (VectorLength(NPC->client->ps.velocity) < 4)
	{// Standing still...
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else if ( NPC->client->ps.eFlags2 & EF2_USE_ALT_ANIM )
		{//holding someone
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND4, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
		else if ( NPC->client->ps.eFlags2 & EF2_ALERTED )
		{//have an enemy or have had one since we spawned
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
		else
		{//just stand there
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}
	else if (VectorLength(NPC->client->ps.velocity) <= 100/*64*/)
	{// Use walking anims..
		if (ucmd.forwardmove < 0)
		{
			if (NPC->client->ps.pm_flags & PMF_DUCKED)
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALKBACK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else if (NPC->client->ps.weapon == WP_SABER)
			{// Walk with saber...
				NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_WALKBACK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
				//NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_NORMAL );
			}
			else
			{// Standard walk anim..
				NPC_SetAnim( NPC, SETANIM_LEGS, BOTH_WALKBACK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
				NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			}
		}
		else
		{
			if (NPC->client->ps.pm_flags & PMF_DUCKED)
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else if (NPC->client->ps.weapon == WP_SABER)
			{// Walk with saber...
				NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_WALK1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
				//NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_NORMAL );
			}
			else
			{// Standard walk anim..
				NPC_SetAnim( NPC, SETANIM_LEGS, BOTH_WALK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
				NPC_SetAnim( NPC, SETANIM_TORSO, TORSO_WEAPONREADY3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			}
		}
	}
	else if ( NPC->client->ps.eFlags2 & EF2_USE_ALT_ANIM )
	{//full on run, on all fours
		if (NPC->client->ps.pm_flags & PMF_DUCKED)
		{
			NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		}
		else 
		{
			NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_RUN1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
		}
	}
	else
	{//regular, upright run
		if (ucmd.forwardmove < 0)
		{
			if (NPC->client->ps.pm_flags & PMF_DUCKED)
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALKBACK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else 
			{
				NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_RUNBACK2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			}
		}
		else
		{
			if (NPC->client->ps.pm_flags & PMF_DUCKED)
			{
				NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_CROUCH1WALK, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
			}
			else 
			{
				NPC_SetAnim( NPC, SETANIM_BOTH, BOTH_RUN2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
			}
		}
	}

	NPC->client->ps.torsoTimer = 200;
	NPC->client->ps.legsTimer = 200;
}

void NPC_Run()
{
	//NPC->client->pers.cmd.forwardmove = 127;
	ucmd.forwardmove = 127;
	NPC->client->ps.speed = NPCInfo->stats.runSpeed;
	NPC_SelectMoveAnimation();
}

void NPC_Walk()
{
	//NPC->client->pers.cmd.forwardmove = 48;
	ucmd.forwardmove = 48;
	NPC->client->ps.speed = NPCInfo->stats.walkSpeed;
	NPC_SelectMoveAnimation();
}

void NPC_SelectMoveType()
{
	if (NPC->enemy && NPC->enemy->s.eType == ET_PLAYER)
		NPC_Run();

	NPC_Walk();
}

#ifdef __CRAPPY_NPC_AVOIDANCE__
int AVOIDANCE_METHOD = 0;

void NPC_Avoidance()
{
	// Try some (currently really really simple) avoidance stuff...
	if (AVOIDANCE_METHOD <= 5)
	{
		ucmd.rightmove = 127;
		NPC_SelectMoveAnimation();
	}
	else if (AVOIDANCE_METHOD <= 10)
	{
		ucmd.rightmove = -127;
		NPC_SelectMoveAnimation();
	}
	else
	{
		ucmd.upmove = 127;
		ucmd.forwardmove = 127;
		NPC_SelectMoveAnimation();
	}

	AVOIDANCE_METHOD++;

	if (AVOIDANCE_METHOD >= 12)
		AVOIDANCE_METHOD = 0;
}

#else //!__CRAPPY_NPC_AVOIDANCE__

qboolean JKG_CheckBelowPoint( vec3_t point )
{
	trace_t tr;
	vec3_t org, org2;

	VectorCopy(point, org);
	VectorCopy(point, org2);
	org2[2] = -65536.0f;

	trap->Trace( &tr, org, NULL, NULL, org2, -1, MASK_PLAYERSOLID|CONTENTS_TRIGGER, 0, 0, 0);
	
	if ( tr.startsolid )
	{
		//trap->Print("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.allsolid )
	{
		//trap->Print("Waypoint %i is in solid.\n", wp);
		return qfalse;
	}

	if ( tr.fraction == 1 )
	{
		//trap->Print("Waypoint %i is too high above ground.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_LAVA )
	{
		//trap->Print("Waypoint %i is in lava.\n", wp);
		return qfalse;
	}
	
	if ( tr.contents & CONTENTS_SLIME )
	{
		//trap->Print("Waypoint %i is in slime.\n", wp);
		return qfalse;
	}

	if ( tr.contents & CONTENTS_TRIGGER )
	{
		//trap->Print("Waypoint %i is in trigger.\n", wp);
		return qfalse;
	}

	return qtrue;
}

//#define	APEX_HEIGHT		200.0f
#define	APEX_HEIGHT		128.0f
#define	PARA_WIDTH		128.0f
//#define	PARA_WIDTH		(sqrt(APEX_HEIGHT)+sqrt(APEX_HEIGHT))
//#define	JUMP_SPEED		200.0f
#define	JUMP_SPEED		128.0f

static qboolean NPC_Jump( gentity_t *NPC, vec3_t dest )
{//FIXME: if land on enemy, knock him down & jump off again
	if ( 1 )
	{
		float	targetDist, shotSpeed = 300, travelTime, impactDist, bestImpactDist = Q3_INFINITE;//fireSpeed, 
		vec3_t	targetDir, shotVel, failCase; 
		trace_t	trace;
		trajectory_t	tr;
		qboolean	blocked;
		int		elapsedTime, timeStep = 500, hitCount = 0, maxHits = 7;
		vec3_t	lastPos, testPos, bottom;

		while ( hitCount < maxHits )
		{
			VectorSubtract( dest, NPC->r.currentOrigin, targetDir );
			targetDist = VectorNormalize( targetDir );

			VectorScale( targetDir, shotSpeed, shotVel );
			travelTime = targetDist/shotSpeed;
			shotVel[2] += travelTime * 0.5 * NPC->client->ps.gravity;

			if ( !hitCount )		
			{//save the first one as the worst case scenario
				VectorCopy( shotVel, failCase );
			}

			if ( 1 )//tracePath )
			{//do a rough trace of the path
				blocked = qfalse;

				VectorCopy( NPC->r.currentOrigin, tr.trBase );
				VectorCopy( shotVel, tr.trDelta );
				tr.trType = TR_GRAVITY;
				tr.trTime = level.time;
				travelTime *= 1000.0f;
				VectorCopy( NPC->r.currentOrigin, lastPos );
				
				//This may be kind of wasteful, especially on long throws... use larger steps?  Divide the travelTime into a certain hard number of slices?  Trace just to apex and down?
				for ( elapsedTime = timeStep; elapsedTime < floor(travelTime)+timeStep; elapsedTime += timeStep )
				{
					if ( (float)elapsedTime > travelTime )
					{//cap it
						elapsedTime = floor( travelTime );
					}
					BG_EvaluateTrajectory( &tr, level.time + elapsedTime, testPos );
					if ( testPos[2] < lastPos[2] )
					{//going down, ignore botclip
						trap->Trace( &trace, lastPos, NPC->r.mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask , 0, 0, 0);
					}
					else
					{//going up, check for botclip
						trap->Trace( &trace, lastPos, NPC->r.mins, NPC->r.maxs, testPos, NPC->s.number, NPC->clipmask|CONTENTS_BOTCLIP , 0, 0, 0);
					}

					if ( trace.allsolid || trace.startsolid )
					{
						blocked = qtrue;
						break;
					}
					if ( trace.fraction < 1.0f )
					{//hit something
						if ( Distance( trace.endpos, dest ) < 96 )
						{//hit the spot, that's perfect!
							//Hmm, don't want to land on him, though...
							break;
						}
						else 
						{
							if ( trace.contents & CONTENTS_BOTCLIP )
							{//hit a do-not-enter brush
								blocked = qtrue;
								break;
							}
							if ( trace.plane.normal[2] > 0.7 && DistanceSquared( trace.endpos, dest ) < 4096 )//hit within 64 of desired location, should be okay
							{//close enough!
								break;
							}
							else
							{//FIXME: maybe find the extents of this brush and go above or below it on next try somehow?
								impactDist = DistanceSquared( trace.endpos, dest );
								if ( impactDist < bestImpactDist )
								{
									bestImpactDist = impactDist;
									VectorCopy( shotVel, failCase );
								}
								blocked = qtrue;
								break;
							}
						}
					}
					if ( elapsedTime == floor( travelTime ) )
					{//reached end, all clear
						if ( trace.fraction >= 1.0f )
						{//hmm, make sure we'll land on the ground...
							//FIXME: do we care how far below ourselves or our dest we'll land?
							VectorCopy( trace.endpos, bottom );
							bottom[2] -= 128;
							trap->Trace( &trace, trace.endpos, NPC->r.mins, NPC->r.maxs, bottom, NPC->s.number, NPC->clipmask , 0, 0, 0);
							if ( trace.fraction >= 1.0f )
							{//would fall too far
								blocked = qtrue;
							}
						}
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
					shotSpeed = 300 + ((hitCount-2) * 100);//from 100 to 900 (skipping 300)
					if ( hitCount >= 2 )
					{//skip 300 since that was the first value we tested
						shotSpeed += 100;
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
			//NOTE: or try failcase?
			VectorCopy( failCase, NPC->client->ps.velocity );
		}
		VectorCopy( shotVel, NPC->client->ps.velocity );
	}
	else
	{//a more complicated jump
		vec3_t		dir, p1, p2, apex;
		float		time, height, forward, z, xy, dist, apexHeight;

		if ( NPC->r.currentOrigin[2] > dest[2] )//NPCInfo->goalEntity->r.currentOrigin
		{
			VectorCopy( NPC->r.currentOrigin, p1 );
			VectorCopy( dest, p2 );//NPCInfo->goalEntity->r.currentOrigin
		}
		else if ( NPC->r.currentOrigin[2] < dest[2] )//NPCInfo->goalEntity->r.currentOrigin
		{
			VectorCopy( dest, p1 );//NPCInfo->goalEntity->r.currentOrigin
			VectorCopy( NPC->r.currentOrigin, p2 );
		}
		else
		{
			VectorCopy( NPC->r.currentOrigin, p1 );
			VectorCopy( dest, p2 );//NPCInfo->goalEntity->r.currentOrigin
		}

		//z = xy*xy
		VectorSubtract( p2, p1, dir );
		dir[2] = 0;

		//Get xy and z diffs
		xy = VectorNormalize( dir );
		z = p1[2] - p2[2];

		apexHeight = APEX_HEIGHT/2;

		//Determine most desirable apex height
		//FIXME: length of xy will change curve of parabola, need to account for this
		//somewhere... PARA_WIDTH
		/*
		apexHeight = (APEX_HEIGHT * PARA_WIDTH/xy) + (APEX_HEIGHT * z/128);
		if ( apexHeight < APEX_HEIGHT * 0.5 )
		{
			apexHeight = APEX_HEIGHT*0.5;
		}
		else if ( apexHeight > APEX_HEIGHT * 2 )
		{
			apexHeight = APEX_HEIGHT*2;
		}
		*/

		z = (sqrt(apexHeight + z) - sqrt(apexHeight));

		assert(z >= 0);

//		Com_Printf("apex is %4.2f percent from p1: ", (xy-z)*0.5/xy*100.0f);

		xy -= z;
		xy *= 0.5;
		
		assert(xy > 0);

		VectorMA( p1, xy, dir, apex );
		apex[2] += apexHeight;

		VectorCopy(apex, NPC->pos1);
		
		//Now we have the apex, aim for it
		height = apex[2] - NPC->r.currentOrigin[2];
		time = sqrt( height / ( .5 * NPC->client->ps.gravity ) );//was 0.5, but didn't work well for very long jumps
		if ( !time ) 
		{
			//Com_Printf( S_COLOR_RED"ERROR: no time in jump\n" );
			return qfalse;
		}

		VectorSubtract ( apex, NPC->r.currentOrigin, NPC->client->ps.velocity );
		NPC->client->ps.velocity[2] = 0;
		dist = VectorNormalize( NPC->client->ps.velocity );

		forward = dist / time * 1.25;//er... probably bad, but...
		VectorScale( NPC->client->ps.velocity, forward, NPC->client->ps.velocity );

		//FIXME:  Uh.... should we trace/EvaluateTrajectory this to make sure we have clearance and we land where we want?
		NPC->client->ps.velocity[2] = time * NPC->client->ps.gravity;

		//Com_Printf("Jump Velocity: %4.2f, %4.2f, %4.2f\n", NPC->client->ps.velocity[0], NPC->client->ps.velocity[1], NPC->client->ps.velocity[2] );
	}
	return qtrue;
}

//extern void G_SoundOnEnt( gentity_t *ent, int channel, const char *soundPath );

static qboolean NPC_TryJump( gentity_t *NPC, vec3_t goal )
{//FIXME: never does a simple short, regular jump...
	if ( TIMER_Done( NPC, "jumpChaseDebounce" ) )
	{
		{
			if ( !PM_InKnockDown( &NPC->client->ps ) && !BG_InRoll( &NPC->client->ps, NPC->client->ps.legsAnim ) )
			{//enemy is on terra firma
				vec3_t goal_diff;
				float goal_z_diff;
				float goal_xy_dist;
				VectorSubtract( goal, NPC->r.currentOrigin, goal_diff );
				goal_z_diff = goal_diff[2];
				goal_diff[2] = 0;
				goal_xy_dist = VectorNormalize( goal_diff );
				if ( goal_xy_dist < 550 && goal_z_diff > -400/*was -256*/ )//for now, jedi don't take falling damage && (NPC->health > 20 || goal_z_diff > 0 ) && (NPC->health >= 100 || goal_z_diff > -128 ))//closer than @512
				{
					qboolean debounce = qfalse;
					if ( NPC->health < 150 && ((NPC->health < 30 && goal_z_diff < 0) || goal_z_diff < -128 ) )
					{//don't jump, just walk off... doesn't help with ledges, though
						debounce = qtrue;
					}
					else if ( goal_z_diff < 32 && goal_xy_dist < 200 )
					{//what is their ideal jump height?
						
						ucmd.upmove = 127;
						debounce = qtrue;
					}
					else
					{
						/*
						//NO!  All Jedi can jump-navigate now...
						if ( NPCInfo->rank != RANK_CREWMAN && NPCInfo->rank <= RANK_LT_JG )
						{//can't do acrobatics
							return qfalse;
						}
						*/
						if ( goal_z_diff > 0 || goal_xy_dist > 128 )
						{//Fake a force-jump
							//Screw it, just do my own calc & throw
							vec3_t dest;
							VectorCopy( goal, dest );
							
							{
								int	sideTry = 0;
								while( sideTry < 10 )
								{//FIXME: make it so it doesn't try the same spot again?
									trace_t	trace;
									vec3_t	bottom;

									VectorCopy( dest, bottom );
									bottom[2] -= 128;
									trap->Trace( &trace, dest, NPC->r.mins, NPC->r.maxs, bottom, NPC->s.number, NPC->clipmask , 0, 0, 0);
									if ( trace.fraction < 1.0f )
									{//hit floor, okay to land here
										break;
									}
									sideTry++;
								}
								if ( sideTry >= 10 )
								{//screw it, just jump right at him?
									VectorCopy( goal, dest );
								}
							}

							if ( NPC_Jump( NPC, dest ) )
							{
								//Com_Printf( "(%d) pre-checked force jump\n", level.time );

								//FIXME: store the dir we;re going in in case something gets in the way of the jump?
								//? = vectoyaw( NPC->client->ps.velocity );
								/*
								if ( NPC->client->ps.velocity[2] < 320 )
								{
									NPC->client->ps.velocity[2] = 320;
								}
								else
								*/
								{//FIXME: make this a function call
									int jumpAnim;
									//FIXME: this should be more intelligent, like the normal force jump anim logic
									//if ( NPC->client->NPC_class == CLASS_BOBAFETT 
									//	||( NPCInfo->rank != RANK_CREWMAN && NPCInfo->rank <= RANK_LT_JG ) )
									//{//can't do acrobatics
									//	jumpAnim = BOTH_FORCEJUMP1;
									//}
									//else
									//{
										jumpAnim = BOTH_FLIP_F;
									//}
									G_SetAnim( NPC, &ucmd, SETANIM_BOTH, jumpAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
								}

								NPC->client->ps.fd.forceJumpZStart = NPC->r.currentOrigin[2];
								//NPC->client->ps.pm_flags |= PMF_JUMPING;

								NPC->client->ps.weaponTime = NPC->client->ps.torsoTimer;
								NPC->client->ps.fd.forcePowersActive |= ( 1 << FP_LEVITATION );
								
								//if ( NPC->client->NPC_class == CLASS_BOBAFETT )
								//{
								//	G_SoundOnEnt( NPC, CHAN_ITEM, "sound/boba/jeton.wav" );
								//	NPC->client->jetPackTime = level.time + Q_irand( 1000, 15000 + irand(0, 30000) );
								//}
								//else
								//{
								//	G_SoundOnEnt( NPC, CHAN_BODY, "sound/weapons/force/jump.wav" );
								//}

								TIMER_Set( NPC, "forceJumpChasing", Q_irand( 2000, 3000 ) );
								debounce = qtrue;
							}
						}
					}

					if ( debounce )
					{
						//Don't jump again for another 2 to 5 seconds
						TIMER_Set( NPC, "jumpChaseDebounce", Q_irand( 2000, 5000 ) );
						ucmd.forwardmove = 127;
						VectorClear( NPC->client->ps.moveDir );
						TIMER_Set( NPC, "duck", -level.time );
						return qtrue;
					}
				}
			}
		}
	}

	return qfalse;
}

qboolean NPC_ClearPathToJump( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	mins, start, end, dir;
	float	dist, drop;
	float	i;

	//Offset the step height
	VectorSet( mins, NPC->r.mins[0], NPC->r.mins[1], NPC->r.mins[2] + STEPSIZE );
	
	trap->Trace( &trace, NPC->r.currentOrigin, mins, NPC->r.maxs, dest, NPC->s.number, NPC->clipmask , 0, 0, 0);

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
		{//hit what we're going after
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}

	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap->Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask , 0, 0, 0);//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		//no floor here! (or a long drop?)
		return qfalse;
	}
	//we made it!
	return qtrue;
}

qboolean DOM_NPC_ClearPathBetweenSpots( vec3_t from, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, (float)NPC->client->ps.crouchheight};

	VectorCopy(from, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	trap->Trace( &trace, org, mins, maxs, destorg, NPC->s.number, CONTENTS_PLAYERCLIP/*NPC->clipmask*/ , 0, 0, 0);

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//trap->Print("SOLID!\n");
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
			/*|| !Q_stricmp(g_entities[trace.entityNum].classname, "worldspawn")*/ )
		{//hit what we're going after
			//trap->Print("OK!\n");
			return qtrue;
		}
		else
		{
			//trap->Print("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap->Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask , 0, 0, 0);//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		trap->Print("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

qboolean DOM_NPC_CrouchPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	vec3_t	mins = {-8, -8, -6};
	//vec3_t	maxs = {18, 18, 48};
	vec3_t	maxs = {8, 8, (float)NPC->client->ps.crouchheight};

	VectorCopy(NPC->s.origin, org);
	org[2]+=STEPSIZE;

	VectorCopy(dest, destorg);
	destorg[2]+=STEPSIZE;

	trap->Trace( &trace, org, mins, maxs, destorg, NPC->s.number, CONTENTS_PLAYERCLIP/*NPC->clipmask*/ , 0, 0, 0);

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//trap->Print("SOLID!\n");
		return qfalse;
	}

	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum )
			/*|| !Q_stricmp(g_entities[trace.entityNum].classname, "worldspawn")*/ )
		{//hit what we're going after
			//trap->Print("OK!\n");
			return qtrue;
		}
		else
		{
			//trap->Print("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap->Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask , 0, 0, 0);//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		trap->Print("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

typedef enum
{// Avoidance methods...
	AVOIDANCE_NONE,
	AVOIDANCE_STRAFE_RIGHT,
	AVOIDANCE_STRAFE_LEFT,
	AVOIDANCE_STRAFE_CROUCH,
	AVOIDANCE_STRAFE_JUMP,
} avoidanceMethods_t;

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum );// below
/*
qboolean NPC_FindTemporaryWaypoint()
{
	int i = 0;
	int j = 0;

	for (i = 0; i < gWPArray[NPC->wpCurrent]->neighbornum; i++)
	{
		int test_wp = gWPArray[NPC->wpCurrent]->neighbors[i].num;

		if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[test_wp]->origin, -1 ) && Q_irand(0,2) == 1)
		{// This link is walkable for us... Now check if it can see the current waypoint...
			if (DOM_NPC_ClearPathBetweenSpots( gWPArray[test_wp]->origin, gWPArray[NPC->wpCurrent]->origin, -1 ))
			{// Found one!
				VectorCopy(gWPArray[test_wp]->origin, NPC->bot_strafe_target_position);
				NPC->wpTravelTime = level.time + 10000; // give them time to traverse this wp...
				NPC->bot_strafe_target_timer = level.time + 5000;
				trap->Print("NPC %s is using an avoidance waypoint.\n", NPC->NPC_type);
				return qtrue;
			}
		}
	}

	return qfalse;
}
*/

vec3_t jumpLandPosition;

qboolean NPC_NeedJump()
{
	trace_t		tr;
	vec3_t		org1, org2;
	vec3_t		forward;

	VectorCopy(NPC->r.currentOrigin, org1);

	AngleVectors( NPC->r.currentAngles, forward, NULL, NULL );

	// Check jump...
	org1[2] += 8;
	forward[PITCH] = forward[ROLL] = 0;
	VectorMA( org1, 64, forward, org2 );

	if (NPC->waterlevel > 0)
	{// Always jump out of water...
		VectorCopy(org2, jumpLandPosition);
		return qtrue;
	}

	trap->Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);

	if (tr.fraction < 1.0f)
	{// Looks like we might need to jump... Check if it would work...
		VectorCopy(NPC->r.currentOrigin, org1);
		org1[2] += 32;
		VectorMA( org1, 64, forward, org2 );
		trap->Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);

		if (tr.fraction >= 0.7f)
		{// Close enough...
			//trap->Print("need jump");
			VectorCopy(org2, jumpLandPosition);
			return qtrue;
		}
	}

	return qfalse;
}

int NPC_SelectBestAvoidanceMethod()
{// Just find the largest visible distance direction...
	trace_t		tr;
	vec3_t		org1, org2;
	vec3_t		forward, right;
	int			i = 0;
	qboolean	SKIP_RIGHT = qfalse;
	qboolean	SKIP_LEFT = qfalse;

	if (NPC->bot_strafe_right_timer > level.time)
		return AVOIDANCE_STRAFE_RIGHT;

	if (NPC->bot_strafe_left_timer > level.time)
		return AVOIDANCE_STRAFE_LEFT;

	if (NPC->bot_strafe_crouch_timer > level.time)
		return AVOIDANCE_STRAFE_CROUCH;

	if (NPC->bot_strafe_jump_timer > level.time)
		return AVOIDANCE_STRAFE_JUMP;

	//if (NPC_FindTemporaryWaypoint())
	//	return AVOIDANCE_NONE;

	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	AngleVectors( NPC->move_vector, NPC->movedir, NULL, NULL );

	VectorCopy(NPC->r.currentOrigin, org1);
	org1[2] += STEPSIZE;

	VectorCopy(gWPArray[NPC->wpCurrent]->origin, org2);
	org2[2] += STEPSIZE;

	trap->Trace( &tr, org1, NPC->r.mins, NPC->r.maxs, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
		
	if (tr.fraction == 1.0f)
	{// It is accessable normally...
		return AVOIDANCE_NONE;
	}

	// OK, our waypoint is not accessable normally, we need to select a strafe direction...
	for (i = STEPSIZE; i <= STEPSIZE*4; i += STEPSIZE)
	{// First one to make it is the winner... The race is on!
		if (!SKIP_RIGHT)
		{// Check right side...
			VectorCopy(NPC->r.currentOrigin, org1);
			org1[2] += STEPSIZE;
			AngleVectors( NPC->move_vector, forward, right, NULL );
			VectorMA( org1, i, right, org1 );

			if (!OrgVisible(NPC->r.currentOrigin, org1, NPC->s.number)) 
				SKIP_RIGHT = qtrue;

			if (!SKIP_RIGHT)
			{
				trap->Trace( &tr, org1, NPC->r.mins, NPC->r.maxs, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
		
				if (tr.fraction == 1.0f)
				{
					if (JKG_CheckBelowPoint(org1))
					{
						return AVOIDANCE_STRAFE_RIGHT;
					}
				}
			}
		}

		if (!SKIP_LEFT)
		{// Check left side...
			VectorCopy(NPC->r.currentOrigin, org1);
			org1[2] += STEPSIZE;
			AngleVectors( NPC->move_vector, forward, right, NULL );
			VectorMA( org1, 0 - i, right, org1 );
		
			if (!OrgVisible(NPC->r.currentOrigin, org1, NPC->s.number)) 
				SKIP_LEFT = qtrue;

			if (!SKIP_LEFT)
			{
				trap->Trace( &tr, org1, NPC->r.mins, NPC->r.maxs, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
		
				if (tr.fraction == 1.0f)
				{
					if (JKG_CheckBelowPoint(org1))
					{
						return AVOIDANCE_STRAFE_LEFT;
					}
				}
			}
		}
	}

	return AVOIDANCE_NONE;
}

qboolean NPC_NPCBlockingPath()
{
	int		i;
	int		BEST_METHOD = AVOIDANCE_NONE;

	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	AngleVectors( NPC->move_vector, NPC->movedir, NULL, NULL );

	for (i = 0; i < MAX_GENTITIES; i++)
	{
		gentity_t *ent = &g_entities[i];

		if (!ent) continue;
		if (ent->s.eType != ET_PLAYER && ent->s.eType != ET_NPC) continue;
		if (Distance(ent->r.currentOrigin, NPC->r.currentOrigin) > 64) continue;

		if (InFOV3( ent->r.currentOrigin, NPC->r.currentOrigin, NPC->move_vector, 90, 120 ))
		{
			if (InFOV3( NPC->r.currentOrigin, ent->r.currentOrigin, NPC->move_vector, 90, 120 ))
				ent->bot_strafe_left_timer = level.time + 200;
			else
				ent->bot_strafe_right_timer = level.time + 200;

			NPC->bot_strafe_left_timer = level.time + 200;
			return qtrue;
		}
	}

	BEST_METHOD = NPC_SelectBestAvoidanceMethod();
	
	switch (BEST_METHOD)
	{
	case AVOIDANCE_STRAFE_RIGHT:
		NPC->bot_strafe_right_timer = level.time + 200;
		return qtrue;
		break;
	case AVOIDANCE_STRAFE_LEFT:
		NPC->bot_strafe_left_timer = level.time + 200;
		return qtrue;
		break;
	case AVOIDANCE_STRAFE_CROUCH:
		NPC->bot_strafe_crouch_timer = level.time + 200;
		break;
	case AVOIDANCE_STRAFE_JUMP:
		break;
	default:
		break;
	}

	return qfalse;
}

void NPC_Avoidance()
{
	/*int BEST_METHOD = NPC_SelectBestAvoidanceMethod();
	
	switch (BEST_METHOD)
	{
	case AVOIDANCE_STRAFE_RIGHT:
		ucmd.rightmove = 48;
		NPC->bot_strafe_right_timer = level.time + 100;
		break;
	case AVOIDANCE_STRAFE_LEFT:
		ucmd.rightmove = -48;
		NPC->bot_strafe_left_timer = level.time + 100;
		break;
	case AVOIDANCE_STRAFE_CROUCH:
		NPC->playerState->pm_flags |= PMF_DUCKED;
		NPC->bot_strafe_crouch_timer = level.time + 1000;
		break;
	case AVOIDANCE_STRAFE_JUMP:
		NPC_TryJump( NPC, gWPArray[NPC->wpCurrent]->origin );
		break;
	default:
		break;
	}
	*/

	// Try to find an alternative waypoint...
	int i = 0;

	if (NPC->wpNext >= 0 && NPC->wpNext < gWPNum)
	{
		// Can we move direct to our next waypoint?
		if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[NPC->wpNext]->origin, -1 ))
		{// Looks good. Use this waypoint instead...
			NPC->wpLast = NPC->wpCurrent;
			NPC->wpCurrent = NPC->wpNext;
			NPC->wpNext = NPC_GetNextNode(NPC);
			//NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
			if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
			{
				float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

				if (dist < 1) dist = 1;

				NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
			}
			else
				NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
			return;
		}

		// Look at our next waypoint's neighbours for one we can see...
		for (i = 0; i < gWPArray[NPC->wpNext]->neighbornum; i++)
		{
			int wp = gWPArray[NPC->wpNext]->neighbors[i].num;

			if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[wp]->origin, -1 ))
			{// Looks good. Use this waypoint instead...
				NPC->wpCurrent = wp;
				//NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
				if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
				{
					float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

					if (dist < 1) dist = 1;

					NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
				}
				else
					NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
				return;
			}
		}

		// Look at this waypoint's neighbours for a visible path to the next...
		for (i = 0; i < gWPArray[NPC->wpCurrent]->neighbornum; i++)
		{
			int wp = gWPArray[NPC->wpCurrent]->neighbors[i].num;
			int j = 0;
		
			for (j = 0; j < gWPArray[wp]->neighbornum; j++)
			{
				if (gWPArray[wp]->neighbors[j].num == NPC->wpNext)
				{
					if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[wp]->origin, -1 ))
					{// Looks good. Use this waypoint instead...
						NPC->wpCurrent = wp;
						//NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
						if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
						{
							float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

							if (dist < 1) dist = 1;

							NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
						}
						else
							NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
						return;
					}
				}
			}
		}
	}

	// Reset our pathing and find a new one...
	NPC->longTermGoal = -1;
	NPC->wpCurrent = -1;
	NPC->pathsize = -1;
}

#endif //__CRAPPY_NPC_AVOIDANCE__

void NPC_PickRandomIdleAnimantionCivilian()
{
	int randAnim = irand(0,10);

	switch (randAnim)
	{
	case 0:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 1:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND4, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 2:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND6, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 3:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND8, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 4:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND9, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 5:
	case 6:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND9IDLE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 7:
	case 8:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_IDLE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 9:
	default:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_LOOKAROUND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	}
}

void NPC_PickRandomIdleAnimantion()
{
	int randAnim = irand(0,10);

	if (NPC->enemy) return; // No idle anims when we got an enemy...

	if (NPC->client->lookTime > level.time) return; // Wait before next anim...

	NPC->client->lookTime = level.time + irand(5000, 15000);

	if (NPC->client->NPC_class == CLASS_CIVILIAN)
	{
		NPC_PickRandomIdleAnimantionCivilian();
		return;
	}

	switch (randAnim)
	{
	case 0:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND3, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	case 1:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND4, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	case 2:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND6, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	case 3:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND8, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	case 4:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND9, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	case 5:
	case 6:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_STAND9IDLE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 7:
	case 8:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_IDLE1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	case 9:
	default:
		NPC_SetAnim(NPC, SETANIM_BOTH, BOTH_GUARD_LOOKAROUND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
		break;
	}
}

int NPC_GetPatrolWP(gentity_t *NPC, vec3_t org)
{
	int		i, NUM_FOUND = 0, FOUND_LIST[MAX_WPARRAY_SIZE];
	float	PATROL_RANGE = NPC->patrol_range;
	float	flLen;

	i = 0;

	while (i < gWPNum)
	{
		if (gWPArray[i] && gWPArray[i]->inuse)
		{
			vec3_t org, org2;

			flLen = Distance(NPC->r.currentOrigin, gWPArray[i]->origin);

			/*if (gWPArray[i]->origin[2] > NPC->spawn_pos[2]+8
				|| gWPArray[i]->origin[2] < NPC->spawn_pos[2]-8)
			{
				i++;
				continue; // Lets just keep them on flat ground... Avoid jumping...
			}*/

			if (flLen < PATROL_RANGE && flLen > PATROL_RANGE * 0.25)
			{
				VectorCopy(NPC->r.currentOrigin, org);
				org[2]+=8;

				VectorCopy(gWPArray[i]->origin, org2);
				org2[2]+=8;

				//if (DOM_NPC_ClearPathToSpot( NPC, gWPArray[i]->origin, NPC->s.number ))
				if (OrgVisible(org, org2, NPC->s.number))
				{
					FOUND_LIST[NUM_FOUND] = i;
					NUM_FOUND++;
				}
			}
		}

		i++;
	}

	if (NUM_FOUND <= 0)
		return -1;

	// Return a random one...
	return FOUND_LIST[Q_irand(0, NUM_FOUND)];
}

qboolean NPC_FindNewPatrolWaypoint()
{
	if (NPC->noWaypointTime > level.time)
	{// Only try to find a new waypoint every 25 seconds...
		NPC_PickRandomIdleAnimantion();
		return qfalse;
	}

	NPC->noWaypointTime = level.time + 15000 + irand (0, 30000); // 15 to 45 seconds before we try again... (it will run avoidance in the meantime)

	NPC->wpCurrent = NPC_GetPatrolWP(NPC, NPC->spawn_pos);

	if (NPC->wpCurrent <= 0 || NPC->wpCurrent >= gWPNum)
	{
		NPC_PickRandomIdleAnimantion();
		return qfalse;
	}

	NPC->wpNext = NPC->wpCurrent;
	NPC->longTermGoal = NPC->wpCurrent;

	NPC->wpTravelTime = level.time + (Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) / 24.0f) * 1000.0f;

	if (NPC->wpSeenTime < NPC->noWaypointTime)
		NPC->wpSeenTime = NPC->noWaypointTime; // also make sure we don't try to make a new route for the same length of time...

	//trap->Print("NPC Waypointing Debug: NPC %i [%s] (spawn pos %f %f %f) found a patrol waypoint for itself at %f %f %f (patrol range %f).", NPC->s.number, NPC->NPC_type, NPC->spawn_pos[0], NPC->spawn_pos[1], NPC->spawn_pos[2], gWPArray[NPC->wpCurrent]->origin[0], gWPArray[NPC->wpCurrent]->origin[1], gWPArray[NPC->wpCurrent]->origin[2], NPC->patrol_range);
	return qtrue; // all good, we have a new waypoint...
}

extern int DOM_GetNearestVisibleWP(vec3_t org, int ignore, int badwp);
extern int DOM_GetNearestVisibleWP_Goal(vec3_t org, int ignore, int badwp);
extern int DOM_GetNearestVisibleWP_NOBOX(vec3_t org, int ignore, int badwp);

qboolean NPC_FindNewWaypoint()
{
	//if (NPC->noWaypointTime > level.time)
	//{// Only try to find a new waypoint every 5 seconds...
	//	NPC_PickRandomIdleAnimantion();
	//	return qfalse;
	//}

	NPC->wpCurrent = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP*//*DOM_GetBestWaypoint*/(NPC->r.currentOrigin, NPC->s.number, -1);
	//NPC->noWaypointTime = level.time + 3000; // 3 seconds before we try again... (it will run avoidance in the meantime)

	//if (NPC->wpSeenTime < NPC->noWaypointTime)
	//	NPC->wpSeenTime = NPC->noWaypointTime; // also make sure we don't try to make a new route for the same length of time...

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
	{
		NPC_PickRandomIdleAnimantion();
		//trap->Print("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for itself.", NPC->s.number, NPC->NPC_type);
		return qfalse; // failed... try again after som avoidance code...
	}

	return qtrue; // all good, we have a new waypoint...
}

qboolean NPC_CopyPathFromNearbyNPC()
{
	int i = 0;

	for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
	{
		gentity_t *test = &g_entities[i];

		if (i == NPC->s.number) continue;
		if (!test) continue;
		if (!test->inuse) continue;
		if (test->s.eType != ET_NPC) continue;
		if (test->pathsize <= 0) continue;
		if (test->client->NPC_class != NPC->client->NPC_class) continue; // Only copy from same NPC classes???
		if (Distance(NPC->r.currentOrigin, test->r.currentOrigin) > 128) continue;
		if (test->wpCurrent <= 0) continue;
		if (test->longTermGoal <= 0) continue;
		if (test->npc_dumb_route_time > level.time) continue;
		
		// Don't let them be copied again for 2 seconds...
		test->npc_dumb_route_time = level.time + 2000;

		// Seems we found one!
		memcpy(NPC->pathlist, test->pathlist, sizeof(int)*test->pathsize);
		NPC->pathsize = test->pathsize;
		NPC->wpCurrent = test->wpCurrent;
		NPC->wpNext = test->wpNext;
		NPC->wpLast = test->wpLast;
		NPC->longTermGoal = test->longTermGoal;
		
		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 2000;
		// Delay before giving up on this new waypoint/route...
		//NPC->wpTravelTime = level.time + 15000;
		if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
		{
			float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

			if (dist < 1) dist = 1;

			NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
		}
		else
			NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...

		// Don't let me be copied for 5 seconds...
		NPC->npc_dumb_route_time = level.time + 5000;

		//trap->Print("NPC Waypointing Debug: NPC %i (%s) copied a %i waypoint path between waypoints %i and %i from %i (%s).", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, test->s.number, test->NPC_type);
		return qtrue;
	}

	return qfalse;
}

void NPC_SetNewGoalAndPath()
{
	if (NPC->client->NPC_class != CLASS_TRAVELLING_VENDOR)
		if (NPC_CopyPathFromNearbyNPC()) 
			return;

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion();
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint())
	{
		return; // wait before trying to get a new waypoint...
	}

	if (NPC->return_home)
	{// Returning home...
		NPC->longTermGoal = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP_Goal*/(NPC->spawn_pos, NPC->s.number, -1);
	}
	else
	{// Find a new generic goal...
		NPC->longTermGoal = NPC_FindGoal( NPC );
	}

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);

		//if (NPC->pathsize <= 0) // Use the alternate (older) A* pathfinding code as alternative/fallback...
		//	NPC->pathsize = DOM_FindIdealPathtoWP(NULL, NPC->wpCurrent, NPC->longTermGoal, -1, NPC->pathlist);

		if (NPC->pathsize > 0)
		{
			//trap->Print("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else
		{
			//trap->Print("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			NPC_PickRandomIdleAnimantion();
			return;
		}
	}
	else
	{
		//trap->Print("NPC Waypointing Debug: NPC %i failed to find a goal waypoint.", NPC->s.number);

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 1000;//30000;
		NPC_PickRandomIdleAnimantion();
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 1000;//30000;
	// Delay before giving up on this new waypoint/route...
	//NPC->wpTravelTime = level.time + 15000;
	if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
	{
		float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

		if (dist < 1) dist = 1;

		NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
	}
	else
		NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
}

void NPC_SetNewWarzoneGoalAndPath()
{
	if (NPC->client->NPC_class == CLASS_TRAVELLING_VENDOR)
	{
		NPC_SetNewGoalAndPath(); // Use normal waypointing...
		return;
	}

	if (NPC->wpSeenTime > level.time)
	{
		NPC_PickRandomIdleAnimantion();
		return; // wait for next route creation...
	}

	if (!NPC_FindNewWaypoint())
	{
		return; // wait before trying to get a new waypoint...
	}

	// Find a new warzone goal...
	NPC->longTermGoal = NPC_FindWarzoneGoal( NPC );

	if (NPC->longTermGoal <= 0) // Backup - Find a new generic goal...
		NPC->longTermGoal = NPC_FindGoal( NPC );

	if (NPC->longTermGoal >= 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, sizeof(int)*MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);

		if (NPC->pathsize > 0)
		{
			//trap->Print("NPC Waypointing Debug: NPC %i created a %i waypoint path for a random goal between waypoints %i and %i.", NPC->s.number, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal);
			NPC->wpLast = -1;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from
		}
		else
		{
			//trap->Print("NPC Waypointing Debug: NPC %i failed to create a route between waypoints %i and %i.", NPC->s.number, NPC->wpCurrent, NPC->longTermGoal);
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 1000;//30000;
			NPC_PickRandomIdleAnimantion();
			return;
		}
	}
	else
	{
		//trap->Print("NPC Waypointing Debug: NPC %i failed to find a goal waypoint.", NPC->s.number);

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 1000;//30000;
		NPC_PickRandomIdleAnimantion();
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 1000;//30000;
	// Delay before giving up on this new waypoint/route...
	//NPC->wpTravelTime = level.time + 15000;
	if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
	{
		float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

		if (dist < 1) dist = 1;

		NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
	}
	else
		NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
}

void NPC_SetEnemyGoal()
{
	qboolean IS_COVERPOINT = qfalse;
	int			COVERPOINT_WP = -1;
	int			COVERPOINT_OFC_WP = -1;

	//if (NPC->wpSeenTime > level.time)
	//	return; // wait for next route creation...

	/*
	if (NPC->wpTravelTime < level.time)
		trap->Print("wp travel time\n");
	else 
		trap->Print("Bad wps (lt: %i) (ps: %i) (wc: %i) (wn: %i)\n", NPC->longTermGoal, NPC->pathsize, NPC->wpCurrent, NPC->wpNext);
	*/

	if (!NPC_FindNewWaypoint())
		return; // wait before trying to get a new waypoint...

	// UQ1: Gunner NPCs find cover...
	if (NPC->client->ps.weapon != WP_SABER)
	{// Should we find a cover point???
		if (NPC->enemy->wpCurrent <= 0 || NPC->enemy->wpCurrent < gWPNum)
		{// Find a new waypoint for them...
			NPC->enemy->wpCurrent = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP*/(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
		}

		if (NPC->enemy->wpCurrent > 0 
			&& NPC->enemy->wpCurrent < gWPNum
			&& Distance(gWPArray[NPC->enemy->wpCurrent]->origin, NPC->enemy->r.currentOrigin) <= 256)
		{
			int i = 0;

			for (i = 0; i < num_cover_spots; i++)
			{
				qboolean BAD = qfalse;

				if (Distance(NPC->r.currentOrigin, gWPArray[cover_nodes[i]]->origin) <= 2048.0f
					&& Distance(NPC->enemy->r.currentOrigin, gWPArray[cover_nodes[i]]->origin) <= 2048.0f)
				{// Range looks good from both places...
					int thisWP = cover_nodes[i];
					
					// OK, looking good so far... Let's see how the visibility is...
					if (NPC_IsCoverpointFor(thisWP, NPC->enemy))
					{// Looks good for a cover point...
						int j = 0;
						int z = 0;

						for (z = 0; z < MAX_GENTITIES; z++)
						{// Now just check to make sure noone else is using it... 30 stormies behind a barrel anyone???
							gentity_t *ent = &g_entities[z];

							if (!ent) continue;
							if (!ent->inuse) continue;

							if (ent->coverpointGoal == thisWP
								|| ent->wpCurrent == thisWP
								|| ent->wpNext == thisWP)
							{// Meh, he already claimed it!
								BAD = qtrue;
								break;
							}
						}

						// Twas a stormie barrel... *sigh*
						if (BAD) continue;

						// So far, so good... Now check if a link from it can see the enemy.. (to dip in and out of cover to/from)
						for (j = 0; j < gWPArray[thisWP]->neighbornum; j++)
						{
							int lookWP = gWPArray[thisWP]->neighbors[j].num;

							if (!NPC_IsCoverpointFor(lookWP, NPC->enemy))
							{// Yes! Found one!
								COVERPOINT_WP = thisWP;
								COVERPOINT_OFC_WP = lookWP;
								IS_COVERPOINT = qtrue;
								break;
							}
						}

						if (IS_COVERPOINT) break; // We got one!
					}
				}

				if (IS_COVERPOINT) break; // We got one!
			}

			if (IS_COVERPOINT)
			{// WooHoo!!!! We got one! *dance*
				NPC->longTermGoal = NPC->coverpointGoal = COVERPOINT_WP;
				NPC->coverpointOFC = COVERPOINT_OFC_WP;
			}

			if (NPC->longTermGoal <= 0)
			{// Fallback...
				NPC->longTermGoal = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP_Goal*/(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
			}
		}
		else
		{// Just head toward them....
			NPC->longTermGoal = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP_Goal*/(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
		}
	}
	else
	{
		NPC->longTermGoal = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP_Goal*/(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
	}

	if (NPC->longTermGoal > 0)
	{
		memset(NPC->pathlist, WAYPOINT_NONE, MAX_WPARRAY_SIZE);
		NPC->pathsize = ASTAR_FindPathWithTimeLimit(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist);

		//if (NPC->pathsize <= 0) // Use the alternate (older) A* pathfinding code as alternative/fallback...
			//NPC->pathsize = DOM_FindIdealPathtoWP(NULL, NPC->wpCurrent, NPC->longTermGoal, -1, NPC->pathlist);
			//NPC->pathsize = ASTAR_FindPathFast(NPC->wpCurrent, NPC->longTermGoal, NPC->pathlist, qtrue);
		
		if (NPC->pathsize > 0)
		{
			/*
			if (NPC->enemy->s.eType == ET_PLAYER)
			{
				if (IS_COVERPOINT)
					trap->Print("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint COVERPOINT path between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
				else
					trap->Print("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
			}
			else
			{
				if (IS_COVERPOINT)
					trap->Print("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint COVERPOINT path between waypoints %i and %i for enemy %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->classname);
				else
					trap->Print("NPC Waypointing Debug: NPC %i (%s) created a %i waypoint path between waypoints %i and %i for enemy %s.", NPC->s.number, NPC->NPC_type, NPC->pathsize, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->classname);
			}
			*/

			NPC->wpLast = NPC->wpCurrent;
			NPC->wpNext = NPC_GetNextNode(NPC);		//move to this node first, since it's where our path starts from

			//trap->Print("New: wps (lt: %i) (ps: %i) (wc: %i) (wn: %i)\n", NPC->longTermGoal, NPC->pathsize, NPC->wpCurrent, NPC->wpNext);

			if (NPC->client->ps.weapon == WP_SABER)
			{
				G_AddVoiceEvent( NPC, Q_irand( EV_JCHASE1, EV_JCHASE3 ), 15000 + irand(0, 30000) );
			}
			else
			{
				int choice = irand(0,13);

				switch (choice)
				{
				case 0:
					G_AddVoiceEvent( NPC, EV_OUTFLANK1, 15000 + irand(0, 30000) );
					break;
				case 1:
					G_AddVoiceEvent( NPC, EV_OUTFLANK2, 15000 + irand(0, 30000) );
					break;
				case 2:
					G_AddVoiceEvent( NPC, EV_CHASE1, 15000 + irand(0, 30000) );
					break;
				case 3:
					G_AddVoiceEvent( NPC, EV_CHASE2, 15000 + irand(0, 30000) );
					break;
				case 4:
					G_AddVoiceEvent( NPC, EV_CHASE3, 15000 + irand(0, 30000) );
					break;
				case 5:
					G_AddVoiceEvent( NPC, EV_COVER1, 15000 + irand(0, 30000) );
					break;
				case 6:
					G_AddVoiceEvent( NPC, EV_COVER2, 15000 + irand(0, 30000) );
					break;
				case 7:
					G_AddVoiceEvent( NPC, EV_COVER3, 15000 + irand(0, 30000) );
					break;
				case 8:
					G_AddVoiceEvent( NPC, EV_COVER4, 15000 + irand(0, 30000) );
					break;
				case 9:
					G_AddVoiceEvent( NPC, EV_COVER5, 15000 + irand(0, 30000) );
					break;
				case 10:
					G_AddVoiceEvent( NPC, EV_ESCAPING1, 15000 + irand(0, 30000) );
					break;
				case 11:
					G_AddVoiceEvent( NPC, EV_ESCAPING2, 15000 + irand(0, 30000) );
					break;
				case 12:
					G_AddVoiceEvent( NPC, EV_ESCAPING3, 15000 + irand(0, 30000) );
					break;
				default:
					G_AddVoiceEvent( NPC, EV_COVER5, 15000 + irand(0, 30000) );
					break;
				}
			}
		}
		else if (NPC->enemy->s.eType == ET_PLAYER)
		{
			//trap->Print("NPC Waypointing Debug: NPC %i (%s) failed to create a route between waypoints %i and %i for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->wpCurrent, NPC->longTermGoal, NPC->enemy->client->pers.netname);
			NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;
			// Delay before next route creation...
			NPC->wpSeenTime = level.time + 2000;
			return;
		}
	}
	else
	{
		//if (NPC->enemy->s.eType == ET_PLAYER)
		//	trap->Print("NPC Waypointing Debug: NPC %i (%s) failed to find a waypoint for enemy player %s.", NPC->s.number, NPC->NPC_type, NPC->enemy->client->pers.netname);

		NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;

		// Delay before next route creation...
		NPC->wpSeenTime = level.time + 2000;
		return;
	}

	// Delay before next route creation...
	NPC->wpSeenTime = level.time + 2000;
	// Delay before giving up on this new waypoint/route...
	//NPC->wpTravelTime = level.time + 5000;

	if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
	{
		float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

		if (dist < 1) dist = 1;

		NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
	}
	else
		NPC->wpTravelTime = level.time + 10000; // Give him more time to get to the new wp...
}

// Position to position
qboolean NPC_EnemyVisible( gentity_t *self, gentity_t *enemy )
{
	trace_t		tr;
	vec3_t		selfEyes, enemyEyes;

	if (!enemy)
	{
		return qfalse;
	}

	if (!enemy->client)
	{
		self->enemy = NULL;
		return qfalse;
	}

	if (NPC->genericValue15 < level.time)
	{
		VectorCopy(self->r.currentOrigin, selfEyes);
		
		if (self->client->ps.pm_flags & PMF_DUCKED)
			selfEyes[2]+=16;
		else
			selfEyes[2]+=32;

		VectorCopy(enemy->r.currentOrigin, enemyEyes);
		
		if (enemy->client->ps.pm_flags & PMF_DUCKED)
			enemyEyes[2]+=16;
		else
			enemyEyes[2]+=32;
	
		trap->Trace ( &tr, selfEyes, NULL, NULL, enemyEyes, self->s.number, MASK_SHOT, 0, 0, 0 );

		if (tr.entityNum == enemy->s.number)
		{
			// Set a new waypoint...
			self->genericValue15 = level.time + 4000; // set as visible for 4 secs...
			self->genericValue14 = level.time + 30000; // set give up timer for 30 secs...
			return qtrue;
		}

		return qfalse;
	}

	// Enemy still visible from last check...
	return qtrue;
}

qboolean DOM_NPC_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
{
	trace_t	trace;
	vec3_t	/*start, end, dir,*/ org, destorg;
//	float	dist, drop;
//	float	i;

	//Offset the step height
	//vec3_t	mins = {-18, -18, -24};
	//vec3_t	maxs = {18, 18, 48};

	VectorCopy(NPC->s.origin, org);
	//org[2]+=STEPSIZE;
	org[2]+=8;

	VectorCopy(dest, destorg);
	//destorg[2]+=STEPSIZE;
	destorg[2]+=8;

	trap->Trace( &trace, org, NULL/*mins*/, NULL/*maxs*/, destorg, NPC->s.number, MASK_PLAYERSOLID/*NPC->clipmask*/ , 0, 0, 0);

	//Do a simple check
	if ( trace.allsolid || trace.startsolid )
	{//inside solid
		//trap->Print("SOLID!\n");
		return qfalse;
	}

	/*
	if ( trace.fraction < 1.0f )
	{//hit something
		if ( (impactEntNum != ENTITYNUM_NONE && trace.entityNum == impactEntNum ))
		{//hit what we're going after
			//trap->Print("OK!\n");
			return qtrue;
		}
		else
		{
			//trap->Print("TRACE FAIL! - NPC %i hit entity %i (%s).\n", NPC->s.number, trace.entityNum, g_entities[trace.entityNum].classname);
			return qfalse;
		}
	}
	*/

	if ( trace.fraction < 1.0f )
		return qfalse;

	/*
	//otherwise, clear path in a straight line.  
	//Now at intervals of my size, go along the trace and trace down STEPSIZE to make sure there is a solid floor.
	VectorSubtract( dest, NPC->r.currentOrigin, dir );
	dist = VectorNormalize( dir );
	if ( dest[2] > NPC->r.currentOrigin[2] )
	{//going up, check for steps
		drop = STEPSIZE;
	}
	else
	{//going down or level, check for moderate drops
		drop = 64;
	}
	for ( i = NPC->r.maxs[0]*2; i < dist; i += NPC->r.maxs[0]*2 )
	{//FIXME: does this check the last spot, too?  We're assuming that should be okay since the enemy is there?
		VectorMA( NPC->r.currentOrigin, i, dir, start );
		VectorCopy( start, end );
		end[2] -= drop;
		trap->Trace( &trace, start, mins, NPC->r.maxs, end, NPC->s.number, NPC->clipmask , 0, 0, 0);//NPC->r.mins?
		if ( trace.fraction < 1.0f || trace.allsolid || trace.startsolid )
		{//good to go
			continue;
		}
		trap->Print("FLOOR!\n");
		//no floor here! (or a long drop?)
		return qfalse;
	}*/
	//we made it!
	return qtrue;
}

//Adjusts the moveDir to account for strafing
void NPC_AdjustforStrafe(vec3_t moveDir)
{
	vec3_t right, angles;

	if (!(NPC->bot_strafe_right_timer > level.time)
		&& !(NPC->bot_strafe_left_timer > level.time))
		return;

	vectoangles(moveDir, angles);
	AngleVectors(angles /*NPC->client->ps.viewangles*/, NULL, right, NULL);

	//flaten up/down
	right[2] = 0;

	if (NPC->bot_strafe_left_timer > level.time)
	{//strafing left
		VectorScale(right, -64, right);
	}
	else if (NPC->bot_strafe_right_timer > level.time)
	{//strafing right
		VectorScale(right, 64, right);
	}

	//We assume that moveDir has been normalized before this function.
	VectorAdd(moveDir, right, moveDir);
	VectorNormalize(moveDir);
}

//===========================================================================
// Routine      : UQ1_UcmdMoveForDir

// Description  : Set a valid ucmd move for the current move direction... A working one, unlike raven's joke...
void
UQ1_UcmdMoveForDir ( gentity_t *self, usercmd_t *cmd, vec3_t dir )
{
	vec3_t	forward, right, up;
	float	speed = 127.0f;

	NPC_AdjustforStrafe(dir);
	
	//AngleVectors( self->client->ps.viewangles, forward, right, up );
	AngleVectors( self->r.currentAngles, forward, right, up );

	dir[2] = 0;
	VectorNormalize( dir );
	cmd->forwardmove = DotProduct( forward, dir ) * speed;
	cmd->rightmove = DotProduct( right, dir ) * speed;

	//cmd->upmove = abs(forward[3] ) * dir[3] * speed;

	NPC_SelectMoveAnimation();
}

qboolean NPC_OrgVisible(vec3_t org1, vec3_t org2, int ignore)
{
	trace_t tr;
	vec3_t	from, to;

	VectorCopy(org1, from);
	from[2] += 32;
	VectorCopy(org2, to);
	to[2] += 18;

	trap->Trace(&tr, from, NULL, NULL, to, ignore, MASK_SOLID, 0, 0, 0);

	if (tr.fraction == 1)
	{
		return qtrue;
	}

	return qfalse;
}

qboolean NPC_HaveValidEnemy( void )
{
	if (NPC->enemy 
		&& NPC->enemy->health > 0
		&& !(NPC->enemy->s.eFlags & EF_DEAD))
		return qtrue;

	return qfalse;
}

extern qboolean NPC_CoverpointVisible ( gentity_t *NPC, int coverWP );

qboolean NPC_FollowRoutes( void ) 
{// Quick method of following bot routes...
	vec3_t		velocity_vec;//, fwd;
	float		velocity;
	qboolean	ENEMY_VISIBLE = qfalse;
	qboolean	HUNTING_ENEMY = qfalse;
	qboolean	FORCED_COVERSPOT_FIND = qfalse;

	if (NPC->client->NPC_class == CLASS_TRAVELLING_VENDOR && NPC->NPC->walkDebounceTime >= level.time)
	{// UQ1: Wait before moving...
		return qfalse;
	}

	if ( !NPC->enemy )
	{
		switch (NPC->client->NPC_class)
		{
		case CLASS_CIVILIAN:
		case CLASS_GENERAL_VENDOR:
		case CLASS_WEAPONS_VENDOR:
		case CLASS_ARMOR_VENDOR:
		case CLASS_SUPPLIES_VENDOR:
		case CLASS_EQUIPMENT_VENDOR:
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
			// These guys have no enemies...
			break;
		default:
			if ( NPC->client->enemyTeam != NPCTEAM_NEUTRAL )
			{
				NPC->enemy = NPC_PickEnemyExt( qtrue );

				if (NPC->enemy)
				{
					if (NPC->client->ps.weapon == WP_SABER)
						G_AddVoiceEvent( NPC, Q_irand( EV_JDETECTED1, EV_JDETECTED3 ), 15000 + irand(0, 30000) );
					else
					{
						int choice = irand(0,4);

						switch (choice)
						{
						case 0:
							G_AddVoiceEvent( NPC, EV_DETECTED1, 15000 + irand(0, 30000) );
							break;
						case 1:
							G_AddVoiceEvent( NPC, EV_DETECTED2, 15000 + irand(0, 30000) );
							break;
						case 2:
							G_AddVoiceEvent( NPC, EV_DETECTED3, 15000 + irand(0, 30000) );
							break;
						case 3:
							G_AddVoiceEvent( NPC, EV_DETECTED4, 15000 + irand(0, 30000) );
							break;
						default:
							G_AddVoiceEvent( NPC, EV_DETECTED5, 15000 + irand(0, 30000) );
							break;
						}
					}
				}
			}
			break;
		}
	}

	if (NPC->enemy && !NPC_HaveValidEnemy())
	{// Enemy not valid any more... Initialize...
		NPC->enemy = NULL;
	}

	if (NPC_HaveValidEnemy() && NPC->client->ps.weapon != WP_SABER)
	{
		/*
		if ( NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum )
		{
			FORCED_COVERSPOT_FIND = qtrue;
		}
		*/

		if (NPC->enemy->wpCurrent <= 0 || NPC->enemy->wpCurrent >= gWPNum || Distance(gWPArray[NPC->enemy->wpCurrent]->origin, NPC->enemy->r.currentOrigin) > 256)
		{// Make sure an enemy always has a waypoint to calculate cover from...
			NPC->enemy->wpCurrent = DOM_GetRandomCloseWP/*DOM_GetNearestVisibleWP*/(NPC->enemy->r.currentOrigin, NPC->enemy->s.number, -1);
		}
	}

	if ( NPC_HaveValidEnemy() )
	{// UQ1: Main visibility handling...
		// UQ1: Saber wielders should get up close and personal, even if the enemy is visible already...
		if (NPC->client->ps.weapon != WP_SABER
			|| (NPC->client->ps.weapon == WP_SABER && Distance(NPC->r.currentOrigin, NPC->enemy->r.currentOrigin) < 96))
		{
			/*
			if (NPC->client->ps.weapon != WP_SABER
				&& NPC->coverpointGoal > 0 && NPC->coverpointGoal <= gWPNum
				&& NPC->coverpointOFC > 0 && NPC->coverpointOFC <= gWPNum
				&& NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy )
				&& Distance(gWPArray[NPC->coverpointOFC]->origin, NPC->r.currentOrigin) > 24
				&& Distance(gWPArray[NPC->coverpointOFC]->origin, NPC->r.currentOrigin) < 192
				&& NPC_CoverpointVisible(NPC, NPC->coverpointOFC))
			{// We are moving back to our OFC cover point...
				if (NPC->coverpointOFC < level.time)
				{// Duck back into cover again...
					NPC->coverpointHIDEtime = level.time + irand(5000, 15000);
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointGoal;
					NPC->wpTravelTime = level.time + 20000;
				}
				else
				{// Move back behind cover again...
					trap->Print("NPC DEBUG: NPC %i [%s] is moving out from cover.\n", NPC->s.number, NPC->client->pers.netname);
					NPC->wpTravelTime = level.time + 20000;
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointOFC;
					HUNTING_ENEMY = qtrue;
				}
			}
			else if (NPC->client->ps.weapon != WP_SABER
				&& NPC->coverpointGoal > 0 && NPC->coverpointGoal <= gWPNum
				&& NPC->coverpointOFC > 0 && NPC->coverpointOFC <= gWPNum
				&& NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy )
				&& Distance(gWPArray[NPC->coverpointOFC]->origin, NPC->r.currentOrigin) < 24
				&& NPC_CoverpointVisible(NPC, NPC->coverpointOFC))
			{// We are at our OFC cover point... stay here and shoot a bit...
				if (NPC->coverpointOFC < level.time)
				{// Duck back into cover again...
					NPC->coverpointHIDEtime = level.time + irand(5000, 15000);
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointGoal;
					NPC->wpTravelTime = level.time + 20000;
				}
				else if (trap->InPVS(NPC->enemy->r.currentOrigin, NPC->r.currentOrigin) && NPC_CheckVisibility( NPC->enemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV)
				{// Shoot them!
					trap->Print("NPC DEBUG: NPC %i [%s] is out from cover shooting his enemy.\n", NPC->s.number, NPC->client->pers.netname);
					NPC->wpTravelTime = level.time + 20000;
					ENEMY_VISIBLE = qtrue;
					return qfalse;
				}
			}
			else if (NPC->client->ps.weapon != WP_SABER
				&& NPC->coverpointGoal > 0 && NPC->coverpointGoal <= gWPNum
				&& NPC->coverpointOFC > 0 && NPC->coverpointOFC <= gWPNum
				&& NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy )
				&& Distance(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin) >= 24
				&& Distance(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin) < 192
				&& NPC_CoverpointVisible(NPC, NPC->coverpointGoal))
			{// We are moving back to our cover point...
				if (NPC->coverpointHIDEtime < level.time)
				{// Duck back out from cover again...
					NPC->coverpointOFC = level.time + irand(5000, 15000);
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointOFC;
					NPC->wpTravelTime = level.time + 20000;
				}
				else
				{// Move back behind cover again...
					trap->Print("NPC DEBUG: NPC %i [%s] is moving to his cover point.\n", NPC->s.number, NPC->client->pers.netname);
					NPC->wpTravelTime = level.time + 20000;
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointGoal;
					HUNTING_ENEMY = qtrue;
				}
			}
			else if (NPC->client->ps.weapon != WP_SABER
				&& NPC->coverpointGoal > 0 && NPC->coverpointGoal <= gWPNum
				&& NPC->coverpointOFC > 0 && NPC->coverpointOFC <= gWPNum
				&& NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy )
				&& Distance(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin) < 24
				&& NPC_CoverpointVisible(NPC, NPC->coverpointGoal))
			{// We are at our cover point... stay here and shoot a bit...
				if (NPC->coverpointHIDEtime < level.time)
				{// Duck back out from cover again...
					NPC->coverpointOFC = level.time + irand(5000, 15000);
					NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointOFC;
					NPC->wpTravelTime = level.time + 20000;
				}
				else
				{// Hide! We are behind cover...
					trap->Print("NPC DEBUG: NPC %i [%s] is taking cover.\n", NPC->s.number, NPC->client->pers.netname);
					NPC->wpTravelTime = level.time + 20000;
					return qfalse; // Idle...
				}
			}
			else*/ if (NPC->client->ps.weapon != WP_SABER
				&& NPC->coverpointGoal > 0 && NPC->coverpointGoal <= gWPNum
				&& NPC->coverpointOFC > 0 && NPC->coverpointOFC <= gWPNum
				&& NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy )
				&& Distance(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin) >= 24)
			{// Keep moving to cover...
				//if (NPC->coverpointHIDEtime > level.time)
				//	trap->Print("NPC DEBUG: NPC %i [%s] is travelling to his cover point (ps: %i. distance: %f).\n", NPC->s.number, NPC->client->pers.netname, NPC->pathsize, Distance(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin));

				//NPC->coverpointHIDEtime = level.time + irand(3000, 8000);
				//NPC->wpTravelTime = level.time + 20000;
				HUNTING_ENEMY = qtrue;
			}
			/*else if (NPC->client->ps.weapon != WP_SABER
				&& (NPC->coverpointGoal <= 0 || NPC->coverpointGoal >= gWPNum))
			{// Find a new cover point...
				FORCED_COVERSPOT_FIND = qtrue;
			}*/
			else if (/*NPC->client->ps.weapon == WP_SABER 
				&&*/ trap->InPVS(NPC->enemy->r.currentOrigin, NPC->r.currentOrigin) 
				&& NPC_CheckVisibility( NPC->enemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV)
			{// Ignore cover points...
				ENEMY_VISIBLE = qtrue;
			}
		}
	}

	if (ENEMY_VISIBLE /*&& NPC->client->ps.weapon == WP_SABER*/)
	{// This sith/jedi is in range... Let normal attack code run...
		return qfalse;
	}

	if (gWPNum <= 0)
	{// No waypoints available...
		return qfalse;
	}

	if (NPC->NPC->conversationPartner)
	{// Chatting with another NPC... Stay still!
		NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );
		NPC_NPCConversation();
		return qfalse;
	}

	if (!NPC->enemy && !NPC->NPC->conversationPartner)
	{// UQ1: Strange place to do this, but whatever... ;)
		NPC_FindConversationPartner();
	}

	if (NPC->NPC->conversationPartner)
	{// Chatting with another NPC... Stay still!
		NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );
		return qfalse;
	}

	VectorCopy(NPC->client->ps.velocity, velocity_vec);
	velocity = VectorLength(velocity_vec);

	if ( NPC_HaveValidEnemy()
		&& !(NPC->client->ps.weapon != WP_SABER && (NPC->coverpointGoal > 0 || NPC->coverpointGoal < gWPNum))
		&& (Distance(NPC->enemy->r.currentOrigin, NPC->r.currentOrigin) > 128.0f || !ENEMY_VISIBLE || (NPC->client->ps.weapon == WP_SABER && Distance(NPC->r.currentOrigin, NPC->enemy->r.currentOrigin) >= 96)) )
	{// Chasing them around the map...
		HUNTING_ENEMY = qtrue;
	}

	if (HUNTING_ENEMY 
		&& (Distance(NPC->r.currentOrigin, NPC->spawn_pos) > 4096.0f || (NPC->longTermGoal > 0 && NPC->longTermGoal < gWPNum && Distance(gWPArray[NPC->longTermGoal]->origin, NPC->spawn_pos) > 4096.0f))
		&& (NPC->client->NPC_class == CLASS_STORMTROOPER || NPC->client->NPC_class == CLASS_MERC))//Stoiss add merc class
	{// Moved too far from our start position... Return home...
		NPC->enemy = NULL;
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->wpNext = -1;
		NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;
		NPC->return_home = qtrue;

		if (NPC->client->ps.weapon == WP_SABER)
			G_AddVoiceEvent( NPC, Q_irand( EV_JLOST1, EV_JLOST3 ), 15000 + irand(0, 30000) );
		else
		{
			int choice = irand(0,4);

			switch (choice)
			{
			case 0:
				G_AddVoiceEvent( NPC, EV_GIVEUP1, 15000 + irand(0, 30000) );
				break;
			case 1:
				G_AddVoiceEvent( NPC, EV_GIVEUP2, 15000 + irand(0, 30000) );
				break;
			case 2:
				G_AddVoiceEvent( NPC, EV_GIVEUP3, 15000 + irand(0, 30000) );
				break;
			case 3:
				G_AddVoiceEvent( NPC, EV_GIVEUP4, 15000 + irand(0, 30000) );
				break;
			default:
				G_AddVoiceEvent( NPC, EV_LOST1, 15000 + irand(0, 30000) );
				break;
			}
		}
		//trap->Print("Enemy too far away. Returning home...\n");
	}

	/*
	if (NPC_HaveValidEnemy()
		&& NPC->wpCurrent > 0 
		&& NPC->wpCurrent < gWPNum
		&& NPC->coverpointGoal > 0 
		&& NPC->coverpointGoal < gWPNum)
	{
		if (NPC->enemy && !NPC_IsCoverpointFor( NPC->coverpointGoal, NPC->enemy ))
		{
			//trap->Print("!NPC_IsCoverpointFor\n");
			FORCED_COVERSPOT_FIND = qtrue;
		}
	}
	*/

	if ( NPC->wpCurrent < 0 
		|| NPC->wpCurrent >= gWPNum 
		|| NPC->longTermGoal < 0 
		|| NPC->longTermGoal >= gWPNum 
		|| FORCED_COVERSPOT_FIND
		|| (NPC->wpTravelTime < level.time && velocity < 16)
		/*|| (NPC->enemy && (NPC->coverpointGoal <= 0 || NPC->coverpointGoal >= gWPNum))*/ )
	{// We hit a problem in route, or don't have one yet.. Find a new goal and path...
		//VectorSet(NPC->bot_strafe_target_position, 0, 0, 0); // init avoidance temporary waypoint...
		
		/*
		if (NPC->enemy)
		{
			if (FORCED_COVERSPOT_FIND)
				trap->Print("FORCED_COVERSPOT_FIND\n");
			else if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum)
				trap->Print("wpCurrent\n");
			else if (NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
				trap->Print("longTermGoal\n");
			else if (NPC->wpTravelTime < level.time && velocity < 16)
				trap->Print("travel Time\n");
			//else if (NPC->enemy && (NPC->coverpointGoal <= 0 || NPC->coverpointGoal >= gWPNum))
			//	trap->Print("NPC->coverpointGoal\n");
		}
		*/

		NPC->wpCurrent = -1;
		NPC->wpNext = -1;
		NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;

		if (NPC->enemy)
			NPC_SetEnemyGoal();
		else if (g_gametype.integer == GT_WARZONE)
			NPC_SetNewWarzoneGoalAndPath();
		else
			NPC_SetNewGoalAndPath();

		if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
		{
			if (NPC->enemy)
			{
				NPC->enemy = NULL; // UQ1: Added this to stop massive lag spikes when the player hides somewhere they can't go...

				if (NPC->client->ps.weapon == WP_SABER)
					G_AddVoiceEvent( NPC, Q_irand( EV_JLOST1, EV_JLOST3 ), 15000 + irand(0, 30000) );
				else
				{
					int choice = irand(0,4);

					switch (choice)
					{
					case 0:
						G_AddVoiceEvent( NPC, EV_GIVEUP1, 15000 + irand(0, 30000) );
						break;
					case 1:
						G_AddVoiceEvent( NPC, EV_GIVEUP2, 15000 + irand(0, 30000) );
						break;
					case 2:
						G_AddVoiceEvent( NPC, EV_GIVEUP3, 15000 + irand(0, 30000) );
						break;
					case 3:
						G_AddVoiceEvent( NPC, EV_GIVEUP4, 15000 + irand(0, 30000) );
						break;
					default:
						G_AddVoiceEvent( NPC, EV_LOST1, 15000 + irand(0, 30000) );
						break;
					}
				}
			}
			return qfalse; // next think...
		}
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// Try again to find a path. This time it should ignore wpCurrent and find another start waypoint...
		return qfalse; // next think...
	}

	if ( (NPC->coverpointGoal > 0 && NPC->coverpointGoal < gWPNum)
		&& VectorDistanceNoHeight(gWPArray[NPC->coverpointGoal]->origin, NPC->r.currentOrigin) < 24/*24*/)
	{// We're at out goal! Find a new goal...
		//trap->Print("NPC %i hit it's COVERPOINT goal waypoint!!!\n", NPC->s.number);
		/*
		NPC->wpTravelTime = level.time + 20000;
		NPC->longTermGoal = NPC->wpCurrent = NPC->wpNext = NPC->coverpointOFC;
		NPC->coverpointOFC = level.time + irand(5000, 15000);
		*/
		
		/*
		// Find another cover spot and move there while firing... This *should* hopefully have the affect of making them duck from cover to cover...
		NPC_SetEnemyGoal();

		if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
		{
			NPC->enemy = NULL; // UQ1: Added this to stop massive lag spikes when the player hides somewhere they can't go...
			return qfalse; // next think...
		}
		*/

		return qfalse; // next think...
	}
	else if (VectorDistanceNoHeight(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < 32/*24*/
		|| (NPC->bot_strafe_right_timer > level.time && VectorDistanceNoHeight(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < /*48*/64)
		|| (NPC->bot_strafe_left_timer > level.time && VectorDistanceNoHeight(gWPArray[NPC->longTermGoal]->origin, NPC->r.currentOrigin) < /*48*/64))
	{// We're at out goal! Find a new goal...
		//trap->Print("NPC %i hit it's goal waypoint!!!\n", NPC->s.number);
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->pathsize = -1;
		NPC->longTermGoal = NPC->coverpointOFC = NPC->coverpointGoal = -1;
		return qfalse; // next think...
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		return qfalse; // next think...
	}

	if ((VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 32/*24*/
		|| (NPC->bot_strafe_right_timer > level.time && VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < /*48*/64)
		|| (NPC->bot_strafe_left_timer > level.time && VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < /*48*/64))
		&& NPC->wpNext > 0
		&& (NPC_OrgVisible(gWPArray[NPC->wpNext]->origin, NPC->r.currentOrigin, NPC->s.number) || VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 24)
		/*&& NPC_Humanoid_ClearPathToSpot( gWPArray[NPC->wpNext]->origin, NPC->s.number )*/)
	{// At current node.. Pick next in the list...
		NPC->wpLast = NPC->wpCurrent;
		NPC->wpCurrent = NPC->wpNext;
		NPC->wpNext = NPC_GetNextNode(NPC);

		if (NPC->wpNext < 0 
			&& NPC->wpCurrent != NPC->longTermGoal
			&& NPC->wpNext != NPC->longTermGoal)
			NPC->wpNext = NPC->longTermGoal;

		/*
		if (NPC_HaveValidEnemy())
		{
			trap->Print("Next wps: (lt: %i) (ps: %i) (wc: %i) (wn: %i)\n", NPC->longTermGoal, NPC->pathsize, NPC->wpCurrent, NPC->wpNext);
			if (NPC->wpCurrent > 0 && NPC->wpCurrent <= gWPNum)
				trap->Print("NPC %i hit a waypoint. Pathsize is now %i. Next wp is %i dist is %f.\n", NPC->s.number, NPC->pathsize, NPC->wpCurrent, Distance(NPC->r.currentOrigin, gWPArray[NPC->wpCurrent]->origin));
		}
		*/

		//NPC->wpTravelTime = level.time + 15000; // maximum of 10 seconds to traverse to the next waypoint...
		if (NPC->wpCurrent > 0 && NPC->wpCurrent < gWPNum)
		{
			float dist = Distance(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin);

			if (dist < 1) dist = 1;

			NPC->wpTravelTime = level.time + (dist / 24.0f) * 1000.0f;
		}
		else
			NPC->wpTravelTime = level.time + 15000; // maximum of 10 seconds to traverse to the next waypoint...

		if (NPC_HaveValidEnemy())
			NPC->wpTravelTime = level.time + 5000; // maximum of 5 seconds to traverse to the next waypoint...
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		return qfalse; // next think...
	}

	if ( NPC_HaveValidEnemy() )
	{// Shoot on the run :)
		NPC_FacePosition( NPC->enemy->r.currentOrigin, qtrue );

		if (trap->InPVS(NPC->enemy->r.currentOrigin, NPC->r.currentOrigin) && NPC_CheckVisibility( NPC->enemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV )
			ucmd.buttons |= BUTTON_ATTACK;
	}
	else
	{// Look toward our waypoint and keep moving...
		NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	}

	if ((velocity <= 16 && InFOV3( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->client->ps.viewangles, 30, 180 )) // InFOV so they don't do this while turning...
		|| !DOM_NPC_ClearPathToSpot( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
	{// Trouble moving, need some (currently really really simple) avoidance stuff...
		if (NPC->client->ps.weapon == WP_SABER 
			&& gWPArray[NPC->wpCurrent]->origin[2] > NPC->r.currentOrigin[2]+32
			&& NPC_ClearPathToJump( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
		{
			if (!NPC_TryJump( NPC, gWPArray[NPC->wpCurrent]->origin ))
			{
				if (!NPC_TryJump( NPC, jumpLandPosition ))
				{
					//NPC_Avoidance();
				}
			}
		}
		else
		{
			//NPC_Avoidance();
		}
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		return qfalse; // next think...
	}

	if ( NPC_HaveValidEnemy() )
	{// Shoot on the run :)
		NPC_FacePosition( NPC->enemy->r.currentOrigin, qtrue );
		
		if (trap->InPVS(NPC->enemy->r.currentOrigin, NPC->r.currentOrigin) && NPC_CheckVisibility( NPC->enemy, CHECK_360|CHECK_FOV|CHECK_VISRANGE ) == VIS_FOV )
			ucmd.buttons |= BUTTON_ATTACK;
	}
	else
	{// Look toward our waypoint and keep moving...
		NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );
	}

	if (NPC->client->ps.pm_flags & PMF_DUCKED && NPC->bot_strafe_crouch_timer <= level.time)
	{// Clear any previous flags...
		NPC->client->ps.pm_flags &= ~PMF_DUCKED;
	}

	if (NPC->client->ps.pm_flags & PMF_JUMP_HELD)
	{// Clear any previous flags...
		NPC->client->ps.pm_flags &= ~PMF_JUMP_HELD;
	}

	//
	// UQ1: Note that (ucmd) NPC jumping causes wierd screen shaking in JKA... Best avoided if possible...
	//

	if (NPC->bot_strafe_right_timer > level.time)
	{
		
	}
	else if (NPC->bot_strafe_left_timer > level.time)
	{

	}	
	else
	{
		//NPC_NPCBlockingPath();//Stoiss/Xy no good for npc, make the walk in cyrcle in combat and make awalk delay on them
	}
	
	if ( !NPC_HaveValidEnemy() )
		ucmd.buttons |= BUTTON_WALKING;

	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	UQ1_UcmdMoveForDir( NPC, &ucmd, NPC->movedir );

	return qtrue;
}

qboolean NPC_PatrolArea( void ) 
{// Quick method of patroling...
	vec3_t		velocity_vec;
	float		velocity;

	if (gWPNum <= 0)
	{// No waypoints available...
		return qfalse;
	}

	if (NPC_HaveValidEnemy())
	{// Chase them...
		NPC->return_home = qtrue;
		return NPC_FollowRoutes();
	}
	else if (NPC->return_home)
	{// Returning home after chase and kill...
		return NPC_FollowRoutes();
	}

	if ( !NPC->enemy )
	{
		switch (NPC->client->NPC_class)
		{
		case CLASS_CIVILIAN:
		case CLASS_GENERAL_VENDOR:
		case CLASS_WEAPONS_VENDOR:
		case CLASS_ARMOR_VENDOR:
		case CLASS_SUPPLIES_VENDOR:
		case CLASS_EQUIPMENT_VENDOR:
		case CLASS_FOOD_VENDOR:
		case CLASS_MEDICAL_VENDOR:
		case CLASS_GAMBLER_VENDOR:
		case CLASS_TRADE_VENDOR:
		case CLASS_ODDITIES_VENDOR:
		case CLASS_DRUG_VENDOR:
		case CLASS_TRAVELLING_VENDOR:
			// These guys have no enemies...
			break;
		default:
			if ( NPC->client->enemyTeam != NPCTEAM_NEUTRAL )
			{
				NPC->enemy = NPC_PickEnemyExt( qtrue );

				if (NPC->enemy)
				{
					if (NPC->client->ps.weapon == WP_SABER)
						G_AddVoiceEvent( NPC, Q_irand( EV_JDETECTED1, EV_JDETECTED3 ), 15000 + irand(0, 30000) );
					else
					{
						int choice = irand(0,4);

						switch (choice)
						{
						case 0:
							G_AddVoiceEvent( NPC, EV_DETECTED1, 15000 + irand(0, 30000) );
							break;
						case 1:
							G_AddVoiceEvent( NPC, EV_DETECTED2, 15000 + irand(0, 30000) );
							break;
						case 2:
							G_AddVoiceEvent( NPC, EV_DETECTED3, 15000 + irand(0, 30000) );
							break;
						case 3:
							G_AddVoiceEvent( NPC, EV_DETECTED4, 15000 + irand(0, 30000) );
							break;
						default:
							G_AddVoiceEvent( NPC, EV_DETECTED5, 15000 + irand(0, 30000) );
							break;
						}
					}
				}
			}
			break;
		}
	}

	if (NPC->NPC->conversationPartner)
	{// Chatting with another NPC... Stay still!
		NPC_NPCConversation();

		if (NPC->NPC->conversationPartner)
			NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );

		return qfalse;
	}

	if (!NPC->enemy && !NPC->NPC->conversationPartner)
	{// UQ1: Strange place to do this, but whatever... ;)
		NPC_FindConversationPartner();
	}

	if (NPC->NPC->conversationPartner)
	{// Chatting with another NPC... Stay still!
		NPC_FacePosition( NPC->NPC->conversationPartner->r.currentOrigin, qfalse );
		return qfalse;
	}

	VectorCopy(NPC->client->ps.velocity, velocity_vec);
	velocity = VectorLength(velocity_vec);

	if (!NPC->return_home
		&& (NPC->r.currentOrigin[2] > NPC->spawn_pos[2]+24 || NPC->r.currentOrigin[2] < NPC->spawn_pos[2]-24))
	{// We have fallen... Set this spot as our new patrol location...
		VectorCopy(NPC->r.currentOrigin, NPC->spawn_pos);
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->pathsize = -1;
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->wpTravelTime < level.time)
	{// Patrol Point...
		NPC_FindNewPatrolWaypoint();
		NPC->return_home = qfalse;
		return qfalse; // next think...
	}

	if (VectorDistanceNoHeight(gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin) < 24/*24*/)
	{// We're at out goal! Find a new goal...
		//trap->Print("NPC %i hit it's goal waypoint!!!\n", NPC->s.number);
		NPC->longTermGoal = -1;
		NPC->wpCurrent = -1;
		NPC->pathsize = -1;
		return qfalse; // next think...
	}

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );

	if ((velocity <= 16 && InFOV3( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->client->ps.viewangles, 30, 120 )) // InFOV so they don't do this while turning...
		|| !DOM_NPC_ClearPathToSpot( NPC, gWPArray[NPC->wpCurrent]->origin, -1 ))
	{// Trouble moving, need some (currently really really simple) avoidance stuff...
		NPC_Avoidance();
	}

	if (NPC->wpCurrent < 0 || NPC->wpCurrent >= gWPNum || NPC->longTermGoal < 0 || NPC->longTermGoal >= gWPNum)
	{// FIXME: Try to roam out of problems...
		return qfalse; // next think...
	}

	NPC_FacePosition( gWPArray[NPC->wpCurrent]->origin, qfalse );

	if (NPC->bot_strafe_right_timer > level.time)
	{
		
	}
	else if (NPC->bot_strafe_left_timer > level.time)
	{

	}	
	else
	{
		NPC_NPCBlockingPath();//Stoiss/Xy no good for npc, make the walk in cyrcle in combat and make awalk delay on them
	}

	ucmd.buttons |= BUTTON_WALKING;

	VectorSubtract( gWPArray[NPC->wpCurrent]->origin, NPC->r.currentOrigin, NPC->movedir );
	UQ1_UcmdMoveForDir( NPC, &ucmd, NPC->movedir );

	return qtrue;
}


#endif //__DOMINANCE_NPC__

