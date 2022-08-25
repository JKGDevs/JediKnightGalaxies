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

int AcceptBotCommand(char *cmd, gentity_t *pl);
void Cmd_NPC_f( gentity_t *ent );
extern void AIMod_CheckMapPaths ( gentity_t *ent );
extern void AIMod_CheckObjectivePaths ( gentity_t *ent );

/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
	return sv_cheats.integer || ent->client->sess.canUseCheats;
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
G_ClientNumberFromName

Finds the client number of the client with the given name
==================
*/
int G_ClientNumberFromName(const char* name)
{
	char		cleanInput[MAX_NETNAME];
	char		cleanName[MAX_NETNAME];
	int			i;
	gclient_t*	cl;

	Q_strncpyz(cleanInput, name, sizeof(cleanInput));
	Q_StripColor(cleanInput);
	for (i = 0, cl = level.clients; i < level.maxclients; i++, cl++)
	{// check for a name match
		if (cl->pers.connected != CON_CONNECTED)
			continue;
		Q_strncpyz(cleanName, cl->pers.netname, sizeof(cleanName));
		Q_StripColor(cleanName);
		if (!Q_stricmp(cleanName, cleanInput))
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

int G_ClientNumberFromStrippedSubstring(const char* name, qboolean checkAll)
{
	gclient_t	*cl;
	int			idnum, match = -1;
	char		cleanInput[MAX_NETNAME];

	Q_strncpyz(cleanInput, name, sizeof(cleanInput));
	Q_StripColor(cleanInput);

	for (idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++)
	{// check for a name match
		if (cl->pers.connected != CON_CONNECTED)
			continue;

		if (Q_stristr(cl->pers.netname_nocolor, cleanInput))
		{
			if (match != -1)
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


//damage & healing plums copied from g_combat
/*
===========
PlumItems
===========
*/

void PlumItems(gentity_t *ent, vec3_t origin, int damage, int meansOfDeath, int shield, qboolean weak)
{
	meansOfDamage_t* means = JKG_GetMeansOfDamage(meansOfDeath);

	if (means->plums.noDamagePlums)
	{	// this means of death has no damage plums 
		return;
	}

	if (ent->damagePlumTime != level.time) {
		ent->damagePlum = G_TempEntity(origin, EV_DAMAGEPLUM);
		ent->damagePlumTime = level.time;
	}
	ent->damagePlum->s.time = damage;
	ent->damagePlum->s.eventParm = meansOfDeath;
	ent->damagePlum->s.generic1 = shield;
	ent->damagePlum->s.groundEntityNum = weak;
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f(gentity_t *ent) {
	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
		return;
	}
	if (ent->client->tempSpectate >= level.time) {
		return;		// Cant /kill in tempspec
	}

	if (ent->health <= 0) {		//awww sad, zombies can't commit suicide
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
	player_die(ent, ent, ent, 100000, MOD_SUICIDE);
}

/*
=================
Cmd_KillOther_f

=================
*/
void Cmd_KillOther_f(gentity_t* ent) {
	if (trap->Argc() > 1)
	{
		char sArg[MAX_STRING_CHARS];
		int entNum = 0;

		trap->Argv(1, sArg, sizeof(sArg));

		entNum = ClientNumberFromString(ent, sArg, qfalse);

		if (entNum >= 0 && entNum < MAX_GENTITIES)
		{
			gentity_t *kEnt = &g_entities[entNum];

			if (kEnt->inuse && kEnt->client)
			{
				kEnt->flags &= ~FL_GODMODE;
				kEnt->client->ps.stats[STAT_HEALTH] = kEnt->health = -999;
				player_die(kEnt, kEnt, kEnt, 100000, MOD_SUICIDE);
			}
		}
	}
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
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
		if (stringlength + j > MAX_STRING_CHARS-2) //formerly 1022
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	//still want to know the total # of clients
	i = level.numConnectedClients;

	trap->SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
		level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
		string ) );
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
Cmd_Credits_f

==================
*/
void Cmd_Credits_f(gentity_t* ent) {
	trap->SendServerCommand(ent - g_entities, va("print \"You have %i credits.\n\"", ent->client->ps.credits));
}

/*
==================
Cmd_Pay_f

==================
*/
#define PAY_DISTANCE	256
void Cmd_Pay_f(gentity_t* ent) {
	trace_t tr;
	vec3_t traceStart, traceEnd, forward;
	char creditBuffer[MAX_STRING_CHARS];
	int credits;
	int limit;  //credit limit we can pay
	int teamTime; //how much time we've been on the team

	if (trap->Argc() != 2) {
		trap->SendServerCommand(ent - g_entities, "print \"Usage: /pay <# of credits, or \"all\">\n\"");
		return;
	}

	if (jkg_payTime.integer < 1)
	{
		trap->SendServerCommand(ent - g_entities, "/pay is not allowed on this server!\n\"");
		return;
	}

	trap->Argv(1, creditBuffer, MAX_STRING_CHARS);

	if (!Q_stricmp(creditBuffer, "all")) {
		credits = ent->client->ps.credits;
	}
	else 
		credits = atoi(creditBuffer);

	teamTime = (level.time - ent->client->pers.enterTime);
	limit = (ent->client->ps.credits - jkg_startingCredits.integer);

	if ( teamTime < 60000* jkg_payTime.integer) //5 mins is default
	{
		trap->SendServerCommand(ent - g_entities, "print \"You cannot use /pay until you've played the game for a longer period of time.\n\"");
		return;
	}

	if (credits <= 0) {
		trap->SendServerCommand(ent - g_entities, "print \"You must offer an amount of 1 or more.\n\"");
		return;
	}
	if (limit < 1)
	{
		trap->SendServerCommand(ent - g_entities, va("print \"You have not earned enough credits yet.  You must have at least %i credits on hand to use /pay.\n\"", jkg_startingCredits.integer));
		return;
	}
	if (credits > limit)
	{
		trap->SendServerCommand(ent - g_entities, va("print \"You can't afford that!  You can only pay up to %i credits.\n\"", limit));
		return;
	}
	if (credits > ent->client->ps.credits) {
		trap->SendServerCommand(ent - g_entities, "print \"You can't afford that!\n\"");
		return;
	}
	

	AngleVectors(ent->client->ps.viewangles, forward, nullptr, nullptr);
	traceEnd[0] = ent->client->ps.origin[0] + forward[0] * PAY_DISTANCE;
	traceEnd[1] = ent->client->ps.origin[1] + forward[1] * PAY_DISTANCE;
	traceEnd[2] = (ent->client->ps.origin[2] + ent->client->ps.viewheight) + forward[2] * PAY_DISTANCE;

	VectorCopy(ent->client->ps.origin, traceStart);
	traceStart[2] += ent->client->ps.viewheight;

	trap->Trace(&tr, traceStart, nullptr, nullptr, traceEnd, ent->s.number, MASK_PLAYERSOLID, 0, 0, 0);
	if (tr.fraction >= 1.0f) {
		trap->SendServerCommand(ent - g_entities, "print \"Couldn't find anything to pay.\n\"");
		return;
	}
	if (tr.entityNum >= MAX_CLIENTS) {
		trap->SendServerCommand(ent - g_entities, "print \"You can't pay that entity money!\n\"");
		return;
	}

	gentity_t* target = &g_entities[tr.entityNum];
	target->client->ps.credits += credits;
	//ent->client->ps.spent += credits;		//paying other players counts as buying something!
	ent->client->ps.credits -= credits;

	trap->SendServerCommand(ent - g_entities, va("print \"You paid %i to %s\n\"", credits, target->client->pers.netname));
	trap->SendServerCommand(tr.entityNum, va("%s " S_COLOR_WHITE "paid you %i credits.\n\"", ent->client->pers.netname, credits));
	G_PreDefSound(ent->r.currentOrigin, PDSOUND_PAY);
}

/*
==================
Cmd_Relax_f

==================
*/
void Cmd_Relax_f(gentity_t* ent) {
	if (ent->client->ps.eFlags & EF_RAG) {
		trap->SendServerCommand(ent - g_entities, "print \"Ragdoll OFF\n\"");
		ent->client->ps.eFlags &= ~EF_RAG;
	}
	else {
		trap->SendServerCommand(ent - g_entities, "print \"Ragdoll ON\n\"");
		ent->client->ps.eFlags |= EF_RAG;
	}
}

/*
==================
Cmd_Wrists_f

/wrists
==================
*/
void Cmd_Wrists_f(gentity_t* ent) {
	gGAvoidDismember = 1;
	player_die(ent, ent, ent, 100000, MOD_SUICIDE);
	gGAvoidDismember = 2;
	G_CheckForDismemberment(ent, ent, ent->client->ps.origin, 999, ent->client->ps.legsAnim, qfalse);
	gGAvoidDismember = 0;
}

/*
==================
Cmd_ArbitraryPrint_f

==================
*/
void Cmd_ArbitraryPrint_f(gentity_t* ent) {
	trap->SendServerCommand(-1, "cp \"Blah blah blah\n\"");
}

/*
==================
Cmd_HeadExplodey_f

==================
*/
void DismembermentTest(gentity_t *self);
void Cmd_HeadExplodey_f(gentity_t* ent) {
	Cmd_Kill_f(ent);
	DismembermentTest(ent);
}

/*
==================
Cmd_LoveAndPeace_f

Missing the kissing animation.
==================
*/
void Cmd_LoveAndPeace_f(gentity_t* ent) {
	trace_t tr;
	vec3_t fPos;

	AngleVectors(ent->client->ps.viewangles, fPos, 0, 0);

	fPos[0] = ent->client->ps.origin[0] + fPos[0] * 40;
	fPos[1] = ent->client->ps.origin[1] + fPos[1] * 40;
	fPos[2] = ent->client->ps.origin[2] + fPos[2] * 40;

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
				VectorSubtract(other->client->ps.origin, ent->client->ps.origin, otherDir);
				VectorCopy(ent->client->ps.viewangles, entAngles);
				entAngles[YAW] = vectoyaw(otherDir);
				SetClientViewAngle(ent, entAngles);

				G_SetAnim(ent, nullptr, SETANIM_BOTH, /*BOTH_KISSER1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
				ent->client->ps.saberMove = LS_NONE;
				ent->client->ps.saberBlocked = 0;
				ent->client->ps.saberBlocking = 0;

				VectorSubtract(ent->client->ps.origin, other->client->ps.origin, entDir);
				VectorCopy(other->client->ps.viewangles, otherAngles);
				otherAngles[YAW] = vectoyaw(entDir);
				SetClientViewAngle(other, otherAngles);

				G_SetAnim(other, nullptr, SETANIM_BOTH, /*BOTH_KISSEE1LOOP*/BOTH_STAND1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD | SETANIM_FLAG_HOLDLESS);
				other->client->ps.saberMove = LS_NONE;
				other->client->ps.saberBlocked = 0;
				other->client->ps.saberBlocking = 0;
			}
		}
	}
}

/*
==================
Cmd_Holdme_f

==================
*/
void Cmd_Holdme_f(gentity_t* ent) {
	if (trap->Argc() > 1)
	{
		char sArg[MAX_STRING_CHARS];
		int entNum = 0;

		trap->Argv(1, sArg, sizeof(sArg));

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

/*
==================
Cmd_KnockMeDown_f

==================
*/
void Cmd_KnockMeDown_f(gentity_t* ent) {
	if (BG_KnockDownable(&ent->client->ps))
	{
		G_Knockdown(ent, ent, ent->s.origin, 10.0, qtrue);
	}
}

/*
==================
Cmd_Butterfingers_f

==================
*/
qboolean saberKnockOutOfHand(gentity_t *saberent, gentity_t *saberOwner, vec3_t velocity);
void Cmd_Butterfingers_f(gentity_t* ent) {
	if (ent->client->ps.weapon == WP_SABER &&
		ent->client->ps.saberEntityNum &&
		!ent->client->ps.saberInFlight)
	{
		saberKnockOutOfHand(&g_entities[ent->client->ps.saberEntityNum], ent, vec3_origin);
	}
}

/*
==================
Cmd_Dismember_f

==================
*/
extern void DismembermentByNum(gentity_t *self, int num);
void Cmd_Dismember_f(gentity_t* ent) {
	Cmd_Kill_f(ent);
	if (ent->health < 1)
	{
		char	arg[MAX_STRING_CHARS];
		int		iArg = 0;

		if (trap->Argc() > 1)
		{
			trap->Argv(1, arg, sizeof(arg));

			if (arg[0])
			{
				iArg = atoi(arg);
			}
		}

		DismembermentByNum(ent, iArg);
	}
}

/*
==================
Cmd_CloseEntities_f

==================
*/
void Cmd_CloseEntities_f(gentity_t* ent) {
	trap->Print("----------------------------------------------\n");
	trap->Print("Entities Within Range.\n");
	trap->Print("----------------------------------------------\n");

	for (int i = MAX_CLIENTS; i < MAX_GENTITIES; i++)
	{
		gentity_t *item = &g_entities[i];
		float dist = 0.0f;

		if (!item) continue;
		if (!item->classname || !item->classname[0]) continue;

		dist = Distance(ent->r.currentOrigin, item->s.origin);

		if (dist > 256) continue;

		trap->Print("Entity %i", i);

		switch (item->s.eType) {
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

/*
==================
Cmd_TUse_f

==================
*/
void Cmd_TUse_f(gentity_t* ent) {
	if (trap->Argc() > 1)
	{
		char sArg[MAX_STRING_CHARS];
		gentity_t *targ;

		trap->Argv(1, sArg, sizeof(sArg));
		targ = G_Find(NULL, FOFS(targetname), sArg);

		while (targ)
		{
			if (targ->use)
			{
				targ->use(targ, ent, ent);
			}
			targ = G_Find(targ, FOFS(targetname), sArg);
		}
	}
}

/*
==================
Cmd_ItemLookup_f

(eezstreet add)
Looks up a range of items and prints them in a list.
Ideally a GM or admin-only command
==================
*/

void Cmd_ItemLookup_f(gentity_t *ent)
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
Cmd_MyAmmo_f

Prints a list of all the ammo that you have
==================
*/
static void Cmd_MyAmmo_f(gentity_t* ent) {
	trap->SendServerCommand(ent - g_entities, "print \"Ammo Name                       |Type                            |Cur/Total\n\"");
	trap->SendServerCommand(ent - g_entities, "print \"================================|================================|==========\n\"");
	for (int i = 0; i < numAmmoLoaded; i++) {
		if (!ent->client->ammoTable[i]) {
			continue;
		}
		ammo_t* ammo = &ammoTable[i];
		trap->SendServerCommand(ent - g_entities, va("print \"%-31s | %-30s | %-4i/%-4i\n\"", ammo->shortname, ammo->substitute, ent->client->ammoTable[i], ammo->ammoMax));	
	}
	trap->SendServerCommand(ent - g_entities, "print \"\n\"");
}

/*
==================
Cmd_ItemCheck_f

Gives details about an item
==================
*/
void Cmd_ItemCheck_f(gentity_t *ent)
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
========================
CustomVendorSounds
does the npc have custom sounds?

========================
*/
qboolean CustomVendorSounds(gentity_t *conversationalist, char *name)
{
	fileHandle_t	f;
	char			filename[256];

	strcpy(filename, va("sound/vendor/%s/%s.mp3", conversationalist->NPC_type, name));

	trap->FS_Open(filename, &f, FS_READ);

	if (!f)
	{// End of conversation...
		trap->FS_Close(f);
		return qfalse;
	}

	trap->FS_Close(f);

	return qtrue;
}


/*
========================
HasCustomCivSounds
does the npc have custom sounds?

========================
*/
qboolean HasCustomCivSounds(char* path, gentity_t* conversationalist, char* name)
{
	fileHandle_t	f;
	char			filename[256];

	strcpy(filename, va("%s/civilian_%s/%s.mp3", path, conversationalist->NPC_type, name));

	trap->FS_Open(filename, &f, FS_READ);

	if (!f)
	{// End of conversation...
		trap->FS_Close(f);
		return qfalse;
	}

	trap->FS_Close(f);

	return qtrue;
}


/*
==================
JKG_BuyItem_f

==================
*/
void Cmd_BuyItem_f(gentity_t *ent)
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
		char* snd;

		trap->SendServerCommand(ent - g_entities, "print \"You do not have enough credits to purchase that item.\n\"");

		//select random unhappy vendor sound to play
		if (CustomVendorSounds(trader, "purchasefail00"))
		{// This NPC has it's own vendor specific sound(s)...
			char	filename[256];
			int		max = 1;

			while (CustomVendorSounds(trader, va("purchasefail0%i", max))) max++;

			strcpy(filename, va("sound/vendor/%s/purchasefail0%i.mp3", trader->NPC_type, irand(0, max - 1)));
			G_Sound(trader, CHAN_AUTO, G_SoundIndex(filename));
		}

		//if not custom vendor, check for fallback sounds
		else
		{
			//check for conversation 'upset' sound
			if (HasCustomCivSounds(va("sound/conversation"), trader, va("upset00")) )
			{
				snd = va("sound/conversation/civilian_%s/upset00.mp3", trader->NPC_type);
				G_Sound(trader, CHAN_AUTO, G_SoundIndex(snd));	//play sound
			}

			//use generic
			else
			{
				snd = va("sound/vendor/generic/purchasefail0%i.mp3", Q_irand(0, 5));
				G_Sound(trader, CHAN_AUTO, G_SoundIndex(snd));	//play sound
			}
		}
		return;
	}

	// Since the item is sent with a trade packet, the item is NOT needed to be networked, it already is through the trade packet
	if (pItem->id->itemType != ITEM_AMMO)
	{
		BG_SendTradePacket(IPT_TRADESINGLE, ent, trader, pItem, pItem->id->baseCost, 0);
		BG_GiveItemNonNetworked(ent, *pItem);
	}
	//ent->client->ps.spent += pItem->id->baseCost;
	ent->client->ps.credits -= pItem->id->baseCost;	// remove credits from player

	if (pItem->id->itemType == ITEM_WEAPON) {
		// If it's a weapon, we need to give the player some ammo - maybe half the maximum?
		// TODO externalize this into a function (JKG_PostItemPurchase) for other item types (special ammo types) 
		weaponData_t* wp = GetWeaponData(pItem->id->weaponData.weapon, pItem->id->weaponData.variation);
		if (!wp->firemodes[0].useQuantity) {
			for (int i = 0; i < wp->numFiringModes; i++) {
				ammo_t* ammoDefault = wp->firemodes[i].ammoDefault;
				if (ammoDefault) {
					BG_GiveAmmo(ent, ammoDefault, qfalse, ammoDefault->ammoMax / 2);
				}
				ent->client->clipammo[pItem->id->weaponData.varID][i] = wp->firemodes[i].clipSize;
				ent->client->ammoTypes[pItem->id->weaponData.varID][i] = wp->firemodes[i].ammoDefault->ammoIndex;
			}
		}
	}
	else if (pItem->id->itemType == ITEM_AMMO) {
		// If it's an ammo, it's not actually added to our inventory. Instead, it's given to us as ammo.
		BG_GiveAmmo(ent, BG_GetAmmo(pItem->id->ammoData.ammoIndex), qfalse, pItem->id->ammoData.quantity);
	}

	//play purchase sound
	{
		//char* snd;  //unused

		//select random purchase vendor sound to play
		if (CustomVendorSounds(trader, "purchase00"))
		{// This NPC has it's own vendor specific sound(s)...
			char	filename[256];
			int		max = 1;

			while (CustomVendorSounds(trader, va("purchase0%i", max))) max++;

			strcpy(filename, va("sound/vendor/%s/purchase0%i.mp3", trader->NPC_type, irand(0, max - 1)));
			G_Sound(trader, CHAN_AUTO, G_SoundIndex(filename));
		}

		//if not custom vendor, use the generic one
		else
			G_PreDefSound(ent->r.currentOrigin, PDSOUND_VENDORPURCHASE);
	}

	// Tell other clients, if the item is not too cheap 
	if (jkg_buyAnnounce.integer > 0)
	{
		char* sendStr = va("cbi %i %s", pItem->id->itemID, ent->client->pers.netname);
		if (pItem->id->baseCost >= jkg_buyAnnounceThreshold.integer)
		{
			if (jkg_buyAnnounce.integer <= 0)
				;	//don't announce anything
			else if (jkg_buyAnnounce.integer == 1)
			{
				// Announce ONLY to the same team !
				for (int i = 0; i < level.numConnectedClients; i++)
				{
					if (level.clients[i].sess.sessionTeam == ent->client->sess.sessionTeam)
					{
						trap->SendServerCommand(level.clients[i].ps.clientNum, sendStr);
					}
				}
			}
			else
			{
				// Announce to EVERYONE !
				trap->SendServerCommand(-1, sendStr);
			}
		}
	}
}

/*
==================
JKG_CloseVendor_f

==================
*/
void Cmd_CloseVendor_f (gentity_t *ent)
{
	if ( ent->client->currentTrader == NULL )
	{
		return;
	}

	ent->flags &= ~FL_BUSYMODE; //no longer marked as busy
	ent->client->pmnomove = false;
	ent->client->currentTrader->genericValue1 = ENTITYNUM_NONE;
	ent->client->currentTrader = NULL;
}

//consume item helper function
void JKG_ConsumeItem_h(gentity_t* ent, int itemNum)
{

	//--futuza todo: tie this to the ui so we can have a nice cooldown timer on the aci of the consumable item as well
	int initHP = ent->client->ps.stats[STAT_HEALTH];    //get initial health before consuming item		--futuza: this is a hacky way of doing it, really needs a LUA function to figure this out
	int initStamina = ent->client->ps.forcePower;		//get initial stamina/force
	switch (BG_ConsumeItem(ent, itemNum))
	{
	case 1:
		trap->SendServerCommand(ent - g_entities, "print \"Could not consume that item - either it is not a consumable or it is not a valid inventory item.\n\"");
		return;
	case 2:
		trap->SendServerCommand(ent - g_entities, "print \"Could not consume that item - not enough quantity to consume this item.\n\"");
		return;
	case 3:
		trap->SendServerCommand(ent - g_entities, "print \"Could not consume that item - you must wait before consuming additional items.\n\"");
		trap->SendServerCommand(ent - g_entities, va("notify 1 \"Consumables on cooldown.\""));
		return;
	case 4:
		trap->SendServerCommand(ent - g_entities, "print \"Could not consume that item - your stats are already at max value.\n\"");
		return;
	default:
		break;
	}
	int endHP = ent->client->ps.stats[STAT_HEALTH];   //get final health after consuming item
	int endStamina = ent->client->ps.forcePower;	//get final stamina
	int take_hp = endHP - initHP;	int take_stamina = endStamina - initStamina;	//calculate diffs
	if (take_hp != 0)
	{
		if (take_hp > 0)
			PlumItems(ent, ent->r.currentOrigin, take_hp, JKG_GetMeansOfDamageIndex("MOD_CURED"), 0, take_hp <= (ent->s.maxhealth / 4));
		else
			PlumItems(ent, ent->r.currentOrigin, -take_hp, JKG_GetMeansOfDamageIndex("MOD_POISONED"), 0, take_hp <= (ent->s.maxhealth / 4));	//take is negated (to pass correct parameters)
	}

	if (take_stamina != 0)
	{
		if (take_stamina > 0)
			PlumItems(ent, ent->r.currentOrigin, take_stamina, JKG_GetMeansOfDamageIndex("MOD_STAMINA_GAIN"), 0, take_stamina <= (ent->client->ps.stats[STAT_MAX_STAMINA] / 4));
		else
			PlumItems(ent, ent->r.currentOrigin, -take_stamina, JKG_GetMeansOfDamageIndex("MOD_STAMINA_LOSE"), 0, take_stamina <= (ent->client->ps.stats[STAT_MAX_STAMINA] / 4));
	}
}

/*
==================
JKG_ConsumeItem_f

==================
*/
void JKG_ConsumeItem_f(gentity_t* ent) {
	char* args = ConcatArgs(1);
	JKG_ConsumeItem_h(ent, atoi(args));
}

/*
==================
Cmd_EquipShield_f

==================
*/
void Cmd_EquipShield_f(gentity_t* ent) {
	char* args = ConcatArgs(1);
	int argNum = atoi(args);

	JKG_ShieldEquipped(ent, argNum, qtrue);
}

/*
==================
Cmd_EquipJetpack_f

==================
*/
void Cmd_EquipJetpack_f(gentity_t* ent) {
	char* args = ConcatArgs(1);
	int argNum = atoi(args);

	JKG_JetpackEquipped(ent, argNum);
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

void Cmd_Crystal1_f(gentity_t* ent) {
	JKG_Crystal_f(ent, 0);
}

void Cmd_Crystal2_f(gentity_t* ent) {
	JKG_Crystal_f(ent, 1);
}

/*
==================
Cmd_GiveBuff_f

Gives a buff/debuff to the player (cheat command)
==================
*/
extern void G_BuffEntity(gentity_t* ent, gentity_t* buffer, int buffID, float intensity, int duration);
void Cmd_GiveBuff_f(gentity_t* ent)
{
	char buffName[BUFF_NAME_LEN]{ 0 };
	char numericBuffer[32]{ 0 };
	int duration = 5000; // default to 5 seconds
	float intensity = 1.0f; // default to normal intensity
	int buffID = -1;

	if (trap->Argc() < 2)
	{
		trap->SendServerCommand(ent - g_entities, "print \"usage: /givebuff <buff name> [duration (in ms)] [intensity (default 1.0)]\n\"");
		return;
	}

	trap->Argv(1, buffName, BUFF_NAME_LEN);
	buffID = JKG_ResolveBuffName(buffName);
	if (buffID < 0)
	{
		trap->SendServerCommand(ent - g_entities, "print \"Invalid buff name. Try using the printbufflist command.\n\"");
		return;
	}

	if (trap->Argc() > 2)
	{
		trap->Argv(2, numericBuffer, 32);
		duration = atoi(numericBuffer);
		if (trap->Argc() > 3)
		{
			trap->Argv(3, numericBuffer, 32);
			intensity = atof(numericBuffer);
		}
	}

	if (duration < 1)
	{
		trap->SendServerCommand(ent - g_entities, "print \"The duration must be greater than 0.\n\"");
		return;
	}

	G_BuffEntity(ent, ent, buffID, intensity, duration);
}

/*
==================
Cmd_PrintBuffList_f

Prints all of the usable buffs.
==================
*/
void Cmd_PrintBuffList_f(gentity_t* ent)
{
	std::vector<std::string> buffNames;

	JKG_GetBuffNames(buffNames);
	std::sort(buffNames.begin(), buffNames.end());	// sort alphabetically

	for (auto it = buffNames.begin(); it != buffNames.end(); ++it)
	{
		trap->SendServerCommand(ent - g_entities, va("print \"%s\n\"", it->c_str()));
	}
}

/*
==================
Cmd_PrintWeaponList_f

Good for testing desync
==================
*/

void Cmd_PrintWeaponList_f( gentity_t *ent )
{
	BG_PrintWeaponList();
}

/*
==================
Cmd_DumpWeaponList_f

Good for testing desync
==================
*/

void Cmd_DumpWeaponList_f( gentity_t *ent )
{
	BG_DumpWeaponList("svweaponlist.txt");
}

/*
==================
G_ClientNumFromArg
==================
*/
int G_ClientNumberFromArg(const char *name)
{
	int client_id = 0;
	const char *cp = name;
	while (*cp)
	{
		if (*cp >= '0' && *cp <= '9') cp++;
		else
		{
			client_id = -1; //mark as alphanumeric
			break;
		}
	}

	if (client_id == 0)
	{ // arg is assumed to be client number
		client_id = atoi(name);
	}
	// arg is client name
	if (client_id == -1)
	{
		client_id = G_ClientNumberFromStrippedSubstring(name, qfalse);
	}
	return client_id;
}

/*
==================
Give cheat
==================
*/

void G_Give( gentity_t *ent, const char *name, const char *args, int argc )
{
	gitem_t		*it;
	int			i;
	qboolean	give_all = qfalse;
	gentity_t	*it_ent;
	trace_t		trace;

	if ( !Q_stricmp( name, "all" ) )
		give_all = qtrue;

	if ( give_all || !Q_stricmp( name, "health" ) )
	{
		if ( argc == 3 )
			ent->health = Com_Clampi( 1, ent->client->ps.stats[STAT_MAX_HEALTH], atoi( args ) );
		else
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "shield" ) )
	{
		if ( argc == 3 )
			ent->client->ps.stats[STAT_SHIELD] = Com_Clampi( 0, ent->client->ps.stats[STAT_MAX_SHIELD], atoi( args ) );
		else
			ent->client->ps.stats[STAT_SHIELD] = ent->client->ps.stats[STAT_MAX_SHIELD];

		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "stamina" ) )
	{
		if ( argc == 3 )
			ent->client->ps.forcePower = Com_Clampi( 0, ent->client->ps.stats[STAT_MAX_STAMINA], atoi( args ) );
		else
			ent->client->ps.forcePower = ent->client->ps.stats[STAT_MAX_STAMINA];

		if ( !give_all )
			return;
	}

	if ( !give_all && !Q_stricmp( name, "weapon" ) )
	{
	    weaponData_t *weapon = BG_GetWeaponByClassName ( args );
	    if ( weapon )
	    {
			int weaponID = BG_GetWeaponIndex(weapon->weaponBaseIndex, weapon->weaponModIndex);
			int itemID = BG_GetItemByWeaponIndex(weaponID)->itemID;

			itemInstance_t item = BG_ItemInstance(itemID, 1);
			BG_GiveItem(ent, item);
			trap->SendServerCommand (ent->s.number, va ("print \"'%s' was added to your inventory.\n\"", itemLookupTable[itemID].displayName));
			
			if (BG_WeaponCanUseSpecialAmmo(weapon)) {
				for (i = 0; i < weapon->numFiringModes; i++)
				{
					ent->client->ammoTypes[weaponID][i] = weapon->firemodes[i].ammoDefault->ammoIndex;
				}
			}
	    }
	    else
	    {
	        trap->SendServerCommand (ent->s.number, va ("print \"'%s' does not exist.\n\"", args));
	    }
	    return;
	}

	if ( give_all || !Q_stricmp( name, "ammo" ) )
	{
		int num = 999;

		if (argc == 3)
		{
			num = atoi(args);
			if (num < 0) num = 0;
		}
		
		for (i = 1; i < MAX_AMMO_TYPES; i++) // don't give NONE ammo
			ent->client->ammoTable[i] = num;
		for (i = 0; i < MAX_WEAPON_TABLE_SIZE; i++)
		{
			int weapVar, weapBase;
			weaponData_t* wp;
			if(!BG_GetWeaponByIndex(i, &weapBase, &weapVar))
			{
				break;
			}
			wp = GetWeaponDataUnsafe(weapBase, weapVar);
			if (wp != nullptr)
			{
				for (int j = 0; j < MAX_FIREMODES; j++)
				{
					ent->client->clipammo[i][j] = wp->firemodes[j].clipSize;
				}
			}
		}
		ent->client->ps.stats[STAT_TOTALAMMO] = num;
		if ( !give_all )
			return;
	}

	//Inventory items -- eezstreet/JKG
	if( !give_all && !Q_stricmp( name, "item" ) )
	{
		int itemID = atoi( args );
		if(itemID)
		{
			if(!itemLookupTable[itemID].itemID)
			{
				trap->SendServerCommand(ent - g_entities, va("print \"%i refers to an item that does not exist\n\"", itemID));
				return;
			}
			itemInstance_t item = BG_ItemInstance(itemID, 1);

			// If we gave the player an ammo item, we need to actually give them ammo, not the ammo item itself
			if (item.id->itemType == ITEM_AMMO)
			{
				BG_GiveAmmo(ent, BG_GetAmmo(item.id->ammoData.ammoIndex), qfalse, item.id->ammoData.quantity);
			}
			else
			{
				BG_GiveItem(ent, item);
			}
			
		}
		else
		{
			itemInstance_t item = BG_ItemInstance(args, 1);
			if (!item.id) {
				trap->SendServerCommand(ent - g_entities, va("print \"%s refers to an item that does not exist\n\"", args));
				return;
			}

			// If we gave the player an ammo item, we need to actually give them ammo, not the ammo item itself
			if (item.id->itemType == ITEM_AMMO)
			{
				BG_GiveAmmo(ent, BG_GetAmmo(item.id->ammoData.ammoIndex), qfalse, item.id->ammoData.quantity);
			}
			else
			{
				BG_GiveItem(ent, item);
			}
			
		}
		return;
	}

	if ( give_all || !Q_stricmp( name, "credits" ) || !Q_stricmp( name, "credit" ) ) {
		int num = 32000; // FIXME
		// FIXME if we allow addition of large number to an already close to overflow...
		if ( argc == 3 )
			num = Com_Clampi( 0, INT_MAX-2, atoi( args ) ); // putting a minus 2 here for safety
		ent->client->ps.credits = Com_Clampi( 0, INT_MAX-2, ent->client->ps.credits+num );
		trap->SendServerCommand( ent->client->ps.clientNum, va("print \"Your new balance is: %i credits\n\"", ent->client->ps.credits ));
		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "cloak" ) || !Q_stricmp( name, "cloakFuel" ) ) {
		int num = 100;
		if ( argc == 3 )
			num = Com_Clampi( 0, 100, atoi( args ) );
		ent->client->ps.cloakFuel = num;
		if ( !give_all )
			return;
	}

	if ( give_all || !Q_stricmp( name, "jetpack" ) || !Q_stricmp( name, "jetpackFuel" ) ) {
		int num = 100;
		if ( argc == 3 )
			num = Com_Clampi( 0, 100, atoi( args ) );
		ent->client->ps.jetpackFuel = num;
		if ( !give_all )
			return;
	}

	// spawn a specific item right on the player
	if ( !give_all ) {
		it = BG_FindItem( name );
		if ( !it )
			return;

		it_ent = G_Spawn();
		VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
		it_ent->classname = it->classname;
		G_SpawnItem( it_ent, it );
		if ( !it_ent || !it_ent->inuse )
			return;
		FinishSpawningItem( it_ent );
		if ( !it_ent || !it_ent->inuse )
			return;
		memset( &trace, 0, sizeof( trace ) );
		Touch_Item( it_ent, ent, &trace );
		if ( it_ent->inuse )
			G_FreeEntity( it_ent );
	}
}

void Cmd_Give_f( gentity_t *ent )
{
	char name[MAX_TOKEN_CHARS] = {0};

	if ( !CheatsOk( ent ) ) {
		return;
	}

	trap->Argv( 1, name, sizeof( name ) );
	G_Give( ent, name, ConcatArgs( 2 ), trap->Argc() );
}

void Cmd_GiveOther_f( gentity_t *ent )
{
	char		name[MAX_TOKEN_CHARS] = {0};
	int			i;
	char		otherindex[MAX_TOKEN_CHARS];
	gentity_t	*otherEnt = NULL;

	if ( trap->Argc () < 3 ) {
		trap->SendServerCommand( ent-g_entities, "print \"Usage: giveother <player id> <givestring>\n\"" );
		return;
	}

	trap->Argv( 1, otherindex, sizeof( otherindex ) );
	i = ClientNumberFromString( ent, otherindex, qfalse );
	if ( i == -1 ) {
		return;
	}

	otherEnt = &g_entities[i];
	if ( !otherEnt->inuse || !otherEnt->client ) {
		return;
	}

	if ( (otherEnt->health <= 0 || otherEnt->client->deathcamTime || otherEnt->client->tempSpectate >= level.time || otherEnt->client->sess.sessionTeam == TEAM_SPECTATOR) )
	{
		// Intentionally displaying for the command user
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "MUSTBEALIVE" ) ) );
		return;
	}

	trap->Argv( 2, name, sizeof( name ) );

	G_Give( otherEnt, name, ConcatArgs( 3 ), trap->Argc()-1 );
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
			team = PickTeam( clientNum );
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
				//JAC: Invalid clientNum was being used
				trap->SendServerCommand( ent-g_entities,
					va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYRED")) );
				return; // ignore the request
			}
			if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
				//JAC: Invalid clientNum was being used
				trap->SendServerCommand( ent-g_entities,
					va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TOOMANYBLUE")) );
				return; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}
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
		player_die (ent, ent, ent, 100000, MOD_TEAM_CHANGE);
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

	// Wipe the client's inventory before they begin so they won't get their old inventory
	BG_SendItemPacket(IPT_CLEAR, ent, nullptr, 0, 0);

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
	ent->client->ps.emplacedIndex = 0;
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

	//if teams are locked, and its been more than 20% of the match, and we didn't connect in the last 3 minutes
	if (timelimit.integer > 0)
	{
		//if teamlock enabled && (timelimit*60k)*0.2) < matchtime && matchtime - clientConnectedTime > 3 mins
		if ( g_teamsLocked.integer > 0 && ((timelimit.integer * 12000) < level.time) && ((level.time - ent->client->sess.connTime) > 180000) )
		{
			trap->SendServerCommand(ent - g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "TEAMSLOCKED")));
			return;
		}
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

	
	int timeout = 1;
	if (g_teamSwitchTime.integer > 0)
	{
		timeout = g_teamSwitchTime.integer;
	}

	// fix: update team switch time only if team change really happend
	if (oldTeam != ent->client->sess.sessionTeam)
		ent->client->switchTeamTime = level.time + (timeout * 1000);
}



//==========================================================
// INVENTORY RELATED COMMANDS
//==========================================================

/*
=================
JKG_Cmd_ItemAction_f
=================
*/
void JKG_Cmd_ItemAction_f(gentity_t *ent, int itemNum)
{
	if (!ent->client)
	{
		return; //NOTENOTE: NPCs can perform item actions. Nifty, eh?
	}

	if (ent->client->ps.stats[STAT_HEALTH] <= 0 ||
		ent->client->ps.pm_type == PM_DEAD)
	{
		// Bugfix: Can no longer use items when dead --eez
		Com_Printf("Can't use items while dead!\n");
		return;
	}

	if (itemNum < 0 || itemNum >= MAX_INVENTORY_ITEMS)
	{
		return;
	}

	if (ent->inventory->size() < 1)
	{
		//bugfix: empty inventory == nothing to use  --Futuza
		return;
	}

	if (itemNum >= ent->inventory->size())
	{
		//Nope.
		return;
	}

	//if  item is a jetpack
	if(ent->inventory->at(itemNum).id->itemType == ITEM_JETPACK)
	{
		ItemUse_Jetpack(ent);
		return;
	}

	//if item is a shield - TODO: add this feature
	/*if (ent->inventory->at(itemNum).id->itemType == ITEM_SHIELD)
	{
		//call special shield effect
		return;
	}*/

	//if  item is a consumable
	if (ent->inventory->at(itemNum).id->itemType == ITEM_CONSUMABLE)
	{
		JKG_ConsumeItem_h(ent, itemNum);
	}


}

/*
==================
Cmd_ItemUse_f

==================
*/
void Cmd_ItemUse_f(gentity_t* ent) {
	char arg[MAX_STRING_CHARS];
	if (trap->Argc() < 2)
	{
		trap->SendServerCommand(ent - g_entities, "print \"Syntax: /inventoryUse <item num>\n\"");
		return;
	}
	trap->Argv(1, arg, sizeof(arg));
	JKG_Cmd_ItemAction_f(ent, atoi(arg));
}

/*
==================
Cmd_ResendInv_f

==================
*/
void Cmd_ResendInv_f(gentity_t* ent) {
	BG_SendItemPacket(IPT_RESET, ent, nullptr, ent->inventory->size(), 0);
}

/*
=================
JKG_Cmd_ShowInv_f
=================
*/
void Cmd_ShowInv_f(gentity_t *ent)
{
    char buffer[MAX_STRING_CHARS] = { 0 };  //--Futuza: note this will run out of space for most inventories, so we have to break it up (as below)
	float weight = 0;
	float armorweight = 0;
	if(!ent->client)
		return;

	Q_strncpyz (buffer, "Inventory ID | Item Num | Instance Name                                 | Quantity | Weight\n", sizeof (buffer));
	Q_strcat (buffer, sizeof (buffer), "-------------+----------+----------------------------------------------------------+--------\n");
	trap->SendServerCommand(ent->s.number, va("print \"%s\n\"", buffer)); //print out
	memset(buffer, '\0', sizeof(buffer));


	for (auto it = ent->inventory->begin(); it != ent->inventory->end(); ++it) {
		Q_strcat(buffer, sizeof(buffer), va(S_COLOR_WHITE "%12i | %8i | %-45s | %8i | %6.2f\n", it - ent->inventory->begin(), it->id->itemID, it->id->displayName, it->quantity, it->id->weight));
		weight = weight + (it->id->weight * it->quantity);
		if (it->equipped)
		{
			armorweight = armorweight + (it->id->weight * it->quantity);
		}

		//buffer is about to fill - spit out what we got so far
		if (buffer[MAX_STRING_CHARS-100] != '\0')
		{
			trap->SendServerCommand(ent->s.number, va("print \"%s\"", buffer));
			memset(buffer, '\0', sizeof(buffer)); //reset
		}
	}

	//add totals on the end
	Q_strcat(buffer, sizeof(buffer), va("\nEquipped weight: %.2f\n", armorweight));  //--Futuza: including ACI weight would be nice, but we can't access cg from here (yet) to see if weapons are equipped or not
	Q_strcat (buffer, sizeof (buffer), va( "Total weight: %.2f\n", weight));
	Q_strcat (buffer, sizeof (buffer), va( "Total items: %i", ent->inventory->size()));
	trap->SendServerCommand (ent->s.number, va ("print \"%s\n\"", buffer)); 
}

/*
=========================================
JKG_Cmd_EquipToACI_f / JKG_Cmd_Unequip_f
=========================================
*/

extern void JKG_EquipItem(gentity_t *ent, int iNum);
extern void JKG_UnequipItem(gentity_t *ent, int iNum);
void Cmd_EquipItem_f(gentity_t *ent)
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

void Cmd_UnequipItem_f(gentity_t *ent)
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
Cmd_DestroyItem_f
Destroys an item from your inventory
==================
*/
void Cmd_DestroyItem_f(gentity_t *ent)
{
	char arg[64];
	trap->Argv(1, arg, sizeof(arg));

	if(trap->Argc() < 2)
	{
		trap->SendServerCommand(ent->client->ps.clientNum, "print \"Syntax: /inventoryDestroy <inventory ID or internal>\n\"");
		return;
	}

	if (StringIsInteger(arg))
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

void Cmd_SellItem_f(gentity_t *ent)
{
	gentity_t* trader = ent->client->currentTrader;
	// TODO: put proper pricing here
	char arg[64];
	trap->Argv(1, arg, sizeof(arg));
	if(trap->Argc() < 2)
	{
		trap->SendServerCommand(ent->s.number, "print \"Syntax: /inventorySell <inventory ID or internal>\n\"");
		return;
	}

	if(trader == NULL)
	{
		trap->SendServerCommand(ent->s.number, "print \"You are not at a vendor.\n\"");
		return;
	}

	int nInvID = -1;
	if (StringIsInteger(arg))
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
				trap->SendServerCommand(ent - g_entities, "print \"You cannot sell your starter gun unless you have another item in your inventory.\n\""); //only visible in console
				trap->SendServerCommand(ent - g_entities, "notify 1 \"You cannot sell your starter gun unless you have another item in your inventory.\n\"");
				return;
			}
			else {
				creditAmount = 2;		//--futuza: fixme, we need to warn the player that starter guns are worth only 1 credit - BEFORE they sell it
			}
		}
	}
	else if (item.id->itemType == ITEM_SHIELD && item.equipped) {
		// If we're selling an equipped shield, kill it
		Cmd_ShieldUnequipped(ent);
	}
	else if (item.id->itemType == ITEM_JETPACK && item.equipped) {
		Cmd_JetpackUnequipped(ent);
	}

	else if (item.id->itemType == ITEM_ARMOR && item.equipped) {
		JKG_ArmorChanged(ent);
	}


	ent->client->ps.credits += (creditAmount * item.quantity) / 2;
	

	BG_RemoveItemStack(ent, nInvID);
	trap->SendServerCommand(ent->s.number, va("inventory_update %i", ent->client->ps.credits));

	G_PreDefSound(ent->r.currentOrigin, PDSOUND_TRADE);
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

		if (arg[0])
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

	if (strlen(chatText) >= MAX_SAY_TEXT) {
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
Cmd_SayAct_f - Actions
==================
*/
static void Cmd_SayAct_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

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
static void Cmd_SayTeam_f(gentity_t *ent) {
	char		*p;

	if (trap->Argc() < 2) {
		return;
	}

	p = ConcatArgs(1);

	if (g_gametype.integer < GT_TEAM) {
		G_Say(ent, NULL, SAY_GLOBAL, p);
	}
	else {
		G_Say(ent, NULL, SAY_TEAM, p);
	}
}

/*
==================
Cmd_SayWhisper_f - Whisper
==================
*/
static void Cmd_SayWhisper_f( gentity_t *ent ) {
	char		*p;

	if ( trap->Argc () < 2 ) {
		return;
	}

	p = ConcatArgs( 1 );

	G_Say( ent, NULL, SAY_WHISPER, p );
}

/*
==================
Cmd_SayLocal_f
==================
*/
static void Cmd_SayLocal_f(gentity_t* ent) {
	char		*p;

	if (trap->Argc() < 2) {
		return;
	}

	p = ConcatArgs(1);

	G_Say(ent, NULL, SAY_ALL, p);
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
	CCmd_AddCommand("sayteam", CCmd_Say_Team);
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

	if ( ent->health <= 0 || ent->client->tempSpectate >= level.time || (ent->client && ent->client->deathcamTime) ) {
		trap->SendServerCommand( ent-g_entities, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
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
Cmd_CallVote_f
==================
*/

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

qboolean G_VoteClientkick( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int n = atoi ( arg2 );

	if ( n < 0 || n >= level.maxclients ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"invalid client number %d.\n\"", n ) );
		return qfalse;
	}

	if ( g_entities[n].client->pers.connected == CON_DISCONNECTED ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"there is no client with the client number %d.\n\"", n ) );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, g_entities[n].client->pers.netname );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
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

qboolean G_VoteGametype( gentity_t *ent, int numArgs, const char *arg1, const char *arg2 ) {
	int gt = atoi( arg2 );
	// ffa, ctf, tdm, etc
	if ( arg2[0] && isalpha( arg2[0] ) ) {
		gt = BG_GetGametypeForString( arg2 );
		if ( gt == -1 )
		{
			trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%s) unrecognised, defaulting to Faction Battle\n\"", arg2 ) );
			gt = GT_TEAM;
		}
	}
	// numeric but out of range
	else if ( gt < 0 || gt >= GT_MAX_GAME_TYPE ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"Gametype (%i) is out of range, defaulting to Faction Battle\n\"", gt ) );
		gt = GT_TEAM;
	}
	// logically invalid gametypes, or gametypes not fully implemented in MP
	if ( gt == GT_SINGLE_PLAYER || gt == GT_WARZONE) {
		trap->SendServerCommand( ent-g_entities, va( "print \"This gametype is not supported (%s).\n\"", arg2 ) );
		return qfalse;
	}

	level.votingGametype = qtrue;
	level.votingGametypeTo = gt;
	Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, gt );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s %s", arg1, BG_GetGametypeString(gt) );
	Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
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
	{ "clientkick", NULL, G_VoteClientkick, 1, GTB_ALL, qfalse, "<clientnum>" },
	{ "fraglimit", "frags", G_VoteFraglimit, 1, GTB_ALL, qtrue, "<num>" },
	{ "g_gametype", "gametype gt mode", G_VoteGametype, 1, GTB_ALL, qtrue, "<num or name>" },
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

const char *G_GetArenaInfoByMap( const char *map );
void Cmd_CallVote_f( gentity_t *ent ) {
	int				i=0, numArgs=0;
	char			arg1[MAX_CVAR_VALUE_STRING] = {0};
	char			arg2[MAX_CVAR_VALUE_STRING] = {0};
	voteString_t *vote = NULL;

	// not allowed to vote at all
	if ( !g_allowVote.integer ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOVOTE" ) ) );
		return;
	}

	// vote in progress
	else if ( level.voteTime ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "VOTEINPROGRESS" ) ) );
		return;
	}

	// can't vote as a spectator, except in (power)duel
	else if ( level.gametype != GT_DUEL && level.gametype != GT_POWERDUEL && ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", G_GetStringEdString( "MP_SVGAME", "NOSPECVOTE" ) ) );
		return;
	}

	// make sure it is a valid command to vote on
	numArgs = trap->Argc();
	trap->Argv( 1, arg1, sizeof( arg1 ) );
	if ( numArgs > 1 )
		Q_strncpyz( arg2, ConcatArgs( 2 ), sizeof( arg2 ) );

	// filter ; \n \r
	if ( Q_strchrs( arg1, ";\r\n" ) || Q_strchrs( arg2, ";\r\n" ) ) {
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		return;
	}

	// check for invalid votes
	for ( i=0; i<validVoteStringsSize; i++ ) {
		if ( !(g_allowVote.integer & (1<<i)) )
			continue;

		if ( !Q_stricmp( arg1, validVoteStrings[i].string ) )
			break;

		// see if they're using an alias, and set arg1 to the actual vote string
		if ( validVoteStrings[i].aliases ) {
			char tmp[MAX_TOKEN_CHARS] = {0}, *p = NULL;
			const char *delim = " ";
			Q_strncpyz( tmp, validVoteStrings[i].aliases, sizeof( tmp ) );
			p = strtok( tmp, delim );
			while ( p != NULL ) {
				if ( !Q_stricmp( arg1, p ) ) {
					Q_strncpyz( arg1, validVoteStrings[i].string, sizeof( arg1 ) );
					goto validVote;
				}
				p = strtok( NULL, delim );
			}
		}
	}
	// invalid vote string, abandon ship
	if ( i == validVoteStringsSize ) {
		char buf[1024] = {0};
		int toggle = 0;
		trap->SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
		trap->SendServerCommand( ent-g_entities, "print \"Allowed vote strings are: \"" );
		for ( i=0; i<validVoteStringsSize; i++ ) {
			if ( !(g_allowVote.integer & (1<<i)) )
				continue;

			toggle = !toggle;
			if ( validVoteStrings[i].shortHelp ) {
				Q_strcat( buf, sizeof( buf ), va( "^%c%s %s ",
					toggle ? COLOR_GREEN : COLOR_YELLOW,
					validVoteStrings[i].string,
					validVoteStrings[i].shortHelp ) );
			}
			else {
				Q_strcat( buf, sizeof( buf ), va( "^%c%s ",
					toggle ? COLOR_GREEN : COLOR_YELLOW,
					validVoteStrings[i].string ) );
			}
		}

		//FIXME: buffer and send in multiple messages in case of overflow
		trap->SendServerCommand( ent-g_entities, va( "print \"%s\n\"", buf ) );
		return;
	}

validVote:
	vote = &validVoteStrings[i];
	if ( !(vote->validGT & (1<<level.gametype)) ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s is not applicable in this gametype.\n\"", arg1 ) );
		return;
	}

	if ( numArgs < vote->numArgs+2 ) {
		trap->SendServerCommand( ent-g_entities, va( "print \"%s requires more arguments: %s\n\"", arg1, vote->shortHelp ) );
		return;
	}

	level.votingGametype = qfalse;

	level.voteExecuteDelay = vote->voteDelay ? g_voteDelay.integer : 0;

	// there is still a vote to be executed, execute it and store the new vote
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap->SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	// pass the args onto vote-specific handlers for parsing/filtering
	if ( vote->func ) {
		if ( !vote->func( ent, numArgs, arg1, arg2 ) )
			return;
	}
	// otherwise assume it's a command
	else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Q_strncpyz( level.voteDisplayString, level.voteString, sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteStringClean, level.voteString, sizeof( level.voteStringClean ) );
	}
	Q_strstrip( level.voteStringClean, "\"\n\r", NULL );

	trap->SendServerCommand( -1, va( "print \"%s^7 %s (%s)\n\"", ent->client->pers.netname, G_GetStringEdString( "MP_SVGAME", "PLCALLEDVOTE" ), level.voteStringClean ) );

	// start the voting, the caller automatically votes yes
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i=0; i<level.maxclients; i++ ) {
		level.clients[i].mGameFlags &= ~PSG_VOTED;
		level.clients[i].pers.vote = 0;
	}

	ent->client->mGameFlags |= PSG_VOTED;
	ent->client->pers.vote = 1;

	trap->SetConfigstring( CS_VOTE_TIME,	va( "%i", level.voteTime ) );
	trap->SetConfigstring( CS_VOTE_STRING,	level.voteDisplayString );
	trap->SetConfigstring( CS_VOTE_YES,		va( "%i", level.voteYes ) );
	trap->SetConfigstring( CS_VOTE_NO,		va( "%i", level.voteNo ) );
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
			trap->SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
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
	weaponData_t* wp;
	int clipSize;

	weapon = ent->client->ps.weapon;

	wp = GetWeaponData(weapon, ent->client->ps.weaponVariation);

	clipSize = wp->firemodes[ent->client->ps.firingMode].clipSize;
	if (clipSize == -1)
	{
		// Weapon doesn't use clips, bail
		return;
	}

	if (ent->client->ps.weaponTime > 0 && ent->client->ps.weaponstate != WEAPON_READY) {
		// Weapon is busy, cannot reload at this moment
		return;
	}

	if ( ent->client->ps.forceHandExtend != HANDEXTEND_NONE || PM_InKnockDown (&ent->client->ps) )
	{//We're in a knockdown, or something ugly like that...DO NOT WANT!
		return;
	}

	/* No more ammo left to place in the clip */
	if (ent->client->ps.stats[STAT_TOTALAMMO] <= 0 && ent->client->ps.weapon != WP_SABER)
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
	if ( ent->client->ps.weaponstate == WEAPON_CHARGING )
	{
		return;
	}

	if ( weapon == WP_SABER)
	{
		// Do a kick instead, regardless if we're moving
		ent->client->ps.saberActionFlags |= ( 1 << SAF_KICK );
		return;
	}

	ammotoadd = clipSize;
	weaponIndex = BG_GetWeaponIndex(weapon, ent->client->ps.weaponVariation);

	if (ent->client->clipammo[weaponIndex][ent->client->ps.firingMode] >= ammotoadd)	{
		// Current clip is full
		return;
	}

	ammotoadd -= ent->client->clipammo[weaponIndex][ent->client->ps.firingMode];

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
	ammotoadd = Q_min(ammotoadd, ent->client->ps.stats[STAT_TOTALAMMO]);

	//Remove the ammo from 'bag'
	ent->client->ps.stats[STAT_TOTALAMMO] -= ammotoadd;
	ent->client->ammoTable[ent->client->ps.ammoType] -= ammotoadd;

	//Add the ammo to weapon
	ent->client->clipammo[weaponIndex][ent->client->ps.firingMode] += ammotoadd;
	ent->client->ps.stats[STAT_AMMO] += ammotoadd;

	//Take us out of ironsights
	ent->client->ps.ironsightsDebounceStart = level.time + ent->client->ps.weaponTime;

	// Don't shoot any more bullets!
	ent->client->ps.shotsRemaining = 0;

	//flush heat on a reload (default is 50%)
	if (ent->client->ps.heat > 0)
	{
		float heatPerct = wp->firemodes[ent->client->ps.firingMode].reloadClearHeat * 0.01f;
		if (heatPerct < 0)
			heatPerct = 0.0f;

		if (heatPerct > 1)
			heatPerct = 1.0f;

		ent->client->ps.heat -= (heatPerct * ent->client->ps.heat);

		if (ent->client->ps.heat < 1)
			ent->client->ps.heat = 0.0f;
	}

	//check if we need to turn off overheating status after halfing heat
	if (ent->client->ps.overheated)
	{
		if(ent->client->ps.heat < ent->client->ps.heatThreshold)
			ent->client->ps.overheated = false;
	}
	
}

/*
=================
Cmd_TcEval_f
Cmd_TcEvalMulti_f

=================
*/
static void EvaluateTreasureClass(gentity_t *ent, bool bMulti) {
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

void Cmd_TcEval_f(gentity_t* ent) {
	EvaluateTreasureClass(ent, false);
}

void Cmd_TcEvalMulti_f(gentity_t* ent) {
	EvaluateTreasureClass(ent, true);
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

/*
=================
Cmd_AmmoCycle_f
=================
*/
void Cmd_AmmoCycle_f(gentity_t* ent) {
	int weapon = ent->client->ps.weapon;
	int variation = ent->client->ps.weaponVariation;
	weaponData_t* wp = GetWeaponData(weapon, variation);
	int weaponIndex = BG_GetWeaponIndex(weapon, variation);
	int fireMode = ent->client->ps.firingMode;
	int currentAmmo = ent->client->ps.ammoType;
	int desiredAmmo = -1;
	std::vector<ammo_t*> allValidAmmos;

	if (!BG_WeaponCanUseSpecialAmmo(wp)) {
		// It's not a weapon that has special ammo capabilities
		return;
	}

	if (wp->firemodes[fireMode].clipSize <= 0)
	{
		// Current weapon does not use a clip, bail
		return;
	}

	if (ent->client->ps.weaponTime > 0 && ent->client->ps.weaponstate != WEAPON_READY) {
		// Weapon is busy, cannot reload at this moment
		return;
	}

	if (ent->client->ps.forceHandExtend != HANDEXTEND_NONE && !PM_InKnockDown(&ent->client->ps))
	{//We're in a knockdown, or something ugly like that...DO NOT WANT!
		return;
	}

	// Determine if we have ammo to cycle to.
	BG_GetAllAmmoSubstitutions(wp->firemodes[fireMode].ammoBase->ammoIndex, allValidAmmos); //--futuza trouble happening here

	if (allValidAmmos.size() == 0) {
		// Our weapon...is not linked up correctly? or something? It doesn't have any ammo...
		return;
	}

	// Find the first ammo that has an ammo index higher than our current one.
	for (auto it : allValidAmmos) {
		if (ent->client->ammoTable[it->ammoIndex]) {
			if (it->ammoIndex > currentAmmo) {
				desiredAmmo = it->ammoIndex;
				break;
			}
		}
	}

	if (desiredAmmo == -1) {
		// Not found - probably because we were on the last one. Find the first.
		for (auto it : allValidAmmos) {
			if (ent->client->ammoTable[it->ammoIndex]) {
				desiredAmmo = it->ammoIndex;
				break;
			}
		}
	}

	if (desiredAmmo == -1 || desiredAmmo == currentAmmo) {
		// Either we don't have ammo we can swap to, or we aren't actually changing our ammo
		return;
	}

	// Set our ammo to be the desired ammo type
	ent->client->ps.stats[STAT_TOTALAMMO] += ent->client->ps.stats[STAT_AMMO];
	ent->client->ammoTable[ent->client->ps.ammoType] = ent->client->ps.stats[STAT_TOTALAMMO];
	ent->client->ps.stats[STAT_AMMO] = 0;
	ent->client->ps.ammoType = desiredAmmo;
	ent->client->ammoTypes[weaponIndex][ent->client->ps.firingMode] = desiredAmmo;
	ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
	ent->client->clipammo[weaponIndex][ent->client->ps.firingMode] = 0;

	// Trigger a reload
	Cmd_Reload_f(ent);
}

/*
=================
Cmd_SaberAttackCycle_f
=================
*/
extern qboolean WP_SaberCanTurnOffSomeBlades( saberInfo_t *saber );
void Cmd_SaberAttackCycle_f(gentity_t *ent)
{
	int selectLevel = 0;
	int i, j;

	if ( !ent || !ent->client)
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

	if (ent->client->ps.weaponstate != WEAPON_READY)
	{
		// Don't let us swap firing modes while changing weapons --eez
		return;
	}

	if(!JKG_ClientAlive(ent))
	{
		// Don't swap while dead
		return;
	}

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
		ent->client->saberStanceDebounce = level.time + 400;
		trap->SendServerCommand(ent->s.number, va("fmrefresh %i", previousFiringMode));
		ent->client->firingModes[ent->client->ps.weaponId] = ent->client->ps.firingMode;

		// consumables and other weapons don't use ammo, don't try it
		if (wp->firemodes[0].useQuantity || ent->client->ps.weapon == WP_EMPLACED_GUN || ent->client->ps.weapon == WP_TURRET)
			return;

		// Swap our ammo type (but don't trigger a reload) if the new firing mode doesn't accept our old ammo type
		if (!BG_AmmoIsBasedOn(wp->firemodes[ent->client->ps.firingMode].ammoBase->ammoIndex,
			wp->firemodes[previousFiringMode].ammoBase->ammoIndex))
		{
			ent->client->ammoTable[ent->client->ps.ammoType] = ent->client->ps.stats[STAT_TOTALAMMO];
			ent->client->clipammo[ent->client->ps.weaponId][previousFiringMode] = ent->client->ps.stats[STAT_AMMO];
			ent->client->ammoTypes[ent->client->ps.weaponId][previousFiringMode] = ent->client->ps.ammoType;
			ent->client->ps.ammoType = ent->client->ammoTypes[ent->client->ps.weaponId][ent->client->ps.firingMode];
			ent->client->ps.stats[STAT_AMMO] = ent->client->clipammo[ent->client->ps.weaponId][ent->client->ps.firingMode];
			ent->client->ps.stats[STAT_TOTALAMMO] = ent->client->ammoTable[ent->client->ps.ammoType];
		}
		else
		{
			// We need to copy the clipammo and ammo type from the previous firing mode over 
			ent->client->clipammo[ent->client->ps.weaponId][ent->client->ps.firingMode] = 
				ent->client->clipammo[ent->client->ps.weaponId][previousFiringMode];
			ent->client->ammoTypes[ent->client->ps.weaponId][ent->client->ps.firingMode] =
				ent->client->ammoTypes[ent->client->ps.weaponId][previousFiringMode];
		}
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

/*
==================
Cmd_AmmoPriceCheck_f

Checks the price of refilling ammo.
==================
*/
static void Cmd_AmmoPriceCheck_f(gentity_t* ent)
{
	qboolean silent = trap->Argc() > 2;
	int cost;
	int invID;
	char buffer[1024]{ 0 };
	weaponData_t* wp;
	itemInstance_t* item;

	if (trap->Argc() < 2)
	{
		trap->SendServerCommand(ent - g_entities, "print \"usage: /ammopricecheck <inventory ID>\n\"");
		return;
	}

	trap->Argv(1, buffer, 1024);
	invID = atoi(buffer);

	if (ent->inventory == nullptr)
	{
		trap->SendServerCommand(ent - g_entities, "print \"Your inventory is NULL (?)\n\"");
		return;
	}

	if (invID < 0 || invID >= ent->inventory->size())
	{
		trap->SendServerCommand(ent - g_entities, "print \"Invalid item ID.\n\"");
		return;
	}

	item = &(*ent->inventory)[invID];
	if (item->id->itemType != ITEM_WEAPON)
	{
		trap->SendServerCommand(ent - g_entities, "print \"That item is not a weapon.\n\"");
		return;
	}

	wp = GetWeaponData(item->id->weaponData.weapon, item->id->weaponData.variation);

	cost = BG_GetRefillAmmoCost(ent->client->ammoTable, wp);

	if (!silent)
	{
		trap->SendServerCommand(ent - g_entities, va("print \"It will take %i credits to fully load that weapon.\n\"", cost));
		return;
	}

	trap->SendServerCommand(ent - g_entities, va("apc %i %i", invID, cost));
}

/*
=================
Cmd_BuyAllAmmo_f
=================
*/
void Cmd_BuyAmmo_f(gentity_t* ent) {
	int itemSlot;
	char argBuffer[MAX_TOKEN_CHARS] {0};
	int myCredits = ent->client->ps.credits;
	float cost;
	int totalCost = 0, numFiringModesFilled = 0, numUnitsPurchased = 0;
	float perUnitCost = 0;

	gentity_t* trader = ent->client->currentTrader;
	if (trader == nullptr)
	{
		trap->SendServerCommand(ent - g_entities, "print \"You need to be at a vendor in order to buy ammo.\n\"");
		return;
	}

	if (trap->Argc() != 2) {
		trap->SendServerCommand(ent - g_entities, "print \"You need to select an item to buy ammo for.\n\"");
		return;
	}

	trap->Argv(1, argBuffer, MAX_TOKEN_CHARS);
	itemSlot = atoi(argBuffer);

	if (itemSlot < 0 || itemSlot >= ent->inventory->size()) {
		trap->SendServerCommand(ent - g_entities, "print \"Invalid item selected.\n\"");
		return;
	}

	itemInstance_t& item = ent->inventory->at(itemSlot);
	if (item.id->itemType != ITEM_WEAPON) {
		trap->SendServerCommand(ent - g_entities, "print \"That item is not a weapon.\n\"");
		return;
	}

	weaponData_t* wp = GetWeaponData(item.id->weaponData.weapon, item.id->weaponData.variation);
	if (wp->firemodes[0].useQuantity || wp->firemodes[0].clipSize <= 0) {
		trap->SendServerCommand(ent - g_entities, "print \"You cannot purchase ammo for that weapon.\n\"");
		return;
	}

	// Fill all firemodes to max until we run out of money
	for (int i = 0; i < wp->numFiringModes; i++) {
		ammo_t* ammo = wp->firemodes[i].ammoDefault;
		int unitsRequested = ammo->ammoMax - ent->client->ammoTable[ammo->ammoIndex];

		if (unitsRequested <= 0) {
			continue; // our ammo is full
		}

		//if its the starting weapon, ammo is free
		if (!Q_stricmp(item.id->internalName, level.startingWeapon))
		{
			perUnitCost = 0;
		}
		else
			perUnitCost = ammo->pricePerUnit;

		if (myCredits <= 0 && perUnitCost <= 0)
			;
		else
		{
			if (myCredits < perUnitCost) // we don't have enough money to afford one unit of ammo
			{
				trap->SendServerCommand(ent - g_entities, "print \"You cannot afford ammo for that weapon.\n\"");
				continue; 
			}
		}

		cost = perUnitCost * (float)unitsRequested;

		//if passiveUnderdogBonus is enabled and our team is losing, our ammo is discounted!  Awww, thanks vendor!  
		/*if (jkg_passiveUnderdogBonus.integer > 0)	//--Futuza: this needs to match logic of BG_GetRefillAmmoCost() in bg_ammo.cpp, disabled for now
		{
			//who is currently winning?
			auto my_team = ent->client->sess.sessionTeam;
			int curr_winner = -1;
			if (level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE])
				curr_winner = TEAM_RED;
			else if (level.teamScores[TEAM_RED] < level.teamScores[TEAM_BLUE])
				curr_winner = TEAM_BLUE;
			else
				curr_winner = -1;	//tie

			//we're losing - give us discount ammo please!
			if (my_team != curr_winner && curr_winner != -1)
			{
				float ammoAdjust = 0.9;

				//evaluate how badly the losing team is losing by and reduce ammo accordingly
				int diff = level.teamScores[curr_winner] - level.teamScores[my_team];
				ammoAdjust = (1 - (float)diff / level.teamScores[curr_winner]);

				if (ammoAdjust > 0.9)	//minimum discount is 10% off
					ammoAdjust = 0.9;

				if (ammoAdjust < 0.25)
					ammoAdjust = 0.25;		//maximum discount is 75% off

				cost = cost * ammoAdjust;
				if (0 < cost < 1)
					cost = 1;
			}
		}*/

		if (cost > myCredits) {
			// We can't fill it all the way, so fill it as much as we can
			cost = myCredits;
			unitsRequested = floor(cost / perUnitCost);
			cost = unitsRequested * perUnitCost;
		}

		//if our price is between 0 and 1, just make it cost 1 credit - so players can't scam us out of free ammo
		if (0 < cost && cost < 1) 
			cost = 1;
		
		cost = floor(cost); //credits are int only, time to act like it

		// Buy the ammo and update stats
		//ent->client->ps.spent += cost;
		ent->client->ps.credits -= cost;
		myCredits -= cost;
		ent->client->ammoTable[ammo->ammoIndex] += unitsRequested;
		numUnitsPurchased += unitsRequested;
		totalCost += cost;
		numFiringModesFilled++;
	}

	if (numUnitsPurchased <= 0) {
		trap->SendServerCommand(ent - g_entities, "print \"Your ammo for that weapon is full.\n\"");
		return;
	}
	trap->SendServerCommand(ent - g_entities, 
		va("print \"Bought %i ammo for %i firing modes, spent %i credits total.\n\"", numUnitsPurchased, numFiringModesFilled, totalCost));


	G_PreDefSound(ent->r.currentOrigin, PDSOUND_VENDORPURCHASE);

	int newCost = BG_GetRefillAmmoCost(ent->client->ammoTable, wp);
	trap->SendServerCommand(ent - g_entities, va("apc %i %i", itemSlot, newCost));
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

		if (challenged->client->ps.duelIndex == ent->s.number && challenged->client->ps.duelTime >= level.time)
		{
			trap->SendServerCommand( /*challenged-g_entities*/-1, va("print \"%s %s %s!\n\"", challenged->client->pers.netname, G_GetStringEdString("MP_SVGAME", "PLDUELACCEPT"), ent->client->pers.netname));

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

/*
=================
ClientCommand
=================
*/

// Types
using cmdFunc_t = void(*)(gentity_t *);

// Flags
#define CMD_NEEDCHEATS			1		// Cheats must be allowed for this command to work
#define CMD_ONLYALIVE			2		// The user needs to be alive for this command to work
#define CMD_NOINTERMISSION		4		// This command is not allowed during intermission
#define CMD_NOSPECTATOR			8		// Not allowed to be used as a spectator

typedef struct command_s
{
	const char	*cmdName;
	cmdFunc_t	function;
	uint32_t	flags;
} command_t;

int cmdcmp( const void *a, const void *b ) {
	return Q_stricmp( (const char *)a, ((command_t*)b)->cmdName );
}

static const command_t commands[] = {
	{ "ammocycle",				Cmd_AmmoCycle_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "ammopricecheck",			Cmd_AmmoPriceCheck_f,		CMD_NOINTERMISSION },
	{ "arbitraryprint",			Cmd_ArbitraryPrint_f,		CMD_NEEDCHEATS },
	{ "butterfingers",			Cmd_Butterfingers_f,		CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "buyammo",				Cmd_BuyAmmo_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "buyvendor",				Cmd_BuyItem_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "callvote",				Cmd_CallVote_f,				CMD_NOINTERMISSION },
	{ "callteamvote",			Cmd_CallTeamVote_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "checkbotreach",			AIMod_CheckMapPaths,		CMD_NEEDCHEATS },
	{ "checkobjectivesreach",	AIMod_CheckObjectivePaths,	CMD_NEEDCHEATS },
	{ "closeentities",			Cmd_CloseEntities_f,		0 },
	{ "closeVendor",			Cmd_CloseVendor_f,			0 },
	{ "credits",				Cmd_Credits_f,				0 },
	{ "crystal1",				Cmd_Crystal1_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "crystal2",				Cmd_Crystal2_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "debuginventory",			Cmd_ShowInv_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "dismember",				Cmd_Dismember_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "dumpweaponlist_sv",		Cmd_DumpWeaponList_f,		0 },
	{ "equip",					Cmd_EquipItem_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "equipjetpack",			Cmd_EquipJetpack_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "equipshield",			Cmd_EquipShield_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "follow",					Cmd_Follow_f,				CMD_NOINTERMISSION },
	{ "follownext",				Cmd_FollowNext_f,			CMD_NOINTERMISSION },
	{ "followprev",				Cmd_FollowPrev_f,			CMD_NOINTERMISSION },
	{ "give",					Cmd_Give_f,					CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "givebuff",				Cmd_GiveBuff_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "giveother",				Cmd_GiveOther_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION },
	{ "god",					Cmd_God_f,					CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "handcut",				Cmd_Wrists_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "headexplodey",			Cmd_HeadExplodey_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "holdme",					Cmd_Holdme_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "inventorydestroy",		Cmd_DestroyItem_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "inventorysell",			Cmd_SellItem_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "inventoryuse",			Cmd_ItemUse_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "itemcheck",				Cmd_ItemCheck_f,			0 },
	{ "itemlookup",				Cmd_ItemLookup_f,			0 },
	{ "kill",					Cmd_Kill_f,					CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "killother",				Cmd_KillOther_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION },
	{ "knockmedown",			Cmd_KnockMeDown_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "levelshot",				Cmd_LevelShot_f,			CMD_NEEDCHEATS },
	{ "loveandpeace",			Cmd_LoveAndPeace_f,			CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "myammo",					Cmd_MyAmmo_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "noclip",					Cmd_Noclip_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_ONLYALIVE },
	{ "notarget",				Cmd_Notarget_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "npc",					Cmd_NPC_f,					CMD_NEEDCHEATS },
	{ "pay",					Cmd_Pay_f,					CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "printweaponlist_sv",		Cmd_PrintWeaponList_f,		0 },
	{ "printbufflist",			Cmd_PrintBuffList_f,		0 },
	{ "relax",					Cmd_Relax_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "resendInv",				Cmd_ResendInv_f,			0 },	
	{ "say",					Cmd_SayLocal_f,				0 },
	{ "sayact",					Cmd_SayAct_f,				0 },
	{ "sayglobal",				Cmd_SayGlobal_f,			0 },
	{ "sayteam",				Cmd_SayTeam_f,				0 },
	{ "sayyell",				Cmd_SayYell_f,				0 },
	{ "saywhisper",				Cmd_SayWhisper_f,			0 },
	{ "score",					Cmd_Score_f,				0 },
	{ "setviewpos",				Cmd_SetViewpos_f,			0 },
	{ "tceval",					Cmd_TcEval_f,				0 },
	{ "tcevalmulti",			Cmd_TcEvalMulti_f,			0 },
	{ "team",					Cmd_Team_f,					CMD_NOINTERMISSION },
	{ "teamvote",				Cmd_TeamVote_f,				CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "tell",					Cmd_Tell_f,					0 },
	{ "t_use",					Cmd_TUse_f,					CMD_NEEDCHEATS | CMD_ONLYALIVE },
	{ "togglesaber",			Cmd_ToggleSaber_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "unequip",				Cmd_UnequipItem_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "unequipjetpack",			Cmd_JetpackUnequipped,		CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "unequipshield",			Cmd_ShieldUnequipped,		CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
	{ "voice_cmd",				Cmd_VoiceCommand_f,			CMD_NOINTERMISSION | CMD_NOSPECTATOR },
	{ "vote",					Cmd_Vote_f,					CMD_NOINTERMISSION },
	{ "where",					Cmd_Where_f,				0 },
	{ "wrists",					Cmd_Wrists_f,				CMD_NEEDCHEATS | CMD_NOINTERMISSION | CMD_NOSPECTATOR | CMD_ONLYALIVE },
};

static const size_t numCommands = ARRAY_LEN( commands );

void ClientCommand( int clientNum ) {
	gentity_t *ent = g_entities + clientNum;
	if ( !ent->client || ent->client->pers.connected != CON_CONNECTED ) {
		return;		// not fully in game yet
	}

	char cmd[MAX_TOKEN_CHARS];
	trap->Argv( 0, cmd, sizeof( cmd ) );

	//rww - redirect bot commands
	if (strstr(cmd, "bot_") && AcceptBotCommand(cmd, ent))
	{
		return;
	}
	//end rww

	// Check for Glua bound commands
	if (GLua_Command(clientNum, cmd)) return;

	// Check team commands.
	if ( TeamCommand( clientNum, cmd, NULL )) return;

	const command_t *command = (command_t *)Q_LinearSearch( cmd, commands, numCommands, sizeof( commands[0] ), cmdcmp );

	if ( !command )
	{
		trap->SendServerCommand( clientNum, va( "print \"Unknown command %s\n\"", cmd ) );
		return;
	}
	else if ( (command->flags & CMD_NOINTERMISSION) && ( level.intermissionQueued || level.intermissiontime ) )
	{
		// It's intermission time and this command couldn't be done during intermission.
		trap->SendServerCommand(clientNum, va("print \"%s (%s) \n\"", G_GetStringEdString("MP_SVGAME", "CANNOT_TASK_INTERMISSION"), cmd));
		return;
	}
	else if ( (command->flags & CMD_NEEDCHEATS) && !CheatsOk( ent ) )
	{
		// This command requires cheats and we don't have cheats
		trap->SendServerCommand(clientNum, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "NOCHEATS")));
		return;
	}
	else if ( (command->flags & CMD_ONLYALIVE)
		&& !JKG_ClientAlive(ent))
	{
		// This command requires us to be alive and we aren't.
		trap->SendServerCommand(clientNum, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		return;
	}
	else if ( (command->flags & CMD_NOSPECTATOR) && ent->client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		// This command requires us to not be a spectator and we are.
		// FIXME: Use a string from MP_SVGAME!!
		if (command->flags & CMD_ONLYALIVE)
			trap->SendServerCommand(clientNum, va("print \"%s\n\"", G_GetStringEdString("MP_SVGAME", "MUSTBEALIVE")));
		else
			trap->SendServerCommand(clientNum, "print \"That command cannot be used as a spectator.\n\"");
		return;
	}
	else {
		command->function(ent);
	}
}
