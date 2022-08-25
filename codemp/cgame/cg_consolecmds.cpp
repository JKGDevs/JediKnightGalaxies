/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
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

// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "game/bg_saga.h"

extern menuDef_t *menuScoreboard;

void CG_CameraZoomIn( void )
{
	cg_thirdPersonRange.value -= 5;
	if ( cg_thirdPersonRange.value < 30 ) cg_thirdPersonRange.value = 30;
}

void CG_CameraZoomOut( void )
{
	cg_thirdPersonRange.value += 5;
	if ( cg_thirdPersonRange.value > 100 ) cg_thirdPersonRange.value = 100;
}

void CG_Start360Camera( void )
{
	if ( trap->Key_GetCatcher() == 0 && cg.i360CameraTime == 0 )
	{
		usercmd_t cmd;
		trap->GetUserCmd( trap->GetCurrentCmdNumber(), &cmd );

		cg.i360CameraForce		= -1;
		cg.i360CameraTime		= cg.time + 250;
		cg.i360CameraOffset		= 0;
		cg.i360CameraOriginal	= 0;
		cg.i360CameraUserCmd	= cmd.angles[YAW];
	}
}

void CG_Stop360Camera( void )
{
	if ( cg.i360CameraTime )
	{
		/* This was a short click, so only change the third person camera! */
		if ( cg.i360CameraTime > cg.time )
		{
			trap->SendConsoleCommand( "cg_thirdPerson !" );
		}
		/* It was a full rotate so reset the view angles to their original position */
		else
		{
			vec3_t angle;
			angle[YAW] = cg.i360CameraOriginal;
			trap->SetClientForceAngle( cg.time + 10, angle );
		}
	}

	cg.i360CameraTime = 0;
}

void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	trap->Cmd_Argv( 1, test, 4 );
	trap->SendClientCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}

void CG_OpenPartyManagement_f( void ) {
	uiImports->PartyMngtNotify( PARTYNOTIFY_OPEN );
}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap->Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap->Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	trap->Print ("%s (%i %i %i) : (%i %i %i)\n", cgs.mapname, (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdef.viewangles[0], (int)cg.refdef.viewangles[1], (int)cg.refdef.viewangles[2]);
}


static void CG_ScoresDown_f( void ) {

	CG_BuildSpectatorString();
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap->SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

extern menuDef_t *menuScoreboard;
void Menu_Reset();			// FIXME: add to right include file

static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap->Cmd_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap->SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap->Cmd_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap->SendClientCommand( command );
}


/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	trap->Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}
	if (cg_cameraOrbit.value != 0) {
		trap->Cvar_Set ("cg_cameraOrbit", "0");
		trap->Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap->Cvar_Set("cg_cameraOrbit", "5");
		trap->Cvar_Set("cg_thirdPerson", "1");
		trap->Cvar_Set("cg_thirdPersonAngle", "0");
		trap->Cvar_Set("cg_thirdPersonRange", "100");
	}
}

static void CG_StartCinematic(void) {
	if (cg.cinematicState < 1 || cg.cinematicState > 2) {
		cg.cinematicState = 1;
		cg.cinematicTime = cg.time;
	}
}

static void CG_StopCinematic(void) {
	if (cg.cinematicState > 0 && cg.cinematicState < 3) {
		cg.cinematicState = 3;
		cg.cinematicTime = cg.time;
	}
}

void CG_LoadHudMenu();

static void CG_PrintWeaponMuzzleOffset_f ( void )
{
    centity_t *cent = &cg_entities[cg.snap->ps.clientNum];
    void *g2Weapon = cent->ghoul2;
    
    if ( !trap->G2_HaveWeGhoul2Models (g2Weapon) )
    {
        trap->Print ("Current weapon does not use a GHOUL2 model.\n");
    }
    else if ( !trap->G2API_HasGhoul2ModelOnIndex (&g2Weapon, 1) )
    {
        trap->Print ("Current weapon has no model on index 1.\n");
    }
    else
    {
        static const vec3_t worldForward = { 0.0f, 1.0f, 0.0f };
        mdxaBone_t muzzleBone;
        vec3_t muzzleOffset;
        
        if ( !trap->G2API_GetBoltMatrix (g2Weapon, 1, 0, &muzzleBone, worldForward, vec3_origin, cg.time, cgs.gameModels, cent->modelScale) )
        {
            trap->Print ("Unable to get muzzle bolt matrix for the current weapon.\n");
            return;
        }
        
        BG_GiveMeVectorFromMatrix (&muzzleBone, ORIGIN, muzzleOffset);
        VectorSubtract (muzzleOffset, muzzleOffset, cent->lerpOrigin);
        
        trap->Print ("Muzzle offset at (%f %f %f).\n", muzzleOffset[0], muzzleOffset[1], muzzleOffset[2]);
    }
}

static void JKG_OpenInventoryMenu_f ( void )
{
	if (cg.demoPlayback) {
		// Don't do this in a demo
		return;
	}

	uiImports->InventoryNotify (INVENTORYNOTIFY_OPEN);
}

void JKG_OpenShopMenu_f ( void )
{
	if (cg.demoPlayback) {
		// Don't do this in a demo
		return;
	}
	if (cgs.gametype != GT_DUEL) 
		uiImports->ShopNotify( SHOPNOTIFY_REFRESH );
}

static void JKG_UseACI_f ( void )
{
    char buf[3];
    int slot;
    
    if ( trap->Cmd_Argc() != 2 )
    {
        trap->Print ("Usage: /useACI <slot number>\n");
        return;
    }
    
    trap->Cmd_Argv (1, buf, sizeof (buf));
    if ( buf[0] < '0' || buf[0] > '9' )
    {
        return;
    }
    
    slot = atoi (buf);
    
    if ( slot < 0 || slot >= MAX_ACI_SLOTS )
    {
        return;
    }
    
    if ( !cg.playerACI[slot] )
    {
        return;
    }
    
    cg.weaponSelect = slot;
}

static void Cmd_DumpWeaponList_f ( void )
{
    char filename[MAX_QPATH];
    if ( trap->Cmd_Argc() > 1 )
    {
        trap->Cmd_Argv (1, filename, sizeof (filename));
    }
    else
    {
        Q_strncpyz (filename, "weaponlist.txt", sizeof (filename));
    }
    
    if ( BG_DumpWeaponList (filename) )
    {
        trap->Print ("Weapon list was written to %s.\n", filename);
    }
    else
    {
        trap->Print ("Failed to write weapon list to %s.\n", filename);
    }
}

static void Cmd_PrintWeaponList_f ( void )
{
	BG_PrintWeaponList();
}

static void JKG_PrintItemList(void)
{
	BG_PrintItemList();
}

#ifdef __AUTOWAYPOINT__
extern void AIMod_AutoWaypoint ( void );
extern void AIMod_AutoWaypoint_Clean ( void );
extern void AIMod_MarkBadHeight ( void );
extern void AIMod_AddRemovalPoint ( void );
extern void AIMod_AWC_MarkBadHeight ( void );
extern void CG_ShowSurface ( void );
extern void CG_ShowSlope ( void );
#endif //__AUTOWAYPOINT__

typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "loaddeferred", CG_LoadDeferredPlayers },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "scoresDown", CG_scrollScoresDown_f },
	{ "scoresUp", CG_scrollScoresUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "startcin", CG_StartCinematic },
	{ "startOrbit", CG_StartOrbit_f },
	{ "stopcin", CG_StopCinematic },
	{ "tcmd", CG_TargetCommand_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tell_target", CG_TellTarget_f },
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponclean", CG_Weapon_f },	// this exists to keep old binds in check
	{ "weapprev", CG_PrevWeapon_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	
	
	// Jedi Knight Galaxies
	{ "+camera", CG_Start360Camera },
	{ "-camera", CG_Stop360Camera },
	{ "cameraZoomIn", CG_CameraZoomIn },
	{ "cameraZoomOut", CG_CameraZoomOut },
	//{ "party", CG_OpenPartyManagement_f },
	{ "printWeaponMuzzle", CG_PrintWeaponMuzzleOffset_f },
	{ "useACI", JKG_UseACI_f },
	{ "inventory", JKG_OpenInventoryMenu_f },
	{ "shop", JKG_OpenShopMenu_f },
	{ "dumpWeaponList", Cmd_DumpWeaponList_f },
	{ "printWeaponList", Cmd_PrintWeaponList_f },
	{ "printItemList", JKG_PrintItemList},

#ifdef __AUTOWAYPOINT__
	{ "awp", AIMod_AutoWaypoint },
	{ "autowaypoint", AIMod_AutoWaypoint },
	{ "awc", AIMod_AutoWaypoint_Clean },
	{ "autowaypointclean", AIMod_AutoWaypoint_Clean },
	{ "showsurface", CG_ShowSurface },
	{ "showslope", CG_ShowSlope },
	{ "aw_badheight", AIMod_MarkBadHeight },
	{ "awc_addremovalspot", AIMod_AddRemovalPoint },
	{ "awc_addbadheight", AIMod_AWC_MarkBadHeight }
#endif //__AUTOWAYPOINT__
};

static size_t numCommands = ARRAY_LEN( commands );

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < numCommands ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/

/*
* These commands are on the server. We are adding command completion for them on the client.
*/
static const char *gcmds[] = {
	"addbot",
	"callteamvote",
	"callvote",
	"duelteam",
	"follow",
	"follownext",
	"followprev",
	"forcechanged",
	"give",
	"god",
	"kill",
	"levelshot",
	"loaddefered",
	"noclip",
	"notarget",
	"NPC",
	"say",
	"sayact",
	"sayglobal",
	"sayyell",
	"saywhisper",
	"sayteam",
	"setviewpos",
	"stats",
	"team",
	"teamtask",
	"teamvote",
	"tell",
	"voice_cmd",
	"vote",
	"where",
	"zoom"
};
static size_t numgcmds = ARRAY_LEN(gcmds);

void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < numCommands ; i++ )
		trap->AddCommand( commands[i].cmd );

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	for( i = 0; i < numgcmds; i++ )
		trap->AddCommand( gcmds[i] );
}
