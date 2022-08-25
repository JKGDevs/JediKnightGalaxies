// Defines the player object and players. namespace

#include "game/g_local.h"
#include "qcommon/q_shared.h"
#include "game/jkg_utilityfunc.h"
#include "glua.h"

#define ConvertVec(a, b) (b[0] = a->x, b[1] = a->y, b[2] = a->z)

static const char *ChatBox_EscapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = (char *)message;
	while (*t && s != cutoff) {
		if (*t == '%') {
			*s = 0x18;
		} else if (*t == '"') {
			*s = 0x17;
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}


GLua_Data_Player_t *GLua_CheckPlayer(lua_State *L, int idx) {
	GLua_Data_Player_t *ply;
	if (!ValidateObject(L, idx, GO_PLAYER)) {
		luaL_typerror(L, idx, "Player");
	}
	ply = (GLua_Data_Player_t *)lua_touserdata(L,idx);
	if (level.clients[ply->clientNum].pers.connected == CON_DISCONNECTED)
		return NULL;
	if (g_entities[ply->clientNum].IDCode != ply->IDCode)
		return NULL;
	return ply;
}

static int GLua_Player_GetID(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;

	lua_pushnumber(L,ply->clientNum);
	return 1;
}

static int GLua_Player_GetName(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;

	lua_pushstring(L,level.clients[ply->clientNum].pers.netname);
	return 1;
}

static int GLua_Player_GetIP(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int len = 0;
	char *pos;
	if (!ply) return 0;
	for (pos = level.clients[ply->clientNum].sess.IP; *pos && *pos != ':'; pos++, len++);


	lua_pushlstring(L, level.clients[ply->clientNum].sess.IP, len);
	return 1;
}

static int GLua_Player_GetIPPort(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;

	lua_pushstring(L,level.clients[ply->clientNum].sess.IP);
	return 1;
}

static int GLua_Player_IsValid(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) {
		lua_pushboolean(L,0);
	} else {
		lua_pushboolean(L,1);
	}
	return 1;
}

static int GLua_Player_GetTable(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;

	GLua_Push_GetEntDataTableFunc(L);
	GLua_PushEntity(L,&g_entities[ply->clientNum]);
	lua_call(L,1,1);
	return 1;
}

static int GLua_Player_Index(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	const char *key = lua_tostring(L,2);
	if (!key) return 0;
	// Continue even if the player is invalid, we'll check that later
	
	if (key[0] == '_' && key[1] == '_') {
		if (ply) {
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
	// That is.. if the player is valid :P
	if (!ply) return 1; // If we get here, there's a nil on top of the stack, so ya :P
	
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

	// Forward to local storage
	GLua_Push_GetEntDataFunc(L);
	GLua_PushEntity(L,&g_entities[ply->clientNum]);
	lua_pushvalue(L,2);
	lua_call(L,2,1);
	return 1;
}

static int GLua_Player_NewIndex(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	const char *key = lua_tostring(L,2);
	if (!ply) return 0;

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
	GLua_PushEntity(L,&g_entities[ply->clientNum]);
	lua_pushvalue(L,2);
	lua_pushvalue(L,3);
	lua_call(L,3,0);
	return 0;
}

static int GLua_Player_Eq(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	GLua_Data_Player_t *ply2 = GLua_CheckPlayer(L, 2);
	
	if (!ply || !ply2) {
		lua_pushboolean(L,0);
		return 1;
	}
	if (ply->clientNum == ply2->clientNum && ply->IDCode == ply2->IDCode) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_Player_SendChat(lua_State *L) {
	char buff[980] = {0}; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int i;
	if (!ply) return 0;
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 2; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L,1);
	}
	
	trap->SendServerCommand(ply->clientNum, va("chat 100 \"%s\"", ChatBox_EscapeChat(&buff[0])));
	return 0;
}

static int GLua_Player_SendFadedChat(lua_State *L) {
	char buff[980] = {0}; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int fadeLevel;
	int i;
	if (!ply) return 0;
	fadeLevel = lua_tointeger(L,2);
	if (fadeLevel > 100) {
		fadeLevel = 100;
	} else if (fadeLevel < 15) {
		fadeLevel = 15;
	}
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 3; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L,1);
	}
	trap->SendServerCommand(ply->clientNum, va("chat %i \"%s\"", fadeLevel, ChatBox_EscapeChat(&buff[0])));
	return 0;
}

static int GLua_Player_SendCenterPrint(lua_State *L) {
	char buff[980] = {0}; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int i;
	if (!ply) return 0;
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 2; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L,1);
	}
	trap->SendServerCommand(ply->clientNum, va("cp \"%s\n\"", &buff[0]));
	return 0;
}

static int GLua_Player_SendCenterPrintAll(lua_State *L) {
	char buff[980] = { 0 }; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int i;
	if (!ply) return 0;
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 2; i <= args; i++) {
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L, -1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L, 1);
	}
	trap->SendServerCommand(-1, va("cp \"%s\n\"", &buff[0]));
	return 0;
}

static int GLua_Player_SendPrint(lua_State *L) {
	char buff[980] = {0}; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int i;
	if (!ply) return 0;
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 2; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L,1);
	}
	trap->SendServerCommand(ply->clientNum, va("print \"%s\n\"", &buff[0]));
	return 0;
}

//incomplete function, need to probably request the map list from the server itself with a cmd (function does nothing atm)
/*static int GLua_Player_GetMapList(lua_State *L) 
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	char map[24] = "";
	std::string maplist = "";
	for (int i = 0; i < level.arenas.num; i++)
	{
		Q_strncpyz(map, Info_ValueForKey(level.arenas.infos[i], "map"), sizeof(map));
		Q_StripColor(map);
		maplist.append(map);
		maplist.append(",");
	}

	const char *maps = maplist.c_str();
	lua_pushstring(L, maps);
	return 1;
}*/

static int GLua_Player_SendCommand(lua_State *L) { // Use with caution!
	char buff[980] = {0}; // (not 1024, to keep the command from being oversize (cmd included)
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int args = lua_gettop(L);
	const char *res;
	int i;
	if (!ply) return 0;
	// Lets do this a lil different, concat all args and use that as the message ^^
	GLua_Push_ToString(L); // Ref to tostring (instead of a global lookup, in case someone changes it)
	for (i = 2; i <= args; i++) {
		lua_pushvalue(L,-1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1); // Assume this will never error out
		res = lua_tostring(L,-1);
		if (res) {
			Q_strcat(&buff[0], 980, res);
		}
		lua_pop(L,1);
	}
	trap->SendServerCommand(ply->clientNum, va("%s", &buff[0]));
	return 0;
}

static int GLua_Player_Kill(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *g_player;

	if (!ply) return 0; //template :P

	g_player = &g_entities[ply->clientNum];

	//If the player is a spectator, don't do anything.
	if (g_player->client->sess.sessionTeam == TEAM_SPECTATOR || g_player->health <= 0 || g_player->client->tempSpectate > level.time)
		return 0;

	//override godmode flag.
	g_player->flags &= ~FL_GODMODE;

	//assign new health value.
	g_player->client->ps.stats[STAT_HEALTH] = g_player->health = -999;

	//final step.
	player_die(g_player, g_player, g_player, 100000, MOD_SUICIDE);

	return 0;
}

static int GLua_Player_Disintegrate(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *g_player;

	if (!ply) return 0; //template :P

	g_player = &g_entities[ply->clientNum];

	//If the player is a spectator, don't do anything.
	if (g_player->client->sess.sessionTeam == TEAM_SPECTATOR || g_player->health <= 0 || g_player->client->tempSpectate > level.time)
		return 0;

	//override godmode flag.
	g_player->flags &= ~FL_GODMODE;

	//assign new health value.
	g_player->client->ps.stats[STAT_HEALTH] = g_player->health = -999;

	// Disintgrate the player	
	VectorClear( g_player->client->ps.lastHitLoc );
	VectorClear( g_player->client->ps.velocity );

	g_player->client->ps.eFlags	|= EF_DISINTEGRATION;
	g_player->r.contents = 0;

	//final step.
	player_die(g_player, g_player, g_player, 100000, MOD_SUICIDE);

	return 0;
}

static int GLua_Player_GetPos(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	vec3_t orig;
	if (!ply) return 0;
	
	VectorCopy(level.clients[ply->clientNum].ps.origin, orig);
	GLua_PushVector(L, orig[0], orig[1], orig[2]);
	return 1;
}

static int GLua_Player_SetPos(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	GLuaVec_t * orig = GLua_CheckVector(L, 2);
	int rawtele = lua_toboolean(L,3);
	gentity_t *ent;
	vec3_t neworigin;
	if (!ply) return 0;
	memcpy(&neworigin, orig, 12);

	ent = &g_entities[ply->clientNum];

	VectorCopy(neworigin, ent->client->ps.origin);
	VectorCopy(neworigin, ent->r.currentOrigin);
	VectorClear (ent->client->ps.velocity);

	if (rawtele) {	// Dont do stuff like knockback, height correction and teleport flagging
		return 0;	// This would be used in cases where the following things cause complications
	}				// For example when teleporting along with moving objects
	ent->client->ps.origin[2] += 1;

	
	ent->client->ps.pm_time = 160;		// hold time
	ent->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	
	ent->client->ps.eFlags ^= EF_TELEPORT_BIT;

	return 0;
}

static int GLua_Player_GetAngles(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];

	GLua_PushVector(L, ent->client->ps.viewangles[0], ent->client->ps.viewangles[1], ent->client->ps.viewangles[2]);
	return 1;
}

static int GLua_Player_SetAngles(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	GLuaVec_t * angs = GLua_CheckVector(L, 2);
	gentity_t *ent;
	vec3_t newangs;
	if (!ply) return 0;
	memcpy(&newangs, angs, 12);

	ent = &g_entities[ply->clientNum];


	SetClientViewAngle( ent, newangs );

	return 0;
}

static int GLua_Player_Teleport(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	GLuaVec_t * orig = GLua_CheckVector(L, 2);
	GLuaVec_t * angs = NULL;
	vec3_t org, ang;
	if (!ply) return 0;
	ConvertVec(orig, org);
	if (!lua_isnoneornil(L,3)) {
		angs = GLua_CheckVector(L, 3);
		ConvertVec(angs, ang);
	} else {
		//memcpy(angs, level.clients[ply->clientNum].ps.viewangles, 12);
		VectorCopy(level.clients[ply->clientNum].ps.viewangles, ang);
	}
	
	TeleportPlayer(&g_entities[ply->clientNum], org, ang);
	return 0;
}

static int GLua_Player_Kick(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	const char *reason = lua_tostring(L,2);
	if (!ply) return 0;
	if (!reason) reason = "was kicked";
	trap->DropClient(ply->clientNum, reason);
	return 0;
}

static int GLua_Player_SetTeam(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	const char *team = luaL_checkstring(L,2);
	if (!ply) return 0;
	
	SetTeam(&g_entities[ply->clientNum], (char *)team);
	return 0;
}

static int GLua_Player_GetTeam(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].sess.sessionTeam);
	return 1;
}


static int GLua_Player_ToString(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) {
		lua_pushstring(L,"[ NULL Player ]");
	} else {
		lua_pushstring(L,va("Player [%i: %s^7]", ply->clientNum, level.clients[ply->clientNum].pers.netname));
	}
	return 1;
}

static int GLua_Player_Spawn(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int silent = lua_toboolean(L,2);
	if (!ply) return 0;
	if (silent) {
		ClientSpawn(&g_entities[ply->clientNum], qfalse);
	} else {
		respawn(&g_entities[ply->clientNum]); // <-- this one does the tele effect
	}
	return 0;
}

static int GLua_Player_MaxHealth_Get(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.stats[STAT_MAX_HEALTH]);
	return 1;
}

static int GLua_Player_MaxHealth_Set(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.stats[STAT_MAX_HEALTH] = level.clients[ply->clientNum].pers.maxHealth = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_Player_MaxStamina_Get(lua_State* L) {
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].ps.stats[STAT_MAX_STAMINA]);
	return 1;
}

static int GLua_Player_MaxStamina_Set(lua_State* L) {
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.stats[STAT_MAX_STAMINA] = level.clients[ply->clientNum].pers.maxStamina = luaL_checkinteger(L, 2);
	return 0;
}


static int GLua_Player_MaxShield_Get(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.stats[STAT_MAX_SHIELD]);
	return 1;
}

static int GLua_Player_MaxShield_Set(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.stats[STAT_MAX_SHIELD] = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_Player_Shield_Get(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.stats[STAT_SHIELD]);
	return 1;
}

static int GLua_Player_Shield_Set(lua_State* L) {
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.stats[STAT_SHIELD] = g_entities[ply->clientNum].playerState->stats[STAT_SHIELD] = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_Player_Health_Get(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.stats[STAT_HEALTH]);
	return 1;
}

static int GLua_Player_Health_Set(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.stats[STAT_HEALTH] = g_entities[ply->clientNum].health = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_Player_Stamina_Get(lua_State* L) {
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].ps.forcePower);
	return 1;
}

static int GLua_Player_Stamina_Set(lua_State* L) {
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.forcePower = g_entities[ply->clientNum].playerState->forcePower = luaL_checkinteger(L, 2);
	return 0;
}

static int GLua_Player_CanUseCheats_Get(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);

	if(!ply) return 0;

	gentity_t *ent = &g_entities[ply->clientNum];

	lua_pushboolean(L, ent->client->sess.canUseCheats);
	return 1;
}

static int GLua_Player_CanUseCheats_Set(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);

	if(!ply) return 0;

	gentity_t *ent = &g_entities[ply->clientNum];

	ent->client->sess.canUseCheats = lua_toboolean(L, 2);
	return 0;
}
//eezstreet end

static int GLua_Player_GetEyeTrace(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	vec3_t src;
	vec3_t dest;
	vec3_t vf;
	trace_t trace;
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	VectorCopy(ent->client->ps.origin, src);
	src[2] += ent->client->ps.viewheight;

	AngleVectors( ent->client->ps.viewangles, vf, NULL, NULL );

	// Second (optional) arg: Range to check
	VectorMA( src, luaL_optint(L, 2, 131072), vf, dest );

	//Trace ahead to find a valid target
	trap->Trace( &trace, src, vec3_origin, vec3_origin, dest, ent->s.number, MASK_OPAQUE|CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_ITEM|CONTENTS_CORPSE, 0, 0, 0 );
	
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

static int GLua_Player_GetEntity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	GLua_PushEntity(L, &g_entities[ply->clientNum]);
	return 1;
}

// TODO: Make sure these two functions are getting and returning proper weapon & variation, see Ammo set&get functions for similar code
static int GLua_Player_GetWeapon(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].ps.weapon);
	return 1;
}

static int GLua_Player_SetWeapon(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int weapon = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS) return 0;
	// Check if player has the weapon in question
	if (!(level.clients[ply->clientNum].ps.stats[STAT_WEAPONS] & (1 << weapon))) return 0;
	level.clients[ply->clientNum].ps.weapon = weapon;
	level.clients[ply->clientNum].pers.cmd.weapon = BG_GetWeaponIndexFromClass (weapon, 0);
	trap->SendServerCommand(ply->clientNum, va("chw %i", weapon));
	return 0;
}

static int GLua_Player_Damage(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *inflictor = NULL, *attacker = NULL;
	GLuaVec_t *dir = NULL, *point = NULL;
	vec3_t dir2 = {0,0,0}, point2 = {0,0,0};
	int damage, dflags, mod;
	if (!ply) return 0;
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

	if (lua_isnumber(L, 8))
		mod = lua_tointeger(L, 8);

	else if (lua_isstring(L, 8))
		mod = JKG_GetMeansOfDamageIndex(lua_tostring(L, 8));

	else
		mod = 0;

	G_Damage(&g_entities[ply->clientNum], inflictor, attacker, dir2, point2, damage, dflags, mod);
	return 0;
}

static int GLua_Player_GiveForce(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	level.clients[ply->clientNum].ps.fd.forcePowersKnown |=  (1 << force);
	return 0;
}

static int GLua_Player_TakeForce(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	level.clients[ply->clientNum].ps.fd.forcePowersKnown &=  ~(1 << force);
	level.clients[ply->clientNum].ps.fd.forcePowerLevel[force] = FORCE_LEVEL_0;
	level.clients[ply->clientNum].ps.fd.forcePowerBaseLevel[force] = FORCE_LEVEL_0;
	return 0;
}

static int GLua_Player_HasForce(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	lua_pushboolean(L,level.clients[ply->clientNum].ps.fd.forcePowersKnown & (1 << force));
	return 1;
}

static int GLua_Player_SetForceLevel(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	int newlevel = luaL_checkinteger(L,3);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	if (newlevel < 0 || newlevel > 3) return 0; // level > 3 is subject to change if we go for lvl 5
	if (!(level.clients[ply->clientNum].ps.fd.forcePowersKnown & (1 << force))) return 0;
	level.clients[ply->clientNum].ps.fd.forcePowerLevel[force] = newlevel;
	level.clients[ply->clientNum].ps.fd.forcePowerBaseLevel[force] = newlevel;
	return 0;
}

static int GLua_Player_GetForceLevel(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	if (!(level.clients[ply->clientNum].ps.fd.forcePowersKnown & (1 << force))) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.fd.forcePowerLevel[force]);
	return 1;
}

// NEW ADMIN SYSTEM
static int GLua_Player_SetAccount(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	const char *account = luaL_checkstring(L, 2);
	if(!ply) return 0;
	if(!account) return 0;
	Q_strncpyz(level.clients[ply->clientNum].pers.account, account, sizeof(level.clients[ply->clientNum].pers.account));
	return 0;
}

static int GLua_Player_GetAccount(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if(!ply)
	{
		lua_pushnil(L);
		return 1;
	}

	lua_pushstring( L, level.clients[ply->clientNum].pers.account );
	return 1;
}

extern qboolean CCmd_Execute(int clientNum, const char *command);
extern bool jkgIgnoreFloodProtect;
static int GLua_Player_ExecuteChatCommand(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if(!ply)
	{
		return 0;
	}

	const char *cmd = luaL_checkstring(L, 2);
	if(!cmd)
	{
		return 0;
	}

	jkgIgnoreFloodProtect = true;
	CCmd_Execute(ply->clientNum, cmd);
	jkgIgnoreFloodProtect = false;

	return 0;
}

static int GLua_Player_SetActiveForce(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int force = luaL_checkinteger(L,2);
	if (!ply) return 0;
	if (force < FP_FIRST || force >= NUM_FORCE_POWERS) return 0;
	if (!(level.clients[ply->clientNum].ps.fd.forcePowersKnown & (1 << force))) return 0;
	level.clients[ply->clientNum].ps.fd.forcePowerSelected = force;
	return 0;
}

static int GLua_Player_GetActiveForce(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushinteger(L,level.clients[ply->clientNum].ps.fd.forcePowerSelected);
	return 1;
}

static int GLua_Player_StripForce(lua_State *L) {
	int i;
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	level.clients[ply->clientNum].ps.fd.forcePowersKnown = 0;
	for (i = FP_FIRST; i < NUM_FORCE_POWERS; i++) {
		level.clients[ply->clientNum].ps.fd.forcePowerLevel[i] = FORCE_LEVEL_0;
		level.clients[ply->clientNum].ps.fd.forcePowerBaseLevel[i] = FORCE_LEVEL_0;
	}
	level.clients[ply->clientNum].ps.fd.forcePowerSelected = 0;
	return 0;
}


static int GLua_Player_IsConnected(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	if (!ply) return 0;
	lua_pushboolean(L,level.clients[ply->clientNum].pers.connected == CON_CONNECTED);
	return 1;
}

static int GLua_Player_BroadcastEntity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent = GLua_CheckEntity(L, 2);
	int remove = lua_toboolean(L,3);
	if (!ply) return 0;
	if (!ent) return 0;
	if (remove) {
		ent->r.broadcastClients[ply->clientNum / 32] &= ~(1 << (ply->clientNum % 32));
	} else {
		ent->r.broadcastClients[ply->clientNum / 32] |= (1 << (ply->clientNum % 32));
	}
	return 0;
}

static int GLua_Player_SetGodMode(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L,2);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->flags |= FL_GODMODE;
	} else {
		ent->flags &= ~FL_GODMODE;
	}
	return 0;
}

static int GLua_Player_HasGodMode(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->flags & FL_GODMODE) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_Player_SetNoClip(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L,2);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->client->noclip = qtrue;
	} else {
		ent->client->noclip = qfalse;
	}
	return 0;
}

static int GLua_Player_HasNoClip(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->client->noclip) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

//futuza: adding NoKnockback for players
static int GLua_Player_SetNoKnockback(lua_State *L) {	
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L, 2);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->flags |= FL_NO_KNOCKBACK;
	}
	else {
		ent->flags &= ~FL_NO_KNOCKBACK;
	}
	return 0;
}

static int GLua_Player_HasNoKnockback(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->flags & FL_NO_KNOCKBACK) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int GLua_Player_SetBusy(lua_State* L)
{
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L, 2);
	gentity_t* ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->flags |= FL_BUSYMODE;
	}
	else {
		ent->flags &= ~FL_BUSYMODE;
	}
	return 0;
}

static int GLua_Player_HasBusy(lua_State* L)
{
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 1);
	gentity_t* ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->flags & FL_BUSYMODE) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int GLua_Player_SetNoDebuff(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L, 2);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->flags |= FL_NO_DEBUFF;
	}
	else {
		ent->flags &= ~FL_NO_DEBUFF;
	}
	return 0;
}

static int GLua_Player_HasNoDebuff(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->flags & FL_NO_DEBUFF) {
		lua_pushboolean(L, 1);
	}
	else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

extern void G_BuffEntity(gentity_t* ent, gentity_t* buffer, int buffID, float intensity, int duration);
static int GLua_Player_AddBuff(lua_State* L) 
{
	//L1 = Player (required), L2 = Buffname (required), L3 = duration, L4 = intensity
	if (lua_isnoneornil(L, 3)) return luaL_error(L, "No buff name provided");
	GLua_Data_Player_t* ply = GLua_CheckPlayer(L, 2);
	if (!ply) return 0;
	gentity_t* ent;
	ent = &g_entities[ply->clientNum];

	int duration = 5000; // default to 5 seconds
	float intensity = 1.0f; // default to normal intensity
	int buffID = -1;
	
	const char *buffName = luaL_checkstring(L, 3);
	if(sizeof(buffName) > BUFF_NAME_LEN)
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

		if(lua_gettop(L) > 4)
			intensity = lua_tonumber(L, 5);
	}

	G_BuffEntity(ent, ent, buffID, intensity, duration);
	return 0;
}

static int GLua_Player_SetNoTarget(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int active = lua_toboolean(L,2);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (active) {
		ent->flags |= FL_NOTARGET;
	} else {
		ent->flags &= ~FL_NOTARGET;
	}
	return 0;
}

static int GLua_Player_HasNoTarget(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (ent->flags & FL_NOTARGET) {
		lua_pushboolean(L,1);
	} else {
		lua_pushboolean(L,0);
	}
	return 1;
}

static int GLua_Player_SetCinematicMode(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int mode = lua_toboolean(L,2);
	if (!ply) return 0;
	level.clients[ply->clientNum].InCinematic = mode;
	return 0;
}

void JKG_PlayerIsolate(int client1, int client2);
void JKG_PlayerReveal(int client1, int client2);

static int GLua_Player_SetIsolate(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	GLua_Data_Player_t *ply2 = GLua_CheckPlayer(L, 2);
	int isolate = lua_toboolean(L,3);
	if (!ply) return 0;
	if (!ply2) {
		return luaL_error(L, "Bad player provided");
	}
	if (ply->clientNum == ply2->clientNum) {
		return luaL_error(L, "Must be different clients");
	}

	if (isolate) {
		JKG_PlayerIsolate(ply->clientNum, ply2->clientNum);
	} else {
		JKG_PlayerReveal(ply->clientNum, ply2->clientNum);
	}
	return 0;
}

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

static int GLua_Player_GetVelocity(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	vec3_t vel;
	if (!ply) return 0;

	VectorCopy(level.clients[ply->clientNum].ps.velocity, vel);
	GLua_PushVector(L, vel[0], vel[1], vel[2]);
	return 1;
}

static int GLua_Player_GetUserInfo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	char buff[4096] = {0};
	const char *key = luaL_checkstring(L,2);
	if (!ply) return 0;

	trap->GetUserinfo(ply->clientNum, buff, sizeof(buff));
	lua_pushstring(L, Info_ValueForKey(buff, key));

	return 1;
}


static int GLua_Player_IsHacking(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	if (!ply) return 0;
	lua_pushboolean(L,level.clients[ply->clientNum].isHacking);
	return 1;
}

static int GLua_Player_FinishedHacking(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	if (!ply) return 0;
	lua_pushboolean(L,level.clients[ply->clientNum].ps.hackingTime < level.time);
	return 1;
}

static int GLua_Player_StartHacking(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int time;
	if (!ply) return 0;
	if (lua_isnoneornil(L,2)) {
		level.clients[ply->clientNum].isHacking = 0; 
		level.clients[ply->clientNum].ps.hackingTime = 0;
	} else {
		ent = GLua_CheckEntity(L,2);
		time = luaL_checkinteger(L,3);
		if (time < 0) {
			time = 0;
			trap->Print("Warning: GLua_Player_StartHacking: time < 0! Clamped to 0.\n");
		} else if (time > 60000) {
			time = 60000;
			trap->Print("Warning: GLua_Player_StartHacking: time > 60000! Clamped to 60000.\n");
		}
		level.clients[ply->clientNum].isHacking = ent->s.number;
		VectorCopy(level.clients[ply->clientNum].ps.viewangles, level.clients[ply->clientNum].hackingAngles);
		level.clients[ply->clientNum].ps.hackingTime = level.time + time;
		level.clients[ply->clientNum].ps.hackingBaseTime = time;
	}
	return 0;
}

static int GLua_Player_SetAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	const char *ammoString = lua_tostring(L, 2);
	int amt = luaL_checkint(L,3);
	const ammo_t *ammo;
	if (!ammoString) return 0;
	if (!ply) return 0;
	
	if (amt < 0) {
		amt = 0;
	}
	
	ammo = BG_GetAmmo(ammoString);

	g_entities[ply->clientNum].client->ammoTable[ammo->ammoIndex] = amt;
	
	return 0;
}

static int GLua_Player_GetAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	const char *ammoString = lua_tostring(L, 2);
	const ammo_t *ammo;
	if (!ply) return 0;
	if (!ammoString) return 0;
	
	ammo = BG_GetAmmo(ammoString);

	lua_pushinteger(L, g_entities[ply->clientNum].client->ammoTable[ammo->ammoIndex]);
	return 1;
}

static int GLua_Player_ModifyAmmo(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	const char *ammoString = lua_tostring(L, 2);
	const int amount = luaL_checkint(L, 3);
	const ammo_t *ammo;

	if (!ply) return 0;
	if(!ammoString) return 0;

	ammo = BG_GetAmmo(ammoString);

	g_entities[ply->clientNum].client->ammoTable[ammo->ammoIndex] = amount;
	return 1;
}

static int GLua_Player_SetClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int weapon = luaL_checkint(L,2);
	int var = luaL_optint(L,3,0);
	int amt = luaL_checkint(L,4);
	int firemode = luaL_optint(L, 5, 0);

	if (!ply) return 0;

	if (weapon < 0 || weapon >= MAX_WEAPONS) {
		return 0;
	}

	weaponData_t* wp = GetWeaponData(weapon, var);

	if (amt < 0) {
		amt = 0;
	} else if (amt > wp->firemodes[firemode].clipSize) {
		amt = wp->firemodes[firemode].clipSize;
	}

	if (firemode < 0)
		firemode = 0;

	g_entities[ply->clientNum].client->clipammo[BG_GetWeaponIndex(weapon, var)][firemode] = amt;
	return 0;
}

static int GLua_Player_StripClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int i,j;
	if (!ply) return 0;
	for (i=0; i < 256; i++) {
		for(j=0; j < MAX_FIREMODES; j++)
			level.clients[ply->clientNum].clipammo[i][j] = 0;
	}
	return 0;
}

static int GLua_Player_StripAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L, 1);
	int i;
	if (!ply) return 0;
	// TODO: use proper ammo array here
	for (i=0; i < AMMO_MAX; i++) {
		level.clients[ply->clientNum].ammoTable[i] = 0;
	}
	return 0;
}


static int GLua_Player_GetClipAmmo(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int weapon = luaL_checkint(L,2);
	int variation = luaL_checkint(L,3);
	int weaponIndex;
	int fireMode;

	if (!ply) return 0;

	if (weapon < 0 || weapon >= MAX_WEAPONS) {
		return 0;
	}

	weaponIndex = BG_GetWeaponIndex(weapon, variation);
	fireMode = g_entities[ply->clientNum].client->firingModes[weaponIndex];

	lua_pushinteger(L, g_entities[ply->clientNum].client->clipammo[weaponIndex][fireMode]);
	return 1;
}

extern stringID_table_t animTable [MAX_ANIMATIONS+1];

static int GLua_Player_SetAnimLower(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int anim;
	if (!ply) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	ent = &g_entities[ply->clientNum];
	G_SetAnim(ent, NULL, SETANIM_LEGS, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE));
	return 0;
}

static int GLua_Player_SetAnimUpper(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int anim;
	if (!ply) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	ent = &g_entities[ply->clientNum];
	G_SetAnim(ent, NULL, SETANIM_TORSO, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE));
	return 0;
}

static int GLua_Player_SetAnimBoth(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int anim;
	if (!ply) return 0;
	if (lua_isstring(L,2)) {
		anim = GetIDForString(animTable, lua_tostring(L,2));
	} else {
		anim = lua_tointeger(L,2);
	}
	if (anim < 0 || anim >= MAX_ANIMATIONS) {
		return 0;
	}
	ent = &g_entities[ply->clientNum];
	G_SetAnim(ent, NULL, SETANIM_BOTH, anim, luaL_optinteger(L, 3, SETANIM_FLAG_RESTART|SETANIM_FLAG_HOLD|SETANIM_FLAG_OVERRIDE));
	return 0;
}

void BG_SetTorsoAnimTimer(playerState_t *ps, int time );
void BG_SetLegsAnimTimer(playerState_t *ps, int time);
static int GLua_Player_SetAnimHoldTime(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int section = lua_tointeger(L,2);
	int time = lua_tointeger(L,3);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (!section || section == 1) {
		BG_SetTorsoAnimTimer(&ent->client->ps, time);
	}
	if (!section || section == 2) {
		BG_SetLegsAnimTimer(&ent->client->ps, time);
	}
	if (!section || section == 3) {
		BG_SetTorsoAnimTimer(&ent->client->ps, time);
		BG_SetLegsAnimTimer(&ent->client->ps, time);
	}
	return 0;
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
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->pmfreeze);
	return 1;
}

static int GLua_Player_SetNoMove(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (enable) {
		ent->client->pmnomove = qtrue;
	} else {
		ent->client->pmnomove = qfalse;
	}
	return 0;
}

static int GLua_Player_GetNoMove(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->pmnomove);
	return 1;
}

static int GLua_Player_InDeathcam(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	if (!ply) return 0;
	lua_pushboolean(L, level.clients[ply->clientNum].deathcamTime != 0);
	return 1;
}


static int GLua_Player_SetNoDismember(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (enable) {
		ent->client->noDismember = qtrue;
	} else {
		ent->client->noDismember = qfalse;
	}
	return 0;
}

static int GLua_Player_GetNoDismember(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->noDismember);
	return 1;
}

static int GLua_Player_SetNoDisintegrate(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (enable) {
		ent->client->noDisintegrate = qtrue;
	} else {
		ent->client->noDisintegrate = qfalse;
	}
	return 0;
}

static int GLua_Player_GetNoDisintegrate(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->noDisintegrate);
	return 1;
}

static int GLua_Player_SetNoDrops(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int enable = lua_toboolean(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	if (enable) {
		ent->client->noDrops = qtrue;
	} else {
		ent->client->noDrops = qfalse;
	}
	return 0;
}

static int GLua_Player_GetNoDrops(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->noDrops);
	return 1;
}


static int GLua_Player_SetCustomTeam(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int team = lua_tointeger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	
	ent->client->pers.customteam = team;
	ClientUserinfoChanged(ply->clientNum);
	return 0;
}

static int GLua_Player_GetCustomTeam(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	lua_pushboolean(L, ent->client->pers.customteam);
	return 1;
}

static int GLua_Player_ServerTransfer(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	const char *target = luaL_checkstring(L,2);
	const char *flags = luaL_optstring(L,3, "");
	const char *connmsg = luaL_optstring(L,4, "");

	if (!ply) return 0;
	trap->SendServerCommand(ply->clientNum, va("svr \"%s\" \"%s\" \"%s\"", target, flags, connmsg));
	level.clients[ply->clientNum].customDisconnectMsg = 1; // TODO: Use enums for this
	// We need a re-think on client server redirection, the current implementation depends on the
	// client responding to our command and redirecting by itself. We can't guarantee that the client
	// will disconnect. That's why kicks are done server-side of course and force the client off.
	// What I'm suggesting is simply setting the parameters with the svr server command then using
	// trap->DropClient to drop the player off the server with the added bonus of not needing a hook to show
	// a custom disconnect message... even though that's already made. - Didz
	return 0;
}


static int GLua_Player_GetAdminRank(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);

	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].sess.adminRank);
	return 1;
}

// eezstreet add -- Do us a solid and add some stuff for adding credits m8, thx

static int GLua_Player_GetCreditCount(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);

	if (!ply) return 0;
	lua_pushinteger(L, level.clients[ply->clientNum].ps.credits);
	return 1;
}

static int GLua_Player_SetCreditCount(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int setTo = lua_tointeger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	
	ent->client->ps.credits = setTo;
	return 1;
}

static int GLua_Player_ModifyCreditCount(lua_State *L) {
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	int modify = lua_tointeger(L,2);
	if (!ply) return 0;
	ent = &g_entities[ply->clientNum];
	
	ent->client->ps.credits += modify;
	return 1;
}

static int GLua_Player_GetCurrentGunAmmoType(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	gentity_t *ent;
	//weaponData_t *wp; //unused right now

	if(!ply || ply->clientNum < 0 || ply->clientNum > MAX_CLIENTS) return 0;
	ent = &g_entities[ply->clientNum];
	if(!ent) return 0;

	lua_pushinteger(L, ent->client->ps.ammoType);

	return 1;
}

static int GLua_Player_PossessingItem(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int itemID = lua_tointeger(L,2);
	gentity_t *ent;

	if(!ply) return 0;

	ent = &g_entities[ply->clientNum];
	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		if (it->id->itemID == itemID) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	lua_pushboolean(L, 0);	// return qfalse
	return 1;
}

static int GLua_Player_PossessingWeapon(lua_State *L)
{
	GLua_Data_Player_t *ply = GLua_CheckPlayer(L,1);
	int weapon = lua_tointeger(L, 2);
	int variation = lua_tointeger(L, 3);
	gentity_t *ent;

	if(!ply) return 0;

	ent = &g_entities[ply->clientNum];
	if(!ent) return 0;

	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		if (it->id->itemType != ITEM_WEAPON) {
			continue;
		}
		if (it->id->weaponData.weapon == weapon && it->id->weaponData.variation == variation) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	lua_pushboolean(L, 0);	// return qfalse
	return 1;
}

/**************************************************
* player_m
*
* Methods of the player class
**************************************************/

static const struct luaL_reg player_m [] = {
	/* Metamethods */
	{"__index", GLua_Player_Index},
	{"__newindex", GLua_Player_NewIndex},
	{"__eq", GLua_Player_Eq},
	{"__tostring", GLua_Player_ToString},
	/* Normal methods */
	{"GetTable", GLua_Player_GetTable},
	{"GetID", GLua_Player_GetID},
	{"GetName", GLua_Player_GetName},
	{"GetIP", GLua_Player_GetIP},
	{"GetIPPort", GLua_Player_GetIPPort},
	{"IsValid", GLua_Player_IsValid},
	{"SendChat", GLua_Player_SendChat},
	{"SendFadedChat", GLua_Player_SendFadedChat},
	{"SendCenterPrint", GLua_Player_SendCenterPrint},
	{"SendCenterPrintAll", GLua_Player_SendCenterPrintAll},
	{"SendPrint", GLua_Player_SendPrint},
	//{"GetMapList", GLua_Player_GetMapList},
	{"SendCommand", GLua_Player_SendCommand},
	{"Kill", GLua_Player_Kill},
	{"Disintegrate", GLua_Player_Disintegrate},
	{"SetPos", GLua_Player_SetPos},
	{"GetPos", GLua_Player_GetPos},
	{"SetOrigin", GLua_Player_SetPos},
	{"GetOrigin", GLua_Player_GetPos},
	{"SetAngles", GLua_Player_SetAngles},
	{"GetAngles", GLua_Player_GetAngles},
	{"Teleport", GLua_Player_Teleport},
	{"Kick", GLua_Player_Kick},
	{"SetTeam", GLua_Player_SetTeam},
	{"GetTeam", GLua_Player_GetTeam},
	{"Spawn", GLua_Player_Spawn},
	//{"Health", GLua_Player_Health},
	//{"MaxHealth", GLua_Player_MaxHealth},
	//{"MaxShield", GLua_Player_MaxShield},
	//{"Shield", GLua_Player_Shield},
	//{"SetHealth", GLua_Player_SetHealth},
	//{"SetMaxHealth", GLua_Player_SetMaxHealth},
	//{"SetMaxShield", GLua_Player_SetMaxShield},
	//{"SetShield", GLua_Player_SetShield},
	{"GetEyeTrace", GLua_Player_GetEyeTrace},
	{"GetEntity", GLua_Player_GetEntity},
	{"GetWeapon", GLua_Player_GetWeapon},
	{"SetWeapon", GLua_Player_SetWeapon},
	{"Damage", GLua_Player_Damage},
	{"AddBuff", GLua_Player_AddBuff},
	{"GiveForce", GLua_Player_GiveForce},
	{"TakeForce", GLua_Player_TakeForce},
	{"HasForce", GLua_Player_HasForce},
	{"StripForce", GLua_Player_StripForce},
	{"SetForceLevel", GLua_Player_SetForceLevel},
	{"GetForceLevel", GLua_Player_GetForceLevel},
	{"SetActiveForce", GLua_Player_SetActiveForce},
	{"GetActiveForce", GLua_Player_GetActiveForce},
	//{"IsConnected", GLua_Player_IsConnected},
	{"BroadcastEntity", GLua_Player_BroadcastEntity},
	//{"SetGodMode", GLua_Player_SetGodMode},
	//{"HasGodMode", GLua_Player_HasGodMode},
	//{"SetNoClip", GLua_Player_SetNoClip},
	//{"HasNoClip", GLua_Player_HasNoClip},
	//{"SetNoTarget", GLua_Player_SetNoTarget},
	//{"HasNoTarget", GLua_Player_HasNoTarget},
	{"SetCinematicMode", GLua_Player_SetCinematicMode},
	{"SetIsolate", GLua_Player_SetIsolate},
	{"SetVelocity", GLua_Player_SetVelocity},
	{"AddVelocity", GLua_Player_AddVelocity},
	{"GetVelocity", GLua_Player_GetVelocity},
	{"GetUserInfo", GLua_Player_GetUserInfo},
	//{"IsHacking", GLua_Player_IsHacking},  //use ply.Hacking now instead (part of player class instead of function)
	{"FinishedHacking", GLua_Player_FinishedHacking},
	{"StartHacking", GLua_Player_StartHacking},	
	{"SetAmmo", GLua_Player_SetAmmo},
	{"GetAmmo", GLua_Player_GetAmmo},
	{"ModifyAmmo", GLua_Player_ModifyAmmo},
	{"StripClipAmmo", GLua_Player_StripClipAmmo},
	{"StripAmmo", GLua_Player_StripAmmo},
	{"SetClipAmmo", GLua_Player_SetClipAmmo},
	{"GetClipAmmo", GLua_Player_GetClipAmmo},
	{"SetAnimLower", GLua_Player_SetAnimLower},
	{"SetAnimUpper", GLua_Player_SetAnimUpper},
	{"SetAnimBoth", GLua_Player_SetAnimBoth},
	{"SetAnimHoldTime", GLua_Player_SetAnimHoldTime},
	{"SetKnockback", GLua_Player_SetKnockback},
	//{"SetGravity", GLua_Player_SetGravity},
	//{"GetGravity", GLua_Player_GetGravity},
	//{"SetUndying", GLua_Player_SetUndying},
	//{"GetUndying", GLua_Player_GetUndying},
	//{"SetInvulnerable", GLua_Player_SetInvulnerable},
	//{"GetInvulnerable", GLua_Player_GetInvulnerable},
	{"SetFreeze", GLua_Player_SetFreeze},
	{"GetFreeze", GLua_Player_GetFreeze},
	{"SetNoMove", GLua_Player_SetNoMove},
	{"GetNoMove", GLua_Player_GetNoMove},
	{"ServerTransfer", GLua_Player_ServerTransfer},
	// stuff for credits --eez
	{"GetCreditCount", GLua_Player_GetCreditCount},
	{"SetCreditCount", GLua_Player_SetCreditCount},
	{"ModifyCreditCount", GLua_Player_ModifyCreditCount},
	// add 6/2/13
	{"GetCurrentGunAmmoType", GLua_Player_GetCurrentGunAmmoType},
	//{"GetGunAmmoType", GLua_Player_GetGunAmmoType},	// removed 12/11/2016
	{"PossessingItem", GLua_Player_PossessingItem},
	{"PossessingWeapon", GLua_Player_PossessingWeapon},
	// add 8/18/13
	{"SetAccount", GLua_Player_SetAccount},
	{"GetAccount", GLua_Player_GetAccount},
	// add 8/19/13
	{"ExecuteChatCommand", GLua_Player_ExecuteChatCommand},
	{NULL, NULL},
};

/**************************************************
* player_p
*
* Properties of the player class
**************************************************/

static const struct GLua_Prop player_p [] = {
	{"Health",	GLua_Player_Health_Get,		GLua_Player_Health_Set},
	{"Stamina", GLua_Player_Stamina_Get,	GLua_Player_Stamina_Set},
	{"Shield",	GLua_Player_Shield_Get,		GLua_Player_Shield_Set},
	{"MaxHealth", GLua_Player_MaxHealth_Get, GLua_Player_MaxHealth_Set},
	{"MaxStamina", GLua_Player_MaxStamina_Get, GLua_Player_MaxStamina_Set},
	{"MaxShield", GLua_Player_MaxShield_Get, GLua_Player_MaxShield_Set},
	{"ID",		GLua_Player_GetID,			NULL},
	{"Name",	GLua_Player_GetName,		NULL},
	{"IP",		GLua_Player_GetIP,			NULL},
	{"IPPort",	GLua_Player_GetIPPort,		NULL},
	{"Valid",	GLua_Player_IsValid,		NULL},
	{"Connected", GLua_Player_IsConnected,	NULL},
	{"Hacking",	GLua_Player_IsHacking,		NULL},
	{"Origin",	GLua_Player_GetPos,			GLua_Player_SetPos},
	{"Pos",		GLua_Player_GetPos,			GLua_Player_SetPos},
	{"Angles",	GLua_Player_GetAngles,		GLua_Player_SetAngles},
	{"Entity",	GLua_Player_GetEntity,		NULL},
	{"GodMode", GLua_Player_HasGodMode,		GLua_Player_SetGodMode},
	{"NoClip",	GLua_Player_HasNoClip,		GLua_Player_SetNoClip},
	{"NoKnockback", GLua_Player_HasNoKnockback, GLua_Player_SetNoKnockback },		//futuza: adding NoKnockback for players
	{"Busy",	GLua_Player_HasBusy,		GLua_Player_SetBusy },					//futuza: check if the player is busy with something else (eg: in a pazaak game, hacking, trading, etc)
	{"NoDebuff", GLua_Player_HasNoDebuff,	GLua_Player_SetNoDebuff },
	{"NoTarget",GLua_Player_HasNoTarget,	GLua_Player_SetNoTarget},
	{"Gravity", GLua_Player_GetGravity,		GLua_Player_SetGravity},
	{"Undying", GLua_Player_GetUndying,		GLua_Player_SetUndying},
	{"Invulnerable", GLua_Player_GetInvulnerable,GLua_Player_SetInvulnerable},
	{"Freeze", GLua_Player_GetGravity,		GLua_Player_SetGravity},
	{"InDeathcam", GLua_Player_InDeathcam,	NULL},
	{"NoDismember", GLua_Player_GetNoDismember, GLua_Player_SetNoDismember},
	{"NoDisintegrate", GLua_Player_GetNoDisintegrate, GLua_Player_SetNoDisintegrate},
	{"NoDrops", GLua_Player_GetNoDrops, GLua_Player_SetNoDrops},
	{"AdminRank",	GLua_Player_GetAdminRank,	NULL},
	{"CustomTeam", GLua_Player_GetCustomTeam, GLua_Player_SetCustomTeam},
	{"CanUseCheats", GLua_Player_CanUseCheats_Get, GLua_Player_CanUseCheats_Set},
	{NULL,		NULL,						NULL},
};

void GLua_PushPlayer(lua_State *L, int clientNum) {
	GLua_Data_Player_t *data;
	if (clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		lua_pushnil(L);
		return;
	}
	if (level.clients[clientNum].pers.connected != CON_DISCONNECTED) {
		// Client is here
		data = (GLua_Data_Player_t *)lua_newuserdata(L, sizeof(GLua_Data_Player_t));
		data->clientNum = clientNum;
		data->IDCode = g_entities[clientNum].IDCode;
		luaL_getmetatable(L,"Player");
		lua_setmetatable(L,-2);
	} else {
		// Invalid client, push nil
		lua_pushnil(L);
	}
	return;
}

static int GLua_Players_GetByID(lua_State *L) {
	if (lua_gettop(L) < 1) {
		return 0;
	} else {
		GLua_PushPlayer(L, luaL_checkint(L,1)); // Either puts a player or a nil on the stack
		return 1;
	}
}

int G_ClientNumberFromStrippedName ( const char* name );
int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll );
static int GLua_Players_GetByName(lua_State *L) {
	if (lua_gettop(L) < 1) {
		return 0;
	} else {
		// This will do a lookup for the player ID, if the player is invalid, it pushes nil
		// If multiple players are found, the first match is used
		const char *name;
		int id;
		name = luaL_checkstring(L,1);
		id = G_ClientNumberFromStrippedName(name);
		if (id < 0) {
			id = G_ClientNumberFromStrippedSubstring(name, qfalse);
		}
		GLua_PushPlayer(L, id);
		return 1;
	}
}

// This one is to make it easier to write commands that accept players as an argument
// This combines GetByID and GetByName, depending on the argument provided
int G_ClientNumberFromArg ( const char *name);
static int GLua_Players_GetByArg(lua_State *L) {
	if (lua_gettop(L) < 1) {
		return 0;
	} else {
		const char *name;
		int id;
		name = luaL_checkstring(L,1);
		id = G_ClientNumberFromArg((char *)name);
		GLua_PushPlayer(L, id);
		return 1;
	}
}

static int GLua_Players_GetAll(lua_State *L) {
	int idx;
	int i;
	lua_newtable(L);
	for (i=0, idx=1; i < MAX_CLIENTS; i++) {
		if (level.clients[i].pers.connected != CON_DISCONNECTED) {
			GLua_PushPlayer(L, i);
			lua_rawseti(L,-2,idx++);
		}
	}
	return 1;
}

static const struct luaL_reg player_f [] = {
	{"GetByID", GLua_Players_GetByID},
	{"GetByName", GLua_Players_GetByName},
	{"GetByArg", GLua_Players_GetByArg},
	{"GetAll", GLua_Players_GetAll},
	{NULL, NULL},
};

void GLua_Define_Player(lua_State *L) {

	STACKGUARD_INIT(L)
	// Defines the Player object so it can be used

	luaL_register(L, "players", player_f);
	lua_pop(L,1);

	luaL_newmetatable(L,"Player");
	luaL_register(L, NULL, player_m);
	GLua_RegisterProperties(L, player_p, 0);
	
	lua_pushstring(L,"ObjID");
	lua_pushinteger(L, GO_PLAYER);
	lua_settable(L,-3);

	lua_pushstring(L,"ObjName");
	lua_pushstring(L,"Player");
	lua_settable(L,-3);

	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}
