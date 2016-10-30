//includes
#include "g_local.h"
#include "ai_main.h"

//externs
extern bot_state_t	*botstates[MAX_CLIENTS];

extern vmCvar_t bot_fps;

#define		MAX_OBJECTIVES			6

//max allowed objective dependancy
#define		MAX_OBJECTIVEDEPENDANCY	6

int ObjectiveDependancy[MAX_OBJECTIVES][MAX_OBJECTIVEDEPENDANCY];

int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp); // below...
extern int BotSelectChoiceWeapon(bot_state_t *bs, int weapon, int doselection);
extern int CheckForFunc(vec3_t org, int ignore);
extern float forceJumpStrength[NUM_FORCE_POWER_LEVELS];
extern int BotMindTricked(int botClient, int enemyClient);
extern int BotCanHear(bot_state_t *bs, gentity_t *en, float endist);
extern int PassStandardEnemyChecks(bot_state_t *bs, gentity_t *en);
extern qboolean BotPVSCheck( const vec3_t p1, const vec3_t p2 );
extern int PassLovedOneCheck(bot_state_t *bs, gentity_t *ent);
extern void BotDeathNotify(bot_state_t *bs);
extern void BotReplyGreetings(bot_state_t *bs);
extern void MoveTowardIdealAngles(bot_state_t *bs);
extern int forcePowerNeeded[NUM_FORCE_POWER_LEVELS][NUM_FORCE_POWERS];
qboolean G_PointInBounds( vec3_t point, vec3_t mins, vec3_t maxs );
qboolean G_NameInTriggerClassList(char *list, char *str);

extern siegeClass_t *BG_GetClassOnBaseClass(const int team, const short classIndex, const short cntIndex);

extern void G_SoundOnEnt( gentity_t *ent, soundChannel_t channel, const char *soundPath );

//inits
int DOM_FavoriteWeapon(bot_state_t *bs, gentity_t *target);
void DOM_ResetWPTimers(bot_state_t *bs);
void DOM_AdjustforStrafe(bot_state_t *bs, vec3_t moveDir);
void DOM_BotObjective(bot_state_t *bs);
void DOM_FindAngles(gentity_t *ent, vec3_t angles);
void DOM_FindOrigin(gentity_t *ent, vec3_t origin);
gentity_t *GetObjectThatTargets(gentity_t *ent);
void DOM_BotBehave_Attack(bot_state_t *bs);
void DOM_BotBehave_AttackBasic(bot_state_t *bs, gentity_t* target);
float DOM_TargetDistance(bot_state_t *bs, gentity_t* target, vec3_t targetorigin);
void DOM_BotBehave_DefendBasic(bot_state_t *bs, vec3_t defpoint);
int DOM_GetNearestVisibleWP(vec3_t org, int ignore, int badwp);
qboolean DOM_AttackLocalBreakable(bot_state_t *bs, vec3_t origin);
int DOM_BotWeapon_Detpack(bot_state_t *bs, gentity_t *target);
qboolean DOM_DontBlockAllies(bot_state_t *bs);

//distance at which the bot will move out of the way of an ally.
#define BLOCKALLIESDISTANCE 50

//targets must be inside this distance for the bot to attempt to blow up det packs.
#define DETPACK_DETDISTANCE 500

//bots will stay within this distance to their defend targets while fighting enemies
#define	DEFEND_MAXDISTANCE	500

//if noone enemies are in the area, the bot will stay this close to their defend target
#define DEFEND_MINDISTANCE	200

/* */
float VectorDistanceNoHeight ( vec3_t v1, vec3_t v2 )
{
	vec3_t	dir;
	vec3_t	v1a, v2a;
	VectorCopy( v1, v1a );
	VectorCopy( v2, v2a );
	v2a[2] = v1a[2];
	VectorSubtract( v2a, v1a, dir );
	return ( VectorLength( dir) );
}


/* */
float
HeightDistance ( vec3_t v1, vec3_t v2 )
{
	vec3_t	dir;
	vec3_t	v1a, v2a;
	VectorCopy( v1, v1a );
	VectorCopy( v2, v2a );
	v2a[0] = v1a[0];
	v2a[1] = v1a[1];
	VectorSubtract( v2a, v1a, dir );
	return ( VectorLength( dir) );
}


int DOM_MinimumAttackDistance[WP_NUM_WEAPONS] = 
{
	0, //WP_NONE,
	0, //WP_STUN_BATON,
	30, //WP_MELEE,
	30, //WP_SABER,
	0, //WP_BRYAR_PISTOL,
	0, //WP_BLASTER,
	0, //WP_DISRUPTOR,
	0, //WP_BOWCASTER,
	0, //WP_REPEATER,
	0, //WP_DEMP2,
	0, //WP_FLECHETTE,
	100, //WP_ROCKET_LAUNCHER,
	100, //WP_THERMAL,
	100, //WP_TRIP_MINE,
	0, //WP_DET_PACK,
	0, //WP_CONCUSSION,
	0, //WP_BRYAR_OLD,
	0, //WP_EMPLACED_GUN,
	0 //WP_TURRET,
	//WP_NUM_WEAPONS
};


int DOM_MaximumAttackDistance[WP_NUM_WEAPONS] = 
{
	0, //WP_NONE,
	0, //WP_STUN_BATON,
	100, //WP_MELEE,
	70/*100*/, //WP_SABER,
	9999, //WP_BRYAR_PISTOL,
	9999, //WP_BLASTER,
	9999, //WP_DISRUPTOR,
	9999, //WP_BOWCASTER,
	9999, //WP_REPEATER,
	9999, //WP_DEMP2,
	9999, //WP_FLECHETTE,
	9999, //WP_ROCKET_LAUNCHER,
	9999, //WP_THERMAL,
	9999, //WP_TRIP_MINE,
	9999, //WP_DET_PACK,
	9999, //WP_CONCUSSION,
	9999, //WP_BRYAR_OLD,
	9999, //WP_EMPLACED_GUN,
	9999 //WP_TURRET,
	//WP_NUM_WEAPONS
};


int DOM_IdealAttackDistance[WP_NUM_WEAPONS] = 
{
	0, //WP_NONE,
	0, //WP_STUN_BATON,
	30, //WP_MELEE,
	48/*60*/, //WP_SABER,
	1000, //WP_BRYAR_PISTOL,
	1000, //WP_BLASTER,
	1000, //WP_DISRUPTOR,
	1000, //WP_BOWCASTER,
	1000, //WP_REPEATER,
	1000, //WP_DEMP2,
	1000, //WP_FLECHETTE,
	1000, //WP_ROCKET_LAUNCHER,
	1000, //WP_THERMAL,
	1000, //WP_TRIP_MINE,
	1000, //WP_DET_PACK,
	1000, //WP_CONCUSSION,
	1000, //WP_BRYAR_OLD,
	1000, //WP_EMPLACED_GUN,
	1000 //WP_TURRET,
	//WP_NUM_WEAPONS
};

//used by track attac to prevent the bot from thinking you've disappeared the second it loses
//visual/hearing on you (like if you hid behind it).  It will continue to know where you 
//are during this time.
#define BOT_VISUALLOSETRACKTIME	2000

//the amount of Force Power you'll need for a given Force Jump level
int DOM_ForcePowerforJump[NUM_FORCE_POWER_LEVELS] =
{	
	0,
	30,
	30,
	30
};

//Returns the Force level needed to make this jump
//FORCE_LEVEL_4 (4) = Jump too high!
int DOM_ForceJumpNeeded(vec3_t startvect, vec3_t endvect)
{
	float heightdif, lengthdif; 
	vec3_t tempstart, tempend;

	heightdif = endvect[2] - startvect[2];
	
	VectorCopy(startvect, tempstart);
	VectorCopy(endvect, tempend);

	tempend[2] = tempstart[2] = 0;
	lengthdif = Distance(tempstart, tempend);

	if (heightdif < 128 && lengthdif < 128)
	{ //no force needed
		return FORCE_LEVEL_0;
	}

	if (heightdif > 512)
	{ //too high
		return FORCE_LEVEL_4;
	}
	if (heightdif > 350 || lengthdif > 700)
	{
		return FORCE_LEVEL_3;
	}
	else if (heightdif > 256 || lengthdif > 512)
	{
		return FORCE_LEVEL_2;
	}
	else
	{
		return FORCE_LEVEL_1;
	}
}
	
// UQ1: Added for fast random nearby waypoints...
int DOM_GetRandomCloseWP(vec3_t org, int badwp, int unused)
{
	int		i;
	float	bestdist;
	float	flLen;
	vec3_t	a;
	int		GOOD_LIST[MAX_WPARRAY_SIZE];
	int		NUM_GOOD = 0;

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
		if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
		{
			VectorSubtract(org, gWPArray[i]->origin, a);
			flLen = VectorLength(a);

			if (flLen < bestdist)
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
			if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
			{
				VectorSubtract(org, gWPArray[i]->origin, a);
				flLen = VectorLength(a);

				if (flLen < bestdist)
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
			if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
			{
				VectorSubtract(org, gWPArray[i]->origin, a);
				flLen = VectorLength(a);

				if (flLen < bestdist)
				{
					GOOD_LIST[NUM_GOOD] = i;
					NUM_GOOD++;
				}
			}

			i++;
		}
	}

	if (NUM_GOOD <= 0)
		return -1;

	return GOOD_LIST[irand(0, NUM_GOOD-1)];
}


//just like GetNearestVisibleWP except without visiblity checks
int DOM_GetNearestWP(vec3_t org, int badwp)
{
	int i;
	float bestdist;
	float flLen;
	int bestindex;
	vec3_t a;

	i = 0;
	if (RMG.integer)
	{
		bestdist = 300;
	}
	else
	{
		//We're not doing traces!
		bestdist = 99999;

	}
	bestindex = -1;

	while (i < gWPNum)
	{
		if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
		{
			VectorSubtract(org, gWPArray[i]->origin, a);
			flLen = VectorLength(a);

			if(gWPArray[i]->flags & WPFLAG_WAITFORFUNC 
				|| (gWPArray[i]->flags & WPFLAG_NOMOVEFUNC)/*
				|| (gWPArray[i]->flags & WPFLAG_DESTROY_FUNCBREAK)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPUSH)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPULL)*/ )
			{//boost the distance for these waypoints so that we will try to avoid using them
				//if at all possible
				flLen = flLen + 500;
			}

			if (flLen < bestdist)
			{
				bestdist = flLen;
				bestindex = i;
			}
		}

		i++;
		//i+=Q_irand(1,3);
	}

	return bestindex;
}


/*
=========================
START A* Pathfinding Code
=========================
*/

//Maximum distance allowed between gWPArray for them to count as seqencial wp.
#define MAX_NODE_DIS 1000

//Clear out the Route
void DOM_ClearRoute( int Route[MAX_WPARRAY_SIZE] )
{
	int i;
	for( i=0; i < MAX_WPARRAY_SIZE; i++ )
	{
		Route[i] = -1;
	}
}

#define	WAYPOINT_NONE	-1

int DOM_CreateDumbRoute(int from, int to, int *pathlist)
{// Create a route not using links...
	int count = 0;
	int number = 0;
	int upto = 0;

	if (to < from)
	{
		if (from - to > MAX_WPARRAY_SIZE)
			return -1;

		for (upto = from; upto > to; upto--)
		{
			pathlist[number] = upto;
			number++;
		}

		count = number;
	}
	else
	{
		if (to - from > MAX_WPARRAY_SIZE)
			return -1;

		for (upto = to; upto > from; upto--)
		{
			pathlist[number] = upto;
			number++;
		}

		count = number;
	}

	return count-1;
}

int BOT_GetFCost ( gentity_t *bot, int to, int num, int parentNum, float *gcost )
{
	float	gc = 0;
	float	hc = 0;
	vec3_t	v;
	float	height_diff = 0;
	
	if ( gcost[num] == -1 )
	{
		if ( parentNum != -1 )
		{
			/*gc = gcost[parentNum];

			gc += Distance(gWPArray[num]->origin, gWPArray[parentNum]->origin);

			// UQ1: Make Height Diffs matter (prefer single plane travel over the bumpy road)!!!
			height_diff = HeightDistance(gWPArray[num]->origin, gWPArray[parentNum]->origin);
			gc += (height_diff * height_diff); // Squared for massive preferance to staying at same plane...

			if (gc > 65000)
				gc = 65000.0f;
				*/

			gc = gcost[parentNum];
			VectorSubtract( gWPArray[num]->origin, gWPArray[parentNum]->origin, v );
			gc += VectorLength( v );

			gc += HeightDistance(gWPArray[num]->origin, gWPArray[parentNum]->origin) * 4;

			if (gc > 64000)
				gc = 64000.0f;
		}

		gcost[num] = gc;
	}
	else
	{
		gc = gcost[num];
	}

	/*
	VectorSubtract( gWPArray[to]->origin, gWPArray[num]->origin, v );
	hc = VectorLength( v );
	*/

	hc = Distance(gWPArray[num]->origin, gWPArray[parentNum]->origin);
	height_diff = HeightDistance(gWPArray[num]->origin, gWPArray[parentNum]->origin);
	hc += (height_diff * height_diff); // Squared for massive preferance to staying at same plane...

	return (int) ( (gc*0.1) + (hc*0.1) );
}

#define NODE_INVALID -1
#define MAX_NODELINKS 32

int			*openlist;												//add 1 because it's a binary heap, and they don't use 0 - 1 is the first used index
float		*gcost;
int			*fcost;
char		*list;														//0 is neither, 1 is open, 2 is closed - char because it's the smallest data type
int			*parent;

qboolean PATHFINDING_MEMORY_ALLOCATED = qfalse;

void AllocatePathFindingMemory()
{
	if (PATHFINDING_MEMORY_ALLOCATED) return;

	openlist = (int *)malloc(sizeof(int)*(MAX_WPARRAY_SIZE));
	gcost = (float *)malloc(sizeof(float)*(MAX_WPARRAY_SIZE));
	fcost = (int *)malloc(sizeof(int)*(MAX_WPARRAY_SIZE));
	list = (char *)malloc(sizeof(char)*(MAX_WPARRAY_SIZE));
	parent = (int *)malloc(sizeof(int)*(MAX_WPARRAY_SIZE));

	PATHFINDING_MEMORY_ALLOCATED = qtrue;
}

//#define __DISABLED_OLD_ASTAR__

int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist)
{
#ifdef __DISABLED_OLD_ASTAR__
	//all the data we have to hold...since we can't do dynamic allocation, has to be MAX_WPARRAY_SIZE
	//we can probably lower this later - eg, the open list should never have more than at most a few dozen items on it
	int			badwp = -1;
	int			numOpen = 0;
	int			atNode, temp, newnode = -1;
	qboolean	found = qfalse;
	int			count = -1;
	float		gc;
	int			i, j, u, v, m;
	gentity_t	*bot = NULL;

	if ( (from == NODE_INVALID) || (to == NODE_INVALID) || (from >= gWPNum) || (to >= gWPNum) || (from == to) )
	{
		//trap->Print("Bad from or to node.\n");
		return ( -1 );
	}

	// Check if memory needs to be allocated...
	AllocatePathFindingMemory();
	
	if (bs)
	{
		bot = &g_entities[bs->client];
		badwp = bs->wpCurrent->index;
	}

	memset( openlist, 0, (sizeof(int) * (gWPNum + 1)) );
	memset( gcost, 0, (sizeof(float) * gWPNum) );
	memset( fcost, 0, (sizeof(int) * gWPNum) );
	memset( list, 0, (sizeof(char) * gWPNum) );
	memset( parent, 0, (sizeof(int) * gWPNum) );

	openlist[gWPNum+1] = 0;

	openlist[1] = from;																	//add the starting node to the open list
	numOpen++;
	gcost[from] = 0;																	//its f and g costs are obviously 0
	fcost[from] = 0;

	while ( 1 )
	{
		if ( numOpen != 0 )																//if there are still items in the open list
		{
			//pop the top item off of the list
			atNode = openlist[1];
			list[atNode] = 2;															//put the node on the closed list so we don't check it again
			numOpen--;
			openlist[1] = openlist[numOpen + 1];										//move the last item in the list to the top position
			v = 1;

			//this while loop reorders the list so that the new lowest fcost is at the top again
			while ( 1 )
			{
				u = v;
				if ( (2 * u + 1) < numOpen )											//if both children exist
				{
					if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
					{
						v = 2 * u;
					}

					if ( fcost[openlist[v]] >= fcost[openlist[2 * u + 1]] )
					{
						v = 2 * u + 1;
					}
				}
				else
				{
					if ( (2 * u) < numOpen )											//if only one child exists
					{
						if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
						{
							v = 2 * u;
						}
					}
				}

				if ( u != v )															//if they're out of order, swap this item with its parent
				{
					temp = openlist[u];
					openlist[u] = openlist[v];
					openlist[v] = temp;
				}
				else
				{
					break;
				}
			}

			for ( i = 0; i < gWPArray[atNode]->neighbornum && i < MAX_NODELINKS; i++ )								//loop through all the links for this node
			{
				newnode = gWPArray[atNode]->neighbors[i].num;

				if (newnode > gWPNum)
					continue;

				if (newnode < 0)
					continue;

				//if (nodes[newnode].objectNum[0] == 1)
				//	continue; // Skip water/ice disabled node!

				if ( list[newnode] == 2 )
				{																		//if this node is on the closed list, skip it
					continue;
				}

				if ( list[newnode] != 1 )												//if this node is not already on the open list
				{
					openlist[++numOpen] = newnode;										//add the new node to the open list
					list[newnode] = 1;
					parent[newnode] = atNode;											//record the node's parent
					
					if ( newnode == to )
					{																	//if we've found the goal, don't keep computing paths!
						break;															//this will break the 'for' and go all the way to 'if (list[to] == 1)'
					}

					fcost[newnode] = BOT_GetFCost( bot, to, newnode, parent[newnode], gcost );	//store it's f cost value

					//this loop re-orders the heap so that the lowest fcost is at the top
					m = numOpen;

					while ( m != 1 )													//while this item isn't at the top of the heap already
					{
						if ( fcost[openlist[m]] <= fcost[openlist[m / 2]] )				//if it has a lower fcost than its parent
						{
							temp = openlist[m / 2];
							openlist[m / 2] = openlist[m];
							openlist[m] = temp;											//swap them
							m /= 2;
						}
						else
						{
							break;
						}
					}
				}
				else										//if this node is already on the open list
				{
					gc = gcost[atNode];

					if (gWPArray[atNode]->neighbors[i].cost)
					{// UQ1: Already have a cost value, skip the calculations!
						gc += gWPArray[atNode]->neighbors[i].cost;
					}
					else
					{
						vec3_t	vec;

						VectorSubtract( gWPArray[newnode]->origin, gWPArray[atNode]->origin, vec );
						gc += VectorLength( vec );				//calculate what the gcost would be if we reached this node along the current path
						gWPArray[atNode]->neighbors[i].cost = VectorLength( vec );

						/*
						float height_diff = 0.0f;
						float cost = 0.0f;

						cost = Distance(gWPArray[newnode]->origin, gWPArray[atNode]->origin);

						height_diff = HeightDistance(gWPArray[newnode]->origin, gWPArray[atNode]->origin);
						cost += (height_diff * height_diff); // Squared for massive preferance to staying at same plane...

						gWPArray[atNode]->neighbors[i].cost = cost;

						gc += cost;
						*/
					}

					if ( gc < gcost[newnode] )				//if the new gcost is less (ie, this path is shorter than what we had before)
					{
						parent[newnode] = atNode;			//set the new parent for this node
						gcost[newnode] = gc;				//and the new g cost

						for ( j = 1; j < numOpen; j++ )		//loop through all the items on the open list
						{
							if ( openlist[j] == newnode )	//find this node in the list
							{
								//calculate the new fcost and store it
								fcost[newnode] = BOT_GetFCost( bot, to, newnode, parent[newnode], gcost );

								//reorder the list again, with the lowest fcost item on top
								m = j;

								while ( m != 1 )
								{
									if ( fcost[openlist[m]] < fcost[openlist[m / 2]] )	//if the item has a lower fcost than it's parent
									{
										temp = openlist[m / 2];
										openlist[m / 2] = openlist[m];
										openlist[m] = temp;								//swap them
										m /= 2;
									}
									else
									{
										break;
									}
								}
								break;													//exit the 'for' loop because we already changed this node
							}															//if
						}																//for
					}											//if (gc < gcost[newnode])
				}												//if (list[newnode] != 1) --> else
			}													//for (loop through links)
		}														//if (numOpen != 0)
		else
		{
			found = qfalse;										//there is no path between these nodes
			break;
		}

		if ( list[to] == 1 )									//if the destination node is on the open list, we're done
		{
			found = qtrue;
			break;
		}
	}															//while (1)

	if ( found == qtrue )							//if we found a path, and are trying to store the pathlist...
	{
		count = 0;
		temp = to;												//start at the end point
		
		while ( temp != from )									//travel along the path (backwards) until we reach the starting point
		{
			if (count+1 >= MAX_WPARRAY_SIZE)
			{
				trap->Print("ERROR: pathlist count > MAX_WPARRAY_SIZE.\n", count);
				return -1; // UQ1: Added to stop crash if path is too long for the memory allocation...
			}

			pathlist[count++] = temp;							//add the node to the pathlist and increment the count
			temp = parent[temp];								//move to the parent of this node to continue the path
		}

		pathlist[count++] = from;								//add the beginning node to the end of the pathlist

		//trap->Print("Pathsize is %i.\n", count);
		return ( count );
	}
#endif //__DISABLED_OLD_ASTAR__

	//trap->Print("Failed to find path.\n");
	return ( -1 );											//return the number of nodes in the path, -1 if not found
}

//extern int GetNearestWP(vec3_t org, int badwp);
void Update_DOM_Goal_Lists ( void );

extern int DOMObjectives[MAX_GENTITIES];
extern int num_DOM_objectives;

void ShowLinkInfo ( int wp, gentity_t *ent )
{
	int i = 0, unreachable = 0;

	if (wp < 0 || wp > gWPNum || !gWPArray[wp])
		return;

	for (i = 0; i < gWPArray[wp]->neighbornum; i++)
	{
		if (!gWPArray[gWPArray[wp]->neighbors[i].num])
			continue;

		if (!OrgVisible(gWPArray[wp]->origin, gWPArray[gWPArray[wp]->neighbors[i].num]->origin, -1))
			unreachable++;
	}

	trap->Print("Waypoint %i has %i links (%i seem to be unreachable).\n", wp, gWPArray[wp]->neighbornum, unreachable);
	trap->Print("Links are to waypoints: ");

	for (i = 0; i < gWPArray[wp]->neighbornum; i++)
	{
		if (i+1 == gWPArray[wp]->neighbornum)
			trap->Print("%i.\n", gWPArray[wp]->neighbors[i].num);
		else
			trap->Print("%i, ", gWPArray[wp]->neighbors[i].num);
	}
}


void 
AIMod_CreateNewRoute ( gentity_t *ent )
{
	int				i = 0, tries = 0;
	int				wp = DOM_GetBestWaypoint( ent->r.currentOrigin, ent->s.number, -1 );
	int				goal_wp;
	wpobject_t		*my_wp = NULL;
	wpobject_t		*my_wp_goal = NULL;

	if (wp < 0)
	{
		trap->Print( "No routes - No waypoint was found at your current position!\n");
		return;
	}

	my_wp = gWPArray[wp];

	trap->Print( "Finding bot objectives for %s at node number %i (%f %f %f).\n", ent->client->pers.netname,
		my_wp->index, my_wp->origin[0], my_wp->origin[1], my_wp->origin[2] );

	ShowLinkInfo(wp, ent);

	Update_DOM_Goal_Lists();

	i = irand(0, num_DOM_objectives);

	while (1)
	{
		gentity_t	*goal = &g_entities[DOMObjectives[i]];

		if (tries >= num_DOM_objectives*2)
		{
			trap->Print(  "Failed to find any path!\n"  );
			break;
		}

		goal_wp = DOM_GetNearestWP( goal->r.currentOrigin, -1 );
		my_wp_goal = NULL;

		if (goal_wp < 0)
		{
			i = irand(0, num_DOM_objectives);
			tries++;
			continue;
		}

		my_wp_goal = gWPArray[goal_wp];
		
		ent->pathsize = ASTAR_FindPathFast(wp, goal_wp, ent->pathlist, qtrue);

		//if (ent->pathsize <= 0) // Alt A* Pathing...
		//	ent->pathsize = DOM_FindIdealPathtoWP(NULL, wp, goal_wp, -1, ent->pathlist);

		if (ent->pathsize < 0 && tries < num_DOM_objectives*2)
		{
			i = irand(0, num_DOM_objectives);
			tries++;
			continue;
		}

		trap->Print( "Objective %i (%s) pathsize is %i.\n", i, goal->classname, ent->pathsize );
		break;
	}

	trap->Print( va( "Complete.\n") );
}

/*///////////////////////////////////////////////////
NPC_GetNextNode
if the bot has reached a node, this function selects the next node
that he will go to, and returns it
right now it's being developed, feel free to experiment
*////////////////////////////////////////////////////

int DOM_GetNextWaypoint(bot_state_t *bs)
{
	short int node = WAYPOINT_NONE;
	gentity_t *bot = &g_entities[bs->client];

	//we should never call this in BOTSTATE_MOVE with no goal
	//setup the goal/path in HandleIdleState
	if (bs->wpDestination->index == WAYPOINT_NONE)
	{
		return WAYPOINT_NONE;
	}

	if (bot->pathsize == 0)	//if the bot is at the end of his path, this shouldn't have been called
	{
		bs->wpDestination = NULL;/*gWPArray[WAYPOINT_NONE]*/;	//reset to having no goal
		return WAYPOINT_NONE;
	}

	node = bs->botRoute[bot->pathsize-1];	//pathlist is in reverse order
	bot->pathsize--;	//mark that we've moved another node

	return node;
}

/*
=========================
END A* Pathfinding Code
=========================
*/



//TAB bot Behaviors
//[Linux]
#ifndef __linux__
typedef enum
#else
enum
#endif
//[/Linux]
{
	BBEHAVE_NONE,  //use this if you don't want a behavior function to be run
	BBEHAVE_STILL, //bot stands still
	BBEHAVE_MOVETO, //Move to the current inputted goalPosition;
	BBEHAVE_ATTACK,  //Attack given entity
	BBEHAVE_VISUALSCAN	//visually scanning around
};


char *DOM_OrderNames[BOTORDER_MAX] = 
{
	"none",					//BOTORDER_NONE
	"kneel before",			//BOTORDER_KNEELBEFOREZOD
	"attack",				//BOTORDER_ATTACK
	"compete objectives",	//BOTORDER_OBJECTIVE
	"become infantry",		//BOTORDER_SIEGECLASS_INFANTRY
	"become scout",			//BOTORDER_SIEGECLASS_VANGUARD
	"become tech",			//BOTORDER_SIEGECLASS_SUPPORT
	"become Jedi",			//BOTORDER_SIEGECLASS_JEDI
	"become demolitionist",	//BOTORDER_SIEGECLASS_DEMOLITIONIST
	"become heavy weapons"	//BOTORDER_SIEGECLASS_HEAVY_WEAPONS
	//BOTORDER_MAX
};


void DOM_BotOrder( gentity_t *orderer, gentity_t *orderee, int order, gentity_t *objective)
{
	bot_state_t *bs;
	if(!orderer || !orderee || !orderer->client || !orderee->client || !(orderee->r.svFlags & SVF_BOT))
	{//Sanity check
		return;
	}

	bs = botstates[orderee->client->ps.clientNum];

	//orders with objective entities
	if(order == BOTORDER_KNEELBEFOREZOD || (order == BOTORDER_SEARCHANDDESTROY && objective) )
	{
		if(objective)
		{
			bs->botOrder = order;
			bs->orderEntity = objective;
			bs->ordererNum = orderer->client->ps.clientNum;
			//orders for a client objective 
			if(objective->client)
			{
				trap->Print("%s ordered %s to %s %s\n", orderer->client->pers.netname, orderee->client->pers.netname, DOM_OrderNames[order], objective->client->pers.netname);
			}
		}
		else
		{//no objective!  bad!
			return;
		}
	}
	else if ( order == BOTORDER_SEARCHANDDESTROY)
	{
		bs->botOrder = order;
		bs->orderEntity = NULL;
		bs->ordererNum = orderer->client->ps.clientNum;
		trap->Print("%s ordered %s to %s\n", orderer->client->pers.netname, orderee->client->pers.netname, DOM_OrderNames[order]);

	}
	else
	{//bad order
		return;
	}

	BotDoChat(bs, "OrderAccepted", 1);
}


//BOTORDER_KNEELBEFOREZOD Defines
//Distance at which point you stop and kneel before kneel target
#define	KNEELZODDISTANCE	100

//Distance at which point you quit running towards kneel target
#define	WALKZODDISTANCE		200


//Bot Order implimentation for BOTORDER_KNEELBEFOREZOD
void DOM_BotKneelBeforeZod(bot_state_t *bs)
{
	int dist;

	//Sanity checks
	if(!bs->tacticEntity || !bs->tacticEntity->client)
	{//bad!
		trap->Print("Bad tacticEntity sent to DOM_BotKneelBeforeZod.\n");
		return;
	}

	dist = Distance(bs->origin, bs->tacticEntity->client->ps.origin);

	if( dist < KNEELZODDISTANCE || bs->doZodKneel)
	{//Close enough to hold and kneel
		vec3_t ang;
		vec3_t viewDir;

		bs->duckTime = level.time + 100;
		bs->botBehave = BBEHAVE_STILL;	

		VectorSubtract(bs->tacticEntity->client->ps.origin, bs->eye, viewDir);
		vectoangles(viewDir, ang);
		VectorCopy(ang, bs->goalAngles);
		bs->goalAngles[PITCH] = bs->goalAngles[ROLL] = 0;

		if(bs->cur_ps.weapon != WP_SABER && bs->virtualWeapon != WP_SABER )
		{//don't have saber selected, try switching to it
			BotSelectChoiceWeapon(bs, WP_SABER, 1);
		}
		else if(bs->cur_ps.weapon == WP_SABER)
		{
				if (!bs->cur_ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(&g_entities[bs->client]);
				}
		}

		if(bs->doZodKneel)
		{
			if(bs->zodKneelTime < level.time)
			{//objective complete, clear tactic and order
				bs->currentTactic = BOTORDER_NONE;
				bs->doZodKneel = qfalse;
				if(bs->botOrder == BOTORDER_KNEELBEFOREZOD)
				{
					bs->botOrder = BOTORDER_NONE;
					bs->orderEntity = NULL;
				}
			}
		}
		else
		{
			bs->doZodKneel = qtrue;
			bs->zodKneelTime = level.time + Q_irand(5000, 15000);
		}

	}
	else
	{//Still need to move to kneel target
		if( dist < WALKZODDISTANCE )
		{//close enough to quit running like an idiot.
			bs->doWalk = qtrue;
		}

		bs->botBehave = BBEHAVE_MOVETO;
		VectorCopy(bs->tacticEntity->client->ps.origin, bs->DestPosition);
		bs->DestIgnore = bs->tacticEntity->s.number;
	}

}


//Just stand still
void DOM_BotBeStill(bot_state_t *bs)
{
	VectorCopy(bs->origin, bs->goalPosition);
	bs->beStill = level.time + 100;
//	VectorClear(bs->DestPosition);
//	bs->DestIgnore = -1;
//	bs->wpCurrent = NULL;
}


//TAB version of BotTrace_Jump()
//This isn't used anymore.  Use DOM_TraceJumpCrouchFall
//1 = can/should jump to clear
//0 = don't jump
int DOM_TraceJump(bot_state_t *bs, vec3_t traceto)
{
	vec3_t mins, maxs, fwd, traceto_mod, tracefrom_mod;
	trace_t tr;
	int orTr;

	VectorSubtract(traceto, bs->origin, fwd);
	

	traceto_mod[0] = bs->origin[0] + fwd[0]*4;
	traceto_mod[1] = bs->origin[1] + fwd[1]*4;
	traceto_mod[2] = bs->origin[2] + fwd[2]*4;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -18;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 32;

	trap->Trace(&tr, bs->origin, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction == 1)
	{
		return 0;
	}

	orTr = tr.entityNum;

	VectorCopy(bs->origin, tracefrom_mod);

	tracefrom_mod[2] += 41;
	traceto_mod[2] += 41;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = 0;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 8;

	trap->Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction == 1)
	{
		if (orTr >= 0 && orTr < MAX_CLIENTS && botstates[orTr] && botstates[orTr]->jumpTime > level.time)
		{
			return 0; //so bots don't try to jump over each other at the same time
		}

		return 1;
	}

	return 0;
}


//Performs the obsticle jump/crouch/fall traces
//moveDir must be normalized.
//0 = no obstruction/action needed
//1 = Force Jump
//2 = crouch
//-1 = cliff or no way to get there from here.
int DOM_TraceJumpCrouchFall(bot_state_t *bs, vec3_t moveDir, int targetNum)
{
	vec3_t mins, maxs, traceto_mod, tracefrom_mod;
	trace_t tr;
	int orTr;

	//Fall Check
	traceto_mod[0] = bs->origin[0] + moveDir[0]*45;
	traceto_mod[1] = bs->origin[1] + moveDir[1]*45;
	traceto_mod[2] = bs->origin[2] + moveDir[2]*45;

	VectorCopy(traceto_mod, tracefrom_mod);

	//check for 50+ feet drops
	traceto_mod[2] -= 532;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -18;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 32;

	trap->Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_SOLID, 0, 0, 0);
	if ((tr.fraction == 1 && !tr.startsolid))
	{//CLIFF! or dangerous liquid.
		return -1;
	}

	if(bs->jumpTime < level.time && tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
	{//don't try to go into lava/slime
		return -1;
	}

	//Ok, check for obstructions then.
	traceto_mod[0] = bs->origin[0] + moveDir[0]*19;
	traceto_mod[1] = bs->origin[1] + moveDir[1]*19;
	traceto_mod[2] = bs->origin[2] + moveDir[2]*19;

	//obstruction trace
	trap->Trace(&tr, bs->origin, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction == 1 || tr.entityNum == targetNum)
	{//nothing blocking our path
		return 0;
	}

	//check the slope of the thing blocking us
	if(tr.entityNum == ENTITYNUM_WORLD)
	{
		vec3_t ang;
		vectoangles(tr.plane.normal, ang);
		if(ang[PITCH] < 0 && ang[PITCH] > -50)
		{//you're probably moving up a steep ledge that's still walkable
			return 0;
		}
	}

	orTr = tr.entityNum;

	VectorCopy(bs->origin, tracefrom_mod);

	traceto_mod[0] = bs->origin[0] + moveDir[0]*45;
	traceto_mod[1] = bs->origin[1] + moveDir[1]*45;

	//RAFIXME - make this based on Force jump skill level/force power availible.
	tracefrom_mod[2] += 20;
	traceto_mod[2] += 20;
	//tracefrom_mod[2] += 41;
	//traceto_mod[2] += 41;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = 0;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 8;

	trap->Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction == 1 || tr.entityNum == targetNum)
	{
		if (orTr >= 0 && orTr < MAX_CLIENTS && botstates[orTr] && botstates[orTr]->jumpTime > level.time)
		{
			return 0; //so bots don't try to jump over each other at the same time
		}

		if(bs->currentEnemy)
		{
			if(bs->currentEnemy->s.number != orTr)
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}

	//ok, can't jump the obsticle, let's try crawling under it.
	VectorCopy(bs->origin, tracefrom_mod);

	traceto_mod[0] = bs->origin[0] + moveDir[0]*19;
	traceto_mod[1] = bs->origin[1] + moveDir[1]*19;
	traceto_mod[2] = bs->origin[2] + moveDir[2]*19;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -23;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 8;

	trap->Trace(&tr, tracefrom_mod, mins, maxs, traceto_mod, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction == 1 || tr.entityNum == targetNum)
	{
		return 2;
	}

	//Well, none of that worked.
	return -1;
}


//701
//6 2
//543
//Find the movement quad you're trying to move into
int DOM_FindMovementQuad( playerState_t *ps, vec3_t moveDir )
{
	vec3_t viewfwd, viewright;
	vec3_t move;
	float x;
	float y;

	AngleVectors(ps->viewangles, viewfwd, viewright, NULL);

	VectorCopy(moveDir, move);

	viewfwd[2] = 0;
	viewright[2] = 0;
	move[2] = 0;

	VectorNormalize(viewfwd);
	VectorNormalize(viewright);
	VectorNormalize(move);

	x = DotProduct(viewright, move);
	y = DotProduct(viewfwd, move);

	if( x > .8 )
	{//right
		return 2;
	}
	else if( x < -0.8 )
	{//left
		return 6;
	}
	else if( x > .2 )
	{//right forward/back
		if( y < 0 )
		{//right back
			return 3;
		}
		else
		{//right forward
			return 1;
		}
	}
	else if ( x < -0.2 )
	{//left forward/back
		if( y < 0 )
		{//left back
			return 5;
		}
		else
		{//left forward
			return 7;
		}
	}
	else
	{//forward/back
		if( y < 0 )
		{//back
			return 4;
		}
		else
		{//forward
			return 0;
		}
	}
}


//701
//6 2
//543
//Adjusts moveDir based on a given move Quad.
//moveDir is suppose to be VectorNormalized
void DOM_AdjustMoveDirection( bot_state_t *bs, vec3_t moveDir, int Quad )
{
	vec3_t fwd, right;
	vec3_t addvect;

	AngleVectors(bs->cur_ps.viewangles, fwd, right, NULL);
	fwd[2] = 0;
	right[2] = 0;

	VectorNormalize(fwd);
	VectorNormalize(right);

	switch(Quad)
	{
		case 0:
			VectorCopy(fwd, addvect);
			break;
		case 1:
			VectorAdd(fwd, right, addvect);
			VectorNormalize(addvect);
			break;
		case 2:
			VectorCopy(right, addvect);
			break;
		case 3:
			VectorScale(fwd, -1, fwd);
			VectorAdd(fwd, right, addvect);
			VectorNormalize(addvect);
			break;
		case 4:
			VectorScale(fwd, -1, addvect);
			break;
		case 5:
			VectorScale(fwd, -1, fwd);
			VectorScale(right, -1, right);
			VectorAdd(fwd, right, addvect);
			VectorNormalize(addvect);
			break;
		case 6:
			VectorScale(right, -1, addvect);
			break;
		case 7:
			VectorScale(right, -1, right);
			VectorAdd(fwd, right, addvect);
			VectorNormalize(addvect);
			break;
		default:
			trap->Print("Bad Quad in DOM_AdjustMoveDirection.\n");
			return;
	}

	//VectorAdd(addvect, moveDir, moveDir);
	VectorCopy(addvect, moveDir);
	VectorNormalize(moveDir);
}


//process the return value from DOM_TraceJumpCrouchFall to do whatever you need to do to 
//get where.
void DOM_MovementCommand(bot_state_t *bs, int command, vec3_t moveDir)
{
	if(!command)
	{//don't need to do anything
		return;
	}
	else if(command == 1)
	{//Force Jump
		//bs->jumpTime = level.time + 100;
		return;
	}
	else if(command == 2)
	{//crouch
		bs->duckTime = level.time + 100;
		return;
	}
	else
	{//can't move!
		VectorCopy(vec3_origin, moveDir);
	}
}


//701
//6 2
//543
//shift the Quad to make sure it's aways valid
int DOM_AdjustQuad(int Quad)
{
	int Dir = Quad;
	while(Dir > 7)
	{//shift
		Dir -= 8;
	}
	while(Dir < 0)
	{//shift	
		Dir += 8;
	}

	return Dir;
}


//do all the nessicary calculations/traces for movement, automatically uses obstical evasion
//targetNum is the moveto target.  you're suppose to trace hit that eventually
void DOM_TraceMove(bot_state_t *bs, vec3_t moveDir, int targetNum)
{	
	vec3_t Dir;
	int movecom;
	int fwdstrafe;
	int i = 7;
	int Quad;

	return;

	movecom = DOM_TraceJumpCrouchFall(bs, moveDir, targetNum);

	VectorCopy(moveDir, Dir);

	if(movecom != -1)
	{
		DOM_MovementCommand(bs, movecom, Dir);
		return;
	}

	if(bs->evadeTime > level.time)
	{//try the current evade direction to prevent spazing
		DOM_AdjustMoveDirection(bs, Dir, bs->evadeDir);
		movecom = DOM_TraceJumpCrouchFall(bs, Dir, targetNum);
		if(movecom != -1)
		{
			DOM_MovementCommand(bs, movecom, Dir);
			VectorCopy(Dir, moveDir);
			bs->evadeTime = level.time + 500;
			return;
		}
		i--;
	}

	//Since our default direction didn't work we need to switch melee strafe directions if 
	//we are melee strafing.
	//0 = no strafe
	//1 = strafe right
	//2 = strafe left
	if( bs->meleeStrafeTime > level.time )
	{
		bs->meleeStrafeDir = Q_irand(0,2);
		bs->meleeStrafeTime = level.time + Q_irand(500, 1800);
	}

	fwdstrafe = DOM_FindMovementQuad(&bs->cur_ps, moveDir);
	
	if(Q_irand(0, 1))
	{//try strafing left 
		Quad = fwdstrafe - 2; 
	}
	else
	{
		Quad = fwdstrafe + 2;
	}

	Quad = DOM_AdjustQuad(Quad);
	
	//reset Dir to original moveDir
	VectorCopy(moveDir, Dir);

	//shift movedir for quad
	DOM_AdjustMoveDirection(bs, Dir, Quad);

	movecom = DOM_TraceJumpCrouchFall(bs, Dir, targetNum);

	if(movecom != -1)
	{
		DOM_MovementCommand(bs, movecom, Dir);
		VectorCopy(Dir, moveDir);
		bs->evadeDir = Quad;
		bs->evadeTime = level.time + 100;
		return;
	}
	i--;

	//no luck, try the other full strafe direction
	Quad += 4;
	Quad = DOM_AdjustQuad(Quad);

	//reset Dir to original moveDir
	VectorCopy(moveDir, Dir);

	//shift movedir for quad
	DOM_AdjustMoveDirection(bs, Dir, Quad);

	movecom = DOM_TraceJumpCrouchFall(bs, Dir, targetNum);

	if(movecom != -1)
	{
		DOM_MovementCommand(bs, movecom, Dir);
		VectorCopy(Dir, moveDir);
		bs->evadeDir = Quad;
		bs->evadeTime = level.time + 100;
		return;
	}
	i--;

	//still no dice
	for(; i > 0; i--)
	{
		Quad++;
		Quad = DOM_AdjustQuad(Quad);

		if(Quad == fwdstrafe || Quad == DOM_AdjustQuad(fwdstrafe - 2) || Quad == DOM_AdjustQuad(fwdstrafe + 2) 
			|| (bs->evadeTime > level.time && Quad == bs->evadeDir) )
		{//Already did those directions
			continue;
		}

		VectorCopy(moveDir, Dir);

		//shift movedir for quad
		DOM_AdjustMoveDirection(bs, Dir, Quad);
		movecom = DOM_TraceJumpCrouchFall(bs, Dir, targetNum);
		if(movecom != -1)
		{//find a good direction
			bs->evadeDir = Quad;
			bs->evadeTime = level.time + 100;
			break;
		}

	}

	DOM_MovementCommand(bs, movecom, Dir);

	if(!VectorCompare(Dir, moveDir) )
	{
		VectorCopy(Dir, moveDir);
	}

}


//does this current inventory have a heavy weapon?
qboolean DOM_HaveHeavyWeapon(int weapons)
{
	if( (weapons & (1 << WP_SABER))
		|| (weapons & (1 << WP_ROCKET_LAUNCHER))
		|| (weapons & (1 << WP_THERMAL))
		|| (weapons & (1 << WP_TRIP_MINE))
		|| (weapons & (1 << WP_DET_PACK))
		|| (weapons & (1 << WP_CONCUSSION)))
	{
		return qtrue;
	}
	return qfalse;
}


//checks to see if this weapon will damage FL_DMG_BY_HEAVY_WEAP_ONLY targets
qboolean DOM_IsHeavyWeapon(bot_state_t *bs, int weapon)
{//right now we only show positive for weapons that can do this in primary fire mode
	switch (weapon)
	{
	case WP_SABER:
	case WP_ROCKET_LAUNCHER:
	case WP_THERMAL:
	case WP_DET_PACK:
	case WP_CONCUSSION:
		return qtrue;
		break;
	};

	return qfalse;
}

//use Force push or pull on local func_doors
qboolean DOM_UseForceonLocal(bot_state_t *bs, vec3_t origin, qboolean pull)
{
	gentity_t *test = NULL;
	vec3_t center, pos1, pos2, size;
	qboolean SkipPushPull = qfalse;


	if(bs->DontSpamPushPull > level.time)
	{//pushed/pulled for this waypoint recently
		SkipPushPull = qtrue;
	}

	if(!SkipPushPull)
	{
		//Force Push/Pull
		while ( (test = G_Find( test, FOFS( classname ), "func_door" )) != NULL )
		{
			if(!(test->spawnflags & 2/*MOVER_FORCE_ACTIVATE*/))
			{//can't use the Force to move this door, ignore
				continue;
			}

			if(test->wait < 0 && test->moverState == MOVER_POS2)
			{//locked in position already, ignore.
				continue;
			}

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

			if(Distance(origin, center) > 400)
			{//too far away
				continue;
			}

			if ( Distance( pos1, bs->eye ) < Distance( pos2, bs->eye ) )
			{//pos1 is closer
				if ( test->moverState == MOVER_POS1 )
				{//at the closest pos
					if ( pull )
					{//trying to pull, but already at closest point, so screw it
						continue;
					}
				}
				else if ( test->moverState == MOVER_POS2 )
				{//at farthest pos
					if ( !pull )
					{//trying to push, but already at farthest point, so screw it
						continue;
					}
				}
			}
			else
			{//pos2 is closer
				if ( test->moverState == MOVER_POS1 )
				{//at the farthest pos
					if ( !pull )
					{//trying to push, but already at farthest point, so screw it
						continue;
					}
				}
				else if ( test->moverState == MOVER_POS2 )
				{//at closest pos
					if ( pull )
					{//trying to pull, but already at closest point, so screw it
						continue;
					}
				}
			}
			//we have a winner!
			break;
		}

		if(test)
		{//have a push/pull able door
			vec3_t viewDir;
			vec3_t ang;
			
			//doing special wp move
			bs->wpSpecial = qtrue;

			VectorSubtract(center, bs->eye, viewDir);
			vectoangles(viewDir, ang);
			VectorCopy(ang, bs->goalAngles);
			bs->goalAngles[PITCH] = bs->goalAngles[ROLL] = 0;
			
			if(InFieldOfVision(bs->viewangles, 5, ang))
			{//use the force
				if(pull)
				{
					bs->doForcePull = level.time + 700;
				}
				else
				{
					bs->doForcePush = level.time + 700;
				}
				//Only press the button once every 15 seconds
				//this way the bots will be able to go up/down a lift weither the elevator
				//was up or down.
				//This debounce only applies to this waypoint.
				bs->DontSpamPushPull = level.time + 15000;
			}
			return qtrue;
		}
	}

	if(bs->DontSpamButton > level.time)
	{//pressed a button recently
		if(bs->useTime > level.time)
		{//holding down button
			//update the hacking buttons to account for lag about crap
			if(g_entities[bs->client].client->isHacking)
			{
				bs->useTime = bs->cur_ps.hackingTime + 100;
				bs->DontSpamButton = bs->useTime + 15000;
				bs->wpSpecial = qtrue;
				return qtrue;
			}

			//lost hack target.  clear things out
			bs->useTime = level.time;
			return qfalse;
		}
		else
		{
			return qfalse;
		}
	}

	//use button checking
	while ( (test = G_Find( test, FOFS( classname ), "trigger_multiple" )) != NULL )
	{
		if ( test->flags & FL_INACTIVE )
		{//set by target_deactivate
			continue;
		}

		if (!(test->spawnflags & 4) /*USE_BUTTON*/)
		{//can't use button this trigger
			continue;
		}

		if(test->alliedTeam)
		{
			if( g_entities[bs->client].client->sess.sessionTeam != test->alliedTeam )
			{//not useable by this team
				continue;
			}
		}

		DOM_FindOrigin(test, center);

		if(Distance(origin, center) > 200)
		{//too far away
			continue;
		}

		break;
	}

	if(!test)
	{//not luck so far
		while ( (test = G_Find( test, FOFS( classname ), "trigger_once" )) != NULL )
		{
			if ( test->flags & FL_INACTIVE )
			{//set by target_deactivate
				continue;
			}

			if(!test->use)
			{//trigger already fired
				continue;
			}

			if (!(test->spawnflags & 4) /*USE_BUTTON*/)
			{//can't use button this trigger
				continue;
			}

			if(test->alliedTeam)
			{
				if( g_entities[bs->client].client->sess.sessionTeam != test->alliedTeam )
				{//not useable by this team
					continue;
				}
			}
		
			DOM_FindOrigin(test, center);

			if(Distance(origin, center) > 200)
			{//too far away
				continue;
			}

			break;
		}
	}

	if(test)
	{//found a pressable/hackable button
		vec3_t ang, viewDir;
		//trace_t tr;

		//trap->Trace(&tr, bs->eye, NULL, NULL, center, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

		//if(tr.entityNum == test->s.number || tr.fraction == 1.0)
		{
			bs->wpSpecial = qtrue;
			
			//you can use use
			//set view angles.
			if(test->spawnflags & 2 /*FACING*/)
			{//you have to face in the direction of the trigger to have it work
				vectoangles(test->movedir, ang);
			}
			else
			{
				VectorSubtract( center, bs->eye, viewDir);
				vectoangles(viewDir, ang);
			}

			VectorCopy(ang, bs->goalAngles);
			bs->goalAngles[PITCH] = bs->goalAngles[ROLL] = 0;

			if (G_PointInBounds( bs->origin, test->r.absmin, test->r.absmax ))
			{//inside trigger zone, press use.
				bs->useTime = level.time + test->genericValue7 + 100;
				bs->DontSpamButton = bs->useTime + 15000;
			}
			else
			{//need to move closer
				VectorSubtract(center, bs->origin, viewDir);

				viewDir[2] = 0;
				VectorNormalize(viewDir);

				trap->EA_Move(bs->client, viewDir, 5000);
			}
			return qtrue;
		}
	}
	return qfalse;
}


//scan for nearby breakables that we can destroy
qboolean DOM_AttackLocalBreakable(bot_state_t *bs, vec3_t origin)
{
	gentity_t *test = NULL;
	gentity_t *valid = NULL;
	qboolean defend = qfalse;
	vec3_t testorigin;

	while ( (test = G_Find( test, FOFS( classname ), "func_breakable" )) != NULL )
	{
		DOM_FindOrigin(test, testorigin);

		if( DOM_TargetDistance(bs, test, testorigin) < 300 )
		{
			if(test->teamnodmg && test->teamnodmg == g_entities[bs->client].client->sess.sessionTeam)
			{//on a team that can't damage this breakable, as such defend it from immediate harm
				defend = qtrue;
			}
		
			valid = test;
			break;
		}

		//reset for next check
		VectorClear(testorigin);
	}

	if(valid)
	{//due to crazy stack overflow issues, just attack wildly while moving towards the
		//breakable
		trace_t tr;
		int desiredweap = DOM_FavoriteWeapon(bs, valid);	

		VectorCopy(bs->origin, bs->eye);
		bs->eye[2]+=32; //up...

		//visual check
		trap->Trace(&tr, bs->eye, NULL, NULL, testorigin, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

		if(tr.entityNum == test->s.number || tr.fraction == 1.0)
		{//we can see the breakable

			//doing special wp move
			bs->wpSpecial = qtrue;

			if(defend)
			{//defend this target since we can assume that the other team is going to try to
				//destroy this thingy
				//RAFIXME:  Add repair code.
				DOM_BotBehave_DefendBasic(bs, testorigin);
			}
			else if((test->flags & FL_DMG_BY_HEAVY_WEAP_ONLY) && !DOM_IsHeavyWeapon(bs, desiredweap))
			{//we currently don't have a heavy weap that we can use to destroy this target
				if(DOM_HaveHeavyWeapon(bs->cur_ps.stats[STAT_WEAPONS]))
				{//we have a weapon that could destroy this target but we don't have ammo
					//RAFIXME:  at this point we should have the bot go look for some ammo
					//but for now just defend this area.
					DOM_BotBehave_DefendBasic(bs, testorigin);
				}
				else
				{//go hunting for a weapon that can destroy this object
					//RAFIXME:  Add this code
					DOM_BotBehave_DefendBasic(bs, testorigin);
				}
			}
			else
			{//ATTACK!
				//determine which weapon you want to use
				if(desiredweap != bs->virtualWeapon)
				{//need to switch to desired weapon
					BotSelectChoiceWeapon(bs, desiredweap, qtrue);
				}
				DOM_BotBehave_AttackBasic(bs, valid);
			}


			return qtrue;
		}
	}
	return qfalse;
}


qboolean DOM_CalculateJump(bot_state_t *bs, vec3_t origin, vec3_t dest)
{//should we try jumping to this location?
	vec3_t flatorigin, flatdest;
	float dist;
	float heightDif = dest[2] - origin[2];
	
	VectorCopy(origin, flatorigin);
	VectorCopy(dest, flatdest);

	dist = Distance(flatdest, flatorigin);

	if(heightDif > 30 && dist < 100)
	{
		return qtrue;
	}
	return qfalse;
}


//update the waypoint visibility debounce
void DOM_WPVisibleUpdate(bot_state_t *bs)
{
	vec3_t eye;

	VectorCopy(bs->origin, eye);
	eye[2]+=32; // UQ1: Make it easier to see over bumps, etc :)

	if(OrgVisible(eye, bs->wpCurrent->origin, bs->client))
	//if (Distance(bs->origin, bs->wpCurrent->origin) > 256)
	{//see the waypoint hold the counter
		bs->wpSeenTime = level.time + 6000;
	}
}

void DOM_IncreaseWaypointLinkCost(bot_state_t *bs)
{
	int i = 0;

	if (!bs->wpLast || !bs->wpCurrent)
		return;

	if (bs->frame_Enemy_Vis || bs->enemySeenTime < level.time - 2000)
		return; // Do not do this if we have been fighting recently... (2 secs)..

	for (i = 0; i < bs->wpLast->neighbornum; i++)
	{
		if (bs->wpLast->neighbors[i].num == bs->wpCurrent->index)
		{// Found the link.. Increase the link's cost so it doesnt get used again unless there is no choice!
			if (bs->wpLast->neighbors[i].cost)
				bs->wpLast->neighbors[i].cost *= 2;
			else
				bs->wpLast->neighbors[i].cost = 999999; // wtf 999999.9f

			//trap->Print("BOT DEBUG: Increasing cost of wp %i link %i.\n", bs->wpLast->index, i);
			break;
		}
	}
}

extern qboolean JKG_CheckBelowPoint( vec3_t point );

typedef enum
{// Avoidance methods...
	AVOIDANCE_NONE,
	AVOIDANCE_STRAFE_RIGHT,
	AVOIDANCE_STRAFE_LEFT,
	AVOIDANCE_STRAFE_JUMP,
} avoidanceMethods_t;

int DOM_SelectBestAvoidanceMethod(bot_state_t *bs, gentity_t *NPC)
{// Just find the largest visible distance direction...
	/*
	trace_t		tr;
	vec3_t		org1, org2;
	vec3_t		forward, right;
	float		distance = 0.0f;
	float		BEST_DISTANCE = 0.0f;
	int			BEST_AVOID_TYPE = AVOIDANCE_NONE;

	if (NPC->bot_strafe_right_timer > level.time)
		return AVOIDANCE_STRAFE_RIGHT;

	if (NPC->bot_strafe_left_timer > level.time)
		return AVOIDANCE_STRAFE_LEFT;

	if (NPC->bot_strafe_jump_timer > level.time)
		return AVOIDANCE_STRAFE_JUMP;

	VectorCopy(NPC->r.currentOrigin, org1);
	org1[2] += 24;

	AngleVectors( NPC->r.currentAngles, forward, right, NULL );
	
	// Check right side...
	VectorMA( org1, 256, right, org2 );
	trap->Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
	distance = Distance(NPC->r.currentOrigin, tr.endpos);
	
	if (JKG_CheckBelowPoint(org2))
	{
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_RIGHT;
		NPC->bot_strafe_right_timer = level.time + 500;
		NPC->bot_strafe_left_timer = 0;
		NPC->bot_strafe_jump_timer = 0;
	}

	// Check left side...
	VectorMA( org1, -256, right, org2 );
	trap->Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
	distance = Distance(NPC->r.currentOrigin, tr.endpos);
	
	if (distance > BEST_DISTANCE && JKG_CheckBelowPoint(org2))
	{// Better...
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_LEFT;
		NPC->bot_strafe_right_timer = 0;
		NPC->bot_strafe_left_timer = level.time + 500;
		NPC->bot_strafe_jump_timer = 0;
	}

	// Check jump...
	VectorMA( org1, 256, forward, org2 );
	trap->Trace( &tr, org1, NULL, NULL, org2, NPC->s.number, MASK_PLAYERSOLID , 0, 0, 0);
	distance = Distance(NPC->r.currentOrigin, tr.endpos);
	
	if (distance >= BEST_DISTANCE && JKG_CheckBelowPoint(org2))
	{// Better...
		BEST_DISTANCE = distance;
		BEST_AVOID_TYPE = AVOIDANCE_STRAFE_JUMP;
		NPC->bot_strafe_left_timer = 0;
		NPC->bot_strafe_right_timer = 0;
		NPC->bot_strafe_jump_timer = level.time + 100;
	}

	return BEST_AVOID_TYPE;
	*/

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

	//if (NPC->bot_strafe_crouch_timer > level.time)
	//	return AVOIDANCE_STRAFE_CROUCH;

	if (NPC->bot_strafe_jump_timer > level.time)
		return AVOIDANCE_STRAFE_JUMP;

	//if (NPC_FindTemporaryWaypoint())
	//	return AVOIDANCE_NONE;

	if (NPC->wpCurrent <= 0 || NPC->wpCurrent >= gWPNum)
		return AVOIDANCE_NONE;

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
			AngleVectors( NPC->client->ps.viewangles, forward, right, NULL );
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
			AngleVectors( NPC->client->ps.viewangles, forward, right, NULL );
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

int DOM_Avoidance(bot_state_t *bs, gentity_t *NPC)
{
	int BEST_METHOD = DOM_SelectBestAvoidanceMethod(bs, NPC);

	if (NPC->client->ps.pm_flags & PMF_JUMP_HELD)
		NPC->client->ps.pm_flags &= ~PMF_JUMP_HELD;

	switch (BEST_METHOD)
	{
	case AVOIDANCE_STRAFE_RIGHT:
		return AVOIDANCE_STRAFE_RIGHT;
		break;
	case AVOIDANCE_STRAFE_LEFT:
		return AVOIDANCE_STRAFE_LEFT;
		break;
	case AVOIDANCE_STRAFE_JUMP:
		NPC->client->ps.pm_flags |= PMF_JUMP_HELD;
		trap->EA_Jump(bs->client);
		return AVOIDANCE_STRAFE_JUMP;
		break;
	default:
		break;
	}
	
	return AVOIDANCE_NONE;
}

//move to the given location.  This is for point to pointto point navigation only.  Use 
//DOM_BotMoveto if you want the bot to use waypoints instead of just blindly moving
//towards the target location.
//wptravel = traveling to waypoint?
//strafe = strafe while moving here?
//target = ignore number for trace move stuff, this should be for the person you're 
//attacking/moving to

gentity_t * DOM_DetermineObjectiveType(gentity_t *bot, int team, int objective, int *type, int wanted_type, gentity_t *obj, int attacker);
int DOM_PickWARZONEGoalType(bot_state_t *bs);

void DOM_UQ1_UcmdMoveForDir ( bot_state_t *bs, usercmd_t *cmd, vec3_t dir )
{
	vec3_t	forward, right, up;
	float	speed = 127.0f;

	/*
	if ( cmd->buttons & BUTTON_WALKING )
	{
		//speed = 64.0f;
		speed = 90.0f;

		NPCInfo->stats.walkSpeed = 90;
		NPCInfo->stats.runSpeed = 200;
		NPC->NPC->desiredSpeed = NPC->NPC->stats.walkSpeed;
		NPC->client->ps.basespeed = NPC->client->ps.speed = NPCInfo->stats.walkSpeed;

		trap->Print("Speed is %f. (w: %i) (r: %i)\n", NPC->client->ps.speed, NPCInfo->stats.walkSpeed, NPCInfo->stats.runSpeed);
	}
	*/
	
	AngleVectors( g_entities[bs->client].client->ps.viewangles, forward, right, up );

	dir[2] = 0;
	VectorNormalize( dir );
	cmd->forwardmove = DotProduct( forward, dir ) * speed;
	cmd->rightmove = DotProduct( right, dir ) * speed;

	//cmd->upmove = abs(forward[3] ) * dir[3] * speed; // LOL BRO DO YOU EVEN VECTOR? --eez
	cmd->upmove = fabsf(forward[2] ) * dir[2] * speed;
}

void DOM_BotMove(bot_state_t *bs, vec3_t dest, qboolean wptravel, qboolean strafe)
{
	//vec3_t moveDir;
	vec3_t viewDir;
	vec3_t ang;
	qboolean movetrace = qtrue;

	/*if(wptravel && bs->wpCurrent)
	{
		VectorCopy(bs->wpCurrent->origin, dest);
	}*/

	VectorCopy(g_entities[bs->client].r.currentOrigin, bs->eye);
	bs->eye[2]+=DEFAULT_VIEWHEIGHT;

	VectorSubtract(dest, bs->eye, viewDir);
	vectoangles(viewDir, ang);
	VectorCopy(ang, bs->goalAngles);
	bs->goalAngles[PITCH] = bs->goalAngles[ROLL] = 0;

	if(wptravel)
	{//if we're traveling between waypoints, don't bob the view up and down.
		bs->goalAngles[PITCH] = 0;
	}

	VectorSubtract(dest, bs->origin, bs->goalMovedir); 
	bs->goalMovedir[2] = 0;
	VectorNormalize(bs->goalMovedir);

	if (wptravel && !bs->wpCurrent)
	{
		return;
	}
	else if( wptravel )
	{
		//special wp moves
		if (bs->wpCurrent->flags & WPFLAG_DESTROY_FUNCBREAK)
		{//look for nearby func_breakable and break them if we can before we continue
			if(DOM_AttackLocalBreakable(bs, bs->wpCurrent->origin))
			{//found a breakable that we can destroy
				bs->wpSeenTime = level.time + 6000;
				//DOM_WPVisibleUpdate(bs);
				return;
			}
		}

		if (bs->wpCurrent->flags & WPFLAG_FORCEPUSH)
		{
			if(DOM_UseForceonLocal(bs, bs->wpCurrent->origin, qfalse))
			{//found something we can Force Push
				bs->wpSeenTime = level.time + 6000;
				//DOM_WPVisibleUpdate(bs);
				return;
			}
		}

		if (bs->wpCurrent->flags & WPFLAG_FORCEPULL)
		{
			if(DOM_UseForceonLocal(bs, bs->wpCurrent->origin, qtrue))
			{//found something we can Force Pull
				DOM_WPVisibleUpdate(bs);
				return;
			}
		}

		if (bs->wpCurrent->flags & WPFLAG_JUMP 
			|| (dest[2] > bs->origin[2] + 8)
			|| VectorLength(g_entities[bs->client].playerState->velocity) < 8)
		{ //jump while travelling to this point
			vec3_t ang;
			vec3_t viewang;
			vec3_t velocity;
			vec3_t flatorigin, flatstart, flatend;
			float diststart;
			float distend;

			VectorCopy(bs->origin, flatorigin);
			VectorCopy(bs->wpCurrentLoc, flatstart);
			VectorCopy(bs->wpCurrent->origin, flatend);

			flatorigin[2] = flatstart[2] = flatend[2] = 0;

			diststart = Distance(flatorigin, flatstart);
			distend = Distance(flatorigin, flatend);

			VectorSubtract(dest, bs->origin, viewang); 
			vectoangles(viewang, ang);
			ang[PITCH] = ang[ROLL] = 0;

			//never strafe during when jumping somewhere
			strafe = qfalse;

			if(bs->cur_ps.groundEntityNum != ENTITYNUM_NONE &&
				(diststart < distend || bs->origin[2] < bs->wpCurrent->origin[2]) )
			{//before jump attempt
				if(DOM_ForcePowerforJump[DOM_ForceJumpNeeded(bs->origin, bs->wpCurrent->origin)] > bs->cur_ps.fd.forcePower)
				{//we don't have enough energy to make our jump.  wait here.
					bs->wpSpecial = qtrue;
					return;
				}
			}

			//velocity analysis
			viewang[PITCH] = 0;
			viewang[ROLL] = 0;
			VectorNormalize(viewang);
			VectorCopy(bs->cur_ps.velocity, velocity);
			velocity[2] = 0;
			VectorNormalize(velocity);

			//make sure we're stopped or moving towards our goal before jumping
			/*if(VectorCompare(vec3_origin, velocity) || DotProduct(velocity, viewang) > .7
				|| bs->cur_ps.groundEntityNum == ENTITYNUM_NONE)
			{//moving towards to our jump target or not moving at all or already on route
				if((diststart < distend || bs->origin[2] < bs->wpCurrent->origin[2]) 
					&& bs->velocity[2] >= -10)
				{//jump
					bs->jumpTime = level.time + 100;
					bs->wpSpecial = qtrue;
					DOM_WPVisibleUpdate(bs);
					trap->EA_Move(bs->client, bs->goalMovedir, 5000);
					return;
				}

			}*/
		}

		//not doing a special wp move so clear that flag.
		bs->wpSpecial = qfalse;

		if (bs->wpCurrent->flags & WPFLAG_WAITFORFUNC)
		{
			if (!CheckForFunc(bs->wpCurrent->origin, bs->client))
			{
				//DOM_WPVisibleUpdate(bs);

				if(!bs->AltRouteCheck && (bs->wpTravelTime - level.time) < 20000)
				{//been waiting for 10 seconds, try looking for alt route if we haven't 
					//already
					//bot_route_t routeTest;
					int newwp = DOM_GetBestWaypoint(bs->origin, bs->client, bs->wpCurrent->index);

					//trap->Print("BOT DEBUG: Bot %s created new route because of wpTravelTime.\n", g_entities[bs->client].client->pers.netname);

					bs->AltRouteCheck = qtrue;
					
					if(newwp == -1)
					{
						//DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, strafe);
						return;
					}

					if(!bs->objectiveType || !bs->tacticEntity || bs->tacticEntity->s.number < MAX_CLIENTS
						|| strcmp(bs->tacticEntity->classname, "freed") == 0 || bs->wpDestSwitchTime < level.time)
					{//don't have objective entity type, don't have tacticEntity, or the tacticEntity you had
						//was killed/freed
#ifdef __WARZONE__
						if (g_gametype.integer == GT_WARZONE)
							DOM_PickWARZONEGoalType(bs);
#endif //__WARZONE__

						bs->tacticEntity = DOM_DetermineObjectiveType(&g_entities[bs->client], g_entities[bs->client].client->sess.sessionTeam, 
							bs->tacticObjective, &bs->objectiveType, bs->objectiveType, NULL, 0);

						{
							//destination WP
							int destwp = DOM_GetBestWaypoint(bs->tacticEntity->r.currentOrigin, bs->client, -1);
	
							if( destwp == -1 )
							{//crap, this map has no wps.  try just autonaving it then
								DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, strafe);
								return;
							}

							bs->wpDestination = gWPArray[destwp];
						}
					}

					bs->wpLast = NULL;
					bs->wpCurrent = gWPArray[newwp];

					//trap->Print("DOM BOT DEBUG: Creating route (4)\n");

					g_entities[bs->client].pathsize = ASTAR_FindPathFast(bs->wpCurrent->index, bs->wpDestination->index, bs->botRoute, qtrue);

					//if (g_entities[bs->client].pathsize <= 0) // Alt A* Pathing...
					//	g_entities[bs->client].pathsize = DOM_FindIdealPathtoWP(bs, bs->wpCurrent->index, bs->wpDestination->index, -2, bs->botRoute);

					bs->wpNext = gWPArray[DOM_GetNextWaypoint(bs)];
					DOM_ResetWPTimers(bs);
				}
				return;
			}
			
		}
		if (bs->wpCurrent->flags & WPFLAG_NOMOVEFUNC)
		{
			if (CheckForFunc(bs->wpCurrent->origin, bs->client))
			{
				DOM_WPVisibleUpdate(bs);

				if(!bs->AltRouteCheck && (bs->wpTravelTime - level.time) < 20000)
				{//been waiting for 10 seconds, try looking for alt route if we haven't 
					//already
					//bot_route_t routeTest;
					int newwp = DOM_GetBestWaypoint(bs->origin, bs->client, bs->wpCurrent->index);

					//trap->Print("BOT DEBUG: Bot %s created new route because of wpTravelTime2.\n", g_entities[bs->client].client->pers.netname);

					bs->AltRouteCheck = qtrue;
					
					if(newwp == -1)
					{//crap, this map has no wps.  try just autonaving it then
						//DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, strafe);
						return;
					}

					if(!bs->objectiveType || !bs->tacticEntity || bs->tacticEntity->s.number < MAX_CLIENTS
						|| strcmp(bs->tacticEntity->classname, "freed") == 0 || bs->wpDestSwitchTime < level.time)
					{//don't have objective entity type, don't have tacticEntity, or the tacticEntity you had
						//was killed/freed
#ifdef __WARZONE__
						if (g_gametype.integer == GT_WARZONE)
							DOM_PickWARZONEGoalType(bs);
#endif //__WARZONE__

						bs->tacticEntity = DOM_DetermineObjectiveType(&g_entities[bs->client], g_entities[bs->client].client->sess.sessionTeam, 
							bs->tacticObjective, &bs->objectiveType, bs->objectiveType, NULL, 0);

						{
							//destination WP
							int destwp = DOM_GetBestWaypoint(bs->tacticEntity->r.currentOrigin, bs->client, -1);
	
							if( destwp == -1 )
							{//crap, this map has no wps.  try just autonaving it then
								DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, strafe);
								return;
							}

							bs->wpDestination = gWPArray[destwp];
						}
					}

					bs->wpLast = NULL;
					bs->wpCurrent = gWPArray[newwp];

					//trap->Print("DOM BOT DEBUG: Creating route (3)\n");

					g_entities[bs->client].pathsize = ASTAR_FindPathFast(bs->wpCurrent->index, bs->wpDestination->index, bs->botRoute, qtrue);

					//if (g_entities[bs->client].pathsize <= 0) // Alt A* Pathing...
					//	g_entities[bs->client].pathsize = DOM_FindIdealPathtoWP(bs, bs->wpCurrent->index, bs->wpDestination->index, -2, bs->botRoute);
					bs->wpNext = gWPArray[DOM_GetNextWaypoint(bs)];
					DOM_ResetWPTimers(bs);
				}
				return;
			}
		}

		if (bs->wpCurrent->flags & WPFLAG_DUCK)
		{ //duck while travelling to this point
			bs->duckTime = level.time + 100;
		}

		//visual check
		if(!(bs->wpCurrent->flags & WPFLAG_NOVIS))
		{//do visual check
			DOM_WPVisibleUpdate(bs);
		}
		else
		{
			movetrace = qfalse;
			strafe = qfalse;
		}
	}
	else
	{//jump to dest if we need to.
		/*
		if(DOM_CalculateJump(bs, bs->origin, dest))
		{
			bs->jumpTime = level.time + 100;
		}
		*/
	}

	//set strafing.
	//if(strafe)
	{
		/*
		if(bs->meleeStrafeTime < level.time)
		{//select a new strafing direction, since we're actively navigating, switch strafe
			//directions more often
			//0 = no strafe
			//1 = strafe right
			//2 = strafe left
			bs->meleeStrafeDir = Q_irand(0,2);
			bs->meleeStrafeTime = level.time + Q_irand(500, 1000);
		}
		*/

		int STRAFE_DIRECTION = DOM_Avoidance(bs, &g_entities[bs->client]);

		if (STRAFE_DIRECTION == AVOIDANCE_STRAFE_RIGHT)
		{
			bs->meleeStrafeDir = AVOIDANCE_STRAFE_RIGHT;
		}
		else if (STRAFE_DIRECTION == AVOIDANCE_STRAFE_LEFT)
		{
			bs->meleeStrafeDir = AVOIDANCE_STRAFE_LEFT;
		}
		/*else if (STRAFE_DIRECTION == AVOIDANCE_STRAFE_JUMP)
		{
			trap->EA_MoveUp(bs->client);
			bs->jumpTime = level.time + 200;
		}*/

		//adjust the moveDir to do strafing
		DOM_AdjustforStrafe(bs, bs->goalMovedir);
	}

	if(movetrace)
	{
		DOM_TraceMove(bs, bs->goalMovedir, bs->DestIgnore);
	}

	VectorCopy(dest, bs->goalPosition);
	//DOM_UQ1_UcmdMoveForDir( bs, &bs->lastucmd, bs->goalMovedir );
	trap->EA_Move(bs->client, bs->goalMovedir, 5000);

	//if (VectorDistance(bs->origin, dest) > 1024) assert(0);

	//trap->Print("Bot %s move to wp %i (dist %f).\n", g_entities[bs->client].client->pers.netname, bs->wpCurrent->index, VectorDistance(bs->origin, dest));
}


void DOM_WPTouch(bot_state_t *bs)
{//Touched the target WP
	//trap->Print("%s - WPTouch (wp is %i, distance is %f)!\n", g_entities[bs->client].client->pers.netname, bs->wpCurrent->index, Distance(bs->origin, bs->wpCurrent->origin));

	if(!bs->objectiveType || !bs->tacticEntity || !bs->wpDestination || bs->tacticEntity->s.number < MAX_CLIENTS
		|| strcmp(bs->tacticEntity->classname, "freed") == 0 || bs->wpDestSwitchTime < level.time)
	{//don't have objective entity type, don't have tacticEntity, or the tacticEntity you had
		//was killed/freed
#ifdef __WARZONE__
		if (g_gametype.integer == GT_WARZONE)
			DOM_PickWARZONEGoalType(bs);
#endif //__WARZONE__

		bs->tacticEntity = DOM_DetermineObjectiveType(&g_entities[bs->client], g_entities[bs->client].client->sess.sessionTeam, 
			bs->tacticObjective, &bs->objectiveType, bs->objectiveType, NULL, 0);

		{
			//destination WP
			int destwp = DOM_GetBestWaypoint(bs->tacticEntity->r.currentOrigin, bs->client, -1);
	
			if( destwp == -1 )
			{//crap, this map has no wps.  try just autonaving it then
				DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, qfalse);
				return;
			}

			bs->wpDestination = gWPArray[destwp];
		}
	}

	if (!bs->wpCurrent || !bs->wpNext || bs->wpCurrent->index <= 0)
	{// We need a new path...
		int wp = DOM_GetBestWaypoint(bs->origin, bs->client, -1);
		
		if (wp == -1)
		{//no waypoints
			DOM_BotMove(bs, bs->DestPosition, qfalse, qfalse);
			return;
		}

		bs->wpLast = NULL;
		bs->wpCurrent = gWPArray[wp];
		
		//trap->Print("BOT DEBUG: Bot %s created new route because of bad wpCurrent/wpNext.\n", g_entities[bs->client].client->pers.netname);

		g_entities[bs->client].pathsize = ASTAR_FindPathFast(bs->wpCurrent->index, bs->wpDestination->index, bs->botRoute, qtrue);

		//if (g_entities[bs->client].pathsize <= 0) // Alt A* Pathing...
		//	g_entities[bs->client].pathsize = DOM_FindIdealPathtoWP(bs, bs->wpCurrent->index, bs->wpDestination->index, -2, bs->botRoute);
		
		if (g_entities[bs->client].pathsize > 0)
		{
			bs->wpNext = gWPArray[DOM_GetNextWaypoint(bs)];
			//bs->wpCurrent = bs->wpDestination;
			VectorCopy(bs->origin, bs->wpCurrentLoc);
			DOM_ResetWPTimers(bs);
		}
		return;
	}

	bs->wpLast = bs->wpCurrent;
	bs->wpCurrent = bs->wpNext;
	bs->wpNext = gWPArray[DOM_GetNextWaypoint(bs)];
	//trap->Print("%s - WPTouch (new wp is %i, distance is %f)!\n", g_entities[bs->client].client->pers.netname, bs->wpCurrent->index, Distance(bs->origin, bs->wpCurrent->origin));
	VectorCopy(bs->origin, bs->wpCurrentLoc);
	DOM_ResetWPTimers(bs);
}


void DOM_ResetWPTimers(bot_state_t *bs)
{
	if( (bs->wpCurrent->flags & WPFLAG_WAITFORFUNC) 
		|| (bs->wpCurrent->flags & WPFLAG_NOMOVEFUNC)
		|| (bs->wpCurrent->flags & WPFLAG_DESTROY_FUNCBREAK)
		|| (bs->wpCurrent->flags & WPFLAG_FORCEPUSH)
		|| (bs->wpCurrent->flags & WPFLAG_FORCEPULL) )
	{//it's an elevator or something waypoint time needs to be longer.
		bs->wpSeenTime = level.time + 6000;
		bs->wpTravelTime = level.time + 30000;	
	}
	else if(bs->wpCurrent->flags & WPFLAG_NOVIS)
	{
		//10 sec
		bs->wpSeenTime = level.time + 10000;
		bs->wpTravelTime = level.time + 10000;
	}
	else
	{
		//3 sec visual time
		bs->wpSeenTime = level.time + 6000;

		//10 sec move time
		bs->wpTravelTime = level.time + 10000;
	}

	bs->AltRouteCheck = qfalse;
	bs->DontSpamPushPull = 0;
}

int DOM_GetNearestVisibleWP_Goal(vec3_t org, int ignore, int badwp)
{
	int i;
	float bestdist;
	float flLen;
	int bestindex;
	vec3_t a, mins, maxs;

	i = 0;
	if (RMG.integer)
	{
		bestdist = 300;
	}
	else
	{
		bestdist = 800;//99999;
				   //don't trace over 800 units away to avoid GIANT HORRIBLE SPEED HITS ^_^
	}
	bestindex = -1;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -1;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 1;

	while (i < gWPNum)
	{
		if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
		{
			VectorSubtract(org, gWPArray[i]->origin, a);
			flLen = VectorLength(a);

			if(gWPArray[i]->flags & WPFLAG_WAITFORFUNC 
				|| (gWPArray[i]->flags & WPFLAG_NOMOVEFUNC)
				|| (gWPArray[i]->flags & WPFLAG_DESTROY_FUNCBREAK)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPUSH)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPULL) )
			{//boost the distance for these waypoints so that we will try to avoid using them
				//if at all possible
				flLen += 500;
			}

			if (flLen < bestdist /*&& (g_RMG.integer || BotPVSCheck(org, gWPArray[i]->origin))*/ && OrgVisibleBox(org, mins, maxs, gWPArray[i]->origin, ignore))
			{
				bestdist = flLen;
				bestindex = i;
			}
		}

		//i++;
		i+=Q_irand(1,3);
	}

	return bestindex;
}

//get the index to the nearest visible waypoint in the global trail
//just like GetNearestVisibleWP except with a bad waypoint input
int DOM_GetNearestVisibleWP(vec3_t org, int ignore, int badwp)
{
	int i;
	float bestdist;
	float flLen;
	int bestindex;
	vec3_t a, mins, maxs;

	i = 0;
	if (RMG.integer)
	{
		bestdist = 300;
	}
	else
	{
		bestdist = 800;//99999;
				   //don't trace over 800 units away to avoid GIANT HORRIBLE SPEED HITS ^_^
	}
	bestindex = -1;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -1;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 1;

	while (i < gWPNum)
	{
		if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
		{
			VectorSubtract(org, gWPArray[i]->origin, a);
			flLen = VectorLength(a);

			if(gWPArray[i]->flags & WPFLAG_WAITFORFUNC 
				|| (gWPArray[i]->flags & WPFLAG_NOMOVEFUNC)
				|| (gWPArray[i]->flags & WPFLAG_DESTROY_FUNCBREAK)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPUSH)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPULL) )
			{//boost the distance for these waypoints so that we will try to avoid using them
				//if at all possible
				flLen += 500;
			}

			if (flLen < bestdist /*&& (g_RMG.integer || BotPVSCheck(org, gWPArray[i]->origin))*/ && OrgVisibleBox(org, mins, maxs, gWPArray[i]->origin, ignore))
			{
				bestdist = flLen;
				bestindex = i;
			}
		}

		i++;
	}

	return bestindex;
}

int DOM_GetNearestVisibleWP_NOBOX(vec3_t org, int ignore, int badwp)
{
	int i;
	float bestdist;
	float flLen;
	int bestindex;
	vec3_t a, mins, maxs;

	i = 0;
	if (RMG.integer)
	{
		bestdist = 300;
	}
	else
	{
		bestdist = 800;//99999;
				   //don't trace over 800 units away to avoid GIANT HORRIBLE SPEED HITS ^_^
	}
	bestindex = -1;

	mins[0] = -15;
	mins[1] = -15;
	mins[2] = -1;
	maxs[0] = 15;
	maxs[1] = 15;
	maxs[2] = 1;

	while (i < gWPNum)
	{
		if (gWPArray[i] && gWPArray[i]->inuse && i != badwp)
		{
			vec3_t wpOrg, fromOrg;

			VectorSubtract(org, gWPArray[i]->origin, a);
			flLen = VectorLength(a);

			if(gWPArray[i]->flags & WPFLAG_WAITFORFUNC 
				|| (gWPArray[i]->flags & WPFLAG_NOMOVEFUNC)
				|| (gWPArray[i]->flags & WPFLAG_DESTROY_FUNCBREAK)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPUSH)
				|| (gWPArray[i]->flags & WPFLAG_FORCEPULL) )
			{//boost the distance for these waypoints so that we will try to avoid using them
				//if at all possible
				flLen += 500;
			}

			VectorCopy(gWPArray[i]->origin, wpOrg);
			wpOrg[2]+=32;
			VectorCopy(org, fromOrg);
			fromOrg[2]+=18;

			if (flLen < bestdist /*&& (g_RMG.integer || BotPVSCheck(org, gWPArray[i]->origin))*/ && OrgVisible(fromOrg, wpOrg, ignore))
			{
				bestdist = flLen;
				bestindex = i;
			}
		}

		i++;
	}

	return bestindex;
}

int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp)
{// UQ1: Find the best waypoint for an origin...
	int wp = DOM_GetNearestVisibleWP(org, ignore, badwp);

	if (wp == -1)
	{// UQ1: Can't find a visible wp with a box trace.. fall back to a non-box trace and hope avoidance works...
		wp = DOM_GetNearestVisibleWP_NOBOX(org, ignore, badwp);
		
		if ( wp == -1 )
		{// UQ1: No visible waypoints at all...
			wp = DOM_GetNearestWP(org, badwp);

			if ( wp == -1 )
			{// UQ1: No waypoints
				return -1;
			}
		}
	}

	return wp;
}

qboolean DOM_WaitForRouteCalculation(bot_state_t *bs)
{
	if (bs->next_path_calculate_time > level.time && !bs->wpCurrent)
	{// Stay still and wait!
		bs->ready_to_calculate_path = qtrue;

		if (bs->tacticEntity && bs->tacticEntity->inuse && Distance(bs->tacticEntity->r.currentOrigin, bs->origin) < 256)
			DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, qfalse);
		else if (bs->currentEnemy)
			DOM_BotMove(bs, bs->currentEnemy->r.currentOrigin, qfalse, qfalse);
		//else if (bs->goalPosition)
		//	DOM_BotMove(bs, bs->goalPosition, qfalse, qfalse);

		return qtrue;
	}

	// Ok, recalculate now!
	return qfalse;
}

extern void NPC_ConversationAnimation(gentity_t *NPC);

void DOM_AI_CheckSpeak(gentity_t *bot, qboolean moving)
{
	char			filename[256];
	char			botSoundDir[256];
	fileHandle_t	f;

	if (bot->npc_dumb_route_time > level.time) return; // not yet...

	bot->npc_dumb_route_time = level.time + 5000 + irand(0, 15000);

	strcpy(botSoundDir, bot->client->botSoundDir);
	
	if (botstates[bot->s.number]->currentEnemy && botstates[bot->s.number]->enemySeenTime >= level.time)
	{// Have enemy...
		int randChoice = irand(0,50);

		switch (randChoice)
		{
		case 0:
			strcpy(filename, va("sound/chars/%s/misc/anger1.mp3", botSoundDir));
			break;
		case 1:
			strcpy(filename, va("sound/chars/%s/misc/anger2.mp3", botSoundDir));
			break;
		case 2:
			strcpy(filename, va("sound/chars/%s/misc/anger3.mp3", botSoundDir));
			break;
		case 3:
			strcpy(filename, va("sound/chars/%s/misc/chase1.mp3", botSoundDir));
			break;
		case 4:
			strcpy(filename, va("sound/chars/%s/misc/chase2.mp3", botSoundDir));
			break;
		case 5:
			strcpy(filename, va("sound/chars/%s/misc/chase3.mp3", botSoundDir));
			break;
		case 6:
			strcpy(filename, va("sound/chars/%s/misc/combat1.mp3", botSoundDir));
			break;
		case 7:
			strcpy(filename, va("sound/chars/%s/misc/combat2.mp3", botSoundDir));
			break;
		case 8:
			strcpy(filename, va("sound/chars/%s/misc/combat3.mp3", botSoundDir));
			break;
		case 9:
			strcpy(filename, va("sound/chars/%s/misc/confuse1.mp3", botSoundDir));
			break;
		case 10:
			strcpy(filename, va("sound/chars/%s/misc/confuse2.mp3", botSoundDir));
			break;
		case 11:
			strcpy(filename, va("sound/chars/%s/misc/confuse3.mp3", botSoundDir));
			break;
		case 12:
			strcpy(filename, va("sound/chars/%s/misc/cover1.mp3", botSoundDir));
			break;
		case 13:
			strcpy(filename, va("sound/chars/%s/misc/cover2.mp3", botSoundDir));
			break;
		case 14:
			strcpy(filename, va("sound/chars/%s/misc/cover3.mp3", botSoundDir));
			break;
		case 15:
			strcpy(filename, va("sound/chars/%s/misc/cover4.mp3", botSoundDir));
			break;
		case 16:
			strcpy(filename, va("sound/chars/%s/misc/cover5.mp3", botSoundDir));
			break;
		case 17:
			strcpy(filename, va("sound/chars/%s/misc/deflect1.mp3", botSoundDir));
			break;
		case 18:
			strcpy(filename, va("sound/chars/%s/misc/deflect2.mp3", botSoundDir));
			break;
		case 19:
			strcpy(filename, va("sound/chars/%s/misc/deflect3.mp3", botSoundDir));
			break;
		case 20:
			strcpy(filename, va("sound/chars/%s/misc/detected1.mp3", botSoundDir));
			break;
		case 21:
			strcpy(filename, va("sound/chars/%s/misc/detected2.mp3", botSoundDir));
			break;
		case 22:
			strcpy(filename, va("sound/chars/%s/misc/detected3.mp3", botSoundDir));
			break;
		case 23:
			strcpy(filename, va("sound/chars/%s/misc/detected4.mp3", botSoundDir));
			break;
		case 24:
			strcpy(filename, va("sound/chars/%s/misc/detected5.mp3", botSoundDir));
			break;
		case 25:
			strcpy(filename, va("sound/chars/%s/misc/escaping1.mp3", botSoundDir));
			break;
		case 26:
			strcpy(filename, va("sound/chars/%s/misc/escaping2.mp3", botSoundDir));
			break;
		case 27:
			strcpy(filename, va("sound/chars/%s/misc/escaping3.mp3", botSoundDir));
			break;
		case 28:
			strcpy(filename, va("sound/chars/%s/misc/giveup1.mp3", botSoundDir));
			break;
		case 29:
			strcpy(filename, va("sound/chars/%s/misc/giveup2.mp3", botSoundDir));
			break;
		case 30:
			strcpy(filename, va("sound/chars/%s/misc/giveup3.mp3", botSoundDir));
			break;
		case 31:
			strcpy(filename, va("sound/chars/%s/misc/giveup4.mp3", botSoundDir));
			break;
		case 32:
			strcpy(filename, va("sound/chars/%s/misc/gloat1.mp3", botSoundDir));
			break;
		case 33:
			strcpy(filename, va("sound/chars/%s/misc/gloat2.mp3", botSoundDir));
			break;
		case 34:
			strcpy(filename, va("sound/chars/%s/misc/gloat3.mp3", botSoundDir));
			break;
		case 35:
			strcpy(filename, va("sound/chars/%s/misc/jchase1.mp3", botSoundDir));
			break;
		case 36:
			strcpy(filename, va("sound/chars/%s/misc/jchase2.mp3", botSoundDir));
			break;
		case 37:
			strcpy(filename, va("sound/chars/%s/misc/jchase3.mp3", botSoundDir));
			break;
		case 38:
			strcpy(filename, va("sound/chars/%s/misc/jdetected1.mp3", botSoundDir));
			break;
		case 39:
			strcpy(filename, va("sound/chars/%s/misc/jdetected2.mp3", botSoundDir));
			break;
		case 40:
			strcpy(filename, va("sound/chars/%s/misc/jdetected3.mp3", botSoundDir));
			break;
		case 41:
			strcpy(filename, va("sound/chars/%s/misc/jlost1.mp3", botSoundDir));
			break;
		case 42:
			strcpy(filename, va("sound/chars/%s/misc/jlost2.mp3", botSoundDir));
			break;
		case 43:
			strcpy(filename, va("sound/chars/%s/misc/jlost3.mp3", botSoundDir));
			break;
		case 44:
			strcpy(filename, va("sound/chars/%s/misc/outflank1.mp3", botSoundDir));
			break;
		case 45:
			strcpy(filename, va("sound/chars/%s/misc/outflank2.mp3", botSoundDir));
			break;
		case 46:
			strcpy(filename, va("sound/chars/%s/misc/outflank3.mp3", botSoundDir));
			break;
		case 47:
			strcpy(filename, va("sound/chars/%s/misc/taunt.mp3", botSoundDir));
			break;
		case 48:
			strcpy(filename, va("sound/chars/%s/misc/taunt1.mp3", botSoundDir));
			break;
		case 49:
			strcpy(filename, va("sound/chars/%s/misc/taunt2.mp3", botSoundDir));
			break;
		default:
			strcpy(filename, va("sound/chars/%s/misc/taunt3.mp3", botSoundDir));
			break;
		}
	}
	else
	{// No enemy...
		int randChoice = irand(0,10);

		switch (randChoice)
		{
		case 0:
			strcpy(filename, va("sound/chars/%s/misc/sight1.mp3", botSoundDir));
			break;
		case 1:
			strcpy(filename, va("sound/chars/%s/misc/sight2.mp3", botSoundDir));
			break;
		case 2:
			strcpy(filename, va("sound/chars/%s/misc/sight3.mp3", botSoundDir));
			break;
		case 3:
			strcpy(filename, va("sound/chars/%s/misc/look1.mp3", botSoundDir));
			break;
		case 4:
			strcpy(filename, va("sound/chars/%s/misc/look2.mp3", botSoundDir));
			break;
		case 5:
			strcpy(filename, va("sound/chars/%s/misc/look3.mp3", botSoundDir));
			break;
		case 6:
			strcpy(filename, va("sound/chars/%s/misc/suspicious1.mp3", botSoundDir));
			break;
		case 7:
			strcpy(filename, va("sound/chars/%s/misc/suspicious2.mp3", botSoundDir));
			break;
		case 8:
			strcpy(filename, va("sound/chars/%s/misc/suspicious3.mp3", botSoundDir));
			break;
		case 9:
			strcpy(filename, va("sound/chars/%s/misc/suspicious4.mp3", botSoundDir));
			break;
		default:
			strcpy(filename, va("sound/chars/%s/misc/suspicious5.mp3", botSoundDir));
			break;
		}
	}

	trap->FS_Open( filename, &f, FS_READ );

	if ( !f )
	{// this file doesnt exist for this char...
		//trap->Print("File [%s] does not exist.\n", filename);
		trap->FS_Close( f );
		return;
	}

	trap->FS_Close( f );

	trap->Print("Bot sound file [%s] played.\n", filename);

	// Play a taunt/etc...
	G_SoundOnEnt( bot, CHAN_VOICE_ATTEN, filename );

	if (!moving)
		NPC_ConversationAnimation(bot);
}

//Behavior to move to the given DestPosition
//strafe = do some strafing while moving to this location
void DOM_BotMoveto(bot_state_t *bs, qboolean strafe)
{
	qboolean recalcroute = qfalse;
	qboolean findwp = qfalse;
	int badwp = -2;
	//int destwp = -1;
	float distthen, distnow;

	if(!bs->wpCurrent || bs->wpCurrent <= 0)
	{////ok, we just did something other than wp navigation.  find the closest wp.
		findwp = qtrue;
		//trap->Print("(!bs->wpCurrent || bs->wpCurrent <= 0)\n");
	}
	else if( bs->wpSeenTime < level.time )
	{//lost track of the waypoint
		findwp = qtrue;
		badwp = bs->wpCurrent->index;
		bs->wpDestination = NULL;
		recalcroute = qtrue;
		//trap->Print("( bs->wpSeenTime < level.time )\n");

		// UQ1: Mark wp the link as bad!
		//DOM_IncreaseWaypointLinkCost(bs);
	}
	else if( bs->wpTravelTime < level.time )
	{//spent too much time traveling to this point or lost sight for too long.
		//recheck everything
		findwp = qtrue;
		badwp = bs->wpCurrent->index;
		bs->wpDestination = NULL;
		recalcroute = qtrue;
		trap->Print("( bs->wpTravelTime < level.time )\n");

		// UQ1: Mark wp the link as bad!
		//DOM_IncreaseWaypointLinkCost(bs);
	}
	//Check to make sure we didn't get knocked way off course.
	else if( !bs->wpSpecial )
	{
		distthen = Distance(bs->wpCurrentLoc, bs->wpCurrent->origin);
		distnow = Distance(bs->wpCurrent->origin, bs->origin);
		if( distnow > 50 && distthen < distnow )
		{//we're pretty far off the path, check to make sure we didn't get knocked way off course.
			findwp = qtrue;
			//trap->Print("( distnow > 50 && distthen < distnow )\n");
		}
	}

	/*
	if( VectorCompare(bs->DestPosition, bs->lastDestPosition) < -64 || !bs->wpDestination)
	{//The goal position has moved from last frame.  make sure it's not closer to a different
		//destination WP
		destwp = DOM_GetBestWaypoint(bs->DestPosition, bs->client, badwp);
	
		if( destwp == -1 )
		{//crap, this map has no wps.  try just autonaving it then
			DOM_BotMove(bs, bs->DestPosition, qfalse, strafe);
			return;
		}
		
		if (!bs->wpDestination || bs->wpDestination->index != destwp)
		{
			bs->wpDestination = gWPArray[destwp];
			recalcroute = qtrue;

			//if (!bs->wpDestination)
			//	trap->Print("(!bs->wpDestination)\n");
			//else if (bs->wpDestination->index != destwp)
			//	trap->Print("(bs->wpDestination->index != destwp)\n");
		}
	}
	*/

	if(findwp || !bs->wpCurrent || bs->wpCurrent->index <= 0)
	{
		/*
		int wp = DOM_GetBestWaypoint(bs->origin, bs->client, badwp);

		if (wp == -1)
		{//no waypoints
			//trap->Print("%s failed to find a waypoint.\n", g_entities[bs->client].client->pers.netname);
			DOM_BotMove(bs, bs->DestPosition, qfalse, strafe);
			return;
		}
		
		//got a waypoint, lock on and move towards it
		bs->wpLast = NULL;
		bs->wpCurrent = gWPArray[wp];
		DOM_ResetWPTimers(bs);
		VectorCopy(bs->origin, bs->wpCurrentLoc);
		*/
		recalcroute = qtrue;
	}

	if(recalcroute && !bs->ready_to_calculate_path)
	{// UQ1: Delay between creating new paths!
		if (bs->next_path_calculate_time < level.time)
		{
			bs->next_path_calculate_time = level.time + 2000;
			bs->ready_to_calculate_path = qtrue;
			bs->beStill = level.time + 2000; // Stand still a moment and wait...
		}
	}

	if (DOM_WaitForRouteCalculation(bs))
	{
		DOM_AI_CheckSpeak(&g_entities[bs->client], qfalse);
		return; // Waiting!
	}

	DOM_AI_CheckSpeak(&g_entities[bs->client], qtrue);

	if(recalcroute && bs->ready_to_calculate_path)
	{
		bs->next_path_calculate_time = level.time + 2000;

		/*
		if(!bs->objectiveType || !bs->tacticEntity || bs->tacticEntity->s.number < MAX_CLIENTS
			|| strcmp(bs->tacticEntity->classname, "freed") == 0 || bs->wpDestSwitchTime < level.time)
		{//don't have objective entity type, don't have tacticEntity, or the tacticEntity you had
			//was killed/freed
#ifdef __WARZONE__
			if (g_gametype.integer == GT_WARZONE)
				DOM_PickWARZONEGoalType(bs);
#endif //__WARZONE__
			bs->tacticEntity = DOM_DetermineObjectiveType(&g_entities[bs->client], g_entities[bs->client].client->sess.sessionTeam, 
				bs->tacticObjective, &bs->objectiveType, bs->objectiveType, NULL, 0);

			if (bs->tacticEntity)
			{
				//destination WP
				destwp = DOM_GetBestWaypoint(bs->tacticEntity->r.currentOrigin, bs->client, -1);
	
				if( destwp == -1 )
				{//crap, this map has no wps.  try just autonaving it then
					//trap->Print("BOT DEBUG: Bot %s failed to find a destwp.\n", g_entities[bs->client].client->pers.netname);

					DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, strafe);
					return;
				}

				bs->wpDestination = gWPArray[destwp];
			}
			else
			{
				//trap->Print("BOT DEBUG: Bot %s failed to find an objective.\n", g_entities[bs->client].client->pers.netname);
			}
		}
		*/

		// UQ1: Since our maps have few or no entities...
		bs->wpDestination = gWPArray[irand(1, gWPNum-1)];

		//if(!bs->wpCurrent || bs->wpCurrent->index <= 0)
		{
			int wp = DOM_GetBestWaypoint(bs->origin, bs->client, badwp);

			if (wp == -1)
			{//no waypoints
#ifdef _DEBUG
				trap->Print("BOT DEBUG: %s failed to find a waypoint.\n", g_entities[bs->client].client->pers.netname);
#endif
				DOM_BotMove(bs, bs->DestPosition, qfalse, strafe);
				return;
			}

			//got a waypoint, lock on and move towards it
			bs->wpLast = NULL;
			bs->wpCurrent = gWPArray[wp];
			DOM_ResetWPTimers(bs);
			VectorCopy(bs->origin, bs->wpCurrentLoc);
			recalcroute = qtrue;
		}

		//trap->Print("BOT DEBUG: Bot %s created new route because of readytocalculate.\n", g_entities[bs->client].client->pers.netname);
		g_entities[bs->client].pathsize = ASTAR_FindPathFast(bs->wpCurrent->index, bs->wpDestination->index, bs->botRoute, qtrue);

		if (g_entities[bs->client].pathsize > 0)
		{
#ifdef _DEBUG
			trap->Print("BOT DEBUG: Bot %s found a %i waypoint path from wp %i to wp %i.\n", g_entities[bs->client].client->pers.netname, g_entities[bs->client].pathsize, bs->wpCurrent->index, bs->wpDestination->index);
#endif
			bs->wpNext = gWPArray[DOM_GetNextWaypoint(bs)];
			DOM_ResetWPTimers(bs);
		}
		else
		{
#ifdef _DEBUG
			trap->Print("BOT DEBUG: Bot %s failed to find a path from wp %i to wp %i.\n", g_entities[bs->client].client->pers.netname, bs->wpCurrent->index, bs->wpDestination->index);
#endif
		}

		bs->ready_to_calculate_path = qfalse;
	}

	if (!bs->wpDestination)
	{
		DOM_WPTouch(bs);
	}

	//travelling to a waypoint
 	if(bs->wpDestination && bs->wpCurrent
		&& bs->wpCurrent->index != bs->wpDestination->index 
		&& VectorDistanceNoHeight(bs->origin, bs->wpCurrent->origin) <= 24//16//BOT_WPTOUCH_DISTANCE
		&& !bs->wpSpecial)
	{
		//trap->Print("WP (%i) Distance %f. HIT!\n", bs->wpCurrent->index, VectorDistanceNoHeight(bs->origin, bs->wpCurrent->origin));
		DOM_WPTouch(bs);
	}
	/*else if(bs->wpDestination && bs->wpCurrent->index != bs->wpDestination->index 
		&& VectorDistanceNoHeight(bs->origin, bs->wpCurrent->origin) >= 16//BOT_WPTOUCH_DISTANCE
		&& !bs->wpSpecial)
	{
		trap->Print("WP (%i) Distance %f.\n", bs->wpCurrent->index, VectorDistanceNoHeight(bs->origin, bs->wpCurrent->origin));
	}*/

	VectorCopy(bs->eye, bs->origin);
	bs->eye[2]+=48;

	//if you're closer to your bs->DestPosition than you are to your next waypoint, just 
	//move to your bs->DestPosition.  This is to prevent the bots from backstepping when 
	//very close to their target
	if (!bs->wpCurrent) return;

	if(!bs->wpSpecial && bs->wpDestination && bs->wpCurrent
		&& (bs->wpCurrent->index == bs->wpDestination->index 
		|| (Distance(bs->origin, bs->wpCurrent->origin) > Distance(bs->origin, bs->wpDestination->origin))) )
	{//move to DestPosition
		DOM_BotMove(bs, bs->wpDestination->origin, qfalse, strafe);
	}
	else
	{//move to next waypoint
		DOM_BotMove(bs, bs->wpCurrent->origin, qtrue, strafe);
	}

	/*
	if(bs->wpCurrent->index != bs->wpDestination->index)
	{
		DOM_BotMove(bs, bs->wpCurrent->origin, qtrue);
	}
	else
	{
		DOM_BotMove(bs, bs->DestPosition, qfalse);
	}
	*/
}


//We want to get some ammo
//returns the weapon type we want
//-1 = nah, not right now.
int DOM_WanttoGetAmmo(bot_state_t *bs)
{
	//RAFIXME - fill out function
	return -1;
}


//find the closest weapon spawnpoint for given weapon
qboolean DOM_FindNearestWeapon(int weapon, vec3_t weaploc)
{
	//RAFIXME - fill out function
	return qfalse;
}


//Adjusts the moveDir to account for strafing
void DOM_AdjustforStrafe(bot_state_t *bs, vec3_t moveDir)
{
	vec3_t right, angles;

	if(!bs->meleeStrafeDir || bs->meleeStrafeTime < level.time )
	{//no strafe
		return;
	}

	vectoangles(moveDir, angles);
	AngleVectors(angles /*g_entities[bs->client].client->ps.viewangles*/, NULL, right, NULL);

	//flaten up/down
	right[2] = 0;

	if(bs->meleeStrafeDir == 1)
	{//strafing right
		VectorScale(right, 64, right);
	}
	else if(bs->meleeStrafeDir == 2)
	{//strafing left
		VectorScale(right, -64, right);
	}

	//We assume that moveDir has been normalized before this function.
	VectorAdd(moveDir, right, moveDir);
	VectorNormalize(moveDir);
}


//swiped from the Unique's AotCTC code.
#define AI_LUNGE_DISTANCE 128

qboolean DOM_AI_CanLunge(bot_state_t *bs)
{
	gentity_t *ent;
	trace_t tr;
	vec3_t flatAng;
	vec3_t fwd, traceTo;
	vec3_t trmins = {-15, -15, -8};
	vec3_t trmaxs = {15, 15, 8};

	VectorCopy(bs->cur_ps.viewangles, flatAng);
	flatAng[PITCH] = 0;

	AngleVectors(flatAng, fwd, 0, 0);

	traceTo[0] = bs->origin[0] + fwd[0]*AI_LUNGE_DISTANCE;
	traceTo[1] = bs->origin[1] + fwd[1]*AI_LUNGE_DISTANCE;
	traceTo[2] = bs->origin[2] + fwd[2]*AI_LUNGE_DISTANCE;

	trap->Trace(&tr, bs->origin, trmins, trmaxs, traceTo, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	ent = &g_entities[tr.entityNum];

	if ( tr.fraction != 1.0 && !OnSameTeam(&g_entities[bs->client], ent) && PassStandardEnemyChecks(bs, ent) && PassLovedOneCheck(bs, ent)&& (ent->s.eType == ET_NPC || ent->s.eType == ET_PLAYER) )
	{
		return qtrue;
	}

	return qfalse;
}


//Find the favorite weapon for this range.
int DOM_FindWeaponforRange(bot_state_t *bs, float range)
{
	int bestweap = -1;
	int bestfav = -1;
	int weapdist;
	int i;

	//try to find the fav weapon for this attack range
	for(i=0;i < WP_NUM_WEAPONS; i++)
	{
		if(i == WP_SABER)
		{//hack to prevent the player from switching away from the saber when close to 
			//target
			weapdist = 300;
		}
		else
		{
			weapdist = 2048.0f;
			//weapdist = DOM_MaximumAttackDistance[i];
			//weapdist = DOM_IdealAttackDistance[i] * 1.1;
		}

		if( range < weapdist /*&& bs->cur_ps.ammo[weaponData[i].ammoIndex] >= weaponData[i].energyPerShot*/ &&
			bs->botWeaponWeights[i] > bestfav &&
			(bs->cur_ps.stats[STAT_WEAPONS] & (1 << i)))
		{
			bestweap = i;
			bestfav = bs->botWeaponWeights[i];
		}
	}

	return bestweap;
}

//
//
// BEGIN: Experimental Jedi Jumping....
//
//

#define	APEX_HEIGHT		200.0f
#define	PARA_WIDTH		(sqrt(APEX_HEIGHT)+sqrt(APEX_HEIGHT))
#define	JUMP_SPEED		200.0f

static qboolean DOM_Jedi_Jump( gentity_t *NPC, vec3_t dest, int goalEntNum )
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
						if ( trace.entityNum == goalEntNum )
						{//hit the enemy, that's perfect!
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

extern qboolean PM_InKnockDown( playerState_t *ps );

static qboolean DOM_Jedi_TryJump( gentity_t *NPC, gentity_t *goal )
{//FIXME: never does a simple short, regular jump...
	usercmd_t	*ucmd = &NPC->client->pers.cmd;

	//FIXME: I need to be on ground too!
	//if ( NPC->playerState->weapon != WP_SABER )
	//{// Not a jedi...
	//	return qfalse;
	//}

	if ( TIMER_Done( NPC, "jumpChaseDebounce" ) )
	{
		if ( (!goal->client || goal->client->ps.groundEntityNum != ENTITYNUM_NONE) )
		{
			if ( !PM_InKnockDown( &NPC->client->ps ) && !BG_InRoll( &NPC->client->ps, NPC->client->ps.legsAnim ) )
			{//enemy is on terra firma
				vec3_t goal_diff;
				float goal_z_diff;
				float goal_xy_dist;
				VectorSubtract( goal->r.currentOrigin, NPC->r.currentOrigin, goal_diff );
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
						
						ucmd->upmove = 127;
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
							VectorCopy( goal->r.currentOrigin, dest );
							if ( goal == NPC->enemy )
							{
								int	sideTry = 0;
								while( sideTry < 10 )
								{//FIXME: make it so it doesn't try the same spot again?
									trace_t	trace;
									vec3_t	bottom;

									if ( Q_irand( 0, 1 ) )
									{
										dest[0] += NPC->enemy->r.maxs[0]*1.25;
									}
									else
									{
										dest[0] += NPC->enemy->r.mins[0]*1.25;
									}
									if ( Q_irand( 0, 1 ) )
									{
										dest[1] += NPC->enemy->r.maxs[1]*1.25;
									}
									else
									{
										dest[1] += NPC->enemy->r.mins[1]*1.25;
									}
									VectorCopy( dest, bottom );
									bottom[2] -= 128;
									trap->Trace( &trace, dest, NPC->r.mins, NPC->r.maxs, bottom, goal->s.number, NPC->clipmask , 0, 0, 0);
									if ( trace.fraction < 1.0f )
									{//hit floor, okay to land here
										break;
									}
									sideTry++;
								}
								if ( sideTry >= 10 )
								{//screw it, just jump right at him?
									VectorCopy( goal->r.currentOrigin, dest );
								}
							}

							if ( DOM_Jedi_Jump( NPC, dest, goal->s.number ) )
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
									G_SetAnim( NPC, ucmd, SETANIM_BOTH, jumpAnim, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD );
								}

								NPC->client->ps.fd.forceJumpZStart = NPC->r.currentOrigin[2];
								//NPC->client->ps.pm_flags |= PMF_JUMPING;

								NPC->client->ps.weaponTime = NPC->client->ps.torsoTimer;
								NPC->client->ps.fd.forcePowersActive |= ( 1 << FP_LEVITATION );
								
								//if ( NPC->client->NPC_class == CLASS_BOBAFETT )
								//{
								//	G_SoundOnEnt( NPC, CHAN_ITEM, "sound/boba/jeton.wav" );
								//	NPC->client->jetPackTime = level.time + Q_irand( 1000, 3000 );
								//}
								//else
								//{
									G_SoundOnEnt( NPC, CHAN_BODY, "sound/weapons/force/jump.wav" );
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
						ucmd->forwardmove = 127;
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

qboolean DOM_Jedi_ClearPathToSpot( gentity_t *NPC, vec3_t dest, int impactEntNum )
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

//
//
// END: Experimental Jedi Jumping....
//
//

extern void DOM_Jedi_SetEnemyInfo( vec3_t enemy_dest, vec3_t enemy_dir, float *enemy_dist, vec3_t enemy_movedir, float *enemy_movespeed, int prediction );
extern void DOM_Jedi_EvasionSaber( vec3_t enemy_movedir, float enemy_dist, vec3_t enemy_dir );
extern void Cmd_Reload_f ( gentity_t *ent );
extern gentity_t *NPC;

//attack/fire at currentEnemy while moving towards DestPosition
void DOM_BotBehave_AttackMove(bot_state_t *bs)
{
	vec3_t viewDir;
	vec3_t ang;
	vec3_t enemyOrigin;
	vec3_t	enemy_dir, enemy_movedir, enemy_dest;
	float	enemy_dist, enemy_movespeed;

	//switch to an approprate weapon
	int desiredweap;
	float range;

	if(!bs->frame_Enemy_Vis && bs->enemySeenTime < level.time)
	{//lost track of enemy
		bs->currentEnemy = NULL;
		//trap->Print("Not visible\n");
		return;
	}

	if ( bs->cur_ps.weapon == WP_SABER )
	{
		if ( !DOM_Jedi_ClearPathToSpot( &g_entities[bs->client], bs->currentEnemy->r.currentOrigin, bs->currentEnemy->s.number ) )
		{
			if (DOM_Jedi_TryJump( &g_entities[bs->client], bs->currentEnemy ))
			{// Try special jedi jumping! :)
				return;
			}
		}
	}

	NPC = &g_entities[bs->client];

	//See where enemy will be 300 ms from now
	DOM_Jedi_SetEnemyInfo( enemy_dest, enemy_dir, &enemy_dist, enemy_movedir, &enemy_movespeed, 300 );
	DOM_Jedi_EvasionSaber( enemy_movedir, enemy_dist, enemy_dir );

	DOM_FindOrigin(bs->currentEnemy, enemyOrigin);

	range = DOM_TargetDistance(bs, bs->currentEnemy, enemyOrigin);
	
	desiredweap = DOM_FindWeaponforRange(bs, range);

	if(desiredweap != bs->virtualWeapon && desiredweap != -1)
	{//need to switch to desired weapon otherwise stay with what you go
		BotSelectChoiceWeapon(bs, desiredweap, qtrue);
	}

	//move towards DestPosition
	if (VectorLength(bs->velocity) <= 16)
		DOM_BotMoveto(bs, qtrue);
	else
		DOM_BotMoveto(bs, qfalse);

	if(bs->wpSpecial)
	{//in special wp move, don't do interrupt it.
		//trap->Print("wpSpecial\n");
		return;
	}

	//set viewangle
	//bs->eye[2]+=32;

	//VectorSubtract(bs->currentEnemy->client->ps.origin, bs->eye, viewDir);
	VectorCopy(bs->currentEnemy->client->ps.origin, enemyOrigin);
	//enemyOrigin[2]+=irand(16,32);
	VectorSubtract(enemyOrigin, bs->eye, viewDir);
	vectoangles(viewDir, ang);
	//ang[PITCH]=0;
	//ang[ROLL]=0;
	VectorCopy(ang, bs->goalAngles);

	bs->virtualWeapon = bs->cur_ps.weapon;

	if ( g_entities[bs->cur_ps.clientNum].client->ps.stats[STAT_AMMO] <= 0 )
	{
		//trap->Print("DEBUG: Reload!\n");
		g_entities[bs->cur_ps.clientNum].client->ps.ammo = 100; // UQ1: NPCs need to cheat a little :)
		g_entities[bs->cur_ps.clientNum].client->ps.stats[STAT_AMMO] = 100; // UQ1: NPCs need to cheat a little :)
		g_entities[bs->cur_ps.clientNum].client->clipammo[bs->cur_ps.weapon] = 100;//GetWeaponAmmoClip( bs->cur_ps.weapon, g_entities[bs->cur_ps.clientNum].s.weaponVariation );

		//Add_Ammo (NPC, client->ps.weapon, 100);
		Cmd_Reload_f (&g_entities[bs->cur_ps.clientNum]);
		//trap->Print("reload\n");
	}
	else /*if(range < 2048.0f //DOM_MaximumAttackDistance[bs->virtualWeapon]
		&& range > 0 //DOM_MinimumAttackDistance[bs->virtualWeapon]
	//if(bs->cur_ps.weapon == bs->virtualWeapon && range <= DOM_IdealAttackDistance[bs->virtualWeapon] * 1.1
		&& (InFieldOfVision(g_entities[bs->cur_ps.clientNum].client->ps.viewangles, 64, ang) 
			|| (bs->virtualWeapon == WP_SABER && InFieldOfVision(g_entities[bs->cur_ps.clientNum].client->ps.viewangles, 100, ang))) )*/
	{//don't attack unless you're inside your AttackDistance band and actually pointing at your enemy.  
		//This is to prevent the bots from attackmoving with the saber @ 500 meters. :)
		trap->EA_Attack(bs->client);
		//trap->Print("fired\n");
	}
	//else
	//	trap->Print("range\n");
}


//special function to hack the distance readings for extremely tall entities like
//the rancor or func_breakable doors.
float DOM_TargetDistance(bot_state_t *bs, gentity_t* target, vec3_t targetorigin)
{
	vec3_t enemyOrigin;

	if(strcmp(target->classname, "misc_siege_item") == 0
		|| strcmp(target->classname, "func_breakable") == 0 
		|| (target->client && target->client->NPC_class == CLASS_RANCOR)) 
	{//flatten origin heights and measure
		VectorCopy(targetorigin, enemyOrigin);
		if(fabsf(enemyOrigin[2] - bs->eye[2]) < 150)
		{//don't flatten unless you're on the same relative plane
			enemyOrigin[2] = bs->eye[2];
		}
		
		if(target->client && target->client->NPC_class == CLASS_RANCOR)
		{//Rancors are big and stuff
			return Distance(bs->eye, enemyOrigin) - 60;
		}
		else if(strcmp(target->classname, "misc_siege_item") == 0)
		{//assume this is a misc_siege_item.  These have absolute based mins/maxs.
			//Scale for the entity's bounding box
			float adjustor;
			float x = fabsf(bs->eye[0] - enemyOrigin[0]);
			float y = fabsf(bs->eye[1] - enemyOrigin[1]);
			float z = fabsf(bs->eye[2] - enemyOrigin[2]);
			
			//find the general direction of the impact to determine which bbox length to
			//scale with
			if(x > y && x > z)
			{//x
				adjustor = target->r.maxs[0];
				//adjustor = target->r.maxs[0] - enemyOrigin[0];
			}
			else if( y > x && y > z )
			{//y
				adjustor = target->r.maxs[1];
				//adjustor = target->r.maxs[1] - enemyOrigin[1];
			}
			else
			{//z
				adjustor = target->r.maxs[2];
				//adjustor = target->r.maxs[2] - enemyOrigin[2];
			}

			return Distance(bs->eye, enemyOrigin) - adjustor + 15;
		}
		else if(strcmp(target->classname, "func_breakable") == 0)
		{
			//Scale for the entity's bounding box
			float adjustor;
			//float x = fabs(bs->eye[0] - enemyOrigin[0]);
			//float y = fabs(bs->eye[1] - enemyOrigin[1]);
			//float z = fabs(bs->eye[2] - enemyOrigin[2]);
			
			//find the smallest min/max and use that.
			if((target->r.absmax[0] - enemyOrigin[0]) < (target->r.absmax[1] - enemyOrigin[1]))
			{
				adjustor = target->r.absmax[0] - enemyOrigin[0];
			}
			else
			{
				adjustor = target->r.absmax[1] - enemyOrigin[1];
			}

			/*
			//find the general direction of the impact to determine which bbox length to
			//scale with
			if(x > y && x > z)
			{//x
				adjustor = target->r.absmax[0] - enemyOrigin[0];
			}
			else if( y > x && y > z )
			{//y
				adjustor = target->r.absmax[1] - enemyOrigin[1];
			}
			else
			{//z
				adjustor = target->r.absmax[2] - enemyOrigin[2];
			}
			*/

			return Distance(bs->eye, enemyOrigin) - adjustor + 15;
		}
		else
		{//func_breakable
			return Distance(bs->eye, enemyOrigin);
		}
	}
	else
	{//standard stuff
		return Distance(bs->eye, targetorigin);
	}
}


//Basic form of defend system.  Used for situation where you don't want to use the waypoint
//system
void DOM_BotBehave_DefendBasic(bot_state_t *bs, vec3_t defpoint)
{
	float dist;

	dist = Distance(bs->origin, defpoint);

	if(bs->currentEnemy)
	{//see an enemy
		DOM_BotBehave_AttackBasic(bs, bs->currentEnemy);
		if(dist > DEFEND_MAXDISTANCE)
		{//attack move back into the defend range
			//RAFIXME:  We need to have a basic version of the attackmove function
			DOM_BotMove(bs, defpoint, qfalse, qfalse);
		}
		else if(dist > DEFEND_MAXDISTANCE * .9)
		{//nearing max distance hold here and attack
			trap->EA_Move(bs->client, vec3_origin, 0);
		}
		else
		{//just attack them
		}
	}
	else
	{//don't see an enemy
		if(DOM_DontBlockAllies(bs))
		{
		}
		else if(dist < DEFEND_MINDISTANCE)
		{//just stand around and wait
			//RAFIXME:  visual scan stuff here?
		}
		else
		{//move closer to defend target
			DOM_BotMove(bs, defpoint, qfalse, qfalse);
		}
	}
}

qboolean DOM_BotBehave_CheckBackstab(bot_state_t *bs)
{// UQ1: Check if there is an enemy behind us that we can backstab...
	vec3_t		forward, back_org, cur_org, moveDir;
	trace_t		tr;
	gentity_t	*enemy = NULL;

	if (bs->cur_ps.weapon != WP_SABER)
		return qfalse;

	VectorCopy(bs->cur_ps.origin, cur_org);
	cur_org[2]+=24;
	AngleVectors( bs->cur_ps.viewangles, forward, NULL, NULL );
	VectorMA( cur_org, -64, forward, back_org );

	trap->Trace( &tr, cur_org, NULL, NULL, back_org, bs->client, MASK_SHOT , 0, 0, 0);

	if (tr.entityNum < 0 || tr.entityNum > ENTITYNUM_MAX_NORMAL)
		return qfalse;

	enemy = &g_entities[tr.entityNum];

	if (!enemy 
		|| (enemy->s.eType != ET_PLAYER && enemy->s.eType != ET_NPC)
		|| !enemy->client 
		|| enemy->health < 1 
		|| (enemy->client->ps.eFlags & EF_DEAD))
		return qfalse;

	if (OnSameTeam(&g_entities[bs->client], enemy))
		return qfalse;

	// OK. We have a target. First make it not happen constantly with some randomization!
	//if (irand(0, 100) > 50)
	//	return qfalse;

	// OK, let's stab away! bwahahaha!
	VectorCopy(bs->cur_ps.origin, cur_org);
	VectorMA( cur_org, -64, forward, back_org );

	VectorSubtract(cur_org, bs->origin, moveDir); 
		
	moveDir[2] = 0;
	VectorNormalize(moveDir);

	//adjust the moveDir to do strafing
	DOM_AdjustforStrafe(bs, moveDir);
	DOM_TraceMove(bs, moveDir, tr.entityNum);
	trap->EA_Move(bs->client, moveDir, 5000);
	trap->EA_Attack(bs->client);

	//trap->Print("DOM BOT DEBUG: %s is using a backstab attack.\n", g_entities[bs->client].client->pers.netname);

	return qtrue;
}

qboolean DOM_BotBehave_CheckUseKata(bot_state_t *bs)
{// UQ1: Check if there is an in front of us that we can use our kata on...
	vec3_t		forward, back_org, cur_org;
	trace_t		tr;
	gentity_t	*enemy = NULL;

	if (bs->cur_ps.weapon != WP_SABER)
		return qfalse;

	VectorCopy(bs->cur_ps.origin, cur_org);
	cur_org[2]+=24;
	AngleVectors( bs->cur_ps.viewangles, forward, NULL, NULL );
	VectorMA( cur_org, 64, forward, back_org );

	trap->Trace( &tr, cur_org, NULL, NULL, back_org, bs->client, MASK_SHOT , 0, 0, 0);

	if (tr.entityNum < 0 || tr.entityNum > ENTITYNUM_MAX_NORMAL)
		return qfalse;

	enemy = &g_entities[tr.entityNum];

	if (!enemy 
		|| (enemy->s.eType != ET_PLAYER && enemy->s.eType != ET_NPC)
		|| !enemy->client 
		|| enemy->health < 1 
		|| (enemy->client->ps.eFlags & EF_DEAD))
		return qfalse;

	if (OnSameTeam(&g_entities[bs->client], enemy))
		return qfalse;

	// OK. We have a target. First make it not happen constantly with some randomization!
	if (irand(0, 200) > 50)
		return qfalse;

	// OK, let's stab away! bwahahaha!
	trap->EA_Attack(bs->client);
	trap->EA_Alt_Attack(bs->client);

	//trap->Print("DOM BOT DEBUG: %s is using a kata attack.\n", g_entities[bs->client].client->pers.netname);

	return qtrue;
}

qboolean DOM_BotBehave_CheckUseCrouchAttack(bot_state_t *bs)
{// UQ1: Check if there is an in front of us that we can use our special crouch attack on...
	vec3_t		forward, back_org, cur_org, moveDir;
	trace_t		tr;
	gentity_t	*enemy = NULL;

	if (bs->cur_ps.weapon != WP_SABER)
		return qfalse;

	VectorCopy(bs->cur_ps.origin, cur_org);
	cur_org[2]+=24;
	AngleVectors( bs->cur_ps.viewangles, forward, NULL, NULL );
	VectorMA( cur_org, 64, forward, back_org );

	trap->Trace( &tr, cur_org, NULL, NULL, back_org, bs->client, MASK_SHOT , 0, 0, 0);

	if (tr.entityNum < 0 || tr.entityNum > ENTITYNUM_MAX_NORMAL)
		return qfalse;

	enemy = &g_entities[tr.entityNum];

	if (!enemy 
		|| (enemy->s.eType != ET_PLAYER && enemy->s.eType != ET_NPC)
		|| !enemy->client 
		|| enemy->health < 1 
		|| (enemy->client->ps.eFlags & EF_DEAD))
		return qfalse;

	if (OnSameTeam(&g_entities[bs->client], enemy))
		return qfalse;

	// OK. We have a target. First make it not happen constantly with some randomization!
	if (irand(0, 200) > 10)
		return qfalse;

	// OK, let's stab away! bwahahaha!
	VectorCopy(bs->cur_ps.origin, cur_org);
	VectorMA( cur_org, 64, forward, back_org );

	VectorSubtract(cur_org, bs->origin, moveDir); 
		
	moveDir[2] = 0;
	VectorNormalize(moveDir);

	//adjust the moveDir to do strafing
	DOM_AdjustforStrafe(bs, moveDir);
	DOM_TraceMove(bs, moveDir, tr.entityNum);
	trap->EA_Crouch(bs->client);
	trap->EA_Move(bs->client, moveDir, 5000);
	trap->EA_Attack(bs->client);
	trap->EA_Alt_Attack(bs->client);

	//trap->Print("DOM BOT DEBUG: %s is using a crouch attack.\n", g_entities[bs->client].client->pers.netname);

	return qtrue;
}

gentity_t *DOM_GetRandomCloseEntityForJump(vec3_t origin)
{
	int			i = 0;
	gentity_t	*ent = NULL;
	int			IN_RANGE_ENTITIES[MAX_GENTITIES];
	int			NUM_IN_RANGE_ENTITIES = 0;

	for (i = 0; i < MAX_GENTITIES; i++)
	{
		float dist;

		ent = &g_entities[i];

		if (!ent) continue;
		if (!ent->classname || !ent->classname[0]) continue;

		dist = Distance(ent->s.origin, origin);

		if (dist <= 400 && dist > 192)
		{
			IN_RANGE_ENTITIES[NUM_IN_RANGE_ENTITIES] = ent->s.number;
			NUM_IN_RANGE_ENTITIES++;
		}
	}

	if (NUM_IN_RANGE_ENTITIES > 0)
		return &g_entities[Q_irand(0, NUM_IN_RANGE_ENTITIES-1)];

	return NULL;
}

//This is the basic, basic attack system, this doesn't use the waypoints system so you can
//link to it from inside the waypoint nav system without problems
void DOM_BotBehave_AttackBasic(bot_state_t *bs, gentity_t* target)
{
	vec3_t enemyOrigin, viewDir, ang, moveDir;
	float dist;

	if ( bs->virtualWeapon == WP_SABER )
	{
		if ( !DOM_Jedi_ClearPathToSpot( &g_entities[bs->client], bs->currentEnemy->r.currentOrigin, bs->currentEnemy->s.number ) )
		{
			if (DOM_Jedi_TryJump( &g_entities[bs->client], bs->currentEnemy ))
			{// Try special jedi jumping! :)
				return;
			}
		}
	}

	DOM_FindOrigin(target, enemyOrigin);

	dist = DOM_TargetDistance(bs, target, enemyOrigin);

	//face enemy
	VectorSubtract(enemyOrigin, bs->eye, viewDir);
	vectoangles(viewDir, ang);
	ang[PITCH] = 0;
	ang[ROLL]=0;
	VectorCopy(ang, bs->goalAngles);

	//check to see if there's a detpack in the immediate area of the target.
	if(bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_DET_PACK))
	{//only check if you got det packs.
		DOM_BotWeapon_Detpack(bs, target);
	}

	if(!BG_SaberInKata(bs->cur_ps.saberMove) && bs->cur_ps.fd.forcePower > 60 && 
		bs->cur_ps.weapon == WP_SABER && dist < 128 && InFieldOfVision(bs->viewangles, 120, ang))
	{//KATA!
		trap->EA_Attack(bs->client);
		trap->EA_Alt_Attack(bs->client);
		return;
	}

	//Stoiss add, Stance swift for npcs with a saber, --- eez's saber style change code.
	if (bs->saberPowerTime < level.time)
			{ //Don't just use strong attacks constantly, switch around a bit
				if (Q_irand(1, 10) <= 5)
				{
					bs->saberPower = qtrue;
				}
				else
				{
					bs->saberPower = qfalse;
				}

				bs->saberPowerTime = level.time + Q_irand(3000, 15000);
			}

			if ( g_entities[bs->client].client->ps.fd.saberAnimLevel != SS_STAFF
				&& g_entities[bs->client].client->ps.fd.saberAnimLevel != SS_DUAL )
			{
				if (bs->currentEnemy->health > 75 
					&& g_entities[bs->client].client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > 2)
				{
					if (g_entities[bs->client].client->ps.fd.saberAnimLevel != SS_SORESU 
						&& bs->saberPower)
					{ //if we are up against someone with a lot of health and we have a strong attack available, then h4q them
						Cmd_SaberAttackCycle_f(&g_entities[bs->client]);
					}
				}
				else if (bs->currentEnemy->health > 40 
					&& g_entities[bs->client].client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > 1)
				{
					if (g_entities[bs->client].client->ps.fd.saberAnimLevel != SS_SHII_CHO)
					{ //they're down on health a little, use level 2 if we can
						Cmd_SaberAttackCycle_f(&g_entities[bs->client]);
					}
				}
				else
				{
					if (g_entities[bs->client].client->ps.fd.saberAnimLevel != SS_MAKASHI)
					{ //they've gone below 40 health, go at them with quick attacks
						Cmd_SaberAttackCycle_f(&g_entities[bs->client]);
					}
				}
			}

#ifndef __MMO__
	if (bs->virtualWeapon == WP_SABER)
	{
		// UQ1: Added, backstab check...
		if (DOM_BotBehave_CheckBackstab(bs))
			return;

		// UQ1: Added, kata check...
		if (DOM_BotBehave_CheckUseKata(bs))
			return;

		// UQ1: Added, special crouch attack check...
		if (DOM_BotBehave_CheckUseCrouchAttack(bs))
			return;
	}
#endif //__MMO__

	if(bs->meleeStrafeTime < level.time)
	{//select a new strafing direction
		//0 = no strafe
		//1 = strafe right
		//2 = strafe left
		bs->meleeStrafeDir = Q_irand(0,2);
		bs->meleeStrafeTime = level.time + Q_irand(1500, 2800);
	}

	VectorSubtract(enemyOrigin, bs->origin, moveDir); 

	if(dist < DOM_MinimumAttackDistance[bs->virtualWeapon])
	//if(dist < DOM_IdealAttackDistance[bs->virtualWeapon] * .7)
	{//move back
		VectorScale(moveDir, -1, moveDir);
	}
	else if(dist > DOM_IdealAttackDistance[bs->virtualWeapon])
	//if(dist < DOM_IdealAttackDistance[bs->virtualWeapon] * .7)
	{//move forward
		VectorScale(moveDir, 1, moveDir);
	}
	if ( bs->virtualWeapon == WP_SABER && Q_irand(1, 100) >= 95)
	{// Let's jump away from them and jump back... Just to mix things up...
		gentity_t *RANDOM_ENT = DOM_GetRandomCloseEntityForJump(bs->currentEnemy->r.currentOrigin);

		if ( RANDOM_ENT && !DOM_Jedi_ClearPathToSpot( &g_entities[bs->client], RANDOM_ENT->s.origin, RANDOM_ENT->s.number ) )
		{
			if (DOM_Jedi_TryJump( &g_entities[bs->client], RANDOM_ENT ))
			{// Try special jedi jumping! :)
				return;
			}
		}
	}
		
	moveDir[2] = 0;
	VectorNormalize(moveDir);

	//adjust the moveDir to do strafing
	DOM_AdjustforStrafe(bs, moveDir);

	/*
	if(!VectorCompare(vec3_origin, moveDir))
	{
		DOM_TraceMove(bs, moveDir, target->s.clientNum);
		trap->EA_Move(bs->client, moveDir, 5000);
	}

	if(bs->cur_ps.weapon == bs->virtualWeapon
		&& (InFieldOfVision(bs->viewangles, 30, ang) 
		|| (bs->virtualWeapon == WP_SABER && InFieldOfVision(bs->viewangles, 120, ang))) )
	{//not switching weapons so attack
		trap->EA_Attack(bs->client);
	}
	*/

	DOM_BotBehave_AttackMove(bs);
}


void DOM_BotBehave_Attack(bot_state_t *bs)
{
	int desiredweap = DOM_FavoriteWeapon(bs, bs->currentEnemy);

	if( bs->frame_Enemy_Len >2048.0f /*DOM_MaximumAttackDistance[desiredweap]*/)
	//if( bs->frame_Enemy_Len > DOM_IdealAttackDistance[desiredweap] * 1.1)
	{//this should be an attack while moving function but for now we'll just use moveto
		vec3_t enemyOrigin;
		DOM_FindOrigin(bs->currentEnemy, enemyOrigin);
		VectorCopy(enemyOrigin, bs->DestPosition);
		bs->DestIgnore = bs->currentEnemy->s.number;
		DOM_BotBehave_AttackMove(bs);
		//trap->Print("Attack move.\n");
		return;
	}

	//determine which weapon you want to use
	if(desiredweap != bs->virtualWeapon)
	{//need to switch to desired weapon
		BotSelectChoiceWeapon(bs, desiredweap, qtrue);
	}

	//we're going to go get in close so null out the wpCurrent so it will update when we're 
	//done.
	bs->wpLast = NULL;
	bs->wpCurrent = NULL;

	//use basic attack
	DOM_BotBehave_AttackBasic(bs, bs->currentEnemy);
	//trap->Print("Attack basic.\n");
}


//Attack an enemy or track after them if you loss them.
void DOM_BotTrackAttack(bot_state_t *bs)
{
	int distance = Distance(bs->origin, bs->lastEnemySpotted);
	if(bs->frame_Enemy_Vis || bs->enemySeenTime > level.time )
	{//attack!
		bs->botBehave = BBEHAVE_ATTACK;
		VectorClear(bs->DestPosition);
		bs->DestIgnore = -1;
		return;
	}
	else if( distance < BOT_WEAPTOUCH_DISTANCE )
	{//do a visual scan
		if( bs->doVisualScan && bs->VisualScanTime < level.time )
		{//no dice.
			bs->doVisualScan = qfalse;
			bs->currentEnemy = NULL;
			bs->botBehave = BBEHAVE_STILL;
			return;
		}
		else 
		{//try looking around for 5 seconds
			VectorCopy(bs->lastEnemyAngles, bs->VisualScanDir);
			if(!bs->doVisualScan)
			{
				bs->doVisualScan = qtrue;
				bs->VisualScanTime = level.time + 5000;
			}
			bs->botBehave = BBEHAVE_VISUALSCAN;
			return;
		}
	}
	else
	{//lost him, go to the last seen location and see if we can find them
		bs->botBehave = BBEHAVE_MOVETO;
		VectorCopy(bs->lastEnemySpotted, bs->DestPosition);
		bs->DestIgnore = bs->currentEnemy->s.number;
		return;
	}
}


void DOM_BotSearchAndDestroy(bot_state_t *bs)
{
	if(!bs->currentEnemy && (VectorCompare(bs->DestPosition, vec3_origin) || Distance(bs->origin, bs->DestPosition) < BOT_WEAPTOUCH_DISTANCE) )
	{//hmmm, noone in the area and we're not already going somewhere
		int weapon = DOM_WanttoGetAmmo(bs);
		vec3_t weaploc;
		//Check to see if we need some weapons
		if(weapon != -1 && DOM_FindNearestWeapon(weapon, weaploc))
		{//want to get a weapon or weapon ammo and there's a spawnpoint for that weapon
			bs->botBehave = BBEHAVE_MOVETO;
			VectorCopy(weaploc, bs->DestPosition);
			bs->DestIgnore = -1;
			return;
		}
		else
		{//let's just randomly go to a spawnpoint
			/*gentity_t *spawnpoint;
			vec3_t temp;
			spawnpoint = SelectSpawnPoint(vec3_origin, temp, temp, level.clients[bs->client].sess.sessionTeam);
			if(spawnpoint)
			{
				VectorCopy(spawnpoint->s.origin, bs->DestPosition);
				bs->DestIgnore = -1;
				bs->botBehave = BBEHAVE_MOVETO;
				return;
			}*/
			// UQ1: That sucks.. Pick a random waypoint instead! :)
			int choice = irand(0, gWPNum-1);

			if (choice >= 0 && choice < gWPNum)
			{
				VectorCopy(gWPArray[choice]->origin, bs->DestPosition);
				bs->DestIgnore = -1;
				bs->botBehave = BBEHAVE_MOVETO;
				return;
			}
			else
			{//that's not good
				bs->botBehave = BBEHAVE_STILL;
				VectorClear(bs->DestPosition);
				bs->DestIgnore = -1;
				return;
			}
		}
	}
	else if( !bs->currentEnemy && !VectorCompare(bs->DestPosition, vec3_origin) )
	{//moving towards a weapon or spawnpoint
		bs->botBehave = BBEHAVE_MOVETO;
		return;
	}
	else
	{//have an enemy and can see him
		if( bs->currentEnemy != bs->tacticEntity )
		{//attacking someone other that your target
			//This should probably be some sort of attack move or something
			DOM_BotTrackAttack(bs);
			return;
		}
		else
		{
			DOM_BotTrackAttack(bs);
			return;
		}
	}
}


//get out of the way of allies if they're close
qboolean DOM_DontBlockAllies(bot_state_t *bs)
{
	int i;
	for(i = 0; i < level.maxclients; i++)
	{
		if(i != bs->client 
			&& g_entities[i].client
			&& g_entities[i].client->sess.sessionTeam == g_entities[bs->client].client->sess.sessionTeam
			&& Distance(g_entities[i].client->ps.origin, bs->origin) < BLOCKALLIESDISTANCE)
		{//on your team and too close
			vec3_t moveDir, DestOrigin;
			VectorSubtract(bs->origin, g_entities[i].client->ps.origin, moveDir);
			VectorAdd(bs->origin, moveDir, DestOrigin);
			DOM_BotMove(bs, DestOrigin, qfalse, qfalse);
			return qtrue;
		}
	}
	return qfalse;
}


//defend given entity from attack from attack
void DOM_BotDefend(bot_state_t *bs, gentity_t *defendEnt)
{
	vec3_t defendOrigin;
	float dist;

	DOM_FindOrigin(defendEnt, defendOrigin);

	if(strcmp(defendEnt->classname, "func_breakable") == 0 
		&& strcmp(defendEnt->paintarget, "shieldgen_underattack") == 0)
	{//dirty hack to get the bots to attack the shield generator on siege_hoth
		VectorSet(defendOrigin, -369, 858, -231);
	}

	dist = Distance(bs->origin, defendOrigin);

	if(bs->currentEnemy)
	{//see an enemy
		if(dist > DEFEND_MAXDISTANCE)
		{//attack move back into the defend range
			VectorCopy(defendOrigin, bs->DestPosition);
			bs->DestIgnore = defendEnt->s.number;
			DOM_BotBehave_AttackMove(bs);
		}
		else
		{//just attack them
			VectorClear(bs->DestPosition);
			bs->DestIgnore = -1;
			DOM_BotTrackAttack(bs);
		}
	}
	else
	{//don't see an enemy
		if(DOM_DontBlockAllies(bs))
		{
		}
		else if(dist < DEFEND_MINDISTANCE)
		{//just stand around and wait	
			VectorClear(bs->DestPosition);
			bs->DestIgnore = -1;
			bs->botBehave = BBEHAVE_STILL;
		}
		else
		{//move closer to defend target
			VectorCopy(defendOrigin, bs->DestPosition);
			bs->DestIgnore = defendEnt->s.number;
			bs->botBehave = BBEHAVE_MOVETO;
		}
	}
}


//update the currentEnemy visual data for the current enemy.
void DOM_EnemyVisualUpdate(bot_state_t *bs)
{
	vec3_t a;
	vec3_t enemyOrigin;
	vec3_t enemyAngles;
	trace_t tr;
	float dist;

	if(!bs->currentEnemy)
	{//bad!  This should never happen
		return;
	}

#ifdef __MMO__
	if (bs->enemySeenTime > level.time)
	{// UQ1: Speed things up in MMO mode...
		bs->frame_Enemy_Len = DOM_TargetDistance(bs, bs->currentEnemy, enemyOrigin);
		bs->frame_Enemy_Vis = 1;
		return;
	}
#endif //__MMO__

	DOM_FindOrigin(bs->currentEnemy, enemyOrigin);
	DOM_FindAngles(bs->currentEnemy, enemyAngles);

	VectorSubtract(enemyOrigin, bs->eye, a);
	dist = VectorLength(a);
	vectoangles(a, a);
	a[PITCH] = a[ROLL] = 0;

	trap->Trace(&tr, bs->eye, NULL, NULL, enemyOrigin, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if ((tr.entityNum == bs->currentEnemy->s.number && InFieldOfVision(bs->viewangles, 90, a) && !BotMindTricked(bs->client, bs->currentEnemy->s.number)) 
		|| BotCanHear(bs, bs->currentEnemy, dist))
	{//spotted him
		bs->frame_Enemy_Len = DOM_TargetDistance(bs, bs->currentEnemy, enemyOrigin);
		bs->frame_Enemy_Vis = 1;
		VectorCopy(enemyOrigin, bs->lastEnemySpotted);
		VectorCopy(bs->currentEnemy->s.angles, bs->lastEnemyAngles);
		bs->lastEnemyAngles[PITCH] = bs->lastEnemyAngles[ROLL] = 0;
		bs->enemySeenTime = level.time + BOT_VISUALLOSETRACKTIME;
	}
	else
	{//can't see him
		bs->frame_Enemy_Vis = 0;
	}
}

void DOM_CheckHolsterSaber(bot_state_t *bs)
{
	/*
	if( bs->cur_ps.weapon == WP_SABER 
		&& (!bs->currentEnemy || (bs->currentEnemy && VectorDistance(bs->origin, bs->currentEnemy->r.currentOrigin) > 256)) )
	{
		if (!bs->cur_ps.saberHolstered)
		{
			Cmd_ToggleSaber_f(&g_entities[bs->client]);
		}
	}
	*/
}

//the main scan regular scan for enemies for the TAB Bot
void DOM_ScanforEnemies(bot_state_t *bs)
{
	vec3_t	a, EnemyOrigin;
	int		closeEnemyNum = -1;
	float	closestdist = 99999;
	int		i = 0;
	float	distcheck;
	float	hasEnemyDist = 0;
	int		currentEnemyNum = -1;

	if (bs->enemySeenTime > level.time)
		return; // Not too quick!

	if (bs->enemyScanTime > level.time)
		return; // Not too quick!

	//RAFIXME - This is sort of a hack but I'm not in the mood to do a corpse analysis
	//function at the moment
	if(bs->currentEnemy)
	{
		if(bs->currentEnemy->health < 1 
			|| (bs->currentEnemy->client && bs->currentEnemy->client->sess.sessionTeam == TEAM_SPECTATOR)
			|| (bs->currentEnemy->s.eFlags & EF_DEAD) 
			|| (bs->currentEnemy->client && (bs->currentEnemy->client->ps.eFlags & EF_DEAD))
			|| OnSameTeam(bs->currentEnemy, &g_entities[bs->cur_ps.clientNum])
			|| strcmp(bs->currentEnemy->classname, "freed") == 0 )
		{//target died or because invalid, move to next target
			bs->currentEnemy = NULL;
		}
		else
		{
			return;
		}
	}

	bs->enemyScanTime = level.time + 5000;

	if(bs->currentEnemy)
	{//we're already locked onto an enemy
		if( bs->client == bs->currentEnemy->s.number )
		{
			trap->Print("Somehow this bot has locked onto itself. Not good.\n");
		}

		if(bs->currentTactic == BOTORDER_SEARCHANDDESTROY 
			&& bs->tacticEntity)
		{//currently going after search and destroy target
			if(bs->tacticEntity->s.number == bs->currentEnemy->s.number)
			{
				DOM_EnemyVisualUpdate(bs);
				DOM_CheckHolsterSaber(bs);
				return;
			}
		}
		//If you're locked onto an objective, don't lose it.
		else if(bs->currentTactic == BOTORDER_OBJECTIVE
			&& bs->tacticEntity)
		{
			if(bs->tacticEntity->s.number == bs->currentEnemy->s.number)
			{
				DOM_EnemyVisualUpdate(bs);
				DOM_CheckHolsterSaber(bs);
				return;
			}
		}
		else if (bs->frame_Enemy_Vis || bs->enemySeenTime > level.time - 2000)
		{
			return;
		}

		hasEnemyDist = bs->frame_Enemy_Len;
		currentEnemyNum = bs->currentEnemy->s.number;
	}


	for(i = 0; i <= level.num_entities; i++)
	{
		if (i != bs->client 
			&& ( (g_entities[i].client && !OnSameTeam(&g_entities[bs->client], &g_entities[i]) && PassStandardEnemyChecks(bs, &g_entities[i]) && PassLovedOneCheck(bs, &g_entities[i]))
			|| (g_entities[i].s.eType == ET_NPC && g_entities[i].s.NPC_class != CLASS_VEHICLE && g_entities[i].health > 0 && !(g_entities[i].s.eFlags & EF_DEAD)) 
			|| (g_entities[i].s.weapon == WP_EMPLACED_GUN 
			&& g_entities[i].teamnodmg != g_entities[bs->client].client->sess.sessionTeam
			&& g_entities[i].s.pos.trType == TR_STATIONARY)) )
		{
			DOM_FindOrigin(&g_entities[i], EnemyOrigin);

			//[test]
			//I'm guessing that this isn't nessicary and might speed things up.
			/*
			if(!BotPVSCheck(EnemyOrigin, bs->eye))
			{//can't physically see this entity
				continue;
			}
			*/

			VectorSubtract(EnemyOrigin, bs->eye, a);
			distcheck = VectorLength(a);
			vectoangles(a, a);
			a[PITCH] = a[ROLL] = 0;

			if (distcheck < closestdist && ((InFieldOfVision(bs->viewangles, 90, a) 
				&& !BotMindTricked(bs->client, i)) 
				|| BotCanHear(bs, &g_entities[i], distcheck)) 
				&& OrgVisible(bs->eye, EnemyOrigin, -1))
			{
				if (!hasEnemyDist || distcheck < (hasEnemyDist - 128) || currentEnemyNum == i)
				{ //if we have an enemy, only switch to closer if he is 128+ closer to avoid flipping out
					closestdist = distcheck;
					closeEnemyNum = i;
				}
			}
		}
	}

	if(closeEnemyNum == -1)
	{
		if(bs->currentEnemy)
		{//no enemies in the area but we were locked on previously.  Clear frame visual data so
		//we know that we should go go try to find them again.
			bs->frame_Enemy_Vis = 0;
		}
		else
		{//we're all along.  No real need to update stuff in this case.
		}
	}
	else
	{//have a target, update their data
		vec3_t EnemyAngles;
		DOM_FindOrigin(&g_entities[closeEnemyNum], EnemyOrigin);
		DOM_FindAngles(&g_entities[closeEnemyNum], EnemyAngles);

		bs->frame_Enemy_Len = closestdist;
		bs->frame_Enemy_Vis = 1;
		bs->currentEnemy = &g_entities[closeEnemyNum];
		VectorCopy(EnemyOrigin, bs->lastEnemySpotted);
		VectorCopy(EnemyAngles, bs->lastEnemyAngles);
		bs->lastEnemyAngles[PITCH] = bs->lastEnemyAngles[ROLL] = 0;
		bs->enemySeenTime = level.time + BOT_VISUALLOSETRACKTIME;
	}

	DOM_CheckHolsterSaber(bs);
}


//visually scanning in the given direction.
void DOM_BotBehave_VisualScan(bot_state_t *bs)
{
	//RAFIXME - this could use more stuff
	VectorCopy(bs->VisualScanDir, bs->goalAngles);
	bs->goalAngles[PITCH] = bs->goalAngles[ROLL] = 0;
	bs->wpLast = NULL;
	bs->wpCurrent = NULL;
}

void DOM_DetermineCTFGoal(bot_state_t *bs)
{//has the bot decide what role it should take in a CTF fight
	int i;
	int NumOffence = 0;		//number of bots on offence
	int NumDefense = 0;		//number of bots on defence

	bot_state_t *tempbot;
	
	//clear out the current tactic
	bs->currentTactic = BOTORDER_OBJECTIVE;
	bs->tacticEntity = NULL;

	for(i=0; i < MAX_CLIENTS; i++)
	{
		tempbot = botstates[i];

		if(!tempbot || !tempbot->inuse || tempbot->client == bs->client)
		{//this bot isn't in use or this is the current bot
			continue;
		}

		if(g_entities[tempbot->client].client->sess.sessionTeam 
			!= g_entities[bs->client].client->sess.sessionTeam)
		{
			continue;
		}

		if(tempbot->currentTactic == BOTORDER_OBJECTIVE)
		{//this bot is going for/defending the flag
			if(tempbot->objectiveType == OT_CAPTURE)
			{
				NumOffence++;
			}
			else
			{//it's on defense
				NumDefense++;
			}
		}
	}

	if( NumDefense < NumOffence )
	{//we have less defenders than attackers.  Go on the defense.
			bs->objectiveType = OT_DEFENDCAPTURE;
	}
	else
	{//go on the attack
			bs->objectiveType = OT_CAPTURE;
	}
}

void DOM_DetermineWARZONEGoal(bot_state_t *bs)
{//has the bot decide what role it should take in a CTF fight
	int i;
	int NumOffence = 0;		//number of bots on offence
	int NumDefense = 0;		//number of bots on defence

	bot_state_t *tempbot;
	
	//clear out the current tactic
	bs->currentTactic = BOTORDER_OBJECTIVE;
	bs->tacticEntity = NULL;

	for(i=0; i < MAX_CLIENTS; i++)
	{
		tempbot = botstates[i];

		if(!tempbot || !tempbot->inuse || tempbot->client == bs->client)
		{//this bot isn't in use or this is the current bot
			continue;
		}

		if(g_entities[tempbot->client].client->sess.sessionTeam 
			!= g_entities[bs->client].client->sess.sessionTeam)
		{
			continue;
		}

		if(tempbot->currentTactic == BOTORDER_OBJECTIVE)
		{//this bot is going for/defending the flag
			if(tempbot->objectiveType == OT_TOUCH/*OT_CAPTURE*/)
			{
				NumOffence++;
			}
			else
			{//it's on defense
				NumDefense++;
			}
		}
	}

	if( NumDefense < NumOffence * 0.25 )
	{//we have less defenders than attackers.  Go on the defense.
		bs->objectiveType = OT_DEFENDCAPTURE;
	}
	else
	{//go on the attack
		bs->objectiveType = OT_TOUCH;//OT_CAPTURE;
	}
}

int DOM_PickWARZONEGoalType(bot_state_t *bs)
{//has the bot decide what role it should take in a CTF fight
	int i;
	int NumOffence = 0;		//number of bots on offence
	int NumDefense = 0;		//number of bots on defence

	bot_state_t *tempbot;
	
	//clear out the current tactic
	bs->currentTactic = BOTORDER_OBJECTIVE;
	bs->tacticEntity = NULL;

	for(i=0; i < MAX_CLIENTS; i++)
	{
		tempbot = botstates[i];

		if(!tempbot || !tempbot->inuse || tempbot->client == bs->client)
		{//this bot isn't in use or this is the current bot
			continue;
		}

		if(g_entities[tempbot->client].client->sess.sessionTeam 
			!= g_entities[bs->client].client->sess.sessionTeam)
		{
			continue;
		}

		if(tempbot->currentTactic == BOTORDER_OBJECTIVE)
		{//this bot is going for/defending the flag
			if(tempbot->objectiveType == OT_TOUCH/*OT_CAPTURE*/)
			{
				NumOffence++;
			}
			else
			{//it's on defense
				NumDefense++;
			}
		}
	}

	if( NumDefense < NumOffence * 0.25 )
	{//we have less defenders than attackers.  Go on the defense.
		bs->objectiveType = OT_DEFENDCAPTURE;
	}
	else
	{//go on the attack
		bs->objectiveType = OT_TOUCH;//OT_CAPTURE;
	}

	return bs->objectiveType;
}

//
//
// Guts of the DOM Bot's AI
//
//
void DOM_StandardBotAI(bot_state_t *bs, float thinktime)
{
	qboolean UsetheForce = qfalse;

#ifdef __BASIC_RANDOM_MOVEMENT_AI__
	bs->ideal_viewangles[PITCH] = bs->ideal_viewangles[ROLL] = 0;
	bs->ideal_viewangles[YAW] = irand(0,360);
	MoveTowardIdealAngles(bs);
	AngleVectors (bs->ideal_viewangles, g_entities[bs->client].movedir, NULL, NULL);
	trap->EA_Move(bs->client, g_entities[bs->client].movedir, 5000);
	return;
#endif //__BASIC_RANDOM_MOVEMENT_AI__

#ifdef __TEMPORARY_HACK_FOR_BOT_LIGHTSABERS__
	g_entities[bs->client].client->ps.weapon = WP_SABER;
	g_entities[bs->client].client->ps.weaponVariation = 0;
	bs->virtualWeapon = g_entities[bs->client].client->ps.weapon;
#endif //__TEMPORARY_HACK_FOR_BOT_LIGHTSABERS__

	//Reset the action states
	bs->doAttack = qfalse;
	bs->doAltAttack = qfalse;
	bs->doWalk = qfalse;
	bs->virtualWeapon = bs->cur_ps.weapon;
	bs->botBehave = BBEHAVE_NONE;

	if (gDeactivated || g_entities[bs->client].client->tempSpectate > level.time)
	{//disable ai during bot editting or while waiting for respawn in siege.
		bs->wpLast = NULL;
		bs->wpCurrent = NULL;
		bs->currentEnemy = NULL;
		bs->wpDestination = NULL;
		DOM_ClearRoute(bs->botRoute);
		VectorClear(bs->lastDestPosition);
		bs->wpSpecial = qfalse;

		//reset tactical stuff
		bs->currentTactic = BOTORDER_NONE;
		bs->tacticEntity = NULL;
		bs->objectiveType = 0;
		bs->doZodKneel = qfalse;
		return;

	}

	if(bs->cur_ps.pm_type == PM_INTERMISSION 
		|| g_entities[bs->client].client->sess.sessionTeam == TEAM_SPECTATOR)
	{//in intermission
		//Mash the button to prevent the game from sticking on one level.
		trap->EA_Attack(bs->client);
		return;
	}

	//ripped the death stuff right from the standard AI.
	if (!bs->lastDeadTime)
	{ //just spawned in?
		bs->lastDeadTime = level.time;
		bs->doZodKneel = qfalse;
		bs->botOrder = BOTORDER_NONE;
		bs->orderEntity = NULL;
		bs->ordererNum = bs->client;
		VectorClear(bs->DestPosition);
		bs->DestIgnore = -1;
	}

	if (g_entities[bs->client].health < 1 || g_entities[bs->client].client->ps.pm_type == PM_DEAD)
	{
		bs->lastDeadTime = level.time;

		if (!bs->deathActivitiesDone && bs->lastHurt && bs->lastHurt->client && bs->lastHurt->s.number != bs->client)
		{
			BotDeathNotify(bs);
			if (PassLovedOneCheck(bs, bs->lastHurt))
			{
				//CHAT: Died
				bs->chatObject = bs->lastHurt;
				bs->chatAltObject = NULL;
				BotDoChat(bs, "Died", 0);
			}
			else if (!PassLovedOneCheck(bs, bs->lastHurt) &&
				botstates[bs->lastHurt->s.number] &&
				PassLovedOneCheck(botstates[bs->lastHurt->s.number], &g_entities[bs->client]))
			{ //killed by a bot that I love, but that does not love me
				bs->chatObject = bs->lastHurt;
				bs->chatAltObject = NULL;
				BotDoChat(bs, "KilledOnPurposeByLove", 0);
			}

			bs->deathActivitiesDone = 1;
		}
		
		//movement code
		bs->wpLast = NULL;
		bs->wpCurrent = NULL;
		bs->currentEnemy = NULL;
		bs->wpDestination = NULL;
		VectorClear(bs->lastDestPosition);
		DOM_ClearRoute(bs->botRoute);
		bs->wpSpecial = qfalse;
		

		//reset tactical stuff
		bs->currentTactic = BOTORDER_NONE;
		bs->tacticEntity = NULL;
		bs->objectiveType = 0;
		bs->doZodKneel = qfalse;

		//RACC - Try to respawn if you're done talking.
		if (rand()%10 < 5 &&
			(!bs->doChat || bs->chatTime < level.time))
		{
			trap->EA_Attack(bs->client);
		}

		return;
	}

	VectorCopy(bs->DestPosition, bs->lastDestPosition);


	//higher level thinking
	//for now, always do whatever your current order is
	//and jsut attack if you're not given an order.
	if(bs->botOrder == BOTORDER_NONE)
	{
		if(bs->currentTactic)
		{//already have a tactic, use it.
		}
		else if(g_gametype.integer == GT_CTF)
		{
			DOM_DetermineCTFGoal(bs);
		}
		else if(g_gametype.integer == GT_WARZONE)
		{
			DOM_DetermineWARZONEGoal(bs);
		}
		else
		{
			bs->currentTactic = BOTORDER_SEARCHANDDESTROY;
			bs->tacticEntity = NULL;
		}
	}
	else
	{
		bs->currentTactic = bs->botOrder;
		bs->tacticEntity = bs->orderEntity;
	}

	//Scan for enemy targets and update the visual check data
	DOM_ScanforEnemies(bs);

	//doing current order
	if(bs->currentTactic == BOTORDER_KNEELBEFOREZOD)
	{
		DOM_BotKneelBeforeZod(bs);
	}
	else if(bs->currentTactic == BOTORDER_SEARCHANDDESTROY)
	{
		DOM_BotSearchAndDestroy(bs);
	}
	else if(bs->currentTactic == BOTORDER_OBJECTIVE)
	{
		DOM_BotObjective(bs);
	}

	if(bs->currentEnemy && bs->currentEnemy->health <= 0)
	{
		bs->currentEnemy = NULL;
		bs->botBehave = BBEHAVE_NONE;
	}

	//behavior implimentation
#ifdef __WARZONE__
	if(bs->botBehave == BBEHAVE_MOVETO
		&& g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM] > 0
		&& !(g_entities[g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM]].s.teamowner == g_entities[bs->cur_ps.clientNum].client->sess.sessionTeam && g_entities[g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM]].s.time2 == 100)
		&& bs->wpCurrent && bs->wpNext && bs->wpDestination
		&& Distance(g_entities[g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM]].r.currentOrigin, bs->wpCurrent->origin) < Distance(g_entities[g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM]].r.currentOrigin, bs->wpNext->origin)
		&& g_gametype.integer == GT_WARZONE
		&& !bs->currentEnemy)
	{// Force the bot to stay still and finish the capture...
		DOM_BotBeStill(bs);
	}
	else
#endif //__WARZONE__
	if(bs->botBehave == BBEHAVE_MOVETO)
	{
		if (VectorLength(bs->velocity) <= 16)
			DOM_BotMoveto(bs, qtrue);
		else
			DOM_BotMoveto(bs, qfalse);
	}
	else if(bs->botBehave == BBEHAVE_ATTACK)
	{
		DOM_BotBehave_Attack(bs);
	}
	else if(bs->botBehave == BBEHAVE_VISUALSCAN)
	{
		DOM_BotBehave_VisualScan(bs);
	}
	else if(bs->botBehave == BBEHAVE_STILL)
	{
		DOM_BotBeStill(bs);
	}
	else
	{//BBEHAVE_NONE
		DOM_BotSearchAndDestroy(bs);
	}

#ifndef __MMO__
	//check for hard impacts
	//borrowed directly from impact code, so we might want to add in some fudge factor
	//at some point.  mmmmMM, fudge.
	if(bs->cur_ps.lastOnGround + 300 < level.time)
	{//been off the ground for a little while
		float speed = VectorLength(bs->cur_ps.velocity);
		if( ( speed >= 100 + g_entities[bs->client].health && bs->virtualWeapon != WP_SABER ) || ( speed >= 700 ) )
		{//moving fast enough to get hurt get ready to crouch roll
			if(bs->virtualWeapon != WP_SABER)
			{//try switching to saber
				if(!BotSelectChoiceWeapon(bs, WP_SABER, 1))
				{//ok, try switching to melee
					BotSelectChoiceWeapon(bs, WP_MELEE, 1);
				}
			}

			if(bs->virtualWeapon == WP_MELEE || bs->virtualWeapon == WP_SABER)
			{//in or switching to a weapon that allows us to do roll landings
				bs->duckTime = level.time + 300;
				if(!bs->lastucmd.forwardmove && !bs->lastucmd.rightmove)
				{//not trying to move at all so we should at least attempt to move
					trap->EA_MoveForward(bs->client);
				}
			}
		}
	}

	//Chat Stuff
	/*
	if (bs->doChat && bs->chatTime <= level.time)
	{
		if (bs->chatTeam)
		{
			trap->EA_SayTeam(bs->client, bs->currentChat);
			bs->chatTeam = 0;
		}
		else
		{
			trap->EA_Say(bs->client, bs->currentChat);
		}
		if (bs->doChat == 2)
		{
			BotReplyGreetings(bs);
		}
		bs->doChat = 0;
	}
	*/
#endif //__MMO__

	if(bs->duckTime > level.time)
	{
		trap->EA_Crouch(bs->client);
	}

	/*
	// UQ1: Try forse jumping to enemies and waypoints as needed...
	if(bs->botBehave == BBEHAVE_MOVETO 
		&& !bs->currentEnemy 
		&& bs->wpNext
		&& bs->cur_ps.fd.forcePowerLevel[FP_LEVITATION] >= FORCE_LEVEL_1
		&& VectorDistance(bs->cur_ps.origin, bs->wpNext->origin) <= 512)
	{
		if (bs->wpNext)
		{
			if (VectorDistanceNoHeight(bs->cur_ps.origin, bs->wpNext->origin) <= 16
				&& bs->cur_ps.origin[2] >= bs->wpNext->origin[2] + 16)
			{// We are there!

			}
			else if (bs->cur_ps.origin[2] + 16 < bs->wpNext->origin[2])
			{
				trap->EA_Jump(bs->client);

				if (g_entities[bs->client].client->ps.groundEntityNum == ENTITYNUM_NONE)
				{
					g_entities[bs->client].client->ps.pm_flags |= PMF_JUMP_HELD;
				}

				if (g_entities[bs->client].client->ps.fd.forcePower < 30)
				{
					g_entities[bs->client].client->ps.fd.forcePower = 30; // UQ1: Cheating bots lol!

					// UQ1: Gonna try something crazy here and let them really cheat when falling... hahehhe
					// super save from certain death below jump back to safety like NPCs do...
					if (bs->virtualWeapon == WP_SABER) // Jedi Only!
						g_entities[bs->client].client->ps.velocity[2] = 127;
				}
			}
		}
	}
	else if (bs->currentEnemy 
		&& bs->cur_ps.fd.forcePowerLevel[FP_LEVITATION] >= FORCE_LEVEL_1
		&& VectorDistance(bs->cur_ps.origin, bs->currentEnemy->r.currentOrigin) <= 512)
	{
		if (g_entities[bs->client].client->ps.groundEntityNum != ENTITYNUM_NONE
			&& VectorDistance(bs->cur_ps.origin, bs->currentEnemy->r.currentOrigin) >= 128
			&& VectorDistanceNoHeight(bs->cur_ps.origin, bs->currentEnemy->r.currentOrigin) <= 128)
		{// Wait till we get a better spot to jump from!

		}
		else if (VectorDistanceNoHeight(bs->cur_ps.origin, bs->currentEnemy->r.currentOrigin) <= 16
			&& bs->cur_ps.origin[2] >= bs->currentEnemy->r.currentOrigin[2] + 16)
		{// We are there!

		}
		else if (bs->cur_ps.origin[2] + 16 < bs->currentEnemy->r.currentOrigin[2])
		{
			//if (bs->currentEnemy->client && bs->currentEnemy->client->ps.groundEntityNum != ENTITYNUM_NONE)
			{
				trap->EA_Jump(bs->client);

				if (g_entities[bs->client].client->ps.groundEntityNum == ENTITYNUM_NONE)
				{
					g_entities[bs->client].client->ps.pm_flags |= PMF_JUMP_HELD;
				}

				if (g_entities[bs->client].client->ps.fd.forcePower < 30)
					g_entities[bs->client].client->ps.fd.forcePower = 30; // UQ1: Cheating bots lol!
			}
		}
	}
	else
	*/

	if(bs->jumpTime > level.time)
	{
		trap->EA_Jump(bs->client);

		if (g_entities[bs->client].client->ps.groundEntityNum == ENTITYNUM_NONE)
		{
			g_entities[bs->client].client->ps.pm_flags |= PMF_JUMP_HELD;
		}
	}

	//use action
	if(bs->useTime >level.time)
	{
		trap->EA_Use(bs->client);
	}

	//attack actions
	if(bs->doAttack)
	{
		trap->EA_Attack(bs->client);
	}

	if(bs->doAltAttack)
	{
		trap->EA_Alt_Attack(bs->client);
	}

	//Force powers are listed in terms of priority

	if ((bs->cur_ps.fd.forcePowersKnown & (1 << FP_PUSH)) && (bs->doForcePush > level.time || bs->cur_ps.fd.forceGripBeingGripped > level.time) && level.clients[bs->client].ps.fd.forcePower > forcePowerNeeded[level.clients[bs->client].ps.fd.forcePowerLevel[FP_PUSH]][FP_PUSH])
	{//use force push
		level.clients[bs->client].ps.fd.forcePowerSelected = FP_PUSH;
		UsetheForce = qtrue;
	}

	if (!UsetheForce && (bs->cur_ps.fd.forcePowersKnown & (1 << FP_PULL)) && bs->doForcePull > level.time && level.clients[bs->client].ps.fd.forcePower > forcePowerNeeded[level.clients[bs->client].ps.fd.forcePowerLevel[FP_PULL]][FP_PULL])
	{//use force pull
		level.clients[bs->client].ps.fd.forcePowerSelected = FP_PULL;
		UsetheForce = qtrue;
	}


	if(UsetheForce)
	{
		if (bot_forcepowers.integer && !g_forcePowerDisable.integer)
		{
			trap->EA_ForcePower(bs->client);
		}
	}

	//Set view angles
	MoveTowardIdealAngles(bs);

}


//find the favorite weapon that this bot has for this target at the moment.  This doesn't do any sort of distance checking, it 
//just returns the favorite weapon.
int DOM_FavoriteWeapon(bot_state_t *bs, gentity_t *target)
{
	int i;
	int bestweight = -1;
	int bestweapon = 0;

	for(i=0; i < WP_NUM_WEAPONS; i++)
	{
		if (target->flags & FL_DMG_BY_HEAVY_WEAP_ONLY)
		{//don't use weapons that can't damage this target
			if(!DOM_IsHeavyWeapon(bs, i))
			{
				continue;
			}
		}

		//try to use explosives on breakables if we can.
		if(strcmp(target->classname, "func_breakable") == 0)
		{
			if(/*bs->cur_ps.ammo[weaponData[i].ammoIndex] >= weaponData[i].energyPerShot
				&&*/ bs->cur_ps.stats[STAT_WEAPONS] & (1 << i))
			{	
				if(i == WP_DET_PACK)
				{
					bestweight = 100;
					bestweapon = i;
				}
				else if(i == WP_THERMAL)
				{
					bestweight = 99;
					bestweapon = i;
				}
				else if(i == WP_ROCKET_LAUNCHER)
				{
					bestweight = 99;
					bestweapon = i;
				}	
				else if(bs->botWeaponWeights[i] > bestweight)
				{
					bestweight = bs->botWeaponWeights[i];
					bestweapon = i;
				}
			}
		}
		else if (/*bs->cur_ps.ammo[weaponData[i].ammoIndex] >= weaponData[i].energyPerShot &&*/
			bs->botWeaponWeights[i] > bestweight &&
			(bs->cur_ps.stats[STAT_WEAPONS] & (1 << i)))
		{
			bestweight = bs->botWeaponWeights[i];
			bestweapon = i;
		}
	}

	return bestweapon;
}

char gObjectiveCfgStr[1024];

qboolean DOM_ObjectiveStillActive(int objective)
{//Is the given Objective for the given team still active?
	int i = 0;
	int	curteam = SIEGETEAM_TEAM1;
	int objectiveNum = 0;

	if(objective <= 0)
	{//bad objective number
		return qfalse;
	}

	while (gObjectiveCfgStr[i])
	{
		if (gObjectiveCfgStr[i] == '|')
		{ //switch over to team2, this is the next section
			/*
			if(team == SIEGETEAM_TEAM1)
			{//didn't find the objective
				return qfalse;
			}
			*/
            curteam = SIEGETEAM_TEAM2;
			objectiveNum = 0;
		}
		else if (gObjectiveCfgStr[i] == '-')
		{
			objectiveNum++;
			i++;
			if(gObjectiveCfgStr[i] == '1' && objectiveNum == objective)
			{//completed
				return qfalse;
			}
		}
		i++;
	}

	if(objective > objectiveNum)
	{//this objective doesn't exist
		return qfalse;
	}
	else
	{
		return qtrue;
	}
}


int DOM_FindValidObjective(int objective)
{
	int x = 0;

	//since the game only ever does 6 objectives
	if(objective == -1)
	{ 
		objective = Q_irand(1,6);
	}

	//we assume that the round over check is done before this point
	while(!DOM_ObjectiveStillActive(objective))
	{
		objective--;
		if(objective < 1)
		{
			objective = 6;
		}
	}

	//depandancy checking
	for(x=0; x < MAX_OBJECTIVEDEPENDANCY; x++)
	{
		if(ObjectiveDependancy[objective-1][x])
		{//dependancy
			if(DOM_ObjectiveStillActive(ObjectiveDependancy[objective-1][x]))
			{//a prereq objective hasn't been completed, do that first
				return DOM_FindValidObjective(ObjectiveDependancy[objective-1][x]);
			}
		}
	}

	return objective;
}

/*
//Find the first active objective for this team
int FindFirstActiveObjective(int team)
{
	int i = 0;
	int	curteam = SIEGETEAM_TEAM1;
	int objectiveNum = 0;

	while (gObjectiveCfgStr[i])
	{
		if (gObjectiveCfgStr[i] == '|')
		{ //switch over to team2, this is the next section
            curteam = SIEGETEAM_TEAM2;
			objectiveNum = 0;
		}
		else if (gObjectiveCfgStr[i] == '-')
		{
			objectiveNum++;
			i++;
			if(gObjectiveCfgStr[i] == '0')
			{//completed
				return objectiveNum;
			}
		}
		i++;
	}
	return -1;
}
*/

//determines the trigger entity and type for a seige objective
//attacker = objective attacker or defender? This is used for the recursion stuff.
//use 0 to have it be determined by the side of the info_siege_objective


const char* DOMObjectiveList[] = {
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

int DOMObjectives[MAX_GENTITIES];
int num_DOM_objectives = 0;

int next_DOM_goal_update = -1;

void Update_DOM_Goal_Lists ( void )
{// Create arrays containing up to date valid goal entity numbers...
	int loop = 0;

	// Initialize the max values...
	num_DOM_objectives = 0;

	for (loop = 0;loop < MAX_GENTITIES;loop++)
	{
		int nameNum = 0;

		for (nameNum = 0; DOMObjectiveList[nameNum][0] != '\0'; nameNum++)
		{
			if ( !Q_stricmp( g_entities[loop].classname, DOMObjectiveList[nameNum] ) 
				&& !(g_entities[loop].s.origin[0] == 0 && g_entities[loop].s.origin[1] == 0 && g_entities[loop].s.origin[2] == 0) )
			{
				DOMObjectives[num_DOM_objectives] = loop;
				num_DOM_objectives++;
				break;
			}
		}
	}

	num_DOM_objectives--;

	if (num_DOM_objectives < 0)
		num_DOM_objectives = 0;

#ifdef _DEBUG
	//trap->Print("DOMINATION WAYPOINTING DEBUG: There are currently %i DOM objectives.\n", num_DOM_objectives);
#endif //_DEBUG
}

int DOM_Find_Goal_EntityNum ( int ignoreEnt, int ignoreEnt2, vec3_t current_org, int teamNum )
{// Return standard goals only for soldiers...
	//gentity_t *goal = NULL;
	int loop = 0;

	if (next_DOM_goal_update < level.time)
	{// Update new goal lists.. Unique1
		Update_DOM_Goal_Lists();
		next_DOM_goal_update = level.time + 10000;
	}

	if (num_DOM_objectives > 0)
	{
		/*
		int choice = DOMObjectives[Q_irand(0, num_DOM_objectives)];
	
		if (choice != ignoreEnt && choice != ignoreEnt2)
			return choice;*/

		float	best_dist = 99999.9f;
		int		best_choice = -1;
		int		second_best_choice = -1;

		for ( loop = 0; loop < num_DOM_objectives; loop++)
		{
			int			current_obj = DOMObjectives[Q_irand(0, loop)];
			gentity_t	*test = &g_entities[current_obj];
			float		current_dist = Distance(test->s.origin, current_org);
			
			if ( current_dist > best_dist)
				continue;

			if ( current_dist < 256 )
				continue; // Already at this one! Ignore!

#ifdef __WARZONE__
			if (test->s.eType == ET_FLAG)
			{// Check if it is already ours, skip it if it is!
				if (test->s.teamowner == teamNum && test->s.time2 >= 100)
				{// It's ours, continue looking...
					continue;
				}
			}
#endif //__WARZONE__

			if (!second_best_choice)
				second_best_choice = best_choice;

			best_dist = current_dist;
			best_choice = DOMObjectives[loop];
		}

		if (second_best_choice)
		{
			if (Q_irand(1, 2) == 1)
				return second_best_choice;
			else
				return best_choice;
		}
		else
		{
			return best_choice;
		}
	}

	return -1;
}

int DOM_FindFFAGoal( gentity_t *bot, bot_state_t *bs )
{
	int wp = -1;
	gentity_t *ent = NULL;

	while (!ent)
		ent = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->s.teamowner )];

	if (!ent)
	{
		//trap->Print("Failed to find a goal entity!\n");
		return -1;
	}

	//trap->Print("NPC_FindGoal: Found a goal entity %s (%i).\n", ent->classname, ent->s.number);

	wp = DOM_GetBestWaypoint( ent->r.currentOrigin, ent->s.number, -1 );//DOM_GetNearestWP(ent->s.origin, -1);

	bs->objectiveType = OT_TOUCH;
	bs->tacticEntity = ent;

	return wp;
}


gentity_t * DOM_DetermineObjectiveType(gentity_t *bot, int team, int objective, int *type, int wanted_type, gentity_t *obj, int attacker)
{
	gentity_t *test = NULL;

#ifdef __WARZONE__
	if(g_gametype.integer == GT_WARZONE)
	{//find the flag for this objective type
#ifdef __OLD__
		int tries = 0;
		int found_type = 0;

		if (wanted_type == /*OT_CAPTURE*/OT_TOUCH)
		{
			test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];
			
			while (wanted_type != found_type && tries <= 5)
			{
				test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];
				
				if (test->s.teamowner != team)
					found_type = OT_TOUCH;//OT_CAPTURE;
				else
					found_type = OT_DEFENDCAPTURE;

				tries++;
			}
		}
		else if (wanted_type == OT_DEFENDCAPTURE)
		{
			test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];

			while (wanted_type != found_type && tries <= 5)
			{
				test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];
				
				if (test->s.teamowner != team)
					found_type = OT_TOUCH;//OT_CAPTURE;
				else
					found_type = OT_DEFENDCAPTURE;

				tries++;
			}
		}

		if (tries > 5)
		{// Did not find the type we actually wanted, just pick one randomly and set a new type...
			/*test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];
				
			if (test->s.teamowner != team)
				*type = OT_TOUCH;//OT_CAPTURE;
			else
				*type = OT_DEFENDCAPTURE;*/
			botstates[bot->s.number]->wpDestination = gWPArray[DOM_FindFFAGoal( bot, botstates[bot->s.number] )];
			test = botstates[bot->s.number]->tacticEntity;

			//trap->Print("%s: Warzone wanted objective type not found! Using a random type!\n", bot->client->pers.netname);
		}
		else
		{
			*type = found_type;
			//trap->Print("%s: Warzone wanted objective type set!\n", bot->client->pers.netname);
		}
#else //!__OLD__
		test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];
		
		if (test->s.eType != ET_FLAG)
			*type = OT_TOUCH;
		else if (test->s.teamowner != team)
			*type = OT_TOUCH;//OT_CAPTURE;
		else
			*type = OT_DEFENDCAPTURE;
#endif //__OLD__

		botstates[bot->s.number]->wpDestSwitchTime = level.time + (Q_irand(300, 1200) * 100);

		return test;
	}
	else 
#endif //__WARZONE__
	if(g_gametype.integer == GT_CTF)
	{//find the flag for this objective type
		char *c;
		if(*type == OT_CAPTURE)
		{
			if(team == TEAM_RED)
			{
				c = "team_CTF_blueflag";
			}
			else
			{
				c = "team_CTF_redflag";
			}
		}
		else if(*type == OT_DEFENDCAPTURE)
		{
			if(team == TEAM_RED)
			{
				c = "team_CTF_redflag";
			}
			else
			{
				c = "team_CTF_blueflag";
			}
		}
		else
		{
			trap->Print("DOM_DetermineObjectiveType() Error: Bad ObjectiveType Given for CTF flag find.\n");
			return NULL;
		}
		test = G_Find (test, FOFS(classname), c);
			return test;

	}
	else
	{
		botstates[bot->s.number]->wpDestination = gWPArray[DOM_FindFFAGoal( bot, botstates[bot->s.number] )];
		return botstates[bot->s.number]->tacticEntity;
	}

	if(!obj)
	{//don't already have obj entity to scan from, find it.
		while ( (obj = G_Find( obj, FOFS( classname ), "info_siege_objective" )) != NULL )
		{
			if( objective == obj->objective )
			{//found it
				if(!attacker)
				{//this should always be true
					if(obj->side == team)
					{//we get points from this objective, we're the attacker.
						attacker = 1;
					}
					else
					{//we're the defender
						attacker = 2;
					}
				}
				break;
			}
		}
	}

	/*if (!obj)
	{
		test = &g_entities[DOM_Find_Goal_EntityNum( -1, -1, bot->r.currentOrigin, bot->client->sess.sessionTeam )];

		if (test)
		{
			*type = OT_TOUCH;
			botstates[bot->s.number]->wpDestSwitchTime = level.time + (Q_irand(300, 1200) * 100);
			return test;
		}
	}*/

	if(!obj)
	{//hmmm, couldn't find the thing.  That's not good.
		*type = OT_NONE;
		return NULL;
	}

	//let's try back tracking and figuring out how this trigger is triggered

	//try scanning thru the target triggers first.
	while ( (test = G_Find( test, FOFS( target ), obj->targetname )) != NULL )
	//while ( (test = G_Find( test, FOFS( target4 ), obj->classname )) != NULL )
	{
		if(test->flags & FL_INACTIVE)
		{//this entity isn't active, ignore it
			continue;
		}
		else if(strcmp(test->classname, "func_breakable") == 0)
		{//Destroyable objective
			if(attacker == 1)
			{//attack
				*type = OT_ATTACK;
				return test;
			}
			else if( attacker == 2)
			{//Defend this target
				*type = OT_DEFEND;
				return test;
			}
			else
			{
				trap->Print("Bad attacker state for func_breakable objective in DOM_DetermineObjectiveType().\n");
				return test;
			}
			break;
		}
		else if((strcmp(test->classname, "trigger_multiple") == 0) 
			|| (strcmp(test->classname, "target_relay") == 0)
			|| (strcmp(test->classname, "target_counter") == 0)
			|| (strcmp(test->classname, "func_usable") == 0)
			|| (strcmp(test->classname, "trigger_once") == 0))
		{//ok, you can't do something directly to a trigger_multiple or a target_relay
			//scan for whatever links to this relay
			gentity_t * triggerer = DOM_DetermineObjectiveType(bot, team, objective, type, -1, test, attacker);
			if(triggerer)
			{//success!
				return triggerer;
			}
			else if((strcmp(test->classname, "func_usable") == 0)
				|| (strcmp(test->classname, "trigger_multiple") == 0) 
				|| (strcmp(test->classname, "trigger_once") == 0) )
			{//ok, so they aren't linked to anything, try using them directly then
				if(test->NPC_targetname)
				{//vehicle objective
					if(attacker == 1)
					{//attack
						*type = OT_VEHICLE;
						return test;
					}
					else if( attacker == 2)
					{//destroy the vehicle
						gentity_t *vehicle = NULL;
						//Find the vehicle
						while ( (vehicle = G_Find( vehicle, FOFS( script_targetname ), test->NPC_targetname )) != NULL )
						{
							if (vehicle->inuse && vehicle->client && vehicle->s.eType == ET_NPC &&
								vehicle->s.NPC_class == CLASS_VEHICLE && vehicle->m_pVehicle)
							{
								break;
							}
						}

						if(!vehicle)
						{//can't find the vehicle?!
							*type = OT_WAIT;
							return NULL;
						}

						test = vehicle;
						*type = OT_ATTACK;
						return test;
					}
					else
					{
						trap->Print("Bad attacker state for vehicle trigger_once objective in DOM_DetermineObjectiveType().\n");
						return test;
					}
				}
				else
				{
					if(attacker == 1)
					{//attack
						*type = OT_TOUCH;
						return test;
					}
					else if( attacker == 2)
					{//Defend this target
						*type = OT_DEFEND;
						return test;
					}
					else
					{
						trap->Print("Bad attacker state for func_usable objective in DOM_DetermineObjectiveType().\n");
						return test;
					}
				}
				break;
			}
		}
	}

	test = NULL;

	//ok, see obj is triggered by the goaltarget of a capturable misc_siege_item 
	while ( (test = G_Find( test, FOFS( goaltarget ), obj->targetname )) != NULL )
	{
		if(strcmp(test->classname, "misc_siege_item") == 0)
		{//Destroyable objective
			if(attacker == 1)
			{//attack
				*type = OT_CAPTURE;
				return test;
			}
			else if (attacker == 2)
			{//Defend this target
				*type = OT_DEFENDCAPTURE;
				return test;
			}
			else
			{
				trap->Print("Bad attacker state for misc_siege_item objective in DOM_DetermineObjectiveType().\n");
				return test;
			}
			break;
		}
	}

	test = NULL;

	//ok, see obj is triggered by the target3 (delivery target) of a capturable misc_siege_item 
	while ( (test = G_Find( test, FOFS( target3 ), obj->targetname )) != NULL )
	{
		if(strcmp(test->classname, "misc_siege_item") == 0)
		{//capturable objective
			if(attacker == 1)
			{//attack
				*type = OT_CAPTURE;
				return test;
			}
			else if (attacker == 2)
			{//Defend this target
				*type = OT_DEFENDCAPTURE;
				return test;
			}
			else
			{
				trap->Print("Bad attacker state for misc_siege_item (target3) objective in DOM_DetermineObjectiveType().\n");
				return test;
			}
			break;
		}
	}

	test = NULL;

	//check for a destroyable misc_siege_item that triggers this objective
	while ( (test = G_Find( test, FOFS( target4 ), obj->targetname )) != NULL )
	{
		if(strcmp(test->classname, "misc_siege_item") == 0)
		{
			if(attacker == 1)
			{//attack
				*type = OT_ATTACK;
				return test;
			}
			else if( attacker == 2)
			{//Defend this target
				*type = OT_DEFEND;
				return test;
			}
			else
			{
				trap->Print("Bad attacker state for misc_siege_item (target4) objective in DOM_DetermineObjectiveType().\n");
				return test;
			}
			break;
		}
	}

	//no dice
	*type = OT_NONE;
	return NULL;
}


//use/touch the given objective
void DOM_objectiveType_Touch(bot_state_t *bs)
{
	vec3_t objOrigin;

	DOM_FindOrigin(bs->tacticEntity, objOrigin);

	if(!G_PointInBounds( bs->origin, bs->tacticEntity->r.absmin, bs->tacticEntity->r.absmax ))
	{//move closer
		VectorCopy(objOrigin, bs->DestPosition);
		bs->DestIgnore = bs->tacticEntity->s.number;
		if(bs->currentEnemy)
		{//have a local enemy, attackmove
			DOM_BotBehave_AttackMove(bs);
		}
		else
		{//normal move
			bs->botBehave = BBEHAVE_MOVETO;
		}	
	}
	else
	{//in range hold down use
		bs->useTime = level.time + 100;
		if(bs->tacticEntity->spawnflags & 2 /*FACING*/)
		{//you have to face in the direction of the trigger to have it work
			vec3_t ang;
			vectoangles(bs->tacticEntity->movedir, ang);
			ang[PITCH] = ang[ROLL] = 0;
			VectorCopy(ang, bs->goalAngles);
		}
	}
}

	
//Basically this is the tactical order for attacking an object with a known location
void DOM_objectiveType_Attack(bot_state_t *bs, gentity_t *target)
{
	vec3_t objOrigin;
	vec3_t a;
	trace_t tr;
	float dist;

	DOM_FindOrigin(target, objOrigin);

	//Do visual check to target
	VectorSubtract(objOrigin, bs->eye, a);
	dist = DOM_TargetDistance(bs, target, objOrigin);
	vectoangles(a, a);
	a[PITCH] = a[ROLL] = 0;

	trap->Trace(&tr, bs->eye, NULL, NULL, objOrigin, bs->client, MASK_PLAYERSOLID, 0, 0, 0);

	if (((tr.entityNum == target->s.number || tr.fraction == 1)
		&& (InFieldOfVision(bs->viewangles, 90, a) || bs->cur_ps.groundEntityNum == target->s.number)
		&& !BotMindTricked(bs->client, target->s.number)) 
		|| BotCanHear(bs, target, dist) || dist < 100)
	{//we see the objective, go for it.
		int desiredweap = DOM_FavoriteWeapon(bs, target);
		if((target->flags & FL_DMG_BY_HEAVY_WEAP_ONLY) && !DOM_IsHeavyWeapon(bs, desiredweap))
		{//we currently don't have a heavy weap that we can use to destroy this target
			if(DOM_HaveHeavyWeapon(bs->cur_ps.stats[STAT_WEAPONS]))
			{//we have a weapon that could destroy this target but we don't have ammo
				//RAFIXME:  at this point we should have the bot go look for some ammo
				//but for now just defend this area.
				DOM_BotDefend(bs, target);
			}
			else
			{//go hunting for a weapon that can destroy this object
				//RAFIXME:  Add this code
				DOM_BotDefend(bs, target);
			}
		}
		else if((target->flags & FL_DMG_BY_SABER_ONLY) && !(bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_SABER)) )
		{//This is only damaged by sabers and we don't have a saber
			DOM_BotDefend(bs, target);
		}
		else
		{//cleared to attack
			bs->frame_Enemy_Len = dist;
			bs->frame_Enemy_Vis = 1;
			bs->currentEnemy = target;
			VectorCopy(objOrigin, bs->lastEnemySpotted);
			DOM_FindAngles(target, bs->lastEnemyAngles);
			bs->enemySeenTime = level.time + BOT_VISUALLOSETRACKTIME;
			DOM_BotBehave_Attack(bs);
		}
		return;
	}
	else if(bs->currentEnemy == target)
	{//can't see the target so null it out so we can find other enemies.
		bs->currentEnemy = NULL;
		bs->frame_Enemy_Vis = 0;
		bs->frame_Enemy_Len = 0;
	}

	if(strcmp(target->classname, "func_breakable") == 0 
		&& strcmp(target->paintarget, "shieldgen_underattack") == 0)
	{//dirty hack to get the bots to attack the shield generator on siege_hoth
		vec3_t temp;
		VectorSet(temp, -369, 858, -231);
		if(Distance(bs->origin, temp) < DEFEND_MAXDISTANCE)
		{//automatically see target.
			//int desiredweap = DOM_FavoriteWeapon(bs, target);
			if(!(bs->cur_ps.stats[STAT_WEAPONS] & ( 1 << WP_DEMP2))
				&& !(bs->cur_ps.stats[STAT_WEAPONS] & ( 1 << WP_ROCKET_LAUNCHER ))
				&& !(bs->cur_ps.stats[STAT_WEAPONS] & ( 1 << WP_CONCUSSION ))
				&& !(bs->cur_ps.stats[STAT_WEAPONS] & ( 1 << WP_REPEATER )))
			{//we currently don't have a heavy weap that can reach this target
				DOM_BotDefend(bs, target);
			}
			else
			{//cleared to attack
				bs->frame_Enemy_Len = dist;
				bs->frame_Enemy_Vis = 1;
				bs->currentEnemy = target;
				VectorCopy(objOrigin, bs->lastEnemySpotted);
				DOM_FindAngles(target, bs->lastEnemyAngles);
				bs->enemySeenTime = level.time + BOT_VISUALLOSETRACKTIME;
				DOM_BotBehave_Attack(bs);
			}
			return;
		}
		VectorCopy(temp, objOrigin);
	}

	//ok, we can't see the objective, move towards its location
	VectorCopy(objOrigin, bs->DestPosition);
	bs->DestIgnore = target->s.number;
	if(bs->currentEnemy)
	{//have a local enemy, attackmove
		DOM_BotBehave_AttackMove(bs);
	}
	else
	{//normal move
		bs->botBehave = BBEHAVE_MOVETO;
	}	
}


int DOM_FlagColorforObjective(bot_state_t *bs)
{
	if(g_entities[bs->client].client->sess.sessionTeam == TEAM_RED) 
	{
		if(bs->objectiveType == OT_CAPTURE)
		{
			return PW_BLUEFLAG;
		}
		else
		{
			return PW_REDFLAG;
		}
	}
	else
	{
		if(bs->objectiveType == OT_CAPTURE)
		{
			return PW_REDFLAG;
		}
		else
		{
			return PW_BLUEFLAG;
		}
	}
}

qboolean DOM_CapObjectiveIsCarried(bot_state_t *bs)
{//check to see if the current objective capture item is being carried
	int flagpr, i;
	gentity_t *carrier;

	//Set which flag powerup we're looking for
	flagpr = DOM_FlagColorforObjective(bs);

	// check for carrier on desired flag
	for (i = 0; i < sv_maxclients.integer; i++) 
	{
		carrier = g_entities + i;
		if (carrier->inuse && carrier->client->ps.powerups[flagpr])
			return qtrue;
	}
			
	return qfalse;
}


gentity_t *DOM_CapObjectiveCarrier(bot_state_t *bs)
{//Returns the gentity for the current carrier of the capture objective
	int flagpr, i;
	gentity_t *carrier;

	//Set which flag powerup we're looking for
	flagpr = DOM_FlagColorforObjective(bs);

	// find attacker's team's flag carrier
	for (i = 0; i < sv_maxclients.integer; i++) 
	{
		carrier = g_entities + i;
		if (carrier->inuse && carrier->client->ps.powerups[flagpr])
			return carrier;
	}

	trap->Print("DOM_CapObjectiveCarrier() Error: Couldn't find carrier entity.\n");
	return NULL;
}


qboolean DOM_CarryingCapObjective(bot_state_t *bs)
{//Carrying the Capture Objective?
	if(g_entities[bs->client].client->ps.powerups[PW_REDFLAG] 
	|| g_entities[bs->client].client->ps.powerups[PW_BLUEFLAG])
		return qtrue;
	return qfalse;
}


gentity_t *DOM_FindGoalPointEnt( bot_state_t *bs )
{//Find the goalpoint entity for this capture objective point
	char *c;
	if(g_entities[bs->client].client->sess.sessionTeam == TEAM_RED)
	{
		c = "team_CTF_redflag";
	}
	else
	{
		c = "team_CTF_blueflag";
	}
	return G_Find( NULL, FOFS( classname ), c );
}


void DOM_objectiveType_Capture(bot_state_t *bs)
{
	if(DOM_CapObjectiveIsCarried(bs))
	{//objective already being carried
		if(DOM_CarryingCapObjective(bs))
		{//I'm carrying the flag.
			//find the goaltarget
			gentity_t *goal = NULL;
			goal = DOM_FindGoalPointEnt(bs);
			if(goal)
			{
				vec3_t goalorigin;
				DOM_FindOrigin(goal, goalorigin);
				VectorCopy(goalorigin, bs->DestPosition);
				bs->DestIgnore = goal->s.number;
				if(bs->currentEnemy)
				{
					DOM_BotBehave_AttackMove(bs);
				}
				else
				{
					bs->botBehave = BBEHAVE_MOVETO;
				}
			}
			return;
		}
		else
		{//someone else is covering the flag, cover them
			DOM_BotDefend(bs, DOM_CapObjectiveCarrier(bs));
			return;
		}
	}
	else
	{//not being carried
		//get the flag!
		vec3_t origin;
		DOM_FindOrigin(bs->tacticEntity, origin);
		VectorCopy(origin, bs->DestPosition);
		bs->DestIgnore = bs->tacticEntity->s.number;
		if(bs->currentEnemy)
		{
			DOM_BotBehave_AttackMove(bs);
		}
		else
		{
			bs->botBehave = BBEHAVE_MOVETO;
		}
		return;
	}
}

void DOM_objectiveType_Warzone_Capture(bot_state_t *bs)
{
	//get the flag!
	/*vec3_t origin;
	DOM_FindOrigin(bs->tacticEntity, origin);
	VectorCopy(origin, bs->DestPosition);
	bs->DestIgnore = bs->tacticEntity->s.number;
	if(bs->currentEnemy)
	{
		DOM_BotBehave_AttackMove(bs);
	}
	else if (g_entities[bs->client].client->ps.stats[STAT_CAPTURE_ENTITYNUM] == bs->tacticEntity->s.number)
	{
		bs->tacticEntity = NULL;
		bs->wpDestination = NULL;
		bs->wpLast = NULL;
		bs->wpCurrent = NULL;
	}
	else if (Distance(bs->tacticEntity->r.currentOrigin, bs->origin) < 16)
	{
		bs->tacticEntity = NULL;
		bs->wpDestination = NULL;
		bs->wpLast = NULL;
		bs->wpCurrent = NULL;
	}
	else
	{
		bs->botBehave = BBEHAVE_MOVETO;
	}
	return;*/
}

extern gentity_t *droppedBlueFlag;
extern gentity_t *droppedRedFlag;
gentity_t *DOM_FindFlag(bot_state_t *bs)
{//find the flag item entity for this bot's objective entity
	if(DOM_FlagColorforObjective(bs) == PW_BLUEFLAG)
	{//blue flag
		return droppedBlueFlag;
	}
	else
	{
		return droppedRedFlag;
	}

	//bad flag?!
	return NULL;
}

	
//Prevent this objective from getting captured.
void DOM_objectiveType_DefendCapture(bot_state_t *bs)
{
	if(!DOM_CapObjectiveIsCarried(bs))
	{
		gentity_t *flag = DOM_FindFlag(bs);
		if(flag && flag->flags & FL_DROPPED_ITEM)
		{//dropped, touch it
			vec3_t origin;
			DOM_FindOrigin(flag, origin);
			VectorCopy(origin, bs->DestPosition);
			bs->DestIgnore = flag->s.number;
			if(bs->currentEnemy)
			{
				DOM_BotBehave_AttackMove(bs);
			}
			else
			{
				bs->botBehave = BBEHAVE_MOVETO;
			}
		}
		else
		{//objective at homebase, defend it
			DOM_BotDefend(bs, bs->tacticEntity);
		}
	}
	else
	{//object has been taken, attack the carrier
		DOM_objectiveType_Attack(bs, DOM_CapObjectiveCarrier(bs));
	}
}

//Prevent this objective from getting captured.
void DOM_objectiveType_Warzone_DefendCapture(bot_state_t *bs)
{
	if(bs->currentEnemy)
	{
		DOM_BotBehave_AttackMove(bs);
	}
	else
	{
		DOM_BotDefend(bs, bs->tacticEntity);
	}
}

//vehicle
void DOM_objectiveType_Vehicle(bot_state_t *bs)
{
	gentity_t *vehicle = NULL;
	gentity_t *botEnt = &g_entities[bs->client];

	//find the vehicle that must trigger this trigger.
	while ( (vehicle = G_Find( vehicle, FOFS( script_targetname ), bs->tacticEntity->NPC_targetname )) != NULL )
	{
		if (vehicle->inuse && vehicle->client && vehicle->s.eType == ET_NPC &&
			vehicle->s.NPC_class == CLASS_VEHICLE && vehicle->m_pVehicle)
		{
			break;
		}
	}

	if(!vehicle)
	{//can't find the vehicle?!
		return;
	}

	if (botEnt->inuse && botEnt->client 
			&& botEnt->client->ps.m_iVehicleNum == vehicle->s.number)
	{//in the vehicle
		//move towards trigger point
		vec3_t objOrigin;
		DOM_FindOrigin(bs->tacticEntity, objOrigin);

		//RAFIXME:  Get rid of that crappy use stuff when we can.
		bs->noUseTime += level.time + 5000;

		bs->DestIgnore = bs->tacticEntity->s.number;
		DOM_BotMove(bs, objOrigin, qfalse, qfalse);
	}
	else if(vehicle->client->ps.m_iVehicleNum)
	{//vehicle already occuped, cover it.
		DOM_BotDefend(bs, vehicle);
	}
	else
	{//go to the vehicle!
		//hack!
		vec3_t vehOrigin;
		DOM_FindOrigin(vehicle, vehOrigin);

		//bs->useTime = level.time + 100;
				
		bs->botBehave = BBEHAVE_MOVETO;
		VectorCopy(vehOrigin, bs->DestPosition);		
		bs->DestIgnore = vehicle->s.number;
	}	
}


//Siege Objective attack/defend
void DOM_BotObjective(bot_state_t *bs)
{
	if(!bs->objectiveType || !bs->tacticEntity || bs->tacticEntity->s.number < MAX_CLIENTS
		|| strcmp(bs->tacticEntity->classname, "freed") == 0 || bs->wpDestSwitchTime < level.time)
	{//don't have objective entity type, don't have tacticEntity, or the tacticEntity you had
		//was killed/freed
#ifdef __WARZONE__
		if (g_gametype.integer == GT_WARZONE)
			DOM_PickWARZONEGoalType(bs);
#endif //__WARZONE__

		bs->tacticEntity = DOM_DetermineObjectiveType(&g_entities[bs->client], g_entities[bs->client].client->sess.sessionTeam, 
			bs->tacticObjective, &bs->objectiveType, bs->objectiveType, NULL, 0);

		if (!bs->tacticEntity)
			return;

		{
			//destination WP
			int destwp = DOM_GetBestWaypoint(bs->tacticEntity->r.currentOrigin, bs->client, -1);
	
			if( destwp == -1 )
			{//crap, this map has no wps.  try just autonaving it then
				DOM_BotMove(bs, bs->tacticEntity->r.currentOrigin, qfalse, qfalse);
				return;
			}

			bs->wpDestination = gWPArray[destwp];
		}
	}

	if(bs->objectiveType == OT_ATTACK)
	{
		//attack tactical code
		DOM_objectiveType_Attack(bs, bs->tacticEntity);
	}
	else if(bs->objectiveType == OT_DEFEND)
	{//defend tactical code
		DOM_BotDefend(bs, bs->tacticEntity);
	}
	else if(bs->objectiveType == OT_CAPTURE)
	{//capture tactical code
#ifdef __WARZONE__
		if (g_gametype.integer == GT_WARZONE)
			DOM_objectiveType_Warzone_Capture(bs);
		else
#endif //__WARZONE__
			DOM_objectiveType_Capture(bs);
	}
	else if(bs->objectiveType == OT_DEFENDCAPTURE)
	{//defend capture tactical
#ifdef __WARZONE__
		if (g_gametype.integer == GT_WARZONE)
			DOM_objectiveType_Warzone_DefendCapture(bs);
		else
#endif //__WARZONE__
			DOM_objectiveType_DefendCapture(bs);
	}
	else if(bs->objectiveType == OT_TOUCH)
	{//touch tactical
#ifdef __WARZONE__
		if (g_gametype.integer == GT_WARZONE)
			DOM_objectiveType_Warzone_Capture(bs);
		else
#endif //__WARZONE__
		DOM_objectiveType_Touch(bs);
	}
	else if(bs->objectiveType == OT_VEHICLE)
	{//vehicle techical
		DOM_objectiveType_Vehicle(bs);
	}
	else if(bs->objectiveType == OT_WAIT)
	{//just run around and attack people, since we're waiting for the objective to become valid.
		DOM_BotSearchAndDestroy(bs);
	}
	else
	{
		trap->Print("Bad/Unknown ObjectiveType in DOM_BotObjective.\n");
	}
}


//Find the origin location of a given entity
void DOM_FindOrigin(gentity_t *ent, vec3_t origin)
{
	if(!ent->classname)
	{//some funky entity, just set vec3_origin
		VectorCopy(vec3_origin, origin);
		return;
	}

	if(ent->client)
	{
		VectorCopy(ent->client->ps.origin, origin);
	}
	else
	{
		if(strcmp(ent->classname, "func_breakable") == 0 
			|| strcmp(ent->classname, "trigger_multiple") == 0
			|| strcmp(ent->classname, "trigger_once") == 0
			|| strcmp(ent->classname, "func_usable") == 0)
		{//func_breakable's don't have normal origin data
			origin[0] = (ent->r.absmax[0]+ent->r.absmin[0])/2;
			origin[1] = (ent->r.absmax[1]+ent->r.absmin[1])/2;
			origin[2] = (ent->r.absmax[2]+ent->r.absmin[2])/2;
		}
		else
		{
			VectorCopy(ent->r.currentOrigin, origin);
		}
	}

}


//find angles/viewangles for entity
void DOM_FindAngles(gentity_t *ent, vec3_t angles)
{
	if(ent->client)
	{//player
		VectorCopy(ent->client->ps.viewangles, angles);
		angles[PITCH] = angles[ROLL] = 0;
	}
	else
	{//other stuff
		VectorCopy(ent->s.angles, angles);
		angles[PITCH] = angles[ROLL] = 0;
	}
}


int DOM_BotWeapon_Detpack(bot_state_t *bs, gentity_t *target)
{
	gentity_t *dp = NULL;
	gentity_t *bestDet = NULL;
	vec3_t TargOrigin;
	float bestDistance = 9999;
	float tempDist;

	DOM_FindOrigin(target, TargOrigin);

	while ( (dp = G_Find( dp, FOFS(classname), "detpack") ) != NULL )
	{
		if (dp && dp->parent && dp->parent->s.number == bs->client)
		{
			tempDist = Distance(TargOrigin, dp->s.pos.trBase);
			if(tempDist < bestDistance)
			{
				bestDistance = tempDist;
				bestDet = dp;
			}

			/*
			//check to make sure the det isn't too close to the bot.
			if(Distance(bs->origin, dp->s.pos.trBase) < DETPACK_DETDISTANCE)
			{//we're too close!
				return qfalse;
			}
			*/
		}
	}

	if (!bestDet || bestDistance > DETPACK_DETDISTANCE)
	{
		return qfalse;
	}

	//check to see if the bot knows that the det is near the target.

	//found the closest det to the target and it is in blow distance.
	if(WP_DET_PACK != bs->cur_ps.weapon)
	{//need to switch to desired weapon
		BotSelectChoiceWeapon(bs, WP_DET_PACK, qtrue);
	}
	else
	{//blast it!
		bs->doAltAttack = qtrue;
	}

	return qtrue;
}

/* */
void
AIMod_CheckObjectivePaths ( gentity_t *ent )
{
	int				pathlist[MAX_WPARRAY_SIZE];
	int				pathsize;
	int				i;
	int				wp = DOM_GetBestWaypoint( ent->r.currentOrigin, ent->s.number, -1 );
	int				goal_wp;
	wpobject_t		*my_wp = NULL;
	wpobject_t		*my_wp_goal = NULL;

	if (wp < 0)
	{
		trap->Print( "No routes - No waypoint was found at your current position!\n");
		return;
	}

	PATHING_IGNORE_FRAME_TIME = qtrue;

	my_wp = gWPArray[wp];

	trap->Print( "Finding bot objectives for %s at node number %i (%f %f %f).\n", ent->client->pers.netname,
		my_wp->index, my_wp->origin[0], my_wp->origin[1], my_wp->origin[2] );

	ShowLinkInfo(wp, ent);

	Update_DOM_Goal_Lists();

	for ( i = 0; i < num_DOM_objectives; i++ )
	{
		gentity_t	*goal = &g_entities[DOMObjectives[i]];
		//ent->longTermGoal = AIMOD_NAVIGATION_FindClosestReachableNode( goal, NODE_DENSITY, NODEFIND_ALL, ent );
		goal_wp = DOM_GetBestWaypoint( goal->r.currentOrigin, goal->s.number, -1 );;//DOM_GetNearestWP( goal->r.currentOrigin, -1 );
		my_wp_goal = NULL;

		if (goal_wp < 0)
			continue;

		my_wp_goal = gWPArray[goal_wp];

		pathsize = ASTAR_FindPathFast(wp, goal_wp, pathlist, qtrue);

		//if (pathsize <= 0) // Alt A* Pathing...
		//	pathsize = DOM_FindIdealPathtoWP(NULL, wp, goal_wp, -1, pathlist);

		trap->Print( "Objective %i (%s) pathsize is %i.\n", i, goal->classname, pathsize );
	}

	trap->Print( "Complete.\n" );

	PATHING_IGNORE_FRAME_TIME = qfalse;
}

void
AIMod_CheckMapPaths ( gentity_t *ent )
{
	/*short*/ int	pathlist[MAX_WPARRAY_SIZE];
	int			pathsize;
	int			i;
	int			current_wp, longTermGoal;

	current_wp = DOM_GetBestWaypoint(ent->r.currentOrigin, -1, -1);

	if (!current_wp)
	{
		trap->Print("No waypoint found!\n");
		return;
	}

	PATHING_IGNORE_FRAME_TIME = qtrue;

	trap->Print( "Finding bot objectives for %s at node number %i (%f %f %f).\n", ent->client->pers.netname,
				 current_wp, gWPArray[current_wp]->origin[0], gWPArray[current_wp]->origin[1],
				 gWPArray[current_wp]->origin[2] );

	ShowLinkInfo(current_wp, ent);

	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		gentity_t	*goal = &g_entities[i];

		if (!goal) continue;
		
		if (!goal->classname 
			|| !goal->classname[0] 
			|| !Q_stricmp(goal->classname, "freed")
			|| !Q_stricmp(goal->classname, "noclass")) 
				continue;
		
		if (i == ent->s.number) continue;

		longTermGoal = DOM_GetBestWaypoint(goal->s.origin, -1, -1);

		pathsize = ASTAR_FindPathFast(current_wp, longTermGoal, pathlist, qtrue);

		//if (pathsize <= 0) // Alt A* Pathing...
		//	pathsize = DOM_FindIdealPathtoWP(NULL, current_wp, longTermGoal, -1, pathlist);

		trap->Print( "Objective %i (%s) pathsize is %i.\n", i, goal->classname, pathsize );
	}

	trap->Print( "Complete.\n" );

	PATHING_IGNORE_FRAME_TIME = qfalse;
}

