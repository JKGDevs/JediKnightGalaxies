// Defines json. namespace, which parses JSON files. This is pure win, obviously.

#include "game/g_local.h"
#include "qcommon/q_shared.h"
#include <json/cJSON.h>
#include <stdlib.h>
#include <vector>
#include <string>

#include "glua.h"

//
// Variable type definitions/declarations
//

typedef struct
{
	std::vector<cJSON *> jsonBinding;
	cJSONStream *streamHandle;
	std::string fileBinding;
} gluaJSONBinding_t;

static gluaJSONBinding_t fJSON;

//
// Utility functions
//

// Add a cJSON handle
static void FJSON_PassLastElement( lua_State *L, cJSON *element )
{
	// FIXME: JSON object handles are handled as numbers, not integers. FIX.
	lua_pushnumber( L, fJSON.jsonBinding.size() );
	fJSON.jsonBinding.push_back( element );
}

// Make sure that a cJSON handle is valid
static bool FJSON_ValidateNode( lua_State *L, int argNumber )
{
	std::size_t index = luaL_checkint( L, argNumber );
	if( index < 0 || index > fJSON.jsonBinding.size() )
	{
		return false;
	}
	return true;
}

static cJSON *FJSON_RetrieveNode( lua_State *L, int argNumber )
{
	int index = luaL_checkint( L, argNumber );
	cJSON *node = fJSON.jsonBinding.at( index );
	return node;
}

//
// Namespace function definitions
//


/*
json.RegisterFile(fileName:string):number

Returns the root JSON node of the file
*/

static int GLua_JSON_Register( lua_State *L )
{
	// Check and make sure the file exists, for starters
	fileHandle_t f;
	const char *fName = luaL_checkstring(L,1);
	int len = trap->FS_Open( fName, &f, FS_READ );

	if( len < 0 || !f )
	{
		lua_pushnil(L);
		return 1;
	}

	// Read it now.
	char *buff = (char *)malloc(len + 1);
	
	trap->FS_Read( buff, len, f );
	buff[len] = '\0';
	trap->FS_Close( f );

	fJSON.fileBinding = buff;

	// fileBinding takes a copy of the buffer. Let's just free the original right now.
	free( buff );

	// Okay, now handle the deal with the first node.
	char error[MAX_STRING_CHARS];
	cJSON *json = cJSON_ParsePooled( fJSON.fileBinding.c_str(), error, sizeof( error ) );

	if( json == NULL )
	{
		trap->Print(va( S_COLOR_RED "Failed to get JSON node for Lua (file: %s): %s\n", fName, error));
		lua_pushnil(L);
		return 1;
	}

	// Pass the result to Lua
	FJSON_PassLastElement( L, json );
	
	return 1;
}

/*
json.GetObjectItem(node:number, desiredNodeName:string):number

Gets the node handle, given the base node and the desired node name.
*/

static int GLua_JSON_GetObjectItem( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Alright, all good. Now let's get the desired node name, make sure it's valid and whatnot.
	const char *nodename = luaL_checkstring(L, 2);
	if( !nodename )
	{
		lua_pushnil(L);
		return 1;
	}

	// Ok, do the deal. Pass the result to Lua once we're done.
	int index = luaL_checkint( L, 1 );
	cJSON *root = fJSON.jsonBinding.at(index);
	cJSON *node = cJSON_GetObjectItem(root, nodename);

	FJSON_PassLastElement( L, node );

	return 1;
}

/*
json.Clear(nil):nil

Once we are done with a file, we should call this to perform garbage collection for this file.
*/

static int GLua_JSON_Clear( lua_State *L )
{
	// Use this whenever we're done doing JSON crap

	// Kill files
	fJSON.fileBinding.clear();

	// Kill JSON nodes
	std::vector<cJSON *>::iterator jit;

	for( jit = fJSON.jsonBinding.begin(); jit != fJSON.jsonBinding.end(); jit++ )
	{
		cJSON_Delete(*jit);
	}

	fJSON.jsonBinding.clear();

	return 0;
}

/*
json.GetFirstObject(node:number):number

Returns handle to first JSON object within a node.
*/

static int GLua_JSON_GetFirstObject( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Cool, okay, let's actually perform the function call.
	int index = luaL_checkint( L, 1 );
	cJSON *node = cJSON_GetFirstItem( fJSON.jsonBinding.at( index ) );
	if( !node )
	{
		lua_pushnil(L);
		return 1;
	}

	FJSON_PassLastElement( L, node );

	return 1;
}

/*
json.GetNextObject(node:number):number

Returns handle to next JSON object that is currently being referenced within a node.
*/

static int GLua_JSON_GetNextObject( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Get the next object now?
	int index = luaL_checkint( L, 1 );
	cJSON *node = cJSON_GetNextItem( fJSON.jsonBinding.at( index ) );
	if( !node )
	{
		lua_pushnil( L );
		return 1;
	}
	FJSON_PassLastElement( L, node );
	return 1;
}

/*
json.IsNumber(node:number):boolean

Is the node value a number?
*/

static int GLua_JSON_IsNumber( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushboolean( L, cJSON_IsNumber( node ) );

	return 1;
}

/*
json.IsObject(node:number):boolean

Is the node value an object?
*/

static int GLua_JSON_IsObject( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushboolean( L, cJSON_IsObject( node ) );

	return 1;
}

/*
json.IsBoolean(node:number):boolean

Is the node value a boolean?
*/

static int GLua_JSON_IsBoolean( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushboolean( L, cJSON_IsBoolean( node ) );

	return 1;
}

/*
json.ToString(node:number):string

Converts the node value into a string.
*/

static int GLua_JSON_ToString( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Convert it to string (^.^)
	int index = luaL_checkint( L, 1 );
	const char *text = cJSON_ToString( fJSON.jsonBinding.at( index ) );
	if( !text )
	{
		lua_pushnil( L );
		return 1;
	}

	lua_pushstring( L, text );
	return 1;
}

/*
json.ToNumber(node:number):number

Converts the node value into a number.
*/

static int GLua_JSON_ToNumber( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Convert to number
	int index = luaL_checkint( L, 1 );
	double number = cJSON_ToNumber( fJSON.jsonBinding.at( index ) );

	lua_pushnumber( L, number );
	return 1;
}

/*
json.ToInteger(node:number):integer

Converts the node value into a number. Typecasts to int-style typing.
*/

static int GLua_JSON_ToInteger( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Convert to integer
	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushinteger( L, cJSON_ToInteger( node ) );

	return 1;
}

/*
json.ToBoolean(node:number):boolean

Converts the node value into a boolean.
*/

static int GLua_JSON_ToBoolean( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	// Convert to boolean
	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushboolean( L, cJSON_ToBoolean( node ) );

	return 1;
}

/*
json.ToStringOpt(node:number, optvalue:string):string

Converts the node value into a string. If no value for the string is found, use optvalue instead.
*/

static int GLua_JSON_ToStringOpt( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushstring( L, cJSON_ToStringOpt( node, luaL_checkstring( L, 2 ) ) );

	return 1;
}

/*
json.ToNumberOpt(node:number, optvalue:number):number

Converts the node value into a number. If no value for the number is found, use optvalue instead.
*/

static int GLua_JSON_ToNumberOpt( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushnumber( L, cJSON_ToNumberOpt( node, luaL_checknumber( L, 2 ) ) );

	return 1;
}

/*
json.ToIntegerOpt(node:number, optvalue:number):number

Converts the node value into a number. If no value for the number is found, use optvalue instead. Use integer-style typecasting.
*/

static int GLua_JSON_ToIntegerOpt( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushinteger( L, cJSON_ToIntegerOpt( node, luaL_checkinteger( L, 2 ) ) );

	return 1;
}

/*
json.ToBooleanOpt(node:number, optvalue:boolean):boolean

Converts the node value into a boolean. If no value for the bool is found, use optvalue instead.
*/

static int GLua_JSON_ToBooleanOpt( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushboolean( L, cJSON_ToBooleanOpt( node, lua_toboolean( L, 2 ) ) );

	return 1;
}

/*
json.GetArraySize(node:number):integer

Returns number of elements in an array
*/
static int GLua_JSON_GetArraySize( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	lua_pushinteger( L, cJSON_GetArraySize( node ) );

	return 1;
}

/*
json.GetArrayItem(node:number, elementnum:number):number

Returns handle to JSON object in array
*/
static int GLua_JSON_GetArrayItem( lua_State *L )
{
	// Check and make sure we're using a valid index
	if( !FJSON_ValidateNode( L, 1 ) )
	{
		lua_pushnil(L);
		return 1;
	}

	cJSON *node = FJSON_RetrieveNode( L, 1 );
	cJSON *arryItm = cJSON_GetArrayItem( node, luaL_checkinteger( L, 2 ) );

	FJSON_PassLastElement( L, arryItm );

	return 1;
}

/*
json.RegisterStream(maxdepth:integer, formatted:boolean):nil

Starts a stream.
*/

static int GLua_JSON_RegisterStream( lua_State *L )
{
	fJSON.streamHandle = cJSON_Stream_New( luaL_checkinteger( L, 1 ), lua_toboolean( L, 2 ), 0, 0 );
	return 0;
}

/*
json.FinishStream(filename:string):nil

Finishes the current stream.
*/

static int GLua_JSON_FinishStream( lua_State *L )
{
	if( fJSON.streamHandle == NULL ) // ya...no..just no...
		return 0;

	const char *buffer = cJSON_Stream_Finalize( fJSON.streamHandle );
	const char *file = luaL_checkstring(L, 1);
	if( !file )
		return 0;

	fileHandle_t f;

	trap->FS_Open( file, &f, FS_WRITE );
	trap->FS_Write( buffer, strlen(buffer), f );
	trap->FS_Close( f );
	return 0;
}

/*
json.BeginObject(object:string):nil

Begins an object. If object is "0", then we will pass NULL.
*/

static int GLua_JSON_BeginObject( lua_State *L )
{
	const char *arg = luaL_checkstring(L, 1);
	if( !arg ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(arg, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_BeginObject( fJSON.streamHandle, firstArgNull ? NULL : arg );
	return 0;
}

/*
json.EndObject(nil):nil

Ends the highest-scoped object.
*/

static int GLua_JSON_EndObject( lua_State *L )
{
	cJSON_Stream_EndBlock( fJSON.streamHandle );
	return 0;
}

/*
json.BeginArray(object:string):nil

Same as json.BeginObject, but treats the value like an array.
*/

static int GLua_JSON_BeginArray( lua_State *L )
{
	const char *arg = luaL_checkstring(L, 1);
	if( !arg ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(arg, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_BeginArray( fJSON.streamHandle, firstArgNull ? NULL : arg );
	return 0;
}

/*
json.WriteString(key:string, value:string):nil

Writes a string object. key can be "0" for NULL (good for arrays)
*/

static int GLua_JSON_WriteString( lua_State *L )
{
	const char *key = luaL_checkstring(L, 1);
	const char *value = luaL_checkstring(L, 2);
	if( !key ) return 0;
	if( !value ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(key, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_WriteString( fJSON.streamHandle, firstArgNull ? NULL : key, value );
	return 0;
}

/*
json.WriteInteger(key:string, value:integer):nil

Writes an integer object. key can be "0" for NULL (good for arrays)
*/

static int GLua_JSON_WriteInteger( lua_State *L )
{
	const char *key = luaL_checkstring(L, 1);
	int value = luaL_checkinteger(L, 2);
	if( !key ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(key, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_WriteInteger( fJSON.streamHandle, firstArgNull ? NULL : key, value );
	return 0;
}

/*
json.WriteNumber(key:string, value:number):nil

Writes a number object. key can be "0" for NULL (good for arrays)
*/

static int GLua_JSON_WriteNumber( lua_State *L )
{
	const char *key = luaL_checkstring(L, 1);
	double value = luaL_checknumber(L, 2);
	if( !key ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(key, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_WriteNumber( fJSON.streamHandle, firstArgNull ? NULL : key, value );
	return 0;
}

/*
json.WriteBoolean(key:string, value:boolean):nil

Writes a boolean object. key can be "0" for NULL (good for arrays)
*/

static int GLua_JSON_WriteBoolean( lua_State *L )
{
	const char *key = luaL_checkstring(L, 1);
	int value = lua_toboolean(L, 2);
	if( !key ) return 0;
	bool firstArgNull = false;

	if( Q_stricmp(key, "0") == 0 )
	{
		firstArgNull = true;
	}

	cJSON_Stream_WriteBoolean( fJSON.streamHandle, firstArgNull ? NULL : key, value );
	return 0;
}

static const struct luaL_reg json_f [] = {
	// Basics
	{ "RegisterFile", GLua_JSON_Register },				// Opens a file. Returns the handle to the first node.
	{ "GetObjectItem", GLua_JSON_GetObjectItem },		// Same as cJSON_GetObjectItem, returns a handle to a node
	{ "Clear", GLua_JSON_Clear },						// Clears all nodes and the current file pointer.

	// Basic object handling
	{ "GetFirstObject", GLua_JSON_GetFirstObject },		// Get first object in node
	{ "GetNextObject", GLua_JSON_GetNextObject },		// Get next object in node
	{ "IsNumber", GLua_JSON_IsNumber },					// cJSON_IsNumber
	{ "IsObject", GLua_JSON_IsObject },					// cJSON_IsObject
	{ "IsBoolean", GLua_JSON_IsBoolean },				// cJSON_IsBoolean

	// Retrieve values
	{ "ToString", GLua_JSON_ToString },					// cJSON_ToString
	{ "ToNumber", GLua_JSON_ToNumber },					// cJSON_ToNumber
	{ "ToInteger", GLua_JSON_ToInteger },				// cJSON_ToInteger
	{ "ToBoolean", GLua_JSON_ToBoolean },				// cJSON_ToBoolean
	{ "ToStringOpt", GLua_JSON_ToStringOpt },			// cJSON_ToStringOpt
	{ "ToNumberOpt", GLua_JSON_ToNumberOpt },			// cJSON_ToNumberOpt
	{ "ToIntegerOpt", GLua_JSON_ToIntegerOpt },			// cJSON_ToIntegerOpt
	{ "ToBooleanOpt", GLua_JSON_ToBooleanOpt },			// cJSON_ToBooleanOpt

	// Array handling
	{ "GetArraySize", GLua_JSON_GetArraySize },			// cJSON_GetArraySize
	{ "GetArrayItem", GLua_JSON_GetArrayItem },			// cJSON_GetArrayItem

	// Writing Capabilities
	{ "RegisterStream", GLua_JSON_RegisterStream },		// Opens a file. Returns the handle to the stream.
	{ "FinishStream", GLua_JSON_FinishStream },			// Finishes a JSON file.

	{ "BeginObject", GLua_JSON_BeginObject },
	{ "EndObject", GLua_JSON_EndObject },
	{ "BeginArray", GLua_JSON_BeginArray },

	{ "WriteString", GLua_JSON_WriteString },
	{ "WriteInteger", GLua_JSON_WriteInteger },
	{ "WriteNumber", GLua_JSON_WriteNumber },
	{ "WriteBoolean", GLua_JSON_WriteBoolean },
	{ NULL, NULL }
};

void GLua_Define_JSON( lua_State *L )
{
	STACKGUARD_INIT(L)

	luaL_register(L, "json", json_f);
	lua_pop(L,1);

	STACKGUARD_CHECK(L)
}