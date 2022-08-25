/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
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

#define LOGGING_WEAPONS	

// Weapon statistic logging.
// Nothing super-fancy here, I just want to keep track of, per player:
//		--hom many times a weapon/item is picked up
//		--how many times a weapon/item is used/fired
//		--the total damage done by that weapon
//		--the number of kills by that weapon
//		--the number of deaths while holding that weapon
//		--the time spent with each weapon
//
// Additionally,
//		--how many times each powerup or item is picked up


#ifdef LOGGING_WEAPONS
int G_WeaponLogPickups[MAX_CLIENTS][WP_NUM_WEAPONS];
int G_WeaponLogFired[MAX_CLIENTS][WP_NUM_WEAPONS];
int G_WeaponLogDamage[MAX_CLIENTS];
int G_WeaponLogKills[MAX_CLIENTS];
int G_WeaponLogDeaths[MAX_CLIENTS][WP_NUM_WEAPONS];
int G_WeaponLogFrags[MAX_CLIENTS][MAX_CLIENTS];
int G_WeaponLogTime[MAX_CLIENTS][WP_NUM_WEAPONS];
int G_WeaponLogLastTime[MAX_CLIENTS];
qboolean G_WeaponLogClientTouch[MAX_CLIENTS];
int	G_WeaponLogItems[MAX_CLIENTS][PW_NUM_POWERUPS];

extern char	*modNames[];

#endif //LOGGING_WEAPONS

/*
=================
G_LogWeaponInit
=================
*/
void G_LogWeaponInit(void) {
#ifdef LOGGING_WEAPONS
	memset(G_WeaponLogPickups, 0, sizeof(G_WeaponLogPickups));
	memset(G_WeaponLogFired, 0, sizeof(G_WeaponLogFired));
	memset(G_WeaponLogDamage, 0, sizeof(G_WeaponLogDamage));
	memset(G_WeaponLogKills, 0, sizeof(G_WeaponLogKills));
	memset(G_WeaponLogDeaths, 0, sizeof(G_WeaponLogDeaths));
	memset(G_WeaponLogFrags, 0, sizeof(G_WeaponLogFrags));
	memset(G_WeaponLogTime, 0, sizeof(G_WeaponLogTime));
	memset(G_WeaponLogLastTime, 0, sizeof(G_WeaponLogLastTime));
	memset(G_WeaponLogItems, 0, sizeof(G_WeaponLogItems));
#endif //LOGGING_WEAPONS
}

void QDECL G_LogWeaponPickup(int client, int weaponid)
{
#ifdef LOGGING_WEAPONS
	if (client >= MAX_CLIENTS)
		return;

	G_WeaponLogPickups[client][weaponid]++;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponFire(int client, int weaponid)
{
#ifdef LOGGING_WEAPONS
	int dur;

	if (client >= MAX_CLIENTS)
		return;

	G_WeaponLogFired[client][weaponid]++;
	dur = level.time - G_WeaponLogLastTime[client];
	if (dur > 5000)		// 5 second max.
		G_WeaponLogTime[client][weaponid] += 5000;
	else
		G_WeaponLogTime[client][weaponid] += dur;
	G_WeaponLogLastTime[client] = level.time;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponDamage(int client, int mod, int amount)
{
#ifdef LOGGING_WEAPONS
	// JKG FIX: See above note in G_LogWeaponFire
	if (client >= MAX_CLIENTS) return;

	if (client >= MAX_CLIENTS)
		return;
	G_WeaponLogDamage[client] += amount;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponKill(int client, int mod)
{
#ifdef LOGGING_WEAPONS
	if (client >= MAX_CLIENTS)
		return;
	G_WeaponLogKills[client]++;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponFrag(int attacker, int deadguy)
{
#ifdef LOGGING_WEAPONS
	if ((attacker >= MAX_CLIENTS) || (deadguy >= MAX_CLIENTS))
		return;
	G_WeaponLogFrags[attacker][deadguy]++;
	G_WeaponLogClientTouch[attacker] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponDeath(int client, int weaponid)
{
#ifdef LOGGING_WEAPONS
	if (client >= MAX_CLIENTS)
		return;
	G_WeaponLogDeaths[client][weaponid]++;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponPowerup(int client, int powerupid)
{
#ifdef LOGGING_WEAPONS
	if (client >= MAX_CLIENTS)
		return;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}

void QDECL G_LogWeaponItem(int client, int itemid)
{
#ifdef LOGGING_WEAPONS
	if (client >= MAX_CLIENTS)
		return;
	G_WeaponLogItems[client][itemid]++;
	G_WeaponLogClientTouch[client] = qtrue;
#endif //_LOGGING_WEAPONS
}


// Run through each player.  Print out:
//	-- Most commonly picked up weapon.
//  -- Weapon with which the most time was spent.
//  -- Weapon that was most often died with.
//  -- Damage type with which the most damage was done.
//  -- Damage type with the most kills.
//  -- Weapon with which the most damage was done.
//	-- Weapon with which the most damage was done per shot.
//
// For the whole game, print out:
//  -- Total pickups of each weapon.
//  -- Total time spent with each weapon.
//  -- Total damage done with each weapon.
//  -- Total damage done for each damage type.
//  -- Number of kills with each weapon.
//  -- Number of kills for each damage type.
//  -- Damage per shot with each weapon.
//  -- Number of deaths with each weapon.

void G_LogWeaponOutput(void)
{
#ifdef LOGGING_WEAPONS
	int i, j;
	fileHandle_t weaponfile;
	char string[1024];

	int totalpickups[WP_NUM_WEAPONS];
	int totaltime[WP_NUM_WEAPONS];
	int totaldeaths[WP_NUM_WEAPONS];
	int totaldamage = 0;
	int totalkills = 0;
	int totalshots[WP_NUM_WEAPONS];
	int percharacter[WP_NUM_WEAPONS];
	char info[1024];
	char mapname[128];
	char *nameptr, *unknownname = "<Unknown>";

	if (!g_statLog.integer)
	{
		return;
	}

	G_LogPrintf("*****************************Weapon Log:\n");

	memset(totalpickups, 0, sizeof(totalpickups));
	memset(totaltime, 0, sizeof(totaltime));
	memset(totaldeaths, 0, sizeof(totaldeaths));
	memset(totalshots, 0, sizeof(totalshots));

	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				totalpickups[j] += G_WeaponLogPickups[i][j];
				totaltime[j] += G_WeaponLogTime[i][j];
				totaldeaths[j] += G_WeaponLogDeaths[i][j];
				totalshots[j] += G_WeaponLogFired[i][j];
			}
		}
	}

	G_LogPrintf("\n****Data by Weapon:\n");
	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		G_LogPrintf("%15s:  Pickups: %4d,  Time:  %5d,  Deaths: %5d\n",
			weaponNameFromIndex[j], totalpickups[j], (int)(totaltime[j] / 1000), totaldeaths[j]);
	}*/

	G_LogPrintf("\n****Combat Data by Weapon:\n");
	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		if (totalshots[j] > 0)
		{
			pershot = (float)(totaldamage[j]) / (float)(totalshots[j]);
		}
		else
		{
			pershot = 0;
		}
		G_LogPrintf("%15s:  Damage: %6d,  Kills: %5d,  Dmg per Shot: %f\n",
			weaponNameFromIndex[j], totaldamage[j], totalkills[j], pershot);
	}*/

	G_LogPrintf("\n****Combat Data By Damage Type:\n");
	/*for (j = 0; j<MOD_MAX; j++)
	{
		G_LogPrintf("%25s:  Damage: %6d,  Kills: %5d\n",
			modNames[j], totaldamageMOD[j], totalkillsMOD[j]);
	}*/

	G_LogPrintf("\n");



	// Write the whole weapon statistic log out to a file.
	trap->FS_Open(g_statLogFile.string, &weaponfile, FS_APPEND);
	if (!weaponfile) {	//failed to open file, let's not crash, shall we?
		return;
	}

	// Write out the level name
	trap->GetServerinfo(info, sizeof(info));
	Q_strncpyz(mapname, Info_ValueForKey(info, "mapname"), sizeof(mapname));

	Com_sprintf(string, sizeof(string), "\n\n\nLevel:\t%s\n\n\n", mapname);
	trap->FS_Write(string, strlen(string), weaponfile);


	// Combat data per character

	// Start with Pickups per character
	Com_sprintf(string, sizeof(string), "Weapon Pickups per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogPickups[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totalpickups[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);


	// Weapon fires per character
	Com_sprintf(string, sizeof(string), "Weapon Shots per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogFired[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totalshots[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);


	// Weapon time per character
	Com_sprintf(string, sizeof(string), "Weapon Use Time per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogTime[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totaltime[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);



	// Weapon deaths per character
	Com_sprintf(string, sizeof(string), "Weapon Deaths per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogDeaths[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totaldeaths[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);




	// Weapon damage per character

	Com_sprintf(string, sizeof(string), "Weapon Damage per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!

			// We must grab the totals from the damage types for the player and map them to the weapons.
			memset(percharacter, 0, sizeof(percharacter));
			/*for (j = 0; j<MOD_MAX; j++)
			{
				if (j <= MOD_SENTRY)
				{
					curwp = weaponFromMOD[j];
					percharacter[curwp] += G_WeaponLogDamage[i][j];
				}
			}*/

			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", percharacter[j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totaldamage);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);



	// Weapon kills per character

	Com_sprintf(string, sizeof(string), "Weapon Kills per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	//for (j = 0; j<WP_NUM_WEAPONS; j++)
	//{
	//	Com_sprintf(string, sizeof(string), "\t%s", weaponNameFromIndex[j]);
	//	trap->FS_Write(string, strlen(string), weaponfile);
	//}
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!

			// We must grab the totals from the damage types for the player and map them to the weapons.
			memset(percharacter, 0, sizeof(percharacter));
			/*for (j = 0; j<MOD_MAX; j++)
			{
				if (j <= MOD_SENTRY)
				{
					curwp = weaponFromMOD[j];
					percharacter[curwp] += G_WeaponLogKills[i][j];
				}
			}*/

			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			for (j = 0; j<WP_NUM_WEAPONS; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", percharacter[j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	for (j = 0; j<WP_NUM_WEAPONS; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totalkills);
		trap->FS_Write(string, strlen(string), weaponfile);
	}

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);



	// Damage type damage per character
	Com_sprintf(string, sizeof(string), "Typed Damage per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<MOD_MAX; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", modNames[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			/*for (j = 0; j<MOD_MAX; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogDamage[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}*/

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<MOD_MAX; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totaldamageMOD[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);



	// Damage type kills per character
	Com_sprintf(string, sizeof(string), "Damage-Typed Kills per Player:\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	Com_sprintf(string, sizeof(string), "Player");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<MOD_MAX; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%s", modNames[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/
	Com_sprintf(string, sizeof(string), "\n");
	trap->FS_Write(string, strlen(string), weaponfile);

	// Cycle through each player, give their name and the number of times they picked up each weapon.
	for (i = 0; i<MAX_CLIENTS; i++)
	{
		if (G_WeaponLogClientTouch[i])
		{	// Ignore any entity/clients we don't care about!
			if (g_entities[i].client)
			{
				nameptr = g_entities[i].client->pers.netname;
			}
			else
			{
				nameptr = unknownname;
			}
			trap->FS_Write(nameptr, strlen(nameptr), weaponfile);

			/*for (j = 0; j<MOD_MAX; j++)
			{
				Com_sprintf(string, sizeof(string), "\t%d", G_WeaponLogKills[i][j]);
				trap->FS_Write(string, strlen(string), weaponfile);
			}*/

			Com_sprintf(string, sizeof(string), "\n");
			trap->FS_Write(string, strlen(string), weaponfile);
		}
	}

	// Sum up the totals.
	Com_sprintf(string, sizeof(string), "\n***TOTAL:");
	trap->FS_Write(string, strlen(string), weaponfile);

	/*for (j = 0; j<MOD_MAX; j++)
	{
		Com_sprintf(string, sizeof(string), "\t%d", totalkillsMOD[j]);
		trap->FS_Write(string, strlen(string), weaponfile);
	}*/

	Com_sprintf(string, sizeof(string), "\n\n\n");
	trap->FS_Write(string, strlen(string), weaponfile);


	trap->FS_Close(weaponfile);


#endif //LOGGING_WEAPONS
}

// kef -- if a client leaves the game, clear out all counters he may have set
void QDECL G_ClearClientLog(int client)
{
	int i = 0;

	for (i = 0; i < WP_NUM_WEAPONS; i++)
	{
		G_WeaponLogPickups[client][i] = 0;
	}
	for (i = 0; i < WP_NUM_WEAPONS; i++)
	{
		G_WeaponLogFired[client][i] = 0;
	}
	/*for (i = 0; i < MOD_MAX; i++)
	{
		G_WeaponLogDamage[client][i] = 0;
	}*/
	/*for (i = 0; i < MOD_MAX; i++)
	{
		G_WeaponLogKills[client][i] = 0;
	}*/
	for (i = 0; i < WP_NUM_WEAPONS; i++)
	{
		G_WeaponLogDeaths[client][i] = 0;
	}
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		G_WeaponLogFrags[client][i] = 0;
	}
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		G_WeaponLogFrags[i][client] = 0;
	}
	for (i = 0; i < WP_NUM_WEAPONS; i++)
	{
		G_WeaponLogTime[client][i] = 0;
	}
	G_WeaponLogLastTime[client] = 0;
	G_WeaponLogClientTouch[client] = qfalse;
	for (i = 0; i < PW_NUM_POWERUPS; i++)
	{
		G_WeaponLogItems[client][i] = 0;
	}
}

