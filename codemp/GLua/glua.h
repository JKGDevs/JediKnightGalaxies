// Main code for Lua in Jedi Knight Galaxies
// This file is the way jampgame gets in contact with lua, everything goes through here
extern "C" {
#include <Lua/lua.h>
#include <Lua/lauxlib.h>
#include <Lua/lualib.h>
}
#define GLUA_UNUSED(x) (void)x

// Stack balancing asserts
#ifdef _DEBUG
	#define STACKGUARD_INIT(L) int luastackguard = lua_gettop(L);
	#define STACKGUARD_CHECK(L) if (lua_gettop(L) != luastackguard) assert(!"WARNING: Lua stack unbalanced");
#else
	#define STACKGUARD_INIT(L)
	#define STACKGUARD_CHECK(L)
#endif

// Structs

// Property list struct
typedef struct GLua_Prop {
  const char *prop;
  lua_CFunction getfunc;
  lua_CFunction setfunc;
} GLua_Prop;

typedef struct {
	int clientNum;
	int IDCode;
} GLua_Data_Player_t;

typedef struct {
	int entNum;
	int IDCode;
} GLua_Data_Entity_t;

typedef struct {
	KeyPairSet_t keys;
} GLua_Data_EntityFactory_t;

typedef struct {
	vmCvar_t cvar;
	char name[MAX_CVAR_VALUE_STRING];
	int lastUpdate;
	int lastModifiedCount;
} GLua_Data_Cvar_t;

typedef struct {
	float x;
	float y;
	float z;
} GLuaVec_t;

typedef enum {
	GO_NONE,
	GO_VECTOR,
	GO_CVAR,
	GO_ENTITY,
	GO_ENTITYFACTORY,
	GO_PLAYER,
	GO_NPC,
	GO_BITSTREAM,
	GO_DIGESTCONTEXT,
	GO_CIPHERCONTEXT,
	GO_PRNG,
} GLua_Object_Type;

//extern int EntDataRef;

// glua_main.c
int GLua_LoadFile(lua_State *L, const char* file);
void GLua_Init();
int GLua_Push_SetEntDataFunc(lua_State *L);
int GLua_Push_GetEntDataFunc(lua_State *L);
int GLua_Push_GetEntDataTableFunc(lua_State *L);
int GLua_Push_ToString(lua_State *L);
lua_State *GLua_GetState();
int GLua_GetEntityTypeID(const char* classname);
void GLua_SpawnEntity(gentity_t *ent, const char* classname);
void GLua_Push_CallEntityFunc();

int GLua_NPCExists(const char* npcname);
void GLua_SpawnNPC(gentity_t *npc, const char* npcname);
void GLua_Push_CallNPCFunc();
// Cmds
int GLua_Command(int clientNum, const char *cmd);
int GLua_RconCommand(const char *cmd);
// Timers
void GLua_Timer();
void GLua_TimerReset();
// Threads
void GLua_Thread();
void GLua_ThreadReset();
// Hooks
void GLua_Hook_GameInit(int leveltime, int restart);
void GLua_Hook_MapLoadFinished();
void GLua_Hook_GameShutdown();
int GLua_Hook_PlayerSay(gentity_t * ent, gentity_t * target, int mode, const char* text);
const char *GLua_Hook_PlayerConnect(int clientNum, int firsttime, int isbot);
void GLua_Hook_PlayerBegin(int clientNum);
void GLua_Hook_PlayerDeath(int clientNum, gentity_t *inflictor, gentity_t* attacker, int damage, int mod);
void GLua_Hook_PlayerDisconnect(int clientNum);
void GLua_Hook_PlayerValidationFailed(int clientNum);
void GLua_Hook_PlayerSpawned(int clientNum);
void GLua_Hook_PlayerDeathcam(int clientNum, int *deathcamtime, int *forcerespawn);
int GLua_Hook_SelectInitialSpawn(int clientNum, gentity_t **spawnpoint, int team, vec3_t spawnorigin, vec3_t spawnangles);
int GLua_Hook_SelectSpawn(int clientNum, gentity_t **spawnpoint, int team, vec3_t avoidpoint, vec3_t spawnorigin, vec3_t spawnangles);
int GLua_Hook_SelectSpectatorSpawn(int clientNum, gentity_t **spawnpoint, vec3_t spawnorigin, vec3_t spawnangles);
// Items
void GLua_ConsumeItem(gentity_t* consumer, itemInstance_t* item);
void GLua_ShieldOverloadScript(gentity_t* player, shieldData_t* shd);

// ---
void GLua_Run(const char *line);
void GLua_Close();
void GLua_SoftRestart();
int ValidateObject(lua_State *L, int idx, int Obj);
void GLua_Wipe_EntDataSlot(gentity_t *ent);

// GLua Libraries

// glua_engine.cpp
void GLua_LoadBaseLibs(lua_State *L);
void GLua_LoadLibs(lua_State *L);
void GLua_RegisterProperties(lua_State *L, const GLua_Prop *props, int nup);

// glua_entity.cpp
void GLua_Define_Entity(lua_State *L);
void GLua_PushEntity(lua_State *L, gentity_t *ent);
int GLua_IsEntity(lua_State *L, int idx);
gentity_t *GLua_CheckEntity(lua_State *L, int idx);

// glua_player.cpp
void GLua_Define_Player(lua_State *L);
void GLua_PushPlayer(lua_State *L, int clientNum);
GLua_Data_Player_t *GLua_CheckPlayer(lua_State *L, int idx);

// glua_npc.cpp
void GLua_PushNPC(lua_State *L, gentity_t *ent);
void GLua_Define_NPC(lua_State *L);
gentity_t *GLua_CheckNPC(lua_State *L, int idx);

// glua_files.cpp
void GLua_Define_File(lua_State *L);

// glua_bitwise.cpp
void GLua_Define_Bit(lua_State *L);

// glua_encoding.cpp
void GLua_Define_Encoding(lua_State *L);

// glua_cryptography.cpp
void GLua_Define_Cryptography(lua_State *L);

// glua_bitstream.cpp
void GLua_Define_BitStream(lua_State *L);

// glua_prng.cpp
void GLua_Define_PRNG(lua_State *L);

// glua_cvar.cpp
void GLua_Define_Cvar(lua_State *L);

// glua_sys.cpp
void GLua_Define_Sys(lua_State *L);

// glua_json.cpp
void GLua_Define_JSON( lua_State *L );

// glua_vector.c
void GLua_PushVector(lua_State *L, float x, float y, float z);
int GLua_IsVector(lua_State *L, int idx);
GLuaVec_t *GLua_CheckVector(lua_State *L, int idx);
void GLua_Define_Vector(lua_State *L);
