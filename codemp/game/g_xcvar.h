/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
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

#ifdef XCVAR_PROTO
	#define XCVAR_DEF( name, defVal, update, flags, announce ) extern vmCvar_t name;
#endif

#ifdef XCVAR_DECL
	#define XCVAR_DEF( name, defVal, update, flags, announce ) vmCvar_t name;
#endif

#ifdef XCVAR_LIST
	#define XCVAR_DEF( name, defVal, update, flags, announce ) { & name , #name , defVal , update , flags , announce },
#endif

XCVAR_DEF( bg_fighterAltControl,		"0",			NULL,				CVAR_SYSTEMINFO,								qtrue )
XCVAR_DEF( capturelimit,				"8",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_NORESTART,	qtrue )
XCVAR_DEF( com_optvehtrace,				"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( d_altRoutes,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_asynchronousGroupAI,		"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_break,						"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_JediAI,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_noGroupAI,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_noroam,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_npcai,						"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_npcaiming,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_npcfreeze,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_noIntermissionWait,		"0",			NULL,				CVAR_NONE,										qfalse ) //0 == wait for players to readyup, 1==don't wait, 2==wait 2 minutes, 3==wait 3 minutes etc
XCVAR_DEF( d_patched,					"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_perPlayerGhoul2,			"0",			NULL,				CVAR_CHEAT,										qtrue )
XCVAR_DEF( d_powerDuelPrint,			"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( d_projectileGhoul2Collision,	"1",			NULL,				CVAR_CHEAT,										qtrue )
XCVAR_DEF( d_saberCombat,				"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( d_saberKickTweak,			"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( d_saberStanceDebug,			"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( d_siegeSeekerNPC,			"0",			NULL,				CVAR_CHEAT,										qtrue )
XCVAR_DEF( dedicated,					"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( developer,					"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( dmflags,						"0",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE,					qtrue )
XCVAR_DEF( duel_fraglimit,				"10",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_NORESTART,	qtrue )
XCVAR_DEF( fraglimit,					"20",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_NORESTART,	qtrue )
XCVAR_DEF( g_adaptRespawn,				"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_allowDuelSuicide,			"1",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_allowHighPingDuelist,		"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_allowNPC,					"1",			NULL,				CVAR_CHEAT,										qtrue )
XCVAR_DEF( g_allowTeamVote,				"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_allowVote,					"-1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_antiFakePlayer,			"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_armBreakage,				"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_austrian,					"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_autoMapCycle,				"0",			NULL,				CVAR_ARCHIVE|CVAR_NORESTART,					qtrue )
XCVAR_DEF( g_banIPs,					"",				NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_charRestrictRGB,			"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_duelWeaponDisable,			"1",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_LATCH,		qtrue )
XCVAR_DEF( g_debugAlloc,				"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_debugDamage,				"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_debugMelee,				"0",			NULL,				CVAR_SERVERINFO,								qtrue )
XCVAR_DEF( g_debugMove,					"0",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_debugSaberLocks,			"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( g_debugServerSkel,			"0",			NULL,				CVAR_CHEAT,										qfalse )
#ifdef _DEBUG
XCVAR_DEF( g_disableServerG2,			"0",			NULL,				CVAR_NONE,										qtrue )
#endif
XCVAR_DEF( g_dismember,					"2",			NULL,				CVAR_ARCHIVE,									qtrue )
//XCVAR_DEF( g_engineModifications,		"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_ff_objectives,				"0",			NULL,				CVAR_CHEAT|CVAR_NORESTART,						qtrue )
XCVAR_DEF( g_filterBan,					"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_forceBasedTeams,			"0",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_LATCH,		qfalse )
XCVAR_DEF( g_forceClientUpdateRate,		"250",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_forceDodge,				"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_forcePowerDisable,			"1",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_LATCH,		qtrue )
XCVAR_DEF( g_forceRegenTime,			"240",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_forceRespawn,				"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_fraglimitVoteCorrection,	"1",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_friendlyFire,				"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_friendlySaber,				"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_g2TraceLod,				"3",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_gametype,					"0",			NULL,				CVAR_SERVERINFO|CVAR_LATCH,						qfalse )
XCVAR_DEF( g_gravity,					"800",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_inactivity,				"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_knockback,					"1000",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_locationBasedDamage,		"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_log,						"games.log",	NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_logClientInfo,				"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_logSync,					"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_maxConnPerIP,				"3",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_maxForceRank,				"0",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_LATCH,		qfalse )
XCVAR_DEF( g_maxGameClients,			"0",			NULL,				CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE,		qfalse )
XCVAR_DEF( g_motd,						"",				NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_needpass,					"0",			NULL,				CVAR_SERVERINFO|CVAR_ROM,						qfalse )
XCVAR_DEF( g_noSpecMove,				"0",			NULL,				CVAR_SERVERINFO,								qtrue )
XCVAR_DEF( g_npcspskill,				"0",			NULL,				CVAR_ARCHIVE|CVAR_INTERNAL,						qfalse )
XCVAR_DEF( g_password,					"",				NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_powerDuelEndHealth,		"90",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_powerDuelStartHealth,		"150",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_privateDuel,				"1",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_randFix,					"1",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_restarted,					"0",			NULL,				CVAR_ROM,										qfalse )
XCVAR_DEF( g_saberBladeFaces,			"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_saberDamageScale,			"1",			NULL,				CVAR_ARCHIVE,									qtrue )
#ifdef DEBUG_SABER_BOX
XCVAR_DEF( g_saberDebugBox,				"0",			NULL,				CVAR_CHEAT,										qfalse )
#endif
#ifndef FINAL_BUILD
XCVAR_DEF( g_saberDebugPrint,			"0",			NULL,				CVAR_CHEAT,										qfalse )
#endif
XCVAR_DEF( g_saberDmgDelay_Idle,		"350",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberDmgDelay_Wound,		"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberDmgVelocityScale,		"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberLockFactor,			"2",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberLocking,				"1",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberLockRandomNess,		"2",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_saberRealisticCombat,		"0",			NULL,				CVAR_CHEAT,										qfalse )
XCVAR_DEF( g_saberRestrictForce,		"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_saberTraceSaberFirst,		"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_saberWallDamageScale,		"0.4",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_securityLog,				"1",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_showDuelHealths,			"0",			NULL,				CVAR_SERVERINFO,								qfalse )
XCVAR_DEF( g_siegeRespawn,				"20",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_siegeTeam1,				"none",			NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO,					qfalse )
XCVAR_DEF( g_siegeTeam2,				"none",			NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO,					qfalse )
XCVAR_DEF( g_siegeTeamSwitch,			"1",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE,					qfalse )
XCVAR_DEF( g_slowmoDuelEnd,				"0",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_smoothClients,				"1",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_spawnInvulnerability,		"5000",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( g_speed,						"250",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_statLog,					"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_statLogFile,				"statlog.log",	NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_stepSlideFix,				"1",			NULL,				CVAR_SERVERINFO,								qtrue )
XCVAR_DEF( g_synchronousClients,		"0",			NULL,				CVAR_SYSTEMINFO,								qfalse )
XCVAR_DEF( g_teamAutoJoin,				"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_teamSwitchTime,			"5",			NULL,				CVAR_ARCHIVE,									qtrue )	//how long to wait before switching teams is allowed (in seconds - forces at least 1 sec)
XCVAR_DEF( g_teamsLocked,				"0",			NULL,				CVAR_ARCHIVE,									qtrue )	//teams are locked after 20% of timelimit has passed (new players are allowed to switch teams for 3 mins)
XCVAR_DEF( g_teamForceBalance,			"0",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_timeouttospec,				"70",			NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_userinfoValidate,			"25165823",		NULL,				CVAR_ARCHIVE,									qfalse )
XCVAR_DEF( g_useWhileThrowing,			"1",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( g_voteDelay,					"3000",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( g_warmup,					"20",			NULL,				CVAR_ARCHIVE,									qtrue )
XCVAR_DEF( gamedate,					__DATE__,		NULL,				CVAR_ROM,										qfalse )
XCVAR_DEF( gamename,					GAMEVERSION,	NULL,				CVAR_SERVERINFO|CVAR_ROM,						qfalse ) //reports version string and gamename
XCVAR_DEF( gameversion, JKG_VERSION JKG_VERSION_SUFFIX, NULL,				CVAR_SERVERINFO|CVAR_ROM,						qfalse ) //reports just the version string
XCVAR_DEF( pmove_fixed,					"0",			NULL,				CVAR_SYSTEMINFO|CVAR_ARCHIVE,					qtrue )
XCVAR_DEF( pmove_float,					"1",			NULL,				CVAR_SYSTEMINFO|CVAR_ARCHIVE,					qtrue )
XCVAR_DEF( pmove_msec,					"8",			NULL,				CVAR_SYSTEMINFO|CVAR_ARCHIVE,					qtrue )
XCVAR_DEF( RMG,							"0",			NULL,				CVAR_NONE,										qtrue )
XCVAR_DEF( sv_cheats,					"1",			NULL,				CVAR_NONE,										qfalse )
XCVAR_DEF( sv_fps,						"40",			NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO,					qtrue )
XCVAR_DEF( sv_maxclients,				"8",			NULL,				CVAR_SERVERINFO|CVAR_LATCH|CVAR_ARCHIVE,		qfalse )
XCVAR_DEF( timelimit,					"45",			NULL,				CVAR_SERVERINFO|CVAR_ARCHIVE|CVAR_NORESTART,	qtrue )

// Jedi Knight Galaxies
XCVAR_DEF( jkg_shop_replenish_time,		"300",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_startingCredits,			"500",			NULL,				CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO,		false )
XCVAR_DEF( jkg_bounty,					"225",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_killsPerBounty,			"3",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_maxKillStreakBounty,		"7",			NULL,				CVAR_ARCHIVE,									true )	//the max number of killstreaks
XCVAR_DEF( jkg_creditsPerKill,			"150",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_passiveCreditsAmount,	"15",			NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_LATCH,		true )	//0 >= disables, default: 15
XCVAR_DEF( jkg_passiveCreditsRate,		"30000",		NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_LATCH,		true )	//default: 30000
XCVAR_DEF( jkg_passiveCreditsWait,		"60000",		NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO|CVAR_LATCH,		true )	//how long to wait before starting passiveCredits, default: 60000 (60 seconds)
XCVAR_DEF( jkg_passiveUnderdogBonus,	"1",			NULL,				CVAR_ARCHIVE,									true )	//give additional bonus credits to the losing team and 50% off ammo costs
XCVAR_DEF( jkg_underdogBonus,			"1",			NULL,				CVAR_ARCHIVE,									true )	//give bonus credits to late joiners who join the losing team
XCVAR_DEF( jkg_teamKillBonus,			"10",			NULL,				CVAR_ARCHIVE,									true )	//0 >= disables, default: 10
XCVAR_DEF( jkg_payTime,					"5",			NULL,				CVAR_ARCHIVE,									true )  //0 >= disables /pay cmd, default: 5 (mins)
XCVAR_DEF( jkg_minAssistAwardRatio,		"25",			NULL,				CVAR_ARCHIVE,									true )	//value is % out of 100, Max allowed value is 50
XCVAR_DEF( jkg_creditsPerTeamCapture,	"125",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_creditsPerCapture,		"300",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_creditsPerReturn,		"75",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_buyAnnounce,				"1",			NULL,				CVAR_ARCHIVE,									true )	//0 = don't announce, 1 = announce to same team, 2+ = announce to all
XCVAR_DEF( jkg_buyAnnounceThreshold,	"500",			NULL,				CVAR_ARCHIVE,									false )
XCVAR_DEF( jkg_announceShopRefresh,		"1",			NULL,				CVAR_ARCHIVE,									false )	//have npc vendors announce when their treasure classes update?
XCVAR_DEF( g_listEntity,				"0",			NULL,				CVAR_CHEAT,										false )
XCVAR_DEF( jkg_arearestrictions,		"0",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_deathTimer,				"1",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_antifakeplayer,			"0",			NULL,				CVAR_ARCHIVE,									false )
XCVAR_DEF( jkg_chatFloodProtect,		"1",			NULL,				CVAR_ARCHIVE,									false )
XCVAR_DEF( jkg_startingGun,				"pistol_DL-18",	NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO,					true )
XCVAR_DEF( jkg_startingSaberDuel,		"saber_kyle",	NULL,				CVAR_ARCHIVE|CVAR_SERVERINFO,					true )
XCVAR_DEF( g_banfile,					"",				NULL,				CVAR_ARCHIVE,									false )
XCVAR_DEF( jkg_healthRegen,				"1",			NULL,				CVAR_ARCHIVE,									true )
XCVAR_DEF( jkg_healthRegenDelay,		"25000",		NULL,				CVAR_ARCHIVE,									true )	//after 25 secs no dmg, start regen
XCVAR_DEF( jkg_healthRegenSpeed,		"1500",			NULL,				CVAR_ARCHIVE,									true )  //every 1.5 secs, regen a health
XCVAR_DEF( jkg_safeTreasureClasses,		"0",			NULL,				CVAR_ARCHIVE|CVAR_LATCH,						false )	//temporarily defaults to off until we can fix the treasureclass checker
XCVAR_DEF( jkg_minVendors,				"2",			NULL,				CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO,		true)	//0 == don't spawn extra vendors, 1 == make sure we have at least 1 vendor, 2 == make sure we have at least 2, etc.
XCVAR_DEF( jkg_removenpcbody,			"0",			NULL,				CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO,		true )
XCVAR_DEF( jkg_removenpctime_default,	"10000",		NULL,				CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO,		true )
XCVAR_DEF( jkg_removenpctime_droid,		"0",			NULL,				CVAR_ARCHIVE|CVAR_LATCH|CVAR_SERVERINFO,		true )
XCVAR_DEF( jkg_allowDebuffKills,		"1",			NULL,				CVAR_ARCHIVE,									true )	//0 == carebear debuffs, 1 == debuffs are deadly as specified by .wpn, 2 == all damaging debuffs are deadly
XCVAR_DEF( jkg_allowDodge,				"1",			NULL,				CVAR_ARCHIVE,									true ) //allow roll dodging?
XCVAR_DEF( jkg_heatDissipateTime,		"100",			NULL,				CVAR_ARCHIVE,									true ) // dissipate rate for heat

#undef XCVAR_DEF
