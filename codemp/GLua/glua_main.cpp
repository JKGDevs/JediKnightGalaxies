// Main code for Lua in Jedi Knight Galaxies
// This file is the way jampgame gets in contact with lua, everything goes through here

#include "game/g_local.h"
#include "qcommon/q_shared.h"

#include "glua.h"

static lua_State *LuaInstance;

// Framework functions constants
enum {
	GLUA_HOOK,
	GLUA_CMD,
	GLUA_CMDRCON,
	GLUA_CHATCMD,
	GLUA_TIMER,
	GLUA_TIMERRESET,
	GLUA_THREAD,
	GLUA_THREADRESET,
	GLUA_GETENTDATA,
	GLUA_SETENTDATA,
	GLUA_CLEARENTDATA,
	GLUA_GETENTDATATABLE,
	GLUA_TOSTRING,
	GLUA_GETCINCAMPOS,
	GLUA_GETENTTYPEID,
	GLUA_SPAWNENTITY,
	GLUA_CALLENTITYFUNC,
	GLUA_NPCEXISTS,
	GLUA_SPAWNNPC,
	GLUA_CALLNPCFUNC,
	GLUA_CONSUMEITEM,
	//GLUA_SHIELDOVERLOAD,
	GLUA_MAX,
};

int GLua_Framework[GLUA_MAX]; // Contains lua references to framework functions, if any of these are 0 after init, we got a serious problem

#define GLUA_LOAD_CHUNKSIZE 1024

typedef struct {
	fileHandle_t f;
	int dataRemaining;
	char buff[GLUA_LOAD_CHUNKSIZE];
} gfd_t; // Glua File Data

const char *GLua_LoadFile_Reader(lua_State *L, void *ud, size_t *sz) { // Called by the loader, never access it directly!
	gfd_t *gfd = (gfd_t *)ud;
	if (!gfd->dataRemaining) {
		return NULL;
	}
	if (gfd->dataRemaining >= GLUA_LOAD_CHUNKSIZE) {
		trap->FS_Read(gfd->buff, GLUA_LOAD_CHUNKSIZE, gfd->f);
		gfd->dataRemaining -= GLUA_LOAD_CHUNKSIZE;
		*sz = GLUA_LOAD_CHUNKSIZE;
		return gfd->buff;
	} else {
		trap->FS_Read(gfd->buff,gfd->dataRemaining,gfd->f);
		*sz = gfd->dataRemaining;
		gfd->dataRemaining = 0;
		return gfd->buff;
	}
}

int GLua_LoadFile(lua_State *L, const char* file) {	// Loads a file using JA's FS functions, only use THIS to load files into lua!
	int len = 0;
	fileHandle_t f = 0;
	gfd_t gfd;
	int status;
	
	len = trap->FS_Open(file, &f, FS_READ);
	if (!f || len < 0) {
		// File doesn't exist
		trap->Print("GLua_LoadFile: Failed to load %s, file doesn't exist\n",file);
		return 1;
	} else if ( len == 0 ) {
		// File is empty
		trap->Print("GLua_LoadFile: Failed to load %s, file is empty\n",file);
		return 1;
	}
	gfd.f = f;
	gfd.dataRemaining = len;
	
	

	status = (lua_load(L, GLua_LoadFile_Reader, &gfd, va("@%s", file)) || lua_pcall(L,0,0,0));
	if (status) {
		// Error occurred
		trap->Print("GLua_LoadFile: Failed to load %s: %s\n",file, lua_tostring(L,-1));
		lua_pop(L,1);
	}
	
	trap->FS_Close(f);
	return status;
}

int GLua_GetFrameworkRef(lua_State *L, const char *module, const char *name) {
	// Will attempt to locate the function in question and make a ref to it, returns <0 if it failed
	
	STACKGUARD_INIT(L)
	
	int ref;
	lua_getglobal(L,module);
	if (lua_isnil(L,-1)) {
		lua_pop(L,1);
		return -1;
	}
	lua_pushstring(L,name);
	lua_gettable(L,-2);
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)

	return ref;
}

int GLua_Push_SetEntDataFunc(lua_State *L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_SETENTDATA]);
	return 1;
}

int GLua_Push_GetEntDataFunc(lua_State *L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_GETENTDATA]);
	return 1;
}

int GLua_Push_GetEntDataTableFunc(lua_State *L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_GETENTDATATABLE]);
	return 1;
}

int GLua_Push_ToString(lua_State *L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_TOSTRING]);
	return 1;
}

void GLua_GetCinematicCamPosition(int clientNum, float *x, float *y, float *z) {
	GLuaVec_t *vec;
	lua_State *L = LuaInstance;
	if (level.clients[clientNum].InCinematic) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_GETCINCAMPOS]);
		GLua_PushPlayer(L, clientNum);
		if (lua_pcall(L,1,1,0)) {
			// Since this is called every frame for ppl in cinematics, dont show the error, just cancel out
			trap->Print("ERROR: GLua_GetCinematicCamPosition: %s", lua_tostring(L,-1));
			lua_pop(L,1);
			return;
		}
		if (lua_isnil(L,-1)) {
			lua_pop(L,1);
			return;
		}
		vec = GLua_CheckVector(L,-1);
		*x = vec->x;
		*y = vec->y;
		*z = vec->z;
		lua_pop(L,1);
	}
}

void GLua_Wipe_EntDataSlot(gentity_t *ent) {

	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CLEARENTDATA]);
	GLua_PushEntity(L,ent);

	if (lua_pcall(L,1,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while clearing an entity data slot: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)

	return;
}

int ValidateObject(lua_State *L, int idx, int Obj) {
	if (!lua_isuserdata(L,idx)) return 0;
	lua_getmetatable(L,idx);
	if (lua_isnil(L,-1)) {
		lua_pop(L,1);
		return 0;
	}
	lua_getfield(L, -1, "ObjID");
	if (lua_tointeger(L,-1) == Obj) {
		lua_pop(L,2);
		return 1;
	} else {
		lua_pop(L,2);
		return 0;
	}
}

lua_State *GLua_GetState() {
	return LuaInstance;
}	

static int GLua_Panic(lua_State *L) {
	Com_Error(ERR_FATAL, "FATAL ERROR: Lua Panic! - %s\n", lua_tostring(L,-1));
	return 0;
}

void GLua_Init() {
	lua_State *L;
	int i;
	if (LuaInstance) {
		// Apparently its already active?
		lua_close(LuaInstance);
	}
	memset(GLua_Framework, -1 ,sizeof(GLua_Framework)); // Fill it with NULL refs

	trap->Print("--------- Galaxies Lua initialization ---------\n");
	//LuaZ_Initialize();
	//L = LuaInstance = lua_newstate(GLua_Allocator, 0);
	trap->Print(" -> Initializing Lua...\n");
	L = LuaInstance = lua_open();
	if (!L) {
		Com_Error(ERR_FATAL, "FATAL ERROR: Could not properly initialize Lua!\n");
		//trap->Print("FATAL ERROR: Could not properly initialize Lua!\n");
		//while(1){};
	}
	lua_atpanic(L, GLua_Panic);
	trap->Print(" -> Loading base libraries...\n");
	GLua_LoadBaseLibs(L);
	
	trap->Print(" -> Loading GLua libraries...\n");
	// Initialize namespaces and class metatables
	GLua_LoadLibs(L);

	// Initialize framework
	trap->Print(" -> Initializing GLua Framework...\n");
	if (GLua_LoadFile(L, "glua/framework/init.lua")) {
		Com_Error(ERR_FATAL, "FATAL ERROR: Could not properly initialize the GLua framework!\n");
		//trap->Print("FATAL ERROR: Could not properly initialize the GLua framework!\n");
		
	}

	GLua_Framework[GLUA_HOOK] = GLua_GetFrameworkRef(L,"hook","Call");
	GLua_Framework[GLUA_CMD] = GLua_GetFrameworkRef(L,"cmds","Exec");
	GLua_Framework[GLUA_CMDRCON] = GLua_GetFrameworkRef(L,"cmds","ExecRcon");
	GLua_Framework[GLUA_CHATCMD] = GLua_GetFrameworkRef(L,"chatcmds","Exec");
	GLua_Framework[GLUA_TIMER] = GLua_GetFrameworkRef(L,"timer","Check");
	GLua_Framework[GLUA_TIMERRESET] = GLua_GetFrameworkRef(L,"timer","Reset");
	GLua_Framework[GLUA_THREAD] = GLua_GetFrameworkRef(L,"thread","RunThreads");
	GLua_Framework[GLUA_THREADRESET] = GLua_GetFrameworkRef(L,"thread","Reset");
	
	GLua_Framework[GLUA_GETENTDATA] = GLua_GetFrameworkRef(L,"entstorage","GetData");
	GLua_Framework[GLUA_SETENTDATA] = GLua_GetFrameworkRef(L,"entstorage","SetData");
	GLua_Framework[GLUA_CLEARENTDATA] = GLua_GetFrameworkRef(L,"entstorage","ClearData");
	GLua_Framework[GLUA_GETENTDATATABLE] = GLua_GetFrameworkRef(L,"entstorage","GetTable");
	lua_getglobal(L, "tostring");
	GLua_Framework[GLUA_TOSTRING] = luaL_ref(L, LUA_REGISTRYINDEX);
	GLua_Framework[GLUA_GETCINCAMPOS] = GLua_GetFrameworkRef(L, "cinematics" , "GetCameraPos");
	
	GLua_Framework[GLUA_GETENTTYPEID] = GLua_GetFrameworkRef(L, "entmanager" , "GetEntTypeID");
	GLua_Framework[GLUA_SPAWNENTITY] = GLua_GetFrameworkRef(L, "entmanager" , "SpawnEntity");
	GLua_Framework[GLUA_CALLENTITYFUNC] = GLua_GetFrameworkRef(L, "entmanager" , "CallEntityFunc");

	
	GLua_Framework[GLUA_NPCEXISTS] = GLua_GetFrameworkRef(L, "npcmanager" , "NPCExists");
	GLua_Framework[GLUA_SPAWNNPC] = GLua_GetFrameworkRef(L, "npcmanager" , "SpawnNPC");
	GLua_Framework[GLUA_CALLNPCFUNC] = GLua_GetFrameworkRef(L, "npcmanager" , "CallNPCFunc");

	GLua_Framework[GLUA_CONSUMEITEM] = GLua_GetFrameworkRef(L, "items", "ConsumeItem");
	//GLua_Framework[GLUA_SHIELDOVERLOAD] = GLua_GetFrameworkRef(L, "items", "ShieldOverloadScript");

	// Validate framework state
	for (i=0; i<GLUA_MAX; i++) {
		if (GLua_Framework[i] < 0) {
			Com_Error(ERR_FATAL, "FATAL ERROR: Could not properly initialize the GLua framework!\n");
		}
	}

	trap->Print(" -> Executing init.lua...\n");
	// Run main script
	if (GLua_LoadFile(L, "glua/init.lua")) {
		// This is really bad: error during the init script
		// For diagnostic reasons we wont go fatal here, but still, this needs fixing asap
		trap->Print("CRITICAL ERROR: glua/init.lua failed to run correctly!\n");
	}
	trap->Print(" -> Init successful\n");
	trap->Print("-----------------------------------------------\n");
}

int GLua_GetEntityTypeID(const char* classname) {
	
	STACKGUARD_INIT(LuaInstance)
	
	int ret;
	lua_State *L = LuaInstance;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_GETENTTYPEID]);
	lua_pushstring(L, classname);
	if (lua_pcall(L,1,1,0)) {
		// Error
		trap->Print("GLua: Error occurred in GLua_GetEntityTypeID: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		return 0;
	}
	ret = lua_tointeger(L,-1);
	lua_pop(L,1);

	STACKGUARD_CHECK(LuaInstance)

	return ret;
}

void GLua_SpawnEntity(gentity_t *ent, const char* classname) {
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_SPAWNENTITY]);
	GLua_PushEntity(L, ent);
	lua_pushstring(L, classname);
	if (lua_pcall(L,2,0,0)) {
		// Error
		trap->Print("GLua: Error occurred in GLua_SpawnEntity: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Push_CallEntityFunc() {
	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CALLENTITYFUNC]);
	return;
}


int GLua_NPCExists(const char* npcname) {

	STACKGUARD_INIT(LuaInstance)
	int ret;
	lua_State *L = LuaInstance;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_NPCEXISTS]);
	lua_pushstring(L, npcname);
	if (lua_pcall(L,1,1,0)) {
		// Error
		trap->Print("GLua: Error occurred in GLua_NPCExists: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		return 0;
	}
	ret = lua_toboolean(L,-1);
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return ret;
}

void GLua_SpawnNPC(gentity_t *npc, const char* npcname) {
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_SPAWNNPC]);
	GLua_PushNPC(L, npc);
	lua_pushstring(L, npcname);
	if (lua_pcall(L,2,0,0)) {
		// Error
		trap->Print("GLua: Error occurred in GLua_SpawnNPC: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_ConsumeItem(gentity_t* consumer, itemInstance_t* item) {
	STACKGUARD_INIT(LuaInstance);
	lua_State* L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CONSUMEITEM]);
	GLua_PushPlayer(L, consumer->s.number);
	lua_pushstring(L, item->id->consumableData.consumeScript);
	lua_pushinteger(L, item->quantity);
	if (lua_pcall(L, 3, 0, 0)) {
		trap->Print("GLua: Error occurred in GLua_ConsumeItem: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	STACKGUARD_CHECK(LuaInstance);
	return;
}

void GLua_ShieldOverloadScript(gentity_t* player, shieldData_t* shd)
{
	STACKGUARD_INIT(LuaInstance);
	lua_State* L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CONSUMEITEM]);
	GLua_PushPlayer(L, player->s.number);
	lua_pushstring(L, shd->overloadScript);
	//lua_pushinteger(L, player->quantity);
	if (lua_pcall(L, 3, 0, 0)) {
		trap->Print("GLua: Error occurred in GLua_ShieldOverloadScript: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	STACKGUARD_CHECK(LuaInstance);
	return;
}

void GLua_Push_CallNPCFunc() {
	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CALLNPCFUNC]);
	return;
}

static void GLua_ProcessNewLines(char *line) {
	// This function converts all "\n"'s in the text into an actual \n
	// This will allow more than 1 line to be typed in the console when using lua_run
	char *w = line;	// Write pointer
	char *r = line; // Read pointer
	while (*r) {
		if (*r == '\\') {
			if (*(r+1) == 'n') {
				// Found one!
				*w = 10;
				w++;
				r+=2;
				continue;
			}
		}
		*w = *r;
		w++;
		r++;
	}
	*w = *r;	// Write the NULL
}

void GLua_Run(const char *line) {
	lua_State *L = LuaInstance;
	int status;
	if (!L) {
		trap->Print("Error: GLua is not active, please use lua_restart\n");
		return;
	}
	trap->Print("Executing %s...\n", line);
	GLua_ProcessNewLines((char *)line);	// Do it after the printf, otherwise we'd get newlines in there too.. which is a lil iffy
	status = (luaL_loadbuffer(L, line, strlen(line), "=lua_run") || lua_pcall(L,0,0,0));
	if (status) {
		trap->Print("Error executing Lua: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
}

void GLua_Close() {
	if (LuaInstance) {
		GLua_Hook_GameShutdown();
		lua_close(LuaInstance);
		LuaInstance=0;
	}
}

void GLua_SoftRestart() {

	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	// Run main script
	if (!L) {
		trap->Print("Error: GLua is not active, please use lua_restart\n");
		return;
	}
	lua_pushboolean(L, 1);
	lua_setglobal(L, "RELOADING");
	// Kill all simple timers and automatic threads
	GLua_TimerReset();
	GLua_ThreadReset();

	if (GLua_LoadFile(L, "glua/init.lua")) {
		// This is really bad, error during init script
		// For diagnostic reasons we wont go fatal here, but still, this needs fixing asap
		trap->Print("CRITICAL ERROR: glua/init.lua failed to run! Fix this problem immediately!\n");
	}
	lua_pushnil(L);
	lua_setglobal(L, "RELOADING");
	// Do a full garbage collection cycle, since we'll probably have tons of it
	lua_gc(L, /*LUA_GCCOLLECT*/ 2, 0);

	STACKGUARD_CHECK(LuaInstance)
}

void GLua_Hook_GameInit(int leveltime, int restart) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"Init");
	lua_pushboolean(L,0); // Ignore return values
	if (lua_pcall(L,2,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'Init': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)

	return;
}

void GLua_Hook_MapLoadFinished() {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"MapLoaded");
	lua_pushboolean(L,0); // Ignore return values
	if (lua_pcall(L,2,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'MapLoaded': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)

	return;
}

void GLua_Hook_GameShutdown() {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"Shutdown");
	lua_pushboolean(L,0); // Ignore return values
	if (lua_pcall(L,2,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'Shutdown': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Timer() {

	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_TIMER]);
	if (lua_pcall(L,0,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while processing timers: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_TimerReset() {
	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_TIMERRESET]);
	if (lua_pcall(L,0,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while resetting timers: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}

	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Thread() {

	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_THREAD]);
	if (lua_pcall(L,0,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while processing threads: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_ThreadReset() {
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return;
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_THREADRESET]);
	if (lua_pcall(L,0,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while resetting threads: %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

int GLua_Hook_PlayerSay(gentity_t * ent, gentity_t * target, int mode, const char* text) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushboolean(L,1); // Allow return values
	lua_pushstring(L,"PlayerSay");
	GLua_PushPlayer(L, ent->s.number);
	if (target) 
		GLua_PushPlayer(L, target->s.number);
	else
		lua_pushnil(L);
	lua_pushinteger(L, mode);
	lua_pushstring(L,text);
	if (lua_pcall(L,6,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerSay': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	if (!lua_isnil(L,-1)) {
		if (lua_toboolean(L,-1)) {
			lua_pop(L,1);
			STACKGUARD_CHECK(LuaInstance)
			return 1; // True was returned from the hook, cancel the chat message
		}
	}
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return 0;
}

const char *GLua_Hook_PlayerConnect(int clientNum, int firsttime, int isbot) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	const char *luaresp;
	static char resp[2048];
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerConnect");
	lua_pushboolean(L,1); // Allow return values
	GLua_PushPlayer(L, clientNum);
	lua_pushboolean(L, firsttime);
	lua_pushboolean(L, isbot);
	if (lua_pcall(L,5,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerConnect': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	if (!lua_isnil(L,-1)) {
		luaresp = lua_tostring(L,-1);
		if (luaresp) {
			Q_strncpyz(resp, luaresp ,sizeof(resp));
			lua_pop(L,1);
			STACKGUARD_CHECK(LuaInstance)
			return &resp[0]; // Return reason for rejection
		}
	}
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return NULL;
}

void GLua_Hook_PlayerBegin(int clientNum) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerBegin");
	lua_pushboolean(L,0); // Ignore return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerBegin': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Hook_PlayerSpawned(int clientNum) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerSpawned");
	lua_pushboolean(L,0); // Ignore return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerSpawned': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Hook_PlayerDeathcam(int clientNum, int *deathcamtime, int *forcerespawn) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerDeathcam");
	lua_pushboolean(L,1); // Allow return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerDeathcam': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return;
	}
	if (lua_isnumber(L,-1)) {
		int time = lua_tointeger(L,-1);
		if (time < 0) {
			*forcerespawn = 1;
			*deathcamtime = level.time + (time * -1);
		} else {
			*forcerespawn = 0;
			*deathcamtime = level.time + time;
		}
	}
	lua_pop(L,1);

	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Hook_PlayerDeath(int clientNum, gentity_t *inflictor, gentity_t* attacker, int damage, int mod) {
	// Hook.Call(Hookname, ...)

	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerDeath");
	lua_pushboolean(L,0); // Ignore return values
	GLua_PushPlayer(L, clientNum);
	GLua_PushEntity(L, inflictor);
	GLua_PushEntity(L, attacker);
	lua_pushinteger(L, damage);
	lua_pushinteger(L, mod);
	if (lua_pcall(L,7,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerDeath': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}


int GLua_Hook_SelectInitialSpawn(int clientNum, gentity_t **spawnpoint, int team, vec3_t spawnorigin, vec3_t spawnangles) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	GLuaVec_t	*vec;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"SelectInitialSpawn");
	lua_pushboolean(L,1); // Allow return values
	GLua_PushPlayer(L, clientNum);
	lua_pushinteger(L, team);
	if (lua_pcall(L,4,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'SelectInitialSpawn': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	// Alright, first determine the type of the return value
	if (GLua_IsEntity(L, -1)) {
		// We got an entity, so pass it as the spawnpoint and copy the origin and angles
		*spawnpoint = GLua_CheckEntity(L, -1);
		VectorCopy((*spawnpoint)->s.origin, spawnorigin);
		VectorCopy((*spawnpoint)->s.angles, spawnangles);
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	} else if (lua_istable(L,-1)) {
		// Got a table, so check if the first entry is a vector
		lua_rawgeti(L,-1, 1);
		if (GLua_IsVector(L, -1)) {
			vec = GLua_CheckVector(L,-1);
			*spawnpoint = 0;
			spawnorigin[0] = vec->x;
			spawnorigin[1] = vec->y;
			spawnorigin[2] = vec->z;
			lua_pop(L,1);
			// Check if it has angles too
			lua_rawgeti(L,-1, 2);
			if (GLua_IsVector(L,-1)) {
				vec = GLua_CheckVector(L,-1);
				*spawnpoint = 0;
				spawnangles[0] = vec->x;
				spawnangles[1] = vec->y;
				spawnangles[2] = vec->z;
				lua_pop(L,1);
			} else {
				// Nope, use default
				VectorSet(spawnangles, 0, 0, 0);
			}
			lua_pop(L,1);
			STACKGUARD_CHECK(LuaInstance)
			return 1;
		} else {
			lua_pop(L,2);
			STACKGUARD_CHECK(LuaInstance)
			return 0;
		}
	} else {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}	
}

int GLua_Hook_SelectSpawn(int clientNum, gentity_t **spawnpoint, int team, vec3_t avoidpoint, vec3_t spawnorigin, vec3_t spawnangles) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	GLuaVec_t	*vec;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"SelectSpawn");
	lua_pushboolean(L,1); // Allow return values
	GLua_PushPlayer(L, clientNum);
	lua_pushinteger(L, team);
	GLua_PushVector(L, avoidpoint[0], avoidpoint[1], avoidpoint[2]);
	if (lua_pcall(L,5,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'SelectSpawn': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	// Alright, first determine the type of the return value
	if (GLua_IsEntity(L, -1)) {
		// We got an entity, so pass it as the spawnpoint and copy the origin and angles
		*spawnpoint = GLua_CheckEntity(L, -1);
		VectorCopy((*spawnpoint)->s.origin, spawnorigin);
		VectorCopy((*spawnpoint)->s.angles, spawnangles);
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	} else if (lua_istable(L,-1)) {
		// Got a table, so check if the first entry is a vector
		lua_rawgeti(L,-1, 1);
		if (GLua_IsVector(L, -1)) {
			vec = GLua_CheckVector(L,-1);
			*spawnpoint = 0;
			spawnorigin[0] = vec->x;
			spawnorigin[1] = vec->y;
			spawnorigin[2] = vec->z;
			lua_pop(L,1);
			// Check if it has angles too
			lua_rawgeti(L,-1, 2);
			if (GLua_IsVector(L,-1)) {
				vec = GLua_CheckVector(L,-1);
				*spawnpoint = 0;
				spawnangles[0] = vec->x;
				spawnangles[1] = vec->y;
				spawnangles[2] = vec->z;
				lua_pop(L,1);
			} else {
				// Nope, use default
				VectorSet(spawnangles, 0, 0, 0);
			}
			lua_pop(L,1);
			STACKGUARD_CHECK(LuaInstance)
			return 1;
		} else {
			lua_pop(L,2);
			STACKGUARD_CHECK(LuaInstance)
			return 0;
		}
	} else {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}	
}

int GLua_Hook_SelectSpectatorSpawn(int clientNum, gentity_t **spawnpoint, vec3_t spawnorigin, vec3_t spawnangles) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	GLuaVec_t	*vec;
	if (!L) return 0;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"SelectSpectatorSpawn");
	lua_pushboolean(L,1); // Allow return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'SelectSpectatorSpawn': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	// Alright, first determine the type of the return value
	if (GLua_IsEntity(L, -1)) {
		// We got an entity, so pass it as the spawnpoint and copy the origin and angles
		*spawnpoint = GLua_CheckEntity(L, -1);
		VectorCopy((*spawnpoint)->s.origin, spawnorigin);
		VectorCopy((*spawnpoint)->s.angles, spawnangles);
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	} else if (lua_istable(L,-1)) {
		// Got a table, so check if the first entry is a vector
		lua_rawgeti(L,-1, 1);
		if (GLua_IsVector(L, -1)) {
			vec = GLua_CheckVector(L,-1);
			*spawnpoint = 0;
			spawnorigin[0] = vec->x;
			spawnorigin[1] = vec->y;
			spawnorigin[2] = vec->z;
			lua_pop(L,1);
			// Check if it has angles too
			lua_rawgeti(L,-1, 2);
			if (GLua_IsVector(L,-1)) {
				vec = GLua_CheckVector(L,-1);
				*spawnpoint = 0;
				spawnangles[0] = vec->x;
				spawnangles[1] = vec->y;
				spawnangles[2] = vec->z;
				lua_pop(L,1);
			} else {
				// Nope, use default
				VectorSet(spawnangles, 0, 0, 0);
			}
			lua_pop(L,1);
			STACKGUARD_CHECK(LuaInstance)
			return 1;
		} else {
			lua_pop(L,2);
			STACKGUARD_CHECK(LuaInstance)
			return 0;
		}
	} else {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}	
}

void GLua_Hook_PlayerDisconnect(int clientNum) {
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerDisconect");
	lua_pushboolean(L,0); // Ignore return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerDisconnect': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

void GLua_Hook_PlayerValidationFailed(int clientNum) {
	// Client got dropped due to validation timeout
	// Hook.Call(Hookname, ...)
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;

	if (!L) return;

	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_HOOK]);
	lua_pushstring(L,"PlayerValidationFailed");
	lua_pushboolean(L,0); // Ignore return values
	GLua_PushPlayer(L, clientNum);
	if (lua_pcall(L,3,0,0)) {
		// Error
		trap->Print("GLua: Error occurred while calling hook 'PlayerValidationFailed': %s\n", lua_tostring(L,-1));
		lua_pop(L,1);
	}
	STACKGUARD_CHECK(LuaInstance)
	return;
}

#include "../game/jkg_chatcmds.h"

int GLua_ChatCommand(int clientNum, const char *cmd) { // Returns 1 if processed, 0 if not
	
	STACKGUARD_INIT(LuaInstance)

	lua_State *L = LuaInstance;
	int i;
	int argc = CCmd_Argc();

	if (!L) return 0;
	// Command (player, argc, argv)
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CHATCMD]);
	// Push command (for handler)
	lua_pushstring(L,cmd);
	// Push player object
	GLua_PushPlayer(L, clientNum);
	// Push argc
	lua_pushnumber(L, argc);
	// Create a table with all args in it
	lua_newtable(L);
	for (i=0; i<argc; i++) {
		lua_pushstring(L, CCmd_Argv(i));
		lua_rawseti(L,-2,i);
	}
	if (lua_pcall(L,4,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while executing chat command '%s': %s\n",cmd, lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	if (lua_toboolean(L,-1)) {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	}
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return 0;
}

int GLua_Command(int clientNum, const char *cmd) { // Returns 1 if processed, 0 if not
	
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	int i;
	int argc = trap->Argc();
	char arg[MAX_STRING_CHARS];

	if (!L) return 0;
	// Command (player, argc, argv)
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CMD]);
	// Push command (for handler)
	lua_pushstring(L,cmd);
	// Push player object
	GLua_PushPlayer(L, clientNum);
	// Push argc
	lua_pushnumber(L, argc);
	// Create a table with all args in it
	lua_newtable(L);
	for (i=0; i<argc; i++) {
		trap->Argv(i,arg,sizeof(arg));
		lua_pushstring(L, arg);
		lua_rawseti(L,-2,i);
	}
	if (lua_pcall(L,4,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while executing command '%s': %s\n",cmd, lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	if (lua_toboolean(L,-1)) {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	}
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return 0;
}

int GLua_RconCommand(const char *cmd) { // Returns 1 if processed, 0 if not
	STACKGUARD_INIT(LuaInstance)
	lua_State *L = LuaInstance;
	int i;
	int argc = trap->Argc();
	char arg[MAX_STRING_CHARS];

	if (!L) return 0;
	// Command (player, argc, argv)
	lua_rawgeti(L, LUA_REGISTRYINDEX, GLua_Framework[GLUA_CMDRCON]);
	// Push command (for handler)
	lua_pushstring(L,cmd);
	// Push argc
	lua_pushnumber(L, argc);
	// Create a table with all args in it
	lua_newtable(L);
	for (i=0; i<argc; i++) {
		trap->Argv(i,arg,sizeof(arg));
		lua_pushstring(L, arg);
		lua_rawseti(L,-2,i);
	}
	if (lua_pcall(L,3,1,0)) {
		// Error
		trap->Print("GLua: Error occurred while executing command '%s': %s\n",cmd, lua_tostring(L,-1));
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 0;
	}
	if (lua_toboolean(L,-1)) {
		lua_pop(L,1);
		STACKGUARD_CHECK(LuaInstance)
		return 1;
	}
	lua_pop(L,1);
	STACKGUARD_CHECK(LuaInstance)
	return 0;
}