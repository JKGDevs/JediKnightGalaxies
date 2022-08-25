//includes
#include "g_local.h"
#include "ai_main.h"

//externs
extern bot_state_t	*botstates[MAX_CLIENTS];
//Local Variables
extern gentity_t		*NPC;
extern gNPC_t			*NPCInfo;
extern gclient_t		*client;
extern usercmd_t		ucmd;
extern visibility_t		enemyVisibility;

extern gNPC_t *New_NPC_t(int entNum);
extern qboolean NPC_ParseParms( const char *NPCName, gentity_t *NPC );
extern void NPC_DefaultScriptFlags( gentity_t *ent );
extern void NPC_Think ( gentity_t *self);
extern void GLua_NPCEV_OnThink(gentity_t *self);
extern qboolean NPC_UpdateAngles ( qboolean doPitch, qboolean doYaw );
extern void NPC_Begin (gentity_t *ent);
extern void NPC_ExecuteBState ( gentity_t *self);
extern void SetNPCGlobals( gentity_t *ent );

void DOM_InitFakeNPC(gentity_t *bot)
{
	bot->NPC = New_NPC_t(bot->s.number);

	//Assign the pointer for bg entity access
	bot->playerState = &bot->client->ps;

	bot->NPC_type = G_NewString( "reborn" );

	//bot->flags |= FL_NO_KNOCKBACK;//don't fall off ledges

	bot->client->ps.weapon = WP_SABER;//init for later check in NPC_Begin
	bot->client->ps.weaponVariation = 0;

	NPC_ParseParms( bot->NPC_type, bot );

	NPC_DefaultScriptFlags( bot );

	NPC_Begin(bot);
	bot->s.eType = ET_PLAYER;

	// UQ1: Mark every NPC's spawn position. For patrolling that spot and stuff...
	VectorCopy(bot->r.currentOrigin, bot->spawn_pos);
	VectorCopy(bot->r.currentOrigin, bot->spawn_pos);

	// Init patrol range...
	if (bot->patrol_range <= 0) bot->patrol_range = 512.0f;

	// Init waypoints...
	bot->wpCurrent = -1;
	bot->wpNext = -1;
	bot->wpLast = -1;
	bot->longTermGoal = -1;

	// Init enemy...
	bot->enemy = NULL;

	bot->client->NPC_class = CLASS_BOT_FAKE_NPC;

	bot->client->playerTeam = NPCTEAM_ENEMY;
}

vec3_t oldMoveDir;

// UQ1: Now lets see if bots can share NPC AI....
void DOM_StandardBotAI2(bot_state_t *bs, float thinktime)
{
	gentity_t *bot = &g_entities[bs->client];

	NPC = bot;
	client = NPC->client;
	NPCInfo = bot->NPC;
	ucmd = NPC->client->pers.cmd;

	if (!bot->NPC)
		DOM_InitFakeNPC(bot);

	SetNPCGlobals( bot );

	memset( &ucmd, 0, sizeof( ucmd ) );

	if (bot->health < 1 || bot->client->ps.pm_type == PM_DEAD)
	{
		//RACC - Try to respawn if you're done talking.
		if (rand()%10 < 5 &&
			(!bs->doChat || bs->chatTime < level.time))
		{
			trap->EA_Attack(bs->client);
		}

		return;
	}

	// If it's a lua NPC, run the OnThink event
	if (bot->NPC->isLuaNPC) {
		GLua_NPCEV_OnThink(bot);
	}

	VectorCopy( oldMoveDir, bot->client->ps.moveDir );
	//or use client->pers.lastCommand?

	NPCInfo->last_ucmd.serverTime = level.time - 50;

	//nextthink is set before this so something in here can override it
	NPC_ExecuteBState(bot);

	//NPC_UpdateAngles(qtrue, qtrue);
	//memcpy( &ucmd, &NPCInfo->last_ucmd, sizeof( usercmd_t ) );
	//ClientThink(bot->s.number, &ucmd);

	trap->ICARUS_MaintainTaskManager(bot->s.number);
	VectorCopy(bot->r.currentOrigin, bot->client->ps.origin);

	if (bot->client->ps.pm_flags & PMF_DUCKED && bot->r.maxs[2] > bot->client->ps.crouchheight)
	{
		bot->r.maxs[2] = bot->client->ps.crouchheight;
		bot->r.maxs[1] = 8;
		bot->r.maxs[0] = 8;
		bot->r.mins[1] = -8;
		bot->r.mins[0] = -8;
	}
	else if (!(bot->client->ps.pm_flags & PMF_DUCKED) && bot->r.maxs[2] < bot->client->ps.standheight)
	{
		bot->r.maxs[2] = bot->client->ps.standheight;
		bot->r.maxs[1] = 10;
		bot->r.maxs[0] = 10;
		bot->r.mins[1] = -10;
		bot->r.mins[0] = -10;
	}

	trap->LinkEntity((sharedEntity_t *)bot);
}
