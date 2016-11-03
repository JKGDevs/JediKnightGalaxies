/*
=============================================================================
Unique1's Experimental OM SpawnSys Code.
=============================================================================
OrgVisibleBox
CheckAboveOK_Player
CheckBelowOK
CheckEntitiesInSpot
Extrapolate_Advanced_Spawnpoint
=============================================================================
*/

#include "g_local.h"
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];
extern int gWPNum;

extern int OrgVisibleBox(vec3_t org1, vec3_t mins, vec3_t maxs, vec3_t org2, int ignore);
void NPC_SelectWarzoneSpawnpoint ( int TEAM ); // below...
extern gentity_t *SelectRandomDeathmatchSpawnPoint( void );

qboolean CheckAboveOK_Player(vec3_t origin) // For player/npc/bot spawns!
{// Check directly above a point for clearance.
	trace_t tr;
	vec3_t up, mins, maxs;

	mins[0] = -16;
	mins[1] = -16;
	mins[2] = -16;
	maxs[0] = 16;
	maxs[1] = 16;
	maxs[2] = 16;

	VectorCopy(origin, up);

	up[2] += 4096;

	trap->Trace(&tr, origin, mins, maxs, up, ENTITYNUM_NONE, MASK_SOLID, 0, 0, 0); // Look for ground.

	VectorSubtract(origin, tr.endpos, up);

	if (up[2] <= 96
		&& tr.fraction == 1)
		return qfalse; // No room above!
	else
		return qtrue; // All is ok!
}

qboolean CheckBelowOK(vec3_t origin)
{// Check directly below us.
	trace_t tr;
	vec3_t down, mins, maxs;

	mins[0] = -12;
	mins[1] = -12;
	mins[2] = -12;
	maxs[0] = 12;
	maxs[1] = 12;
	maxs[2] = 12;

	VectorCopy(origin, down);

	down[2] -= 128;

	trap->Trace(&tr, origin, mins, maxs, down, ENTITYNUM_NONE, MASK_PLAYERSOLID, 0, 0, 0); // Look for ground.

	VectorSubtract(origin, tr.endpos, down);

	if (tr.fraction == 1.0f || tr.startsolid || tr.allsolid || tr.fraction > 0.4f)
		return qfalse; // Long way down!
	else
		return qtrue; // All is ok!
}

qboolean AdvancedWouldTelefrag(vec3_t point)
{
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	vec3_t		mins, maxs;
	vec3_t	playerMins = {-12, -12, DEFAULT_MINS_2};
	vec3_t	playerMaxs = {12, 12, DEFAULT_MAXS_2};

	if (!point)
		return qtrue;

	VectorAdd( point, playerMins, mins );
	VectorAdd( point, playerMaxs, maxs );

	num = trap->EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) 
	{
		hit = &g_entities[touch[i]];

		if ( hit->client ) 
		{
			return qtrue;
		}

#ifdef __NPC__
		if ( hit->NPC_client ) 
		{
			return qtrue;
		}
#endif //__NPC__

		if (hit->r.contents)
		{
			if (hit->r.contents&MASK_SOLID)
				return qtrue;
		}
	}

	return qfalse;
}

qboolean CheckEntitiesInSpot(vec3_t point)
{// Any entities too close to us?
	int entitynum = 0;
	qboolean foundbad = qfalse;

	while (entitynum < MAX_CLIENTS)
	{// Find a clear point.
		gentity_t *ent = &g_entities[entitynum];

		if (!ent)
			continue;

		//if (ent->s.solid != SOLID_BMODEL && !(ent->r.contents&CONTENTS_MONSTERCLIP) && !(ent->r.contents&CONTENTS_BOTCLIP) && !(ent->r.contents&CONTENTS_SOLID))
		//	continue;

		if (Distance(point, ent->r.currentOrigin) < 128 || Distance(point, ent->s.origin) < 128)
		{// Bad point.
			return qtrue;
		}
		
		if (entitynum < MAX_CLIENTS 
			&& ent->client 
			//&& ent->client->ps.origin
			&& Distance(point, ent->client->ps.origin) < 128)
		{// Bad point.
			return qtrue;
		}

#ifdef __NPC__
		if (ent->NPC_client 
			&& Distance(point, ent->NPC_client->ps.origin) < 128)
		{// Bad point.
			return qtrue;
		}
#endif //__NPC__

		// Bad point?
		entitynum++;
	}

	return foundbad;
}

qboolean Extrapolate_Advanced_Spawnpoint( vec3_t original_origin, vec3_t newspawn )
{// Will output newspawn for spawnpoint extrapolation from a single point... (Eg: Up to 64 players(guess) or so per origin).
	vec3_t point;
	qboolean visible = qfalse;
	int tries = 0, tries2 = 0;

	VectorCopy(original_origin, point);
		
	while (visible == qfalse && tries < 8)
	{
		vec3_t	playerMins = {-18, -18, -24};
		vec3_t	playerMaxs = {18, 18, 24};

		tries++;
		tries2 = 0;

		while (visible == qfalse && tries2 < 16)
		{
			int num_tries; // For secondary spawns. (Behind point).

			tries2++;

			num_tries = tries2;
				
			if (num_tries > 8)// Secondary Set.
				num_tries -= 8;

			VectorCopy(original_origin, point);

			if (num_tries <= 4)
			{
				point[0] += (64*8)-(tries*64);
				point[1] += (64*8)-(num_tries*64);
			}
			else if (num_tries <= 8)
			{
				point[0] += (64*8)-(tries*64);
				point[1] -= (64*8)-(num_tries*64);
			}
			else if (num_tries <= 12)
			{
				point[0] -= (64*8)-(tries*64);
				point[1] += (64*8)-(num_tries*64);
			}
			else
			{
				point[0] -= (64*8)-(tries*64);
				point[1] -= (64*8)-(num_tries*64);
			}

			if (!CheckAboveOK_Player(point))
				continue;

			if (OrgVisibleBox(original_origin, playerMins, playerMaxs, point, -1)
				&& CheckBelowOK(point)
				&& !AdvancedWouldTelefrag(point) 
				&& !CheckEntitiesInSpot(point) )
			{
				visible = qtrue;
				break;
			}
		}
	}

	if (!visible)
	{// Try another spawnpoint.
		return qfalse;
	}

	VectorCopy(point, newspawn);

	return qtrue;
}

//
// Spawning at flags!
//

/*
===========
GetNumberOfWarzoneFlags

Returns the number of Warzone flags for this map.
============
*/
extern int number_of_flags;

int GetNumberOfWarzoneFlags ( void )
{
	/*
	int num_flags = 0;

	for (num_flags = 0; num_flags < 1024; num_flags++)
	{
#ifdef __UNUSED__
		if (!flag_list[num_flags].num_spawnpoints)
			break;
#else //!__UNUSED__
		if (!flag_list[num_flags].flagentity)
			break;
#endif //__UNUSED__
	}
	num_flags--;
	
	return num_flags;*/

	return number_of_flags;
}

/*
===========
WarzoneSpawnpointAvailable

Returns qtrue if there is one available to use...
============
*/

qboolean WarzoneSpawnpointAvailable ( gentity_t *ent )
{
	if ( !level.intermissiontime )
	{// Spawn around flag...
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		{
/*			int spawnpoint_num = 0;
			int num_flags = 0;
			int test_flag = 0;
			int flagnum = -1;
			int last_flag_num = -1;
			qboolean notgood = qtrue;
			vec3_t newspawn;

			num_flags = GetNumberOfWarzoneFlags();

			if (num_flags >= 2)
			{// We have flags on the map... Find one that belongs to us..
				float best_dist = 64000.0f;
				qboolean redfound = qfalse;
				qboolean bluefound = qfalse;

				for (test_flag = 0; test_flag <= num_flags; test_flag++)
				{// Check we have a playable map...
					if (flag_list[test_flag].flagentity)
					{
						if (flag_list[test_flag].flagentity->s.modelindex == TEAM_RED)
							redfound = qtrue;
						if (flag_list[test_flag].flagentity->s.modelindex == TEAM_BLUE)
							bluefound = qtrue;
					}
				}

				if (!redfound)
				{
					flag_list[num_flags].flagentity->s.modelindex = TEAM_RED;
				}

				if (!bluefound)
				{
					flag_list[0].flagentity->s.modelindex = TEAM_BLUE;
				}

				for (test_flag = 0; test_flag <= num_flags; test_flag++)
				{// We need to find the most forward flag point to spawn at... FIXME: Selectable spawnpoints in UI...
					if (flag_list[test_flag].flagentity)
					{
						if (flag_list[test_flag].num_spawnpoints > 0
							&& (flag_list[test_flag].flagentity->s.modelindex == ent->client->sess.sessionTeam || ent->client->sess.sessionTeam == TEAM_SPECTATOR))
						{// This is our flag...
							int test = 0;

							for (test = 0; test < num_flags; test++)
							{// Find enemy flags...
								if (flag_list[test].flagentity)
								{// FIXME: Selectable spawnpoints in UI...
									if (flag_list[test].flagentity->s.modelindex != ent->client->sess.sessionTeam
										&& flag_list[test].flagentity->s.modelindex != 0)
									{// This is our enemy's flag...
										if (VectorDistance(flag_list[test_flag].flagentity->s.origin, flag_list[test].flagentity->s.origin) < best_dist)
										{
											if (flagnum != -1)
												last_flag_num = flagnum;

											flagnum = test_flag;
											best_dist = VectorDistance(flag_list[test_flag].flagentity->s.origin, flag_list[test].flagentity->s.origin);
										}
									}
								}
							}
						}
					}
				}

				spawnpoint_num = -1;

				if ((ent->client->ps.teamNum == TEAM_RED && flag_list[flagnum].associated_red_spawnpoints_number > 32) || (ent->client->ps.teamNum == TEAM_BLUE && flag_list[flagnum].associated_blue_spawnpoints_number > 32))
				{// Use flag's associated spawnpoints first..
					if (ent->client->ps.teamNum == TEAM_RED)
					{
						while (notgood && spawnpoint_num < flag_list[flagnum].associated_red_spawnpoints_number)
						{
							spawnpoint_num++;		

							if (!AdvancedWouldTelefrag(flag_list[flagnum].associated_red_spawnpoints[spawnpoint_num])
								&& !CheckEntitiesInSpot(flag_list[flagnum].associated_red_spawnpoints[spawnpoint_num]))
							{
								//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 associated point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
								notgood = qfalse;
								VectorCopy(flag_list[flagnum].associated_red_spawnpoints[spawnpoint_num], newspawn);
							}
						}
					}
					else
					{
						while (notgood && spawnpoint_num < flag_list[flagnum].associated_blue_spawnpoints_number)
						{
							spawnpoint_num++;		

							if (!AdvancedWouldTelefrag(flag_list[flagnum].associated_blue_spawnpoints[spawnpoint_num])
								&& !CheckEntitiesInSpot(flag_list[flagnum].associated_blue_spawnpoints[spawnpoint_num]))
							{
								//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 associated point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
								notgood = qfalse;
								VectorCopy(flag_list[flagnum].associated_blue_spawnpoints[spawnpoint_num], newspawn);
							}
						}
					}
				}

				if (notgood)
				{// Use spawnpoint at the flag for fallback...
					spawnpoint_num = -1;

					while (notgood && spawnpoint_num < flag_list[flagnum].num_spawnpoints)
					{
						spawnpoint_num++;	

						if (!AdvancedWouldTelefrag(flag_list[flagnum].spawnpoints[spawnpoint_num])
							&& !CheckEntitiesInSpot(flag_list[flagnum].spawnpoints[spawnpoint_num]))
						{
							//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
							notgood = qfalse;
							VectorCopy(flag_list[flagnum].spawnpoints[spawnpoint_num], newspawn);
						}
					}
				}

				if (notgood)
				{// Use an associated spawnpoint...
					spawnpoint_num = -1;

					if (ent->client->ps.teamNum == TEAM_RED)
					{
						while (notgood && spawnpoint_num < flag_list[flagnum].unassociated_red_spawnpoints_number)
						{
							spawnpoint_num++;		

							if (!AdvancedWouldTelefrag(flag_list[flagnum].unassociated_red_spawnpoints[spawnpoint_num])
								&& !CheckEntitiesInSpot(flag_list[flagnum].unassociated_red_spawnpoints[spawnpoint_num]))
							{
								//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 unassociated point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
								notgood = qfalse;
								VectorCopy(flag_list[flagnum].unassociated_red_spawnpoints[spawnpoint_num], newspawn);
							}
						}
					}
					else
					{
						while (notgood && spawnpoint_num < flag_list[flagnum].unassociated_blue_spawnpoints_number)
						{
							spawnpoint_num++;		

							if (!AdvancedWouldTelefrag(flag_list[flagnum].unassociated_blue_spawnpoints[spawnpoint_num])
								&& !CheckEntitiesInSpot(flag_list[flagnum].unassociated_blue_spawnpoints[spawnpoint_num]))
							{
								//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 unassociated point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
								notgood = qfalse;
								VectorCopy(flag_list[flagnum].unassociated_blue_spawnpoints[spawnpoint_num], newspawn);
							}
						}
					}
				}
				
				if (notgood)
				{// Try previous (further) flag if desperate...
					spawnpoint_num = -1;

					while (notgood && spawnpoint_num < flag_list[last_flag_num].num_spawnpoints)
					{
						spawnpoint_num++;	

						if (!AdvancedWouldTelefrag(flag_list[last_flag_num].spawnpoints[spawnpoint_num])
							&& !CheckEntitiesInSpot(flag_list[last_flag_num].spawnpoints[spawnpoint_num]))
						{
							//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, last_flag_num, spawnpoint_num);
							notgood = qfalse;
							VectorCopy(flag_list[last_flag_num].spawnpoints[spawnpoint_num], newspawn);
						}
					}
				}

				if (notgood)
				{// Fallback! Pick any flag owned by our team if possible...
					int tempflag = 0;
					spawnpoint_num = -1;

					while (notgood && tempflag <= num_flags)
					{
						if (flag_list[tempflag].flagentity->s.modelindex == ent->client->sess.sessionTeam)
						{
							while (notgood && spawnpoint_num < flag_list[tempflag].num_spawnpoints)
							{
								spawnpoint_num++;	

								if (!AdvancedWouldTelefrag(flag_list[tempflag].spawnpoints[spawnpoint_num])
									&& !CheckEntitiesInSpot(flag_list[tempflag].spawnpoints[spawnpoint_num]))
								{
									//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, tempflag, spawnpoint_num);
									notgood = qfalse;
									VectorCopy(flag_list[tempflag].spawnpoints[spawnpoint_num], newspawn);
									break;
								}
							}
						}

						tempflag++;
					}
				}

				if (!notgood)
				{// We have a spot...
					return qtrue;
				}
				else
				{
					return qfalse;
				}
			}*/

//			if (g_gametype.integer == GT_WARZONE || g_gametype.integer == GT_WARZONE_CAMPAIGN)
//			{
//				RandomSelectWarzoneSpawnpoint( bot );
//			}

			return qtrue;
		}
	}

	return qfalse;
}

/*
===========
RandomSelectWarzoneSpawnpoint

Randomly auto-selects a spawnpoint for the Warzone gametype for ent.
============
*/
#ifdef __OLD__
void RandomSelectWarzoneSpawnpoint ( gentity_t *ent )
{
	if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
	{// Pick a spawnpoint for the ent!
		int			num_flags = GetNumberOfWarzoneFlags();
		int			test_flag = 0;
		qboolean	redfound = qfalse;
		qboolean	bluefound = qfalse;
		int			flag_choice = 0;
		float		best_dist = 64000.0f;
		int			first_choice = -1;

		for (test_flag = 0; test_flag <= num_flags; test_flag++)
		{// Check we have a playable map...
			if (flag_list[test_flag].flagentity)
			{
				if (flag_list[test_flag].flagentity->s.modelindex == TEAM_RED)
					redfound = qtrue;
				if (flag_list[test_flag].flagentity->s.modelindex == TEAM_BLUE)
					bluefound = qtrue;
			}
		}

		if (!redfound)
		{
			flag_list[num_flags].flagentity->s.modelindex = TEAM_RED;
		}

		if (!bluefound)
		{
			flag_list[0].flagentity->s.modelindex = TEAM_BLUE;
		}

		for (test_flag = 0; test_flag <= num_flags; test_flag++)
		{// We need to find the most forward flag point to spawn at...
			if (flag_list[test_flag].flagentity)
			{
				if (flag_list[test_flag].num_spawnpoints > 0
					&& (flag_list[test_flag].flagentity->s.modelindex == ent->client->sess.sessionTeam || ent->client->sess.sessionTeam == TEAM_SPECTATOR))
				{// This is our flag...
					int test = 0;

					for (test = 0; test < num_flags; test++)
					{// Find enemy flags...
						if (flag_list[test].flagentity)
						{// FIXME: Selectable spawnpoints in UI...
							if (flag_list[test].flagentity->s.modelindex != ent->client->sess.sessionTeam
								&& flag_list[test].flagentity->s.modelindex != 0)
							{// This is our enemy's flag...
								if (VectorDistance(flag_list[test_flag].flagentity->s.origin, flag_list[test].flagentity->s.origin) < best_dist)
								{
									flag_choice = test_flag;
									first_choice = test_flag;
									best_dist = VectorDistance(flag_list[test_flag].flagentity->s.origin, flag_list[test].flagentity->s.origin);
								}
							}
						}
					}
				}
			}
		}

		if (Q_irand(0,2) >= 1)
		{// Sometimes pick a different one!
			for (test_flag = 0; test_flag <= num_flags; test_flag++)
			{// We need to find the most forward flag point to spawn at... FIXME: Selectable spawnpoints in UI...
				if (flag_list[test_flag].flagentity && test_flag != first_choice)
				{
					if (flag_list[test_flag].num_spawnpoints > 0
						&& flag_list[test_flag].flagentity->s.modelindex == ent->client->sess.sessionTeam)
					{// This is our flag...
						flag_choice = test_flag;	

						if (Q_irand(0,1) == 0)
							break;
					}
				}
			}
		}

//		SetPlayerSpawn( ent, flag_choice+1, qtrue ); // +1 because 0 is auto-select!
	}
}
#endif //__OLD__

/*
===========
SelectWarzoneSpectatorSpawnpoint

Selects a Spectator spawnpoint for the Warzone gametype for ent.
============
*/
/*
===========
SelectWarzoneSpawnpoint

Selects a spawnpoint for the Warzone gametype for ent.
============
*/

gentity_t *SelectWarzoneSpectatorSpawnpoint ( gentity_t *ent )
{
	gentity_t *tent = NULL;

	ent->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = 0;

	if ( !level.intermissiontime )
	{// Spawn around flag...
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		{
			int spawnpoint_num = 0;
			int num_flags = 0;
			int test_flag = 0;
			int flagnum = -1;
			int last_flag_num = -1;
			qboolean notgood = qtrue;
//			vec3_t newspawn;

			/*
			// UQ1: Try to use NPC spots... Cleaner code for JKG...
			NPC_SelectWarzoneSpawnpoint ( ent->client->sess.sessionTeam );

			if (!(NPC_SPAWNPOINT[0] == 0 && NPC_SPAWNPOINT[1] == 0 && NPC_SPAWNPOINT[2] == 0))
			{// Looks good... Use it!
				// Make a temp spawnpoint entity...
				tent = G_TempEntity (NPC_SPAWNPOINT, EV_NONE);
				VectorCopy(NPC_SPAWNPOINT, tent->s.origin);
				VectorCopy(NPC_SPAWNPOINT, tent->r.currentOrigin);
				VectorCopy(NPC_SPAWNPOINT, tent->s.pos.trBase);
				G_SetOrigin(tent, NPC_SPAWNPOINT);
				VectorCopy(ent->s.angles, tent->s.angles);
				VectorCopy(ent->s.angles, tent->r.currentAngles);

				ent->enemy = ent;

				tent->s.time = 1;
				tent->s.time2 = 1;
				//	tent->s.density = 0;

				trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5.\n", ent->client->pers.netname, NPC_SPAWNFLAG);

				return tent;
			}
			*/

			num_flags = GetNumberOfWarzoneFlags();

			flagnum = irand(0, num_flags-1);

			//for (flagnum = 0; flagnum < num_flags; flagnum++)
			{// Find a close waypoint...
				int		i;
				float	bestdist;
				float	flLen;
				vec3_t	a;
				int		GOOD_LIST[MAX_WPARRAY_SIZE];
				int		NUM_GOOD = 0;

				//if (flag_list[flagnum].flagentity->s.teamowner != ent->client->sess.sessionTeam)
				//	continue;

				i = 0;
				if (RMG.integer)
				{
					bestdist = 300;
				}
				else
				{
					//We're not doing traces!
					bestdist = 256.0f;

				}

				while (i < gWPNum)
				{
					if (gWPArray[i] && gWPArray[i]->inuse)
					{
						VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
						flLen = VectorLength(a);

						if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
						{
							GOOD_LIST[NUM_GOOD] = i;
							NUM_GOOD++;
						}
					}

					i++;
				}

				if (NUM_GOOD <= 0)
				{// Try further...
					bestdist = 512.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}

				if (NUM_GOOD <= 0)
				{// Try further... last chance...
					bestdist = 768.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}
				
				if (NUM_GOOD > 0)
				{// Ok we found some...
					int SPAWN_WP = GOOD_LIST[irand(0, NUM_GOOD-1)];

					// Make a temp spawnpoint entity...
					tent = G_TempEntity (gWPArray[SPAWN_WP]->origin, EV_NONE);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->s.origin);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->r.currentOrigin);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->s.pos.trBase);
					G_SetOrigin(tent, gWPArray[SPAWN_WP]->origin);
					VectorCopy(ent->s.angles, tent->s.angles);
					VectorCopy(ent->s.angles, tent->r.currentAngles);

					ent->enemy = ent;

					tent->s.time = 1;
					tent->s.time2 = 1;
					//	tent->s.density = 0;

					trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 (SPECTATOR) at flag ^7%i^5.\n", ent->client->pers.netname, flagnum);

					return tent;
				}
			}
		}
	}

	// FAILED! - Return normal spawnpoint...
	{
		gentity_t *spawnPoint = NULL;

		vec3_t spawn_origin, spawn_angles;
		VectorSet(spawn_origin, 0, 0, 0);
		VectorSet(spawn_angles, 0, 0, 0);
		spawnPoint = SelectCTFSpawnPoint(ent->client->sess.sessionTeam, ent->client->pers.teamState.state, spawn_origin, spawn_angles, (ent->r.svFlags & SVF_BOT) != 0);

		if (spawnPoint) return spawnPoint;

		spawnPoint = SelectRandomDeathmatchSpawnPoint();
		return spawnPoint;
	}
}


gentity_t *SelectWarzoneSpectatorSpawnpoint2 ( gentity_t *ent )
{
	gentity_t *tent = NULL;

	int spawnpoint_num = 0;
	int num_flags = 0;
	int test_flag = 0;
	int flagnum = -1;
	qboolean notgood = qtrue;
	vec3_t newspawn, good_angles;//, temp_angles;

	num_flags = GetNumberOfWarzoneFlags();

	if (num_flags >= 2)
	{// We have flags on the map... Find one that belongs to us..
		vec3_t	upOrg, good_pos, temp_pos, good_dir;
		trace_t tr;
		float	best_dist = 0.0f;
//		vec3_t	look_target;
		qboolean look_target_found = qfalse;
		qboolean bad = qtrue;
		int		tries = 0;
//		vec3_t  forward;

//		int		choices[256];
		int		upto = 0;

		/*
		for (flagnum = 0; flagnum <= num_flags; flagnum++)
		{
			if (flag_list[flagnum].flagentity->s.modelindex == TEAM_RED || flag_list[flagnum].flagentity->s.modelindex == TEAM_BLUE)
			{
				choices[upto] = flagnum;
				upto++;
			}
		}
		upto--;

		if (upto > 0) // Select a team's flag if we can!
			flagnum = choices[Q_irand(0, upto)];
		else // Any flag will do... Should be impossible,,,
			flagnum = Q_irand(0, num_flags);
		*/

		gentity_t *spawnPoint = SelectWarzoneSpectatorSpawnpoint2( ent );
		VectorCopy(spawnPoint->s.origin, newspawn);

		//trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 (^4SPECTATOR^5) at flag ^7%i^5.\n", ent->client->pers.netname, flagnum);
		
		/*
		for (spawnpoint_num = 0; spawnpoint_num < flag_list[flagnum].num_spawnpoints; spawnpoint_num++)
		{
			float distance = VectorDistance(flag_list[flagnum].flagentity->s.origin, flag_list[flagnum].spawnpoints[spawnpoint_num]);
			
			if (distance >= best_dist)
			{
				VectorCopy(flag_list[flagnum].spawnpoints[spawnpoint_num], newspawn);
				best_dist = distance;
			}
		}
		*/

		VectorCopy(newspawn, upOrg);
		//upOrg[2]+=65000;
		upOrg[2]+=512;

		trap->Trace( &tr, newspawn, NULL, NULL, upOrg, flag_list[flagnum].flagentity->s.number, MASK_SHOT|MASK_SOLID, 0, 0, 0);
		VectorCopy(tr.endpos, temp_pos);
		VectorCopy(newspawn, good_pos);
		good_pos[2] += ((temp_pos[2]-good_pos[2])*0.25); // 3/4 of the way to sky...

		VectorSubtract(flag_list[flagnum].flagentity->s.origin, good_pos, good_dir);
		vectoangles (good_dir, good_angles);
		good_angles[0]+=Q_irand(-25,25);
		good_angles[1]+=Q_irand(-25,25);
		//good_angles[2]+=Q_irand(-25,25);

		VectorCopy(good_pos, ent->client->ps.origin);
		VectorCopy(good_pos, ent->s.origin);
		VectorCopy(good_pos, ent->r.currentOrigin);
		VectorCopy(good_angles, ent->s.angles);
		VectorCopy(good_angles, ent->r.currentAngles);
		VectorCopy(good_angles, ent->client->ps.viewangles);
	}

	// Make a temp spawnpoint entity...
	tent = G_TempEntity (ent->client->ps.origin, EV_NONE);
	VectorCopy (ent->client->ps.origin, tent->s.origin);

	VectorCopy(good_angles, tent->s.angles);
	VectorCopy(good_angles, tent->r.currentAngles);
	//VectorCopy(good_angles, tent->client->ps.viewangles);

	ent->enemy = ent;

	tent->s.time = 1;
	tent->s.time2 = 1;
//	tent->s.density = 0;

	return tent;
}

vec3_t	NPC_SPAWNPOINT;
int		NPC_SPAWNFLAG;

/*
===========
SelectWarzoneSpawnpoint

Selects a spawnpoint for the Warzone gametype for ent.
============
*/

gentity_t *SelectWarzoneSpawnpoint ( gentity_t *ent )
{
	gentity_t *tent = NULL;

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
		return SelectWarzoneSpectatorSpawnpoint( ent );

	ent->client->ps.stats[STAT_CAPTURE_ENTITYNUM] = 0;

#ifdef __OLD__
	if ( !level.intermissiontime )
	{// Spawn around flag...
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		{
			int spawnpoint_num = 0;
			int num_flags = 0;
			int test_flag = 0;
			int flagnum = -1;
			int last_flag_num = -1;
			qboolean notgood = qtrue;
			vec3_t newspawn;

			// UQ1: Try to use NPC spots... Cleaner code for JKG...
			NPC_SelectWarzoneSpawnpoint ( ent->client->sess.sessionTeam );

			if (!(NPC_SPAWNPOINT[0] == 0 && NPC_SPAWNPOINT[1] == 0 && NPC_SPAWNPOINT[2] == 0))
			{// Looks good... Use it!
				// Make a temp spawnpoint entity...
				tent = G_TempEntity (NPC_SPAWNPOINT, EV_NONE);
				VectorCopy(NPC_SPAWNPOINT, tent->s.origin);
				VectorCopy(NPC_SPAWNPOINT, tent->r.currentOrigin);
				VectorCopy(NPC_SPAWNPOINT, tent->s.pos.trBase);
				G_SetOrigin(tent, NPC_SPAWNPOINT);
				VectorCopy(ent->s.angles, tent->s.angles);
				VectorCopy(ent->s.angles, tent->r.currentAngles);

				ent->enemy = ent;

				tent->s.time = 1;
				tent->s.time2 = 1;
				//	tent->s.density = 0;

				trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5.\n", ent->client->pers.netname, NPC_SPAWNFLAG);

				return tent;
			}

			num_flags = GetNumberOfWarzoneFlags();

			if (num_flags >= 2)
			{// We have flags on the map... Find one that belongs to us..
				if (ent->r.svFlags & SVF_BOT)
				{
					if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
					{
						RandomSelectWarzoneSpawnpoint( ent );
					}
				}
				else
				{
					RandomSelectWarzoneSpawnpoint( ent );
				}

				spawnpoint_num = -1;

				if (notgood)
				{// Use spawnpoint at the flag for fallback...
					spawnpoint_num = -1;

					while (notgood && spawnpoint_num < flag_list[flagnum].num_spawnpoints)
					{
						spawnpoint_num++;	

						if (!AdvancedWouldTelefrag(flag_list[flagnum].spawnpoints[spawnpoint_num])
							&& !CheckEntitiesInSpot(flag_list[flagnum].spawnpoints[spawnpoint_num]))
						{
							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, flagnum, spawnpoint_num);
							notgood = qfalse;
							VectorCopy(flag_list[flagnum].spawnpoints[spawnpoint_num], newspawn);
						}
					}
				}

				if (notgood)
				{// Try previous (further) flag if desperate...
					spawnpoint_num = -1;

					while (notgood && spawnpoint_num < flag_list[last_flag_num].num_spawnpoints)
					{
						spawnpoint_num++;	

						if (!AdvancedWouldTelefrag(flag_list[last_flag_num].spawnpoints[spawnpoint_num])
							&& !CheckEntitiesInSpot(flag_list[last_flag_num].spawnpoints[spawnpoint_num]))
						{
							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, last_flag_num, spawnpoint_num);
							notgood = qfalse;
							VectorCopy(flag_list[last_flag_num].spawnpoints[spawnpoint_num], newspawn);
						}
					}
				}

				if (notgood)
				{// Fallback! Pick any flag owned by our team if possible...
					int tempflag = 0;
					spawnpoint_num = -1;

					while (notgood && tempflag <= num_flags)
					{
						if (flag_list[tempflag].flagentity->s.modelindex == ent->client->sess.sessionTeam)
						{
							while (notgood && spawnpoint_num < flag_list[tempflag].num_spawnpoints)
							{
								spawnpoint_num++;	

								if (!AdvancedWouldTelefrag(flag_list[tempflag].spawnpoints[spawnpoint_num])
									&& !CheckEntitiesInSpot(flag_list[tempflag].spawnpoints[spawnpoint_num]))
								{
									trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 point ^7%i^5.\n", ent->client->pers.netname, tempflag, spawnpoint_num);
									notgood = qfalse;
									VectorCopy(flag_list[tempflag].spawnpoints[spawnpoint_num], newspawn);
									break;
								}
							}
						}

						tempflag++;
					}
				}

				if (!notgood)
				{// We have a spot...
					VectorCopy(newspawn, ent->client->ps.origin);
					VectorCopy(newspawn, ent->s.origin);
					VectorCopy(newspawn, ent->r.currentOrigin);
				}
				else
				{
					if (trap->Nav_GetNumNodes() > 0)
					{// Try using a close nav waypoint to any owned flag!
						int		num_nodes = trap->Nav_GetNumNodes();
						int		i, j;
						int		best_flag = -1;
						int		best_flag2 = -1;
						float	best_distance = 99999.9f;
						float	best_distance2 = 99999.9f;
						vec3_t	best_spot;
						vec3_t	best_spot2;

						VectorClear(best_spot);
						VectorClear(best_spot2);

						for (j = 0; j < num_flags; j++)
						{
							if (flag_list[j].flagentity->s.modelindex == ent->client->sess.sessionTeam)
							{
								vec3_t flag_org;
								VectorCopy(flag_list[j].flagentity->s.origin, flag_org);

								for (i = 0; i < num_nodes; i++)
								{
									trace_t tr;
									vec3_t	wp_org, test_org;
									float	distance;
									vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
									vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

									trap->Nav_GetNodePosition( i, wp_org );
									wp_org[2]+=8;

									// First test for solids/hazards!
									VectorCopy(wp_org, test_org);
									test_org[2]+=8;
									trap->Trace(&tr, wp_org, playerMins, playerMaxs, test_org, -1, MASK_SOLID);

									if (tr.allsolid || tr.startsolid)
										continue; // Don't make spawnpoints in solids!
					
									if (tr.contents & CONTENTS_SOLID || tr.contents & CONTENTS_SLIME || tr.contents & CONTENTS_LAVA || tr.contents & CONTENTS_TERRAIN)
										continue; // Don't make spawnpoints in ugly spots!

									distance = VectorDistance(wp_org, flag_org);

									if (distance <= 512)
									{// This is close enough, check if it is the closest so far...
										if (distance < best_distance && !AdvancedWouldTelefrag(wp_org))
										{// best so far... mark it!
											VectorCopy(wp_org, best_spot);
											best_distance = distance;
											best_flag = j;
										}
										else if (distance < best_distance)
										{// best spot that *may* telefrag...
											VectorCopy(wp_org, best_spot2);
											best_distance2 = distance;
											best_flag2 = j;
										}
									}
								}
							}
						}

						if (best_distance < 99999.9f)
						{// Found a good one!
							VectorCopy(best_spot, ent->client->ps.origin);
							VectorCopy(best_spot, ent->s.origin);
							VectorCopy(best_spot, ent->r.currentOrigin);

							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 (close nav waypoint).\n", ent->client->pers.netname, best_flag);
						}
						else if (best_distance2 < 99999.9f)
						{// Try using the backup one (*may* telefrag!
							VectorCopy(best_spot2, ent->client->ps.origin);
							VectorCopy(best_spot2, ent->s.origin);
							VectorCopy(best_spot2, ent->r.currentOrigin);

							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 (close nav waypoint).\n", ent->client->pers.netname, best_flag2);
						}
						else
						{// Argh! This sucks!
							vec3_t spawn_origin, spawn_angles;
							VectorSet(spawn_origin, 0, 0, 0);
							VectorSet(spawn_angles, 0, 0, 0);
							return SelectSpawnPoint(ent->r.currentOrigin, spawn_origin, spawn_angles, ent->client->sess.sessionTeam);
						}
					}
					else if (gWPNum > 0)
					{// Try using a close waypoint to any owned flag!
						int		i, j;
						int		best_flag = -1;
						int		best_flag2 = -1;
						float	best_distance = 99999.9f;
						float	best_distance2 = 99999.9f;
						vec3_t	best_spot;
						vec3_t	best_spot2;

						VectorClear(best_spot);
						VectorClear(best_spot2);

						for (j = 0; j < num_flags; j++)
						{
							if (flag_list[j].flagentity->s.modelindex == ent->client->sess.sessionTeam)
							{
								vec3_t flag_org;
								VectorCopy(flag_list[j].flagentity->s.origin, flag_org);

								for (i = 0; i < gWPNum; i++)
								{
									if (gWPArray[i])
									{
										trace_t tr;
										vec3_t wp_org, test_org;
										float distance;
										vec3_t	playerMins = {-15, -15, DEFAULT_MINS_2};
										vec3_t	playerMaxs = {15, 15, DEFAULT_MAXS_2};

										VectorCopy(gWPArray[i]->origin, wp_org);
										wp_org[2]+=8;

										// First test for solids/hazards!
										VectorCopy(wp_org, test_org);
										test_org[2]+=8;
										trap->Trace(&tr, wp_org, playerMins, playerMaxs, test_org, -1, MASK_SOLID);

										if (tr.allsolid || tr.startsolid)
											continue; // Don't make spawnpoints in solids!
					
										if (tr.contents & CONTENTS_SOLID || tr.contents & CONTENTS_SLIME || tr.contents & CONTENTS_LAVA || tr.contents & CONTENTS_TERRAIN)
											continue; // Don't make spawnpoints in ugly spots!

										distance = VectorDistance(wp_org, flag_org);

										if (distance <= 512)
										{// This is close enough, check if it is the closest so far...
											if (distance < best_distance && !AdvancedWouldTelefrag(wp_org))
											{// best so far... mark it!
												VectorCopy(wp_org, best_spot);
												best_distance = distance;
												best_flag = j;
											}
											else if (distance < best_distance)
											{// best spot that *may* telefrag...
												VectorCopy(wp_org, best_spot2);
												best_distance2 = distance;
												best_flag2 = j;
											}
										}
									}
								}
							}
						}

						if (best_distance < 99999.9f)
						{// Found a good one!
							VectorCopy(best_spot, ent->client->ps.origin);
							VectorCopy(best_spot, ent->s.origin);
							VectorCopy(best_spot, ent->r.currentOrigin);

							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 (close waypoint).\n", ent->client->pers.netname, best_flag);
						}
						else if (best_distance2 < 99999.9f)
						{// Try using the backup one (*may* telefrag!
							VectorCopy(best_spot2, ent->client->ps.origin);
							VectorCopy(best_spot2, ent->s.origin);
							VectorCopy(best_spot2, ent->r.currentOrigin);

							trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5 (close waypoint).\n", ent->client->pers.netname, best_flag2);
						}
						else
						{// Argh! This sucks!
							vec3_t spawn_origin, spawn_angles;
							VectorSet(spawn_origin, 0, 0, 0);
							VectorSet(spawn_angles, 0, 0, 0);
							return SelectSpawnPoint(ent->r.currentOrigin, spawn_origin, spawn_angles, ent->client->sess.sessionTeam);
						}
					}
					else if (ent->s.number == 0)
					{// Return CTF spawn for a local client on a Warzone map without flags...
						vec3_t spawn_origin, spawn_angles;
						VectorSet(spawn_origin, 0, 0, 0);
						VectorSet(spawn_angles, 0, 0, 0);
						return SelectCTFSpawnPoint(ent->client->sess.sessionTeam, ent->client->pers.teamState.state, spawn_origin, spawn_angles);
					}
					else
						return NULL;
				}
			}
		}
	}

	if (ent->s.number == 0 && GetNumberOfWarzoneFlags() < 2) // Local client needs access for adding routes and flags...
	{// Return CTF spawn for a local client on a Warzone map without flags...
		vec3_t spawn_origin, spawn_angles;
		VectorSet(spawn_origin, 0, 0, 0);
		VectorSet(spawn_angles, 0, 0, 0);
		return SelectCTFSpawnPoint(ent->client->sess.sessionTeam, ent->client->pers.teamState.state, spawn_origin, spawn_angles);
	}

/*	if (g_spawnInvulnerability.integer > 0)
		ent->client->invulnerabilityTime = level.time + g_spawnInvulnerability.integer;
	else*/
//		ent->client->invulnerabilityTime = 0;

	// Make a temp spawnpoint entity...
	tent = G_TempEntity (ent->client->ps.origin, EV_NONE);
	VectorCopy (ent->client->ps.origin, tent->s.origin);
	tent->s.origin[2]+=16; // Raise the point a little for safety...

	tent->s.angles[0] = 0;
	tent->s.angles[1] = 0;
	tent->s.angles[2] = 0;
	VectorCopy(tent->s.angles, tent->r.currentAngles);

	//VectorCopy (ent->r.currentAngles, tent->s.angles);

	VectorSet(ent->client->ps.viewangles, 0, 0, 0);
	ent->enemy = ent;

	tent->s.time = 1;//500;
	tent->s.time2 = 1;//100;
//	tent->s.density = 0;

	return tent;
#else //!__OLD__

	if ( !level.intermissiontime )
	{// Spawn around flag...
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		{
			int spawnpoint_num = 0;
			int num_flags = 0;
			int test_flag = 0;
			int flagnum = -1;
			int last_flag_num = -1;
			qboolean notgood = qtrue;
//			vec3_t newspawn;

			/*
			// UQ1: Try to use NPC spots... Cleaner code for JKG...
			NPC_SelectWarzoneSpawnpoint ( ent->client->sess.sessionTeam );

			if (!(NPC_SPAWNPOINT[0] == 0 && NPC_SPAWNPOINT[1] == 0 && NPC_SPAWNPOINT[2] == 0))
			{// Looks good... Use it!
				// Make a temp spawnpoint entity...
				tent = G_TempEntity (NPC_SPAWNPOINT, EV_NONE);
				VectorCopy(NPC_SPAWNPOINT, tent->s.origin);
				VectorCopy(NPC_SPAWNPOINT, tent->r.currentOrigin);
				VectorCopy(NPC_SPAWNPOINT, tent->s.pos.trBase);
				G_SetOrigin(tent, NPC_SPAWNPOINT);
				VectorCopy(ent->s.angles, tent->s.angles);
				VectorCopy(ent->s.angles, tent->r.currentAngles);

				ent->enemy = ent;

				tent->s.time = 1;
				tent->s.time2 = 1;
				//	tent->s.density = 0;

				trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5.\n", ent->client->pers.netname, NPC_SPAWNFLAG);

				return tent;
			}
			*/

			int team_flags_total = 0;
			int team_flags[256];
			num_flags = GetNumberOfWarzoneFlags();

			// Count this team's flags and record them...
			for (flagnum = 0; flagnum < num_flags; flagnum++)
			{
				if (flag_list[flagnum].flagentity->s.teamowner != ent->client->sess.sessionTeam)
					continue;

				team_flags[team_flags_total] = flagnum;
				team_flags_total++;
			}

			// Pick a flag at random from recorded team flags...
			flagnum = team_flags[irand(0, team_flags_total-1)];

			//for (flagnum = 0; flagnum < num_flags; flagnum++)
			{// Find a close waypoint...
				int		i;
				float	bestdist;
				float	flLen;
				vec3_t	a;
				int		GOOD_LIST[MAX_WPARRAY_SIZE];
				int		NUM_GOOD = 0;

				//if (flag_list[flagnum].flagentity->s.teamowner != ent->client->sess.sessionTeam)
				//	continue;

				i = 0;
				if (RMG.integer)
				{
					bestdist = 300;
				}
				else
				{
					//We're not doing traces!
					bestdist = 256.0f;

				}

				while (i < gWPNum)
				{
					if (gWPArray[i] && gWPArray[i]->inuse)
					{
						VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
						flLen = VectorLength(a);

						if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
						{
							GOOD_LIST[NUM_GOOD] = i;
							NUM_GOOD++;
						}
					}

					i++;
				}

				if (NUM_GOOD <= 0)
				{// Try further...
					bestdist = 512.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}

				if (NUM_GOOD <= 0)
				{// Try further... last chance...
					bestdist = 768.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}
				
				if (NUM_GOOD > 0)
				{// Ok we found some...
					int SPAWN_WP = GOOD_LIST[irand(0, NUM_GOOD-1)];

					// Make a temp spawnpoint entity...
					tent = G_TempEntity (gWPArray[SPAWN_WP]->origin, EV_NONE);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->s.origin);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->r.currentOrigin);
					VectorCopy(gWPArray[SPAWN_WP]->origin, tent->s.pos.trBase);
					G_SetOrigin(tent, gWPArray[SPAWN_WP]->origin);
					VectorCopy(ent->s.angles, tent->s.angles);
					VectorCopy(ent->s.angles, tent->r.currentAngles);

					ent->enemy = ent;

					tent->s.time = 1;
					tent->s.time2 = 1;
					//	tent->s.density = 0;

					trap->Print("^3*** ^3WarZone^5: Spawning ^3%s^5 at flag ^7%i^5.\n", ent->client->pers.netname, flagnum);

					return tent;
				}
			}
		}
	}

	// FAILED! - Return normal spawnpoint...
	{
		gentity_t *spawnPoint = NULL;

		vec3_t spawn_origin, spawn_angles;
		VectorSet(spawn_origin, 0, 0, 0);
		VectorSet(spawn_angles, 0, 0, 0);
		spawnPoint = SelectCTFSpawnPoint(ent->client->sess.sessionTeam, ent->client->pers.teamState.state, spawn_origin, spawn_angles, (ent->r.svFlags & SVF_BOT) != 0);

		if (spawnPoint) return spawnPoint;

		spawnPoint = SelectRandomDeathmatchSpawnPoint();
		return spawnPoint;
	}
#endif //__OLD__
}


/*
===========
NPC_SelectWarzoneSpawnpoint

Selects a spawnpoint for the Warzone gametype for TEAM.
============
*/

void NPC_SelectWarzoneSpawnpoint ( int TEAM )
{
	// Init the spawnpoint info...
	VectorSet(NPC_SPAWNPOINT, 0, 0, 0);

#ifdef __OLD__
	if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
	{// Pick a spawnpoint for the ent!
		int			num_flags = GetNumberOfWarzoneFlags();
		int			test_flag = 0;
		//qboolean	redfound = qfalse;
		//qboolean	bluefound = qfalse;
		int			flag_choice = -1;
		int			spawn_choice = -1;

		/*
		for (test_flag = 0; test_flag <= num_flags; test_flag++)
		{// Check we have a playable map...
			if (flag_list[test_flag].flagentity)
			{
				if (flag_list[test_flag].flagentity->s.modelindex == TEAM_RED)
					redfound = qtrue;
				if (flag_list[test_flag].flagentity->s.modelindex == TEAM_BLUE)
					bluefound = qtrue;
			}
		}

		if (!redfound)
		{
			flag_list[num_flags].flagentity->s.modelindex = TEAM_RED;
		}

		if (!bluefound)
		{
			flag_list[0].flagentity->s.modelindex = TEAM_BLUE;
		}
		*/

		for (test_flag = 0; test_flag <= num_flags; test_flag++)
		{// We need to find the most forward flag point to spawn at... FIXME: Selectable spawnpoints in UI...
			if (flag_list[test_flag].flagentity)
			{
				if (flag_list[test_flag].num_spawnpoints > 0
					&& flag_list[test_flag].flagentity->s.teamowner == TEAM)
				{// This is our flag...
					int test_spawn = 0;

					// Find a spawnpoint that won't telefrag anyone...
					for (test_spawn = 0; test_spawn < flag_list[test_flag].num_spawnpoints; test_spawn++)
					{
						if (!AdvancedWouldTelefrag(flag_list[test_flag].spawnpoints[test_spawn]) 
							&& CheckBelowOK(flag_list[test_flag].spawnpoints[test_spawn]))
						{// Found one here!
							flag_choice = test_flag;
							spawn_choice = test_spawn;
							break;
						}
					}
				}
			}

			if (spawn_choice >= 0 && irand(0,2) > 1)
			{// We have a spot already, but we want to vary them, so check other flags too 2/3 of the time... (spead NPCs out)
				break;
			}
		}

		if (flag_choice >= 0 && spawn_choice >= 0 
			&& !(flag_list[flag_choice].spawnpoints[spawn_choice][0] == 0 && flag_list[flag_choice].spawnpoints[spawn_choice][1] == 0 && flag_list[flag_choice].spawnpoints[spawn_choice][2] == 0))
		{// Got one!
			NPC_SPAWNFLAG = flag_choice;
			VectorCopy(flag_list[flag_choice].spawnpoints[spawn_choice], NPC_SPAWNPOINT);
			return;
		}
	}
#else //!__OLD__
	//if ( !level.intermissiontime )
	{// Spawn around flag...
		if (g_gametype.integer == GT_WARZONE /*|| g_gametype.integer == GT_WARZONE_CAMPAIGN*/)
		{
			int spawnpoint_num = 0;
			int num_flags = 0;
			int test_flag = 0;
			int flagnum = -1;
			int last_flag_num = -1;
			qboolean notgood = qtrue;
//			vec3_t newspawn;

			int team_flags_total = 0;
			int team_flags[256];
			num_flags = GetNumberOfWarzoneFlags();

			// Count this team's flags and record them...
			for (flagnum = 0; flagnum < num_flags; flagnum++)
			{
				if (flag_list[flagnum].flagentity->s.teamowner != TEAM)
					continue;

				team_flags[team_flags_total] = flagnum;
				team_flags_total++;
			}

			// Pick a flag at random from recorded team flags...
			flagnum = team_flags[irand(0, team_flags_total-1)];

			//for (flagnum = 0; flagnum < num_flags; flagnum++)
			{// Find a close waypoint...
				int		i;
				float	bestdist;
				float	flLen;
				vec3_t	a;
				int		GOOD_LIST[MAX_WPARRAY_SIZE];
				int		NUM_GOOD = 0;

				//if (flag_list[flagnum].flagentity->s.teamowner != TEAM)
				//	continue;

				i = 0;
				if (RMG.integer)
				{
					bestdist = 300;
				}
				else
				{
					//We're not doing traces!
					bestdist = 256.0f;

				}

				while (i < gWPNum)
				{
					if (gWPArray[i] && gWPArray[i]->inuse)
					{
						VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
						flLen = VectorLength(a);

						if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
						{
							GOOD_LIST[NUM_GOOD] = i;
							NUM_GOOD++;
						}
					}

					i++;
				}

				if (NUM_GOOD <= 0)
				{// Try further...
					bestdist = 512.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}

				if (NUM_GOOD <= 0)
				{// Try further... last chance...
					bestdist = 768.0f;

					while (i < gWPNum)
					{
						if (gWPArray[i] && gWPArray[i]->inuse)
						{
							VectorSubtract(flag_list[flagnum].flagentity->s.origin, gWPArray[i]->origin, a);
							flLen = VectorLength(a);

							if (flLen < bestdist && !AdvancedWouldTelefrag(gWPArray[i]->origin) /*&& JKG_CheckBelowWaypoint(i)*/)
							{
								GOOD_LIST[NUM_GOOD] = i;
								NUM_GOOD++;
							}
						}

						i++;
					}
				}
				
				if (NUM_GOOD > 0)
				{// Ok we found some...
					int SPAWN_WP = GOOD_LIST[irand(0, NUM_GOOD-1)];

					NPC_SPAWNFLAG = flagnum;
					VectorCopy(gWPArray[SPAWN_WP]->origin, NPC_SPAWNPOINT);
					return;
				}
			}
		}
	}

	// FAILED! - Return normal spawnpoint...
	{
		gentity_t *spawnPoint = SelectRandomDeathmatchSpawnPoint();

		if (spawnPoint)
		{
			NPC_SPAWNFLAG = 9999;
			VectorCopy(spawnPoint->s.origin, NPC_SPAWNPOINT);
			return;
		}
	}
#endif //__OLD__

	// NOTHING FOUND!!!
	VectorSet(NPC_SPAWNPOINT, 0, 0, 0);
}