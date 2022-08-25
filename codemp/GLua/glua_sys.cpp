// Defines sys. namespace

#include "game/g_local.h"
#include "qcommon/q_shared.h"

#include "glua.h"

#define ConvertVec(a, b) (b[0] = a->x, b[1] = a->y, b[2] = a->z)

static int GLua_Sys_GetCvarString(lua_State *L) {
	const char *cvarname = luaL_checkstring(L,1);
	char buff[2048];
	trap->Cvar_VariableStringBuffer(cvarname, buff, sizeof(buff));
	lua_pushstring(L,buff);
	return 1;
}

static int GLua_Sys_GetCvarInt(lua_State *L) {
	const char *cvarname = luaL_checkstring(L,1);
	lua_pushinteger(L,trap->Cvar_VariableIntegerValue(cvarname));
	return 1;
}

static int GLua_Sys_GetCvarFloat(lua_State *L) {
	const char *cvarname = luaL_checkstring(L,1);
	char value[20];
	trap->Cvar_VariableStringBuffer( cvarname, value, sizeof( value ) );
	lua_pushnumber(L, atof( value ));
	return 1;
}

static int GLua_Sys_SetCvar(lua_State *L) {
	const char *cvarname = luaL_checkstring(L,1);
	const char *cvarvalue = luaL_checkstring(L,2);
	trap->Cvar_Set(cvarname, cvarvalue);
	return 0;
}

static int GLua_Sys_SetCS(lua_State *L) {
	int csnum = luaL_checkint(L,1);
	const char *csvalue = luaL_checkstring(L,2);
	trap->SetConfigstring(csnum, csvalue);
	return 0;
}

static int GLua_Sys_GetCS(lua_State *L) {
	int csnum = luaL_checkint(L,1);
	char buff[2048];
	trap->GetConfigstring(csnum, buff, sizeof(buff));
	lua_pushstring(L,buff);
	return 1;
}

static int GLua_Sys_Milliseconds(lua_State *L) {
	lua_pushinteger(L, trap->Milliseconds());
	return 1;
}

static int GLua_Sys_Time(lua_State *L) {
	lua_pushinteger(L, level.time);
	return 1;
}

static int GLua_Sys_Command(lua_State *L) {
	trap->SendConsoleCommand(EXEC_APPEND, luaL_checkstring(L,2));
	return 0;
}

static int GLua_Sys_RemapShader(lua_State *L) {
	int supressupdate = lua_toboolean(L,3);
	AddRemap(luaL_checkstring(L,1), luaL_checkstring(L,2), level.time);
	if (!supressupdate) {
		trap->SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	}
	return 0;
}

static int GLua_Sys_EffectIndex(lua_State *L) {
	lua_pushinteger(L, G_EffectIndex(luaL_checkstring(L,1)));
	return 1;
}

static int GLua_Sys_PlayEffect(lua_State *L) {
	vec3_t org, angs;
	GLuaVec_t *org2, *angs2;
	org2 = GLua_CheckVector(L,2);
	ConvertVec(org2, org);
	if (!lua_isnoneornil(L,3)) {
		angs2 = GLua_CheckVector(L,2);
		ConvertVec(angs2, angs);
	} else {
		VectorClear(angs);
	}
	G_PlayEffectID(luaL_checkint(L,1), org, angs);
	return 0;
}

static int GLua_Sys_RadiusDamage(lua_State *L) {
	vec3_t org;
	gentity_t *attacker = 0, *ignore = 0, *missile = 0;
	GLuaVec_t *org2;
	org2 = GLua_CheckVector(L,1);
	ConvertVec(org2, org);
	if (!lua_isnoneornil(L,2)) attacker = GLua_CheckEntity(L,2);
	if (!lua_isnoneornil(L,5)) ignore = GLua_CheckEntity(L,5);
	if (!lua_isnoneornil(L,6)) missile = GLua_CheckEntity(L,6);

	G_RadiusDamage(org, attacker, lua_tonumber(L,3), lua_tonumber(L,4), ignore, missile, lua_tonumber(L,7));
	return 0;
}

void G_SoundAtLoc( vec3_t loc, int channel, int soundIndex );

static int GLua_Sys_PlaySoundIdx(lua_State *L) {
	vec3_t org;
	GLuaVec_t *org2;
	org2 = GLua_CheckVector(L,1);
	ConvertVec(org2, org);

	G_SoundAtLoc(org, luaL_checkint(L,2), luaL_checkint(L,3));
	return 0;
}

static int GLua_Sys_PlaySound(lua_State *L) {
	vec3_t org;
	GLuaVec_t *org2;
	org2 = GLua_CheckVector(L,1);
	ConvertVec(org2, org);

	G_SoundAtLoc(org, luaL_checkint(L,2), G_SoundIndex(luaL_checkstring(L,3)));
	return 0;
}

static int GLua_Sys_SoundIndex(lua_State *L) {
	lua_pushinteger(L, G_SoundIndex(luaL_checkstring(L,1)));
	return 1;
}

static int GLua_Sys_WeaponClipSize(lua_State *L) {
	int wp = luaL_checkint(L,1);
	int var = luaL_optint(L,2,0);
	int fm = luaL_optint(L, 3, 0);
	if (wp < 0 || wp >= MAX_WEAPONS) {
		return 0;
	}

	weaponData_t* wpd = GetWeaponData(wp, var);

	lua_pushinteger(L, wpd->firemodes[fm].clipSize);	// If no clips are used, the size if 0, so that can be used to determine IF they're used at all
	return 1;
}

static int GLua_Sys_AmmoLimit(lua_State *L) {
	int ammo = luaL_checkint(L,1);
	if (ammo < 0 || ammo >= AMMO_MAX) {
		return 0;
	}

	lua_pushinteger(L, GetAmmoMax( ammo ));	// If no clips are used, the size if 0, so that can be used to determine IF they're used at all
	return 1;
}

static int GLua_Sys_MapName(lua_State *L) {
	char cs[1024];
	trap->GetServerinfo( cs, sizeof( cs ) );
	lua_pushstring(L, Info_ValueForKey( cs, "mapname" ));
	return 1;
}

qboolean SpotWouldTelefrag2( gentity_t *mover, vec3_t dest );

static int GLua_Sys_SpotWouldTelefrag(lua_State *L) {
	gentity_t *ent = GLua_CheckEntity(L, 1);
	GLuaVec_t *vec = GLua_CheckVector(L, 2);
	vec3_t vec2;
	ConvertVec(vec, vec2);
	if (SpotWouldTelefrag2(ent, vec2)) {
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

static int GLua_Sys_StripColorcodes(lua_State *L) {
	const char *input = luaL_checkstring(L, 1);

	if( input && *input )
	{
		char *output = nullptr;
		output = (char *)malloc(strlen(input) + 1);
		strcpy( output, input ); 
		Q_StripColor( output );

		lua_pushstring(L, output);
		free(output);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

static int GLua_Sys_GetRandomInt(lua_State* L)
{
	int min = 0, max = 1, result = 0;
	if (luaL_checkinteger(L, 1) && luaL_checkinteger(L, 2))
	{
		min = lua_tointeger(L, 1), max = lua_tointeger(L, 2);
		result = Q_irandSafe(min, max);
		lua_pushinteger(L, result);
		return 1;
	}

	else
		return 0;
}


static const struct luaL_reg sys_f [] = {
	{"GetCvarString", GLua_Sys_GetCvarString},
	{"GetCvarInt", GLua_Sys_GetCvarInt},
	{"GetCvarFloat", GLua_Sys_GetCvarFloat},
	{"SetCvar", GLua_Sys_SetCvar},
	{"GetConfigString", GLua_Sys_GetCS},
	{"SetConfigString", GLua_Sys_SetCS},
	{"Milliseconds", GLua_Sys_Milliseconds},
	{"Time", GLua_Sys_Time},
	{"Command", GLua_Sys_Command},
	{"RemapShader", GLua_Sys_RemapShader},
	{"EffectIndex", GLua_Sys_EffectIndex},
	{"PlayEffect", GLua_Sys_PlayEffect},
	{"RadiusDamage", GLua_Sys_RadiusDamage},
	{"PlaySound", GLua_Sys_PlaySound},
	{"PlaySoundIdx", GLua_Sys_PlaySoundIdx},
	{"SoundIndex", GLua_Sys_SoundIndex},
	{"WeaponClipSize", GLua_Sys_WeaponClipSize},
	{"AmmoLimit", GLua_Sys_AmmoLimit},
	{"MapName", GLua_Sys_MapName},
	{"SpotWouldTelefrag", GLua_Sys_SpotWouldTelefrag},
	{"StripColorcodes", GLua_Sys_StripColorcodes},
	{"GetRandomInt", GLua_Sys_GetRandomInt},
	{NULL, NULL},
};

void GLua_Define_Sys(lua_State *L) {
	STACKGUARD_INIT(L)

	luaL_register(L, "sys", sys_f);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}