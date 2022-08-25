// Defines the npc object

#include "game/g_local.h"
#include "qcommon/q_shared.h"

#include "glua.h"

#define ConvertVec(a, b) (b[0] = a->x, b[1] = a->y, b[2] = a->z)

gentity_t *GLua_CheckNPC(lua_State *L, int idx) {
	GLua_Data_Entity_t *npc;
	gentity_t *ent;
	if (!ValidateObject(L, idx, GO_NPC)) {
		luaL_typerror(L, idx, "NPC");
	}
	npc = (GLua_Data_Entity_t *)lua_touserdata(L,idx);
	ent = &g_entities[npc->entNum];

	if (!ent->inuse && !ent->LuaUsable)
		return NULL;
	if (ent->s.eType != ET_NPC)
		return NULL;
	if (ent->IDCode != npc->IDCode)
		return NULL;
	if (!ent->client)	// Safety check, since most functions will access client, ensure it's set
		return NULL;
	return ent;
}

static int GLua_NPC_GetIndex(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	lua_pushnumber(L,npc->s.number);
	return 1;
}

static int GLua_NPC_IsValid(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) {
		lua_pushboolean(L,0);
	} else {
		lua_pushboolean(L,1);
	}
	return 1;
}

static int GLua_NPC_GetTable(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	GLua_Push_GetEntDataTableFunc(L);
	GLua_PushEntity(L, npc);
	lua_call(L,1,1);
	return 1;
}

static int GLua_NPC_Index(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	const char *key = lua_tostring(L,2);
	// Continue even if the npc is invalid, we'll check that later

	if (key[0] == '_' && key[1] == '_') {
		if (npc) {
			goto skipmeta;
		} else {
			return 0;
		}
	}

	lua_getmetatable(L,1);
	lua_getfield(L,-1,key);
	if (!lua_isnoneornil(L,-1)) {
		return 1;
	}
	// Its not a method, access the table and check for this entry
	// That is.. if the npc is valid :P
	if (!npc) return 1; // If we get here, there's a nil on top of the stack, so ya :P
	
	// Check properties
	lua_getfield(L, -2, "__propget");
	if (!lua_isnoneornil(L,-1)) {
		// Check properties
		lua_getfield(L,-1,key);
		if (lua_isnoneornil(L,-1)) {
			// Not defined
			lua_pop(L,2);
		} else if (!lua_isfunction(L,-1)) {
			// Write-only property
			lua_Debug ar;
			memset(&ar, 0, sizeof(lua_Debug));
			lua_getstack(L, 1, &ar);
			lua_getinfo(L, "Sl", &ar);
			Com_Printf("Warning: %s:%i: Attempted to get write-only property '%s' in %s", ar.short_src, ar.currentline, key, lua_tostring(L,1));
			return 0;
		} else {
			lua_pushvalue(L,1);	// Push object
			//lua_pushvalue(L,2);	// Push key
			lua_call(L, 1, 1);
			return 1;
		}
	} else {
		lua_pop(L,1);
	}
	lua_pop(L,2);

skipmeta:

	GLua_Push_GetEntDataFunc(L);
	GLua_PushEntity(L, npc);
	lua_pushvalue(L,2);
	lua_call(L,2,1);
	return 1;
}

static int GLua_NPC_NewIndex(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	const char *key = lua_tostring(L,2);
	if (!npc) return 0;

	lua_getmetatable(L,1);

	// Check properties
	lua_getfield(L, -1, "__propset");
	if (!lua_isnoneornil(L,-1)) {
		// Check properties
		lua_getfield(L,-1,key);
		if (lua_isnoneornil(L,-1)) {
			// Not defined
			lua_pop(L,2);
		} else if (!lua_isfunction(L,-1)) {
			// Read-only property
			lua_Debug ar;
			memset(&ar, 0, sizeof(lua_Debug));
			lua_getstack(L, 1, &ar);
			lua_getinfo(L, "Sl", &ar);
			Com_Printf("Warning: %s:%i: Attempted to set read-only property '%s' in %s", ar.short_src, ar.currentline, key, lua_tostring(L,1));
			return 0;
		} else {
			lua_pushvalue(L,1);	// Push object
			//lua_pushvalue(L,2);	// Push key
			lua_pushvalue(L,3);	// Push value
			lua_call(L, 2, 1);
			return 1;
		}
	} else {
		lua_pop(L,1);
	}

	lua_pop(L,1);

	GLua_Push_SetEntDataFunc(L);
	GLua_PushEntity(L, npc);
	lua_pushvalue(L,2);
	lua_pushvalue(L,3);
	lua_call(L,3,0);
	return 0;
}

static int GLua_NPC_Eq(lua_State *L) {
	gentity_t *npc1 = GLua_CheckNPC(L, 1);
	gentity_t *npc2 = GLua_CheckNPC(L, 2);

	if ((npc1 && npc2) && npc1 == npc2) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_NPC_Kill(lua_State *L)
{
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int health;

	if (!npc) return 0; //template :P

	//If the NPC is dead, dont bother
	if (npc->health <= 0)
		return 0;

	//override godmode flag.
	npc->flags &= ~FL_GODMODE;
	if ( npc->client )
	{
		npc->flags |= FL_NO_KNOCKBACK;
	}
	//assign new health value.
	health = npc->health;
	npc->health = 0;

	//final step.
	npc->die(npc, npc, npc, health, MOD_UNKNOWN);

	return 0;
}

static int GLua_NPC_MakeVendor(lua_State *L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	if (npc->health <= 0)
		return 0;

	const char* tc = luaL_checkstring(L, 2);
	JKG_MakeNPCVendor(npc, (char*)tc);

	return 0;
}

static int GLua_NPC_UnmakeVendor(lua_State* L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	JKG_UnmakeNPCVendor(npc);

	return 0;
}

static int GLua_NPC_RefreshStock(lua_State *L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	if (npc->health <= 0)
		return 0;

	JKG_RegenerateStock(npc);

	return 0;
}

static int GLua_NPC_UseVendor(lua_State *L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 2);
	
	if (!npc) return 0;
	if (!ply) return 0;

	if (npc->health <= 0)
		return 0;

	gentity_t* player;
	player = &g_entities[ply->clientNum];

	JKG_target_vendor_use(npc, player, player);
	return 1;
}

static int GLua_NPC_GetPos(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	vec3_t orig;
	if (!npc) return 0;
	
	VectorCopy(npc->client->ps.origin, orig);
	GLua_PushVector(L, orig[0], orig[1], orig[2]);
	return 1;
}

static int GLua_NPC_SetPos(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	GLuaVec_t * orig = GLua_CheckVector(L, 2);
	vec3_t neworigin;
	if (!npc) return 0;
	memcpy(&neworigin, orig, 12);

	VectorCopy(neworigin, npc->client->ps.origin);
	VectorCopy(neworigin, npc->r.currentOrigin);
	npc->client->ps.origin[2] += 1;

	VectorClear (npc->client->ps.velocity);
	npc->client->ps.pm_time = 160;		// hold time
	npc->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	
	npc->client->ps.eFlags ^= EF_TELEPORT_BIT;
	return 0;
}

static int GLua_NPC_GetAngles(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;

	GLua_PushVector(L, npc->client->ps.viewangles[0], npc->client->ps.viewangles[1], npc->client->ps.viewangles[2]);
	return 1;
}

static int GLua_NPC_SetAngles(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	GLuaVec_t * angs = GLua_CheckVector(L, 2);
	vec3_t newangs;
	if (!npc) return 0;
	memcpy(&newangs, angs, 12);

	SetClientViewAngle( npc, newangs );
	return 0;
}

static int GLua_NPC_Teleport(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	GLuaVec_t * orig = GLua_CheckVector(L, 2);
	GLuaVec_t * angs = NULL;
	vec3_t org, ang;
	if (!npc) return 0;
	ConvertVec(orig, org);
	if (!lua_isnoneornil(L,3)) {
		angs = GLua_CheckVector(L, 3);
		ConvertVec(angs, ang);
	} else {
		//memcpy(angs, level.clients[ply->clientNum].ps.viewangles, 12);
		VectorCopy(npc->client->ps.viewangles, ang);
	}
	
	TeleportPlayer(npc, org, ang);
	return 0;
}

static int GLua_NPC_ToString(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) {
		lua_pushstring(L,"[ NULL NPC ]");
	} else {
		lua_pushstring(L,va("NPC [%i]", npc->s.number));
	}
	return 1;
}

extern void NPC_SetMoveGoal( gentity_t *ent, vec3_t point, int radius, qboolean isNavGoal, int combatPoint, gentity_t *targetEnt ); //isNavGoal = qfalse, combatPoint = -1, targetEnt = NULL
void GLua_NPCEV_OnReached(gentity_t *self);
#define	WAYPOINT_NONE	-1

static int GLua_NPC_SetNavGoal(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	gentity_t *targ;
	GLuaVec_t *vec; //= GLua_CheckVector(L, 2);
	const char *name;
	vec3_t vec2;
	if (!npc)
		return 0;
	if ( !npc->health )
	{
		trap->Print( "GLua_NPC_SetNavGoal: tried to set a navgoal on a corpse! \"%s\"\n", npc->script_targetname );
		return 0;
	}
	if ( !npc->NPC )
	{
		trap->Print( "GLua_NPC_SetNavGoal: tried to set a navgoal on a non-NPC: \"%s\"\n", npc->script_targetname );
		return 0;
	}
	if ( !npc->NPC->tempGoal )
	{
		trap->Print( "GLua_NPC_SetNavGoal: tried to set a navgoal on a dead NPC: \"%s\"\n", npc->script_targetname );
		return 0;
	}
	if ( !npc->NPC->tempGoal->inuse )
	{
		trap->Print( "GLua_NPC_SetNavGoal: NPC's navgoal is freed: \"%s\"\n", npc->script_targetname );
		return 0;
	}
	if (lua_isnoneornil(L,2)) {
		// Clear nav goal
		npc->NPC->goalEntity = NULL;
		trap->ICARUS_TaskIDComplete( (sharedEntity_t *)npc, TID_MOVE_NAV );
		npc->NPC->luaFlags.isMoving = qfalse;
		GLua_NPCEV_OnReached(npc);
		return 0;
	}
	if (lua_isstring(L,2)) {
		name = lua_tostring(L,2);
		// We got a string, so treat it as a navpoint targetname
		if ( TAG_GetOrigin2( NULL, name, vec2 ) == qfalse )
		{
			targ = G_Find(NULL, FOFS(targetname), (char*)name);
			if ( !targ )
			{
				trap->Print( "GLua_NPC_SetNavGoal: can't find NAVGOAL \"%s\"\n", name );
				return 0;
			}
			else
			{
				npc->NPC->goalEntity = targ;
				npc->NPC->goalRadius = sqrt(npc->r.maxs[0]+npc->r.maxs[0]) + sqrt(targ->r.maxs[0]+targ->r.maxs[0]);
				npc->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
				npc->NPC->luaFlags.isMoving = qtrue;
				return 0;
			}
		}
		else
		{
			int	goalRadius = TAG_GetRadius( NULL, name );
			NPC_SetMoveGoal( npc, vec2, goalRadius, qtrue, -1, NULL );
			//We know we want to clear the lastWaypoint here
			npc->NPC->goalEntity->lastWaypoint = WAYPOINT_NONE;
			npc->NPC->luaFlags.isMoving = qtrue;
			npc->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
		}
		return 0;
	}
	if (GLua_IsVector(L,2)) {
		vec = GLua_CheckVector(L,2);
		ConvertVec(vec, vec2);

		NPC_SetMoveGoal(npc, vec2, luaL_optint(L, 3, 32), qtrue, -1, NULL);
		npc->NPC->goalEntity->lastWaypoint = -1; //WAYPOINT_NONE
		npc->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
		npc->NPC->luaFlags.isMoving = qtrue;
		return 0;
	}
	if (GLua_IsEntity(L,2)) {
		targ = GLua_CheckEntity(L,2);
		npc->NPC->goalEntity = targ;
		npc->NPC->goalRadius = sqrt(npc->r.maxs[0]+npc->r.maxs[0]) + sqrt(targ->r.maxs[0]+targ->r.maxs[0]);
		npc->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;
		npc->NPC->luaFlags.isMoving = qtrue;
		return 0;
	}
	luaL_error(L, "GLua_NPC_SetNavGoal: Invalid NavGoal specified");
	//trap_ICARUS_TaskIDSet( npc, TID_MOVE_NAV, 0 );
	return 0;
}

static int GLua_NPC_Health(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L,npc->client->ps.stats[STAT_HEALTH]);
	return 1;
}

static int GLua_NPC_MaxHealth(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L,npc->client->ps.stats[STAT_MAX_HEALTH]);
	return 1;
}

static int GLua_NPC_MaxShield(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L,npc->client->ps.stats[STAT_MAX_SHIELD]);
	return 1;
}

static int GLua_NPC_Shield(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L,npc->client->ps.stats[STAT_SHIELD]);
	return 1;
}

static int GLua_NPC_SetHealth(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	npc->client->ps.stats[STAT_HEALTH] = npc->health = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_NPC_SetMaxHealth(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	npc->client->ps.stats[STAT_MAX_HEALTH] = npc->client->pers.maxHealth = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_NPC_SetShield(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	npc->client->ps.stats[STAT_SHIELD] = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_NPC_SetMaxShield(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	npc->client->ps.stats[STAT_MAX_SHIELD] = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_NPC_GetEyeTrace(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	vec3_t src;
	vec3_t dest;
	vec3_t vf;
	trace_t trace;
	if (!npc) return 0;
	VectorCopy(npc->client->ps.origin, src);
	src[2] += npc->client->ps.viewheight;

	AngleVectors( npc->client->ps.viewangles, vf, NULL, NULL );

	VectorMA( src, luaL_optint(L,2,131072), vf, dest );

	//Trace ahead to find a valid target
	trap->Trace( &trace, src, vec3_origin, vec3_origin, dest, npc->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE, 0, 0, 0 );
	
	lua_newtable(L);
	lua_pushstring(L,"StartSolid"); lua_pushboolean(L,trace.startsolid); lua_settable(L,-3);
	lua_pushstring(L,"AllSolid"); lua_pushboolean(L,trace.allsolid); lua_settable(L,-3);
	lua_pushstring(L,"Hit"); lua_pushboolean(L,trace.fraction != 1); lua_settable(L,-3);
	lua_pushstring(L,"HitWorld"); lua_pushboolean(L,(trace.entityNum == ENTITYNUM_WORLD)); lua_settable(L,-3);
	lua_pushstring(L,"HitEnt"); lua_pushboolean(L,(trace.entityNum <= ENTITYNUM_MAX_NORMAL)); lua_settable(L,-3);
	lua_pushstring(L,"Entity"); GLua_PushEntity(L,trace.entityNum <= ENTITYNUM_MAX_NORMAL ? &g_entities[trace.entityNum] : NULL); lua_settable(L,-3);
	lua_pushstring(L,"Contents"); lua_pushinteger(L,trace.contents); lua_settable(L,-3);
	lua_pushstring(L,"SurfaceFlags"); lua_pushinteger(L,trace.surfaceFlags); lua_settable(L,-3);
	lua_pushstring(L,"StartPos"); GLua_PushVector(L,src[0],src[1],src[2]); lua_settable(L,-3);
	lua_pushstring(L,"EndPos"); GLua_PushVector(L,trace.endpos[0],trace.endpos[1],trace.endpos[2]); lua_settable(L,-3);
	lua_pushstring(L,"HitNormal"); GLua_PushVector(L,trace.plane.normal[0],trace.plane.normal[1],trace.plane.normal[2]); lua_settable(L,-3);

	return 1;
}

static int GLua_NPC_GetEntity(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	GLua_PushEntity(L, npc);
	return 1;
}

static int GLua_NPC_Damage(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	gentity_t *inflictor = NULL, *attacker = NULL;
	GLuaVec_t *dir = NULL, *point = NULL;
	vec3_t dir2 = {0,0,0}, point2 = {0,0,0};
	int damage, dflags, mod;
	if (!npc) return 0;
	if (!lua_isnoneornil(L,2)) inflictor = GLua_CheckEntity(L, 2);
	if (!lua_isnoneornil(L,3)) attacker = GLua_CheckEntity(L, 3);
	if (!lua_isnoneornil(L,4)) {
		dir = GLua_CheckVector(L, 4);
		ConvertVec(dir, dir2);
	}
	if (!lua_isnoneornil(L,5)) {
		point = GLua_CheckVector(L, 5);
		ConvertVec(point, point2);
	}
	damage = lua_tointeger(L, 6);
	dflags = lua_tointeger(L, 7);
	mod = lua_tointeger(L, 8);
	
	G_Damage(npc, inflictor, attacker, dir2, point2, damage, dflags, mod);
	return 0;
}

static int GLua_NPC_GiveForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!npc) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	npc->client->ps.fd.forcePowersKnown |=  (1 << force);
	return 0;
}

static int GLua_NPC_TakeForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!npc) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	npc->client->ps.fd.forcePowersKnown &=  ~(1 << force);
	npc->client->ps.fd.forcePowerLevel[force] = FORCE_LEVEL_0;
	npc->client->ps.fd.forcePowerBaseLevel[force] = FORCE_LEVEL_0;
	return 0;
}

static int GLua_NPC_HasForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!npc) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	lua_pushboolean(L,npc->client->ps.fd.forcePowersKnown & (1 << force));
	return 1;
}

static int GLua_NPC_SetForceLevel(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int force = luaL_checkinteger(L,2);
	int newlevel = luaL_checkinteger(L,3);
	if (!npc) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	if (newlevel < 0 || newlevel > 3) return 0; // level > 3 is subject to change if we go for lvl 5
	if (!(npc->client->ps.fd.forcePowersKnown & (1 << force))) return 0;
	npc->client->ps.fd.forcePowerLevel[force] = newlevel;
	npc->client->ps.fd.forcePowerBaseLevel[force] = newlevel;
	return 0;
}

static int GLua_NPC_GetForceLevel(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!npc) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	if (!(npc->client->ps.fd.forcePowersKnown & (1 << force))) return 0;
	lua_pushinteger(L,npc->client->ps.fd.forcePowerLevel[force]);
	return 1;
}

static int GLua_NPC_GetActiveForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L,npc->client->ps.fd.forcePowerSelected);
	return 1;
}


static int GLua_NPC_SetGodMode(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int active = lua_toboolean(L,2);
	if (!npc) return 0;
	if (active) {
		npc->flags |= FL_GODMODE;
	} else {
		npc->flags &= ~FL_GODMODE;
	}
	return 0;
}

static int GLua_NPC_HasGodMode(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->flags & FL_GODMODE) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_NPC_SetNoKnockback(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int active = lua_toboolean(L,2);
	if (!npc) return 0;
	if (active) {
		npc->flags |= FL_NO_KNOCKBACK;
	} else {
		npc->flags &= ~FL_NO_KNOCKBACK;
	}
	return 0;
}

static int GLua_NPC_HasNoKnockback(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->flags & FL_NO_KNOCKBACK) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}


static int GLua_NPC_SetNoTarget(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int active = lua_toboolean(L,2);
	if (!npc) return 0;
	if (active) {
		npc->flags |= FL_NOTARGET;
	} else {
		npc->flags &= ~FL_NOTARGET;
	}
	return 0;
}

static int GLua_NPC_HasNoTarget(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->flags & FL_NOTARGET) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_NPC_SetBusy(lua_State* L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	int active = lua_toboolean(L, 2);
	if (!npc) return 0;
	if (active) {
		npc->flags |= FL_BUSYMODE;
	}
	else {
		npc->flags &= ~FL_BUSYMODE;
	}
	return 0;
}

static int GLua_NPC_HasBusy(lua_State* L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->flags & FL_BUSYMODE) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int GLua_NPC_SetNoDebuff(lua_State* L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	int active = lua_toboolean(L, 2);
	if (!npc) return 0;
	if (active) {
		npc->flags |= FL_NO_DEBUFF;
	}
	else {
		npc->flags &= ~FL_NO_DEBUFF;
	}
	return 0;
}

static int GLua_NPC_HasNoDebuff(lua_State* L)
{
	gentity_t* npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->flags & FL_NO_DEBUFF) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

extern void G_BuffEntity(gentity_t* ent, gentity_t* buffer, int buffID, float intensity, int duration);
static int GLua_NPC_AddBuff(lua_State* L)
{
	//L1 = Player (required), L2 = Buffname (required), L3 = duration, L4 = intensity
	if (lua_isnoneornil(L, 3)) return luaL_error(L, "No buff name provided");
	gentity_t* npc = GLua_CheckNPC(L, 2);
	if (!npc) return 0;

	int duration = 5000; // default to 5 seconds
	float intensity = 1.0f; // default to normal intensity
	int buffID = -1;

	const char* buffName = luaL_checkstring(L, 3);
	if (sizeof(buffName) > BUFF_NAME_LEN)
	{
		return luaL_error(L, "Invalid buff name provided");
	}

	buffID = JKG_ResolveBuffName(buffName);
	if (buffID < 0)
	{
		return luaL_error(L, "Invalid buff name provided");
	}

	//grab duration and intensity values (if present)
	if (lua_gettop(L) > 3)
	{
		duration = lua_tointeger(L, 4);

		if (lua_gettop(L) > 4)
			intensity = lua_tonumber(L, 5);
	}

	G_BuffEntity(npc, npc, buffID, intensity, duration);
	return 0;
}

extern stringID_table_t BSTable[];
int NAV_FindClosestWaypointForEnt( gentity_t *ent, int targWp );
void NPC_BSSearchStart( int homeWp, bState_t bState );

static int GLua_NPC_SetBehaviorState(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	bState_t	bSID;

	if (!npc) return 0;

	// Following code is taken from icarus

	if (lua_isstring(L,2)) {
		bSID = (bState_t)(GetIDForString( BSTable, lua_tostring(L,2) ));
	} else {
		bSID = (bState_t)luaL_checkinteger(L,2);
	}

	if ( bSID != (bState_t)-1 )
	{
		if ( bSID == BS_SEARCH || bSID == BS_WANDER )
		{
			//FIXME: Reimplement
			
			if( npc->waypoint != WAYPOINT_NONE )
			{
				NPC_BSSearchStart( npc->waypoint, bSID );
			}
			else
			{
				npc->waypoint = NAV_FindClosestWaypointForEnt( npc, WAYPOINT_NONE );

				if( npc->waypoint != WAYPOINT_NONE )
				{
					NPC_BSSearchStart( npc->waypoint, bSID );
				}
				/*else if( ent->lastWaypoint >=0 && ent->lastWaypoint < num_waypoints )
				{
					NPC_BSSearchStart( ent->lastWaypoint, bSID );
				}
				else if( ent->lastValidWaypoint >=0 && ent->lastValidWaypoint < num_waypoints )
				{
					NPC_BSSearchStart( ent->lastValidWaypoint, bSID );
				}*/
				else
				{
					//G_DebugPrint( WL_ERROR, "Q3_SetBState: '%s' is not in a valid waypoint to search from!\n", ent->targetname );
					return 0;
				}
			}
		}

		npc->NPC->tempBehavior = BS_DEFAULT;//need to clear any temp behaviour
		if ( npc->NPC->behaviorState == BS_NOCLIP && bSID != BS_NOCLIP )
		{//need to rise up out of the floor after noclipping
			npc->r.currentOrigin[2] += 0.125;
			G_SetOrigin( npc, npc->r.currentOrigin );
		}
		npc->NPC->behaviorState = bSID;
		if ( bSID == BS_DEFAULT )
		{
			npc->NPC->defaultBehavior = bSID;
		}
	}

	npc->NPC->aiFlags &= ~NPCAI_TOUCHED_GOAL;

//	if ( bSID == BS_FLY )
//	{//FIXME: need a set bState wrapper
//		ent->client->moveType = MT_FLYSWIM;
//	}
//	else
	{
		//FIXME: these are presumptions!
		//Q3_SetGravity( entID, g_gravity->value );
		//ent->client->moveType = MT_RUNJUMP;
	}

	if ( bSID == BS_NOCLIP )
	{
		npc->client->noclip = qtrue;
	}
	else
	{
		npc->client->noclip = qfalse;
	}

/*
	if ( bSID == BS_FACE || bSID == BS_POINT_AND_SHOOT || bSID == BS_FACE_ENEMY )
	{
		ent->NPC->aimTime = level.time + 5 * 1000;//try for 5 seconds
		return qfalse;//need to wait for task complete message
	}
*/

//	if ( bSID == BS_SNIPER || bSID == BS_ADVANCE_FIGHT )
	if ( bSID == BS_ADVANCE_FIGHT )
	{
		return 0;//need to wait for task complete message
	}

/*
	if ( bSID == BS_SHOOT || bSID == BS_POINT_AND_SHOOT )
	{//Let them shoot right NOW
		ent->NPC->shotTime = ent->attackDebounceTime = level.time;
	}
*/
	if ( bSID == BS_JUMP )
	{
		npc->NPC->jumpState = JS_FACING;
	}

	return 0;
}

static int GLua_NPC_SetHFov(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int hfov = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.hfov = hfov;
	return 0;
}

static int GLua_NPC_GetHFov(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.hfov);
	return 1;
}

static int GLua_NPC_SetVFov(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int vfov = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.vfov = vfov;
	return 0;
}

static int GLua_NPC_GetVFov(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.vfov);
	return 1;
}

static int GLua_NPC_SetVisRange(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	float visrange = luaL_checknumber(L,2);
	if (!npc) return 0;
	npc->NPC->stats.visrange = visrange;
	return 0;
}

static int GLua_NPC_GetVisRange(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushnumber(L, npc->NPC->stats.visrange);
	return 1;
}

static int GLua_NPC_SetEarShot(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	float earshot = luaL_checknumber(L,2);
	if (!npc) return 0;
	npc->NPC->stats.earshot = earshot;
	return 0;
}

static int GLua_NPC_GetEarShot(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushnumber(L, npc->NPC->stats.earshot);
	return 1;
}

static int GLua_NPC_SetAim(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int aim = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.aim = aim;
	return 0;
}

static int GLua_NPC_GetAim(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.aim);
	return 1;
}

static int GLua_NPC_SetVigilance(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int vigilance = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.vigilance = vigilance;
	return 0;
}

static int GLua_NPC_GetVigilance(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.vigilance);
	return 1;
}

static int GLua_NPC_SetAggression(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int aggression = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.aggression = aggression;
	return 0;
}

static int GLua_NPC_GetAggression(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.aggression);
	return 1;
}

static int GLua_NPC_SetEvasion(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int evasion = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.evasion = evasion;
	return 0;
}

static int GLua_NPC_GetEvasion(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.evasion);
	return 1;
}

static int GLua_NPC_SetReactions(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int reactions = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.reactions = reactions;
	return 0;
}

static int GLua_NPC_GetReactions(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.reactions);
	return 1;
}

static int GLua_NPC_SetFollowDist(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	float followdist = luaL_checknumber(L,2);
	if (!npc) return 0;
	npc->NPC->followDist = followdist;
	return 0;
}

static int GLua_NPC_GetFollowDist(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushnumber(L, npc->NPC->followDist);
	return 1;
}

static int GLua_NPC_SetWalkSpeed(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int walkspeed = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.walkSpeed = walkspeed;
	return 0;
}

static int GLua_NPC_GetWalkSpeed(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.walkSpeed);
	return 1;
}

static int GLua_NPC_SetRunSpeed(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int runspeed = luaL_checkinteger(L,2);
	if (!npc) return 0;
	npc->NPC->stats.runSpeed = runspeed;
	return 0;
}

static int GLua_NPC_GetRunSpeed(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->stats.runSpeed);
	return 1;
}

static int GLua_NPC_SetWalking(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_WALKING;
		npc->NPC->scriptFlags &= ~SCF_RUNNING;
	} else {
		npc->NPC->scriptFlags &= ~SCF_WALKING;
	}
	return 0;
}

static int GLua_NPC_GetWalking(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_WALKING);
	return 1;
}

static int GLua_NPC_SetRunning(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_RUNNING;
	} else {
		npc->NPC->scriptFlags &= ~SCF_RUNNING;
	}
	return 0;
}

static int GLua_NPC_GetRunning(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_RUNNING);
	return 1;
}

static int GLua_NPC_SetLookForEnemies(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_LOOK_FOR_ENEMIES;
	} else {
		npc->NPC->scriptFlags &= ~SCF_LOOK_FOR_ENEMIES;
	}
	return 0;
}

static int GLua_NPC_GetLookForEnemies(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_LOOK_FOR_ENEMIES);
	return 1;
}

static int GLua_NPC_SetChaseEnemies(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_CHASE_ENEMIES;
	} else {
		npc->NPC->scriptFlags &= ~SCF_CHASE_ENEMIES;
	}
	return 0;
}

static int GLua_NPC_GetChaseEnemies(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_CHASE_ENEMIES);
	return 1;
}

static int GLua_NPC_SetNoForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_NO_FORCE;
	} else {
		npc->NPC->scriptFlags &= ~SCF_NO_FORCE;
	}
	return 0;
}

static int GLua_NPC_GetNoForce(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_NO_FORCE);
	return 1;
}

static int GLua_NPC_SetNoMindTrick(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_NO_MIND_TRICK;
		npc->NPC->confusionTime = 0;
	} else {
		npc->NPC->scriptFlags &= ~SCF_NO_MIND_TRICK;
	}
	return 0;
}

static int GLua_NPC_GetNoMindTrick(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_NO_MIND_TRICK);
	return 1;
}

static int GLua_NPC_SetNoAcrobatics(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_NO_ACROBATICS;
	} else {
		npc->NPC->scriptFlags &= ~SCF_NO_ACROBATICS;
	}
	return 0;
}

static int GLua_NPC_GetNoAcrobatics(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_NO_ACROBATICS);
	return 1;
}

static int GLua_NPC_SetDontFire(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_DONT_FIRE;
	} else {
		npc->NPC->scriptFlags &= ~SCF_DONT_FIRE;
	}
	return 0;
}

static int GLua_NPC_GetDontFire(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_DONT_FIRE);
	return 1;
}

static int GLua_NPC_SetFireWeapon(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_FIRE_WEAPON;
	} else {
		npc->NPC->scriptFlags &= ~SCF_FIRE_WEAPON;
	}
	return 0;
}

static int GLua_NPC_GetFireWeapon(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_FIRE_WEAPON);
	return 1;
}

static int GLua_NPC_SetForcedMarch(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_FORCED_MARCH;
	} else {
		npc->NPC->scriptFlags &= ~SCF_FORCED_MARCH;
	}
	return 0;
}

static int GLua_NPC_GetForcedMarch(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_FORCED_MARCH);
	return 1;
}

static int GLua_NPC_SetAltFire(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_ALT_FIRE;
	} else {
		npc->NPC->scriptFlags &= ~SCF_ALT_FIRE;
	}
	return 0;
}

static int GLua_NPC_GetAltFire(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_ALT_FIRE);
	return 1;
}

static int GLua_NPC_SetNoAvoid(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->aiFlags |= NPCAI_NO_COLL_AVOID;
	} else {
		npc->NPC->aiFlags &= ~NPCAI_NO_COLL_AVOID;
	}
	return 0;
}

static int GLua_NPC_GetNoAvoid(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->aiFlags & NPCAI_NO_COLL_AVOID);
	return 1;
}

static int GLua_NPC_SetNPCFreeze(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->r.svFlags |= SVF_ICARUS_FREEZE;
	} else {
		npc->r.svFlags &= ~SVF_ICARUS_FREEZE;
	}
	return 0;
}

static int GLua_NPC_GetNPCFreeze(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->r.svFlags & SVF_ICARUS_FREEZE);
	return 1;
}

static int GLua_NPC_SetCrouched(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int add = lua_toboolean(L,2);
	if (!npc) return 0;
	if (add) {
		npc->NPC->scriptFlags |= SCF_CROUCHED;
	} else {
		npc->NPC->scriptFlags &= ~SCF_CROUCHED;
	}
	return 0;
}

static int GLua_NPC_GetCrouched(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushboolean(L, npc->NPC->scriptFlags & SCF_CROUCHED);
	return 1;
}

void G_ClearEnemy (gentity_t *self);
static int GLua_NPC_SetEnemy(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	gentity_t *enemy ;
	if (!npc) return 0;
	if (lua_isnoneornil(L, 2)) {
		G_ClearEnemy(npc);	// Clear the enemy
		return 0;
	}
	if (lua_isstring(L,2)) {
		// It's a targetname, so do a G_Find
		enemy = G_Find( NULL, FOFS(targetname), lua_tostring(L,2));
		if (!enemy) {
			trap->Print("GLua_NPC_SetEnemy: no such enemy: '%s'\n", lua_tostring(L,2));
			return 0;
		}
		G_SetEnemy(npc, enemy);
		return 0;
	}
	if (GLua_IsEntity(L, 2)) {
		enemy = GLua_CheckEntity(L,2);
		if (!enemy) {
			// NULL entity was passed
			G_ClearEnemy(npc);
		} else {
			G_SetEnemy(npc, enemy);
		}
		return 0;
	}
	luaL_error(L, "GLua_NPC_SetEnemy: Invalid enemy specified");
	return 0;
}

static int GLua_NPC_GetEnemy(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	GLua_PushEntity(L, npc->enemy);
	return 1;
}

static int GLua_NPC_SetLeader(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	gentity_t *leader ;
	if (!npc) return 0;
	if (lua_isnoneornil(L, 2)) {
		G_ClearEnemy(npc);	// Clear the enemy
		return 0;
	}
	if (lua_isstring(L,2)) {
		// It's a targetname, so do a G_Find
		leader = G_Find( NULL, FOFS(targetname), lua_tostring(L,2));
		if (!leader) {
			trap->Print("GLua_NPC_SetLeader: no such leader: '%s'\n", lua_tostring(L,2));
			return 0;
		}
		npc->client->leader = leader;
		return 0;
	}
	if (GLua_IsEntity(L, 2)) {
		leader = GLua_CheckEntity(L,2);
		if (!leader) {
			// NULL entity was passed
			npc->client->leader = NULL;
		} else {
			npc->client->leader = leader;
		}
		return 0;
	}
	luaL_error(L, "GLua_NPC_SetLeader: Invalid leader specified");
	return 0;
}

static int GLua_NPC_GetLeader(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	GLua_PushEntity(L, npc->client->leader);
	return 1;
}

static int GLua_NPC_SetDYaw(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	float yaw = luaL_checknumber(L,2);
	if (!npc) return 0;
	if (!npc->enemy) {
		npc->NPC->lockedDesiredYaw = npc->NPC->desiredYaw = npc->s.angles[1] = yaw;
	}
	return 0;
}

static int GLua_NPC_GetDYaw(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushnumber(L, npc->NPC->desiredYaw);
	return 1;
}

static int GLua_NPC_SetDPitch(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	float pitch = luaL_checknumber(L,2);
	int pitchMin; //= -ent->client->renderInfo.headPitchRangeUp + 1;
	int pitchMax; //= ent->client->renderInfo.headPitchRangeDown - 1;
	if (!npc) return 0;

	pitchMin = -npc->client->renderInfo.headPitchRangeUp + 1;
	pitchMax = npc->client->renderInfo.headPitchRangeDown - 1;
	
	//clamp angle to -180 -> 180
	pitch = AngleNormalize180( pitch );

	//Clamp it to my valid range
	if ( pitch < -1 )
	{
		if ( pitch < pitchMin )
		{
			pitch = pitchMin;
		}
	}
	else if ( pitch > 1 )
	{
		if ( pitch > pitchMax )
		{
			pitch = pitchMax;
		}
	}
	
	npc->NPC->lockedDesiredPitch = npc->NPC->desiredPitch = pitch;
	return 0;
}

static int GLua_NPC_GetDPitch(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushnumber(L, npc->NPC->desiredPitch);
	return 1;
}

static int GLua_NPC_SetViewTarget(lua_State *L) {		// Does not have a Get equivalent!
	gentity_t *npc = GLua_CheckNPC(L, 1);
	gentity_t	*viewtarget;
	vec3_t		viewspot, selfspot, viewvec, viewangles;
	GLuaVec_t *vec;
	float pitch;
	int pitchMin;
	int pitchMax;

	if (!npc) return 0;
	if (npc->enemy) {
		return 0;		// Overriding viewangles doesnt work once the AI kicks in
	}					// So if this NPC has an enemy, dont bother continuing
	if (lua_isstring(L,2)) {
		viewtarget = G_Find( NULL, FOFS(targetname), lua_tostring(L,2));
		if (!viewtarget) {
			trap->Print("GLua_NPC_SetViewTarget: Could not find entity '%s'\n",  lua_tostring(L,2));
			return 0;
		}

		if ( viewtarget->client ) {
			VectorCopy ( viewtarget->client->renderInfo.eyePoint, viewspot );
		} else {
			VectorCopy ( viewtarget->r.currentOrigin, viewspot );
		}
	} else if (GLua_IsEntity(L,2)) {
		viewtarget = GLua_CheckEntity(L,2);	
		if (!viewtarget) {
			trap->Print("GLua_NPC_SetViewTarget: NULL entity provided\n");
			return 0;
		}

		if ( viewtarget->client ) {
			VectorCopy ( viewtarget->client->renderInfo.eyePoint, viewspot );
		} else {
			VectorCopy ( viewtarget->r.currentOrigin, viewspot );
		}

	} else if (GLua_IsVector(L,2)) {
		vec = GLua_CheckVector(L,2);
		ConvertVec(vec, viewspot);
	} else {
		luaL_error(L, "GLua_NPC_SetViewTarget: Invalid target specified");
	}
	

	VectorCopy ( npc->r.currentOrigin, selfspot );
	selfspot[2] += npc->client->ps.viewheight;

	VectorSubtract( viewspot, selfspot, viewvec );
	
	vectoangles( viewvec, viewangles );

	npc->NPC->lockedDesiredYaw = npc->NPC->desiredYaw = npc->s.angles[1] = viewangles[YAW];

	pitchMin = -npc->client->renderInfo.headPitchRangeUp + 1;
	pitchMax = npc->client->renderInfo.headPitchRangeDown - 1;
	
	//clamp angle to -180 -> 180
	pitch = AngleNormalize180( viewangles[PITCH] );

	//Clamp it to my valid range
	if ( pitch < -1 )
	{
		if ( pitch < pitchMin )
		{
			pitch = pitchMin;
		}
	}
	else if ( pitch > 1 )
	{
		if ( pitch > pitchMax )
		{
			pitch = pitchMax;
		}
	}
	
	npc->NPC->lockedDesiredPitch = npc->NPC->desiredPitch = viewangles[PITCH];
	return 0;
}

static int GLua_NPC_SetWatchTarget(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (lua_isnoneornil(L,2)) {
		npc->NPC->watchTarget = NULL;
	} else if (lua_isstring(L,2)) {
		npc->NPC->watchTarget = G_Find( NULL, FOFS(targetname), lua_tostring(L,2));
	} else if (GLua_IsEntity(L,2)) {
		npc->NPC->watchTarget = GLua_CheckEntity(L,2);
	} else {
		luaL_error(L, "GLua_NPC_SetWatchTarget: Invalid target specified");
	}
	return 0;
}

static int GLua_NPC_GetWatchTarget(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	if (npc->NPC->watchTarget) {
		GLua_PushEntity(L, npc->NPC->watchTarget);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int GLua_NPC_Remove(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	// Queue this NPC for deletion
	npc->think = G_FreeEntity;
	npc->nextthink = level.time + FRAMETIME;
	return 0;
}

extern stringID_table_t animTable [MAX_ANIMATIONS+1];
void NPC_SetAnim(gentity_t *ent, int setAnimParts, int anim, int setAnimFlags);

static int GLua_NPC_SetAnimLower(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int anim;
	if (!npc) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	//G_SetAnim(ent, NULL, SETANIM_LEGS, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE),  luaL_optinteger(L, 4, 0));
	NPC_SetAnim(npc, SETANIM_LEGS, anim, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE);
	return 0;
}

static int GLua_NPC_SetAnimUpper(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int anim;
	if (!npc) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	//G_SetAnim(ent, NULL, SETANIM_TORSO, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE),  luaL_optinteger(L, 4, 0));
	NPC_SetAnim(npc, SETANIM_TORSO, anim, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE);
	return 0;
}

static int GLua_NPC_SetAnimBoth(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int anim;
	if (!npc) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	//G_SetAnim(ent, NULL, SETANIM_BOTH, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE),  luaL_optinteger(L, 4, 0));
	NPC_SetAnim(npc, SETANIM_BOTH, anim, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE);
	return 0;
}

void BG_SetTorsoAnimTimer(playerState_t *ps, int time );
void BG_SetLegsAnimTimer(playerState_t *ps, int time);
static int GLua_NPC_SetAnimHoldTime(lua_State *L) {
	gentity_t *npc = GLua_CheckNPC(L, 1);
	int section = lua_tointeger(L,2);
	int time = lua_tointeger(L,3);
	if (!npc) return 0;
	if (section & 1) {
		BG_SetTorsoAnimTimer(&npc->client->ps, time);
	}
	if (section & 2) {
		BG_SetLegsAnimTimer(&npc->client->ps, time);
	}
	return 0;
}

static int GLua_NPC_SetUseRange(lua_State *L)
{
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	npc->NPC->maxUseRange = luaL_checknumber(L,2);
	return 0;
}

static int GLua_NPC_GetUseRange(lua_State *L)
{
	gentity_t *npc = GLua_CheckNPC(L, 1);
	if (!npc) return 0;
	lua_pushinteger(L, npc->NPC->maxUseRange);
	return 1;
}

/*
static int GLua_Player_SetVelocity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	GLuaVec_t *vel2 = GLua_CheckVector(L,2);
	vec3_t vel;
	if (!ply) return 0;
	ConvertVec(vel2, vel);
	VectorCopy(level.clients[ply->clientNum].ps.velocity, vel);
	return 0;
}


static int GLua_Player_AddVelocity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	GLuaVec_t *vel2 = GLua_CheckVector(L,2);
	vec3_t vel;
	if (!ply) return 0;
	ConvertVec(vel2, vel);
	VectorAdd(level.clients[ply->clientNum].ps.velocity, vel, level.clients[ply->clientNum].ps.velocity);
	return 0;
}

static int GLua_Player_SetAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int ammo = luaL_checkint(L,2);
	int amt = luaL_checkint(L,3);
	if (!ply) return 0;
	if (ammo < 0 || ammo >= AMMO_MAX) {
		return 0;
	}
	if (amt < 0) {
		amt = 0;
	}
	g_entities[ply->clientNum].client->ps.ammo[ammo] = amt;
	
	return 0;
}

static int GLua_Player_GetAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int ammo = luaL_checkint(L,2);
	if (!ply) return 0;
	if (ammo < 0 || ammo >= AMMO_MAX) {
		return 0;
	}
	lua_pushinteger(L, g_entities[ply->clientNum].client->ps.ammo[ammo]);
	return 1;
}

static int GLua_Player_SetClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int weapon = luaL_checkint(L,2);
	int amt = luaL_checkint(L,3);
	if (!ply) return 0;
	if (weapon < 0 || weapon >= MAX_WEAPONS) {
		return 0;
	}
	if (amt < 0) {
		amt = 0;
	} else if (amt > weaponData[weapon].clipSize) {
		amt = weaponData[weapon].clipSize;
	}
	g_entities[ply->clientNum].client->clipammo[weapon] = amt;
	
	return 0;
}

static int GLua_Player_StripClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int i;
	if (!ply) return 0;
	for (i=0; i < MAX_WEAPONS; i++) {
		level.clients[ply->clientNum].clipammo[i] = 0;
	}
	return 0;
}

static int GLua_Player_StripAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int i;
	if (!ply) return 0;
	for (i=0; i < AMMO_MAX; i++) {
		level.clients[ply->clientNum].ps.ammo[i] = 0;
	}
	return 0;
}


static int GLua_Player_GetClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int weapon = luaL_checkint(L,2);
	if (!ply) return 0;
	if (weapon < 0 || weapon >= MAX_WEAPONS) {
		return 0;
	}
	lua_pushinteger(L, g_entities[ply->clientNum].client->clipammo[weapon]);
	return 1;
}

static int GLua_Player_SetKnockback(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int set = lua_toboolean(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (set) {
		ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	} else {
		ent->client->ps.pm_flags &= ~PMF_TIME_KNOCKBACK;
	}
	return 0;
}

static int GLua_Player_SetGravity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (lua_isnoneornil(L,2)) {
		ent->client->customGravity = qfalse;
	} else {
		ent->client->customGravity = qtrue;
		ent->client->ps.gravity = lua_tointeger(L, 2);
	}
	return 0;
}

static int GLua_Player_GetGravity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->client->customGravity) {
		lua_pushinteger(L, ent->client->ps.gravity);
	} else {
		lua_pushinteger(L, g_gravity.integer);
	}
	return 1;
}

static int GLua_Player_SetUndying(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (lua_toboolean(L,2)) {
		ent->flags |= FL_UNDYING;
	} else {
		ent->flags &= ~FL_UNDYING;
	}
	return 0;
}

static int GLua_Player_GetUndying(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->flags & FL_UNDYING);
	return 1;
}

static int GLua_Player_SetInvulnerable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (lua_isnoneornil(L,2)) {
		// Perma-invulerability
		ent->client->ps.eFlags |= EF_INVULNERABLE;
		ent->client->invulnerableTimer = Q3_TIMEINFINITE;
	} else {
		int time = lua_tointeger(L,2);
		if (time > 0) {
			ent->client->ps.eFlags |= EF_INVULNERABLE;
			ent->client->invulnerableTimer = level.time + time;
		} else {
			ent->client->ps.eFlags &= ~EF_INVULNERABLE;
			ent->client->invulnerableTimer = 0;
		}
	}
	return 0;
}

static int GLua_Player_GetInvulnerable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->client->ps.eFlags & EF_INVULNERABLE) {
		lua_pushinteger(L, level.time - ent->client->invulnerableTimer);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int GLua_Player_SetFreeze(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	int lock = lua_toboolean(L,3);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (enable) {
		if (lock) {
			ent->client->pmfreeze = qtrue;
			ent->client->pmlock = qtrue;
		} else {
			ent->client->pmfreeze = qtrue;
			ent->client->pmlock = qfalse;
		}
	} else {
		ent->client->pmfreeze = qfalse;
		ent->client->pmlock = qfalse;
	}
	return 0;
}

static int GLua_Player_GetFreeze(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	int lock = lua_toboolean(L,3);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->pmfreeze);
	return 1;
}

static int GLua_Player_GiveHoldable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int item = luaL_checkinteger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (item < 0 || item >= HI_NUM_HOLDABLE) {
		return 0;
	}
	ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << item);
	return 0;
}

void Jetpack_Off(gentity_t *ent);
void NPC_Humanoid_Decloak( gentity_t *self );

static int GLua_Player_TakeHoldable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int item = luaL_checkinteger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (item < 0 || item >= HI_NUM_HOLDABLE) {
		return 0;
	}
	ent->client->ps.stats[STAT_HOLDABLE_ITEMS] &= ~(1 << item);
	switch (item) {
		case HI_JETPACK:
			Jetpack_Off(ent);
			break;
		case HI_CLOAK:
			Jedi_Decloak(ent);
			break;
		default:
			break;
	}
	return 0;
}

static int GLua_Player_HasHoldable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int item = luaL_checkinteger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (item < 0 || item >= HI_NUM_HOLDABLE) {
		return 0;
	}
	lua_pushboolean(L, (ent->client->ps.stats[STAT_HOLDABLE_ITEMS] & item));;
	return 1;
}
*/

//Stoiss added this again as it fucked up the lua code for npcs
static const struct luaL_reg npc_m[] = {
	/* Metamethods */
	{"__index", GLua_NPC_Index},
	{"__newindex", GLua_NPC_NewIndex},
	{"__eq", GLua_NPC_Eq},
	{"__tostring", GLua_NPC_ToString},
	/* Normal methods */
	{"GetTable", GLua_NPC_GetTable},
	{"GetIndex", GLua_NPC_GetIndex},
	{"IsValid", GLua_NPC_IsValid},
	{"Kill", GLua_NPC_Kill},
	{"MakeVendor", GLua_NPC_MakeVendor},
	{"UnmakeVendor", GLua_NPC_UnmakeVendor},
	{"RefreshVendorStock", GLua_NPC_RefreshStock},
	{"UseVendor", GLua_NPC_UseVendor},
	{"SetPos", GLua_NPC_SetPos},
	{"GetPos", GLua_NPC_GetPos},
	{"SetOrigin", GLua_NPC_SetPos},
	{"GetOrigin", GLua_NPC_GetPos},
	{"SetAngles", GLua_NPC_SetAngles},
	{"GetAngles", GLua_NPC_GetAngles},
	{"Teleport", GLua_NPC_Teleport},
	{"SetNavGoal", GLua_NPC_SetNavGoal},
	{"Health", GLua_NPC_Health},
	{"MaxHealth", GLua_NPC_MaxHealth},
	{"MaxArmor", GLua_NPC_MaxShield},
	{"Armor", GLua_NPC_Shield},
	{"SetHealth", GLua_NPC_SetHealth},
	{"SetMaxHealth", GLua_NPC_SetMaxHealth},
	{"SetShield", GLua_NPC_SetShield},
	{"SetMaxShield", GLua_NPC_SetMaxShield},
	{"GetEyeTrace", GLua_NPC_GetEyeTrace},
	{"GetEntity", GLua_NPC_GetEntity},
	{"Damage", GLua_NPC_Damage},
	{"AddBuff", GLua_NPC_AddBuff},
	{"GiveForce", GLua_NPC_GiveForce},
	{"TakeForce", GLua_NPC_TakeForce},
	{"HasForce", GLua_NPC_HasForce},
	{"SetForceLevel", GLua_NPC_SetForceLevel},
	{"GetForceLevel", GLua_NPC_GetForceLevel},
	{"GetActiveForce", GLua_NPC_GetActiveForce},
	{"SetGodMode", GLua_NPC_SetGodMode},
	{"HasGodMode", GLua_NPC_HasGodMode},
	{"SetNoKnockback", GLua_NPC_SetNoKnockback},
	{"HasNoKnockback", GLua_NPC_HasNoKnockback},
	{"SetNoTarget", GLua_NPC_SetNoTarget},
	{"HasNoTarget", GLua_NPC_HasNoTarget},
	{"SetBehaviorState", GLua_NPC_SetBehaviorState},
	{"SetHFov", GLua_NPC_SetHFov},
	{"GetHFov", GLua_NPC_GetHFov},
	{"SetVFov", GLua_NPC_SetVFov},
	{"GetVFov", GLua_NPC_GetVFov},
	{"SetVisRange", GLua_NPC_SetVisRange},
	{"GetVisRange", GLua_NPC_GetVisRange},
	{"SetEarShot", GLua_NPC_SetEarShot},
	{"GetEarShot", GLua_NPC_GetEarShot},
	{"SetAim", GLua_NPC_SetAim},
	{"GetAim", GLua_NPC_GetAim},
	{"SetVigilance", GLua_NPC_SetVigilance},
	{"GetVigilance", GLua_NPC_GetVigilance},
	{"SetAggression", GLua_NPC_SetAggression},
	{"GetAggression", GLua_NPC_GetAggression},
	{"SetEvasion", GLua_NPC_SetEvasion},
	{"GetEvasion", GLua_NPC_GetEvasion},
	{"SetReactions", GLua_NPC_SetReactions},
	{"GetReactions", GLua_NPC_GetReactions},
	{"SetFollowDist", GLua_NPC_SetFollowDist},
	{"GetFollowDist", GLua_NPC_GetFollowDist},
	{"SetWalkSpeed", GLua_NPC_SetWalkSpeed},
	{"GetWalkSpeed", GLua_NPC_GetWalkSpeed},
	{"SetRunSpeed", GLua_NPC_SetRunSpeed},
	{"GetRunSpeed", GLua_NPC_GetRunSpeed},
	{"SetWalking", GLua_NPC_SetWalking},
	{"GetWalking", GLua_NPC_GetWalking},
	{"SetRunning", GLua_NPC_SetRunning},
	{"GetRunning", GLua_NPC_GetRunning},
	{"SetLookForEnemies", GLua_NPC_SetLookForEnemies},
	{"GetLookForEnemies", GLua_NPC_GetLookForEnemies},
	{"SetChaseEnemies", GLua_NPC_SetChaseEnemies},
	{"GetChaseEnemies", GLua_NPC_GetChaseEnemies},
	{"SetNoForce", GLua_NPC_SetNoForce},
	{"GetNoForce", GLua_NPC_GetNoForce},
	{"SetNoMindTrick", GLua_NPC_SetNoMindTrick},
	{"GetNoMindTrick", GLua_NPC_GetNoMindTrick},
	{"SetNoAcrobatics", GLua_NPC_SetNoAcrobatics},
	{"GetNoAcrobatics", GLua_NPC_GetNoAcrobatics},
	{"SetDontFire", GLua_NPC_SetDontFire},
	{"GetDontFire", GLua_NPC_GetDontFire},
	{"SetFireWeapon", GLua_NPC_SetFireWeapon},
	{"GetFireWeapon", GLua_NPC_GetFireWeapon},
	{"SetForcedMarch", GLua_NPC_SetForcedMarch},
	{"GetForcedMarch", GLua_NPC_GetForcedMarch},
	{"SetAltFire", GLua_NPC_SetAltFire},
	{"GetAltFire", GLua_NPC_GetAltFire},
	{"SetNoAvoid", GLua_NPC_SetNoAvoid},
	{"GetNoAvoid", GLua_NPC_GetNoAvoid},
	{"SetNPCFreeze", GLua_NPC_SetNPCFreeze},
	{"GetNPCFreeze", GLua_NPC_GetNPCFreeze},
	{"SetCrouched", GLua_NPC_SetCrouched},
	{"GetCrouched", GLua_NPC_GetCrouched},
	{"SetEnemy", GLua_NPC_SetEnemy},
	{"GetEnemy", GLua_NPC_GetEnemy},
	{"SetLeader", GLua_NPC_SetLeader},
	{"GetLeader", GLua_NPC_GetLeader},
	{"SetDYaw", GLua_NPC_SetDYaw},
	{"GetDYaw", GLua_NPC_GetDYaw},
	{"SetDPitch", GLua_NPC_SetDPitch},
	{"GetDPitch", GLua_NPC_GetDPitch},
	{"SetViewTarget", GLua_NPC_SetViewTarget},
	{"SetWatchTarget", GLua_NPC_SetWatchTarget},
	{"GetWatchTarget", GLua_NPC_GetWatchTarget},
	{"Remove", GLua_NPC_Remove},
	{"SetAnimLower", GLua_NPC_SetAnimLower},
	{"SetAnimUpper", GLua_NPC_SetAnimUpper},
	{"SetAnimBoth", GLua_NPC_SetAnimBoth},
	{"SetAnimHoldTime", GLua_NPC_SetAnimHoldTime},
	{"SetUseRange", GLua_NPC_SetUseRange},
	{"GetUseRange", GLua_NPC_GetUseRange},
	{NULL, NULL},
};

static const struct GLua_Prop npc_p [] = {
	{"Health", GLua_NPC_Health, GLua_NPC_SetHealth},
	{"MaxHealth", GLua_NPC_MaxHealth, GLua_NPC_SetMaxHealth},
	{"Shield", GLua_NPC_Shield, GLua_NPC_SetShield},
	{"MaxShield", GLua_NPC_MaxShield, GLua_NPC_SetMaxShield},
	{"Entity", GLua_NPC_GetEntity, NULL},
	{"Pos", GLua_NPC_GetPos, GLua_NPC_SetPos},
	{"Origin", GLua_NPC_GetPos, GLua_NPC_SetPos},
	{"Angles", GLua_NPC_GetAngles, GLua_NPC_SetAngles},
	{"Index", GLua_NPC_GetIndex, NULL},
	{"Valid", GLua_NPC_IsValid, NULL},
	{"HFov", GLua_NPC_GetHFov, GLua_NPC_SetHFov},
	{"VFov", GLua_NPC_GetVFov, GLua_NPC_SetVFov},
	{"VisRange", GLua_NPC_GetVisRange, GLua_NPC_SetVisRange},
	{"EarShot", GLua_NPC_GetEarShot, GLua_NPC_SetEarShot},
	{"Aim", GLua_NPC_GetAim, GLua_NPC_SetAim},
	{"Vigilance", GLua_NPC_GetVigilance, GLua_NPC_SetVigilance},
	{"Aggression", GLua_NPC_GetAggression, GLua_NPC_SetAggression},
	{"Evasion", GLua_NPC_GetEvasion, GLua_NPC_SetEvasion},
	{"Reactions", GLua_NPC_GetReactions, GLua_NPC_SetReactions},
	{"FollowDist", GLua_NPC_GetFollowDist, GLua_NPC_SetFollowDist},
	{"WalkSpeed", GLua_NPC_GetWalkSpeed, GLua_NPC_SetWalkSpeed},
	{"RunSpeed", GLua_NPC_GetRunSpeed, GLua_NPC_SetRunSpeed},
	{"Walking", GLua_NPC_GetWalking, GLua_NPC_SetWalking},
	{"Running", GLua_NPC_GetRunning, GLua_NPC_SetRunning},
	{"LookForEnemies", GLua_NPC_GetLookForEnemies, GLua_NPC_SetLookForEnemies},
	{"ChaseEnemies", GLua_NPC_GetChaseEnemies, GLua_NPC_SetChaseEnemies},
	{"NoForce", GLua_NPC_GetNoForce, GLua_NPC_SetNoForce},
	{"NoMindTrick", GLua_NPC_GetNoMindTrick, GLua_NPC_SetNoMindTrick},
	{"NoAcrobatics", GLua_NPC_GetNoAcrobatics, GLua_NPC_SetNoAcrobatics},
	{"DontFire", GLua_NPC_GetDontFire, GLua_NPC_SetDontFire},
	{"FireWeapon", GLua_NPC_GetFireWeapon, GLua_NPC_SetFireWeapon},
	{"ForcedMarch", GLua_NPC_GetForcedMarch, GLua_NPC_SetForcedMarch},
	{"AltFire", GLua_NPC_GetAltFire, GLua_NPC_SetAltFire},
	{"NoAvoid", GLua_NPC_GetNoAvoid, GLua_NPC_SetNoAvoid},
	{"NPCFreeze", GLua_NPC_GetNPCFreeze, GLua_NPC_SetNPCFreeze},
	{"Crouched", GLua_NPC_GetCrouched, GLua_NPC_SetCrouched},
	{"Enemy", GLua_NPC_GetEnemy, GLua_NPC_SetEnemy},
	{"Leader", GLua_NPC_GetLeader, GLua_NPC_SetLeader},
	{"DYaw", GLua_NPC_GetDYaw, GLua_NPC_SetDYaw},
	{"DPitch", GLua_NPC_GetDPitch, GLua_NPC_SetDPitch},
	{"WatchTarget", GLua_NPC_GetWatchTarget, GLua_NPC_SetWatchTarget},
	{"GodMode", GLua_NPC_HasGodMode, GLua_NPC_SetGodMode},
	{"NoKnockback", GLua_NPC_HasNoKnockback, GLua_NPC_SetNoKnockback},
	{"NoTarget", GLua_NPC_HasNoTarget, GLua_NPC_SetNoTarget},
	{"Busy",	GLua_NPC_HasBusy,		GLua_NPC_SetBusy },					//futuza: check if the npc is busy with something else (eg: in a pazaak game, a quest, etc.) and shouldn't be interacted with
	{"NoDebuff", GLua_NPC_HasNoDebuff,	GLua_NPC_SetNoDebuff },
	{"UseRange", GLua_NPC_GetUseRange, GLua_NPC_SetUseRange},
	{NULL,		NULL,						NULL},
};

void GLua_PushNPC(lua_State *L, gentity_t *ent) {
	GLua_Data_Entity_t *data;
	int entNum;
	if (!ent || ent->s.eType != ET_NPC) { // Push a NULL NPC
		data = (GLua_Data_Entity_t *)lua_newuserdata(L, sizeof(GLua_Data_Entity_t));
		data->entNum = 0;
		data->IDCode = g_entities[0].IDCode - 1; // Invalid on purpose
		luaL_getmetatable(L,"NPC");
		lua_setmetatable(L,-2);
		return;
	}
	entNum = ent->s.number;

	if (g_entities[entNum].inuse || g_entities[entNum].LuaUsable) {
		// Ent is valid
		data = (GLua_Data_Entity_t *)lua_newuserdata(L, sizeof(GLua_Data_Entity_t));
		data->entNum = entNum;
		data->IDCode = ent->IDCode;
		luaL_getmetatable(L,"NPC");
		lua_setmetatable(L,-2);
	} else {
		// Invalid entity, push null entity
		data = (GLua_Data_Entity_t *)lua_newuserdata(L, sizeof(GLua_Data_Entity_t));
		data->entNum = entNum;
		data->IDCode = ent->IDCode - 1; // Invalid on purpose
		luaL_getmetatable(L,"NPC");
		lua_setmetatable(L,-2);
	}
	return;
}


///////////////////////////
// NPC spawning
///////////////////////////

/// Define NPC events
static void GLua_NPCEV_OnInit(gentity_t *self, gentity_t *spawner) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnInit");
	GLua_PushEntity(L, spawner);
	if (lua_pcall(L,3,0,0)) {
		trap->Print("GLua: Failed to call npc.OnInit: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnSpawn(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnSpawn");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnSpawn: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnThink(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnThink");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnThink: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnUse(gentity_t *self, gentity_t *other, gentity_t *activator) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnUse");
	GLua_PushEntity(L, other);
	GLua_PushEntity(L, activator);
	if (lua_pcall(L,4,0,0)) {
		trap->Print("GLua: Failed to call npc.OnUse: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnRemove(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnRemove");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnRemove: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnPain(gentity_t *self, gentity_t *attacker, int damage) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnPain");
	GLua_PushEntity(L, attacker);
	lua_pushnumber(L, damage);
	if (lua_pcall(L,4,0,0)) {
		trap->Print("GLua: Failed to call npc.OnPain: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnDie");
	GLua_PushEntity(L, inflictor);
	GLua_PushEntity(L, attacker);
	lua_pushnumber(L, damage);
	lua_pushnumber(L, mod);
	if (lua_pcall(L,6,0,0)) {
		trap->Print("GLua: Failed to call npc.OnDie: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnTouch(gentity_t *self, gentity_t *other, trace_t* tr) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnTouch");
	GLua_PushEntity(L, other);
	if (lua_pcall(L,3,0,0)) {
		trap->Print("GLua: Failed to call npc.OnTouch: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnReached(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnReached");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnReached: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnStuck(gentity_t *self) {	// NAV cant find a path to the navgoal
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnStuck");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnStuck: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnBlocked(gentity_t *self, gentity_t *blocker) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnBlocked");
	GLua_PushEntity(L, blocker);
	if (lua_pcall(L,3,0,0)) {
		trap->Print("GLua: Failed to call npc.OnBlocked: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnAwake(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnAwake");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnAwake: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnAnger(gentity_t *self, gentity_t *enemy) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnAnger");
	GLua_PushEntity(L, enemy);
	if (lua_pcall(L,3,0,0)) {
		trap->Print("GLua: Failed to call npc.OnAnger: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnAttack(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnAttack");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnAttack: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnVictory(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnVictory");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnVictory: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnLostEnemy(gentity_t *self) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnLostEnemy");
	if (lua_pcall(L,2,0,0)) {
		trap->Print("GLua: Failed to call npc.OnLostEnemy: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

void GLua_NPCEV_OnMindTrick(gentity_t *self, gentity_t *user) {
	lua_State *L = GLua_GetState();
	if (!L) return;
	GLua_Push_CallNPCFunc();
	GLua_PushNPC(L, self);
	lua_pushstring(L, "OnMindTrick");
	GLua_PushEntity(L, user);
	if (lua_pcall(L,3,0,0)) {
		trap->Print("GLua: Failed to call npc.OnMindTrick: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	return;
}

/// Main spawner
int GLua_Spawn_NPC(gentity_t* npc, gentity_t *spawner) {
	const char *npcname = spawner->npcscript;
	int valid = GLua_NPCExists(npcname);
	if (!valid) {
		// Not valid, so the npc is not defined in lua
		return 0;
	}
	// Valid NPC ^^
	// Mark the NPC as lua-enabled, load in the lua code and run the OnInit event
	GLua_SpawnNPC(npc, npcname);
	npc->NPC->isLuaNPC = qtrue;
	npc->remove = GLua_NPCEV_OnRemove;
	// Run the OnInit event
	GLua_NPCEV_OnInit(npc, spawner);
	return 1;
}


void GLua_Define_NPC(lua_State *L) {

	STACKGUARD_INIT(L)
	// Defines the NPC object so it can be used

	luaL_newmetatable(L,"NPC");
	luaL_register(L, NULL, npc_m);
	GLua_RegisterProperties(L, npc_p, 0);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_NPC);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"NPC");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}