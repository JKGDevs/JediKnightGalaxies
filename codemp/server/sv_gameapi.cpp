/*
===========================================================================
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// sv_gameapi.cpp  -- interface to the game dll
//Anything above this #include will be ignored by the compiler

#include "server.h"
#include "botlib/botlib.h"
#include "qcommon/stringed_ingame.h"
#include "qcommon/RoffSystem.h"
#include "ghoul2/ghoul2_shared.h"
#include "qcommon/cm_public.h"
#include "icarus/GameInterface.h"
#include "qcommon/timing.h"
#include "NPCNav/navigator.h"

botlib_export_t	*botlib_export;

// game interface
static gameExport_t *ge; // game export table
static vm_t *gvm; // game vm, valid for legacy and new api

//
// game vmMain calls
//

void GVM_InitGame( int levelTime, int randomSeed, int restart ) {
	VMSwap v( gvm );

	ge->InitGame( levelTime, randomSeed, restart );
}

void GVM_ShutdownGame( int restart ) {
	VMSwap v( gvm );

	ge->ShutdownGame( restart );
}

char *GVM_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	VMSwap v( gvm );

	return ge->ClientConnect( clientNum, firstTime, isBot );
}

void GVM_ClientBegin( int clientNum ) {
	VMSwap v( gvm );

	ge->ClientBegin( clientNum, qtrue );
}

qboolean GVM_ClientUserinfoChanged( int clientNum ) {
	VMSwap v( gvm );

	return ge->ClientUserinfoChanged( clientNum );
}

void GVM_ClientDisconnect( int clientNum ) {
	VMSwap v( gvm );

	ge->ClientDisconnect( clientNum );
}

void GVM_ClientCommand( int clientNum ) {
	VMSwap v( gvm );

	ge->ClientCommand( clientNum );
}

void GVM_ClientThink( int clientNum, usercmd_t *ucmd ) {
	VMSwap v( gvm );

	ge->ClientThink( clientNum, ucmd );
}

void GVM_RunFrame( int levelTime ) {
	VMSwap v( gvm );

	ge->RunFrame( levelTime );
}

qboolean GVM_ConsoleCommand( void ) {
	VMSwap v( gvm );

	return ge->ConsoleCommand();
}

int GVM_BotAIStartFrame( int time ) {
	VMSwap v( gvm );

	return ge->BotAIStartFrame( time );
}

void GVM_ROFF_NotetrackCallback( int entID, const char *notetrack ) {
	VMSwap v( gvm );

	ge->ROFF_NotetrackCallback( entID, notetrack );
}

void GVM_SpawnRMGEntity( void ) {
	VMSwap v( gvm );

	ge->SpawnRMGEntity();
}

int GVM_ICARUS_PlaySound( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_PlaySound();
}

qboolean GVM_ICARUS_Set( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_Set();
}

void GVM_ICARUS_Lerp2Pos( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Lerp2Pos();
}

void GVM_ICARUS_Lerp2Origin( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Lerp2Origin();
}

void GVM_ICARUS_Lerp2Angles( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Lerp2Angles();
}

int GVM_ICARUS_GetTag( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_GetTag();
}

void GVM_ICARUS_Lerp2Start( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Lerp2Start();
}

void GVM_ICARUS_Lerp2End( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Lerp2End();
}

void GVM_ICARUS_Use( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Use();
}

void GVM_ICARUS_Kill( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Kill();
}

void GVM_ICARUS_Remove( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Remove();
}

void GVM_ICARUS_Play( void ) {
	VMSwap v( gvm );

	ge->ICARUS_Play();
}

int GVM_ICARUS_GetFloat( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_GetFloat();
}

int GVM_ICARUS_GetVector( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_GetVector();
}

int GVM_ICARUS_GetString( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_GetString();
}

void GVM_ICARUS_SoundIndex( void ) {
	VMSwap v( gvm );

	ge->ICARUS_SoundIndex();
}

int GVM_ICARUS_GetSetIDForString( void ) {
	VMSwap v( gvm );

	return ge->ICARUS_GetSetIDForString();
}

qboolean GVM_NAV_ClearPathToPoint( int entID, vec3_t pmins, vec3_t pmaxs, vec3_t point, int clipmask, int okToHitEnt ) {
	VMSwap v( gvm );

	return ge->NAV_ClearPathToPoint( entID, pmins, pmaxs, point, clipmask, okToHitEnt );
}

qboolean GVM_NPC_ClearLOS2( int entID, const vec3_t end ) {
	VMSwap v( gvm );

	return ge->NPC_ClearLOS2( entID, end );
}

int GVM_NAVNEW_ClearPathBetweenPoints( vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int ignore, int clipmask ) {
	VMSwap v( gvm );

	return ge->NAVNEW_ClearPathBetweenPoints( start, end, mins, maxs, ignore, clipmask );
}

qboolean GVM_NAV_CheckNodeFailedForEnt( int entID, int nodeNum ) {
	VMSwap v( gvm );

	return ge->NAV_CheckNodeFailedForEnt( entID, nodeNum );
}

qboolean GVM_NAV_EntIsUnlockedDoor( int entityNum ) {
	VMSwap v( gvm );

	return ge->NAV_EntIsUnlockedDoor( entityNum );
}

qboolean GVM_NAV_EntIsDoor( int entityNum ) {
	VMSwap v( gvm );

	return ge->NAV_EntIsDoor( entityNum );
}

qboolean GVM_NAV_EntIsBreakable( int entityNum ) {
	VMSwap v( gvm );

	return ge->NAV_EntIsBreakable( entityNum );
}

qboolean GVM_NAV_EntIsRemovableUsable( int entNum ) {
	VMSwap v( gvm );

	return ge->NAV_EntIsRemovableUsable( entNum );
}

void GVM_NAV_FindCombatPointWaypoints( void ) {
	VMSwap v( gvm );

	ge->NAV_FindCombatPointWaypoints();
}

int GVM_BG_GetItemIndexByTag( int tag, int type ) {
	VMSwap v( gvm );

	return ge->BG_GetItemIndexByTag( tag, type );
}

extern float g_svCullDist;
int CM_ModelContents( clipHandle_t model, int subBSPIndex );
int CM_LoadSubBSP( const char *name, qboolean clientload );
int CM_FindSubBSP( int modelIndex );
char *CM_SubBSPEntityString( int index );
qboolean Q3_TaskIDPending( sharedEntity_t *ent, taskID_t taskType );
void Q3_TaskIDSet( sharedEntity_t *ent, taskID_t taskType, int taskID );
void Q3_TaskIDComplete( sharedEntity_t *ent, taskID_t taskType );
void Q3_SetVar( int taskID, int entID, const char *type_name, const char *data );
int Q3_VariableDeclared( const char *name );
int Q3_GetFloatVariable( const char *name, float *value );
int Q3_GetStringVariable( const char *name, const char **value );
int Q3_GetVectorVariable( const char *name, vec3_t value );
void SV_BotWaypointReception( int wpnum, wpobject_t **wps );
void SV_BotCalculatePaths( int rmg );

static void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t, playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}

static void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );
}

static void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );
	}
}

static qboolean SV_EntityContact( const vec3_t mins, const vec3_t maxs, const sharedEntity_t *gEnt, int capsule ) {
	const float	*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace ( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return (qboolean)trace.startsolid;
}

static void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name)
	{
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if (name[0] == '*')
	{
		ent->s.modelindex = atoi( name + 1 );

		if (sv.mLocalSubBSPIndex != -1)
		{
			ent->s.modelindex += sv.mLocalSubBSPModelOffset;
		}

		h = CM_InlineModel( ent->s.modelindex );

		CM_ModelBounds(h, mins, maxs);

		VectorCopy (mins, ent->r.mins);
		VectorCopy (maxs, ent->r.maxs);
		ent->r.bmodel = qtrue;
		ent->r.contents = CM_ModelContents( h, -1 );
	}
	else if (name[0] == '#')
	{
		ent->s.modelindex = CM_LoadSubBSP(va("maps/%s.bsp", name + 1), qfalse);
		CM_ModelBounds( ent->s.modelindex, mins, maxs );

		VectorCopy (mins, ent->r.mins);
		VectorCopy (maxs, ent->r.maxs);
		ent->r.bmodel = qtrue;

		//rwwNOTE: We don't ever want to set contents -1, it includes CONTENTS_LIGHTSABER.
		//Lots of stuff will explode if there's a brush with CONTENTS_LIGHTSABER that isn't attached to a client owner.
		//ent->contents = -1;		// we don't know exactly what is in the brushes
		h = CM_InlineModel( ent->s.modelindex );
		ent->r.contents = CM_ModelContents( h, CM_FindSubBSP(ent->s.modelindex) );
	}
	else
	{
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}
}

static qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	int		leafnum, cluster;
//	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum( p1 );
	cluster = CM_LeafCluster( leafnum );
//	area1 = CM_LeafArea( leafnum );
	mask = CM_ClusterPVS( cluster );

	leafnum = CM_PointLeafnum( p2 );
	cluster = CM_LeafCluster( leafnum );
//	area2 = CM_LeafArea( leafnum );

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;

	return qtrue;
}

static void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
		return;
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

static void SV_AdjustAreaPortalState( sharedEntity_t *ent, qboolean open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 )
		return;

	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}

static void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
		return;
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

static const char *SV_SetActiveSubBSP( int index ) {
	if ( index >= 0 ) {
		sv.mLocalSubBSPIndex = CM_FindSubBSP( index );
		sv.mLocalSubBSPModelOffset = index;
		sv.mLocalSubBSPEntityParsePoint = CM_SubBSPEntityString( sv.mLocalSubBSPIndex );
		return sv.mLocalSubBSPEntityParsePoint;
	}

	sv.mLocalSubBSPIndex = -1;
	return NULL;
}

static qboolean SV_GetEntityToken( char *buffer, int bufferSize ) {
	char *s;

	if ( sv.mLocalSubBSPIndex == -1 ) {
		s = COM_Parse( (const char **)&sv.entityParsePoint );
		Q_strncpyz( buffer, s, bufferSize );
		if ( !sv.entityParsePoint && !s[0] )
			return qfalse;
		else
			return qtrue;
	}
	else {
		s = COM_Parse( (const char **)&sv.mLocalSubBSPEntityParsePoint);
		Q_strncpyz( buffer, s, bufferSize );
		if ( !sv.mLocalSubBSPEntityParsePoint && !s[0] )
			return qfalse;
		else
			return qtrue;
	}
}

static void SV_PrecisionTimerStart( void **timer ) {
	timing_c *newTimer = new timing_c; //create the new timer
	*timer = newTimer; //assign the pointer within the pointer to point at the mem addr of our new timer
	newTimer->Start(); //start the timer
}

static int SV_PrecisionTimerEnd( void *timer ) {
	int r;
	timing_c *theTimer = (timing_c *)timer; //this is the pointer we assigned in start, so we can directly cast it back
	r = theTimer->End(); //get the result
	delete theTimer; //delete the timer since we're done with it
	return r; //return the result
}

static void SV_RegisterSharedMemory( char *memory ) {
	sv.mSharedMemory = memory;
}

static void SV_SetServerCull( float cullDistance ) {
	g_svCullDist = cullDistance;
}

qboolean SV_ROFF_Clean( void ) {
	return theROFFSystem.Clean( qfalse );
}

void SV_ROFF_UpdateEntities( void ) {
	theROFFSystem.UpdateEntities( qfalse );
}

int SV_ROFF_Cache( char *file ) {
	return theROFFSystem.Cache( file, qfalse );
}

qboolean SV_ROFF_Play( int entID, int roffID, qboolean doTranslation ) {
	return theROFFSystem.Play( entID, roffID, doTranslation, qfalse );
}

qboolean SV_ROFF_Purge_Ent( int entID ) {
	return theROFFSystem.PurgeEnt( entID, qfalse );
}

static qboolean SV_ICARUS_RegisterScript( const char *name, qboolean bCalledDuringInterrogate ) {
	return (qboolean)ICARUS_RegisterScript( name, bCalledDuringInterrogate );
}

static qboolean SV_ICARUS_ValidEnt( sharedEntity_t *ent ) {
	return (qboolean)ICARUS_ValidEnt( ent );
}

static qboolean ICARUS_IsInitialized( int entID ) {
	if ( !gSequencers[entID] || !gTaskManagers[entID] )
		return qfalse;

	return qtrue;
}

static qboolean ICARUS_MaintainTaskManager( int entID ) {
	if ( gTaskManagers[entID] ) {
		gTaskManagers[entID]->Update();
		return qtrue;
	}
	return qfalse;
}

static qboolean ICARUS_IsRunning( int entID ) {
	if ( !gTaskManagers[entID] || !gTaskManagers[entID]->IsRunning() )
		return qfalse;
	return qtrue;
}

static qboolean ICARUS_TaskIDPending( sharedEntity_t *ent, int taskID ) {
	return Q3_TaskIDPending( ent, (taskID_t)taskID );
}

static void SV_ICARUS_TaskIDSet( sharedEntity_t *ent, int taskType, int taskID ) {
	Q3_TaskIDSet( ent, (taskID_t)taskType, taskID );
}

static void SV_ICARUS_TaskIDComplete( sharedEntity_t *ent, int taskType ) {
	Q3_TaskIDComplete( ent, (taskID_t)taskType );
}

static int SV_ICARUS_GetStringVariable( const char *name, const char *value ) {
	const char *rec = (const char *)value;
	return Q3_GetStringVariable( name, (const char **)&rec );
}
static int SV_ICARUS_GetVectorVariable( const char *name, const vec3_t value ) {
	return Q3_GetVectorVariable( name, (float *)value );
}

static void SV_Nav_Init( void ) {
	navigator.Init();
}

static void SV_Nav_Free( void ) {
	navigator.Free();
}

static qboolean SV_Nav_Load( const char *filename, int checksum ) {
	return (qboolean)navigator.Load( filename, checksum );
}

static qboolean SV_Nav_Save( const char *filename, int checksum ) {
	return (qboolean)navigator.Save( filename, checksum );
}

static int SV_Nav_AddRawPoint( vec3_t point, int flags, int radius ) {
	return navigator.AddRawPoint( point, flags, radius );
}

static void SV_Nav_CalculatePaths( qboolean recalc ) {
	navigator.CalculatePaths( recalc );
}

static void SV_Nav_HardConnect( int first, int second ) {
	navigator.HardConnect( first, second );
}

static void SV_Nav_ShowNodes( void ) {
	navigator.ShowNodes();
}

static void SV_Nav_ShowEdges( void ) {
	navigator.ShowEdges();
}

static void SV_Nav_ShowPath( int start, int end ) {
	navigator.ShowPath( start, end );
}

static int SV_Nav_GetNearestNode( sharedEntity_t *ent, int lastID, int flags, int targetID ) {
	return navigator.GetNearestNode( ent, lastID, flags, targetID );
}

static int SV_Nav_GetBestNode( int startID, int endID, int rejectID ) {
	return navigator.GetBestNode( startID, endID, rejectID );
}

static int SV_Nav_GetNodePosition( int nodeID, vec3_t out ) {
	return navigator.GetNodePosition( nodeID, out );
}

static int SV_Nav_GetNodeNumEdges( int nodeID ) {
	return navigator.GetNodeNumEdges( nodeID );
}

static int SV_Nav_GetNodeEdge( int nodeID, int edge ) {
	return navigator.GetNodeEdge( nodeID, edge );
}

static int SV_Nav_GetNumNodes( void ) {
	return navigator.GetNumNodes();
}

static qboolean SV_Nav_Connected( int startID, int endID ) {
	return (qboolean)navigator.Connected( startID, endID );
}

static int SV_Nav_GetPathCost( int startID, int endID ) {
	return navigator.GetPathCost( startID, endID );
}

static int SV_Nav_GetEdgeCost( int startID, int endID ) {
	return navigator.GetEdgeCost( startID, endID );
}

static int SV_Nav_GetProjectedNode( vec3_t origin, int nodeID ) {
	return navigator.GetProjectedNode( origin, nodeID );
}

static void SV_Nav_CheckFailedNodes( sharedEntity_t *ent ) {
	navigator.CheckFailedNodes( ent );
}

static void SV_Nav_AddFailedNode( sharedEntity_t *ent, int nodeID ) {
	navigator.AddFailedNode( ent, nodeID );
}

static qboolean SV_Nav_NodeFailed( sharedEntity_t *ent, int nodeID ) {
	return navigator.NodeFailed( ent, nodeID );
}

static qboolean SV_Nav_NodesAreNeighbors( int startID, int endID ) {
	return navigator.NodesAreNeighbors( startID, endID );
}

static void SV_Nav_ClearFailedEdge( failedEdge_t *failedEdge ) {
	navigator.ClearFailedEdge( failedEdge );
}

static void SV_Nav_ClearAllFailedEdges( void ) {
	navigator.ClearAllFailedEdges();
}

static int SV_Nav_EdgeFailed( int startID, int endID ) {
	return navigator.EdgeFailed( startID, endID );
}

static void SV_Nav_AddFailedEdge( int entID, int startID, int endID ) {
	navigator.AddFailedEdge( entID, startID, endID );
}

static qboolean SV_Nav_CheckFailedEdge( failedEdge_t *failedEdge ) {
	return navigator.CheckFailedEdge( failedEdge );
}

static void SV_Nav_CheckAllFailedEdges( void ) {
	navigator.CheckAllFailedEdges();
}

static qboolean SV_Nav_RouteBlocked( int startID, int testEdgeID, int endID, int rejectRank ) {
	return navigator.RouteBlocked( startID, testEdgeID, endID, rejectRank );
}

static int SV_Nav_GetBestNodeAltRoute( int startID, int endID, int *pathCost, int rejectID ) {
	return navigator.GetBestNodeAltRoute( startID, endID, pathCost, rejectID );
}

static int SV_Nav_GetBestNodeAltRoute2( int startID, int endID, int rejectID ) {
	return navigator.GetBestNodeAltRoute( startID, endID, rejectID );
}

static int SV_Nav_GetBestPathBetweenEnts( sharedEntity_t *ent, sharedEntity_t *goal, int flags ) {
	return navigator.GetBestPathBetweenEnts( ent, goal, flags );
}

static int SV_Nav_GetNodeRadius( int nodeID ) {
	return navigator.GetNodeRadius( nodeID );
}

static void SV_Nav_CheckBlockedEdges( void ) {
	navigator.CheckBlockedEdges();
}

static void SV_Nav_ClearCheckedNodes( void ) {
	navigator.ClearCheckedNodes();
}

static int SV_Nav_CheckedNode( int wayPoint, int ent ) {
	return navigator.CheckedNode( wayPoint, ent );
}

static void SV_Nav_SetCheckedNode( int wayPoint, int ent, int value ) {
	navigator.SetCheckedNode( wayPoint, ent, value );
}

static void SV_Nav_FlagAllNodes( int newFlag ) {
	navigator.FlagAllNodes( newFlag );
}

static qboolean SV_Nav_GetPathsCalculated( void ) {
	return navigator.pathsCalculated;
}

static void SV_Nav_SetPathsCalculated( qboolean newVal ) {
	navigator.pathsCalculated = newVal;
}

static int SV_BotLoadCharacter( char *charfile, float skill ) {
	return botlib_export->ai.BotLoadCharacter( charfile, skill );
}

static void SV_BotFreeCharacter( int character ) {
	botlib_export->ai.BotFreeCharacter( character );
}

static float SV_Characteristic_Float( int character, int index ) {
	return botlib_export->ai.Characteristic_Float( character, index );
}

static float SV_Characteristic_BFloat( int character, int index, float min, float max ) {
	return botlib_export->ai.Characteristic_BFloat( character, index, min, max );
}

static int SV_Characteristic_Integer( int character, int index ) {
	return botlib_export->ai.Characteristic_Integer( character, index );
}

static int SV_Characteristic_BInteger( int character, int index, int min, int max ) {
	return botlib_export->ai.Characteristic_BInteger( character, index, min, max );
}

static void SV_Characteristic_String( int character, int index, char *buf, int size ) {
	botlib_export->ai.Characteristic_String( character, index, buf, size );
}

static int SV_BotAllocChatState( void ) {
	return botlib_export->ai.BotAllocChatState();
}

static void SV_BotFreeChatState( int handle ) {
	botlib_export->ai.BotFreeChatState( handle );
}

static void SV_BotQueueConsoleMessage( int chatstate, int type, char *message ) {
	botlib_export->ai.BotQueueConsoleMessage( chatstate, type, message );
}

static void SV_BotRemoveConsoleMessage( int chatstate, int handle ) {
	botlib_export->ai.BotRemoveConsoleMessage( chatstate, handle );
}

static int SV_BotNextConsoleMessage( int chatstate, void *cm ) {
	return botlib_export->ai.BotNextConsoleMessage( chatstate, (bot_consolemessage_s *)cm );
}

static int SV_BotNumConsoleMessages( int chatstate ) {
	return botlib_export->ai.BotNumConsoleMessages( chatstate );
}

static void SV_BotInitialChat( int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	botlib_export->ai.BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

static int SV_BotReplyChat( int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	return botlib_export->ai.BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

static int SV_BotChatLength( int chatstate ) {
	return botlib_export->ai.BotChatLength( chatstate );
}

static void SV_BotEnterChat( int chatstate, int client, int sendto ) {
	botlib_export->ai.BotEnterChat( chatstate, client, sendto );
}

static int SV_StringContains( char *str1, char *str2, int casesensitive ) {
	return botlib_export->ai.StringContains( str1, str2, casesensitive );
}

static int SV_BotFindMatch( char *str, void *match, unsigned long int context ) {
	return botlib_export->ai.BotFindMatch( str, (bot_match_s *)match, context );
}

static void SV_BotMatchVariable( void *match, int variable, char *buf, int size ) {
	botlib_export->ai.BotMatchVariable( (bot_match_s *)match, variable, buf, size );
}

static void SV_UnifyWhiteSpaces( char *string ) {
	botlib_export->ai.UnifyWhiteSpaces( string );
}

static void SV_BotReplaceSynonyms( char *string, unsigned long int context ) {
	botlib_export->ai.BotReplaceSynonyms( string, context );
}

static int SV_BotLoadChatFile( int chatstate, char *chatfile, char *chatname ) {
	return botlib_export->ai.BotLoadChatFile( chatstate, chatfile, chatname );
}

static void SV_BotSetChatGender( int chatstate, int gender ) {
	botlib_export->ai.BotSetChatGender( chatstate, gender );
}

static void SV_BotSetChatName( int chatstate, char *name, int client ) {
	botlib_export->ai.BotSetChatName( chatstate, name, client );
}

static void SV_BotResetGoalState( int goalstate ) {
	botlib_export->ai.BotResetGoalState( goalstate );
}

static void SV_BotResetAvoidGoals( int goalstate ) {
	botlib_export->ai.BotResetAvoidGoals( goalstate );
}

static void SV_BotPushGoal( int goalstate, void *goal ) {
	botlib_export->ai.BotPushGoal( goalstate, (bot_goal_s *)goal );
}

static void SV_BotPopGoal( int goalstate ) {
	botlib_export->ai.BotPopGoal( goalstate );
}

static void SV_BotEmptyGoalStack( int goalstate ) {
	botlib_export->ai.BotEmptyGoalStack( goalstate );
}

static void SV_BotDumpAvoidGoals( int goalstate ) {
	botlib_export->ai.BotDumpAvoidGoals( goalstate );
}

static void SV_BotDumpGoalStack( int goalstate ) {
	botlib_export->ai.BotDumpGoalStack( goalstate );
}

static void SV_BotGoalName( int number, char *name, int size ) {
	botlib_export->ai.BotGoalName( number, name, size );
}

static int SV_BotGetTopGoal( int goalstate, void *goal ) {
	return botlib_export->ai.BotGetTopGoal( goalstate, (bot_goal_s *)goal );
}

static int SV_BotGetSecondGoal( int goalstate, void *goal ) {
	return botlib_export->ai.BotGetSecondGoal( goalstate, (bot_goal_s *)goal );
}

static int SV_BotChooseLTGItem( int goalstate, vec3_t origin, int *inventory, int travelflags ) {
	return botlib_export->ai.BotChooseLTGItem( goalstate, origin, inventory, travelflags );
}

static int SV_BotChooseNBGItem( int goalstate, vec3_t origin, int *inventory, int travelflags, void *ltg, float maxtime ) {
	return botlib_export->ai.BotChooseNBGItem( goalstate, origin, inventory, travelflags, (bot_goal_s *)ltg, maxtime );
}

static int SV_BotTouchingGoal( vec3_t origin, void *goal ) {
	return botlib_export->ai.BotTouchingGoal( origin, (bot_goal_s *)goal );
}

static int SV_BotItemGoalInVisButNotVisible( int viewer, vec3_t eye, vec3_t viewangles, void *goal ) {
	return botlib_export->ai.BotItemGoalInVisButNotVisible( viewer, eye, viewangles, (bot_goal_s *)goal );
}

static int SV_BotGetLevelItemGoal( int index, char *classname, void *goal ) {
	return botlib_export->ai.BotGetLevelItemGoal( index, classname, (bot_goal_s *)goal );
}

static float SV_BotAvoidGoalTime( int goalstate, int number ) {
	return botlib_export->ai.BotAvoidGoalTime( goalstate, number );
}

static void SV_BotInitLevelItems( void ) {
	botlib_export->ai.BotInitLevelItems();
}

static void SV_BotUpdateEntityItems( void ) {
	botlib_export->ai.BotUpdateEntityItems();
}

static int SV_BotLoadItemWeights( int goalstate, char *filename ) {
	return botlib_export->ai.BotLoadItemWeights( goalstate, filename );
}

static void SV_BotFreeItemWeights( int goalstate ) {
	botlib_export->ai.BotFreeItemWeights( goalstate );
}

static void SV_BotSaveGoalFuzzyLogic( int goalstate, char *filename ) {
	botlib_export->ai.BotSaveGoalFuzzyLogic( goalstate, filename );
}

static int SV_BotAllocGoalState( int state ) {
	return botlib_export->ai.BotAllocGoalState( state );
}

static void SV_BotFreeGoalState( int handle ) {
	botlib_export->ai.BotFreeGoalState( handle );
}

static void SV_BotResetMoveState( int movestate ) {
	botlib_export->ai.BotResetMoveState( movestate );
}

static void SV_BotMoveToGoal( void *result, int movestate, void *goal, int travelflags ) {
	botlib_export->ai.BotMoveToGoal( (bot_moveresult_s *)result, movestate, (bot_goal_s *)goal, travelflags );
}

static int SV_BotMoveInDirection( int movestate, vec3_t dir, float speed, int type ) {
	return botlib_export->ai.BotMoveInDirection( movestate, dir, speed, type );
}

static void SV_BotResetAvoidReach( int movestate ) {
	botlib_export->ai.BotResetAvoidReach( movestate );
}

static void SV_BotResetLastAvoidReach( int movestate ) {
	botlib_export->ai.BotResetLastAvoidReach( movestate );
}

static int SV_BotReachabilityArea( vec3_t origin, int testground ) {
	return botlib_export->ai.BotReachabilityArea( origin, testground );
}

static int SV_BotMovementViewTarget( int movestate, void *goal, int travelflags, float lookahead, vec3_t target ) {
	return botlib_export->ai.BotMovementViewTarget( movestate, (bot_goal_s *)goal, travelflags, lookahead, target );
}

static int SV_BotAllocMoveState( void ) {
	return botlib_export->ai.BotAllocMoveState();
}

static void SV_BotFreeMoveState( int handle ) {
	botlib_export->ai.BotFreeMoveState( handle );
}

static void SV_BotInitMoveState( int handle, void *initmove ) {
	botlib_export->ai.BotInitMoveState( handle, (bot_initmove_s *)initmove );
}

static int SV_BotChooseBestFightWeapon( int weaponstate, int *inventory ) {
	return botlib_export->ai.BotChooseBestFightWeapon( weaponstate, inventory );
}

static void SV_BotGetWeaponInfo( int weaponstate, int weapon, void *weaponinfo ) {
	botlib_export->ai.BotGetWeaponInfo( weaponstate, weapon, (weaponinfo_s *)weaponinfo );
}

static int SV_BotLoadWeaponWeights( int weaponstate, char *filename ) {
	return botlib_export->ai.BotLoadWeaponWeights( weaponstate, filename );
}

static int SV_BotAllocWeaponState( void ) {
	return botlib_export->ai.BotAllocWeaponState();
}

static void SV_BotFreeWeaponState( int weaponstate ) {
	botlib_export->ai.BotFreeWeaponState( weaponstate );
}

static void SV_BotResetWeaponState( int weaponstate ) {
	botlib_export->ai.BotResetWeaponState( weaponstate );
}

static int SV_GeneticParentsAndChildSelection( int numranks, float *ranks, int *parent1, int *parent2, int *child ) {
	return botlib_export->ai.GeneticParentsAndChildSelection( numranks, ranks, parent1, parent2, child );
}

static void SV_BotInterbreedGoalFuzzyLogic( int parent1, int parent2, int child ) {
	botlib_export->ai.BotInterbreedGoalFuzzyLogic( parent1, parent2, child );
}

static void SV_BotMutateGoalFuzzyLogic( int goalstate, float range ) {
	botlib_export->ai.BotMutateGoalFuzzyLogic( goalstate, range );
}

static int SV_BotGetNextCampSpotGoal( int num, void *goal ) {
	return botlib_export->ai.BotGetNextCampSpotGoal( num, (bot_goal_s *)goal );
}

static int SV_BotGetMapLocationGoal( char *name, void *goal ) {
	return botlib_export->ai.BotGetMapLocationGoal( name, (bot_goal_s *)goal );
}

static int SV_BotNumInitialChats( int chatstate, char *type ) {
	return botlib_export->ai.BotNumInitialChats( chatstate, type );
}

static void SV_BotGetChatMessage( int chatstate, char *buf, int size ) {
	botlib_export->ai.BotGetChatMessage( chatstate, buf, size );
}

static void SV_BotRemoveFromAvoidGoals( int goalstate, int number ) {
	botlib_export->ai.BotRemoveFromAvoidGoals( goalstate, number );
}

static int SV_BotPredictVisiblePosition( vec3_t origin, int areanum, void *goal, int travelflags, vec3_t target ) {
	return botlib_export->ai.BotPredictVisiblePosition( origin, areanum, (bot_goal_s *)goal, travelflags, target );
}

static void SV_BotSetAvoidGoalTime( int goalstate, int number, float avoidtime ) {
	botlib_export->ai.BotSetAvoidGoalTime( goalstate, number, avoidtime );
}

static void SV_BotAddAvoidSpot( int movestate, vec3_t origin, float radius, int type ) {
	botlib_export->ai.BotAddAvoidSpot( movestate, origin, radius, type );
}

static int SV_BotLibSetup( void ) {
	return botlib_export->BotLibSetup();
}

static int SV_BotLibShutdown( void ) {
	return botlib_export->BotLibShutdown();
}

static int SV_BotLibVarSet( char *var_name, char *value ) {
	return botlib_export->BotLibVarSet( var_name, value );
}

static int SV_BotLibVarGet( char *var_name, char *value, int size ) {
	return botlib_export->BotLibVarGet( var_name, value, size );
}

static int SV_BotLibDefine( char *string ) {
	return botlib_export->PC_AddGlobalDefine( string );
}

static int SV_BotLibStartFrame( float time ) {
	return botlib_export->BotLibStartFrame( time );
}

static int SV_BotLibLoadMap( const char *mapname ) {
	return botlib_export->BotLibLoadMap( mapname );
}

static int SV_BotLibUpdateEntity( int ent, void *bue ) {
	return botlib_export->BotLibUpdateEntity( ent, (bot_entitystate_t *)bue );
}

static int SV_BotLibTest( int parm0, char *parm1, vec3_t parm2, vec3_t parm3 ) {
	return botlib_export->Test( parm0, parm1, parm2, parm3 );
}

static int SV_BotGetServerCommand( int clientNum, char *message, int size ) {
	return SV_BotGetConsoleMessage( clientNum, message, size );
}

static void SV_BotUserCommand( int clientNum, usercmd_t *ucmd ) {
	SV_ClientThink( &svs.clients[clientNum], ucmd );
}

static int SV_AAS_EnableRoutingArea( int areanum, int enable ) {
	return botlib_export->aas.AAS_EnableRoutingArea( areanum, enable );
}

static int SV_AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas ) {
	return botlib_export->aas.AAS_BBoxAreas( absmins, absmaxs, areas, maxareas );
}

static int SV_AAS_AreaInfo( int areanum, void *info ) {
	return botlib_export->aas.AAS_AreaInfo( areanum, (aas_areainfo_s *)info );
}

static void SV_AAS_EntityInfo( int entnum, void *info ) {
	botlib_export->aas.AAS_EntityInfo( entnum, (aas_entityinfo_s *)info );
}

static int SV_AAS_Initialized( void ) {
	return botlib_export->aas.AAS_Initialized();
}

static void SV_AAS_PresenceTypeBoundingBox( int presencetype, vec3_t mins, vec3_t maxs ) {
	botlib_export->aas.AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
}

static float SV_AAS_Time( void ) {
	return botlib_export->aas.AAS_Time();
}

static int SV_AAS_PointAreaNum( vec3_t point ) {
	return botlib_export->aas.AAS_PointAreaNum( point );
}

static int SV_AAS_TraceAreas( vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas ) {
	return botlib_export->aas.AAS_TraceAreas( start, end, areas, points, maxareas );
}

static int SV_AAS_PointContents( vec3_t point ) {
	return botlib_export->aas.AAS_PointContents( point );
}

static int SV_AAS_NextBSPEntity( int ent ) {
	return botlib_export->aas.AAS_NextBSPEntity( ent );
}

static int SV_AAS_ValueForBSPEpairKey( int ent, char *key, char *value, int size ) {
	return botlib_export->aas.AAS_ValueForBSPEpairKey( ent, key, value, size );
}

static int SV_AAS_VectorForBSPEpairKey( int ent, char *key, vec3_t v ) {
	return botlib_export->aas.AAS_VectorForBSPEpairKey( ent, key, v );
}

static int SV_AAS_FloatForBSPEpairKey( int ent, char *key, float *value ) {
	return botlib_export->aas.AAS_FloatForBSPEpairKey( ent, key, value );
}

static int SV_AAS_IntForBSPEpairKey( int ent, char *key, int *value ) {
	return botlib_export->aas.AAS_IntForBSPEpairKey( ent, key, value );
}

static int SV_AAS_AreaReachability( int areanum ) {
	return botlib_export->aas.AAS_AreaReachability( areanum );
}

static int SV_AAS_AreaTravelTimeToGoalArea( int areanum, vec3_t origin, int goalareanum, int travelflags ) {
	return botlib_export->aas.AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
}

static int SV_AAS_Swimming( vec3_t origin ) {
	return botlib_export->aas.AAS_Swimming( origin );
}

static int SV_AAS_PredictClientMovement( void *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize ) {
	return botlib_export->aas.AAS_PredictClientMovement( (aas_clientmove_s *)move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, visualize );
}

static int SV_AAS_AlternativeRouteGoals( vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, void *altroutegoals, int maxaltroutegoals, int type ) {
	return botlib_export->aas.AAS_AlternativeRouteGoals( start, startareanum, goal, goalareanum, travelflags, (aas_altroutegoal_s *)altroutegoals, maxaltroutegoals, type );
}

static int SV_AAS_PredictRoute( void *route, int areanum, vec3_t origin, int goalareanum, int travelflags, int maxareas, int maxtime, int stopevent, int stopcontents, int stoptfl, int stopareanum ) {
	return botlib_export->aas.AAS_PredictRoute( (aas_predictroute_s *)route, areanum, origin, goalareanum, travelflags, maxareas, maxtime, stopevent, stopcontents, stoptfl, stopareanum );
}

static int SV_AAS_PointReachabilityAreaIndex( vec3_t point ) {
	return botlib_export->aas.AAS_PointReachabilityAreaIndex( point );
}

static void SV_EA_Say( int client, char *str ) {
	botlib_export->ea.EA_Say( client, str );
}

static void SV_EA_SayTeam( int client, char *str ) {
	botlib_export->ea.EA_SayTeam( client, str );
}

static void SV_EA_Command( int client, char *command ) {
	botlib_export->ea.EA_Command( client, command );
}

static void SV_EA_Action( int client, int action ) {
	botlib_export->ea.EA_Action( client, action );
}

static void SV_EA_Gesture( int client ) {
	botlib_export->ea.EA_Gesture( client );
}

static void SV_EA_Talk( int client ) {
	botlib_export->ea.EA_Talk( client );
}

static void SV_EA_Attack( int client ) {
	botlib_export->ea.EA_Attack( client );
}

static void SV_EA_Alt_Attack( int client ) {
	botlib_export->ea.EA_Alt_Attack( client );
}

static void SV_EA_ForcePower( int client ) {
	botlib_export->ea.EA_ForcePower( client );
}

static void SV_EA_Use( int client ) {
	botlib_export->ea.EA_Use( client );
}

static void SV_EA_Respawn( int client ) {
	botlib_export->ea.EA_Respawn( client );
}

static void SV_EA_Crouch( int client ) {
	botlib_export->ea.EA_Crouch( client );
}

static void SV_EA_MoveUp( int client ) {
	botlib_export->ea.EA_MoveUp( client );
}

static void SV_EA_MoveDown( int client ) {
	botlib_export->ea.EA_MoveDown( client );
}

static void SV_EA_MoveForward( int client ) {
	botlib_export->ea.EA_MoveForward( client );
}

static void SV_EA_MoveBack( int client ) {
	botlib_export->ea.EA_MoveBack( client );
}

static void SV_EA_MoveLeft( int client ) {
	botlib_export->ea.EA_MoveLeft( client );
}

static void SV_EA_MoveRight( int client ) {
	botlib_export->ea.EA_MoveRight( client );
}

static void SV_EA_SelectWeapon( int client, int weapon ) {
	botlib_export->ea.EA_SelectWeapon( client, weapon );
}

static void SV_EA_Jump( int client ) {
	botlib_export->ea.EA_Jump( client );
}

static void SV_EA_DelayedJump( int client ) {
	botlib_export->ea.EA_DelayedJump( client );
}

static void SV_EA_Move( int client, vec3_t dir, float speed ) {
	botlib_export->ea.EA_Move( client, dir, speed );
}

static void SV_EA_View( int client, vec3_t viewangles ) {
	botlib_export->ea.EA_View( client, viewangles );
}

static void SV_EA_EndRegular( int client, float thinktime ) {
	botlib_export->ea.EA_EndRegular( client, thinktime );
}

static void SV_EA_GetInput( int client, float thinktime, void *input ) {
	botlib_export->ea.EA_GetInput( client, thinktime, (bot_input_t *)input );
}

static void SV_EA_ResetInput( int client ) {
	botlib_export->ea.EA_ResetInput( client );
}

static int SV_PC_LoadSource( const char *filename ) {
	return botlib_export->PC_LoadSourceHandle( filename );
}

static int SV_PC_FreeSource( int handle ) {
	return botlib_export->PC_FreeSourceHandle( handle );
}

static int SV_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return botlib_export->PC_ReadTokenHandle( handle, pc_token );
}

static int SV_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return botlib_export->PC_SourceFileAndLine( handle, filename, line );
}

static qhandle_t SV_RE_RegisterSkin( const char *name ) {
	return re->RegisterServerSkin( name );
}

static int SV_CM_RegisterTerrain( const char *config ) {
	return 0;
}

static void SV_RMG_Init( void ) { }

static void SV_G2API_ListModelSurfaces( void *ghlInfo ) {
	re->G2API_ListSurfaces( (CGhoul2Info *)ghlInfo );
}

static void SV_G2API_ListModelBones( void *ghlInfo, int frame ) {
	re->G2API_ListBones( (CGhoul2Info *)ghlInfo, frame );
}

static void SV_G2API_SetGhoul2ModelIndexes( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList ) {
	if ( !ghoul2 ) return;
	re->G2API_SetGhoul2ModelIndexes( *((CGhoul2Info_v *)ghoul2), modelList, skinList );
}

static qboolean SV_G2API_HaveWeGhoul2Models( void *ghoul2) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_HaveWeGhoul2Models( *((CGhoul2Info_v *)ghoul2) );
}

static qboolean SV_G2API_GetBoltMatrix( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean SV_G2API_GetBoltMatrix_NoReconstruct( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixReconstruction( qfalse );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean SV_G2API_GetBoltMatrix_NoRecNoRot( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixReconstruction( qfalse );
	re->G2API_BoltMatrixSPMethod( qtrue );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static int SV_G2API_InitGhoul2Model( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 1;
#endif
	return re->G2API_InitGhoul2Model( (CGhoul2Info_v **)ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias );
}

static qboolean SV_G2API_SetSkin( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_SetSkin( g2, modelIndex, customSkin, renderSkin );
}

static void SV_G2API_CollisionDetect( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetect( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, rayStart, rayEnd, scale, G2VertSpaceServer, traceFlags, useLod, fRadius );
}

static void SV_G2API_CollisionDetectCache( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, vec3_t rayStart, vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetectCache( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, rayStart, rayEnd, scale, G2VertSpaceServer, traceFlags, useLod, fRadius );
}

static void SV_G2API_CleanGhoul2Models( void **ghoul2Ptr ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 1;
#endif
	re->G2API_CleanGhoul2Models( (CGhoul2Info_v **)ghoul2Ptr );
}

static qboolean SV_G2API_SetBoneAngles( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags, const int up, const int right, const int forward, qhandle_t *modelList, int blendTime , int currentTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAngles( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, angles, flags, (const Eorientations)up, (const Eorientations)right, (const Eorientations)forward, modelList, blendTime , currentTime );
}

static qboolean SV_G2API_SetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame, const int flags, const float animSpeed, const int currentTime, const float setFrame, const int blendTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAnim( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, startFrame, endFrame, flags, animSpeed, currentTime, setFrame, blendTime );
}

static qboolean SV_G2API_GetBoneAnim( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetBoneAnim( g2, modelIndex, boneName, currentTime, currentFrame, startFrame, endFrame, flags, animSpeed, modelList );
}

static void SV_G2API_GetGLAName( void *ghoul2, int modelIndex, char *fillBuf ) {
	if ( !ghoul2 )
	{
		fillBuf[0] = '\0';
		return;
	}

	char *tmp = re->G2API_GetGLAName( *((CGhoul2Info_v *)ghoul2), modelIndex );
	if ( tmp )
		strcpy( fillBuf, tmp );
	else
		fillBuf[0] = '\0';
}

static int SV_G2API_CopyGhoul2Instance( void *g2From, void *g2To, int modelIndex ) {
	if ( !g2From || !g2To ) return 0;
	return re->G2API_CopyGhoul2Instance( *((CGhoul2Info_v *)g2From), *((CGhoul2Info_v *)g2To), modelIndex );
}

static void SV_G2API_CopySpecificGhoul2Model( void *g2From, int modelFrom, void *g2To, int modelTo ) {
	if ( !g2From || !g2To) return;
	re->G2API_CopySpecificG2Model( *((CGhoul2Info_v *)g2From), modelFrom, *((CGhoul2Info_v *)g2To), modelTo );
}

static void SV_G2API_DuplicateGhoul2Instance( void *g2From, void **g2To ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 1;
#endif
	if ( !g2From || !g2To) return;
	re->G2API_DuplicateGhoul2Instance( *((CGhoul2Info_v *)g2From), (CGhoul2Info_v **)g2To );
}

static qboolean SV_G2API_HasGhoul2ModelOnIndex( void *ghlInfo, int modelIndex ) {
	return re->G2API_HasGhoul2ModelOnIndex( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean SV_G2API_RemoveGhoul2Model( void *ghlInfo, int modelIndex ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 1;
#endif
	return re->G2API_RemoveGhoul2Model( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean SV_G2API_RemoveGhoul2Models( void *ghlInfo ) {
#ifdef _FULL_G2_LEAK_CHECKING
	g_G2AllocServer = 1;
#endif
	return re->G2API_RemoveGhoul2Models( (CGhoul2Info_v **)ghlInfo );
}

static int SV_G2API_Ghoul2Size( void *ghlInfo ) {
	if ( !ghlInfo ) return 0;
	return re->G2API_Ghoul2Size( *((CGhoul2Info_v *)ghlInfo) );
}

static int SV_G2API_AddBolt( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return 0;
	return re->G2API_AddBolt( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName );
}

static void SV_G2API_SetBoltInfo( void *ghoul2, int modelIndex, int boltInfo ) {
	if ( !ghoul2 ) return;
	re->G2API_SetBoltInfo( *((CGhoul2Info_v *)ghoul2), modelIndex, boltInfo );
}

static qboolean SV_G2API_SetRootSurface( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetRootSurface( *((CGhoul2Info_v *)ghoul2), modelIndex, surfaceName );
}

static qboolean SV_G2API_SetSurfaceOnOff( void *ghoul2, const char *surfaceName, const int flags ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetSurfaceOnOff( *((CGhoul2Info_v *)ghoul2), surfaceName, flags );
}

static qboolean SV_G2API_SetNewOrigin( void *ghoul2, const int boltIndex ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetNewOrigin( *((CGhoul2Info_v *)ghoul2), boltIndex );
}

static qboolean SV_G2API_DoesBoneExist( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_DoesBoneExist( g2, modelIndex, boneName );
}

static int SV_G2API_GetSurfaceRenderStatus( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return -1;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetSurfaceRenderStatus( g2, modelIndex, surfaceName );
}

static void SV_G2API_AbsurdSmoothing( void *ghoul2, qboolean status ) {
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	re->G2API_AbsurdSmoothing( g2, status );
}

static void SV_G2API_SetRagDoll( void *ghoul2, sharedRagDollParams_t *params ) {
	if ( !ghoul2 ) return;

	CRagDollParams rdParams;

	if ( !params ) {
		re->G2API_ResetRagDoll( *((CGhoul2Info_v *)ghoul2) );
		return;
	}

	VectorCopy( params->angles, rdParams.angles );
	VectorCopy( params->position, rdParams.position );
	VectorCopy( params->scale, rdParams.scale );
	VectorCopy( params->pelvisAnglesOffset, rdParams.pelvisAnglesOffset );
	VectorCopy( params->pelvisPositionOffset, rdParams.pelvisPositionOffset );

	rdParams.fImpactStrength = params->fImpactStrength;
	rdParams.fShotStrength = params->fShotStrength;
	rdParams.me = params->me;

	rdParams.startFrame = params->startFrame;
	rdParams.endFrame = params->endFrame;

	rdParams.collisionType = params->collisionType;
	rdParams.CallRagDollBegin = params->CallRagDollBegin;

	rdParams.RagPhase = (CRagDollParams::ERagPhase)params->RagPhase;
	rdParams.effectorsToTurnOff = (CRagDollParams::ERagEffector)params->effectorsToTurnOff;

	re->G2API_SetRagDoll( *((CGhoul2Info_v *)ghoul2), &rdParams );
}

static void SV_G2API_AnimateG2Models( void *ghoul2, int time, sharedRagDollUpdateParams_t *params ) {
	if ( !ghoul2 ) return;
	if ( !params ) return;

	CRagDollUpdateParams rduParams;

	VectorCopy( params->angles, rduParams.angles );
	VectorCopy( params->position, rduParams.position );
	VectorCopy( params->scale, rduParams.scale );
	VectorCopy( params->velocity, rduParams.velocity );

	rduParams.me = params->me;
	rduParams.settleFrame = params->settleFrame;

	re->G2API_AnimateG2ModelsRag( *((CGhoul2Info_v *)ghoul2), time, &rduParams );
}

static qboolean SV_G2API_RagPCJConstraint( void *ghoul2, const char *boneName, vec3_t min, vec3_t max ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJConstraint( *((CGhoul2Info_v *)ghoul2), boneName, min, max );
}

static qboolean SV_G2API_RagPCJGradientSpeed( void *ghoul2, const char *boneName, const float speed ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJGradientSpeed( *((CGhoul2Info_v *)ghoul2), boneName, speed );
}

static qboolean SV_G2API_RagEffectorGoal( void *ghoul2, const char *boneName, vec3_t pos ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorGoal( *((CGhoul2Info_v *)ghoul2), boneName, pos );
}

static qboolean SV_G2API_GetRagBonePos( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetRagBonePos( *((CGhoul2Info_v *)ghoul2), boneName, pos, entAngles, entPos, entScale );
}

static qboolean SV_G2API_RagEffectorKick( void *ghoul2, const char *boneName, vec3_t velocity ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorKick( *((CGhoul2Info_v *)ghoul2), boneName, velocity );
}

static qboolean SV_G2API_RagForceSolve( void *ghoul2, qboolean force ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagForceSolve( *((CGhoul2Info_v *)ghoul2), force );
}

static qboolean SV_G2API_SetBoneIKState( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneIKState( *((CGhoul2Info_v *)ghoul2), time, boneName, ikState, params );
}

static qboolean SV_G2API_IKMove( void *ghoul2, int time, sharedIKMoveParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_IKMove( *((CGhoul2Info_v *)ghoul2), time, params );
}

static qboolean SV_G2API_RemoveBone( void *ghoul2, const char *boneName, int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_RemoveBone( g2, modelIndex, boneName );
}

static void SV_G2API_AttachInstanceToEntNum( void *ghoul2, int entityNum, qboolean server ) {
	if ( !ghoul2 ) return;
	re->G2API_AttachInstanceToEntNum( *((CGhoul2Info_v *)ghoul2), entityNum, server );
}

static void SV_G2API_ClearAttachedInstance( int entityNum ) {
	re->G2API_ClearAttachedInstance( entityNum );
}

static void SV_G2API_CleanEntAttachments( void ) {
	re->G2API_CleanEntAttachments();
}

static qboolean SV_G2API_OverrideServer( void *serverInstance ) {
	if ( !serverInstance ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)serverInstance);
	return re->G2API_OverrideServerWithClientData( g2, 0 );
}

static void SV_G2API_GetSurfaceName( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf ) {
	//NOTE CHECKME should this store empty fillBuf like g2api getglaname does same thing if tmp fails?
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	char *tmp = re->G2API_GetSurfaceName( g2, modelIndex, surfNumber );
	strcpy( fillBuf, tmp );
}

static void GVM_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_VM_Set( var_name, value, VM_GAME );
}

void SV_InitGame( qboolean restart ) {
	int i=0;
	client_t *cl = NULL;

	// clear level pointers
	sv.entityParsePoint = CM_EntityString();
	for ( i=0, cl=svs.clients; i<sv_maxclients->integer; i++, cl++ )
		cl->gentity = NULL;

	GVM_InitGame( sv.time, Com_Milliseconds(), restart );
}

void SV_BindGame( void ) {
	static gameImport_t gi;
	gameExport_t		*ret;
	GetGameAPI_t		GetGameAPI;
	char				dllName[MAX_OSPATH] = "game" ARCH_STRING DLL_EXT;

	memset( &gi, 0, sizeof( gi ) );

	gvm = VM_Create( VM_GAME );
	if ( !gvm ) {
		svs.gameStarted = qfalse;
		Com_Error( ERR_DROP, "VM_Create on game failed" );
	}

	gi.Print								= Com_Printf;
	gi.Error								= Com_Error;
	gi.Milliseconds							= Com_Milliseconds;
	gi.PrecisionTimerStart					= SV_PrecisionTimerStart;
	gi.PrecisionTimerEnd					= SV_PrecisionTimerEnd;
	gi.SV_RegisterSharedMemory				= SV_RegisterSharedMemory;
	gi.RealTime								= Com_RealTime;
	gi.TrueMalloc							= VM_Shifted_Alloc;
	gi.TrueFree								= VM_Shifted_Free;
	gi.SnapVector							= Sys_SnapVector;
	gi.Cvar_Register						= Cvar_Register;
	gi.Cvar_Set								= GVM_Cvar_Set;
	gi.Cvar_Update							= Cvar_Update;
	gi.Cvar_VariableIntegerValue			= Cvar_VariableIntegerValue;
	gi.Cvar_VariableStringBuffer			= Cvar_VariableStringBuffer;
	gi.Argc									= Cmd_Argc;
	gi.Argv									= Cmd_ArgvBuffer;
	gi.FS_Close								= FS_FCloseFile;
	gi.FS_GetFileList						= FS_GetFileList;
	gi.FS_Open								= FS_FOpenFileByMode;
	gi.FS_Read								= FS_Read;
	gi.FS_Write								= FS_Write;
	gi.AdjustAreaPortalState				= SV_AdjustAreaPortalState;
	gi.AreasConnected						= CM_AreasConnected;
	gi.DebugPolygonCreate					= BotImport_DebugPolygonCreate;
	gi.DebugPolygonDelete					= BotImport_DebugPolygonDelete;
	gi.DropClient							= SV_GameDropClient;
	gi.EntitiesInBox						= SV_AreaEntities;
	gi.EntityContact						= SV_EntityContact;
	gi.Trace								= SV_Trace;
	gi.GetConfigstring						= SV_GetConfigstring;
	gi.GetEntityToken						= SV_GetEntityToken;
	gi.GetServerinfo						= SV_GetServerinfo;
	gi.GetUsercmd							= SV_GetUsercmd;
	gi.GetUserinfo							= SV_GetUserinfo;
	gi.InPVS								= SV_inPVS;
	gi.InPVSIgnorePortals					= SV_inPVSIgnorePortals;
	gi.LinkEntity							= SV_LinkEntity;
	gi.LocateGameData						= SV_LocateGameData;
	gi.PointContents						= SV_PointContents;
	gi.SendConsoleCommand					= Cbuf_ExecuteText;
	gi.SendServerCommand					= SV_GameSendServerCommand;
	gi.SetBrushModel						= SV_SetBrushModel;
	gi.SetConfigstring						= SV_SetConfigstring;
	gi.SetServerCull						= SV_SetServerCull;
	gi.SetUserinfo							= SV_SetUserinfo;
	gi.UnlinkEntity							= SV_UnlinkEntity;
	gi.ROFF_Clean							= SV_ROFF_Clean;
	gi.ROFF_UpdateEntities					= SV_ROFF_UpdateEntities;
	gi.ROFF_Cache							= SV_ROFF_Cache;
	gi.ROFF_Play							= SV_ROFF_Play;
	gi.ROFF_Purge_Ent						= SV_ROFF_Purge_Ent;
	gi.ICARUS_RunScript						= ICARUS_RunScript;
	gi.ICARUS_RegisterScript				= SV_ICARUS_RegisterScript;
	gi.ICARUS_Init							= ICARUS_Init;
	gi.ICARUS_ValidEnt						= SV_ICARUS_ValidEnt;
	gi.ICARUS_IsInitialized					= ICARUS_IsInitialized;
	gi.ICARUS_MaintainTaskManager			= ICARUS_MaintainTaskManager;
	gi.ICARUS_IsRunning						= ICARUS_IsRunning;
	gi.ICARUS_TaskIDPending					= ICARUS_TaskIDPending;
	gi.ICARUS_InitEnt						= ICARUS_InitEnt;
	gi.ICARUS_FreeEnt						= ICARUS_FreeEnt;
	gi.ICARUS_AssociateEnt					= ICARUS_AssociateEnt;
	gi.ICARUS_Shutdown						= ICARUS_Shutdown;
	gi.ICARUS_TaskIDSet						= SV_ICARUS_TaskIDSet;
	gi.ICARUS_TaskIDComplete				= SV_ICARUS_TaskIDComplete;
	gi.ICARUS_SetVar						= Q3_SetVar;
	gi.ICARUS_VariableDeclared				= Q3_VariableDeclared;
	gi.ICARUS_GetFloatVariable				= Q3_GetFloatVariable;
	gi.ICARUS_GetStringVariable				= SV_ICARUS_GetStringVariable;
	gi.ICARUS_GetVectorVariable				= SV_ICARUS_GetVectorVariable;
	gi.Nav_Init								= SV_Nav_Init;
	gi.Nav_Free								= SV_Nav_Free;
	gi.Nav_Load								= SV_Nav_Load;
	gi.Nav_Save								= SV_Nav_Save;
	gi.Nav_AddRawPoint						= SV_Nav_AddRawPoint;
	gi.Nav_CalculatePaths					= SV_Nav_CalculatePaths;
	gi.Nav_HardConnect						= SV_Nav_HardConnect;
	gi.Nav_ShowNodes						= SV_Nav_ShowNodes;
	gi.Nav_ShowEdges						= SV_Nav_ShowEdges;
	gi.Nav_ShowPath							= SV_Nav_ShowPath;
	gi.Nav_GetNearestNode					= SV_Nav_GetNearestNode;
	gi.Nav_GetBestNode						= SV_Nav_GetBestNode;
	gi.Nav_GetNodePosition					= SV_Nav_GetNodePosition;
	gi.Nav_GetNodeNumEdges					= SV_Nav_GetNodeNumEdges;
	gi.Nav_GetNodeEdge						= SV_Nav_GetNodeEdge;
	gi.Nav_GetNumNodes						= SV_Nav_GetNumNodes;
	gi.Nav_Connected						= SV_Nav_Connected;
	gi.Nav_GetPathCost						= SV_Nav_GetPathCost;
	gi.Nav_GetEdgeCost						= SV_Nav_GetEdgeCost;
	gi.Nav_GetProjectedNode					= SV_Nav_GetProjectedNode;
	gi.Nav_CheckFailedNodes					= SV_Nav_CheckFailedNodes;
	gi.Nav_AddFailedNode					= SV_Nav_AddFailedNode;
	gi.Nav_NodeFailed						= SV_Nav_NodeFailed;
	gi.Nav_NodesAreNeighbors				= SV_Nav_NodesAreNeighbors;
	gi.Nav_ClearFailedEdge					= SV_Nav_ClearFailedEdge;
	gi.Nav_ClearAllFailedEdges				= SV_Nav_ClearAllFailedEdges;
	gi.Nav_EdgeFailed						= SV_Nav_EdgeFailed;
	gi.Nav_AddFailedEdge					= SV_Nav_AddFailedEdge;
	gi.Nav_CheckFailedEdge					= SV_Nav_CheckFailedEdge;
	gi.Nav_CheckAllFailedEdges				= SV_Nav_CheckAllFailedEdges;
	gi.Nav_RouteBlocked						= SV_Nav_RouteBlocked;
	gi.Nav_GetBestNodeAltRoute				= SV_Nav_GetBestNodeAltRoute;
	gi.Nav_GetBestNodeAltRoute2				= SV_Nav_GetBestNodeAltRoute2;
	gi.Nav_GetBestPathBetweenEnts			= SV_Nav_GetBestPathBetweenEnts;
	gi.Nav_GetNodeRadius					= SV_Nav_GetNodeRadius;
	gi.Nav_CheckBlockedEdges				= SV_Nav_CheckBlockedEdges;
	gi.Nav_ClearCheckedNodes				= SV_Nav_ClearCheckedNodes;
	gi.Nav_CheckedNode						= SV_Nav_CheckedNode;
	gi.Nav_SetCheckedNode					= SV_Nav_SetCheckedNode;
	gi.Nav_FlagAllNodes						= SV_Nav_FlagAllNodes;
	gi.Nav_GetPathsCalculated				= SV_Nav_GetPathsCalculated;
	gi.Nav_SetPathsCalculated				= SV_Nav_SetPathsCalculated;
	gi.BotAllocateClient					= SV_BotAllocateClient;
	gi.BotFreeClient						= SV_BotFreeClient;
	gi.BotLoadCharacter						= SV_BotLoadCharacter;
	gi.BotFreeCharacter						= SV_BotFreeCharacter;
	gi.Characteristic_Float					= SV_Characteristic_Float;
	gi.Characteristic_BFloat				= SV_Characteristic_BFloat;
	gi.Characteristic_Integer				= SV_Characteristic_Integer;
	gi.Characteristic_BInteger				= SV_Characteristic_BInteger;
	gi.Characteristic_String				= SV_Characteristic_String;
	gi.BotAllocChatState					= SV_BotAllocChatState;
	gi.BotFreeChatState						= SV_BotFreeChatState;
	gi.BotQueueConsoleMessage				= SV_BotQueueConsoleMessage;
	gi.BotRemoveConsoleMessage				= SV_BotRemoveConsoleMessage;
	gi.BotNextConsoleMessage				= SV_BotNextConsoleMessage;
	gi.BotNumConsoleMessages				= SV_BotNumConsoleMessages;
	gi.BotInitialChat						= SV_BotInitialChat;
	gi.BotReplyChat							= SV_BotReplyChat;
	gi.BotChatLength						= SV_BotChatLength;
	gi.BotEnterChat							= SV_BotEnterChat;
	gi.StringContains						= SV_StringContains;
	gi.BotFindMatch							= SV_BotFindMatch;
	gi.BotMatchVariable						= SV_BotMatchVariable;
	gi.UnifyWhiteSpaces						= SV_UnifyWhiteSpaces;
	gi.BotReplaceSynonyms					= SV_BotReplaceSynonyms;
	gi.BotLoadChatFile						= SV_BotLoadChatFile;
	gi.BotSetChatGender						= SV_BotSetChatGender;
	gi.BotSetChatName						= SV_BotSetChatName;
	gi.BotResetGoalState					= SV_BotResetGoalState;
	gi.BotResetAvoidGoals					= SV_BotResetAvoidGoals;
	gi.BotPushGoal							= SV_BotPushGoal;
	gi.BotPopGoal							= SV_BotPopGoal;
	gi.BotEmptyGoalStack					= SV_BotEmptyGoalStack;
	gi.BotDumpAvoidGoals					= SV_BotDumpAvoidGoals;
	gi.BotDumpGoalStack						= SV_BotDumpGoalStack;
	gi.BotGoalName							= SV_BotGoalName;
	gi.BotGetTopGoal						= SV_BotGetTopGoal;
	gi.BotGetSecondGoal						= SV_BotGetSecondGoal;
	gi.BotChooseLTGItem						= SV_BotChooseLTGItem;
	gi.BotChooseNBGItem						= SV_BotChooseNBGItem;
	gi.BotTouchingGoal						= SV_BotTouchingGoal;
	gi.BotItemGoalInVisButNotVisible		= SV_BotItemGoalInVisButNotVisible;
	gi.BotGetLevelItemGoal					= SV_BotGetLevelItemGoal;
	gi.BotAvoidGoalTime						= SV_BotAvoidGoalTime;
	gi.BotInitLevelItems					= SV_BotInitLevelItems;
	gi.BotUpdateEntityItems					= SV_BotUpdateEntityItems;
	gi.BotLoadItemWeights					= SV_BotLoadItemWeights;
	gi.BotFreeItemWeights					= SV_BotFreeItemWeights;
	gi.BotSaveGoalFuzzyLogic				= SV_BotSaveGoalFuzzyLogic;
	gi.BotAllocGoalState					= SV_BotAllocGoalState;
	gi.BotFreeGoalState						= SV_BotFreeGoalState;
	gi.BotResetMoveState					= SV_BotResetMoveState;
	gi.BotMoveToGoal						= SV_BotMoveToGoal;
	gi.BotMoveInDirection					= SV_BotMoveInDirection;
	gi.BotResetAvoidReach					= SV_BotResetAvoidReach;
	gi.BotResetLastAvoidReach				= SV_BotResetLastAvoidReach;
	gi.BotReachabilityArea					= SV_BotReachabilityArea;
	gi.BotMovementViewTarget				= SV_BotMovementViewTarget;
	gi.BotAllocMoveState					= SV_BotAllocMoveState;
	gi.BotFreeMoveState						= SV_BotFreeMoveState;
	gi.BotInitMoveState						= SV_BotInitMoveState;
	gi.BotChooseBestFightWeapon				= SV_BotChooseBestFightWeapon;
	gi.BotGetWeaponInfo						= SV_BotGetWeaponInfo;
	gi.BotLoadWeaponWeights					= SV_BotLoadWeaponWeights;
	gi.BotAllocWeaponState					= SV_BotAllocWeaponState;
	gi.BotFreeWeaponState					= SV_BotFreeWeaponState;
	gi.BotResetWeaponState					= SV_BotResetWeaponState;
	gi.GeneticParentsAndChildSelection		= SV_GeneticParentsAndChildSelection;
	gi.BotInterbreedGoalFuzzyLogic			= SV_BotInterbreedGoalFuzzyLogic;
	gi.BotMutateGoalFuzzyLogic				= SV_BotMutateGoalFuzzyLogic;
	gi.BotGetNextCampSpotGoal				= SV_BotGetNextCampSpotGoal;
	gi.BotGetMapLocationGoal				= SV_BotGetMapLocationGoal;
	gi.BotNumInitialChats					= SV_BotNumInitialChats;
	gi.BotGetChatMessage					= SV_BotGetChatMessage;
	gi.BotRemoveFromAvoidGoals				= SV_BotRemoveFromAvoidGoals;
	gi.BotPredictVisiblePosition			= SV_BotPredictVisiblePosition;
	gi.BotSetAvoidGoalTime					= SV_BotSetAvoidGoalTime;
	gi.BotAddAvoidSpot						= SV_BotAddAvoidSpot;
	gi.BotLibSetup							= SV_BotLibSetup;
	gi.BotLibShutdown						= SV_BotLibShutdown;
	gi.BotLibVarSet							= SV_BotLibVarSet;
	gi.BotLibVarGet							= SV_BotLibVarGet;
	gi.BotLibDefine							= SV_BotLibDefine;
	gi.BotLibStartFrame						= SV_BotLibStartFrame;
	gi.BotLibLoadMap						= SV_BotLibLoadMap;
	gi.BotLibUpdateEntity					= SV_BotLibUpdateEntity;
	gi.BotLibTest							= SV_BotLibTest;
	gi.BotGetSnapshotEntity					= SV_BotGetSnapshotEntity;
	gi.BotGetServerCommand					= SV_BotGetServerCommand;
	gi.BotUserCommand						= SV_BotUserCommand;
	gi.BotUpdateWaypoints					= SV_BotWaypointReception;
	gi.BotCalculatePaths					= SV_BotCalculatePaths;
	gi.AAS_EnableRoutingArea				= SV_AAS_EnableRoutingArea;
	gi.AAS_BBoxAreas						= SV_AAS_BBoxAreas;
	gi.AAS_AreaInfo							= SV_AAS_AreaInfo;
	gi.AAS_EntityInfo						= SV_AAS_EntityInfo;
	gi.AAS_Initialized						= SV_AAS_Initialized;
	gi.AAS_PresenceTypeBoundingBox			= SV_AAS_PresenceTypeBoundingBox;
	gi.AAS_Time								= SV_AAS_Time;
	gi.AAS_PointAreaNum						= SV_AAS_PointAreaNum;
	gi.AAS_TraceAreas						= SV_AAS_TraceAreas;
	gi.AAS_PointContents					= SV_AAS_PointContents;
	gi.AAS_NextBSPEntity					= SV_AAS_NextBSPEntity;
	gi.AAS_ValueForBSPEpairKey				= SV_AAS_ValueForBSPEpairKey;
	gi.AAS_VectorForBSPEpairKey				= SV_AAS_VectorForBSPEpairKey;
	gi.AAS_FloatForBSPEpairKey				= SV_AAS_FloatForBSPEpairKey;
	gi.AAS_IntForBSPEpairKey				= SV_AAS_IntForBSPEpairKey;
	gi.AAS_AreaReachability					= SV_AAS_AreaReachability;
	gi.AAS_AreaTravelTimeToGoalArea			= SV_AAS_AreaTravelTimeToGoalArea;
	gi.AAS_Swimming							= SV_AAS_Swimming;
	gi.AAS_PredictClientMovement			= SV_AAS_PredictClientMovement;
	gi.AAS_AlternativeRouteGoals			= SV_AAS_AlternativeRouteGoals;
	gi.AAS_PredictRoute						= SV_AAS_PredictRoute;
	gi.AAS_PointReachabilityAreaIndex		= SV_AAS_PointReachabilityAreaIndex;
	gi.EA_Say								= SV_EA_Say;
	gi.EA_SayTeam							= SV_EA_SayTeam;
	gi.EA_Command							= SV_EA_Command;
	gi.EA_Action							= SV_EA_Action;
	gi.EA_Gesture							= SV_EA_Gesture;
	gi.EA_Talk								= SV_EA_Talk;
	gi.EA_Attack							= SV_EA_Attack;
	gi.EA_Alt_Attack						= SV_EA_Alt_Attack;
	gi.EA_ForcePower						= SV_EA_ForcePower;
	gi.EA_Use								= SV_EA_Use;
	gi.EA_Respawn							= SV_EA_Respawn;
	gi.EA_Crouch							= SV_EA_Crouch;
	gi.EA_MoveUp							= SV_EA_MoveUp;
	gi.EA_MoveDown							= SV_EA_MoveDown;
	gi.EA_MoveForward						= SV_EA_MoveForward;
	gi.EA_MoveBack							= SV_EA_MoveBack;
	gi.EA_MoveLeft							= SV_EA_MoveLeft;
	gi.EA_MoveRight							= SV_EA_MoveRight;
	gi.EA_SelectWeapon						= SV_EA_SelectWeapon;
	gi.EA_Jump								= SV_EA_Jump;
	gi.EA_DelayedJump						= SV_EA_DelayedJump;
	gi.EA_Move								= SV_EA_Move;
	gi.EA_View								= SV_EA_View;
	gi.EA_EndRegular						= SV_EA_EndRegular;
	gi.EA_GetInput							= SV_EA_GetInput;
	gi.EA_ResetInput						= SV_EA_ResetInput;
	gi.PC_LoadSource						= SV_PC_LoadSource;
	gi.PC_FreeSource						= SV_PC_FreeSource;
	gi.PC_ReadToken							= SV_PC_ReadToken;
	gi.PC_SourceFileAndLine					= SV_PC_SourceFileAndLine;
	gi.R_RegisterSkin						= SV_RE_RegisterSkin;
	gi.SetActiveSubBSP						= SV_SetActiveSubBSP;
	gi.CM_RegisterTerrain					= SV_CM_RegisterTerrain;
	gi.RMG_Init								= SV_RMG_Init;
	gi.G2_ListModelBones					= SV_G2API_ListModelBones;
	gi.G2_ListModelSurfaces					= SV_G2API_ListModelSurfaces;
	gi.G2_HaveWeGhoul2Models				= SV_G2API_HaveWeGhoul2Models;
	gi.G2_SetGhoul2ModelIndexes				= SV_G2API_SetGhoul2ModelIndexes;
	gi.G2API_GetBoltMatrix					= SV_G2API_GetBoltMatrix;
	gi.G2API_GetBoltMatrix_NoReconstruct	= SV_G2API_GetBoltMatrix_NoReconstruct;
	gi.G2API_GetBoltMatrix_NoRecNoRot		= SV_G2API_GetBoltMatrix_NoRecNoRot;
	gi.G2API_InitGhoul2Model				= SV_G2API_InitGhoul2Model;
	gi.G2API_SetSkin						= SV_G2API_SetSkin;
	gi.G2API_Ghoul2Size						= SV_G2API_Ghoul2Size;
	gi.G2API_AddBolt						= SV_G2API_AddBolt;
	gi.G2API_SetBoltInfo					= SV_G2API_SetBoltInfo;
	gi.G2API_SetBoneAngles					= SV_G2API_SetBoneAngles;
	gi.G2API_SetBoneAnim					= SV_G2API_SetBoneAnim;
	gi.G2API_GetBoneAnim					= SV_G2API_GetBoneAnim;
	gi.G2API_GetGLAName						= SV_G2API_GetGLAName;
	gi.G2API_CopyGhoul2Instance				= SV_G2API_CopyGhoul2Instance;
	gi.G2API_CopySpecificGhoul2Model		= SV_G2API_CopySpecificGhoul2Model;
	gi.G2API_DuplicateGhoul2Instance		= SV_G2API_DuplicateGhoul2Instance;
	gi.G2API_HasGhoul2ModelOnIndex			= SV_G2API_HasGhoul2ModelOnIndex;
	gi.G2API_RemoveGhoul2Model				= SV_G2API_RemoveGhoul2Model;
	gi.G2API_RemoveGhoul2Models				= SV_G2API_RemoveGhoul2Models;
	gi.G2API_CleanGhoul2Models				= SV_G2API_CleanGhoul2Models;
	gi.G2API_CollisionDetect				= SV_G2API_CollisionDetect;
	gi.G2API_CollisionDetectCache			= SV_G2API_CollisionDetectCache;
	gi.G2API_SetRootSurface					= SV_G2API_SetRootSurface;
	gi.G2API_SetSurfaceOnOff				= SV_G2API_SetSurfaceOnOff;
	gi.G2API_SetNewOrigin					= SV_G2API_SetNewOrigin;
	gi.G2API_DoesBoneExist					= SV_G2API_DoesBoneExist;
	gi.G2API_GetSurfaceRenderStatus			= SV_G2API_GetSurfaceRenderStatus;
	gi.G2API_AbsurdSmoothing				= SV_G2API_AbsurdSmoothing;
	gi.G2API_SetRagDoll						= SV_G2API_SetRagDoll;
	gi.G2API_AnimateG2Models				= SV_G2API_AnimateG2Models;
	gi.G2API_RagPCJConstraint				= SV_G2API_RagPCJConstraint;
	gi.G2API_RagPCJGradientSpeed			= SV_G2API_RagPCJGradientSpeed;
	gi.G2API_RagEffectorGoal				= SV_G2API_RagEffectorGoal;
	gi.G2API_GetRagBonePos					= SV_G2API_GetRagBonePos;
	gi.G2API_RagEffectorKick				= SV_G2API_RagEffectorKick;
	gi.G2API_RagForceSolve					= SV_G2API_RagForceSolve;
	gi.G2API_SetBoneIKState					= SV_G2API_SetBoneIKState;
	gi.G2API_IKMove							= SV_G2API_IKMove;
	gi.G2API_RemoveBone						= SV_G2API_RemoveBone;
	gi.G2API_AttachInstanceToEntNum			= SV_G2API_AttachInstanceToEntNum;
	gi.G2API_ClearAttachedInstance			= SV_G2API_ClearAttachedInstance;
	gi.G2API_CleanEntAttachments			= SV_G2API_CleanEntAttachments;
	gi.G2API_OverrideServer					= SV_G2API_OverrideServer;
	gi.G2API_GetSurfaceName					= SV_G2API_GetSurfaceName;

	GetGameAPI = (GetGameAPI_t)gvm->GetModuleAPI;
	ret = GetGameAPI( GAME_API_VERSION, &gi );
	if ( !ret ) {
		//free VM?
		svs.gameStarted = qfalse;
		Com_Error( ERR_FATAL, "GetGameAPI failed on %s", dllName );
	}
	ge = ret;
}

void SV_UnbindGame( void ) {
	GVM_ShutdownGame( qfalse );
	VM_Free( gvm );
	gvm = NULL;
}

void SV_RestartGame( void ) {
	GVM_ShutdownGame( qtrue );

	gvm = VM_Restart( gvm );
	SV_BindGame();
	if ( !gvm ) {
		svs.gameStarted = qfalse;
		Com_Error( ERR_DROP, "VM_Restart on game failed" );
		return;
	}

	SV_InitGame( qtrue );
}
