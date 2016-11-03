/*
=============================================================================
Unique1's FlagSys Code.
=============================================================================
Spawn_Flag_Base
flag_AI_Think_Axis
flag_AI_Think_Allied
flag_AI_Think
OM_Flag_Spawn_Test
OM_Flag_Spawn
=============================================================================
*/

#include "g_local.h"

extern int OrgVisibleBox(vec3_t org1, vec3_t mins, vec3_t maxs, vec3_t org2, int ignore);
extern qboolean CheckAboveOK_Player(vec3_t origin);
extern qboolean CheckBelowOK(vec3_t origin);
extern qboolean CheckEntitiesInSpot(vec3_t point);
extern qboolean AdvancedWouldTelefrag(vec3_t point);

#ifndef TEAM_NONE
#define TEAM_NONE 0
#endif

//===========================================================================
// Routine      : Spawn_Flag_Base
// Description  : Spawn a base to sit the flagpole on. We might not need this! May be useful for moveable bases on tanks/etc???
void Spawn_Flag_Base ( vec3_t origin )
{// Blue Flag...
	gentity_t* ent = G_Spawn();

	VectorCopy(origin, ent->s.origin);
	G_SetOrigin( ent, origin );

	ent->model = "models/map_objects/mp/flag_base.md3";
	ent->s.modelindex = G_ModelIndex( ent->model );
	ent->targetname = NULL;
	ent->classname = "flag_base";
	ent->s.eType = ET_GENERAL;
	ent->setTime = 0;
	
//	trap->R_ModelBounds(trap->R_RegisterModel( "models/map_objects/mp/flag_base.md3" ), ent->r.mins, ent->r.maxs);

	VectorSet( ent->r.mins, -16, -16, 0 );
	VectorSet( ent->r.maxs, 16, 16, 16 );

	//Drop to floor
	if ( 1 )
	{
		trace_t		tr;
		vec3_t		bottom, saveOrg;

		VectorCopy( ent->s.origin, saveOrg );
		VectorCopy( ent->s.origin, bottom );
		bottom[2] = MIN_WORLD_COORD;
		trap->Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, bottom, ent->s.number, MASK_NPCSOLID, 0, 0, 0 );
		if ( !tr.allsolid && !tr.startsolid && tr.fraction < 1.0 )
		{
			VectorCopy(tr.endpos, ent->s.origin);
			G_SetOrigin( ent, tr.endpos );
		}
	}

	ent->r.contents = CONTENTS_SOLID|CONTENTS_OPAQUE|CONTENTS_BODY|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP;//CONTENTS_SOLID;

	trap->LinkEntity ((sharedEntity_t *)ent);
}

//===========================================================================
// Routine      : flag_AI_Think_Axis
// Description  : Main think function for flags.
void PreCalculate_Flag_Spawnpoints( int flagnum, vec3_t angles, vec3_t origin );

extern qboolean flag_file_loaded;
extern int number_of_flags; // Current number of warzone flags on this map...
#ifdef __UNUSED__
qboolean flag_spawnpoints_done = qfalse;
#else //!__UNUSED__
qboolean flag_spawnpoints_done = qtrue;
#endif //__UNUSED__

void Calculate_Warzone_Flag_Spawns ( void )
{
#ifdef __UNUSED__
	if (!flag_spawnpoints_done)
	{// Have we initialized spawnpoints yet?
		int z = 0;

		for (z=0;z<number_of_flags;z++)
		{
			gentity_t *flagent = flag_list[z].flagentity;
				
			PreCalculate_Flag_Spawnpoints( flagent->count, flagent->s.angles, flagent->s.origin );
			flagent->alt_fire = qtrue;
		}
		flag_spawnpoints_done = qtrue;
	}
#endif //__UNUSED__
}

qboolean EntityVisible(gentity_t *from, gentity_t *ent)
{
	/*
	vec3_t		ent_org, from_org;
	trace_t		tr;
	gentity_t	*enemy = NULL;

	if (!from || !ent)
		return qfalse;

	VectorCopy(from->r.currentOrigin, from_org);
	from_org[2]+=24;

	VectorCopy(ent->r.currentOrigin, ent_org);
	ent_org[2]+=24;

	trap->Trace( &tr, from_org, NULL, NULL, ent_org, from->s.number, MASK_SHOT );

	if (tr.entityNum < 0 || tr.entityNum > ENTITYNUM_MAX_NORMAL)
		return qfalse;

	enemy = &g_entities[tr.entityNum];

	if (enemy != ent)
		return qfalse;
	*/
	return qtrue;
}


void Warzone_Flag_Think( gentity_t *ent )
{
	vec3_t mins;
	vec3_t maxs;
	int touch[MAX_GENTITIES];
	int num = 0;
	int i = 0;
	int radius = ent->s.otherEntityNum2;

	ent->s.eFlags |= EF_RADAROBJECT;

#ifdef __UNUSED__
	if (flag_file_loaded)
	{
		if (!flag_spawnpoints_done)
		{// Have we initialized spawnpoints yet?
			int z = 0;

			for (z=0;z<number_of_flags;z++)
			{
				gentity_t *flagent = flag_list[z].flagentity;
				
				PreCalculate_Flag_Spawnpoints( flagent->count, flagent->s.angles, flagent->s.origin );
				//trap->Print("Flagnum is %i. flagent->s.otherEntityNum is %i\n", flagent->count, flagent->s.otherEntityNum);
				flagent->alt_fire = qtrue;
			}
			flag_spawnpoints_done = qtrue;
		}
	}
	else if (!ent->alt_fire)
	{// Have we initialized spawnpoints yet?
		//if (level.numConnectedClients > 0)
		{
			PreCalculate_Flag_Spawnpoints( ent->count, ent->s.angles, ent->s.origin );
			//trap->Print("Flagnum is %i. ent->s.otherEntityNum is %i\n", ent->count, ent->s.otherEntityNum);
			ent->alt_fire = qtrue;
			flag_spawnpoints_done = qtrue;
		}
	}
#endif //__UNUSED__

	if (ent->nextthink > level.time)
		return;

	mins[0] = 0-radius;
	mins[1] = 0-radius;
	mins[2] = 0-(radius*0.5);

	maxs[0] = radius;
	maxs[1] = radius;
	maxs[2] = radius*0.5;

	VectorAdd( mins, ent->s.origin, mins );
	VectorAdd( maxs, ent->s.origin, maxs );

	// Run the think... Look for captures...
	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i=0 ; i<num ; i++ ) 
	{
		gentity_t *hit = &g_entities[touch[i]];

		if (!hit)
			continue;

		if (!hit->client)
			continue;

		if (!hit->health || hit->health <= 0)
			continue;

		if (Q_stricmp (hit->classname, "NPC_Vehicle") == 0)
			continue;

		//if (hit->s.eType == ET_NPC)
		//	hit->s.teamowner = hit->client->playerTeam;

		if (!EntityVisible(ent, hit))
		{
			hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = 0;
			continue;
		}

		if (ent->s.teamowner == TEAM_RED)
		{// Red flag...
			if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) && hit->s.teamowner == TEAM_RED)
			{// Red player consolidating a red flag for spawns... 
				//trap->Print("Consolidating flag!\n");

				ent->s.time2++;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				if (ent->s.time2 >= 100)
				{// Finished consolidating.. Set new team to blue and initialize...
					ent->s.teamowner = TEAM_RED;
					ent->s.time2 = 100;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
			else if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) && hit->s.teamowner == TEAM_BLUE)
			{// Blue player undoing red flag...
				//trap->Print("Uncapturing flag!\n");

				ent->s.time2--;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				if (ent->s.time2 <= 0)
				{// Finished capture.. Set new team to blue...
					ent->s.teamowner = TEAM_NONE;
					ent->s.time2 = 0;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
		}
		else if (ent->s.teamowner == TEAM_BLUE)
		{// Blue flag...
			if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) && hit->s.teamowner == TEAM_BLUE)
			{// Blue player consolidating a blue flag for spawns... 
				//trap->Print("Consolidating flag!\n");

				ent->s.time2++;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				if (ent->s.time2 >= 100)
				{// Finished consolidating.. Set new team to blue and initialize...
					ent->s.teamowner = TEAM_BLUE;
					ent->s.time2 = 100;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
			else if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) && hit->s.teamowner == TEAM_RED)
			{// Red player undoing blue flag...
				//trap->Print("Uncapturing flag!\n");

				ent->s.time2--;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				if (ent->s.time2 <= 0)
				{// Finished capture.. Set new team to blue...
					ent->s.teamowner = TEAM_NONE;
					ent->s.time2 = 0;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
		}
		else
		{// Neutral flag...
			if (!ent->s.genericenemyindex || ent->s.genericenemyindex == TEAM_NONE)
			{// Start capture... Set capturing team number...
				ent->s.genericenemyindex = hit->s.teamowner;
				ent->s.time2 = 0;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;
			}
			else if (ent->s.genericenemyindex == hit->s.teamowner)
			{// Continuing partial capture...
				ent->s.time2++;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				// Set the current capturing team number of a neutral flag..
				ent->s.genericenemyindex = hit->s.teamowner;

				//trap->Print("Capturing flag!\n");

				if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) 
					&& hit->s.teamowner == TEAM_BLUE && ent->s.time2 >= 50)
				{// Blue player capturing neutral flag... Finished capture.. Set new team to blue...
					ent->s.teamowner = TEAM_BLUE;
					ent->s.time2 = 0;
					ent->s.genericenemyindex = TEAM_NONE;
				}
				else if ((hit->s.eType == ET_PLAYER || hit->s.eType == ET_NPC) 
					&& hit->s.teamowner == TEAM_RED && ent->s.time2 >= 50)
				{// Red player capturing neutral flag... Finished capture.. Set new team to red...
					ent->s.teamowner = TEAM_RED;
					ent->s.time2 = 0;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
			else if (ent->s.genericenemyindex != hit->s.teamowner)
			{// Undoing partial capture...
				ent->s.time2--;
				hit->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = ent->s.number;

				//trap->Print("Undoing owned flag!\n");

				if (ent->s.time2 <= 0)
				{// Initialize back to untouched flag...
					ent->s.teamowner = TEAM_NONE;
					ent->s.time2 = 0;
					ent->s.genericenemyindex = TEAM_NONE;
				}
			}
		}
	}

	// Set next think...
	ent->nextthink = level.time + 500;
}

qboolean spots_filled[1024];

void AddFlag_Spawn ( int flagnum, vec3_t origin, vec3_t angles )
{// Add a new spawn to the list for this flag...
	if (flag_list[flagnum].num_spawnpoints < 32)
	{// Create a new spawn point.
		//trap->Print("A new spawn point has been extrapolated. ^1(^5#^7%i^1)^5\n", SP_Spawns);
		VectorCopy( origin, flag_list[flagnum].spawnpoints[flag_list[flagnum].num_spawnpoints] ); // Copy this npc's location to the end of the list.
		VectorCopy( angles, flag_list[flagnum].spawnangles[flag_list[flagnum].num_spawnpoints] ); // Copy this npc's location to the end of the list.
		flag_list[flagnum].num_spawnpoints++;
	}
	else
	{
		if (!spots_filled[flagnum])
		{// Finished making spawns for this flag...
			//trap->Print("Calculated 32 spawns for flag #%i.\n", flagnum);
			spots_filled[flagnum] = qtrue;
		}
	}
}

qboolean TooCloseToOtherSpawnpoint ( int flagnum, vec3_t origin )
{
	int i = 0;

	for (i = 0;i < flag_list[flagnum].num_spawnpoints;i++)
	{
		if (Distance(flag_list[flagnum].spawnpoints[i], origin) < 64)
			return qtrue;
	}

	return qfalse;
}

extern int NAV_FindClosestWaypointForPoint2( vec3_t point );
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int GetNearestVisibleWP(vec3_t org, int ignore);

void PreCalculate_Flag_Spawnpoints( int flagnum, vec3_t angles, vec3_t origin )
{
#ifdef __UNUSED__
	vec3_t fwd, point;
	int tries = 0, tries2 = 0;
	//qboolean visible = qfalse;
	qboolean alt = qfalse;

	origin[2]+=32;
	VectorCopy(origin, point);
		
	AngleVectors( angles, fwd, NULL, NULL );

	while (1)//visible == qfalse)
	{// In case we need to try a second spawnpoint.
		vec3_t playerMins, playerMaxs;

		playerMins[0] = -15;
		playerMins[1] = -15;
		playerMins[2] = -1;
		playerMaxs[0] = 15;
		playerMaxs[1] = 15;
		playerMaxs[2] = 64;//96;//1;

		tries = 0;
		tries2 = 0;

		while (tries < 16)
		{
			tries++;
			tries2 = 0;

			while (tries2 < 128)
			{
				int num_tries; // For secondary spawns. (Behind point).

				tries2++;

				num_tries = tries2;

				if (tries2 <= 16)
				{
				}
				else if (tries2 <= 32)
				{
					num_tries-=16;
				}
				else if (tries2 <= 48)
				{
					num_tries-=32;
				}
				else
				{
					num_tries-=48;
				}
				
				VectorCopy(origin, point);

				if (alt)
				{
					if (tries2 <= 8)
					{
						point[0] -= 1+(tries*64);
						point[1] -= 1+(num_tries*64);
					}
					else if (tries2 <= 16)
					{
						point[0] -= 1+(tries*64);
						point[1] -= 1-(num_tries*64);
					}
					else if (tries2 <= 24)
					{
						point[0] -= 1-(tries*64);
						point[1] -= 1+(num_tries*64);
					}
					else
					{
						point[0] -= 1-(tries*64);
						point[1] -= 1-(num_tries*64);
					}
				}
				else
				{
					if (tries2 <= 8)
					{
						point[0] += 1+(tries*64);
						point[1] += 1+(num_tries*64);
					}
					else if (tries2 <= 16)
					{
						point[0] += 1+(tries*64);
						point[1] += 1-(num_tries*64);
					}
					else if (tries2 <= 24)
					{
						point[0] += 1-(tries*64);
						point[1] += 1+(num_tries*64);
					}
					else
					{
						point[0] += 1-(tries*64);
						point[1] += 1-(num_tries*64);
					}
				}

				//if (CheckAboveOK_Player(point))
				//	point[2] += 32;
				//else
				//	continue;

				//point[2] += 16;

				if (OrgVisibleBox(origin, playerMins, playerMaxs, point, flag_list[flagnum].flagentity->s.number)
					&& CheckBelowOK(point)
					&& !AdvancedWouldTelefrag(point) 
					&& !CheckEntitiesInSpot(point) 
					&& VectorDistance(point, origin) > 128
					&& !TooCloseToOtherSpawnpoint(flagnum, point))
				{
					//trap->Print("Adding spawn at %f %f %f.\n", point[0], point[1], point[2]);
					AddFlag_Spawn(flagnum, point, angles);
					//visible = qtrue;

					if (flag_list[flagnum].num_spawnpoints >= 64)
					{
						//trap->Print("Have 64 spawns!\n");
						trap->Print("^3*** ^3Warzone^5: Added ^7%i^5 spawnpoints at flag #^7%i^5.\n", flag_list[flagnum].num_spawnpoints, flagnum);
						return;
					}
				}
			}
		}

		if (alt)
			break;
		else
			alt = qtrue;
	}

	trap->Print("^3*** ^3Warzone^5: Added ^7%i^5 spawnpoints at flag #^7%i^5.\n", flag_list[flagnum].num_spawnpoints, flagnum);
#endif //__UNUSED__
}

int number_of_flags = 0; // Automatically number the flags if not specified by map makers...

void SP_Spawn_Warzone_Flag ( gentity_t* ent )
{// Spawn Warzone flag from map entity
	int radius = 0, teamowner = TEAM_NONE;
	int flagnum = -1;
	char *model, *model2, *fullname;

	G_SpawnString("model", "models/flags/r_flag.md3", &model); // Custom red model..
	G_SpawnString("model2", "models/flags/b_flag.md3", &model2); // Custom blue model..
	G_SpawnString("fullname", "models/flags/n_flag.md3", &fullname); // Custom neutral model.
	G_SpawnInt("radius", "500", &radius); // Point's capture radius..
	G_SpawnInt("teamowner", "0", &teamowner); // Point's capture radius..
	G_SpawnInt("teamuser", "-1", &flagnum); // Map's flag number.. For linking things to this flag based on team...

	if (flagnum < 0)
	{// We dont have a flag number.. Generate them for this map...
		flagnum = number_of_flags;
	}

	ent->s.otherEntityNum2 = radius; // Set capture radius...
	ent->s.otherEntityNum = flagnum; // Flag number...

	if (model[0] != '0')
	{// Red flag model..
		ent->model = model;
		//RegisterAsModel(model);
		ent->s.modelindex = G_ModelIndex( ent->model );
	}
	else
		ent->s.modelindex = -1;

	if (model2[0] != '0')
	{// Blue flag model..
		ent->model2 = model2;
		//RegisterAsModel(model2);
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
	}
	else
		ent->s.modelindex2 = -1;

	if (fullname[0] != '0')
	{// Neutral flag model..
		ent->fullName = fullname;
		//RegisterAsModel(fullname);
		ent->s.activeForcePass = G_ModelIndex( ent->fullName );
	}
	else
		ent->s.activeForcePass = -1;

	VectorSet( ent->r.mins, -8, -8, 0 );
	VectorSet( ent->r.maxs, 8, 8, 96 );

	ent->s.origin[2]+=64;
	G_SetOrigin( ent, ent->s.origin );

	if (Q_stricmp (model, "models/flags/r_flag.md3") == 0)
		Spawn_Flag_Base(ent->s.origin);

	//Drop to floor
	if ( 1 )
	{
		trace_t		tr;
		vec3_t		bottom, saveOrg;

		VectorCopy( ent->s.origin, saveOrg );
		VectorCopy( ent->s.origin, bottom );
		bottom[2] = MIN_WORLD_COORD;
		trap->Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, bottom, ent->s.number, MASK_NPCSOLID, 0, 0, 0 );
		if ( !tr.allsolid && !tr.startsolid && tr.fraction < 1.0 )
		{
			VectorCopy(tr.endpos, ent->s.origin);
			G_SetOrigin( ent, tr.endpos );
		}
	}

	ent->s.teamowner = teamowner;

	if (teamowner == TEAM_RED)
	{
		ent->s.time2 = 100;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^1TEAM_RED^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}
	else if (teamowner == TEAM_BLUE)
	{
		ent->s.time2 = 100;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^4TEAM_BLUE^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}
	else
	{
		ent->s.time2 = 0;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^6TEAM_NEUTRAL^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}

	ent->targetname = NULL;
	ent->classname = "flag";
	ent->s.eType = ET_FLAG;
	ent->think = Warzone_Flag_Think;

	VectorSet( ent->r.mins, -8, -8, 0 );
	VectorSet( ent->r.maxs, 8, 8, 96 );
//	trap->R_ModelBounds(trap->R_RegisterModel( "models/flags/r_flag.md3" ), ent->r.mins, ent->r.maxs);

	ent->r.contents = CONTENTS_SOLID|CONTENTS_OPAQUE|CONTENTS_BODY|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP;//CONTENTS_SOLID;

	ent->nextthink = level.time + 1;

	ent->s.eFlags |= EF_RADAROBJECT;

	trap->LinkEntity ((sharedEntity_t *)ent);

	flag_list[flagnum].flagentity = ent;
	flag_list[flagnum].num_spawnpoints = 0;
	ent->count = flagnum;
	
	ent->alt_fire = qfalse;
	number_of_flags++;
}

void SP_misc_control_point (gentity_t* ent)
{
	if (g_gametype.integer != GT_WARZONE)
	{
		G_FreeEntity(ent);
		return;
	}

	SP_Spawn_Warzone_Flag(ent);
}

void Spawn_Scenario_Flag_Auto ( vec3_t origin, int teamowner )
{// Red Flag...
	gentity_t* ent = G_Spawn();
	int radius = 0;//, teamowner = TEAM_NONE;
	int flagnum = 0;
	char *model, *model2, *fullname;

	G_SpawnString("model", "models/flags/r_flag.md3", &model); // Custom red model..
	G_SpawnString("model2", "models/flags/b_flag.md3", &model2); // Custom blue model..
	G_SpawnString("fullname", "models/flags/n_flag.md3", &fullname); // Custom neutral model.
	G_SpawnInt("radius", "500", &radius); // Point's capture radius..
	//G_SpawnInt("teamowner", "0", &teamowner); // Point's capture radius..
	G_SpawnInt("teamuser", "-1", &flagnum); // Map's flag number.. For linking things to this flag based on team...

	if (!radius)
	{// Default radius...
		ent->s.otherEntityNum2 = 500; // Set capture radius...
	}
	else
	{
		ent->s.otherEntityNum2 = radius; // Set capture radius...
	}

	if (flagnum < 0)
	{// We dont have a flag number.. Generate them for this map...
		flagnum = number_of_flags;
		ent->count = flagnum;
		//number_of_flags++;
	}

	ent->s.otherEntityNum = flagnum; // Flag number...

//	if (model != "")
//	{// Red flag model..
		ent->model = model;
		//RegisterAsModel(model);
		ent->s.modelindex = G_ModelIndex( ent->model );
//	}
//	else
//		ent->s.modelindex = -1;

//	if (model2 != "")
//	{// Blue flag model..
		ent->model2 = model2;
		//RegisterAsModel(model2);
		ent->s.modelindex2 = G_ModelIndex( ent->model2 );
//	}
//	else
//		ent->s.modelindex2 = -1;

//	if (fullname != "")
//	{// Neutral flag model..
		ent->fullName = fullname;
		//RegisterAsModel(fullname);
		ent->s.activeForcePass = G_ModelIndex( ent->fullName );
//	}
//	else
//		ent->s.activeForcePass = -1;

	VectorSet( ent->r.mins, -8, -8, 0 );
	VectorSet( ent->r.maxs, 8, 8, 96 );

	VectorCopy(origin, ent->s.origin);
	ent->s.origin[2]+=64;
	G_SetOrigin( ent, ent->s.origin );

	if (Q_stricmp (model, "models/flags/r_flag.md3") == 0)
		Spawn_Flag_Base(ent->s.origin);

	//Drop to floor
	if ( 1 )
	{
		trace_t		tr;
		vec3_t		bottom, saveOrg;

		VectorCopy( ent->s.origin, saveOrg );
		VectorCopy( ent->s.origin, bottom );
		bottom[2] = MIN_WORLD_COORD;
		trap->Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, bottom, ent->s.number, MASK_NPCSOLID, 0, 0, 0 );
		if ( !tr.allsolid && !tr.startsolid && tr.fraction < 1.0 )
		{
			VectorCopy(tr.endpos, ent->s.origin);
			G_SetOrigin( ent, tr.endpos );
		}
	}

	ent->s.teamowner = teamowner;

	if (teamowner == TEAM_RED)
	{
		ent->s.time2 = 100;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^1TEAM_RED^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}
	else if (teamowner == TEAM_BLUE)
	{
		ent->s.time2 = 100;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^4TEAM_BLUE^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}
	else
	{
		ent->s.time2 = 0;
		trap->Print("^4*** ^5Flag at ^7%f %f %f^5 is ^6TEAM_NEUTRAL^5! Radius is ^3%i^5.\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2], radius);
	}

	ent->targetname = NULL;
	ent->classname = "flag";
	ent->s.eType = ET_FLAG;
	ent->think = Warzone_Flag_Think;

//	trap->R_ModelBounds(trap->R_RegisterModel( "models/flags/r_flag.md3" ), ent->r.mins, ent->r.maxs);

	ent->r.contents = CONTENTS_SOLID|CONTENTS_OPAQUE|CONTENTS_BODY|CONTENTS_MONSTERCLIP|CONTENTS_BOTCLIP;//CONTENTS_SOLID;

	ent->nextthink = level.time + 1;

	//ent->s.eFlags |= EF_RADAROBJECT;

	trap->LinkEntity ((sharedEntity_t *)ent);

	flag_list[flagnum].flagentity = ent;
	flag_list[flagnum].num_spawnpoints = 0;

	ent->count = flagnum;
	
	ent->alt_fire = qfalse;
	number_of_flags++;
}

gentity_t *EnemyCapturingFlag ( gentity_t *self, int flag_entitynum )
{// Should return an enemy that is trying to capture the same flag as us...
	int client = 0;

	for (client = 0; client < MAX_GENTITIES; client++)
	{
		gentity_t *ent = &g_entities[client];

		if (!ent)
			continue;

		if (!ent->client)
			continue;

		if (ent->health <= 0)
			continue;

		if (ent->s.eType != ET_NPC && ent->s.eType != ET_PLAYER)
			continue;

		if (ent->client->ps.stats[STAT_CAPTURE_ENTITYNUM] != flag_entitynum)
			continue;

		if (OnSameTeam(ent, self))
			continue;

		// OK, we found an enemy trying to capture this flag! Return it!
		return ent;
	}

	return NULL;
}

int WARZONE_GetNumberOfBlueFlags()
{
	int z = 0;
	int count = 0;

	for (z = 0; z < number_of_flags; z++)
	{
		gentity_t *flagent = flag_list[z].flagentity;

		if (flagent->s.teamowner == TEAM_BLUE)
			count++;
	}

	//trap->Print("%i flags - %i blue.\n", number_of_flags, count);
	return count;
}

int WARZONE_GetNumberOfRedFlags()
{
	int z = 0;
	int count = 0;

	for (z = 0; z < number_of_flags; z++)
	{
		gentity_t *flagent = flag_list[z].flagentity;

		if (flagent->s.teamowner == TEAM_RED)
			count++;
	}

	//trap->Print("%i flags - %i red.\n", number_of_flags, count);
	return count;
}

gentity_t *WARZONE_FindGoalForTeam( int TEAM )
{
	int			z = 0;
	gentity_t	*flags[256];
	int			count = 0;

	for (z = 0; z < number_of_flags; z++)
	{
		gentity_t *flagent = flag_list[z].flagentity;

		if (flagent->s.teamowner != TEAM)
		{// Add any we find to the list...
			flags[count] = flagent;
			count++;
		}
	}

	if (count <= 0) return NULL;

	// Return one at random...
	return flags[irand(0, count-1)];
}