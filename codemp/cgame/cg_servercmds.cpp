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

// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "../../JKGalaxies/ui/menudef.h"
#include "ghoul2/G2.h"
#include "ui/ui_public.h"

// Jedi Knight Galaxies
#include "qcommon/game_version.h"
#include "../game/bg_items.h"

/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores( void ) {
	int		i, powerups, readScores;

	cg.numScores = atoi( CG_Argv( 1 ) );

	readScores = cg.numScores;

	if (readScores > MAX_CLIENT_SCORE_SEND)
	{
		readScores = MAX_CLIENT_SCORE_SEND;
	}

	if ( cg.numScores > MAX_CLIENTS ) {
		cg.numScores = MAX_CLIENTS;
	}

	cg.numScores = readScores;

	cg.teamScores[0] = atoi( CG_Argv( 2 ) );
	cg.teamScores[1] = atoi( CG_Argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < readScores ; i++ ) {
		//
		cg.scores[i].client = atoi( CG_Argv( i * 6 + 4 ) );
		cg.scores[i].score = atoi( CG_Argv( i * 6 + 5 ) );
		cg.scores[i].ping = atoi( CG_Argv( i * 6 + 6 ) );
		cg.scores[i].time = atoi( CG_Argv( i * 6 + 7 ) );
		cg.scores[i].scoreFlags = atoi( CG_Argv( i * 6 + 8 ) );
		powerups = atoi( CG_Argv( i * 6 + 9 ) );

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cgs.clientinfo[ cg.scores[i].client ].powerups = powerups;

		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
	}
	CG_SetScoreSelection(NULL);
}

/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo( void )
{
	int	i, iID, iLen;

	for ( i = 0, iLen = (( trap->Cmd_Argc() - 1 ) / 9 ); i < iLen; i++ )
	{
		iID = atoi( CG_Argv( i * 9 + 1 ));;
		cgs.clientinfo[iID].location = atoi( CG_Argv( i * 9 + 2 ));
		cgs.clientinfo[iID].health = atoi( CG_Argv( i * 9 + 3 ));
		cgs.clientinfo[iID].maxhealth = atoi( CG_Argv( i * 9 + 4 ));
		cgs.clientinfo[iID].armor = atoi( CG_Argv( i * 9 + 5 ));
		cgs.clientinfo[iID].maxarmor = atoi( CG_Argv( i * 9 + 6 ));
		cgs.clientinfo[iID].curForcePower = atoi( CG_Argv( i * 9 + 7 ));
		cgs.clientinfo[iID].maxForcePower = atoi( CG_Argv( i * 9 + 8 ));
		cgs.clientinfo[iID].curWeapon = atoi( CG_Argv( i * 9 + 9 ));
	}
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;
	int		i;

	info = CG_ConfigString( CS_SERVERINFO );

	cgs.debugMelee = atoi( Info_ValueForKey( info, "g_debugMelee" ) ); //trap->Cvar_GetHiddenVarValue("g_iknowkungfu");
	cgs.stepSlideFix = atoi( Info_ValueForKey( info, "g_stepSlideFix" ) );

	cgs.noSpecMove = atoi( Info_ValueForKey( info, "g_noSpecMove" ) );

	//trap->Cvar_Set("bg_fighterAltControl", Info_ValueForKey( info, "bg_fighterAltControl" ));

	cgs.showDuelHealths = atoi( Info_ValueForKey( info, "g_showDuelHealths" ) );

	cgs.gametype = (gametype_t)atoi( Info_ValueForKey( info, "g_gametype" ) );
	trap->Cvar_Set("g_gametype", va("%i", cgs.gametype));
	cgs.needpass = atoi( Info_ValueForKey( info, "needpass" ) );
	cgs.wDisable = atoi( Info_ValueForKey( info, "wdisable" ) );
	cgs.fDisable = atoi( Info_ValueForKey( info, "fdisable" ) );
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
	cgs.duel_fraglimit = atoi( Info_ValueForKey( info, "duel_fraglimit" ) );
	cgs.capturelimit = atoi( Info_ValueForKey( info, "capturelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	mapname = Info_ValueForKey( info, "mapname" );

	//rww - You must do this one here, Info_ValueForKey always uses the same memory pointer.
	trap->Cvar_Set ( "ui_about_mapname", mapname );

	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	Com_sprintf( cgs.rawmapname, sizeof( cgs.rawmapname ), "%s", mapname );
//	Q_strncpyz( cgs.redTeam, Info_ValueForKey( info, "g_redTeam" ), sizeof(cgs.redTeam) );
//	trap->Cvar_Set("g_redTeam", cgs.redTeam);
//	Q_strncpyz( cgs.blueTeam, Info_ValueForKey( info, "g_blueTeam" ), sizeof(cgs.blueTeam) );
//	trap->Cvar_Set("g_blueTeam", cgs.blueTeam);

	trap->Cvar_Set ( "ui_about_gametype", va("%i", cgs.gametype ) );
	trap->Cvar_Set ( "ui_about_fraglimit", va("%i", cgs.fraglimit ) );
	trap->Cvar_Set ( "ui_about_duellimit", va("%i", cgs.duel_fraglimit ) );
	trap->Cvar_Set ( "ui_about_capturelimit", va("%i", cgs.capturelimit ) );
	trap->Cvar_Set ( "ui_about_timelimit", va("%i", cgs.timelimit ) );
	trap->Cvar_Set ( "ui_about_maxclients", va("%i", cgs.maxclients ) );
	trap->Cvar_Set ( "ui_about_dmflags", va("%i", cgs.dmflags ) );
	trap->Cvar_Set ( "ui_about_hostname", Info_ValueForKey( info, "sv_hostname" ) );
	trap->Cvar_Set ( "ui_about_needpass", Info_ValueForKey( info, "g_needpass" ) );
	trap->Cvar_Set ( "ui_about_botminplayers", Info_ValueForKey ( info, "bot_minplayers" ) );

	//Set the siege teams based on what the server has for overrides.
	trap->Cvar_Set("cg_siegeTeam1", Info_ValueForKey(info, "g_siegeTeam1"));
	trap->Cvar_Set("cg_siegeTeam2", Info_ValueForKey(info, "g_siegeTeam2"));

	//Raz: Fix bogus vote strings
	Q_strncpyz( cgs.voteString, CG_ConfigString( CS_VOTE_STRING ), sizeof( cgs.voteString ) );

	//Raz: Synchronise our expected snaps/sec with the server's framerate
	//		OpenJK servers will try to match us to the sv_fps too (sv_client.cpp -> SV_UserinfoChanged)
	i = atoi( Info_ValueForKey( info, "sv_fps" ) );
	if ( i )
		trap->Cvar_Set( "snaps", va( "%i", i ) );
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
	const char	*info;
	int			warmup;

	info = CG_ConfigString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	cg.warmup = warmup;
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) 
{
	const char *s;
	const char *str;

	cgs.scores1 = atoi( CG_ConfigString( CS_SCORES1 ) );
	cgs.scores2 = atoi( CG_ConfigString( CS_SCORES2 ) );
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );
	if( cgs.gametype == GT_CTF ) {
		s = CG_ConfigString( CS_FLAGSTATUS );
		cgs.redflag = s[0] - '0';
		cgs.blueflag = s[1] - '0';
	}
	cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );

	// Track who the jedi master is
	cgs.duelWinner = atoi ( CG_ConfigString ( CS_CLIENT_DUELWINNER ) );

	str = CG_ConfigString(CS_CLIENT_DUELISTS);

	if (str && str[0])
	{
		char buf[64];
		int c = 0;
		int i = 0;

		while (str[i] && str[i] != '|')
		{
			buf[c] = str[i];
			c++;
			i++;
		}
		buf[c] = 0;

		cgs.duelist1 = atoi ( buf );
		c = 0;

		i++;
		while (str[i])
		{
			buf[c] = str[i];
			c++;
			i++;
		}
		buf[c] = 0;

		cgs.duelist2 = atoi ( buf );
	}
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;

	o = CG_ConfigString( CS_SHADERSTATE );
	while (o && *o) {
		n = const_cast<char *>(strstr(o, "="));
		if (n && *n) {
			strncpy(originalShader, o, n-o);
			originalShader[n-o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				strncpy(newShader, n, t-n);
				newShader[t-n] = 0;
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				strncpy(timeOffset, t, o-t);
				timeOffset[o-t] = 0;
				o++;
				trap->R_RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

extern char *cg_customSoundNames[MAX_CUSTOM_SOUNDS];
extern const char *cg_customCombatSoundNames[MAX_CUSTOM_COMBAT_SOUNDS];
extern const char *cg_customExtraSoundNames[MAX_CUSTOM_EXTRA_SOUNDS];
extern const char *cg_customJediSoundNames[MAX_CUSTOM_JEDI_SOUNDS];
extern const char *cg_customDuelSoundNames[MAX_CUSTOM_DUEL_SOUNDS];

static const char *GetCustomSoundForType(int setType, int index)
{
	switch (setType)
	{
	case 1:
		return cg_customSoundNames[index];
	case 2:
		return cg_customCombatSoundNames[index];
	case 3:
		return cg_customExtraSoundNames[index];
	case 4:
		return cg_customJediSoundNames[index];
	case 5:
		return bg_customSiegeSoundNames[index];
	case 6:
		return cg_customDuelSoundNames[index];
	default:
		assert(0);
		return NULL;
	}
}

void SetCustomSoundForType(clientInfo_t *ci, int setType, int index, sfxHandle_t sfx)
{
	switch (setType)
	{
	case 1:
		ci->sounds[index] = sfx;
		break;
	case 2:
		ci->combatSounds[index] = sfx;
		break;
	case 3:
		ci->extraSounds[index] = sfx;
		break;
	case 4:
		ci->jediSounds[index] = sfx;
		break;
	case 5:
		ci->siegeSounds[index] = sfx;
		break;
	case 6:
		ci->duelSounds[index] = sfx;
		break;
	default:
		assert(0);
		break;
	}
}

static void CG_RegisterCustomSounds(clientInfo_t *ci, int setType, const char *psDir)
{
	int iTableEntries = 0;
	int i;

	switch (setType)
	{
	case 1:
		iTableEntries = MAX_CUSTOM_SOUNDS;
		break;
	case 2:
		iTableEntries = MAX_CUSTOM_COMBAT_SOUNDS;
		break;
	case 3:
		iTableEntries = MAX_CUSTOM_EXTRA_SOUNDS;
		break;
	case 4:
		iTableEntries = MAX_CUSTOM_JEDI_SOUNDS;
		break;
	case 5:
		iTableEntries = MAX_CUSTOM_SIEGE_SOUNDS;
	default:
		assert(0);
		return;
	}

	for ( i = 0 ; i<iTableEntries; i++ ) 
	{
		sfxHandle_t hSFX;
		const char *s = GetCustomSoundForType(setType, i);

		if ( !s ) 
		{
			break;
		}

		s++;
		hSFX = trap->S_RegisterSound( va("sound/chars/%s/misc/%s", psDir, s) );

		if (hSFX == 0)
		{
			char modifiedSound[MAX_QPATH];
			char *p;

			strcpy(modifiedSound, s);
			p = strchr(modifiedSound,'.');

			if (p)
			{
				char testNumber[2];
				p--;

				//before we destroy it.. we want to see if this is actually a number.
				//If it isn't a number then don't try decrementing and registering as
				//it will only cause a disk hit (we don't try precaching such files)
				testNumber[0] = *p;
				testNumber[1] = 0;
				if (atoi(testNumber))
				{
					*p = 0;

					strcat(modifiedSound, "1.wav");

					hSFX = trap->S_RegisterSound( va("sound/chars/%s/misc/%s", psDir, modifiedSound) );
				}
			}
		}
		
		SetCustomSoundForType(ci, setType, i, hSFX);
	}
}

void CG_PrecacheNPCSounds(const char *str)
{
	char sEnd[MAX_QPATH];
	char pEnd[MAX_QPATH];
	int i = 0;
	int j = 0;
	int k = 0;

	k = 2;

	while (str[k])
	{
		pEnd[k-2] = str[k];
		k++;
	}
	pEnd[k-2] = 0;

	while (i < 4) //4 types
	{ //It would be better if we knew what type this actually was (extra, combat, jedi, etc).
	  //But that would require extra configstring indexing and that is a bad thing.

		while (j < MAX_CUSTOM_SOUNDS)
		{
			const char *s = GetCustomSoundForType(i+1, j);

			if (s && s[0])
			{ //whatever it is, try registering it under this folder.
				k = 1;
				while (s[k])
				{
					sEnd[k-1] = s[k];
					k++;
				}
				sEnd[k-1] = 0;

				trap->S_Shutup(qtrue);
				trap->S_RegisterSound( va("sound/chars/%s/misc/%s", pEnd, sEnd) );
				trap->S_Shutup(qfalse);
			}
			else
			{ //move onto the next set
				break;
			}

			j++;
		}

		j = 0;
		i++;
	}
}

void CG_HandleNPCSounds(centity_t *cent)
{
	if (!cent->npcClient)
	{
		return;
	}

	//standard
	if (cent->currentState.csSounds_Std)
	{
		const char *s = CG_ConfigString( CS_SOUNDS + cent->currentState.csSounds_Std );

		if (s && s[0])
		{
			char sEnd[MAX_QPATH];
			int i = 2;
			int j = 0;

			//Parse past the initial "*" which indicates this is a custom sound, and the $ which indicates
			//it is an NPC custom sound dir.
			while (s[i])
			{
				sEnd[j] = s[i];
				j++;
				i++;
			}
			sEnd[j] = 0;

			CG_RegisterCustomSounds(cent->npcClient, 1, sEnd);
		}
	}
	else
	{
		memset(&cent->npcClient->sounds, 0, sizeof(cent->npcClient->sounds));
	}

	//combat
	if (cent->currentState.csSounds_Combat)
	{
		const char *s = CG_ConfigString( CS_SOUNDS + cent->currentState.csSounds_Combat );

		if (s && s[0])
		{
			char sEnd[MAX_QPATH];
			int i = 2;
			int j = 0;

			//Parse past the initial "*" which indicates this is a custom sound, and the $ which indicates
			//it is an NPC custom sound dir.
			while (s[i])
			{
				sEnd[j] = s[i];
				j++;
				i++;
			}
			sEnd[j] = 0;

			CG_RegisterCustomSounds(cent->npcClient, 2, sEnd);
		}
	}
	else
	{
		memset(&cent->npcClient->combatSounds, 0, sizeof(cent->npcClient->combatSounds));
	}

	//extra
	if (cent->currentState.csSounds_Extra)
	{
		const char *s = CG_ConfigString( CS_SOUNDS + cent->currentState.csSounds_Extra );

		if (s && s[0])
		{
			char sEnd[MAX_QPATH];
			int i = 2;
			int j = 0;

			//Parse past the initial "*" which indicates this is a custom sound, and the $ which indicates
			//it is an NPC custom sound dir.
			while (s[i])
			{
				sEnd[j] = s[i];
				j++;
				i++;
			}
			sEnd[j] = 0;

			CG_RegisterCustomSounds(cent->npcClient, 3, sEnd);
		}
	}
	else
	{
		memset(&cent->npcClient->extraSounds, 0, sizeof(cent->npcClient->extraSounds));
	}

	//jedi
	if (cent->currentState.csSounds_Jedi)
	{
		const char *s = CG_ConfigString( CS_SOUNDS + cent->currentState.csSounds_Jedi );

		if (s && s[0])
		{
			char sEnd[MAX_QPATH];
			int i = 2;
			int j = 0;

			//Parse past the initial "*" which indicates this is a custom sound, and the $ which indicates
			//it is an NPC custom sound dir.
			while (s[i])
			{
				sEnd[j] = s[i];
				j++;
				i++;
			}
			sEnd[j] = 0;

			CG_RegisterCustomSounds(cent->npcClient, 4, sEnd);
		}
	}
	else
	{
		memset(&cent->npcClient->jediSounds, 0, sizeof(cent->npcClient->jediSounds));
	}
}

int CG_HandleAppendedSkin(char *modelName);
void CG_CacheG2AnimInfo(char *modelName);

// nmckenzie: DUEL_HEALTH - fixme - we could really clean this up immensely with some helper functions.
void SetDuelistHealthsFromConfigString ( const char *str ) {
	char buf[64];
	int c = 0;
	int i = 0;

	while (str[i] && str[i] != '|')
	{
		buf[c] = str[i];
		c++;
		i++;
	}
	buf[c] = 0;

	cgs.duelist1health = atoi ( buf );

	c = 0;
	i++;
	while (str[i] && str[i] != '|')
	{
		buf[c] = str[i];
		c++;
		i++;
	}
	buf[c] = 0;

	cgs.duelist2health = atoi ( buf );

	c = 0;
	i++;
	if ( str[i] == '!' )
	{	// we only have 2 duelists, apparently.
		cgs.duelist3health = -1;
		return;
	}

	while (str[i] && str[i] != '|')
	{
		buf[c] = str[i];
		c++;
		i++;
	}
	buf[c] = 0;

	cgs.duelist3health = atoi ( buf );
}

/*
================
CG_ConfigStringModified

================
*/
void CG_ParseWeatherEffect(const char *str);
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap->GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic( qtrue );
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if ( num == CS_SCORES1 ) {
		cgs.scores1 = atoi( str );
	} else if ( num == CS_SCORES2 ) {
		cgs.scores2 = atoi( str );
	}
	else if ( num == CS_CLIENT_DUELWINNER )
	{
		cgs.duelWinner = atoi ( str );
	}
	else if ( num == CS_CLIENT_DUELISTS )
	{
		char buf[64];
		int c = 0;
		int i = 0;

		while (str[i] && str[i] != '|')
		{
			buf[c] = str[i];
			c++;
			i++;
		}
		buf[c] = 0;

		cgs.duelist1 = atoi ( buf );
		c = 0;

		i++;
		while (str[i] && str[i] != '|')
		{
			buf[c] = str[i];
			c++;
			i++;
		}
		buf[c] = 0;

		cgs.duelist2 = atoi ( buf );

		if (str[i])
		{
			c = 0;
			i++;

			while (str[i])
			{
				buf[c] = str[i];
				c++;
				i++;
			}
			buf[c] = 0;

			cgs.duelist3 = atoi(buf);
		}
	}
	else if ( num == CS_CLIENT_DUELHEALTHS ) {	// nmckenzie: DUEL_HEALTH
		SetDuelistHealthsFromConfigString(str);
	}
	else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
	} else if ( num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1) {
		cgs.teamVoteTime[num-CS_TEAMVOTE_TIME] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_TIME] = qtrue;
	} else if ( num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1) {
		cgs.teamVoteYes[num-CS_TEAMVOTE_YES] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_YES] = qtrue;
	} else if ( num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1) {
		cgs.teamVoteNo[num-CS_TEAMVOTE_NO] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_NO] = qtrue;
	} else if ( num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1) {
		Q_strncpyz( cgs.teamVoteString[num-CS_TEAMVOTE_STRING], str, sizeof( cgs.teamVoteString ) );
	} else if ( num == CS_INTERMISSION ) {
		cg.intermissionStarted = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		char modelName[MAX_QPATH];
		strcpy(modelName, str);
		if (strstr(modelName, ".glm") || modelName[0] == '$')
		{ //Check to see if it has a custom skin attached.
			CG_HandleAppendedSkin(modelName);
			CG_CacheG2AnimInfo(modelName);
		}

		cgs.gameModels[ num-CS_MODELS ] = 0;
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap->S_RegisterSound( str );
		}
		else if (str[1] == '$')
		{ //an NPC soundset
			CG_PrecacheNPCSounds(str);
		}
	} else if ( num >= CS_EFFECTS && num < CS_EFFECTS+MAX_FX ) {
		if (str[0] == '*')
		{ //it's a special global weather effect
			CG_ParseWeatherEffect(str);
			cgs.gameEffects[ num-CS_EFFECTS] = 0;
		}
		else
		{
			cgs.gameEffects[ num-CS_EFFECTS] = trap->FX_RegisterEffect( str );
		}
	}
	else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS )
	{
		CG_NewClientInfo( num - CS_PLAYERS, qtrue);
		CG_BuildSpectatorString();
	} else if ( num == CS_FLAGSTATUS ) {
		if( cgs.gametype == GT_CTF ) {
			// format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
			cgs.redflag = str[0] - '0';
			cgs.blueflag = str[1] - '0';
		}
	}
	else if ( num == CS_SHADERSTATE ) {
		CG_ShaderStateChanged();
	}
	else if ( num >= CS_LIGHT_STYLES && num < CS_LIGHT_STYLES + (MAX_LIGHT_STYLES * 3))
	{
		CG_SetLightstyle(num - CS_LIGHT_STYLES);
	}
		
}

//frees all ghoul2 stuff and npc stuff from a centity -rww
void CG_KillCEntityG2(int entNum)
{
	int j;
	clientInfo_t *ci = NULL;
	centity_t *cent = &cg_entities[entNum];

	if (entNum < MAX_CLIENTS)
	{
		ci = &cgs.clientinfo[entNum];
	}
	else
	{
		ci = cent->npcClient;
	}

	if (ci)
	{
		if (ci == cent->npcClient)
		{ //never going to be != cent->ghoul2, unless cent->ghoul2 has already been removed (and then this ptr is not valid)
			ci->ghoul2Model = NULL;
		}
		else if (ci->ghoul2Model == cent->ghoul2)
		{
			ci->ghoul2Model = NULL;
		}
		else if (ci->ghoul2Model && trap->G2_HaveWeGhoul2Models(ci->ghoul2Model))
		{
			trap->G2API_CleanGhoul2Models(&ci->ghoul2Model);
			ci->ghoul2Model = NULL;
		}

		//Clean up any weapon instances for custom saber stuff
		j = 0;
		while (j < MAX_SABERS)
		{
			if (ci->ghoul2Weapons[j] && trap->G2_HaveWeGhoul2Models(ci->ghoul2Weapons[j]))
			{
				trap->G2API_CleanGhoul2Models(&ci->ghoul2Weapons[j]);
				ci->ghoul2Weapons[j] = NULL;
			}

			j++;
		}
	}

	if (cent->ghoul2 && trap->G2_HaveWeGhoul2Models(cent->ghoul2))
	{
		trap->G2API_CleanGhoul2Models(&cent->ghoul2);
		cent->ghoul2 = NULL;
	}

	if (cent->grip_arm && trap->G2_HaveWeGhoul2Models(cent->grip_arm))
	{
		trap->G2API_CleanGhoul2Models(&cent->grip_arm);
		cent->grip_arm = NULL;
	}

	if (cent->frame_hold && trap->G2_HaveWeGhoul2Models(cent->frame_hold))
	{
		trap->G2API_CleanGhoul2Models(&cent->frame_hold);
		cent->frame_hold = NULL;
	}

	if (cent->npcClient)
	{
		CG_DestroyNPCClient(&cent->npcClient);
	}

	cent->isRagging = qfalse; //just in case.
	cent->ikStatus = qfalse;

	cent->localAnimIndex = 0;
}

void CG_KillCEntityInstances(void)
{
	int i = 0;
	centity_t *cent;

	while (i < MAX_GENTITIES)
	{
		cent = &cg_entities[i];

		if (i >= MAX_CLIENTS && cent->currentState.number == i)
		{ //do not clear G2 instances on client ents, they are constant
			CG_KillCEntityG2(i);
		}

		cent->bolt1 = 0;
		cent->bolt2 = 0;
		cent->bolt3 = 0;
		cent->bolt4 = 0;

		cent->bodyHeight = 0;//SABER_LENGTH_MAX;
		//cent->saberExtendTime = 0;

		cent->boltInfo = 0;

		cent->frame_minus1_refreshed = 0;
		cent->frame_minus2_refreshed = 0;
		cent->dustTrailTime = 0;
		cent->ghoul2weapon = NULL;
		//cent->torsoBolt = 0;
		cent->trailTime = 0;
		cent->frame_hold_time = 0;
		cent->frame_hold_refreshed = 0;
		cent->trickAlpha = 0;
		cent->trickAlphaTime = 0;
		VectorClear(cent->turAngles);
		cent->weapon = 0;
		cent->teamPowerEffectTime = 0;
		cent->teamPowerType = 0;
		cent->numLoopingSounds = 0;

		cent->localAnimIndex = 0;

		i++;
	}
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showMiss.integer ) {
		trap->Print( "CG_MapRestart\n" );
	}

	trap->R_ClearDecals ( );
	//FIXME: trap->FX_Reset?

	CG_InitLocalEntities();
	CG_InitMarkPolys();
	CG_KillCEntityInstances();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;

	cg.intermissionStarted = qfalse;

	cgs.voteTime = 0;

	cg.mapRestart = qtrue;

	CG_StartMusic(qtrue);

	trap->S_ClearLoopingSounds();

	// we really should clear more parts of cg here and stop sounds

	// play the "fight" sound if this is a restart without warmup
	if ( cg.warmup == 0 && cgs.gametype != GT_POWERDUEL/* && cgs.gametype == GT_DUEL */) {
		trap->S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		CG_CenterPrint( CG_GetStringEdString("MP_SVGAME", "BEGIN_DUEL"), 120, GIANTCHAR_WIDTH*2 );
	}
	/*
	if (cg_singlePlayerActive.integer) {
		trap->Cvar_Set("ui_matchStartTime", va("%i", cg.time));
		if (cg_recordSPDemo.integer && cg_recordSPDemoName.string && *cg_recordSPDemoName.string) {
			trap->SendConsoleCommand(va("set g_synchronousclients 1 ; record %s \n", cg_recordSPDemoName.string));
		}
	}
	*/
	trap->Cvar_Set("cg_thirdPerson", "0");
	cg.playerInventory->clear();
	memset(cg.playerACI, -1, sizeof(cg.playerACI));
}

/*
=================
JKG_FireModeUpdate

Plays a sound and changes the animation stuffs for the gun
=================
*/

int JKG_GetTransitionForFiringModeSet(int previous, int next)
{
	switch(previous)
	{
		case FMANIM_NONE:
			switch(next)
			{
				case FMANIM_NONE:
					return FMTRANS_NONE_NONE;
				case FMANIM_RAISED:
					return FMTRANS_NONE_RAISED;
				case FMANIM_TILTED:
					return FMTRANS_NONE_TILTED;
			}
		case FMANIM_RAISED:
			switch(next)
			{
				case FMANIM_NONE:
					return FMTRANS_RAISED_NONE;
				case FMANIM_RAISED:
					return FMTRANS_RAISED_RAISED;
				case FMANIM_TILTED:
					return FMTRANS_RAISED_TILTED;
			}
			break;
		case FMANIM_TILTED:
			switch(next)
			{
				case FMANIM_NONE:
					return FMTRANS_TILTED_NONE;
				case FMANIM_RAISED:
					return FMTRANS_TILTED_RAISED;
				case FMANIM_TILTED:
					return FMTRANS_TILTED_TILTED;
			}
			break;
	}
	return 0;
}

static void JKG_FireModeUpdate(void)
{
	const weaponData_t *wpData = GetWeaponData( cg.predictedPlayerState.weapon, cg.predictedPlayerState.weaponVariation );
	const char *previousFM = CG_Argv(1);
	int previousFMInt = atoi(previousFM);
	if( wpData->visuals.visualFireModes[ cg.predictedPlayerState.firingMode ].switchToSound[0] &&
		wpData->numFiringModes > 1 )
	{
		trap->S_StartLocalSound( trap->S_RegisterSound( wpData->visuals.visualFireModes[ cg.predictedPlayerState.firingMode ].switchToSound ), CHAN_AUTO );
	}

	//if( wpData->visuals.visualFireModes[ previousFMInt ].animType != wpData->visuals.visualFireModes[ cg.predictedPlayerState.firingMode ].animType )
	{
		cg.fireModeTransition = JKG_GetTransitionForFiringModeSet( wpData->visuals.visualFireModes[ previousFMInt ].animType, wpData->visuals.visualFireModes[ cg.predictedPlayerState.firingMode ].animType );
	}
	cg.fireModeChangeTime = cg.time;
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

#define MAX_STRINGED_SV_STRING MAX_STRING_CHARS	// this is an quake-engine limit, not a StringEd limit

void CG_CheckSVStringEdRef(char *buf, const char *str)
{ //I don't really like doing this. But it utilizes the system that was already in place.
	int i = 0;
	int b = 0;
	int strLen = 0;
	qboolean gotStrip = qfalse;

	if (!str || !str[0])
	{
		if (str)
		{
			strcpy(buf, str);
		}
		return;
	}

	strcpy(buf, str);

	strLen = strlen(str);

	if (strLen >= MAX_STRINGED_SV_STRING)
	{
		return;
	}

	while (i < strLen && str[i])
	{
		gotStrip = qfalse;

		if (str[i] == '@' && (i+1) < strLen)
		{
			if (str[i+1] == '@' && (i+2) < strLen)
			{
				if (str[i+2] == '@' && (i+3) < strLen)
				{ //@@@ should mean to insert a StringEd reference here, so insert it into buf at the current place
					char stringRef[MAX_STRINGED_SV_STRING];
					int r = 0;

					/*while (i < strLen && str[i] == '@')
					{
						i++;
					}*/
					// Oh c'mon.
					i += 3;

					while (i < strLen && str[i] && str[i] != ' ' && str[i] != ':' && str[i] != '.' && str[i] != '\n')
					{
						stringRef[r] = str[i];
						r++;
						i++;
					}
					stringRef[r] = 0;

					buf[b] = 0;
					// Bugfix -> DONT JUMP TO CONCLUSIONS, SILLY RAVEN
					{
						char buffer2[1024];
						strcpy(buffer2, CG_GetStringEdString2(stringRef));
						if(Q_stricmp(buffer2, stringRef))
						{
							Q_strcat(buf, MAX_STRINGED_SV_STRING, buffer2);
							return;
						}
					}
					Q_strcat(buf, MAX_STRINGED_SV_STRING, CG_GetStringEdString("MP_SVGAME", stringRef));	// Might be a valid point...but WTF seriously
					b = strlen(buf);
				}
			}
		}

		if (!gotStrip)
		{
			buf[b] = str[i];
			b++;
		}
		i++;
	}

	buf[b] = 0;
}

static void CG_BodyQueueCopy(centity_t *cent, int clientNum, int knownWeapon, int weaponVariation)
{
	centity_t		*source;
	animation_t		*anim;
	float			animSpeed;
	int				flags=BONE_ANIM_OVERRIDE_FREEZE;
	clientInfo_t	*ci;

	if (cent->ghoul2)
	{
		trap->G2API_CleanGhoul2Models(&cent->ghoul2);
	}

	if (clientNum < 0 || clientNum >= MAX_CLIENTS)
	{
		return;
	}

	source = &cg_entities[ clientNum ];
	ci = &cgs.clientinfo[ clientNum ];

	if (!source)
	{
		return;
	}

	if (!source->ghoul2)
	{
		return;
	}

	cent->isRagging = qfalse; //reset in case it's still set from another body that was in this cent slot.
	cent->ownerRagging = source->isRagging; //if the owner was in ragdoll state, then we want to go into it too right away.

#if 0
	VectorCopy(source->lerpOriginOffset, cent->lerpOriginOffset);
#endif

	cent->bodyFadeTime = 0;
	cent->bodyHeight = 0;

	cent->dustTrailTime = source->dustTrailTime;

	trap->G2API_DuplicateGhoul2Instance(source->ghoul2, &cent->ghoul2);

	if (source->isRagging)
	{ //just reset it now.
		source->isRagging = qfalse;
		trap->G2API_SetRagDoll(source->ghoul2, NULL); //calling with null parms resets to no ragdoll.
	}

	//either force the weapon from when we died or remove it if it was a dropped weapon
	if (knownWeapon > WP_BRYAR_PISTOL && trap->G2API_HasGhoul2ModelOnIndex(&(cent->ghoul2), 1))
	{
		trap->G2API_RemoveGhoul2Model(&(cent->ghoul2), 1);
	}
	else if (trap->G2API_HasGhoul2ModelOnIndex(&(cent->ghoul2), 1))
	{
		trap->G2API_CopySpecificGhoul2Model(CG_G2WeaponInstance(cent, knownWeapon, weaponVariation), 0, cent->ghoul2, 1);
	}

	if (!cent->ownerRagging)
	{
		int aNum;
		int eFrame;
		qboolean fallBack = qfalse;

		//anim = &bgAllAnims[cent->localAnimIndex].anims[ cent->currentState.torsoAnim ];
		if (!BG_InDeathAnim(source->currentState.torsoAnim))
		{ //then just snap the corpse into a default
			anim = &bgAllAnims[source->localAnimIndex].anims[ BOTH_DEAD1 ];
			fallBack = qtrue;
		}
		else
		{
			anim = &bgAllAnims[source->localAnimIndex].anims[ source->currentState.torsoAnim ];
		}
		animSpeed = 50.0f / anim->frameLerp;

		if (!fallBack)
		{
			//this will just set us to the last frame of the animation, in theory
			aNum = cgs.clientinfo[source->currentState.number].frame+1;

			while (aNum >= anim->firstFrame+anim->numFrames)
			{
				aNum--;
			}

			if (aNum < anim->firstFrame-1)
			{ //wrong animation...?
				aNum = (anim->firstFrame+anim->numFrames)-1;
			}
		}
		else
		{
			aNum = anim->firstFrame;
		}

		eFrame = anim->firstFrame + anim->numFrames;

		//if (!cgs.clientinfo[source->currentState.number].frame || (cent->currentState.torsoAnim) != (source->currentState.torsoAnim) )
		//{
		//	aNum = (anim->firstFrame+anim->numFrames)-1;
		//}

		trap->G2API_SetBoneAnim(cent->ghoul2, 0, "upper_lumbar", aNum, eFrame, flags, animSpeed, cg.time, -1, 150);
		trap->G2API_SetBoneAnim(cent->ghoul2, 0, "model_root", aNum, eFrame, flags, animSpeed, cg.time, -1, 150);
		trap->G2API_SetBoneAnim(cent->ghoul2, 0, "Motion", aNum, eFrame, flags, animSpeed, cg.time, -1, 150);
	}

	//After we create the bodyqueue, regenerate any limbs on the real instance
	if (source->torsoBolt)
	{
		CG_ReattachLimb(source);
	}
}

void JKG_ClientBoughtItem(const char* szPlayerName, int nItemID)
{
	if (nItemID < 0)
	{
		return;
	}

	Com_Printf("%s ^2purchased^7 %s!\n", szPlayerName, Q_strpnl((char*)CG_GetStringEdString2(itemLookupTable[nItemID].displayName)));
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/

char *_Cmd_Argv(int idx) {
	static char buff[1024];
	buff[0] = 0;
	trap->Cmd_Argv(idx, buff, 1024);
	return buff;
}



static void CG_ServerRedirect() {
	char connAddress[MAX_TOKEN_CHARS];
	trap->Cmd_Argv( 1, connAddress, sizeof(connAddress) );

	// We must have the server address at least
	if ( trap->Cmd_Argc() < 2 ) {
		return;
	}

	// Because we're executing a command containing bits of the server command, we need to filter for \n, \r, ;, and "
	if ( strchr(connAddress,'\n') || strchr(connAddress,'\r') || strchr(connAddress,';') || strchr(connAddress,'"') ) {
		return;
	}

	trap->Cvar_Set( "cflag", _Cmd_Argv(2) );
	trap->Cvar_Set( "connmsg", _Cmd_Argv(3) );
	trap->SendConsoleCommand( va( "connect \"%s\"\n", connAddress ) );

	return;
}

/* Enables or disables the engine's self-sabotage mechanism */
/* When activated, this will randomly crash the player with packet parsing errors */

extern void CG_ChatBox_AddString(char *chatStr, int fadeLevel); //cg_draw.c
void Cin_ProcessCinematic_f();
void Cin_ProcessCinematicBinary_f();
void Cmd_CBB_f(void);
void CinBuild_Cmd_f();
void ChatBox_CloseChat();
extern void JKG_OpenShopMenu_f ( void );
extern int shopItems[128];
extern int numShopItems;

static void CG_ServerCommand( void ) {
	const char	*cmd;
	char		text[MAX_NOTIFICATION_CHARS]; // extra bytes for name
	qboolean	IRCG = qfalse;

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	// Jedi Knight Galaxies
	// Check the crossover
	if ( uiImports->HandleServerCommand( cmd ) )
		return;

	if (!strcmp(cmd, "svr")) { // Server redirect
		CG_ServerRedirect();
		return;
	}

	if (!strcmp(cmd, "cin")) {
		Cin_ProcessCinematic_f();
		return;
	}

	if (!strcmp(cmd, "cinb")) {
		Cin_ProcessCinematicBinary_f();
		return;
	}

	if (!strcmp(cmd, "cb")) {
		CinBuild_Cmd_f();
		return;
	}

	if (!strcmp(cmd, "cbb")) {
		Cmd_CBB_f();
		return;
	}
	if (!strcmp(cmd, "dc")) {
		cg.deathcamFadeStart = cg.time;
		cg.deathcamTime = atoi(_Cmd_Argv(1));
		cg.deathcamRadius = atoi(_Cmd_Argv(2));
		VectorSet(cg.deathcamCenter, atof(_Cmd_Argv(3)),atof(_Cmd_Argv(4)),atof(_Cmd_Argv(5)));
		ChatBox_CloseChat();
		return;
	}

	if (!strcmp(cmd, "dcr")) {
		cg.deathcamFadeStart = 0;
		cg.deathcamTime = 0;
		cg.deathcamRadius = 0;
		VectorSet(cg.deathcamCenter, 0, 0, 0);
		return;
	}

	// Forced weapon change
	if (!strcmp(cmd, "chw")) {
		cg.weaponSelect = atoi(CG_Argv(1));
		return;
	}

	if (!strcmp(cmd, "clearinv"))
	{
		cg.playerInventory->clear();
		return;
	}

	if ( !strcmp( cmd, "spc" ) )
	{
		trap->Cvar_Set("ui_myteam", "3");
		trap->OpenUIMenu(UIMENU_PLAYERCONFIG); //UIMENU_CLASSSEL
		return;
	}

	if ( !strcmp( cmd, "nfr" ) )
	{ //"nfr" == "new force rank" (want a short string)
		int doMenu = 0;
		int setTeam = 0;
		int newRank = 0;

		if (trap->Cmd_Argc() < 3)
		{
#ifdef _DEBUG
			Com_Printf("WARNING: Invalid newForceRank string\n");
#endif
			return;
		}

		newRank = atoi(CG_Argv(1));
		doMenu = atoi(CG_Argv(2));
		setTeam = atoi(CG_Argv(3));

		trap->Cvar_Set("ui_rankChange", va("%i", newRank));

		trap->Cvar_Set("ui_myteam", va("%i", setTeam));

		if (!( trap->Key_GetCatcher() & KEYCATCH_UI ) && doMenu)
		{
			trap->OpenUIMenu(UIMENU_PLAYERCONFIG);
		}

		return;
	}

	if ( !strcmp( cmd, "kg2" ) )
	{ //Kill a ghoul2 instance in this slot.
	  //If it has been occupied since this message was sent somehow, the worst that can (should) happen
	  //is the instance will have to reinit with its current info.
		int indexNum = 0;
		int argNum = trap->Cmd_Argc();
		int i = 1;
		
		if (argNum < 1)
		{
			return;
		}

		while (i < argNum)
		{
			indexNum = atoi(CG_Argv(i));

			if (cg_entities[indexNum].ghoul2 && trap->G2_HaveWeGhoul2Models(cg_entities[indexNum].ghoul2))
			{
				if (indexNum < MAX_CLIENTS)
				{ //You try to do very bad thing!
#ifdef _DEBUG
					Com_Printf("WARNING: Tried to kill a client ghoul2 instance with a kg2 command!\n");
#endif
					return;
				}

				CG_KillCEntityG2(indexNum);
			}

			i++;
		}
		
		return;
	}

	if (!strcmp(cmd, "kls"))
	{ //kill looping sounds
		int indexNum = 0;
		int argNum = trap->Cmd_Argc();
		centity_t *clent = NULL;
		centity_t *trackerent = NULL;
		
		if (argNum < 1)
		{
			assert(0);
			return;
		}

		indexNum = atoi(CG_Argv(1));

		if (indexNum != -1)
		{
			clent = &cg_entities[indexNum];
		}

		if (argNum >= 2)
		{
			indexNum = atoi(CG_Argv(2));

			if (indexNum != -1)
			{
				trackerent = &cg_entities[indexNum];
			}
		}

		if (clent)
		{
			CG_S_StopLoopingSound(clent->currentState.number, -1);
		}
		if (trackerent)
		{
			CG_S_StopLoopingSound(trackerent->currentState.number, -1);
		}

		return;
	}

	//eezstreet add
	if ( !strcmp (cmd, "aciset") )
	{
		int number = atoi(CG_Argv(1));
		cg.weaponSelect = number;
		return;
	}

	if ( !strcmp (cmd, "ieq") )
	{
	    if ( trap->Cmd_Argc() == 3 )
	    {
	        int newItem = atoi (CG_Argv (1));
	        int oldItem = atoi (CG_Argv (2));
	        
			(*cg.playerInventory)[newItem].equipped = true;
			if (oldItem != -1) {
				(*cg.playerInventory)[oldItem].equipped = false;
			}
	        uiImports->InventoryNotify( INVENTORYNOTIFY_UPDATE );
	    }
	    return;
	}
	
	if ( !strcmp (cmd, "iueq") )
	{
	    if ( trap->Cmd_Argc() == 2 )
	    {
	        int slot = atoi (CG_Argv (1));
			(*cg.playerInventory)[slot].equipped = false;
	        uiImports->InventoryNotify( INVENTORYNOTIFY_UPDATE );
	    }
	    
	    return;
	}
	if ( !strcmp (cmd, "inventory_update") )
	{
		cg.predictedPlayerState.credits = atoi(CG_Argv(1));
		uiImports->InventoryNotify (INVENTORYNOTIFY_UPDATE);
		return;
	}

	if(!strcmp(cmd, "frcaci"))
	{
		// Force ACI
		JKG_CG_FillACISlot(atoi(CG_Argv(0)), atoi(CG_Argv(1)));
		return;
	}

	if (!strcmp(cmd, "ircg"))
	{ //this means param 2 is the body index and we want to copy to bodyqueue on it
		IRCG = qtrue;
	}

	if (!strcmp(cmd, "rcg") || IRCG)
	{ //rcg - Restore Client Ghoul (make sure limbs are reattached and ragdoll state is reset - this must be done reliably)
		int indexNum = 0;
		int argNum = trap->Cmd_Argc();
		centity_t *clent;
		
		if (argNum < 1)
		{
			assert(0);
			return;
		}

		indexNum = atoi(CG_Argv(1));
		if (indexNum < 0 || indexNum >= MAX_CLIENTS)
		{
			assert(0);
			return;
		}

		clent = &cg_entities[indexNum];

		//assert(clent->ghoul2);
		if (!clent->ghoul2)
		{ //this can happen while connecting as a client
			return;
		}

#ifdef _DEBUG
		if (!trap->G2_HaveWeGhoul2Models(clent->ghoul2))
		{
			assert(!"Tried to reset state on a bad instance. Crash is inevitable.");
		}
#endif

		if (IRCG)
		{
			int bodyIndex = 0;
			int weaponIndex = 0;
			int weaponVariation = 0;
			int side = 0;
			centity_t *body;

			assert(argNum >= 4);
			bodyIndex = atoi(CG_Argv(2));
			weaponIndex = atoi(CG_Argv(3));
			weaponVariation = atoi (CG_Argv (4));
			side = atoi(CG_Argv(5));

			body = &cg_entities[bodyIndex];

			if (side)
			{
				body->teamPowerType = qtrue; //light side
			}
			else
			{
				body->teamPowerType = qfalse; //dark side
			}

			CG_BodyQueueCopy(body, clent->currentState.number, weaponIndex, weaponVariation);
		}

		//reattach any missing limbs
		if (clent->torsoBolt)
		{
			CG_ReattachLimb(clent);
		}

		//make sure ragdoll state is reset
		if (clent->isRagging)
		{
			clent->isRagging = qfalse;
			trap->G2API_SetRagDoll(clent->ghoul2, NULL); //calling with null parms resets to no ragdoll.
		}
		
		//clear all the decals as well
		trap->G2API_ClearSkinGore(clent->ghoul2);

		clent->weapon = 0;
		clent->ghoul2weapon = NULL; //force a weapon reinit

		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		char strEd[MAX_STRINGED_SV_STRING];
		CG_CheckSVStringEdRef(strEd, CG_Argv(1));
		CG_CenterPrint( strEd, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cps" ) ) {
		char strEd[MAX_STRINGED_SV_STRING];
		char *x = (char *)CG_Argv(1);
		if (x[0] == '@')
		{
			x++;
		}
		trap->SE_GetStringTextString(x, strEd, MAX_STRINGED_SV_STRING);
		CG_CenterPrint( strEd, SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	// Warzone Tickets...
	if ( !strcmp( cmd, "tkt" ) ) {
		//CG_Printf("CG_Argv(0) = %s. CG_Argv(1) = %s. CG_Argv(2) = %s. CG_Argv(3) = %s.\n", CG_Argv(0), CG_Argv(1), CG_Argv(2), CG_Argv(3));
		cgs.redtickets = atoi(CG_Argv(1));
		cgs.bluetickets = atoi(CG_Argv(2));
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		char strEd[MAX_STRINGED_SV_STRING];
		CG_CheckSVStringEdRef(strEd, CG_Argv(1));
		trap->Print( "%s", strEd );
		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		if ( !cg_teamChatsOnly.integer ) {
			trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
			Q_strncpyz( text, CG_Argv(2), sizeof (text) );
			CG_RemoveChatEscapeChar( text );
			CG_ChatBox_AddString(text, atoi(CG_Argv(1)));
			trap->Print( "*%s\n", text );
		}
		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		Q_strncpyz( text, CG_Argv(2), MAX_SAY_TEXT );
		CG_RemoveChatEscapeChar( text );
		CG_ChatBox_AddString(text, atoi(CG_Argv(1)));
		trap->Print( "*%s\n", text );

		return;
	}

	//chat with location, possibly localized.
	if ( !strcmp( cmd, "lchat" ) ) {
		if ( !cg_teamChatsOnly.integer ) {
			char name[MAX_STRING_CHARS];
			char loc[MAX_STRING_CHARS];
			char color[8];
			char message[MAX_STRING_CHARS];
			int fadeLevel;

			if (trap->Cmd_Argc() < 4)
			{
				return;
			}
			fadeLevel = atoi(CG_Argv(1));
			strcpy(name, CG_Argv(2));
			strcpy(loc, CG_Argv(3));
			strcpy(color, CG_Argv(4));
			strcpy(message, CG_Argv(5));

			if (loc[0] == '@')
			{ //get localized text
				trap->SE_GetStringTextString(loc+1, loc, MAX_STRING_CHARS);
			}

			trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
			//Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
			Com_sprintf(text, MAX_SAY_TEXT, "%s<%s>^%s%s", name, loc, color, message);
			CG_RemoveChatEscapeChar( text );
			CG_ChatBox_AddString(text, fadeLevel);
			trap->Print( "*%s\n", text );
		}
		return;
	}
	if ( !strcmp( cmd, "ltchat" ) ) {
		char name[MAX_STRING_CHARS];
		char loc[MAX_STRING_CHARS];
		char color[8];
		char message[MAX_STRING_CHARS];
		int fadeLevel;

		if (trap->Cmd_Argc() < 4)
		{
			return;
		}
		fadeLevel = atoi(CG_Argv(1));
		strcpy(name, CG_Argv(2));
		strcpy(loc, CG_Argv(3));
		strcpy(color, CG_Argv(4));
		strcpy(message, CG_Argv(5));

		if (loc[0] == '@')
		{ //get localized text
			trap->SE_GetStringTextString(loc+1, loc, MAX_STRING_CHARS);
		}

		trap->S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		//Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
		Com_sprintf(text, MAX_SAY_TEXT, "%s<%s> ^%s%s", name, loc, color, message);
		CG_RemoveChatEscapeChar( text );
		CG_ChatBox_AddString(text, fadeLevel);
		trap->Print( "*%s\n", text );

		return;
	}

	if ( !strcmp( cmd, "scores" ) ) {
		CG_ParseScores();
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		CG_ParseTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	}

	if ( !strcmp( cmd, "fmrefresh" ) ) {
		JKG_FireModeUpdate();
		return;
	}

	//[OverflowProtection]
	//this command was vulnerable to buffer overflow and could cause problems due to not properly returning 
	//after processing the command.
	if ( !strcmp( cmd, "remapShader" ) ) 
	{
		if ( trap->Cmd_Argc() == 4 )
		{
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			Q_strncpyz( shader1, CG_Argv( 1 ), sizeof( shader1 ) );
			Q_strncpyz( shader2, CG_Argv( 2 ), sizeof( shader2 ) );
			trap->R_RemapShader( shader1, shader2, CG_Argv( 3 ) );
			return;
		}
		return;
	}
	//[/OverflowProtection]

	// loaddeferred can be both a servercmd and a consolecmd
	if ( !strcmp( cmd, "loaddefered" ) ) {	// FIXME: spelled wrong, but not changing for demo
		CG_LoadDeferredPlayers();
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = qtrue;
		return;
	}

	// Team Party List
	if ( !strcmp( cmd, "tpl" ))
	{
		int i, iID, iLen;

		for ( i = 0, iLen = (( trap->Cmd_Argc() - 1 ) / 5 ); i < iLen; i++ )
		{
			iID	= atoi( CG_Argv( i * 5 + 1 ));
			cgs.partyList[iID].id = atoi( CG_Argv( i * 5 + 2 ));
			cgs.partyList[iID].classId = atoi( CG_Argv( i * 5 + 3 ));
			cgs.partyList[iID].time	= atoi( CG_Argv( i * 5 + 4 ));
			Q_strncpyz( cgs.partyList[iID].message, ( char * ) CG_Argv( i * 5 + 5 ), sizeof( cgs.partyList[iID].message ));

			if ( cgs.partyList[iID].time > cgs.partyListTime )
			{
				cgs.partyListTime = cgs.partyList[iID].time;
			}
		}
		/* Notify UI */
		uiImports->PartyMngtNotify( PARTYNOTIFY_UPDATESEEKERS );
		return;
	}

	// Team Party Invites
	if ( !strcmp( cmd, "tpi" ))
	{
		/* Can't do this with an active party o.o */
		cgs.party.active = 0;

		/* Scan the incoming string into the party struct */
		sscanf( CG_Argv( 1 ), "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
			&cgs.party.invites[0].id, &cgs.party.invites[0].leaderId, &cgs.party.invites[0].memberCount,
			&cgs.party.invites[1].id, &cgs.party.invites[1].leaderId, &cgs.party.invites[1].memberCount,
			&cgs.party.invites[2].id, &cgs.party.invites[2].leaderId, &cgs.party.invites[2].memberCount,
			&cgs.party.invites[3].id, &cgs.party.invites[3].leaderId, &cgs.party.invites[3].memberCount,
			&cgs.party.invites[4].id, &cgs.party.invites[4].leaderId, &cgs.party.invites[4].memberCount );

		/* Win cake */

		/* Notify UI */
		uiImports->PartyMngtNotify( PARTYNOTIFY_UPDATESTATE );
		return;
	}

	// Team Party Update
	if ( !strcmp( cmd, "tpu" ))
	{
		int i;

		/* Set the party status to active */
		cgs.party.active = 1;

		/* Scan the incoming string into the party struct */
		sscanf( CG_Argv( 1 ), "%i %i %i %i %i %i %i %i %i %i %i",
			&cgs.party.number,
			&cgs.party.members[0].id, &cgs.party.members[0].classId,
			&cgs.party.members[1].id, &cgs.party.members[1].classId,
			&cgs.party.members[2].id, &cgs.party.members[2].classId,
			&cgs.party.members[3].id, &cgs.party.members[3].classId,
			&cgs.party.members[4].id, &cgs.party.members[4].classId );

		/* Parse the members and fix the status and ID's! */
		for ( i = 0; i < 5; i++ )
		{
			if ( cgs.party.members[i].id < 0 )
			{
				cgs.party.members[i].id = abs( cgs.party.members[i].id + 1 );
				cgs.party.members[i].status = -1;
			}
			else
			{
				cgs.party.members[i].status = 0;
			}
		}

		/* Set the party leader status accordingly */
		cgs.party.members[0].status = 1;

		/* Notify UI */
		uiImports->PartyMngtNotify( PARTYNOTIFY_UPDATESTATE );
		return;
	}

	if( !strcmp( cmd, "pInv" ))
	{
		BG_ReceivedItemPacket(BG_ItemPacketFromName(CG_Argv(1)));
		uiImports->ItemsUpdated();
		return;
	}

	if (!strcmp(cmd, "pTrade"))
	{
		BG_ReceivedTradePacket(BG_TradePacketFromName(CG_Argv(1)));
		uiImports->ItemsUpdated();
		return;
	}

	// UQ1: Use an event!!!!
	// eez: Again, this is only getting sent to one client, so no go
	if( !strcmp( cmd, "hitmarker") )
	{
		// All this does is make a hitmarker display. Nothing too fancy.
		trap->S_StartSound(NULL, cg.clientNum, CHAN_AUTO, cgs.media.hitmarkerSound);
		cg.hitmarkerLastTime = cg.time + 1000;
		return;
	}

	if( !strcmp( cmd, "notify") )
	{
		// add a notification to the display
		CG_Notifications_Add((char *)CG_Argv(2), qfalse);	// first arg is ignored. it's supposed to specify the type of message but it's unused.
		return;
	}

	if (!strcmp(cmd, "apc"))
	{
		// Ammo price check response
		uiImports->InventoryPriceCheckResult(atoi(CG_Argv(1)), atoi(CG_Argv(2)));
		return;
	}

	if (!strcmp(cmd, "cbi"))
	{
		// Client bought item
		char* playerName = va("%s", CG_Argv(2));
		for (int i = 3; i < trap->Cmd_Argc(); i++)
		{
			playerName = va("%s %s", playerName, CG_Argv(i));
		}
		JKG_ClientBoughtItem(playerName, atoi(CG_Argv(1)));
		return;
	}
	//eezstreet end

	trap->Print( "Unknown client game command: %s\n", cmd );
}

/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap->GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
