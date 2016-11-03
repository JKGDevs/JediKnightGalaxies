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

// cl_cgameapi.cpp  -- client system interaction with client game
#include "qcommon/cm_public.h"
#include "qcommon/RoffSystem.h"
#include "qcommon/stringed_ingame.h"
#include "qcommon/timing.h"
#include "client.h"
#include "cl_uiapi.h"
#include "botlib/botlib.h"
#include "snd_ambient.h"
#include "FXExport.h"
#include "FxUtil.h"

extern IHeapAllocator *G2VertSpaceClient;
extern botlib_export_t *botlib_export;

// cgame interface
static cgameExport_t *cge; // cgame export table
static vm_t *cgvm; // cgame vm, valid for legacy and new api

//
// cgame vmMain calls
//
void CGVM_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	VMSwap v( cgvm );

	cge->Init( serverMessageNum, serverCommandSequence, clientNum );
}

void CGVM_Shutdown( void ) {
	VMSwap v( cgvm );

	cge->Shutdown();
}

qboolean CGVM_ConsoleCommand( void ) {
	VMSwap v( cgvm );

	return cge->ConsoleCommand();
}

void CGVM_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	VMSwap v( cgvm );

	cge->DrawActiveFrame( serverTime, stereoView, demoPlayback );
}

int CGVM_CrosshairPlayer( void ) {
	VMSwap v( cgvm );

	return cge->CrosshairPlayer();
}

int CGVM_LastAttacker( void ) {
	VMSwap v( cgvm );

	return cge->LastAttacker();
}

void CGVM_KeyEvent( int key, qboolean down ) {
	VMSwap v( cgvm );

	cge->KeyEvent( key, down );
}

void CGVM_MouseEvent( int x, int y ) {
	VMSwap v( cgvm );

	cge->MouseEvent( x, y );
}

void CGVM_EventHandling( int type ) {
	VMSwap v( cgvm );

	cge->EventHandling( type );
}

int CGVM_PointContents( void ) {
	VMSwap v( cgvm );

	return cge->PointContents();
}

void CGVM_GetLerpOrigin( void ) {
	VMSwap v( cgvm );

	cge->GetLerpOrigin();
}

void CGVM_GetLerpData( void ) {
	VMSwap v( cgvm );

	cge->GetLerpData();
}

void CGVM_Trace( void ) {
	VMSwap v( cgvm );

	cge->Trace();
}

void CGVM_G2Trace( void ) {
	VMSwap v( cgvm );

	cge->G2Trace();
}

void CGVM_G2Mark( void ) {
	VMSwap v( cgvm );

	cge->G2Mark();
}

int CGVM_RagCallback( int callType ) {
	VMSwap v( cgvm );

	return cge->RagCallback( callType );
}

qboolean CGVM_NoUseableForce( void ) {
	VMSwap v( cgvm );

	return cge->NoUseableForce();
}

void CGVM_GetOrigin( int entID, vec3_t out ) {
	VMSwap v( cgvm );

	cge->GetOrigin( entID, out );
}

void CGVM_GetAngles( int entID, vec3_t out ) {
	VMSwap v( cgvm );

	cge->GetAngles( entID, out );
}

trajectory_t *CGVM_GetOriginTrajectory( int entID ) {
	VMSwap v( cgvm );

	return cge->GetOriginTrajectory( entID );
}

trajectory_t *CGVM_GetAngleTrajectory( int entID ) {
	VMSwap v( cgvm );

	return cge->GetAngleTrajectory( entID );
}

void CGVM_ROFF_NotetrackCallback( int entID, const char *notetrack ) {
	VMSwap v( cgvm );

	cge->ROFF_NotetrackCallback( entID, notetrack );
}

void CGVM_MapChange( void ) {
	VMSwap v( cgvm );

	cge->MapChange();
}

void CGVM_AutomapInput( void ) {
	VMSwap v( cgvm );

	cge->AutomapInput();
}

void CGVM_MiscEnt( void ) {
	VMSwap v( cgvm );

	cge->MiscEnt();
}

void CGVM_CameraShake( void ) {
	VMSwap v( cgvm );

	cge->CameraShake();
}

void CGVM_MessageMode( int mode ) {
	VMSwap v( cgvm );

	cge->MessageMode( mode );
}


//
// cgame syscalls
//	only used by legacy mods!
//

extern int CL_GetValueForHidden( const char *s ); //cl_parse.cpp
extern qboolean cl_bUseFighterPitch; //cl_input.cpp
int CM_LoadSubBSP( const char *name, qboolean clientload ); //cm_load.cpp
void FX_FeedTrail( effectTrailArgStruct_t *a ); //FxPrimitives.cpp

// wrappers and such

static void CL_AddCgameCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

static void CL_CM_LoadMap( const char *mapname, qboolean subBSP ) {
	if ( subBSP )	CM_LoadSubBSP( va( "maps/%s.bsp", mapname+1 ), qfalse );
	else			CM_LoadMap( mapname, qtrue, NULL );
}

static void CL_GetGlconfig( glconfig_t *glconfig ) {
	*glconfig = cls.glconfig;
}

static void CL_GetGameState( gameState_t *gs ) {
	*gs = cl.gameState;
}

static void RegisterSharedMemory( char *memory ) {
	cl.mSharedMemory = memory;
}

static int CL_Milliseconds( void ) {
	return Sys_Milliseconds();
}

static void CL_AddReliableCommand2( const char *cmd ) {
	CL_AddReliableCommand( cmd, qfalse );
}

static int CL_CM_RegisterTerrain( const char *config ) {
	return 0;
}

extern int s_entityWavVol[MAX_GENTITIES];
static int CL_S_GetVoiceVolume( int entID ) {
	return s_entityWavVol[entID];
}

static void CL_S_Shutup( qboolean shutup ) {
	s_shutUp = shutup;
}

static int CL_GetCurrentCmdNumber( void ) {
	return cl.cmdNumber;
}

static void _CL_SetUserCmdValue( int stateValue, float sensitivityScale, float mPitchOverride, float mYawOverride, float mSensitivityOverride, int fpSel, int invenSel, qboolean fighterControls ) {
	cl_bUseFighterPitch = fighterControls;
	CL_SetUserCmdValue( stateValue, sensitivityScale, mPitchOverride, mYawOverride, mSensitivityOverride, fpSel, invenSel );
}

static void CL_OpenUIMenu( int menuID ) {
	UIVM_SetActiveMenu( (uiMenuCommand_t)menuID );
}

static void CGFX_AddLine( const vec3_t start, const vec3_t end, float size1, float size2, float sizeParm, float alpha1, float alpha2, float alphaParm, const vec3_t sRGB, const vec3_t eRGB, float rgbParm, int killTime, qhandle_t shader, int flags ) {
	FX_AddLine( const_cast<float *>(start), const_cast<float *>(end), size1, size2, sizeParm, alpha1, alpha2, alphaParm, const_cast<float *>(sRGB), const_cast<float *>(eRGB), rgbParm, killTime, shader, flags );
}

static void CGFX_AddPoly( addpolyArgStruct_t *p ) {
	FX_AddPoly( p->p, p->ev, p->numVerts, p->vel, p->accel, p->alpha1, p->alpha2, p->alphaParm, p->rgb1, p->rgb2, p->rgbParm, p->rotationDelta, p->bounce, p->motionDelay, p->killTime, p->shader, p->flags );
}

static void CGFX_AddBezier( addbezierArgStruct_t *b ) {
	FX_AddBezier( b->start, b->end, b->control1, b->control1Vel, b->control2, b->control2Vel, b->size1, b->size2, b->sizeParm, b->alpha1, b->alpha2, b->alphaParm, b->sRGB, b->eRGB, b->rgbParm, b->killTime, b->shader, b->flags );
}

static void CGFX_AddPrimitive( effectTrailArgStruct_t *e ) {
	FX_FeedTrail( e );
}

static void CGFX_AddSprite( addspriteArgStruct_t *s ) {
	vec3_t rgb = { 1.0f, 1.0f, 1.0f };
	FX_AddParticle( s->origin, s->vel, s->accel, s->scale, s->dscale, 0, s->sAlpha, s->eAlpha, 0, rgb, rgb, 0, s->rotation, 0, vec3_origin, vec3_origin, s->bounce, 0, 0, s->life, s->shader, s->flags );
}

static void CGFX_AddElectricity( addElectricityArgStruct_t *p ) {
	FX_AddElectricity( p->start, p->end, p->size1, p->size2, p->sizeParm, p->alpha1, p->alpha2, p->alphaParm, p->sRGB, p->eRGB, p->rgbParm, p->chaos, p->killTime, p->shader, p->flags );
}

static char *CGFX_GetSharedMemory()
{
	return cl.mSharedMemory;
}

static qboolean CL_ROFF_Clean( void ) {
	return theROFFSystem.Clean( qtrue );
}

static void CL_ROFF_UpdateEntities( void ) {
	theROFFSystem.UpdateEntities( qtrue );
}

static int CL_ROFF_Cache( char *file ) {
	return theROFFSystem.Cache( file, qtrue );
}

static qboolean CL_ROFF_Play( int entID, int roffID, qboolean doTranslation ) {
	return theROFFSystem.Play( entID, roffID, doTranslation, qtrue );
}

static qboolean CL_ROFF_Purge_Ent( int entID ) {
	return theROFFSystem.PurgeEnt( entID, qtrue );
}

static void CL_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime ) {
	*snapshotNumber = cl.snap.messageNum;
	*serverTime = cl.snap.serverTime;
}

static void CL_SetClientForceAngle( int time, vec3_t angle ) {
	cl.cgameViewAngleForceTime = time;
	VectorCopy(angle, cl.cgameViewAngleForce);
}

static void CL_PrecisionTimerStart( void **p ) {
	timing_c *newTimer = new timing_c; //create the new timer
	*p = newTimer; //assign the pointer within the pointer to point at the mem addr of our new timer
	newTimer->Start(); //start the timer
}

static int CL_PrecisionTimerEnd( void *p ) {
	int r = 0;
	timing_c *timer = (timing_c *)p; //this is the pointer we assigned in start, so we can directly cast it back
	r = timer->End(); //get the result
	delete timer; //delete the timer since we're done with it
	return r; //return the result
}

static void CL_RMG_Init( int /* terrainID */, const char * /* terrainInfo */ ) { }

static qboolean CGFX_PlayBoltedEffectID( int id, vec3_t org, void *ghoul2, const int boltNum, const int entNum, const int modelNum, int iLooptime, qboolean isRelative ) {
	if ( !ghoul2 ) return qfalse;

	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	int boltInfo=0;
	if ( re->G2API_AttachEnt( &boltInfo, g2, modelNum, boltNum, entNum, modelNum ) )
	{
		FX_PlayBoltedEffectID(id, org, boltInfo, &g2, iLooptime, isRelative );
		return qtrue;
	}
	return qfalse;
}

static qboolean CL_SE_GetStringTextString( const char *text, char *buffer, int bufferLength ) {
	const char *str;

	assert( text && buffer );

	str = SE_GetString( text );

	if ( str[0] ) {
		Q_strncpyz( buffer, str, bufferLength );
		return qtrue;
	}

	Com_sprintf( buffer, bufferLength, "??%s", str );
	return qfalse;
}

static void CL_G2API_ListModelSurfaces( void *ghlInfo ) {
	re->G2API_ListSurfaces( (CGhoul2Info *)ghlInfo );
}

static void CL_G2API_ListModelBones( void *ghlInfo, int frame ) {
	re->G2API_ListBones( (CGhoul2Info *)ghlInfo, frame );
}

static void CL_G2API_SetGhoul2ModelIndexes( void *ghoul2, qhandle_t *modelList, qhandle_t *skinList ) {
	if ( !ghoul2 ) return;
	re->G2API_SetGhoul2ModelIndexes( *((CGhoul2Info_v *)ghoul2), modelList, skinList );
}

static qboolean CL_G2API_HaveWeGhoul2Models( void *ghoul2) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_HaveWeGhoul2Models( *((CGhoul2Info_v *)ghoul2) );
}

static qboolean CL_G2API_GetBoltMatrix( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean CL_G2API_GetBoltMatrix_NoReconstruct( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixReconstruction( qfalse );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static qboolean CL_G2API_GetBoltMatrix_NoRecNoRot( void *ghoul2, const int modelIndex, const int boltIndex, mdxaBone_t *matrix, const vec3_t angles, const vec3_t position, const int frameNum, qhandle_t *modelList, vec3_t scale ) {
	if ( !ghoul2 ) return qfalse;
	re->G2API_BoltMatrixSPMethod( qtrue );
	return re->G2API_GetBoltMatrix( *((CGhoul2Info_v *)ghoul2), modelIndex, boltIndex, matrix, angles, position, frameNum, modelList, scale );
}

static int CL_G2API_InitGhoul2Model( void **ghoul2Ptr, const char *fileName, int modelIndex, qhandle_t customSkin, qhandle_t customShader, int modelFlags, int lodBias ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	return re->G2API_InitGhoul2Model( (CGhoul2Info_v **)ghoul2Ptr, fileName, modelIndex, customSkin, customShader, modelFlags, lodBias );
}

static qboolean CL_G2API_SetSkin( void *ghoul2, int modelIndex, qhandle_t customSkin, qhandle_t renderSkin ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_SetSkin( g2, modelIndex, customSkin, renderSkin );
}

static void CL_G2API_CollisionDetect( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, const vec3_t rayStart, const vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetect( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, const_cast<float *>(rayStart), const_cast<float *>(rayEnd), scale, G2VertSpaceClient, traceFlags, useLod, fRadius );
}

static void CL_G2API_CollisionDetectCache( CollisionRecord_t *collRecMap, void* ghoul2, const vec3_t angles, const vec3_t position, int frameNumber, int entNum, const vec3_t rayStart, const vec3_t rayEnd, vec3_t scale, int traceFlags, int useLod, float fRadius ) {
	if ( !ghoul2 ) return;
	re->G2API_CollisionDetectCache( collRecMap, *((CGhoul2Info_v *)ghoul2), angles, position, frameNumber, entNum, const_cast<float *>(rayStart), const_cast<float *>(rayEnd), scale, G2VertSpaceClient, traceFlags, useLod, fRadius );
}

static void CL_G2API_CleanGhoul2Models( void **ghoul2Ptr ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	re->G2API_CleanGhoul2Models( (CGhoul2Info_v **)ghoul2Ptr );
}

static qboolean CL_G2API_SetBoneAngles( void *ghoul2, int modelIndex, const char *boneName, const vec3_t angles, const int flags, const int up, const int right, const int forward, qhandle_t *modelList, int blendTime , int currentTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAngles( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, angles, flags, (const Eorientations)up, (const Eorientations)right, (const Eorientations)forward, modelList, blendTime , currentTime );
}

static qboolean CL_G2API_SetBoneAnim( void *ghoul2, const int modelIndex, const char *boneName, const int startFrame, const int endFrame, const int flags, const float animSpeed, const int currentTime, const float setFrame, const int blendTime ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneAnim( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName, startFrame, endFrame, flags, animSpeed, currentTime, setFrame, blendTime );
}

static qboolean CL_G2API_GetBoneAnim( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *startFrame, int *endFrame, int *flags, float *animSpeed, int *modelList, const int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetBoneAnim( g2, modelIndex, boneName, currentTime, currentFrame, startFrame, endFrame, flags, animSpeed, modelList );
}

static qboolean CL_G2API_GetBoneFrame( void *ghoul2, const char *boneName, const int currentTime, float *currentFrame, int *modelList, const int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	int iDontCare1 = 0, iDontCare2 = 0, iDontCare3 = 0;
	float fDontCare1 = 0;

	return re->G2API_GetBoneAnim(g2, modelIndex, boneName, currentTime, currentFrame, &iDontCare1, &iDontCare2, &iDontCare3, &fDontCare1, modelList);
}

static void CL_G2API_GetGLAName( void *ghoul2, int modelIndex, char *fillBuf ) {
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

static int CL_G2API_CopyGhoul2Instance( void *g2From, void *g2To, int modelIndex ) {
	if ( !g2From || !g2To ) return 0;

	return re->G2API_CopyGhoul2Instance( *((CGhoul2Info_v *)g2From), *((CGhoul2Info_v *)g2To), modelIndex );
}

static void CL_G2API_CopySpecificGhoul2Model( void *g2From, int modelFrom, void *g2To, int modelTo ) {
	if ( !g2From || !g2To) return;
	re->G2API_CopySpecificG2Model( *((CGhoul2Info_v *)g2From), modelFrom, *((CGhoul2Info_v *)g2To), modelTo );
}

static void CL_G2API_DuplicateGhoul2Instance( void *g2From, void **g2To ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	if ( !g2From || !g2To ) return;
	re->G2API_DuplicateGhoul2Instance( *((CGhoul2Info_v *)g2From), (CGhoul2Info_v **)g2To );
}

static qboolean CL_G2API_HasGhoul2ModelOnIndex( void *ghlInfo, int modelIndex ) {
	return re->G2API_HasGhoul2ModelOnIndex( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean CL_G2API_RemoveGhoul2Model( void *ghlInfo, int modelIndex ) {
#ifdef _FULL_G2_LEAK_CHECKING
		g_G2AllocServer = 0;
#endif
	return re->G2API_RemoveGhoul2Model( (CGhoul2Info_v **)ghlInfo, modelIndex );
}

static qboolean CL_G2API_SkinlessModel( void *ghlInfo, int modelIndex ) {
	if ( !ghlInfo ) return qfalse;

	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfo);
	return re->G2API_SkinlessModel( g2, modelIndex );
}

static int CL_G2API_GetNumGoreMarks( void *ghlInfo, int modelIndex ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return 0;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfo);
	return re->G2API_GetNumGoreMarks( g2, modelIndex );
#else
	return 0;
#endif
}

static void CL_G2API_AddSkinGore( void *ghlInfo, SSkinGoreData *gore ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return;
	re->G2API_AddSkinGore( *((CGhoul2Info_v *)ghlInfo), *(SSkinGoreData *)gore );
#endif
}

static void CL_G2API_ClearSkinGore( void *ghlInfo ) {
#ifdef _G2_GORE
	if ( !ghlInfo ) return;
	re->G2API_ClearSkinGore( *((CGhoul2Info_v *)ghlInfo) );
#endif
}

static int CL_G2API_Ghoul2Size( void *ghlInfo ) {
	if ( !ghlInfo ) return 0;
	return re->G2API_Ghoul2Size( *((CGhoul2Info_v *)ghlInfo) );
}

static int CL_G2API_AddBolt( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return 0;
	return re->G2API_AddBolt( *((CGhoul2Info_v *)ghoul2), modelIndex, boneName );
}

static qboolean CL_G2API_AttachEnt( int *boltInfo, void *ghlInfoTo, int toBoltIndex, int entNum, int toModelNum ) {
	if ( !ghlInfoTo ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghlInfoTo);
	return re->G2API_AttachEnt( boltInfo, g2, 0, toBoltIndex, entNum, toModelNum );
}

static void CL_G2API_SetBoltInfo( void *ghoul2, int modelIndex, int boltInfo ) {
	if ( !ghoul2 ) return;
	re->G2API_SetBoltInfo( *((CGhoul2Info_v *)ghoul2), modelIndex, boltInfo );
}

static qboolean CL_G2API_SetRootSurface( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetRootSurface( *((CGhoul2Info_v *)ghoul2), modelIndex, surfaceName );
}

static qboolean CL_G2API_SetSurfaceOnOff( void *ghoul2, const char *surfaceName, const int flags ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetSurfaceOnOff( *((CGhoul2Info_v *)ghoul2), surfaceName, flags );
}

static qboolean CL_G2API_SetNewOrigin( void *ghoul2, const int boltIndex ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetNewOrigin( *((CGhoul2Info_v *)ghoul2), boltIndex );
}

static qboolean CL_G2API_DoesBoneExist( void *ghoul2, int modelIndex, const char *boneName ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_DoesBoneExist( g2, modelIndex, boneName );
}

static int CL_G2API_GetSurfaceRenderStatus( void *ghoul2, const int modelIndex, const char *surfaceName ) {
	if ( !ghoul2 ) return -1;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_GetSurfaceRenderStatus( g2, modelIndex, surfaceName );
}

static int CL_G2API_GetTime( void ) {
	return re->G2API_GetTime( 0 );
}

static void CL_G2API_SetTime( int time, int clock ) {
	re->G2API_SetTime( time, clock );
}

static void CL_G2API_AbsurdSmoothing( void *ghoul2, qboolean status ) {
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	re->G2API_AbsurdSmoothing( g2, status );
}

static void CL_G2API_SetRagDoll( void *ghoul2, sharedRagDollParams_t *params ) {
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

static void CL_G2API_AnimateG2Models( void *ghoul2, int time, sharedRagDollUpdateParams_t *params ) {
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

static qboolean CL_G2API_RagPCJConstraint( void *ghoul2, const char *boneName, vec3_t min, vec3_t max ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJConstraint( *((CGhoul2Info_v *)ghoul2), boneName, min, max );
}

static qboolean CL_G2API_RagPCJGradientSpeed( void *ghoul2, const char *boneName, const float speed ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagPCJGradientSpeed( *((CGhoul2Info_v *)ghoul2), boneName, speed );
}

static qboolean CL_G2API_RagEffectorGoal( void *ghoul2, const char *boneName, vec3_t pos ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorGoal( *((CGhoul2Info_v *)ghoul2), boneName, pos );
}

static qboolean CL_G2API_GetRagBonePos( void *ghoul2, const char *boneName, vec3_t pos, vec3_t entAngles, vec3_t entPos, vec3_t entScale ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_GetRagBonePos( *((CGhoul2Info_v *)ghoul2), boneName, pos, entAngles, entPos, entScale );
}

static qboolean CL_G2API_RagEffectorKick( void *ghoul2, const char *boneName, vec3_t velocity ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagEffectorKick( *((CGhoul2Info_v *)ghoul2), boneName, velocity );
}

static qboolean CL_G2API_RagForceSolve( void *ghoul2, qboolean force ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_RagForceSolve( *((CGhoul2Info_v *)ghoul2), force );
}

static qboolean CL_G2API_SetBoneIKState( void *ghoul2, int time, const char *boneName, int ikState, sharedSetBoneIKStateParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_SetBoneIKState( *((CGhoul2Info_v *)ghoul2), time, boneName, ikState, params );
}

static qboolean CL_G2API_IKMove( void *ghoul2, int time, sharedIKMoveParams_t *params ) {
	if ( !ghoul2 ) return qfalse;
	return re->G2API_IKMove( *((CGhoul2Info_v *)ghoul2), time, params );
}

static qboolean CL_G2API_RemoveBone( void *ghoul2, const char *boneName, int modelIndex ) {
	if ( !ghoul2 ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	return re->G2API_RemoveBone( g2, modelIndex, boneName );
}

static void CL_G2API_AttachInstanceToEntNum( void *ghoul2, int entityNum, qboolean server ) {
	if ( !ghoul2 ) return;
	re->G2API_AttachInstanceToEntNum( *((CGhoul2Info_v *)ghoul2), entityNum, server );
}

static void CL_G2API_ClearAttachedInstance( int entityNum ) {
	re->G2API_ClearAttachedInstance( entityNum );
}

static void CL_G2API_CleanEntAttachments( void ) {
	re->G2API_CleanEntAttachments();
}

static qboolean CL_G2API_OverrideServer( void *serverInstance ) {
	if ( !serverInstance ) return qfalse;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)serverInstance);
	return re->G2API_OverrideServerWithClientData( g2, 0 );
}

static void CL_G2API_GetSurfaceName( void *ghoul2, int surfNumber, int modelIndex, char *fillBuf ) {
	//NOTE CHECKME should this store empty fillBuf like g2api getglaname does same thing if tmp fails?
	if ( !ghoul2 ) return;
	CGhoul2Info_v &g2 = *((CGhoul2Info_v *)ghoul2);
	char *tmp = re->G2API_GetSurfaceName( g2, modelIndex, surfNumber );
	strcpy( fillBuf, tmp );
}

static void CL_Key_SetCatcher( int catcher ) {
	// Don't allow the cgame module to close the console
	Key_SetCatcher( catcher | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
}

static void CGVM_Cvar_Set( const char *var_name, const char *value ) {
	Cvar_VM_Set( var_name, value, VM_CGAME );
}

static void CGVM_Cmd_RemoveCommand( const char *cmd_name ) {
	Cmd_VM_RemoveCommand( cmd_name, VM_CGAME );
}

// Stub function for old RMG system.
static void RE_InitRendererTerrain ( const char * /*info*/ ) {}

static void CO_Shutdown() {}

static void CL_GetViewAngles( vec3_t viewangles )
{
	VectorCopy( cl.viewangles, viewangles );
}

static void CL_SetViewAngles( const vec3_t viewangles )
{
	VectorCopy( viewangles, cl.viewangles );
}

static void CL_SetColorTableAlpha( float alpha )
{
	for ( int i = 0; i < 10; i++ )
	{
		g_color_table[i][3] = alpha;
	}
}

void CL_BindCGame( void ) {
	static cgameImport_t cgi;
	cgameExport_t		*ret;
	GetCGameAPI_t		GetCGameAPI;
	char				dllName[MAX_OSPATH] = "cgame" ARCH_STRING DLL_EXT;

	memset( &cgi, 0, sizeof( cgi ) );

	cgvm = VM_Create( VM_CGAME );
	if ( !cgvm ) {
		cls.cgameStarted = qfalse;
		Com_Error( ERR_DROP, "VM_Create on cgame failed" );
	}

	cgi.Print								= Com_Printf;
	cgi.Error								= Com_Error;
	cgi.SnapVector							= Sys_SnapVector;
	cgi.MemoryRemaining						= Hunk_MemoryRemaining;
	cgi.RegisterSharedMemory				= RegisterSharedMemory;
	cgi.TrueMalloc							= VM_Shifted_Alloc;
	cgi.TrueFree							= VM_Shifted_Free;
	cgi.Milliseconds						= CL_Milliseconds;
	cgi.RealTime							= Com_RealTime;
	cgi.PrecisionTimerStart					= CL_PrecisionTimerStart;
	cgi.PrecisionTimerEnd					= CL_PrecisionTimerEnd;
	cgi.Cvar_Register						= Cvar_Register;
	cgi.Cvar_Set							= CGVM_Cvar_Set;
	cgi.Cvar_Update							= Cvar_Update;
	cgi.Cvar_VariableStringBuffer			= Cvar_VariableStringBuffer;
	cgi.AddCommand							= CL_AddCgameCommand;
	cgi.Cmd_Argc							= Cmd_Argc;
	cgi.Cmd_Args							= Cmd_ArgsBuffer;
	cgi.Cmd_Argv							= Cmd_ArgvBuffer;
	cgi.RemoveCommand						= CGVM_Cmd_RemoveCommand;
	cgi.SendClientCommand					= CL_AddReliableCommand2;
	cgi.SendConsoleCommand					= Cbuf_AddText;
	cgi.FS_Close							= FS_FCloseFile;
	cgi.FS_GetFileList						= FS_GetFileList;
	cgi.FS_Open								= FS_FOpenFileByMode;
	cgi.FS_Read								= FS_Read;
	cgi.FS_Write							= FS_Write;
	cgi.UpdateScreen						= SCR_UpdateScreen;
	cgi.CM_InlineModel						= CM_InlineModel;
	cgi.CM_LoadMap							= CL_CM_LoadMap;
	cgi.CM_NumInlineModels					= CM_NumInlineModels;
	cgi.CM_PointContents					= CM_PointContents;
	cgi.CM_RegisterTerrain					= CL_CM_RegisterTerrain;
	cgi.CM_TempModel						= CM_TempBoxModel;
	cgi.CM_Trace							= CM_BoxTrace;
	cgi.CM_TransformedPointContents			= CM_TransformedPointContents;
	cgi.CM_TransformedTrace					= CM_TransformedBoxTrace;
	cgi.RMG_Init							= CL_RMG_Init;
	cgi.S_AddLocalSet						= S_AddLocalSet;
	cgi.S_AddLoopingSound					= S_AddLoopingSound;
	cgi.S_ClearLoopingSounds				= S_ClearLoopingSounds;
	cgi.S_GetVoiceVolume					= CL_S_GetVoiceVolume;
	cgi.S_MuteSound							= S_MuteSound;
	cgi.S_RegisterSound						= S_RegisterSound;
	cgi.S_Respatialize						= S_Respatialize;
	cgi.S_Shutup							= CL_S_Shutup;
	cgi.S_StartBackgroundTrack				= S_StartBackgroundTrack;
	cgi.S_StartLocalSound					= S_StartLocalSound;
	cgi.S_StartSound						= S_StartSound;
	cgi.S_StopBackgroundTrack				= S_StopBackgroundTrack;
	cgi.S_StopLoopingSound					= S_StopLoopingSound;
	cgi.S_UpdateEntityPosition				= S_UpdateEntityPosition;
	cgi.S_UpdateAmbientSet					= S_UpdateAmbientSet;
	cgi.AS_AddPrecacheEntry					= AS_AddPrecacheEntry;
	cgi.AS_GetBModelSound					= AS_GetBModelSound;
	cgi.AS_ParseSets						= AS_ParseSets;
	cgi.R_AddAdditiveLightToScene			= re->AddAdditiveLightToScene;
	cgi.R_AddDecalToScene					= re->AddDecalToScene;
	cgi.R_AddLightToScene					= re->AddLightToScene;
	cgi.R_AddPolysToScene					= re->AddPolyToScene;
	cgi.R_AddRefEntityToScene				= re->AddRefEntityToScene;
	cgi.R_AddMiniRefEntityToScene			= re->AddMiniRefEntityToScene;
	cgi.R_AnyLanguage_ReadCharFromString	= re->AnyLanguage_ReadCharFromString;
	cgi.R_AutomapElevationAdjustment		= re->AutomapElevationAdjustment;
	cgi.R_ClearDecals						= re->ClearDecals;
	cgi.R_ClearScene						= re->ClearScene;
	cgi.R_DrawStretchPic					= re->DrawStretchPic;
	cgi.R_DrawRotatePic						= re->DrawRotatePic;
	cgi.R_DrawRotatePic2					= re->DrawRotatePic2;
	cgi.R_Font_DrawString					= re->Font_DrawString;
	cgi.R_Font_HeightPixels					= re->Font_HeightPixels;
	cgi.R_Font_StrLenChars					= re->Font_StrLenChars;
	cgi.R_Font_StrLenPixels					= re->Font_StrLenPixels;
	cgi.R_GetBModelVerts					= re->GetBModelVerts;
	cgi.R_GetDistanceCull					= re->GetDistanceCull;
	cgi.R_GetEntityToken					= re->GetEntityToken;
	cgi.R_GetLightStyle						= re->GetLightStyle;
	cgi.R_GetRealRes						= re->GetRealRes;
	cgi.R_InitializeWireframeAutomap		= re->InitializeWireframeAutomap;
	cgi.R_InPVS								= re->inPVS;
	cgi.R_Language_IsAsian					= re->Language_IsAsian;
	cgi.R_Language_UsesSpaces				= re->Language_UsesSpaces;
	cgi.R_LerpTag							= re->LerpTag;
	cgi.R_LightForPoint						= re->LightForPoint;
	cgi.R_LoadWorld							= re->LoadWorld;
	cgi.R_MarkFragments						= re->MarkFragments;
	cgi.R_ModelBounds						= re->ModelBounds;
	cgi.R_RegisterFont						= re->RegisterFont;
	cgi.R_RegisterModel						= re->RegisterModel;
	cgi.R_RegisterShader					= re->RegisterShader;
	cgi.R_RegisterShaderNoMip				= re->RegisterShaderNoMip;
	cgi.R_RegisterSkin						= re->RegisterSkin;
	cgi.R_RemapShader						= re->RemapShader;
	cgi.R_RenderScene						= re->RenderScene;
	cgi.R_SetColor							= re->SetColor;
	cgi.R_SetLightStyle						= re->SetLightStyle;
	cgi.R_SetRangedFog						= re->SetRangedFog;
	cgi.R_SetRefractionProperties			= re->SetRefractionProperties;
	cgi.R_WorldEffectCommand				= re->WorldEffectCommand;
	cgi.RE_InitRendererTerrain				= RE_InitRendererTerrain;
	cgi.WE_AddWeatherZone					= re->AddWeatherZone;
	cgi.GetCurrentSnapshotNumber			= CL_GetCurrentSnapshotNumber;
	cgi.GetCurrentCmdNumber					= CL_GetCurrentCmdNumber;
	cgi.GetDefaultState						= CL_GetDefaultState;
	cgi.GetGameState						= CL_GetGameState;
	cgi.GetGlconfig							= CL_GetGlconfig;
	cgi.GetServerCommand					= CL_GetServerCommand;
	cgi.GetSnapshot							= CL_GetSnapshot;
	cgi.GetUserCmd							= CL_GetUserCmd;
	cgi.OpenUIMenu							= CL_OpenUIMenu;
	cgi.SetClientForceAngle					= CL_SetClientForceAngle;
	cgi.SetUserCmdValue						= _CL_SetUserCmdValue;
	cgi.Key_GetCatcher						= Key_GetCatcher;
	cgi.Key_GetKey							= Key_GetKey;
	cgi.Key_IsDown							= Key_IsDown;
	cgi.Key_SetCatcher						= CL_Key_SetCatcher;
	cgi.PC_AddGlobalDefine					= botlib_export->PC_AddGlobalDefine;
	cgi.PC_FreeSource						= botlib_export->PC_FreeSourceHandle;
	cgi.PC_LoadGlobalDefines				= botlib_export->PC_LoadGlobalDefines;
	cgi.PC_LoadSource						= botlib_export->PC_LoadSourceHandle;
	cgi.PC_ReadToken						= botlib_export->PC_ReadTokenHandle;
	cgi.PC_RemoveAllGlobalDefines			= botlib_export->PC_RemoveAllGlobalDefines;
	cgi.PC_SourceFileAndLine				= botlib_export->PC_SourceFileAndLine;
	cgi.CIN_DrawCinematic					= CIN_DrawCinematic;
	cgi.CIN_PlayCinematic					= CIN_PlayCinematic;
	cgi.CIN_RunCinematic					= CIN_RunCinematic;
	cgi.CIN_SetExtents						= CIN_SetExtents;
	cgi.CIN_StopCinematic					= CIN_StopCinematic;
	cgi.FX_AddLine							= CGFX_AddLine;
	cgi.FX_RegisterEffect					= FX_RegisterEffect;
	cgi.FX_PlayEffect						= FX_PlayEffect;
	cgi.FX_PlayEffectID						= FX_PlayEffectID;
	cgi.FX_PlayEntityEffectID				= FX_PlayEntityEffectID;
	cgi.FX_PlayBoltedEffectID				= CGFX_PlayBoltedEffectID;
	cgi.FX_AddScheduledEffects				= FX_AddScheduledEffects;
	cgi.FX_InitSystem						= FX_InitSystem;
	cgi.FX_SetRefDef						= FX_SetRefDef;
	cgi.FX_FreeSystem						= FX_FreeSystem;
	cgi.FX_AdjustTime						= FX_AdjustTime;
	cgi.FX_Draw2DEffects					= FX_Draw2DEffects;
	cgi.FX_AddPoly							= CGFX_AddPoly;
	cgi.FX_AddBezier						= CGFX_AddBezier;
	cgi.FX_AddPrimitive						= CGFX_AddPrimitive;
	cgi.FX_AddSprite						= CGFX_AddSprite;
	cgi.FX_AddElectricity					= CGFX_AddElectricity;
	cgi.FX_GetSharedMemory					= CGFX_GetSharedMemory;
	cgi.SE_GetStringTextString				= CL_SE_GetStringTextString;
	cgi.ROFF_Clean							= CL_ROFF_Clean;
	cgi.ROFF_UpdateEntities					= CL_ROFF_UpdateEntities;
	cgi.ROFF_Cache							= CL_ROFF_Cache;
	cgi.ROFF_Play							= CL_ROFF_Play;
	cgi.ROFF_Purge_Ent						= CL_ROFF_Purge_Ent;
	cgi.G2_ListModelSurfaces				= CL_G2API_ListModelSurfaces;
	cgi.G2_ListModelBones					= CL_G2API_ListModelBones;
	cgi.G2_SetGhoul2ModelIndexes			= CL_G2API_SetGhoul2ModelIndexes;
	cgi.G2_HaveWeGhoul2Models				= CL_G2API_HaveWeGhoul2Models;
	cgi.G2API_GetBoltMatrix					= CL_G2API_GetBoltMatrix;
	cgi.G2API_GetBoltMatrix_NoReconstruct	= CL_G2API_GetBoltMatrix_NoReconstruct;
	cgi.G2API_GetBoltMatrix_NoRecNoRot		= CL_G2API_GetBoltMatrix_NoRecNoRot;
	cgi.G2API_InitGhoul2Model				= CL_G2API_InitGhoul2Model;
	cgi.G2API_SetSkin						= CL_G2API_SetSkin;
	cgi.G2API_CollisionDetect				= CL_G2API_CollisionDetect;
	cgi.G2API_CollisionDetectCache			= CL_G2API_CollisionDetectCache;
	cgi.G2API_CleanGhoul2Models				= CL_G2API_CleanGhoul2Models;
	cgi.G2API_SetBoneAngles					= CL_G2API_SetBoneAngles;
	cgi.G2API_SetBoneAnim					= CL_G2API_SetBoneAnim;
	cgi.G2API_GetBoneAnim					= CL_G2API_GetBoneAnim;
	cgi.G2API_GetBoneFrame					= CL_G2API_GetBoneFrame;
	cgi.G2API_GetGLAName					= CL_G2API_GetGLAName;
	cgi.G2API_CopyGhoul2Instance			= CL_G2API_CopyGhoul2Instance;
	cgi.G2API_CopySpecificGhoul2Model		= CL_G2API_CopySpecificGhoul2Model;
	cgi.G2API_DuplicateGhoul2Instance		= CL_G2API_DuplicateGhoul2Instance;
	cgi.G2API_HasGhoul2ModelOnIndex			= CL_G2API_HasGhoul2ModelOnIndex;
	cgi.G2API_RemoveGhoul2Model				= CL_G2API_RemoveGhoul2Model;
	cgi.G2API_SkinlessModel					= CL_G2API_SkinlessModel;
	cgi.G2API_GetNumGoreMarks				= CL_G2API_GetNumGoreMarks;
	cgi.G2API_AddSkinGore					= CL_G2API_AddSkinGore;
	cgi.G2API_ClearSkinGore					= CL_G2API_ClearSkinGore;
	cgi.G2API_Ghoul2Size					= CL_G2API_Ghoul2Size;
	cgi.G2API_AddBolt						= CL_G2API_AddBolt;
	cgi.G2API_AttachEnt						= CL_G2API_AttachEnt;
	cgi.G2API_SetBoltInfo					= CL_G2API_SetBoltInfo;
	cgi.G2API_SetRootSurface				= CL_G2API_SetRootSurface;
	cgi.G2API_SetSurfaceOnOff				= CL_G2API_SetSurfaceOnOff;
	cgi.G2API_SetNewOrigin					= CL_G2API_SetNewOrigin;
	cgi.G2API_DoesBoneExist					= CL_G2API_DoesBoneExist;
	cgi.G2API_GetSurfaceRenderStatus		= CL_G2API_GetSurfaceRenderStatus;
	cgi.G2API_GetTime						= CL_G2API_GetTime;
	cgi.G2API_SetTime						= CL_G2API_SetTime;
	cgi.G2API_AbsurdSmoothing				= CL_G2API_AbsurdSmoothing;
	cgi.G2API_SetRagDoll					= CL_G2API_SetRagDoll;
	cgi.G2API_AnimateG2Models				= CL_G2API_AnimateG2Models;
	cgi.G2API_RagPCJConstraint				= CL_G2API_RagPCJConstraint;
	cgi.G2API_RagPCJGradientSpeed			= CL_G2API_RagPCJGradientSpeed;
	cgi.G2API_RagEffectorGoal				= CL_G2API_RagEffectorGoal;
	cgi.G2API_GetRagBonePos					= CL_G2API_GetRagBonePos;
	cgi.G2API_RagEffectorKick				= CL_G2API_RagEffectorKick;
	cgi.G2API_RagForceSolve					= CL_G2API_RagForceSolve;
	cgi.G2API_SetBoneIKState				= CL_G2API_SetBoneIKState;
	cgi.G2API_IKMove						= CL_G2API_IKMove;
	cgi.G2API_RemoveBone					= CL_G2API_RemoveBone;
	cgi.G2API_AttachInstanceToEntNum		= CL_G2API_AttachInstanceToEntNum;
	cgi.G2API_ClearAttachedInstance			= CL_G2API_ClearAttachedInstance;
	cgi.G2API_CleanEntAttachments			= CL_G2API_CleanEntAttachments;
	cgi.G2API_OverrideServer				= CL_G2API_OverrideServer;
	cgi.G2API_GetSurfaceName				= CL_G2API_GetSurfaceName;
	cgi.R_OverrideShaderFrame				= re->OverrideShaderFrame;
	cgi.CO_InitCrossover					= UIVM_InitCrossover;
	cgi.CO_Shutdown							= CO_Shutdown;
	cgi.JKG_GetViewAngles					= CL_GetViewAngles;
	cgi.JKG_SetViewAngles					= CL_SetViewAngles;
	cgi.JKG_SetColorTableAlpha				= CL_SetColorTableAlpha;

	GetCGameAPI = (GetCGameAPI_t)cgvm->GetModuleAPI;
	ret = GetCGameAPI( CGAME_API_VERSION, &cgi );
	if ( !ret ) {
		//free VM?
		cls.cgameStarted = qfalse;
		Com_Error( ERR_FATAL, "GetGameAPI failed on %s", dllName );
	}
	cge = ret;
}

void CL_UnbindCGame( void ) {
	CGVM_Shutdown();
	VM_Free( cgvm );
	cgvm = NULL;
}
