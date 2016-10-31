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

#include "g_local.h"
#include "bg_saga.h"
#include "jkg_gangwars.h"
#include "../ui/menudef.h"			// for the voice chats
#include "../GLua/glua.h"
#include "jkg_chatcmds.h"
#include "jkg_utilityfunc.h"
#include "jkg_treasureclass.h"

//rww - for getting bot commands...
int AcceptBotCommand(char *cmd, gentity_t *pl);
//end rww

void Cmd_NPC_f( gentity_t *ent );

extern void AIMod_CheckMapPaths ( gentity_t *ent );
extern void AIMod_CheckObjectivePaths ( gentity_t *ent );

extern void WARZONE_SaveGameInfo ( void );
extern void WARZONE_LoadGameInfo ( void );
extern void WARZONE_AddHealthCrate ( vec3_t origin, vec3_t angles );
extern void WARZONE_AddAmmoCrate ( vec3_t origin, vec3_t angles );
extern void WARZONE_AddFlag ( vec3_t origin, int team );
extern void WARZONE_ChangeFlagTeam ( int flag_number, int new_team );
extern void WARZONE_RaiseAllWarzoneEnts ( int modifier );
extern void WARZONE_ShowInfo ( void );
extern void WARZONE_RemoveAllWarzoneEnts ( void );
extern void WARZONE_RemoveAmmoCrate ( void );
extern void WARZONE_RemoveHealthCrate ( void );

/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	if ( !sv_cheats.integer && !ent->client->sess.canUseCheats ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return qfalse;
	}
	if ( ent->health <= 0 || (ent->client && ent->client->deathcamTime) ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		return qfalse;
	}
	return qtrue;
}

/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = trap->Argc();
	for ( i = start ; i < c ; i++ ) {
		trap->Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
StringIsInteger
==================
*/
qboolean StringIsInteger( const char *s ) {
	int			i=0, len=0;
	qboolean	foundDigit=qfalse;

	for ( i=0, len=strlen( s ); i<len; i++ )
	{
		if ( !isdigit( s[i] ) )
			return qfalse;

		foundDigit = qtrue;
	}

	return foundDigit;
}

/*
==================
ClientNumberFromString
Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, const char *s, qboolean allowconnecting ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanInput[MAX_NETNAME];

	if ( StringIsInteger( s ) )
	{// numeric values could be slot numbers
		idnum = atoi( s );
		if ( idnum >= 0 && idnum < level.maxclients )
		{
			cl = &level.clients[idnum];
			if ( cl->pers.connected == CON_CONNECTED )
				return idnum;
			else if ( allowconnecting && cl->pers.connected == CON_CONNECTING )
				return idnum;
		}
	}

	Q_strncpyz( cleanInput, s, sizeof(cleanInput) );
	Q_StripColor( cleanInput );

	for ( idnum=0,cl=level.clients; idnum < level.maxclients; idnum++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			if ( !allowconnecting || cl->pers.connected < CON_CONNECTING )
				continue;

		if ( !Q_stricmp( cl->pers.netname_nocolor, cleanInput ) )
			return idnum;
	}

	trap->SendServerCommand( to-g_entities, va( "print \"User %s is not on the server\n\"", s ) );
	return -1;
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
#ifndef __MMO__ 
	// UQ1: This is a very spammy one!
	// Suggest, use an event for each player. Staggered over time. 
	// Scores don't need to be instant, and in future maybe not even needed until after a match.
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	gclient_t	*cl;
	int			numSorted, scoreFlags;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;
	
	if (numSorted > MAX_CLIENT_SCORE_SEND)
	{
		numSorted = MAX_CLIENT_SCORE_SEND;
	}

	for (i=0 ; i < numSorted ; i++) {
		int		ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) {
			ping = -1;
		} else {
			ping = cl->ps.ping;
		}

		// FIXME
		Com_sprintf (entry, sizeof(entry),
			" %i %i %i %i %i %i", level.sortedClients[i],
			cl->sess.sessionTeam == TEAM_SPECTATOR ? 0 : cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime)/60000,
			scoreFlags, g_entities[level.sortedClients[i]].s.powerups);
		j = strlen(entry);
		if (stringlength + j > 1022)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap->SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i, 
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
#endif //__MMO__
}

/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	DeathmatchScoreboardMessage( ent );
}

/*
==================
JKG_ItemLookup_f

(eezstreet add)
Looks up a range of items and prints them in a list.
Ideally a GM or admin-only command
==================
*/
extern itemData_t itemLookupTable[MAX_ITEM_TABLE_SIZE];
void JKG_ItemLookup_f(gentity_t *ent)
{
	unsigned int i;
	int badItems = 0;
	int goodItems = 0;
	char buffer[64] = {0};
	char lookupString[1024] = {0};
	int maximum, minimum;

	if (trap->Argc() < 3) {
		trap->SendServerCommand(ent-g_entities, "print \"format: itemLookup <minimum> <maximum>\n\"");
		return;
	}

	//Grab our args.
	trap->Argv(1, buffer, 64);
	minimum = atoi(buffer);
	trap->Argv(2, buffer, 64);
	maximum = atoi(buffer);

	if(minimum > maximum)
	{ //Oops, mixed the args most likely.
		trap->SendServerCommand(ent-g_entities, "print \"Minimum must be less than maximum.\n\"");
		return;
	}
	if((maximum - minimum) > 20)
	{ //Huge range.
		trap->SendServerCommand(ent-g_entities, "print \"Maximum range of itemLookup is 20.\n\"");
		return;
	}

	lookupString[0] = '\0';
	for(i = (unsigned int)minimum; i < (unsigned int)maximum; i++)
	{
		if(itemLookupTable[i].itemID)
		{
			if(i < maximum-1)
			{
				Q_strcat(lookupString, 960, va("%s (%u),", itemLookupTable[i].displayName, i));
			}
			else
			{
				Q_strcat(lookupString, 960, va("%s (%u)\n", itemLookupTable[i].displayName, i));
			}
			goodItems++;
		}
		else
		{
			badItems++;
		}
	}
	if(!goodItems)
	{
		trap->SendServerCommand(ent-g_entities, "print \"No valid items in that range.\n\"");
		return;
	}
	else
	{
		trap->SendServerCommand(ent-g_entities, va("print \"%s\"\n", lookupString));
		if(badItems)
		{
			trap->SendServerCommand(ent-g_entities, va("print \"%i bad indices\n\"", badItems));
		}
	}
}

/*
==================
JKG_ItemCheck_f

Gives details about an item
==================
*/
void JKG_ItemCheck_f(gentity_t *ent)
{
	char buffer[64];
	int itemNum;
	int i;

	if(trap->Argc() < 2) {
		trap->SendServerCommand(ent-g_entities, "print \"usage: /itemcheck <item ID or internal>\"\n");
		return;
	}

	trap->Argv(1, buffer, sizeof(buffer));

	if(StringIsInteger(buffer))
	{
		itemNum = atoi(buffer);
		if(itemLookupTable[itemNum].itemID)
		{
			if(itemLookupTable[itemNum].itemType == ITEM_ARMOR)
			{
				//Meant to put some sort of check for armor type, but didn't bother.
				trap->SendServerCommand(ent-g_entities, va("print \"%s (%i) - Armor\n\"", itemLookupTable[itemNum].displayName, itemNum));
				return;
			}
			else if(itemLookupTable[itemNum].itemType == ITEM_WEAPON)
			{
				trap->SendServerCommand(ent-g_entities, va("print \"%s (%i) - Weapon (w %i, v %i)\n\"", itemLookupTable[itemNum].displayName, itemNum,
					itemLookupTable[itemNum].weaponData.weapon, itemLookupTable[itemNum].weaponData.variation));
				return;
			}
			else
			{
				trap->SendServerCommand(ent-g_entities, va("print \"%s (%i) - Other\n\"", itemLookupTable[itemNum].displayName, itemNum));
				return;
			}
		}
	}
	else
	{
		for(i = 0; i < MAX_ITEM_TABLE_SIZE; i++)
		{
			if(itemLookupTable[i].itemID)
			{
				if(!Q_stricmp(itemLookupTable[i].displayName, buffer))
				{
					trap->SendServerCommand(ent-g_entities, va("print \"%s (%i)\n\"", itemLookupTable[i].displayName, i));
					return;
				}
			}
		}
	}
	trap->SendServerCommand(ent-g_entities, va("print \"%s refers to an item that does not exist!\n\"", buffer));
}


/*
==================
JKG_BuyItem_f

==================
*/
void JKG_BuyItem_f(gentity_t *ent)
{
	gentity_t* trader = ent->client->currentTrader;
	if(trap->Argc() < 1)
	{
		trap->SendServerCommand(ent->s.number, "print \"Not enough args.\n\"");
		return;
	}

	if(trader == NULL)
	{
		trap->SendServerCommand(ent->s.number, "print \"You are not at a vendor.\n\"");
		return;
	}

	char buffer[8];

	trap->Argv (1, buffer, sizeof(buffer));
	int item = atoi(buffer);
	if (item < 0 || item >= trader->inventory->size()) {
		trap->SendServerCommand(ent - g_entities, "print \"Invalid item.\n\"");
		return;
	}

	itemInstance_t* pItem = &(*trader->inventory)[item];
	if (pItem->id->baseCost > ent->client->ps.credits) 
	{
		trap->SendServerCommand(ent - g_entities, "print \"You do not have enough credits to purchase that item.\n\"");
		
		//select random unhappy vendor sound to play
		std::string snd;
		switch (Q_irand(0, 5))
		{	case 0: snd = "sound/vendor/generic/purchasefail00.mp3";
				break;
			case 1: snd = "sound/vendor/generic/purchasefail01.mp3";
				break;
			case 2: snd = "sound/vendor/generic/purchasefail02.mp3";
				break;
			case 3: snd = "sound/vendor/generic/purchasefail03.mp3";
				break;
			case 4: snd = "sound/vendor/generic/purchasefail04.mp3";
				break;
			case 5: snd = "sound/vendor/generic/purchasefail05.mp3";
				break;
			default: snd = "sound/vendor/generic/purchasefail00.mp3";
				break;
		}
		G_Sound(trader, CHAN_AUTO, G_SoundIndex(snd.c_str()));	//play sound
		return;
	}

	BG_SendTradePacket(IPT_TRADESINGLE, ent, trader, pItem, pItem->id->baseCost, 0);
	BG_GiveItemNonNetworked(ent, *pItem);
	ent->client->ps.credits -= pItem->id->baseCost;
}

/*
==================
JKG_CloseVendor_f

==================
*/
void JKG_CloseVendor_f (gentity_t *ent)
{
	if ( ent->client->currentTrader == NULL )
	{
		return;
	}

	ent->client->pmnomove = false;
	ent->client->currentTrader->genericValue1 = ENTITYNUM_NONE;
	ent->client->currentTrader = NULL;
}

/*
==================
JKG_Crystal_f

Sets the saber crystal
==================
*/
extern unsigned int numLoadedCrystals;
void JKG_Crystal_f ( gentity_t *ent, int saberNum )
{
	char *args = ConcatArgs(1);
	int argNum = atoi(args);

	/* Temporary -- We don't need cheats for right now in order to activate the saber system color stuff */
	/*if(!CheatsOk(ent))
	{
		return;
	}*/

	if(argNum)
	{
		if(argNum < 0)
		{
			return;
		}
		else if(argNum >= numLoadedCrystals)
		{
			return;
		}
		else
		{
			ent->s.saberCrystal[saberNum] = argNum;
			ent->client->ps.saberCrystal[saberNum] = argNum;
		}
	}
	else
	{
		const saberCrystalData_t *saber = JKG_GetSaberCrystal(args);
		if(!saber)
		{
			return;
		}
		else
		{
			ent->s.saberCrystal[saberNum] = saber->crystalID;
			ent->client->ps.saberCrystal[saberNum] = saber->crystalID;
		}
	}
}

/*
==================
JKG_PrintWeaponList_f

Good for testing desync
==================
*/

void JKG_PrintWeaponList_f( gentity_t *ent )
{
	BG_PrintWeaponList();
}

/*
==================
JKG_DumpWeaponList_f

Good for testing desync
==================
*/

void JKG_DumpWeaponList_f( gentity_t *ent )
{
	BG_DumpWeaponList("svweaponlist.txt");
}

/*
==================
JKG_CheckIfNumber

Loops through a string and returns the following:
0 if neither decimal nor alpha
1 if alpha
2 if decimal
==================
*/
typedef enum
{
	JKGSTR_NONE,		// unknown string type
	JKGSTR_ALPHA,		// string is plaintext/not a number
	JKGSTR_DECIMAL		// string is a stringized number
} JKGStringType_t;

JKGStringType_t JKG_CheckIfNumber(const char *string)
{
	int i = 0;
	while(string[i] != '\0')
	{
		if(isalpha((int)string[i]))
			return JKGSTR_ALPHA;
		else if(!isdigit((int)string[i]) && string[i] != '.')
			return JKGSTR_NONE;
		i++;
	}
	return JKGSTR_DECIMAL;
}

int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll );
int G_ClientNumberFromArg ( const char* name)
{
	int client_id = 0;
	char *cp;
	
	cp = (char *)name;
	while (*cp)
	{
		if ( *cp >= '0' && *cp <= '9' ) cp++;
		else
		{
			client_id = -1; //mark as alphanumeric
			break;
		}
	}

	if ( client_id == 0 )
	{ // arg is assumed to be client number
		client_id = atoi(name);
	}
	// arg is client name
	if ( client_id == -1 )
	{
		client_id = G_ClientNumberFromStrippedSubstring(name, qfalse);
	}
	return client_id;
}

/*
==================
Callvote Functionality
==================
*/

qboolean G_VoteFraglimit(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	int n = Com_Clampi(0, 0x7FFFFFFF, atoi(arg2));
	Com_sprintf(level.voteString, sizeof(level.voteString), "%s %i", arg1, n);
	Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "%s", level.voteString);
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}

qboolean G_VoteKick(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	int clientid = ClientNumberFromString(ent, arg2, qtrue);
	gentity_t *target = NULL;
	if (clientid == -1)
		return qfalse;
	target = &g_entities[clientid];
	if (!target || !target->inuse || !target->client)
		return qfalse;
	Com_sprintf(level.voteString, sizeof(level.voteString), "clientkick %d", clientid);
	Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", target->client->pers.netname);
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}

const char *G_GetArenaInfoByMap(const char *map);
void Cmd_MapList_f(gentity_t *ent) {
	int i, toggle = 0;
	char map[24] = "--", buf[512] = { 0 };
	Q_strcat(buf, sizeof(buf), "Map list:");
	for (i = 0; i<level.arenas.num; i++) {
		Q_strncpyz(map, Info_ValueForKey(level.arenas.infos[i], "map"), sizeof(map));
		Q_StripColor(map);
		if (G_DoesMapSupportGametype(map, level.gametype)) {
			char *tmpMsg = va(" ^%c%s", (++toggle & 1) ? COLOR_GREEN : COLOR_YELLOW, map);
			if (strlen(buf) + strlen(tmpMsg) >= sizeof(buf)) {
				trap->SendServerCommand(ent - g_entities, va("print \"%s\"", buf));
				buf[0] = '\0';
			}
			Q_strcat(buf, sizeof(buf), tmpMsg);
		}
	}
	trap->SendServerCommand(ent - g_entities, va("print \"%s\n\"", buf));
}
qboolean G_VoteMap(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	char s[MAX_CVAR_VALUE_STRING] = { 0 }, bspName[MAX_QPATH] = { 0 }, *mapName = NULL, *mapName2 = NULL;
	fileHandle_t fp = NULL_FILE;
	const char *arenaInfo;
	// didn't specify a map, show available maps
	if (numArgs < 3) {
		Cmd_MapList_f(ent);
		return qfalse;
	}
	if (strchr(arg2, '\\')) {
		trap->SendServerCommand(ent - g_entities, "print \"Can't have mapnames with a \\\n\"");
		return qfalse;
	}
	Com_sprintf(bspName, sizeof(bspName), "maps/%s.bsp", arg2);
	if (trap->FS_Open(bspName, &fp, FS_READ) <= 0) {
		trap->SendServerCommand(ent - g_entities, va("print \"Can't find map %s on server\n\"", bspName));
		if (fp != NULL_FILE)
			trap->FS_Close(fp);
		return qfalse;
	}
	trap->FS_Close(fp);
	if (!G_DoesMapSupportGametype(arg2, level.gametype)) {
		trap->SendServerCommand(ent - g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")));
		return qfalse;
	}
	// preserve the map rotation
	trap->Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
	if (*s)
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s; set nextmap \"%s\"", arg1, arg2, s);
	else
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
	arenaInfo = G_GetArenaInfoByMap(arg2);
	if (arenaInfo) {
		mapName = Info_ValueForKey(arenaInfo, "longname");
		mapName2 = Info_ValueForKey(arenaInfo, "map");
	}
	if (!mapName || !mapName[0])
		mapName = "ERROR";
	if (!mapName2 || !mapName2[0])
		mapName2 = "ERROR";
	Com_sprintf(level.voteDisplayString, sizeof(level.voteDisplayString), "map %s (%s)", mapName, mapName2);
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}
qboolean G_VoteMapRestart(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	int n = Com_Clampi(0, 60, atoi(arg2));
	if (numArgs < 3)
		n = 5;
	Com_sprintf(level.voteString, sizeof(level.voteString), "%s %i", arg1, n);
	Q_strncpyz(level.voteDisplayString, level.voteString, sizeof(level.voteDisplayString));
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}

qboolean G_VoteNextmap(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	char s[MAX_CVAR_VALUE_STRING];
	trap->Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
	if (!*s) {
		trap->SendServerCommand(ent - g_entities, "print \"nextmap not set.\n\"");
		return qfalse;
	}
	Com_sprintf(level.voteString, sizeof(level.voteString), "vstr nextmap");
	Q_strncpyz(level.voteDisplayString, level.voteString, sizeof(level.voteDisplayString));
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}

qboolean G_VoteTimelimit(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	float tl = Com_Clamp(0.0f, 35790.0f, atof(arg2));
	if (Q_isintegral(tl))
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %i", arg1, (int)tl);
	else
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %.3f", arg1, tl);
	Q_strncpyz(level.voteDisplayString, level.voteString, sizeof(level.voteDisplayString));
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}
qboolean G_VoteWarmup(gentity_t *ent, int numArgs, const char *arg1, const char *arg2) {
	int n = Com_Clampi(0, 1, atoi(arg2));
	Com_sprintf(level.voteString, sizeof(level.voteString), "%s %i", arg1, n);
	Q_strncpyz(level.voteDisplayString, level.voteString, sizeof(level.voteDisplayString));
	Q_strncpyz(level.voteStringClean, level.voteString, sizeof(level.voteStringClean));
	return qtrue;
}
typedef struct voteString_s {
	const char	*string;
	const char	*aliases; // space delimited list of aliases, will always show the real vote string
	qboolean(*func)(gentity_t *ent, int numArgs, const char *arg1, const char *arg2);
	int	numArgs; // number of REQUIRED arguments, not total/optional arguments
	uint32_t	validGT; // bit-flag of valid gametypes
	qboolean voteDelay; // if true, will delay executing the vote string after it's accepted by g_voteDelay
	const char	*shortHelp; // NULL if no arguments needed
} voteString_t;

static voteString_t validVoteStrings[] = {
	// vote string aliases # args valid gametypes exec delay short help
	{ "fraglimit", "frags", G_VoteFraglimit, 1, GTB_ALL, qtrue, "<num>" },
	{ "kick", NULL, G_VoteKick, 1, GTB_ALL, qfalse, "<client name>" },
	{ "map", NULL, G_VoteMap, 0, GTB_ALL, qtrue, "<name>" },
	{ "map_restart", "restart", G_VoteMapRestart, 0, GTB_ALL, qtrue, "<optional delay>" },
	{ "nextmap", NULL, G_VoteNextmap, 0, GTB_ALL, qtrue, NULL },
	{ "timelimit", "time", G_VoteTimelimit, 1, GTB_ALL, qtrue, "<num>" },
};
static const int validVoteStringsSize = ARRAY_LEN(validVoteStrings);

void Svcmd_ToggleAllowVote_f(void) {
	if (trap->Argc() == 1) {
		int i = 0;
		for (i = 0; i<validVoteStringsSize; i++) {
			if ((g_allowVote.integer & (1 << i)))	trap->Print("%2d [X] %s\n", i, validVoteStrings[i].string);
			else									trap->Print("%2d [ ] %s\n", i, validVoteStrings[i].string);
		}
		return;
	}
	else {
		char arg[8] = { 0 };
		int index;

		trap->Argv(1, arg, sizeof(arg));
		index = atoi(arg);

		if (index < 0 || index >= validVoteStringsSize) {
			Com_Printf("ToggleAllowVote: Invalid range: %i [0, %i]\n", index, validVoteStringsSize - 1);
			return;
		}

		trap->Cvar_Set("g_allowVote", va("%i", (1 << index) ^ (g_allowVote.integer & ((1 << validVoteStringsSize) - 1))));
		trap->Cvar_Update(&g_allowVote);

		Com_Printf("%s %s^7\n", validVoteStrings[index].string, ((g_allowVote.integer & (1 << index)) ? "^2Enabled" : "^1Disabled"));
	}
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *cmdent, int baseArg)
{
	char		name[MAX_TOKEN_CHARS];
	gentity_t	*ent;
	gitem_t		*it;
	int			i;
	qboolean	give_all;
	gentity_t		*it_ent;
	trace_t		trace;
	char		arg[MAX_TOKEN_CHARS];

	if ( !CheatsOk( cmdent ) ) {
		return;
	}

	if (baseArg)
	{
		char otherindex[MAX_TOKEN_CHARS];

		trap->Argv( 1, otherindex, sizeof( otherindex ) );

		if (!otherindex[0])
		{
			Com_Printf("giveother requires that the second argument be a client index number.\n");
			return;
		}

		i = atoi(otherindex);

		if (i < 0 || i >= MAX_CLIENTS)
		{
			Com_Printf("%i is not a client index\n", i);
			return;
		}

		ent = &g_entities[i];

		if (!ent->inuse || !ent->client)
		{
			Com_Printf("%i is not an active client\n", i);
			return;
		}
	}
	else
	{
		ent = cmdent;
	}

	trap->Argv( 1+baseArg, name, sizeof( name ) );

	if (Q_stricmp(name, "all") == 0)
		give_all = qtrue;
	else
		give_all = qfalse;

	if (give_all)
	{
		i = 0;
		while (i < HI_NUM_HOLDABLE)
		{
			ent->client->ps.stats[STAT_HOLDABLE_ITEMS] |= (1 << i);
			i++;
		}
		i = 0;
	}

	// Pretty much to test out the awesome seeker upgrades.
	if ( Q_stricmp( name, "seeker" ) == 0 )
	{
		char time[1024];
		trap->Argv( 2 + baseArg, time, sizeof( time ));
		ItemUse_Seeker( cmdent );
		if ( time[0] ) cmdent->client->ps.droneExistTime = level.time + ( atoi( time ) * 1000 );
		else cmdent->client->ps.droneExistTime = level.time + 60000;
	}

	if (give_all || Q_stricmp( name, "health") == 0)
	{
		if (trap->Argc() == 3+baseArg) {
			trap->Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->health = atoi(arg);
			if (ent->health > ent->client->ps.stats[STAT_MAX_HEALTH]) {
				ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
			}
		}
		else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		ent->client->ps.stats[STAT_WEAPONS] = (1 << (LAST_USEABLE_WEAPON+1))  - ( 1 << WP_NONE );
		if (!give_all)
			return;
	}
	
	if ( !give_all && Q_stricmp(name, "weaponnum") == 0 )
	{
		trap->Argv( 2+baseArg, arg, sizeof( arg ) );
		ent->client->ps.stats[STAT_WEAPONS] |= (1 << atoi(arg));
		return;
	}
	
	if ( !give_all && Q_stricmp (name, "weapon") == 0 )
	{
	    weaponData_t *weapon;
	    
	    trap->Argv (2 + baseArg, arg, sizeof (arg));
	    weapon = BG_GetWeaponByClassName (arg);
	    if ( weapon )
	    {
	        int i = 0;
			int itemID;
	        
			//FIXME: The below assumes that there is a valid weapon item
			itemID = BG_GetItemByWeaponIndex(BG_GetWeaponIndex((unsigned int)weapon->weaponBaseIndex, (unsigned int)weapon->weaponModIndex))->itemID;

			itemInstance_t item = BG_ItemInstance(itemID, 1);
			BG_GiveItem(ent, item);
			trap->SendServerCommand (ent->s.number, va ("print \"'%s' was added to your inventory.\n\"", itemLookupTable[itemID].displayName));
			
			// UQ1: Added - update their ACI...
			trap->SendServerCommand(ent->s.number, va("AddToACI %i", itemID));
	    }
	    else
	    {
	        trap->SendServerCommand (ent->s.number, va ("print \"'%s' does not exist.\n\"", arg));
	    }
	    return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		int num = 999;
		if (trap->Argc() == 3+baseArg) {
			trap->Argv( 2+baseArg, arg, sizeof( arg ) );
			num = atoi(arg);
		}
		for ( i = 0 ; i < JKG_MAX_AMMO_INDICES ; i++ ) {
			ent->client->ammoTable[i] = num;		//FIXME: copy to proper ammo array
		}
		for ( i = 0; i <= 255; i++ )
		{
			int weapVar, weapBase;
			if(!BG_GetWeaponByIndex(i, &weapBase, &weapVar))
			{
				break;
			}
			ent->client->clipammo[i] = GetWeaponAmmoClip (weapBase, weapVar);
		}
		ent->client->ps.ammo = num;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		if (trap->Argc() == 3+baseArg) {
			trap->Argv( 2+baseArg, arg, sizeof( arg ) );
			ent->client->ps.stats[STAT_ARMOR] = atoi(arg);
		} else {
			ent->client->ps.stats[STAT_ARMOR] = ent->client->ps.stats[STAT_MAX_ARMOR];
		}

		if (!give_all)
			return;
	}

	//Inventory items -- eezstreet/JKG
	if(!give_all && Q_stricmp(name, "item") == 0)
	{
		int itemID = 0, j = 0;
		qboolean inventoryFull = qtrue;

		trap->Argv(2+baseArg, arg, sizeof( arg ) );
		itemID = atoi(arg);

		if(itemID)
		{
			if(!itemLookupTable[itemID].itemID)
			{
				trap->SendServerCommand(ent - g_entities, va("print \"%i refers to an item that does not exist\n\"", itemID));
				return;
			}
			itemInstance_t item = BG_ItemInstance(itemID, 1);
			BG_GiveItem(ent, item);
		}
		else
		{
			itemInstance_t item = BG_ItemInstance(arg, 1);
			if (!item.id) {
				trap->SendServerCommand(ent - g_entities, va("print \"%s refers to an item that does not exist\n\"", arg));
				return;
			}
			BG_GiveItem(ent, item);
		}
		return;
	}

	if (Q_stricmp(name, "credits") == 0 || Q_stricmp(name, "credit") == 0) {
		int creditAmount;
		trap->Argv(2+baseArg, arg, sizeof( arg ) );

		creditAmount = atoi(arg);
		ent->client->ps.credits += creditAmount;
		int credits = ent->client->ps.credits;
		trap->SendServerCommand( ent->client->ps.clientNum, va("print \"Your new balance is: %i credits\n\"", Q_max (0, credits)) );
		return;
	}

	if ( give_all )
	{
		ent->client->ps.cloakFuel	= 100;
		ent->client->ps.jetpackFuel	= 100;
		ent->client->ps.credits = 32000; // FIXME
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem (name);
		if (!it) {
			return;
		}

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem (it_ent, it);
			if ( !it_ent || !it_ent->inuse )
			return;
		FinishSpawningItem(it_ent );
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item (it_ent, ent, &trace);
		if (it_ent->inuse) {
			G_FreeEntity( it_ent );
		}
	}
}

/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f( gentity_t *ent ) {
	char *msg = NULL;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_GODMODE;
	if ( !(ent->flags & FL_GODMODE) )
		msg = "godmode OFF";
	else
		msg = "godmode ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
	char *msg = NULL;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if ( !(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF";
	else
		msg = "notarget ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char *msg = NULL;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->client->noclip = !ent->client->noclip;
	if ( !ent->client->noclip )
		msg = "noclip OFF";
	else
		msg = "noclip ON";

	trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", msg ) );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent )
{
	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( !ent->client->pers.localClient )
	{
		trap->SendServerCommand(ent-g_entities, "print \"The levelshot command must be executed by a local client\n\"");
		return;
	}

	// doesn't work in single player
	if ( level.gametype == GT_SINGLE_PLAYER )
	{
		trap->SendServerCommand(ent-g_entities, "print \"Must not be in singleplayer mode for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	trap->SendServerCommand( ent-g_entities, "clientLevelShot" );
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if ( ent->client->tempSpectate > level.time) {
		return;		// Cant /kill in tempspec
	}

	if (ent->health <= 0) {		//awww sad, zombies can't commit suicide
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die (ent, ent, ent, 100000, MOD_SUICIDE);
}


/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )														
{
	client->ps.fd.forceDoInit = 1; //every time we change teams make sure our force powers are set right

	if ( client->sess.sessionTeam == TEAM_RED ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString2(bgGangWarsTeams[level.redTeam].joinstring)) );
			//client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEREDTEAM")) );
	} else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
			client->pers.netname, G_GetStringEdString2(bgGangWarsTeams[level.blueTeam].joinstring)) );
		//client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBLUETEAM")));
	} else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHESPECTATORS")));		
	} else if ( client->sess.sessionTeam == TEAM_FREE ) {
		trap->SendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " %s\n\"",
		client->pers.netname, G_GetStringEdString("MP_SVGAME", "JOINEDTHEBATTLE")));
	}

	G_LogPrintf ( "setteam:  %i %s %s\n",
				  client - &level.clients[0],
				  TeamName ( oldTeam ),
				  TeamName ( client->sess.sessionTeam ) );
}

qboolean G_PowerDuelCheckFail(gentity_t *ent)
{
	int			loners = 0;
	int			doubles = 0;

	if (!ent->client || ent->client->sess.duelTeam == DUELTEAM_FREE)
	{
		return qtrue;
	}

	G_PowerDuelCount(&loners, &doubles, qfalse);

	if (ent->client->sess.duelTeam == DUELTEAM_LONE && loners >= 1)
	{
		return qtrue;
	}

	if (ent->client->sess.duelTeam == DUELTEAM_DOUBLE && doubles >= 2)
	{
		return qtrue;
	}

	return qfalse;
}

/*
=================
SetTeam
=================
*/
qboolean g_dontPenalizeTeam = qfalse;
qboolean g_preventTeamBegin = qfalse;
void SetTeam( gentity_t *ent, char *s ) {
	int					team, oldTeam;
	gclient_t			*client;
	int					clientNum;
	spectatorState_t	specState;
	int					specClient;
	int					teamLeader;

	// fix: this prevents rare creation of invalid players
	if (!ent->inuse)
	{
		return;
	}

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;
	specState = SPECTATOR_NOT;
	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( !Q_stricmp( s, "follow1" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -1;
	} else if ( !Q_stricmp( s, "follow2" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FOLLOW;
		specClient = -2;
	} else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
		team = TEAM_SPECTATOR;
		specState = SPECTATOR_FREE;
	} else if ( level.gametype >= GT_TEAM ) {
		// if running a team game, assign player to one of the teams
		specState = SPECTATOR_NOT;
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) {
			team = TEAM_RED;
		} else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) {
			team = TEAM_BLUE;
		} else {
			// pick the team with the least number of players
			//For now, don't do this. The legalize function will set powers properly now.
			/*
			if (g_forceBasedTeams.integer)
			{
				if (ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					team = TEAM_BLUE;
				}
				else
				{
					team = TEAM_RED;
				}
			}
			else
			{
			*/
				team = PickTeam( clientNum );
			//}
		}

#ifdef __JKG_NINELIVES__
#ifdef __JKG_TICKETING__
#ifdef __JKG_ROUNDBASED__
		// Can't switch teams to anything but Spectator if the round is not ready
		if(level.time - level.gamestartTime > 300000 &&
			g_gametype.integer >= GT_LMS_NINELIVES &&
			g_gametype.integer <= GT_LMS_ROUNDS)
		{
			// Restrict joining the game if the game is beyond 5 minutes time
			if( team != TEAM_SPECTATOR )
			{
				return;
			}
		}
#endif
#endif
#endif

		if ( g_teamForceBalance.integer ) {
			int		counts[TEAM_NUM_TEAMS];

			//JAC: Invalid clientNum was being used
			counts[TEAM_BLUE] = TeamCount( ent-g_entities, TEAM_BLUE );
			counts[TEAM_RED] = TeamCount( ent-g_entities, TEAM_RED );

			// We allow a spread of two
			if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_DARKSIDE)
				{
					trap->SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED_SWITCH")) );
				}
				else
				*/
				{
					//JAC: Invalid clientNum was being used
					trap->SendServerCommand( ent-g_entities, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				}
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//For now, don't do this. The legalize function will set powers properly now.
				/*
				if (g_forceBasedTeams.integer && ent->client->ps.fd.forceSide == FORCE_LIGHTSIDE)
				{
					trap->SendServerCommand( ent->client->ps.clientNum, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE_SWITCH")) );
				}
				else
				*/
				{
					//JAC: Invalid clientNum was being used
					trap->SendServerCommand( ent-g_entities, 
						va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				}
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}

		//For now, don't do this. The legalize function will set powers properly now.
		/*
		if (g_forceBasedTeams.integer)
		{
			if (team == TEAM_BLUE && ent->client->ps.fd.forceSide != FORCE_LIGHTSIDE)
			{
				trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBELIGHT")) );
				return;
			}
			if (team == TEAM_RED && ent->client->ps.fd.forceSide != FORCE_DARKSIDE)
			{
				trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEDARK")) );
				return;
			}
		}
		*/

	} else {
		// force them to spectators if there aren't any spots free
		team = TEAM_FREE;
	}

	// override decision if limiting the players
	if ( (level.gametype == GT_DUEL)
		&& level.numNonSpectatorClients >= 2 )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( (level.gametype == GT_POWERDUEL)
		&& (level.numPlayingClients >= 3 || G_PowerDuelCheckFail(ent)) )
	{
		team = TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && 
		level.numNonSpectatorClients >= g_maxGameClients.integer )
	{
		team = TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return;
	}

	//
	// execute the team change
	//

	// if the player was dead leave the body
	if ( client->ps.stats[STAT_HEALTH] <= 0 && client->sess.sessionTeam != TEAM_SPECTATOR ) {
		MaintainBodyQueue(ent);
	}

	// he starts at 'base'
	client->pers.teamState.state = TEAM_BEGIN;
	if ( oldTeam != TEAM_SPECTATOR ) {
		// Kill him (makes sure he loses flags, etc)
		ent->flags &= ~FL_GODMODE;
		ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
		g_dontPenalizeTeam = qtrue;
		player_die (ent, ent, ent, 100000, MOD_SUICIDE);
		g_dontPenalizeTeam = qfalse;

	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR && oldTeam != team )
		AddTournamentQueue( client );

	// clear votes if going to spectator (specs can't vote)
	if ( team == TEAM_SPECTATOR )
		G_ClearVote( ent );
	// also clear team votes if switching red/blue or going to spec
	G_ClearTeamVote( ent, oldTeam );

	client->sess.sessionTeam = (team_t)team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;

	client->sess.teamLeader = qfalse;
	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(g_entities[clientNum].r.svFlags & SVF_BOT) && (g_entities[teamLeader].r.svFlags & SVF_BOT) ) ) {
			//SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if ( oldTeam == TEAM_RED || oldTeam == TEAM_BLUE ) {
		CheckTeamLeader( oldTeam );
	}

	BroadcastTeamChange( client, oldTeam );

	//make a disappearing effect where they were before teleporting them to the appropriate spawn point,
	//if we were not on the spec team
	if (oldTeam != TEAM_SPECTATOR)
	{
		gentity_t *tent = G_TempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = clientNum;
	}

	// get and distribute relevent paramters
	if ( !ClientUserinfoChanged( clientNum ) )
		return;

	if (!g_preventTeamBegin)
	{
		ClientBegin( clientNum, qfalse );
	}
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	int i=0;
	ent->client->ps.persistant[ PERS_TEAM ] = TEAM_SPECTATOR;	
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;	
	ent->client->sess.spectatorState = SPECTATOR_FREE;
	ent->client->ps.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.weapon = WP_NONE;
	ent->client->ps.weaponVariation = 0;
	G_LeaveVehicle( ent, qfalse ); // clears m_iVehicleNum as well
	ent->client->ps.emplacedIndex = 0;
	//ent->client->ps.m_iVehicleNum = 0;
	ent->client->ps.viewangles[ROLL] = 0.0f;
	ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
	ent->client->ps.forceHandExtendTime = 0;
	ent->client->ps.zoomMode = 0;
	ent->client->ps.zoomLocked = qfalse;
	ent->client->ps.zoomLockTime = 0;
	ent->client->ps.legsAnim = 0;
	ent->client->ps.legsTimer = 0;
	ent->client->ps.torsoAnim = 0;
	ent->client->ps.torsoTimer = 0;
	ent->client->ps.cloakFuel = 100; // so that fuel goes away after stop following them
	ent->client->ps.jetpackFuel = 100; // so that fuel goes away after stop following them
	ent->health = ent->client->ps.stats[STAT_HEALTH] = 100; // so that you don't keep dead angles if you were spectating a dead person
	ent->client->ps.pm_type = PM_SPECTATOR;
	ent->client->ps.eFlags &= ~EF_DISINTEGRATION;
	ent->client->ps.bobCycle = 0;
	for ( i=0; i<PW_NUM_POWERUPS; i++ )
		ent->client->ps.powerups[i] = 0;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	oldTeam = ent->client->sess.sessionTeam;

	if ( trap->Argc() != 2 ) {		
		switch ( oldTeam ) {
		case TEAM_BLUE:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", bgGangWarsTeams[level.blueTeam].longname) );
			break;
		case TEAM_RED:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", bgGangWarsTeams[level.redTeam].longname) );
			break;
		case TEAM_FREE:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTFREETEAM")) );
			break;
		case TEAM_SPECTATOR:
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PRINTSPECTEAM")) );
			break;
		}
		return;
	}

	if ( ent->client->switchTeamTime > level.time ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	if (gEscaping)
	{
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( level.gametype == GT_DUEL
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {//in a tournament game
		//disallow changing teams
		trap->SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Duel\n\"" );
		return;
		//FIXME: why should this be a loss???
		//ent->client->sess.losses++;
	}

	if (level.gametype == GT_POWERDUEL)
	{ //don't let clients change teams manually at all in powerduel, it will be taken care of through automated stuff
		trap->SendServerCommand( ent-g_entities, "print \"Cannot switch teams in Power Duel\n\"" );
		return;
	}

	trap->Argv( 1, s, sizeof( s ) );

	SetTeam( ent, s );

	// fix: update team switch time only if team change really happend
	if (oldTeam != ent->client->sess.sessionTeam)
		ent->client->switchTeamTime = level.time + 5000;
}



//==========================================================
// INVENTORY RELATED COMMANDS
//==========================================================

/*
=================
JKG_Cmd_Loot_f
=================
*/
void JKG_Cmd_Loot_f(gentity_t *ent, int otherNum, int lootID, qboolean trade)
{
	return;
}

/*
=================
JKG_Cmd_ItemAction_f
=================
*/
void JKG_Cmd_ItemAction_f (gentity_t *ent, int itemNum)
{
	itemInstance_t *itemInUse;
	int i;
	if(!ent->client)
	{
		return; //NOTENOTE: NPCs can perform item actions. Nifty, eh?
	}

	if(ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->ps.pm_type == PM_DEAD)
	{
		// Bugfix: Can no longer use items when dead --eez
		Com_Printf("Can't use items while dead!\n");
		return;
	}
	
	if ( itemNum < 0 || itemNum >= MAX_INVENTORY_ITEMS )
	{
	    return;
	}
	if( itemNum > ent->inventory->size() )
	{
		//Nope.
		return;
	}
	BG_ConsumeItem(ent, itemNum);
}

/*
=================
JKG_Cmd_ShowInv_f
=================
*/
void JKG_Cmd_ShowInv_f(gentity_t *ent)
{
    char buffer[MAX_STRING_CHARS] = { 0 };
	int i = 0;
	if(!ent->client)
		return;

	Q_strncpyz (buffer, "Inventory ID | Item Num | Instance Name                       | Weight\n", sizeof (buffer));
	Q_strcat (buffer, sizeof (buffer), "-------------+----------+-----------------------------------------------+--------\n");

	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		Q_strcat(buffer, sizeof(buffer), va(S_COLOR_WHITE "%12i | %8i | %-45s | %i\n", i, it->id->itemID, it->id->displayName, it->quantity));
	}
	
	Q_strcat (buffer, sizeof (buffer), va( "%i total items", i));
	
	trap->SendServerCommand (ent->s.number, va ("print \"%s\n\"", buffer));
}

/*
=========================================
JKG_Cmd_EquipToACI_f / JKG_Cmd_Unequip_f
=========================================
*/

extern void JKG_EquipItem(gentity_t *ent, int iNum);
extern void JKG_UnequipItem(gentity_t *ent, int iNum);
void JKG_Cmd_EquipItem_f(gentity_t *ent)
{
	char arg[6];
	if(trap->Argc() != 2)
	{
		trap->SendServerCommand( ent->s.number, "print \"Usage: /equip <item num>\n\"" );
		return;
	}
	
	trap->Argv (1, arg, sizeof(arg));

	JKG_EquipItem (ent, atoi (arg));
}

void JKG_Cmd_UnequipItem_f(gentity_t *ent)
{
    char arg[6];
	if(trap->Argc() != 2)
	{
		trap->SendServerCommand( ent->s.number, "print \"Usage: /unequip <item num>\n\"" );
		return;
	}
	
	trap->Argv (1, arg, sizeof(arg));

	JKG_UnequipItem (ent, atoi (arg));
}

/*
==================
JKG_Cmd_DestroyItem_f
Destroys an item from your inventory
==================
*/
void JKG_Cmd_DestroyItem_f(gentity_t *ent)
{
	char arg[64];
	int i, inventoryID = -1;
	itemInstance_t destroyedItem;
	trap->Argv(1, arg, sizeof(arg));

	memset(&destroyedItem, 0, sizeof(itemInstance_t));

	if(trap->Argc() < 2)
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Syntax: /inventoryDestroy <inventory ID or internal>\n\"");
		return;
	}

	if(JKG_CheckIfNumber(arg) == JKGSTR_DECIMAL)
	{
		int inventoryID = atoi(arg);
		if (inventoryID >= ent->inventory->size() || inventoryID < 0)
		{
			trap->SendServerCommand(ent->client->ps.clientNum, "print \"^1Invalid inventory ID.\n\"");
			return;
		}
		BG_RemoveItemStack(ent, inventoryID);
	}
	else
	{
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (!it->id) {
				continue;
			}
			if (!Q_stricmp(it->id->internalName, arg)) {
				BG_RemoveItemStack(ent, it - ent->inventory->begin());
				return;
			}
		}
		trap->SendServerCommand(ent - g_entities, va("print \"^1Could not find item with internal %s\n\"", ConcatArgs(1)));
	}
}

void JKG_Cmd_SellItem_f(gentity_t *ent)
{
	// TODO: put proper pricing here
	char arg[64];
	trap->Argv(1, arg, sizeof(arg));
	if(trap->Argc() < 2)
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Syntax: /inventorySell <inventory ID or internal>\n\"");
		return;
	}

	int nInvID = -1;
	if(JKG_CheckIfNumber(arg) == JKGSTR_DECIMAL)
	{
		nInvID = atoi(arg);
	}
	else
	{
		for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
			if (!it->id) {
				continue;
			}
			if (!Q_stricmp(it->id->internalName, arg)) {
				nInvID = it - ent->inventory->begin();
				break;
			}
		}
	}

	if (nInvID >= ent->inventory->size() || nInvID < 0) {
		return;
	}

	auto item = (*ent->inventory)[nInvID];
	int creditAmount = item.id->baseCost;
	if (!item.id) {
		return;
	}
	// DO NOT ALLOW SELLING OF STARTER WEAPONS! (unless you already have another gun in your inventory)
	if (item.id->itemType == ITEM_WEAPON) {
		if (!Q_stricmp(item.id->internalName, level.startingWeapon)) {
			if (ent->inventory->size() < 2) {
				trap->SendServerCommand(ent - g_entities, "print \"You cannot sell your starter gun unless you have another item in your inventory.\n\"");
				return;
			}
			else {
				creditAmount = 2;
			}
		}
	}
	ent->client->ps.credits += creditAmount / 2;
	BG_RemoveItemStack(ent, nInvID);
	trap->SendServerCommand(ent->s.number, va("inventory_update %i", ent->client->ps.credits));
}
/*
=================
Cmd_DuelTeam_f
=================
*/
void Cmd_DuelTeam_f(gentity_t *ent)
{
	int			oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if (g_gametype.integer != GT_POWERDUEL)
	{ //don't bother doing anything if this is not power duel
		return;
	}


	if ( trap->Argc() != 2 )
	{ //No arg so tell what team we're currently on.
		oldTeam = ent->client->sess.duelTeam;
		switch ( oldTeam )
		{
		case DUELTEAM_FREE:
			trap->SendServerCommand( ent-g_entities, va("print \"None\n\"") );
			break;
		case DUELTEAM_LONE:
			trap->SendServerCommand( ent-g_entities, va("print \"Single\n\"") );
			break;
		case DUELTEAM_DOUBLE:
			trap->SendServerCommand( ent-g_entities, va("print \"Double\n\"") );
			break;
		default:
			break;
		}
		return;
	}

	if ( ent->client->switchDuelTeamTime > level.time )
	{ //debounce for changing
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSWITCH")) );
		return;
	}

	trap->Argv( 1, s, sizeof( s ) );

	oldTeam = ent->client->sess.duelTeam;

	if (!Q_stricmp(s, "free"))
	{
		ent->client->sess.duelTeam = DUELTEAM_FREE;
	}
	else if (!Q_stricmp(s, "single"))
	{
		ent->client->sess.duelTeam = DUELTEAM_LONE;
	}
	else if (!Q_stricmp(s, "double"))
	{
		ent->client->sess.duelTeam = DUELTEAM_DOUBLE;
	}
	else
	{
		trap->SendServerCommand( ent-g_entities, va("print \"'%s' not a valid duel team.\n\"", s) );
	}

	if (oldTeam == ent->client->sess.duelTeam)
	{ //didn't actually change, so don't care.
		return;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{ //ok..die
		int curTeam = ent->client->sess.duelTeam;
		ent->client->sess.duelTeam = oldTeam;
		G_Damage(ent, ent, ent, NULL, ent->client->ps.origin, 99999, DAMAGE_NO_PROTECTION, MOD_SUICIDE);
		ent->client->sess.duelTeam = curTeam;
	}
	//reset wins and losses
	ent->client->sess.wins = 0;
	ent->client->sess.losses = 0;

	//get and distribute relevent paramters
	ClientUserinfoChanged( ent->s.number );

	ent->client->switchDuelTeamTime = level.time + 5000;
}

/*
=================
Cmd_ForceChanged_f
=================
*/
void Cmd_ForceChanged_f( gentity_t *ent )
{
	char fpChStr[1024];
	const char *buf;
//	Cmd_Kill_f(ent);
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{ //if it's a spec, just make the changes now
		//trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "FORCEAPPLIED")) );
		//No longer print it, as the UI calls this a lot.
		WP_InitForcePowers( ent );
		goto argCheck;
	}

	buf = G_GetStringEdString("MP_SVGAME", "FORCEPOWERCHANGED");

	strcpy(fpChStr, buf);

	trap->SendServerCommand( ent-g_entities, va("print \"%s%s\n\n\"", S_COLOR_GREEN, fpChStr) );

	ent->client->ps.fd.forceDoInit = 1;
argCheck:
	if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
	{ //If this is duel, don't even bother changing team in relation to this.
		return;
	}

	if (trap->Argc() > 1)
	{
		char	arg[MAX_TOKEN_CHARS];

		trap->Argv( 1, arg, sizeof( arg ) );

		if (arg && arg[0])
		{ //if there's an arg, assume it's a combo team command from the UI.
			Cmd_Team_f(ent);
		}
	}
}

extern qboolean WP_SaberStyleValidForSaber( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int saberAnimLevel );
extern qboolean WP_UseFirstValidSaberStyle( saberInfo_t *saber1, saberInfo_t *saber2, int saberHolstered, int *saberAnimLevel );
qboolean G_SetSaber(gentity_t *ent, int saberNum, char *saberName, qboolean siegeOverride)
{
	char truncSaberName[MAX_QPATH] = {0};
	int i = 0;

	while (saberName[i] && i < 64-1)
	{
        truncSaberName[i] = saberName[i];
		i++;
	}
	truncSaberName[i] = 0;

	if ( saberNum == 0 && (Q_stricmp( "none", truncSaberName ) == 0 || Q_stricmp( "remove", truncSaberName ) == 0) )
	{ //can't remove saber 0 like this
		Q_strncpyz( truncSaberName, DEFAULT_SABER, sizeof( truncSaberName ) );
	}

	//Set the saber with the arg given. If the arg is
	//not a valid sabername defaults will be used.
	if(truncSaberName[0] != '\0') {
		WP_SetSaber( ent->s.number, ent->client->saber, saberNum, truncSaberName );
	}

	if ( !ent->client->saber[0].model[0] )
	{
		assert(!"No saber model"); //assert(0); //should never happen! // but does :/
		Q_strncpyz( ent->client->pers.saber1, DEFAULT_SABER, sizeof( ent->client->pers.saber1 ) );
	}
	else
		Q_strncpyz( ent->client->pers.saber1, ent->client->saber[0].name, sizeof( ent->client->pers.saber1 ) );

	if ( !ent->client->saber[1].model[0] )
		Q_strncpyz( ent->client->pers.saber2, "none", sizeof( ent->client->pers.saber2 ) );
	else
		Q_strncpyz( ent->client->pers.saber2, ent->client->saber[1].name, sizeof( ent->client->pers.saber2 ) );

	if ( !WP_SaberStyleValidForSaber( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, ent->client->ps.fd.saberAnimLevel ) )
	{
		WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &ent->client->ps.fd.saberAnimLevel );
		ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = ent->client->ps.fd.saberAnimLevel;
	}

	return qtrue;
}

/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if ( trap->Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg, qfalse );
	if ( i == -1 ) {
		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}

	// JKG - can't follow another spectator
	if ( level.clients[ i ].tempSpectate >= level.time ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator" );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int		clientnum;
	int		original;
	qboolean	looped = qfalse;

	if ( dir != 1 && dir != -1 ) {
		return;
	}

	// if they are playing a tournement game, count as a loss
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& ent->client->sess.sessionTeam == TEAM_FREE ) {
		//WTF???
		ent->client->sess.losses++;
	}
	// first set them to spectator
	if ( ent->client->sess.spectatorState == SPECTATOR_NOT ) {
		SetTeam( ent, "spectator" );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients )
		{
			// Avoid /team follow1 crash
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = 0;
				looped = qtrue;
			}
		}
		if ( clientnum < 0 ) {
			if ( looped )
			{
				clientnum = original;
				break;
			}
			else
			{
				clientnum = level.maxclients - 1;
				looped = qtrue;
			}
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// JKG - can't follow another spectator
		if ( level.clients[ clientnum ].tempSpectate >= level.time ) {
			return;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}

void Cmd_FollowNext_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, 1 );
}

void Cmd_FollowPrev_f( gentity_t *ent ) {
	Cmd_FollowCycle_f( ent, -1 );
}

/*
==================
G_Say
==================
*/

bool jkgIgnoreFloodProtect = false;

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, char *locMsg, int fadeLevel )
{
	if (!other) {
		return;
	}
	if (!other->inuse) {
		return;
	}
	if (!other->client) {
		return;
	}
	if ( other->client->pers.connected != CON_CONNECTED ) {
		return;
	}
	if ( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) {
		return;
	}
	/*
	// no chatting to players in tournements
	if ( (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
		&& other->client->sess.sessionTeam == TEAM_FREE
		&& ent->client->sess.sessionTeam != TEAM_FREE ) {
		//Hmm, maybe some option to do so if allowed?  Or at least in developer mode...
		return;
	}
	*/
	//They've requested I take this out.

	if (locMsg)
	{
		trap->SendServerCommand( other-g_entities, va("%s %i \"%s\" \"%s\" \"%c\" \"%s\"", 
			mode == SAY_TEAM ? "ltchat" : "lchat",
			fadeLevel, name, locMsg, color, message));
	}
	else
	{
		trap->SendServerCommand( other-g_entities, va("%s %i \"%s%c%c%s\"", 
			mode == SAY_TEAM ? "tchat" : "chat",
			fadeLevel, name, Q_COLOR_ESCAPE, color, message));
	}
}

#define EC		"\x19"

// Escape % and ", so they can be sent along properly
static const char *ChatBox_UnescapeChat(const char *message) {
	static char buff[1024] = {0};
	char *s, *t;
	char *cutoff = &buff[1023];
	s = &buff[0];
	t = (char *)message;
	while (*t && s != cutoff) {
		/*if (*t == 0x18) {
			*s = '%';
		} else*/ if (*t == 0x17) {
			*s = '"';
		} else {
			*s = *t;
		}
		t++; s++;
	}
	*s = 0;
	return &buff[0];
}

// Determine the level of fade to use for the chat message
int G_Say_GetFadeLevel(int distance, int range) {
	// Outer 20% of the range triggers the fade out
	// Minimal intensity is 15, (so text is still rendered)
	float cutoff = (float)range*0.75f;
	float cutoffrange = (float)range *0.25f;
	float cutoffarea;
	float fadeLevel;

	if (distance > range) {
		// Shouldn't happen, but if so, minimal level
		return 15;
	}
	if (distance < cutoff) {
		// Closer than cutoff, dont bother doin math, (s)he's well in range
		return 100;
	}
	// Hmk, math time! >:O!
	cutoffarea = distance - cutoff;
	fadeLevel = 1 - (cutoffarea/cutoffrange);
	if (fadeLevel < 0.15f) {
		fadeLevel = 0.15f;
	} else if (fadeLevel > 1) {
		fadeLevel = 1;		// Failsafe, this should never happen
	}

	return fadeLevel*100;

}

extern vmCvar_t		jkg_chatFloodProtect;

qboolean CCmd_Execute(int clientNum, const char *command);
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int			j;
	gentity_t	*other;
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];
	char		*locMsg = NULL;

	int			normalRadius = 0; // The radius of this message
	int			pvsRadius = 0; // The radius of this message if we go through pvs
	vec3_t		tempVec;

	int			fadeLevel = 100;

	if (!chatText || !chatText[0]) {
		return;
	}

	// Check for validity of targets
	if (!ent || !ent->client || (target && !target->client)) {
		return;
	}

	// Check for flood
	// If the target is the same as the sender, then we're sending a message to ourselves (because
	// of private chat).
	if( !jkgIgnoreFloodProtect )
	{
		if (ent != target && (ent->client->lastChatMessage > (level.time - jkg_chatFloodProtect.integer))) {
			return;
		}
	}

	ent->client->lastChatMessage = level.time;

	
	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
		mode = SAY_YELL;
	}

	if (strlen(chatText) > 511) {
		// Malicious use, ignore
		trap->SendServerCommand( ent-g_entities, "chat 100 \"^7System: ^1Malicious use of chat detected, message blocked\"");
		return;
	}

	// First, filter out any unauthorized characters (newlines)
	{
		char *s = (char *)chatText;
		while (*s) {
			if (*s == '\n' || *s == '\r') {
				*s = ' ';
			}
			s++;
		}
	}




	if (chatText[0] == '/' || chatText[0] == '\\') {
		// Command, special treatment
		if (!CCmd_Execute(ent-g_entities, &chatText[1])) {
			trap->SendServerCommand( ent-g_entities, "chat 100 \"^7System: ^1Unknown chat command\"");
		}
		return;
	}

	if (ent->client->pers.silenced) {
		// Player is silenced, ignore all his/her chat messages
		return;
	}

	// Check if Lua wants to block this message
	if (GLua_Hook_PlayerSay(ent, target, mode, chatText))
		return;

	switch ( mode ) {
	default:
	case SAY_ALL:		// Local area chat
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );							//don't apply ^xRBG fix because we use own chat function
		Com_sprintf (name, sizeof(name), "%s%c%c" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		normalRadius = 1280;
		pvsRadius = 0;		// Cant go through pvs
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		if (Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC "(%s%c%c" EC ")" EC ": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC "(%s%c%c" EC ")" EC ": ", 
				ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if (target && target->inuse && target->client && level.gametype >= GT_TEAM &&
			target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
			Team_GetLocationMsg(ent, location, sizeof(location)))
		{
			Com_sprintf (name, sizeof(name), EC "[%s%c%c" EC "]" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
			locMsg = location;
		}
		else
		{
			Com_sprintf (name, sizeof(name), EC "[%s%c%c" EC "]" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		}
		color = COLOR_MAGENTA;
		break;
	case SAY_ACT:
		G_LogPrintf( "sayact: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "*%s%c%c" EC " ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		normalRadius = 1280;
		pvsRadius = 0;		// Cant go through pvs
		color = COLOR_WHITE;
		break;
	case SAY_GLOBAL:
		G_LogPrintf( "sayglobal: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^5Global^7] %s%c%c" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_YELL:
		G_LogPrintf( "sayyell: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^3YELL^7] %s%c%c" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_YELLOW;
		normalRadius = 3500;
		pvsRadius = 2000;
		break;
	case SAY_WHISPER:
		G_LogPrintf( "saywhisper: %s: %s\n", ent->client->pers.netname, ChatBox_UnescapeChat(chatText) );
		Com_sprintf (name, sizeof(name), "^7[^5Whisper^7] %s%c%c" EC ": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		normalRadius = 150;
		pvsRadius = 0;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if ( target ) {
		G_SayTo( ent, target, mode, color, name, text, locMsg, fadeLevel );
		return;
	}

	// echo the text to the console
	if ( dedicated.integer ) {
		trap->Print( "%s%s\n", name, text);
	}

	// send it to all the apropriate clients
	// First check if we got a radius or not
	if (!pvsRadius && !normalRadius) {
		// No radius on either, send to all
		for (j = 0; j < level.maxclients; j++) {
			other = &g_entities[j];
			G_SayTo( ent, other, mode, color, name, text, locMsg, fadeLevel );
		}
	} else {
		// We got a radius here, so filter
		for (j = 0; j < level.maxclients; j++) {
			other = &g_entities[j];
			if (!other->inuse) {
				continue;
			}
			if (!other->client) {
				continue;
			}
			if ( other->client->pers.connected != CON_CONNECTED ) {
				continue;
			}
			if ( other->client->tempSpectate || other->client->sess.sessionTeam == TEAM_SPECTATOR  ) {
				// spectating folks cant see local area chat
				continue;
			}
			if (!trap->InPVS(ent->client->ps.origin, other->client->ps.origin)) {
				// Not in PVS
				if (!pvsRadius) {
					continue;
				} else {
					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, tempVec);
					if (VectorLength(tempVec) > pvsRadius) {
						continue;
					}
					fadeLevel = G_Say_GetFadeLevel(VectorLength(tempVec), pvsRadius);
				}
			} else {
				// In PVS
				VectorSubtract(ent->client->ps.origin, other->client->ps.origin, tempVec);
				if (VectorLength(tempVec) > normalRadius) {
					continue;
				}
				fadeLevel = G_Say_GetFadeLevel(VectorLength(tempVec), normalRadius);
			}
			G_SayTo( ent, other, mode, color, name, text, locMsg, fadeLevel );
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	char		*p;

	if ( trap->Argc () < 2 && !arg0 ) {
		return;
	}

	if (arg0)
	{
		p = ConcatArgs( 0 );
	}
	else
	{
		p = ConcatArgs( 1 );
	}
	
	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Say_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}


	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_SayAct_f - Actions
==================
*/
static void Cmd_SayAct_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );
	
	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_SayAct_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}


	G_Say( ent, NULL, SAY_ACT, p );
}

/*
==================
Cmd_SayGlobal_f - Server-wide
==================
*/
static void Cmd_SayGlobal_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_GLOBAL, p );
}

/*
==================
Cmd_SayYell_f - Yell
==================
*/
static void Cmd_SayYell_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_YELL, p );
}

/*
==================
Cmd_SayTeam_f - Team Chat
VERSUS ONLY
==================
*/
static void Cmd_SayTeam_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );
	
	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_SayTeam_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_Say( ent, NULL, SAY_TEAM, p );
}

/*
==================
Cmd_SayWhisper_f - Yell
==================
*/
static void Cmd_SayWhisper_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );
	
	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_SayWhisper_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}


	G_Say( ent, NULL, SAY_WHISPER, p );
}

/*
==================
Cmd_Tell_f
==================
*/

static void Cmd_Tell_f( gentity_t *ent ) {
	int			targetNum;
	gentity_t	*target;
	char		*p;
	char		arg[MAX_TOKEN_CHARS];

	if ( trap->Argc () < 2 ) {
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	targetNum = G_ClientNumberFromArg( arg );
	if ( targetNum < 0 || targetNum >= level.maxclients ) {
		return;
	}

	target = &g_entities[targetNum];
	if ( !target || !target->inuse || !target->client ) {
		return;
	}

	p = ConcatArgs( 2 );

	//Raz: BOF
	if ( strlen( p ) > MAX_SAY_TEXT )
	{
		p[MAX_SAY_TEXT-1] = '\0';
		G_SecurityLogPrintf( "Cmd_Tell_f from %d (%s) has been truncated: %s\n", ent->s.number, ent->client->pers.netname, p );
	}

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
		G_Say( ent, ent, SAY_TELL, p );
	}
}

void CCmd_Say( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_ALL, CCmd_ConcatArgs( 1 ) );
}

void CCmd_SayGlobal( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_GLOBAL, CCmd_ConcatArgs( 1 ) );
}

void CCmd_SayYell( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_YELL, CCmd_ConcatArgs( 1 ) );
}

void CCmd_SayAct( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_ACT, CCmd_ConcatArgs( 1 ) );
}

void CCmd_SayWhisper( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_WHISPER, CCmd_ConcatArgs( 1 ) );
}

void CCmd_Tell( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_TELL, CCmd_ConcatArgs( 1 ) );
}

void CCmd_Say_Team( void )
{
	G_Say( &g_entities[CCmd_Caller()], NULL, SAY_TEAM, CCmd_ConcatArgs( 1 ) );
}

void JKG_BindChatCommands( void )
{
	// Evil: binds /say, /sayact, etc to chat commands as well...
	CCmd_AddCommand("say", CCmd_Say);
	CCmd_AddCommand("sayglobal", CCmd_SayGlobal);
	CCmd_AddCommand("sayact", CCmd_SayAct);
	CCmd_AddCommand("saywhisper", CCmd_SayWhisper);
	CCmd_AddCommand("tell", CCmd_Tell);
	CCmd_AddCommand("say_team", CCmd_Say_Team);
}

/*
==================
ClientNumberFromString
Returns a player number for a name string
Returns -1 if invalid
==================
*/
int G_ClientNumberFromStrippedName( const char *name ) {
	gclient_t	*cl;
	int			idnum;
	char		cleanInput[MAX_NETNAME];

	Q_strncpyz( cleanInput, name, sizeof(cleanInput) );
	Q_StripColor( cleanInput );

	for ( idnum=0,cl=level.clients; idnum < level.maxclients; idnum++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			continue;

		if ( !Q_stricmp( cl->pers.netname_nocolor, cleanInput ) )
			return idnum;
	}

	return -1;
}

//siege voice command
static void Cmd_VoiceCommand_f(gentity_t *ent)
{
	gentity_t *te;
	char arg[MAX_TOKEN_CHARS];
	char *s;
	int i = 0;

	if (level.gametype < GT_TEAM)
	{
		return;
	}

	if (trap->Argc() < 2)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		ent->client->tempSpectate >= level.time)
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOICECHATASSPEC")) );
		return;
	}

	trap->Argv(1, arg, sizeof(arg));

	if (arg[0] == '*')
	{ //hmm.. don't expect a * to be prepended already. maybe someone is trying to be sneaky.
		return;
	}

	s = va("*%s", arg);

	//now, make sure it's a valid sound to be playing like this.. so people can't go around
	//screaming out death sounds or whatever.
	while (i < MAX_CUSTOM_SIEGE_SOUNDS)
	{
		if (!bg_customSiegeSoundNames[i])
		{
			break;
		}
		if (!Q_stricmp(bg_customSiegeSoundNames[i], s))
		{ //it matches this one, so it's ok
			break;
		}
		i++;
	}

	if (i == MAX_CUSTOM_SIEGE_SOUNDS || !bg_customSiegeSoundNames[i])
	{ //didn't find it in the list
		return;
	}

	te = G_TempEntity(vec3_origin, EV_VOICECMD_SOUND);
	te->s.groundEntityNum = ent->s.number;
	te->s.eventParm = G_SoundIndex((char *)bg_customSiegeSoundNames[i]);
	te->r.svFlags |= SVF_BROADCAST;
}


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};
static size_t numgc_orders = ARRAY_LEN( gc_orders );

void Cmd_GameCommand_f( gentity_t *ent ) {
	int				targetNum;
	unsigned int	order;
	gentity_t		*target;
	char			arg[MAX_TOKEN_CHARS] = {0};

	if ( trap->Argc() != 3 ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"Usage: gc <player id> <order 0-%d>\n\"", numgc_orders - 1 ) );
		return;
	}

	trap->Argv( 2, arg, sizeof( arg ) );
	order = atoi( arg );

	if ( order >= numgc_orders ) {
		trap->SendServerCommand( ent-g_entities, va("print \"Bad order: %i\n\"", order));
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );
	targetNum = ClientNumberFromString( ent, arg, qfalse );
	if ( targetNum == -1 )
		return;

	target = &g_entities[targetNum];
	if ( !target->inuse || !target->client )
		return;

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client->pers.netname, target->client->pers.netname, gc_orders[order] );
	G_Say( ent, target, SAY_TELL, gc_orders[order] );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT) )
		G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
	//JAC: This wasn't working for non-spectators since s.origin doesn't update for active players.
	if(ent->client && ent->client->sess.sessionTeam != TEAM_SPECTATOR )
	{//active players use currentOrigin
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
	}
	else
	{
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
	}
}

/*
==================
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName ( const char* name )
{
	char		cleanInput[MAX_NETNAME];
	char		cleanName[MAX_NETNAME];
	int			i;
	gclient_t*	cl;

	Q_strncpyz( cleanInput, name, sizeof( cleanInput ) );
	Q_StripColor( cleanInput );
	for ( i=0,cl=level.clients; i < level.maxclients; i++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			continue;
		Q_strncpyz( cleanName, cl->pers.netname, sizeof( cleanName ) );
		Q_StripColor( cleanName );
		if ( !Q_stricmp( cleanName, cleanInput ) )
		{
			return i;
		}
	}

	return -1;
}

/*
==================
G_ClientNumberFromStrippedSubstring

Same as above, but strips special characters out of the names before comparing.
Checks substrings rather than the full string and returns -2 on multiple matches (if specified to do so)
==================
*/

int G_ClientNumberFromStrippedSubstring ( const char* name, qboolean checkAll )
{
	gclient_t	*cl;
	int			idnum, match = -1;
	char		cleanInput[MAX_NETNAME];

	Q_strncpyz( cleanInput, name, sizeof(cleanInput) );
	Q_StripColor( cleanInput );

	for ( idnum=0,cl=level.clients; idnum < level.maxclients; idnum++,cl++ )
	{// check for a name match
		if ( cl->pers.connected != CON_CONNECTED )
			continue;

		if ( Q_stristr( cl->pers.netname_nocolor, cleanInput ) )
		{
			if( match != -1 )
			{ //found more than one match
				return -2;
			}
			match = idnum;
			if (!checkAll)
			{ // Don't continue checking
				return match;
			}
		}
	}

	return match; // Could this just be -1? I'm not sure...
}

/*
==================
Cmd_CallVote_f
==================
*/
const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
	int		i;
	char	arg1[MAX_CVAR_VALUE_STRING], arg2[MAX_CVAR_VALUE_STRING];
	char	*mapName = NULL;
	const char *arenaInfo = NULL;

	if ( !g_allowVote.integer ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.voteTime || level.voteExecuteTime >= level.time ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEINPROGRESS")) );
		return;
	}

	if (level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
			return;
		}
	}

	// make sure it is a valid command to vote on
	trap->Argv( 1, arg1, sizeof( arg1 ) );
	trap->Argv( 2, arg2, sizeof( arg2 ) );

	//Raz: callvote exploit, filter \n and \r ==> in both args
	if ( strchr( arg1, ';' ) || strchr( arg2, ';' ) ||
		 strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ||
		 strchr( arg1, '\n' ) || strchr( arg2, '\n' ) )
	{
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( Q_stricmp( arg1, "map_restart" ) &&
		 Q_stricmp( arg1, "nextmap" ) &&
		 Q_stricmp( arg1, "map" ) &&
		 Q_stricmp( arg1, "g_gametype" ) &&
		 Q_stricmp( arg1, "kick" ) &&
		 Q_stricmp( arg1, "clientkick" ) &&
		 Q_stricmp( arg1, "timelimit" ) &&
		 Q_stricmp( arg1, "fraglimit" ) &&
		 Q_stricmp( arg1, "capturelimit" ) )
	{
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap->SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, timelimit <time>, fraglimit <frags>, capturelimit <captures>.\n\"" );
		return;
	}

	level.votingGametype = qfalse;

	// if there is still a vote to be executed
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap->SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
	}

	// special case for g_gametype, check for bad values
	if ( !Q_stricmp( arg1, "g_gametype" ) )
	{
		int gt = atoi( arg2 );

		if ( arg2[0] && isalpha( arg2[0] ) )
		{// ffa, ctf, tdm, etc
			gt = BG_GetGametypeForString( arg2 );
			if ( gt == -1 )
			{
				trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%s) unrecognised, defaulting to FFA/Deathmatch\n\"", arg2 ) );
				gt = GT_FFA;
			}
		}
		else if ( gt < 0 || gt >= GT_MAX_GAME_TYPE )
		{// numeric but out of range
			trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%i) is out of range, defaulting to FFA/Deathmatch\n\"", gt ) );
			gt = GT_FFA;
		}

		if ( gt == GT_SINGLE_PLAYER )
		{// logically invalid gametypes, or gametypes not fully implemented in MP
			trap->SendServerCommand( ent-g_entities, "print \"This gametype is not supported (%s).\n\"" );
			return;
		}

		level.votingGametype = qtrue;
		level.votingGametypeTo = gt;

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, gt );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, BG_GetGametypeString( gt ) );
		Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	}
	else if ( !Q_stricmp( arg1, "map" ) ) 
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_CVAR_VALUE_STRING];

		if (!G_DoesMapSupportGametype(arg2, level.gametype))
		{
			//trap->SendServerCommand( ent-g_entities, "print \"You can't vote for this map, it isn't supported by the current gametype.\n\"" );
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE_MAPNOTSUPPORTEDBYGAME")) );
			return;
		}

		trap->Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (*s) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
		}
		
		arenaInfo	= G_GetArenaInfoByMap(arg2);
		if (arenaInfo)
		{
			mapName = Info_ValueForKey(arenaInfo, "longname");
		}

		if (!mapName || !mapName[0])
		{
			mapName = "ERROR";
		}

		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "map %s", mapName);
	}
	else if ( !Q_stricmp ( arg1, "clientkick" ) )
	{
		int n = atoi ( arg2 );

		if ( n < 0 || n >= MAX_CLIENTS )
		{
			trap->SendServerCommand( ent-g_entities, va("print \"invalid client number %d.\n\"", n ) );
			return;
		}

		if ( g_entities[n].client->pers.connected != CON_CONNECTED )
		{
			trap->SendServerCommand( ent-g_entities, va("print \"there is no client with the client number %d.\n\"", n ) );
			return;
		}
			
		Com_sprintf ( level.voteString, sizeof(level.voteString ), "%s %s", arg1, arg2 );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[n].client->pers.netname );		//--futuza note: maybe need to use JKG_xRBG_ConvertExtToNormal() ^xRBG fix here ...unsure
	}
	else if ( !Q_stricmp ( arg1, "kick" ) )
	{
		int clientid = G_ClientNumberFromName ( arg2 );

		if ( clientid == -1 )
		{
			trap->SendServerCommand( ent-g_entities, va("print \"there is no client named '%s' currently on the server.\n\"", arg2 ) );
			return;
		}

		Com_sprintf ( level.voteString, sizeof(level.voteString ), "clientkick %d", clientid );
		Com_sprintf ( level.voteDisplayString, sizeof(level.voteDisplayString), "kick %s", g_entities[clientid].client->pers.netname );
	}
	else if ( !Q_stricmp( arg1, "nextmap" ) ) 
	{
		char	s[MAX_STRING_CHARS];

		trap->Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if (!*s) {
			trap->SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	//Raz: bounds checking
	else if ( !Q_stricmp( arg1, "timelimit" ) )
	{
		float tl = Com_Clamp( 0.0f, 35790.0f, atof( arg2 ) );
		if ( trap->Argc() < 3 )
		{
			trap->SendServerCommand( ent-g_entities, "print \"Usage: /callvote timelimit <time>.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %.3f", arg1, tl );
		if ( Q_isintegral( tl ) )
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, (int)tl );
		else
			Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %.3f", arg1, tl );
			Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}
	else if ( !Q_stricmp( arg1, "fraglimit" ) )
	{
		int n = Com_Clampi( 0, 0x7FFFFFFF, atoi( arg2 ) );
		if ( trap->Argc() < 3 )
		{
			trap->SendServerCommand( ent-g_entities, "print \"Usage: /callvote fraglimit <frags>.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	}
	else if ( !Q_stricmp( arg1, "capturelimit" ) )
	{
		int n = Com_Clampi( 0, 0x7FFFFFFF, atoi( arg2 ) );
		if ( trap->Argc() < 3 )
		{
			trap->SendServerCommand( ent-g_entities, "print \"Usage: /callvote capturelimit <captures>.\n\"" );
			return;
		}
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	}
	else if ( !Q_stricmp( arg1, "map_restart" ) )
	{
		int n = Com_Clampi( 0, 60, atoi( arg2 ) );
		if ( trap->Argc() < 3 )
			n = 5;
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, n );
	}
	else
	{
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	Q_strstrip( level.voteStringClean, "\"\n\r", NULL );

	trap->SendServerCommand( -1, va("print \"%s^7 %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLCALLEDVOTE") ) );				//--futuza: note apply ^xRBG fix?

	// start the voting, the caller autoamtically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
		level.clients[i].pers.vote = 0;
	}
	ent->client->mGameFlags |= PSG_VOTED;
	ent->client->pers.vote = 1;

	trap->SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
	trap->SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap->SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	trap->SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char		msg[64];

	if ( !level.voteTime ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_VOTED ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "VOTEALREADY")) );
		return;
	}
	if (level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL)
	{
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
			return;
		}
	}

	trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLVOTECAST")) );

	ent->client->mGameFlags |= PSG_VOTED;

	trap->Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.voteYes++;
		ent->client->pers.vote = 1;
		trap->SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
	} else {
		level.voteNo++;
		ent->client->pers.vote = 2;
		trap->SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
	int		i, team, cs_offset;
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !g_allowVote.integer ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTE")) );
		return;
	}

	if ( level.teamVoteTime[cs_offset] ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADY")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOSPECVOTE")) );
		return;
	}

	// make sure it is a valid command to vote on
	trap->Argv( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	for ( i = 2; i < trap->Argc(); i++ ) {
		if (i > 2)
			strcat(arg2, " ");
		trap->Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if ( strchr( arg1, ';' ) || strchr( arg2, ';' ) ||
		 strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ||
		 strchr( arg1, '\n' ) || strchr( arg2, '\n' ) )
	{
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	if ( !Q_stricmp( arg1, "leader" ) ) {
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if ( !arg2[0] ) {
			i = ent->client->ps.clientNum;
		}
		else {
			// numeric values are just slot numbers
			for (i = 0; i < 3; i++) {
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if ( i >= 3 || !arg2[i]) {
				i = atoi( arg2 );
				if ( i < 0 || i >= level.maxclients ) {
					trap->SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if ( !g_entities[i].inuse ) {
					trap->SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else {
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for ( i = 0 ; i < level.maxclients ; i++ ) {
					if ( level.clients[i].pers.connected == CON_DISCONNECTED )
						continue;
					if (level.clients[i].sess.sessionTeam != team)
						continue;
					Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
					Q_CleanStr(netname);
					if ( !Q_stricmp(netname, leader) ) {
						break;
					}
				}
				if ( i >= level.maxclients ) {
					trap->SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} else {
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap->SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED )
			continue;
		if (level.clients[i].sess.sessionTeam == team)
			trap->SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );			//--futuza note: apply ^xRBG fix?
	}

	// start the voting, the caller autoamtically votes yes
	level.teamVoteTime[cs_offset] = level.time;
	level.teamVoteYes[cs_offset] = 1;
	level.teamVoteNo[cs_offset] = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if (level.clients[i].sess.sessionTeam == team) {
			level.clients[i].mGameFlags &= ~PSG_TEAMVOTED;
			level.clients[i].pers.teamvote = 0;
		}
	}
	ent->client->mGameFlags |= PSG_TEAMVOTED;
	ent->client->pers.teamvote = 1;

	trap->SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
	trap->SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
	trap->SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	trap->SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
	int			team, cs_offset;
	char		msg[64];

	team = ent->client->sess.sessionTeam;
	if ( team == TEAM_RED )
		cs_offset = 0;
	else if ( team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !level.teamVoteTime[cs_offset] ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOTEAMVOTEINPROG")) );
		return;
	}
	if ( ent->client->mGameFlags & PSG_TEAMVOTED ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMVOTEALREADYCAST")) );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOVOTEASSPEC")) );
		return;
	}

	trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "PLTEAMVOTECAST")) );

	ent->client->mGameFlags |= PSG_TEAMVOTED;

	trap->Argv( 1, msg, sizeof( msg ) );

	if ( tolower( msg[0] ) == 'y' || msg[0] == '1' ) {
		level.teamVoteYes[cs_offset]++;
		ent->client->pers.teamvote = 1;
		trap->SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
	} else {
		level.teamVoteNo[cs_offset]++;
		ent->client->pers.teamvote = 2;
		trap->SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}

/*
=================
Cmd_Reload_f
=================
*/
qboolean PM_InKnockDown( playerState_t *ps );
void Cmd_Reload_f( gentity_t *ent ) {
	int weapon;
	int ammotoadd;
	int weaponIndex;
	gentity_t *evt;

	weapon = ent->client->ps.weapon;

    #ifdef _DEBUG
    //trap->SendServerCommand (-1, va ("print \"Weapon %s/%d has clip size of %d.\n\"", WPTable[weapon + 1], ent->client->ps.weaponVariation, GetWeaponAmmoClip(weapon, ent->client->ps.weaponVariation)));
    #endif
	if ( GetWeaponAmmoClip( weapon, ent->client->ps.weaponVariation ) == -1 )
	{
		// Current weapon does not use a clip, bail
		return;
	}

	if (ent->client->ps.weaponTime > 0 && ent->client->ps.weaponstate != WEAPON_READY) {
		// Weapon is busy, cannot reload at this moment
		return;
	}

	if ( ent->client->ps.forceHandExtend != HANDEXTEND_NONE && !PM_InKnockDown (&ent->client->ps) )
	{//We're in a knockdown, or something ugly like that...DO NOT WANT!
		return;
	}
	
	/* No more ammo left to place in the clip */
	if ( ent->client->ps.ammo <= 0 && ent->client->ps.weapon != WP_SABER)
	{
		return;
	}
	
	// Can't reload while sprinting
	if ( BG_IsSprinting (&ent->client->ps, &ent->client->pers.cmd, qfalse) )
	{
	    return;
	}

	// Can't reload while rolling or flipping
	if ( ent->client->ps.pm_flags & PMF_ROLLING || BG_FlippingAnim ( ent->client->ps.legsAnim ) )
	{
	    return;
	}

	// Can't reload while on an emplaced gun
	if ( ent->client->ps.emplacedIndex )
	{ 
		return;
	}
	
    // Can't reload while charging the weapon
	if ( ent->client->ps.weaponstate == WEAPON_CHARGING || ent->client->ps.weaponstate == WEAPON_CHARGING_ALT )
	{
		return;
	}

	if ( weapon == WP_SABER)
	{
		// Do a kick instead, regardless if we're moving
		ent->client->ps.saberActionFlags |= ( 1 << SAF_KICK );
		return;
	}

	ammotoadd = GetWeaponAmmoClip( weapon, ent->client->ps.weaponVariation );
	weaponIndex = BG_GetWeaponIndex(weapon, ent->client->ps.weaponVariation);
	
	if (ent->client->clipammo[weaponIndex] >= ammotoadd)	{
		// Current clip is full
		return;
	}
	
	ammotoadd -= ent->client->clipammo[weaponIndex];

    #ifdef _DEBUG
    //trap->SendServerCommand (-1, va ("print \"Reloading weapon %s.\n\"", WPTable[weapon + 1]));
    #endif
    
	evt						= G_TempEntity( ent->s.pos.trBase, EV_RELOAD );
	evt->s.time				= ent->s.number;
	evt->s.eventParm		= weapon;
	evt->s.weaponVariation	= ent->client->ps.weaponVariation;

	ent->client->ps.weaponstate = WEAPON_RELOADING;
	
	// We're in a zoom and start a reload, remove the zoom!
	if ( ent->client->ps.zoomMode )
	{
		ent->client->ps.zoomMode = 0;
	}

	G_SetAnim(ent, NULL, SETANIM_TORSO, GetWeaponData (weapon, ent->client->ps.weaponVariation)->anims.reload.torsoAnim, SETANIM_FLAG_NORMAL);
	ent->client->ps.weaponTime = GetWeaponData( weapon, ent->client->ps.weaponVariation )->weaponReloadTime;
	// TODO: Add sound

	// Check if we have enough ammo remaining
	ammotoadd = Q_min (ammotoadd, ent->client->ps.ammo);

	//Remove the ammo from 'bag'
	ent->client->ps.ammo -= ammotoadd;
	ent->client->ammoTable[GetWeaponAmmoIndex(ent->client->ps.weapon, ent->client->ps.weaponVariation)] -= ammotoadd;

	//Add the ammo to weapon
	ent->client->clipammo[weaponIndex] += ammotoadd;

	//Take us out of ironsights
	//ent->client->ns.ironsightsTime &= ~IRONSIGHTS_MSB;
	ent->client->ps.ironsightsDebounceStart = level.time + ent->client->ps.weaponTime;

	// Don't shoot any more bullets!
	ent->client->ps.shotsRemaining = 0;
}

/*
=================
Cmd_TcEval_f
=================
*/
void Cmd_TcEval_f(gentity_t *ent, bool bMulti = false) {
	// usage: tcEval <tc> <samples>
	if (trap->Argc() != 3 && !bMulti) {
		trap->SendServerCommand(ent - g_entities, "print \"usage: tcEval <tc> <samples>\n\"");
		return;
	}
	else if (trap->Argc() != 3 && bMulti) {
		trap->SendServerCommand(ent - g_entities, "print \"usage: tcEvalMulti <tc> <samples>\n\"");
		return;
	}
	char buffer[MAX_TOKEN_CHARS];
	int nSamples = 0;
	trap->Argv(2, buffer, sizeof(buffer));
	nSamples = atoi(buffer);
	trap->Argv(1, buffer, sizeof(buffer));
	if (nSamples <= 0) {
		return;
	}
	Q_strlwr(buffer);
	std::string sSearch = buffer;
	auto tc = umTreasureClasses.find(sSearch);
	if (tc == umTreasureClasses.end()) {
		trap->SendServerCommand(ent - g_entities, "print \"could not find treasure class.\n\"");
		return;
	}
	TreasureClass* pTC = tc->second;
	pTC->PrintDropRates(ent, nSamples, bMulti);
}

/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t		origin, angles;
	char		buffer[MAX_TOKEN_CHARS];
	int			i;

	if ( trap->Argc() != 5 ) {
		trap->SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap->Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap->Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}

void G_LeaveVehicle( gentity_t* ent, qboolean ConCheck ) {

	if (ent->client->ps.m_iVehicleNum)
	{ //tell it I'm getting off
		gentity_t *veh = &g_entities[ent->client->ps.m_iVehicleNum];

		if (veh->inuse && veh->client && veh->m_pVehicle)
		{
			if ( ConCheck ) { // check connection
				clientConnected_t pCon = ent->client->pers.connected;
				ent->client->pers.connected = CON_DISCONNECTED;
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
				ent->client->pers.connected = pCon;
			} else { // or not.
				veh->m_pVehicle->m_pVehicleInfo->Eject(veh->m_pVehicle, (bgEntity_t *)ent, qtrue);
			}
		}
	}

	ent->client->ps.m_iVehicleNum = 0;
}

int G_ItemUsable(playerState_t *ps, int forcedUse)
{
	vec3_t fwd, fwdorg, dest, pos;
	vec3_t yawonly;
	vec3_t mins, maxs;
	vec3_t trtest;
	trace_t tr;

	// fix: dead players shouldn't use items
	if (ps->stats[STAT_HEALTH] <= 0) {
		return 0;
	}

	if (ps->m_iVehicleNum)
	{
		return 0;
	}
	
	if (ps->pm_flags & PMF_USE_ITEM_HELD)
	{ //force to let go first
		return 0;
	}

	if (!forcedUse)
	{
		forcedUse = bg_itemlist[ps->stats[STAT_HOLDABLE_ITEM]].giTag;
	}

	if (!BG_IsItemSelectable(ps, forcedUse))
	{
		return 0;
	}

	switch (forcedUse)
	{
	case HI_MEDPAC:
	case HI_MEDPAC_BIG:
		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return 0;
		}

		if (ps->stats[STAT_HEALTH] <= 0)
		{
			return 0;
		}

		return 1;
	case HI_SEEKER:
		if (ps->eFlags & EF_SEEKERDRONE)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SEEKER_ALREADYDEPLOYED);
			return 0;
		}

		return 1;
	case HI_SENTRY_GUN:
		if (ps->fd.sentryDeployed)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_ALREADYPLACED);
			return 0;
		}

		yawonly[ROLL] = 0;
		yawonly[PITCH] = 0;
		yawonly[YAW] = ps->viewangles[YAW];

		VectorSet( mins, -8, -8, 0 );
		VectorSet( maxs, 8, 8, 24 );

		AngleVectors(yawonly, fwd, NULL, NULL);

		fwdorg[0] = ps->origin[0] + fwd[0]*64;
		fwdorg[1] = ps->origin[1] + fwd[1]*64;
		fwdorg[2] = ps->origin[2] + fwd[2]*64;

		trtest[0] = fwdorg[0] + fwd[0]*16;
		trtest[1] = fwdorg[1] + fwd[1]*16;
		trtest[2] = fwdorg[2] + fwd[2]*16;

		trap->Trace(&tr, ps->origin, mins, maxs, trtest, ps->clientNum, MASK_PLAYERSOLID, 0, 0, 0);

		if ((tr.fraction != 1 && tr.entityNum != ps->clientNum) || tr.startsolid || tr.allsolid)
		{
			G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SENTRY_NOROOM);
			return 0;
		}

		return 1;
	case HI_SHIELD:
		mins[0] = -8;
		mins[1] = -8;
		mins[2] = 0;

		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 8;

		AngleVectors (ps->viewangles, fwd, NULL, NULL);
		fwd[2] = 0;
		VectorMA(ps->origin, 64, fwd, dest);
		trap->Trace(&tr, ps->origin, mins, maxs, dest, ps->clientNum, MASK_SHOT, 0, 0, 0 );
		if (tr.fraction > 0.9f && !tr.startsolid && !tr.allsolid)
		{
			VectorCopy(tr.endpos, pos);
			VectorSet( dest, pos[0], pos[1], pos[2] - 4096 );
			trap->Trace( &tr, pos, mins, maxs, dest, ps->clientNum, MASK_SOLID, 0, 0, 0 );
			if ( !tr.startsolid && !tr.allsolid )
			{
				return 1;
			}
		}
		G_AddEvent(&g_entities[ps->clientNum], EV_ITEMUSEFAIL, SHIELD_NOROOM);
		return 0;
	case HI_JETPACK: //do something?
		return 1;
	case HI_HEALTHDISP:
		return 1;
	case HI_AMMODISP:
		return 1;
	case HI_EWEB:
		return 1;
	case HI_CLOAK:
		return 1;
	default:
		return 1;
	}
}

void saberKnockDown(gentity_t *saberent, gentity_t *saberOwner, gentity_t *other);

void Cmd_ToggleSaber_f(gentity_t *ent)
{
	int previousSaberHolstered;

	if (ent->client->ps.fd.forceGripCripple)
	{ //if they are being gripped, don't let them unholster their saber
		if (ent->client->ps.saberHolstered)
		{
			return;
		}
	}

	if (ent->client->ps.saberInFlight)
	{
		if (ent->client->ps.saberEntityNum)
		{ //turn it off in midair
			saberKnockDown(&g_entities[ent->client->ps.saberEntityNum], ent, ent);
		}
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

//	if (ent->client->ps.duelInProgress && !ent->client->ps.saberHolstered)
//	{
//		return;
//	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.saberLockTime >= level.time)
	{
		return;
	}

	if (ent->client && ent->client->ps.weaponTime < 1)
	{
		previousSaberHolstered = ent->client->ps.saberHolstered;

		if (ent->client->ps.saberHolstered == 2 ||
			((SaberStances[ent->client->ps.fd.saberAnimLevel].isDualsOnly ||
				SaberStances[ent->client->ps.fd.saberAnimLevel].isStaffOnly) &&
				ent->client->ps.saberHolstered == 1))
 		{
			if((SaberStances[ent->client->ps.fd.saberAnimLevel].isDualsOnly || SaberStances[ent->client->ps.fd.saberAnimLevel].isStaffOnly) &&
				ent->client->ps.saberHolstered == 2)
			{
				ent->client->ps.saberHolstered = 1;
			}
			else if(SaberStances[ent->client->ps.fd.saberAnimLevel].isDualsOnly || SaberStances[ent->client->ps.fd.saberAnimLevel].isStaffOnly)
			{
				ent->client->ps.saberHolstered = 0;
			}
			else if(!SaberStances[ent->client->ps.fd.saberAnimLevel].isDualsOnly &&
				!SaberStances[ent->client->ps.fd.saberAnimLevel].isStaffOnly)
			{
				ent->client->ps.saberHolstered = 0;
			}
			else
			{
				ent->client->ps.saberHolstered = 0;
			}
			ent->client->ps.saberActionFlags &= ~(1 << SAF_BLOCKING);
			ent->client->ps.saberActionFlags &= ~( 1 << SAF_PROJBLOCKING );
			ent->client->blockingLightningAccumulation = 0;

			if(previousSaberHolstered == 0)
			{
				G_SetAnim(ent, NULL, SETANIM_BOTH, BOTH_STAND1TO2, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
			}

			{
				int random = Q_irand(1, 3);

				if(random == 1)
				{
					G_Sound (ent, CHAN_AUTO, G_SoundIndex("sound/weapons/saber/saberon.wav"));
				}
				else
				{
					G_Sound (ent, CHAN_AUTO, G_SoundIndex(va("sound/weapons/saber/saberon%i.wav", random)));
				}
			}
		}
		else
		{
			ent->client->ps.saberHolstered = 2;
			ent->client->ps.saberActionFlags &= ~(1 << SAF_BLOCKING);
			ent->client->ps.saberActionFlags &= ~( 1 << SAF_PROJBLOCKING );
			ent->client->blockingLightningAccumulation = 0;

			if (ent->client->saber[0].soundOff)
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
			}
			if (ent->client->saber[1].soundOff &&
				ent->client->saber[1].model[0])
			{
				G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
			}

			//prevent anything from being done for 200ms after holster // cut from 400 --eez
			ent->client->ps.weaponTime = 200;

			G_SetAnim(ent, NULL, SETANIM_BOTH, BOTH_STAND2TO1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);

			{
				int random = Q_irand(1, 3);
				if(random == 1)
				{
					G_Sound (ent, CHAN_AUTO, G_SoundIndex("sound/weapons/saber/saberoff.wav"));
				}
				else
				{
					G_Sound (ent, CHAN_AUTO, G_SoundIndex(va("sound/weapons/saber/saberoff%i.wav", random)));
				}
			}
		}
	}
}

extern vmCvar_t		d_saberStanceDebug;

extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	int i, j;

	qboolean usingSiegeStyle = qfalse;
	
	if ( !ent || !ent->client )
	{
		return;
	}

	if( ent->client->saberStanceDebounce > level.time)
	{
		// No. Don't want spamming of firing mode change or stance change --eez
		return;
	}

	if( ent->client->ps.weaponTime > 0 )
	{
		// Don't use in the middle of a burst or in a saber move or while reloading --eez
		return;
	}
	/*
	if (ent->client->ps.weaponTime > 0)
	{ //no switching attack level when busy
		return;
	}
	*/

	// Jedi Knight Galaxies, we cant swich saber style if we're not using a saber...
	if (ent->client->ps.weapon != WP_SABER)
	{
		// But we can change firing modes!
		int previousFiringMode = ent->client->ps.firingMode;
		weaponData_t *wp = GetWeaponData( ent->client->ps.weapon, ent->client->ps.weaponVariation );
		ent->client->ps.firingMode++;
		if(ent->client->ps.firingMode >= wp->numFiringModes)
		{
			ent->client->ps.firingMode = 0;
		}
		ent->client->saberStanceDebounce = level.time + 400;	// changed to 400 cuz 1000 was way too slow
		trap->SendServerCommand(ent->s.number, va("fmrefresh %i", previousFiringMode));
		ent->client->firingModes[ent->client->ps.weaponId] = ent->client->ps.firingMode;
		return;
	}
	else
	{
		// Change saber style then, if permitted
		if( ent->client->ps.saberInFlight )
		{
			//Jedi Knight Galaxies, cannot switch saber style if throwing a saber
			return;
		}

		ent->client->saberStanceDebounce = level.time + 400;	// cut down severely, this was like a whole second before, now it's around half that --eez

		if ( BG_SaberInAttack(ent->client->ps.saberMove) )
		{// Jedi Knight Galaxies: cannot change saber style in mid-attack (todo: fancy chaining shizz)
			return;
		}

		if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
		{ //no cycling for akimbo
			if ( WP_SaberCanTurnOffSomeBlades( &ent->client->saber[1] ) )
			{//can turn second saber off 

				int i = 0;
				int j = 0;

				for(i = ent->client->ps.fd.saberAnimLevel+1, j = 0; j < MAX_STANCES; j++)
				{
					if(i >= MAX_STANCES)
					{
						i = 0;
					}
					if(SaberStances[i].isDualsFriendly)
					{
						if ( ent->client->ps.saberHolstered == 1 )
						{
							// Have one saber holstered, just switch to the style and pretend that nothing happened
							ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
							break;
						}
						else if( ent->client->ps.saberHolstered == 0 )
						{
							// Both sabers are in use, so turn the left one off
							G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOn);
							ent->client->ps.saberHolstered = 1;
							//g_active should take care of this, but...
							ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
							break;
						}
					}
					else if(SaberStances[i].isDualsOnly)
					{
						if ( ent->client->ps.saberHolstered == 1 )
						{
							// Have one saber holstered, bring out your second one like a badass
							G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
							ent->client->ps.saberHolstered = 0;
							//g_active should take care of this, but...
							ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
							break;
						}
						else if( ent->client->ps.saberHolstered == 0 )
						{
							// just switch to this
							ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
							break;
						}
					}
				}

				return;
			}
		}
		else if (ent->client->saber[0].numBlades > 1
			&& WP_SaberCanTurnOffSomeBlades( &ent->client->saber[0] ) )
		{ //use staff stance then.

			for(i = ent->client->ps.fd.saberAnimLevel+1, j = 0; j < MAX_STANCES+1; j++)
			{
				if(i >= MAX_STANCES)
				{
					i = 0;
				}
				if(SaberStances[i].isStaffFriendly)
				{
					if ( ent->client->ps.saberHolstered == 1 )
					{
						// Have one blade holstered, just switch to the style and pretend that nothing happened
						ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
						break;
					}
					else if( ent->client->ps.saberHolstered == 0 )
					{
						// Both sabers are in use, so turn the left one off
						G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOn);
						ent->client->ps.saberHolstered = 1;
						//g_active should take care of this, but...
						ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
						break;
					}
				}
				else if(SaberStances[i].isStaffOnly)
				{
					if ( ent->client->ps.saberHolstered == 1 )
					{
						// Have one blade holstered, bring out your second one like a badass
						G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
						ent->client->ps.saberHolstered = 0;
						//g_active should take care of this, but...
						ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
						break;
					}
					else if( ent->client->ps.saberHolstered == 0 )
					{
						// just switch to this
						ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
						break;
					}
				}
				i++;
				return;
			}
		}
		else
		{
			for(i = ent->client->ps.fd.saberAnimLevel+1, j = 0; j < MAX_STANCES+1; j++)
			{
				if(i >= MAX_STANCES)
				{
					i = 0;
				}
				if(SaberStances[i].saberName_simple[0] &&
					!SaberStances[i].isDualsOnly &&
					!SaberStances[i].isStaffOnly)
				{
					ent->client->ps.fd.saberAnimLevel = ent->client->ps.fd.saberAnimLevelBase = i;
					break;
				}
				i++;
			}
			return;
		}

		if (ent->client->saberCycleQueue)
		{ //resume off of the queue if we haven't gotten a chance to update it yet
			selectLevel = ent->client->saberCycleQueue;
		}
		else
		{
			selectLevel = ent->client->ps.fd.saberAnimLevel;
		}

		selectLevel++;
		if ( selectLevel > ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] )
		{
			selectLevel = FORCE_LEVEL_1;
		}
		if (d_saberStanceDebug.integer)
		{
			trap->SendServerCommand( ent-g_entities, va("print \"SABERSTANCEDEBUG: Attempted to cycle stance normally.\n\"") );
		}
	/*
	#ifndef FINAL_BUILD
		switch ( selectLevel )
		{
		case FORCE_LEVEL_1:
			trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sfast\n\"", S_COLOR_BLUE) );
			break;
		case FORCE_LEVEL_2:
			trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %smedium\n\"", S_COLOR_YELLOW) );
			break;
		case FORCE_LEVEL_3:
			trap->SendServerCommand( ent-g_entities, va("print \"Lightsaber Combat Style: %sstrong\n\"", S_COLOR_RED) );
			break;
		}
	#endif
	*/
		if ( !usingSiegeStyle )
		{
			//make sure it's valid, change it if not
			WP_UseFirstValidSaberStyle( &ent->client->saber[0], &ent->client->saber[1], ent->client->ps.saberHolstered, &selectLevel );
		}

		if (ent->client->ps.weaponTime <= 0)
		{ //not busy, set it now
			ent->client->ps.fd.saberAnimLevelBase = ent->client->ps.fd.saberAnimLevel = selectLevel;
		}
		else
		{ //can't set it now or we might cause unexpected chaining, so queue it
			ent->client->ps.fd.saberAnimLevelBase = ent->client->saberCycleQueue = selectLevel;
		}
	}
}

qboolean G_OtherPlayersDueling(void)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent && ent->inuse && ent->client && ent->client->ps.duelInProgress)
		{
			return qtrue;
		}
		i++;
	}

	return qfalse;
}

void Cmd_EngageDuel_f(gentity_t *ent)
{
	trace_t tr;
	vec3_t forward, fwdOrg;

	if (!g_privateDuel.integer)
	{
		return;
	}

	if (level.gametype == GT_DUEL || level.gametype == GT_POWERDUEL)
	{ //rather pointless in this mode..
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (level.gametype >= GT_TEAM)
	{ //no private dueling in team modes
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NODUEL_GAMETYPE")) );
		return;
	}

	if (ent->client->ps.duelTime >= level.time)
	{
		return;
	}

	if (ent->client->ps.weapon != WP_SABER)
	{
		return;
	}

	/*
	if (!ent->client->ps.saberHolstered)
	{ //must have saber holstered at the start of the duel
		return;
	}
	*/
	//NOTE: No longer doing this..

	if (ent->client->ps.saberInFlight)
	{
		return;
	}

	if (ent->client->ps.duelInProgress)
	{
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );

	fwdOrg[0] = ent->client->ps.origin[0] + forward[0]*256;
	fwdOrg[1] = ent->client->ps.origin[1] + forward[1]*256;
	fwdOrg[2] = (ent->client->ps.origin[2]+ent->client->ps.viewheight) + forward[2]*256;

	trap->Trace(&tr, ent->client->ps.origin, NULL, NULL, fwdOrg, ent->s.number, MASK_PLAYERSOLID, 0, 0, 0);

	if (tr.fraction != 1 && tr.entityNum < MAX_CLIENTS)
	{
		gentity_t *challenged = &g_entities[tr.entityNum];

		if (!challenged || !challenged->client || !challenged->inuse ||
			challenged->health < 1 || challenged->client->ps.stats[STAT_HEALTH] < 1 ||
			challenged->client->ps.weapon != WP_SABER || challenged->client->ps.duelInProgress ||
			challenged->client->ps.saberInFlight)
		{
			return;
		}

		if (level.gametype >= GT_TEAM && OnSameTeam(ent, challenged))
		{
			return;
		}

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap->SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname));		//--futuza note: test this, make sure I didn't break dueling with my RGB fix

			ent->client->ps.duelInProgress = qtrue;
			challenged->client->ps.duelInProgress = qtrue;

			ent->client->ps.duelTime = level.time + 2000;
			challenged->client->ps.duelTime = level.time + 2000;

			G_AddEvent(ent, EV_PRIVATE_DUEL, 1);
			G_AddEvent(challenged, EV_PRIVATE_DUEL, 1);

			//Holster their sabers now, until the duel starts (then they'll get auto-turned on to look cool)

			if (!ent->client->ps.saberHolstered)
			{
				if (ent->client->saber[0].soundOff)
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[0].soundOff);
				}
				if (ent->client->saber[1].soundOff &&
					ent->client->saber[1].model[0])
				{
					G_Sound(ent, CHAN_AUTO, ent->client->saber[1].soundOff);
				}
				ent->client->ps.weaponTime = 400;
				ent->client->ps.saberHolstered = 2;
			}
			if (!challenged->client->ps.saberHolstered)
			{
				if (challenged->client->saber[0].soundOff)
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[0].soundOff);
				}
				if (challenged->client->saber[1].soundOff &&
					challenged->client->saber[1].model[0])
				{
					G_Sound(challenged, CHAN_AUTO, challenged->client->saber[1].soundOff);
				}
				challenged->client->ps.weaponTime = 400;
				challenged->client->ps.saberHolstered = 2;
			}
		}
		else
		{
			//Print the message that a player has been challenged in private, only announce the actual duel initiation in private
			trap->SendServerCommand(challenged - g_entities, va("cp \"%s %s\n\"", ent->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGE")));
			trap->SendServerCommand(ent - g_entities, va("cp \"%s %s\n\"", G_GetStringEdString("MP_SVGAME", "PLDUELCHALLENGED"), challenged->client->pers.netname));
		}

		challenged->client->ps.fd.privateDuelTime = 0; //reset the timer in case this player just got out of a duel. He should still be able to accept the challenge.

		ent->client->ps.forceHandExtend = HANDEXTEND_DUELCHALLENGE;
		ent->client->ps.forceHandExtendTime = level.time + 1000;

		ent->client->ps.duelIndex = challenged->s.number;
		ent->client->ps.duelTime = level.time + 5000;
	}
}

#ifndef FINAL_BUILD
extern stringID_table_t animTable[MAX_ANIMATIONS+1];

void Cmd_DebugSetSaberMove_f(gentity_t *self)
{
	int argNum = trap->Argc();
	char arg[MAX_STRING_CHARS];

	if (argNum < 2)
	{
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	self->client->ps.saberMove = atoi(arg);
	self->client->ps.saberBlocked = BLOCKED_BOUNCE_MOVE;

	if (self->client->ps.saberMove >= LS_MOVE_MAX)
	{
		self->client->ps.saberMove = LS_MOVE_MAX-1;
	}

	Com_Printf("Anim for move: %s\n", animTable[SaberStances[self->client->ps.fd.saberAnimLevel].moves[self->client->ps.saberMove].anim].name);
}

void Cmd_DebugSetBodyAnim_f(gentity_t *self, int flags)
{
	int argNum = trap->Argc();
	char arg[MAX_STRING_CHARS];
	int i = 0;

	if (argNum < 2)
	{
		return;
	}

	trap->Argv( 1, arg, sizeof( arg ) );

	if (!arg[0])
	{
		return;
	}

	while (i < MAX_ANIMATIONS)
	{
		if (!Q_stricmp(arg, animTable[i].name))
		{
			break;
		}
		i++;
	}

	if (i == MAX_ANIMATIONS)
	{
		Com_Printf("Animation '%s' does not exist\n", arg);
		return;
	}

	G_SetAnim(self, NULL, SETANIM_BOTH, i, flags);

	Com_Printf("Set body anim to %s\n", arg);
}
#endif

void StandardSetBodyAnim(gentity_t *self, int anim, int flags)
{
	G_SetAnim(self, NULL, SETANIM_BOTH, anim, flags);
}

void DismembermentTest(gentity_t *self);

void Bot_SetForcedMovement(int bot, int forward, int right, int up);

#ifndef FINAL_BUILD
extern void DismembermentByNum(gentity_t *self, int num);
extern void G_SetVehDamageFlags( gentity_t *veh, int shipSurf, int damageLevel );
#endif

static int G_ClientNumFromNetname(char *name)
{
	int i = 0;
	gentity_t *ent;

	while (i < MAX_CLIENTS)
	{
		ent = &g_entities[i];

		if (ent->inuse && ent->client &&
			!Q_stricmp(ent->client->pers.netname, name))
		{
			return ent->s.number;
		}
		i++;
	}

	return -1;
}

qboolean TryGrapple(gentity_t *ent)
{
	if (ent->client->ps.weaponTime > 0)
	{ //weapon busy
		return qfalse;
	}
	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE)
	{ //force power or knockdown or something
		return qfalse;
	}
	if (ent->client->grappleState)
	{ //already grappling? but weapontime should be > 0 then..
		return qfalse;
	}

	if (ent->client->ps.weapon != WP_SABER && ent->client->ps.weapon != WP_MELEE)
	{
		return qfalse;
	}

	if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
	{
		Cmd_ToggleSaber_f(ent);
		if (!ent->client->ps.saberHolstered)
		{ //must have saber holstered
			return qfalse;
		}
	}

	//G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_PA_1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	G_SetAnim(ent, &ent->client->pers.cmd, SETANIM_BOTH, BOTH_KYLE_GRAB, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	if (ent->client->ps.torsoAnim == BOTH_KYLE_GRAB)
	{ //providing the anim set succeeded..
		ent->client->ps.torsoTimer += 500; //make the hand stick out a little longer than it normally would
		if (ent->client->ps.legsAnim == ent->client->ps.torsoAnim)
		{
			ent->client->ps.legsTimer = ent->client->ps.torsoTimer;
		}
		ent->client->ps.weaponTime = ent->client->ps.torsoTimer;
		return qtrue;
	}

	return qfalse;
}

#ifndef FINAL_BUILD
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
#endif

/*
=================
ClientCommand
=================
*/

#include "jkg_threading.h"

void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;		// not fully in game yet
	}


	trap->Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	// IMPORTANT!!
	// DO NOT LET CONNECTING CLIENTS GO BEYOND THIS POINT!
	if (level.clients[clientNum].pers.connected != CON_CONNECTED)
		return;

	// Check for Glua bound commands
	if (GLua_Command(clientNum, cmd)) return;

	// Check team commands.
	if ( TeamCommand( clientNum, cmd, NULL )) return;

	if (!Q_stricmp( cmd, "taskreport" )) {
		JKG_PrintTasksTable( clientNum );
		return;
	}

	if (Q_stricmp (cmd, "say") == 0) {
		Cmd_Say_f (ent, SAY_ALL, qfalse);
		return;
	}


	if (Q_stricmp (cmd, "sayact") == 0) {
		Cmd_SayAct_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "sayglobal") == 0) {
		Cmd_SayGlobal_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "sayyell") == 0) {
		Cmd_SayYell_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "saywhisper") == 0) {
		Cmd_SayWhisper_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "say_team") == 0) {
		if (g_gametype.integer < GT_TEAM)
		{ //not a team game, just refer to regular say.
			Cmd_Say_f (ent, SAY_ALL, qfalse);
		}
		else
		{
			Cmd_Say_f (ent, SAY_TEAM, qfalse);
		}
		return;
	}
	if (Q_stricmp (cmd, "tell") == 0) {
		Cmd_Tell_f ( ent );
		return;
	}
	if (Q_stricmp (cmd, "togglesaber") == 0)
	{
		Cmd_ToggleSaber_f(ent);
		return;
	}

	if (!Q_stricmp (cmd, "crystal1"))
	{
		JKG_Crystal_f(ent, 0);
		return;
	}

	if(!Q_stricmp (cmd, "crystal2"))
	{
		JKG_Crystal_f(ent, 1);
		return;
	}

	if (!Q_stricmp(cmd, "resendInv")) {
		BG_SendItemPacket(IPT_RESET, ent, nullptr, ent->inventory->size(), 0);
		return;
	}

	if (Q_stricmp(cmd, "voice_cmd") == 0)
	{
		Cmd_VoiceCommand_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "score") == 0) {
		Cmd_Score_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "printweaponlist_sv") == 0) {
		JKG_PrintWeaponList_f(ent);
		return;
	}

	if (Q_stricmp (cmd, "dumpweaponlist_sv") == 0) {
		JKG_DumpWeaponList_f(ent);
		return;
	}

	if(Q_stricmp(cmd, "itemLookup") == 0) {
		JKG_ItemLookup_f(ent);
		return;
	}
	if(Q_stricmp(cmd, "itemCheck") == 0) {
		JKG_ItemCheck_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "tcEval") == 0) {
		Cmd_TcEval_f(ent);
		return;
	}
	if (Q_stricmp(cmd, "tcEvalMulti") == 0) {
		Cmd_TcEval_f(ent, true);
		return;
	}

	// ignore all other commands when at intermission
	if (level.intermissiontime)
	{
		qboolean giveError = qfalse;
		//rwwFIXMEFIXME: This is terrible, write it differently

		if (!Q_stricmp(cmd, "give"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "giveother"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "god"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "notarget"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "noclip"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "kill"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamtask"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "levelshot"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follow"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "follownext"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "followprev"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "team"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "duelteam"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "siegeclass"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "forcechanged"))
		{ //special case: still update force change
			Cmd_ForceChanged_f (ent);
			return;
		}
		else if (!Q_stricmp(cmd, "where"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "vote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "callteamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "teamvote"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "gc"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "setviewpos"))
		{
			giveError = qtrue;
		}
		else if (!Q_stricmp(cmd, "stats"))
		{
			giveError = qtrue;
		}
		// Jedi Knight Galaxies begin
		else if (!Q_stricmp(cmd, "buyVendor"))
		{
			giveError = qtrue;
		}
		// Jedi Knight Galaxies end

		if (giveError)
		{
			trap->SendServerCommand( clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd ) );
		}
		else
		{
			Cmd_Say_f (ent, qfalse, qtrue);
		}
		return;
	}

	if (Q_stricmp (cmd, "give") == 0)
	{
		Cmd_Give_f (ent, 0);
	}
	else if (Q_stricmp (cmd, "giveother") == 0)
	{ //for debugging pretty much
		Cmd_Give_f (ent, 1);
	}
	// Jedi Knight Galaxies begin
	else if (Q_stricmp (cmd, "credits") == 0)
	{
		//DEBUG: Show how many credits you have
		int credits = ent->client->ps.credits;
		trap->SendServerCommand(clientNum, va("print \"You have %i credits, %s.\n\"", Q_max(0, credits), ent->client->pers.netname));
		return;
	}
	else if ( Q_stricmp (cmd, "closeVendor") == 0 )
	{
		JKG_CloseVendor_f (ent);
	}
	else if (!Q_stricmp(cmd, "buyVendor"))
	{
		JKG_BuyItem_f(ent);
		return;
	}
	// Jedi Knight Galaxies end
	else if (Q_stricmp (cmd, "t_use") == 0 && CheatsOk(ent))
	{ //debug use map object
		if (trap->Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			gentity_t *targ;

			trap->Argv( 1, sArg, sizeof( sArg ) );
			targ = G_Find( NULL, FOFS(targetname), sArg );

			while (targ)
			{
				if (targ->use)
				{
					targ->use(targ, ent, ent);
				}
				targ = G_Find( targ, FOFS(targetname), sArg );
			}
		}
	}
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if ( Q_stricmp( cmd, "NPC" ) == 0 && CheatsOk(ent) )
	{
		Cmd_NPC_f( ent );
	}
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	#ifdef _DEBUG
	else if (Q_stricmp (cmd, "levelshot") == 0)
		Cmd_LevelShot_f (ent);
	#endif
	else if (Q_stricmp (cmd, "follow") == 0)
		Cmd_Follow_f (ent);
	else if (Q_stricmp (cmd, "follownext") == 0)
		Cmd_FollowCycle_f (ent, 1);
	else if (Q_stricmp (cmd, "followprev") == 0)
		Cmd_FollowCycle_f (ent, -1);
	else if (Q_stricmp (cmd, "team") == 0)
		Cmd_Team_f (ent);
	else if (Q_stricmp (cmd, "duelteam") == 0)
		Cmd_DuelTeam_f (ent);
	else if (Q_stricmp (cmd, "forcechanged") == 0)
		Cmd_ForceChanged_f (ent);
	else if (Q_stricmp (cmd, "where") == 0)
		Cmd_Where_f (ent);
	else if (Q_stricmp (cmd, "callvote") == 0)
		Cmd_CallVote_f (ent);
	else if (Q_stricmp (cmd, "vote") == 0)
		Cmd_Vote_f (ent);
	else if (Q_stricmp (cmd, "callteamvote") == 0)
		Cmd_CallTeamVote_f (ent);
	else if (Q_stricmp (cmd, "teamvote") == 0)
		Cmd_TeamVote_f (ent);
	else if (Q_stricmp (cmd, "gc") == 0)
		Cmd_GameCommand_f( ent );
	else if (Q_stricmp (cmd, "setviewpos") == 0)
		Cmd_SetViewpos_f( ent );
#ifdef __AUTOWAYPOINT__ // __DOMINANCE_NPC__
	else if (Q_stricmp (cmd, "checkbotreach") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{
		AIMod_CheckMapPaths( ent );
	}
	else if (Q_stricmp (cmd, "checkobjectivesreach") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{
		AIMod_CheckObjectivePaths( ent );
	}
	else if (Q_stricmp (cmd, "closeentities") == 0 && !Q_strncmp(ent->client->sess.IP, "127.0.0.1:",10))	{

		int i = 0;

		trap->Print("----------------------------------------------\n");
		trap->Print("Entities Within Range.\n");
		trap->Print("----------------------------------------------\n");
		
		for (i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
		{
			gentity_t *item = &g_entities[i];
			float dist = 0.0f;

			if (!item) continue;
			if (!item->classname || !item->classname[0]) continue;

			dist = Distance(ent->r.currentOrigin, item->s.origin);

			if (dist > 256) continue;

			trap->Print("Entity %i", i);

			switch ( item->s.eType ) {
			case ET_GENERAL:
				trap->Print("[ET_GENERAL         ]");
				break;
			case ET_PLAYER:
				trap->Print("[ET_PLAYER          ]");
				break;
			case ET_ITEM:
				trap->Print("[ET_ITEM            ]");
				break;
			case ET_MISSILE:
				trap->Print("[ET_MISSILE         ]");
				break;
			case ET_MOVER:
				trap->Print("[ET_MOVER           ]");
				break;
			case ET_BEAM:
				trap->Print("[ET_BEAM            ]");
				break;
			case ET_PORTAL:
				trap->Print("[ET_PORTAL          ]");
				break;
			case ET_SPEAKER:
				trap->Print("[ET_SPEAKER         ]");
				break;
			case ET_PUSH_TRIGGER:
				trap->Print("[ET_PUSH_TRIGGER    ]");
				break;
			case ET_TELEPORT_TRIGGER:
				trap->Print("[ET_TELEPORT_TRIGGER]");
				break;
			case ET_INVISIBLE:
				trap->Print("[ET_INVISIBLE       ]");
				break;
			case ET_NPC:
				trap->Print("[ET_NPC             ]");
				break;
			default:
				trap->Print("[%3i                ]", item->s.eType);
				break;
			}

			trap->Print(" [%s] at a distance of %f.\n", item->classname, dist);
		}

		trap->Print("----------------------------------------------\n");
	}
#endif //__AUTOWAYPOINT__ // __DOMINANCE_NPC__
	//for convenient powerduel testing in release
	else if (Q_stricmp(cmd, "killother") == 0 && CheatsOk( ent ))
	{
		if (trap->Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap->Argv( 1, sArg, sizeof( sArg ) );

			entNum = G_ClientNumFromNetname(sArg);

			if (entNum >= 0 && entNum < MAX_GENTITIES)
			{
				gentity_t *kEnt = &g_entities[entNum];

				if (kEnt->inuse && kEnt->client)
				{
					kEnt->flags &= ~FL_GODMODE;
					kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
					player_die (kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
				}
			}
		}
	}
#ifdef _DEBUG
	else if (Q_stricmp(cmd, "relax") == 0 && CheatsOk( ent ))
	{
		if (ent->client->ps.eFlags & EF_RAG)
		{
			ent->client->ps.eFlags &= ~EF_RAG;
		}
		else
		{
			ent->client->ps.eFlags |= EF_RAG;
		}
	}
	else if (Q_stricmp(cmd, "holdme") == 0 && CheatsOk( ent ))
	{
		if (trap->Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int entNum = 0;

			trap->Argv( 1, sArg, sizeof( sArg ) );

			entNum = atoi(sArg);

			if (entNum >= 0 &&
				entNum < MAX_GENTITIES)
			{
				gentity_t *grabber = &g_entities[entNum];

				if (grabber->inuse && grabber->client && grabber->ghoul2)
				{
					if (!grabber->s.number)
					{ //switch cl 0 and entitynum_none, so we can operate on the "if non-0" concept
						ent->client->ps.ragAttach = ENTITYNUM_NONE;
					}
					else
					{
						ent->client->ps.ragAttach = grabber->s.number;
					}
				}
			}
		}
		else
		{
			ent->client->ps.ragAttach = 0;
		}
	}
	else if (Q_stricmp(cmd, "limb_break") == 0 && CheatsOk( ent ))
	{
		if (trap->Argc() > 1)
		{
			char sArg[MAX_STRING_CHARS];
			int breakLimb = 0;

			trap->Argv( 1, sArg, sizeof( sArg ) );
			if (!Q_stricmp(sArg, "right"))
			{
				breakLimb = BROKENLIMB_RARM;
			}
			else if (!Q_stricmp(sArg, "left"))
			{
				breakLimb = BROKENLIMB_LARM;
			}

			G_BreakArm(ent, breakLimb);
		}
	}
	else if (Q_stricmp(cmd, "headexplodey") == 0 && CheatsOk( ent ))
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			DismembermentTest(ent);
		}
	}
	else if (Q_stricmp(cmd, "debugstupidthing") == 0 && CheatsOk( ent ))
	{
		int i = 0;
		gentity_t *blah;
		while (i < MAX_GENTITIES)
		{
			blah = &g_entities[i];
			if (blah->inuse && blah->classname && blah->classname[0] && !Q_stricmp(blah->classname, "NPC_Vehicle"))
			{
				Com_Printf("Found it.\n");
			}
			i++;
		}
	}
	else if (Q_stricmp(cmd, "arbitraryprint") == 0 && CheatsOk( ent ))
	{
		trap->SendServerCommand( -1, va("cp \"Blah blah blah\n\""));
	}
	else if (Q_stricmp(cmd, "handcut") == 0 && CheatsOk( ent ))
	{
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		if (trap->Argc() > 1)
		{
			trap->Argv( 1, sarg, sizeof( sarg ) );

			if (sarg[0])
			{
				bCl = atoi(sarg);

				if (bCl >= 0 && bCl < MAX_GENTITIES)
				{
					gentity_t *hEnt = &g_entities[bCl];

					if (hEnt->client)
					{
						if (hEnt->health > 0)
						{
							gGAvoidDismember = 1;
							hEnt->flags &= ~FL_GODMODE;
							hEnt->client->ps.stats[STAT_HEALTH] = hEnt->health = -999;
							player_die (hEnt, hEnt, hEnt, 100000, MOD_SUICIDE);
						}
						gGAvoidDismember = 2;
						G_CheckForDismemberment(hEnt, ent, hEnt->client->ps.origin, 999, hEnt->client->ps.legsAnim, qfalse);
						gGAvoidDismember = 0;
					}
				}
			}
		}
	}
	else if (Q_stricmp(cmd, "loveandpeace") == 0 && CheatsOk( ent ))
	{
		trace_t tr;
		vec3_t fPos;

		AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

		fPos[0] = ent->client->ps.origin[0] + fPos[0]*40;
		fPos[1] = ent->client->ps.origin[1] + fPos[1]*40;
		fPos[2] = ent->client->ps.origin[2] + fPos[2]*40;

		trap->Trace(&tr, ent->client->ps.origin, 0, 0, fPos, ent->s.number, ent->clipmask, 0, 0, 0);

		if (tr.entityNum < MAX_CLIENTS && tr.entityNum != ent->s.number)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other && other->inuse && other->client)
			{
				vec3_t entDir;
				vec3_t otherDir;
				vec3_t entAngles;
				vec3_t otherAngles;

				if (ent->client->ps.weapon == WP_SABER && !ent->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(ent);
				}

				if (other->client->ps.weapon == WP_SABER && !other->client->ps.saberHolstered)
				{
					Cmd_ToggleSaber_f(other);
				}

				if ((ent->client->ps.weapon != WP_SABER || ent->client->ps.saberHolstered) &&
					(other->client->ps.weapon != WP_SABER || other->client->ps.saberHolstered))
				{
					VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
					VectorCopy( ent->client->ps.viewangles, entAngles );
					entAngles[YAW] = vectoyaw( otherDir );
					SetClientViewAngle( ent, entAngles );

					StandardSetBodyAnim(ent, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					ent->client->ps.saberMove = LS_NONE;
					ent->client->ps.saberBlocked = 0;
					ent->client->ps.saberBlocking = 0;

					VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
					VectorCopy( other->client->ps.viewangles, otherAngles );
					otherAngles[YAW] = vectoyaw( entDir );
					SetClientViewAngle( other, otherAngles );

					StandardSetBodyAnim(other, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD|SETANIM_FLAG_HOLDLESS);
					other->client->ps.saberMove = LS_NONE;
					other->client->ps.saberBlocked = 0;
					other->client->ps.saberBlocking = 0;
				}
			}
		}
	}
#endif
	else if (Q_stricmp(cmd, "thedestroyer") == 0 && CheatsOk( ent ) && ent && ent->client && ent->client->ps.saberHolstered && ent->client->ps.weapon == WP_SABER)
	{
		Cmd_ToggleSaber_f(ent);

		if (!ent->client->ps.saberHolstered)
		{
		}
	}
	//begin bot debug cmds
	else if (Q_stricmp(cmd, "debugBMove_Forward") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap->Argc() > 1);
		trap->Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Back") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap->Argc() > 1);
		trap->Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, arg, -1, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Right") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap->Argc() > 1);
		trap->Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Left") == 0 && CheatsOk(ent))
	{
		int arg = -4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap->Argc() > 1);
		trap->Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, arg, -1);
	}
	else if (Q_stricmp(cmd, "debugBMove_Up") == 0 && CheatsOk(ent))
	{
		int arg = 4000;
		int bCl = 0;
		char sarg[MAX_STRING_CHARS];

		assert(trap->Argc() > 1);
		trap->Argv( 1, sarg, sizeof( sarg ) );

		assert(sarg[0]);
		bCl = atoi(sarg);
		Bot_SetForcedMovement(bCl, -1, -1, arg);
	}
	//end bot debug cmds
/*#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugSetSaberMove") == 0)
	{
		Cmd_DebugSetSaberMove_f(ent);
	}
	else if (Q_stricmp(cmd, "debugSetBodyAnim") == 0)
	{
		Cmd_DebugSetBodyAnim_f(ent, SETANIM_FLAG_OVERRIDE|SETANIM_FLAG_HOLD);
	}
	else if (Q_stricmp(cmd, "debugDismemberment") == 0)
	{
		Cmd_Kill_f (ent);
		if (ent->health < 1)
		{
			char	arg[MAX_STRING_CHARS];
			int		iArg = 0;

			if (trap->Argc() > 1)
			{
				trap->Argv( 1, arg, sizeof( arg ) );

				if (arg[0])
				{
					iArg = atoi(arg);
				}
			}

			DismembermentByNum(ent, iArg);
		}
	}
	else if (Q_stricmp(cmd, "debugDropSaber") == 0)
	{
		if (ent->client->ps.weapon == WP_SABER &&
			ent->client->ps.saberEntityNum &&
			!ent->client->ps.saberInFlight)
		{
			saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
		}
	}
	else if (Q_stricmp(cmd, "debugKnockMeDown") == 0)
	{
		if (BG_KnockDownable(&ent->client->ps))
		{
			ent->client->ps.forceHandExtend = HANDEXTEND_KNOCKDOWN;
			ent->client->ps.forceDodgeAnim = 0;
			if (trap->Argc() > 1)
			{
				ent->client->ps.forceHandExtendTime = level.time + 1100;
				ent->client->ps.quickerGetup = qfalse;
			}
			else
			{
				ent->client->ps.forceHandExtendTime = level.time + 700;
				ent->client->ps.quickerGetup = qtrue;
			}
		}
	}
	else if (Q_stricmp(cmd, "debugSaberSwitch") == 0)
	{
		gentity_t *targ = NULL;

		if (trap->Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap->Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			Cmd_ToggleSaber_f(targ);
		}
	}
	else if (Q_stricmp(cmd, "debugIKGrab") == 0)
	{
		gentity_t *targ = NULL;

		if (trap->Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap->Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			targ->client->ps.heldByClient = ent->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKBeGrabbedBy") == 0)
	{
		gentity_t *targ = NULL;

		if (trap->Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap->Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client && ent->s.number != targ->s.number)
		{
			ent->client->ps.heldByClient = targ->s.number+1;
		}
	}
	else if (Q_stricmp(cmd, "debugIKRelease") == 0)
	{
		gentity_t *targ = NULL;

		if (trap->Argc() > 1)
		{
			char	arg[MAX_STRING_CHARS];

			trap->Argv( 1, arg, sizeof( arg ) );

			if (arg[0])
			{
				int x = atoi(arg);
				
				if (x >= 0 && x < MAX_CLIENTS)
				{
					targ = &g_entities[x];
				}
			}
		}

		if (targ && targ->inuse && targ->client)
		{
			targ->client->ps.heldByClient = 0;
		}
	}
	else if (Q_stricmp(cmd, "debugThrow") == 0)
	{
		trace_t tr;
		vec3_t tTo, fwd;

		if (ent->client->ps.weaponTime > 0 || ent->client->ps.forceHandExtend != HANDEXTEND_NONE ||
			ent->client->ps.groundEntityNum == ENTITYNUM_NONE || ent->health < 1)
		{
			return;
		}

		AngleVectors(ent->client->ps.viewangles, fwd, 0, 0);
		tTo[0] = ent->client->ps.origin[0] + fwd[0]*32;
		tTo[1] = ent->client->ps.origin[1] + fwd[1]*32;
		tTo[2] = ent->client->ps.origin[2] + fwd[2]*32;

		trap->Trace(&tr, ent->client->ps.origin, 0, 0, tTo, ent->s.number, MASK_PLAYERSOLID);

		if (tr.fraction != 1)
		{
			gentity_t *other = &g_entities[tr.entityNum];

			if (other->inuse && other->client && other->client->ps.forceHandExtend == HANDEXTEND_NONE &&
				other->client->ps.groundEntityNum != ENTITYNUM_NONE && other->health > 0 &&
				(int)ent->client->ps.origin[2] == (int)other->client->ps.origin[2])
			{
				float pDif = 40.0f;
				vec3_t entAngles, entDir;
				vec3_t otherAngles, otherDir;
				vec3_t intendedOrigin;
				vec3_t boltOrg, pBoltOrg;
				vec3_t tAngles, vDif;
				vec3_t fwd, right;
				trace_t tr;
				trace_t tr2;

				VectorSubtract( other->client->ps.origin, ent->client->ps.origin, otherDir );
				VectorCopy( ent->client->ps.viewangles, entAngles );
				entAngles[YAW] = vectoyaw( otherDir );
				SetClientViewAngle( ent, entAngles );

				ent->client->ps.forceHandExtend = HANDEXTEND_PRETHROW;
				ent->client->ps.forceHandExtendTime = level.time + 5000;

				ent->client->throwingIndex = other->s.number;
				ent->client->doingThrow = level.time + 5000;
				ent->client->beingThrown = 0;

				VectorSubtract( ent->client->ps.origin, other->client->ps.origin, entDir );
				VectorCopy( other->client->ps.viewangles, otherAngles );
				otherAngles[YAW] = vectoyaw( entDir );
				SetClientViewAngle( other, otherAngles );

				other->client->ps.forceHandExtend = HANDEXTEND_PRETHROWN;
				other->client->ps.forceHandExtendTime = level.time + 5000;

				other->client->throwingIndex = ent->s.number;
				other->client->beingThrown = level.time + 5000;
				other->client->doingThrow = 0;

				//Doing this now at a stage in the throw, isntead of initially.
				//other->client->ps.heldByClient = ent->s.number+1;

				G_EntitySound( other, CHAN_VOICE, G_SoundIndex("*pain100.wav") );
				G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*jump1.wav") );
				G_Sound(other, CHAN_AUTO, G_SoundIndex( "sound/movers/objects/objectHit.wav" ));

				//see if we can move to be next to the hand.. if it's not clear, break the throw.
				VectorClear(tAngles);
				tAngles[YAW] = ent->client->ps.viewangles[YAW];
				VectorCopy(ent->client->ps.origin, pBoltOrg);
				AngleVectors(tAngles, fwd, right, 0);
				boltOrg[0] = pBoltOrg[0] + fwd[0]*8 + right[0]*pDif;
				boltOrg[1] = pBoltOrg[1] + fwd[1]*8 + right[1]*pDif;
				boltOrg[2] = pBoltOrg[2];

				VectorSubtract(boltOrg, pBoltOrg, vDif);
				VectorNormalize(vDif);

				VectorClear(other->client->ps.velocity);
				intendedOrigin[0] = pBoltOrg[0] + vDif[0]*pDif;
				intendedOrigin[1] = pBoltOrg[1] + vDif[1]*pDif;
				intendedOrigin[2] = other->client->ps.origin[2];

				trap->Trace(&tr, intendedOrigin, other->r.mins, other->r.maxs, intendedOrigin, other->s.number, other->clipmask);
				trap->Trace(&tr2, ent->client->ps.origin, ent->r.mins, ent->r.maxs, intendedOrigin, ent->s.number, CONTENTS_SOLID);

				if (tr.fraction == 1.0f && !tr.startsolid && tr2.fraction == 1.0f && !tr2.startsolid)
				{
					VectorCopy(intendedOrigin, other->client->ps.origin);
				}
				else
				{ //if the guy can't be put here then it's time to break the throw off.
					vec3_t oppDir;
					int strength = 4;

					other->client->ps.heldByClient = 0;
					other->client->beingThrown = 0;
					ent->client->doingThrow = 0;

					ent->client->ps.forceHandExtend = HANDEXTEND_NONE;
					G_EntitySound( ent, CHAN_VOICE, G_SoundIndex("*pain25.wav") );

					other->client->ps.forceHandExtend = HANDEXTEND_NONE;
					VectorSubtract(other->client->ps.origin, ent->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					other->client->ps.velocity[0] = oppDir[0]*(strength*40);
					other->client->ps.velocity[1] = oppDir[1]*(strength*40);
					other->client->ps.velocity[2] = 150;

					VectorSubtract(ent->client->ps.origin, other->client->ps.origin, oppDir);
					VectorNormalize(oppDir);
					ent->client->ps.velocity[0] = oppDir[0]*(strength*40);
					ent->client->ps.velocity[1] = oppDir[1]*(strength*40);
					ent->client->ps.velocity[2] = 150;
				}
			}
		}
	}
#endif
#ifdef VM_MEMALLOC_DEBUG
	else if (Q_stricmp(cmd, "debugTestAlloc") == 0)
	{ //rww - small routine to stress the malloc trap stuff and make sure nothing bad is happening.
		char *blah;
		int i = 1;
		int x;

		//stress it. Yes, this will take a while. If it doesn't explode miserably in the process.
		while (i < 32768)
		{
			x = 0;

			trap->TrueMalloc((void **)&blah, i);
			if (!blah)
			{ //pointer is returned null if allocation failed
				trap->SendServerCommand( -1, va("print \"Failed to alloc at %i!\n\"", i));
				break;
			}
			while (x < i)
			{ //fill the allocated memory up to the edge
				if (x+1 == i)
				{
					blah[x] = 0;
				}
				else
				{
					blah[x] = 'A';
				}
				x++;
			}
			trap->TrueFree((void **)&blah);
			if (blah)
			{ //should be nullified in the engine after being freed
				trap->SendServerCommand( -1, va("print \"Failed to free at %i!\n\"", i));
				break;
			}

			i++;
		}

		trap->SendServerCommand( -1, "print \"Finished allocation test\n\"");
	}
#endif
#ifndef FINAL_BUILD
	else if (Q_stricmp(cmd, "debugShipDamage") == 0)
	{
		char	arg[MAX_STRING_CHARS];
		char	arg2[MAX_STRING_CHARS];
		int		shipSurf, damageLevel;

		trap->Argv( 1, arg, sizeof( arg ) );
		trap->Argv( 2, arg2, sizeof( arg2 ) );
		shipSurf = SHIPSURF_FRONT+atoi(arg);
		damageLevel = atoi(arg2);

		G_SetVehDamageFlags( &g_entities[ent->s.m_iVehicleNum], shipSurf, damageLevel );
	}
	else if (Q_stricmp(cmd, "debugHealth") == 0)
	{
		trap->SendServerCommand( clientNum, va("print \"%i \n\"", ent->client->ps.stats[STAT_HEALTH]));
		return;
	}*/
	else if (Q_stricmp(cmd, "debugInventory") == 0)
	{
		JKG_Cmd_ShowInv_f(ent);
		return;
	}
/*#endif
	else if (Q_stricmp(cmd, "bodyLoot") == 0)
	{
		char arg[MAX_STRING_CHARS];
		char arg2[MAX_STRING_CHARS];
		if(trap->Argc() < 3) //not enough args
		{
			trap->SendServerCommand( clientNum, "print \"Syntax: /bodyLoot <entity num> <loot ID>\n\"" );
			return;
		}
		trap->Argv( 1, arg, sizeof ( arg ) );
		trap->Argv( 2, arg2, sizeof ( arg2 ) );
		JKG_Cmd_Loot_f( ent, atoi( arg ), atoi( arg2 ), qfalse );
		return;
	}*/
	else if (Q_stricmp(cmd, "inventoryUse") == 0)
	{
		char arg[MAX_STRING_CHARS];
		if(trap->Argc() < 2)
		{
			trap->SendServerCommand( clientNum, "print \"Syntax: /inventoryUse <item num>\n\"" );
			return;
		}
		trap->Argv( 1, arg, sizeof(arg) );
		JKG_Cmd_ItemAction_f( ent, atoi( arg ) );
		return;
	}
	else if (Q_stricmp(cmd, "equip") == 0)
	{
		JKG_Cmd_EquipItem_f (ent);
	}
	else if (Q_stricmp(cmd, "unequip") == 0)
	{
		JKG_Cmd_UnequipItem_f (ent);
	}
	else if (Q_stricmp(cmd, "inventoryDestroy") == 0)
	{
		JKG_Cmd_DestroyItem_f(ent);
	}
	else if (Q_stricmp(cmd, "inventorySell") == 0)
	{
		JKG_Cmd_SellItem_f(ent);
	}
	else if (Q_stricmp(cmd, "testSetSaber1") == 0)
	{
		char str[MAX_TOKEN_CHARS];

		trap->Argv(1, str, sizeof(str));

		G_SetSaber( ent, 0, str, false );
	}
	//
	// BEGIN - Warzone Gametype Editing...
	//
	else if(!Q_stricmp (cmd, "warzone_savegameinfo") && CheatsOk( ent )) 
	{
		WARZONE_SaveGameInfo();
	} 
	else if(!Q_stricmp (cmd, "warzone_loadgameinfo") && CheatsOk( ent )) 
	{
		WARZONE_LoadGameInfo();
	} 
	else if(!Q_stricmp (cmd, "warzone_addhealthcrate") && CheatsOk( ent )) 
	{
		WARZONE_AddHealthCrate( ent->r.currentOrigin, ent->r.currentAngles );
	} 
	else if(!Q_stricmp (cmd, "warzone_addammocrate") && CheatsOk( ent )) 
	{
		WARZONE_AddAmmoCrate( ent->r.currentOrigin, ent->r.currentAngles );
#ifdef __VEHICLES__
	} 
	else if(!Q_stricmp (cmd, "warzone_addvehicle") && CheatsOk( ent )) 
	{
		int  vehicleType;
		char str[MAX_TOKEN_CHARS];

		trap->Argv(1, str, sizeof(str));

		if (!Q_stricmp(str,"heavy")) 
		{
			vehicleType = VEHICLE_CLASS_HEAVY_TANK;
		} 
		else if (!Q_stricmp(str,"medium")) 
		{
			vehicleType = VEHICLE_CLASS_MEDIUM_TANK;
		} 
		else if (!Q_stricmp(str,"flame")) 
		{
			vehicleType = VEHICLE_CLASS_FLAMETANK;
		} 
		else 
		{
			vehicleType = VEHICLE_CLASS_LIGHT_TANK;
		}

		WARZONE_AddVehicle( ent->r.currentOrigin, ent->r.currentAngles, vehicleType );
#endif //__VEHICLES__
	} 
	else if(!Q_stricmp (cmd, "warzone_addflag") && CheatsOk( ent )) 
	{
		int  team;
		char str[MAX_TOKEN_CHARS];

		trap->Argv(1, str, sizeof(str));

		if (!Q_stricmp(str,"red")) 
		{
			team = TEAM_RED;
		} 
		else if (!Q_stricmp(str,"blue")) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			team = TEAM_FREE;
		}

		WARZONE_AddFlag( ent->r.currentOrigin, team );
	} 
	else if(!Q_stricmp (cmd, "warzone_changeflagteam") && CheatsOk( ent )) 
	{
		int  team = 0, flag_number = 0;
		char str[MAX_TOKEN_CHARS];

		trap->Argv(1, str, sizeof(str));

		if (!str[0])
		{
			trap->Print("Format: /warzone_changeflagteam <flag #> <team name>\n");
			WARZONE_ShowInfo();
			return;
		}

		flag_number = atoi(str);

		trap->Argv(2, str, sizeof(str));

		if (!Q_stricmp(str,"red")) 
		{
			team = TEAM_RED;
		} 
		else if (!Q_stricmp(str,"blue")) 
		{
			team = TEAM_BLUE;
		} 
		else 
		{
			team = TEAM_FREE;
		}
		
		WARZONE_ChangeFlagTeam( flag_number, team );
	} 
	else if(!Q_stricmp (cmd, "warzone_raiseents") && CheatsOk( ent )) 
	{
		WARZONE_RaiseAllWarzoneEnts( -1 );
	} 
	else if(!Q_stricmp (cmd, "warzone_removeents") && CheatsOk( ent )) 
	{
		WARZONE_RemoveAllWarzoneEnts();
	} 
	else if(!Q_stricmp (cmd, "warzone_removeammocrate") && CheatsOk( ent )) 
	{
		WARZONE_RemoveAmmoCrate();
	} 
	else if(!Q_stricmp (cmd, "warzone_removehealthcrate") && CheatsOk( ent )) 
	{
		WARZONE_RemoveHealthCrate();
	} 
	else if(!Q_stricmp (cmd, "warzone_showinfo") && CheatsOk( ent )) 
	{
		WARZONE_ShowInfo();
	}
	//
	// END - Warzone Gametype Editing...
	//
	else
	{
		if (Q_stricmp(cmd, "addbot") == 0)
		{ //because addbot isn't a recognized command unless you're the server, but it is in the menus regardless
//			trap->SendServerCommand( clientNum, va("print \"You can only add bots as the server.\n\"" ) );
			trap->SendServerCommand( clientNum, va("print \"%s.\n\"", G_GetStringEdString("MP_SVGAME", "ONLY_ADD_BOTS_AS_SERVER")));
		}
		else
		{
			trap->SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
		}
	}
}
