#============================================================================
# Copyright (C) 2013 - 2015, OpenJK contributors
# Copyright (C) 2009 - 2024, JKG Team
#
# This file is part of the OpenJK/JKG source code.
# 
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.  JKG is based partially
# on this project and has the same license terms as OpenJK.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

cmake_minimum_required(VERSION 3.2.0...3.30.5)

# Make sure the user is not executing this script directly
if(NOT InJKG)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InJKG)

set(MPGameIncludeDirectories "${MPDir}" "${MPDir}/libraries")
if(WIN32)
	set(MPGameLibraries "winmm")
endif(WIN32)

set(MPGameDefines ${MPSharedDefines} "_GAME" )
set(MPGameGameFiles
	"${MPDir}/game/ai_dominance.cpp"
	"${MPDir}/game/ai_dominance_astar.cpp"
	"${MPDir}/game/ai_dominance_bot.cpp"
	"${MPDir}/game/ai_dominance_evasion.cpp"
	"${MPDir}/game/ai_main.cpp"
	"${MPDir}/game/ai_util.cpp"
	"${MPDir}/game/ai_wpnav.cpp"
	"${MPDir}/game/animtable.cpp"
	"${MPDir}/game/bg_ammo.cpp"
	"${MPDir}/game/bg_armor.cpp"
	"${MPDir}/game/bg_buffs.cpp"
	"${MPDir}/game/bg_constants.cpp"
	"${MPDir}/game/bg_damage.cpp"
	"${MPDir}/game/bg_g2_utils.cpp"
	"${MPDir}/game/bg_items.cpp"
	"${MPDir}/game/bg_jetpacks.cpp"
	"${MPDir}/game/bg_misc.cpp"
	"${MPDir}/game/bg_npcnames.cpp"
	"${MPDir}/game/bg_panimate.cpp"
	"${MPDir}/game/bg_pmove.cpp"
	"${MPDir}/game/bg_saber.cpp"
	"${MPDir}/game/bg_saberData.cpp"
	"${MPDir}/game/bg_saberLoad.cpp"
	"${MPDir}/game/bg_slidemove.cpp"
	"${MPDir}/game/bg_vmove.cpp"
	"${MPDir}/game/bg_weapons.cpp"
	"${MPDir}/game/bg_weapons_load.cpp"
	"${MPDir}/game/g_active.cpp"
	"${MPDir}/game/g_bot.cpp"
	"${MPDir}/game/g_client.cpp"
	"${MPDir}/game/g_cmds.cpp"
	"${MPDir}/game/g_combat.cpp"
	"${MPDir}/game/g_exphysics.cpp"
	"${MPDir}/game/g_ICARUScb.cpp"
	"${MPDir}/game/g_items.cpp"
	"${MPDir}/game/g_log.cpp"
	"${MPDir}/game/g_main.cpp"
	"${MPDir}/game/g_mem.cpp"
	"${MPDir}/game/g_misc.cpp"
	"${MPDir}/game/g_missile.cpp"
	"${MPDir}/game/g_mover.cpp"
	"${MPDir}/game/g_nav.cpp"
	"${MPDir}/game/g_navnew.cpp"
	"${MPDir}/game/g_object.cpp"
	"${MPDir}/game/g_session.cpp"
	"${MPDir}/game/g_spawn.cpp"
	"${MPDir}/game/g_svcmds.cpp"
	"${MPDir}/game/g_target.cpp"
	"${MPDir}/game/g_team.cpp"
	"${MPDir}/game/g_timer.cpp"
	"${MPDir}/game/g_trigger.cpp"
	"${MPDir}/game/g_turret.cpp"
	"${MPDir}/game/g_turret_G2.cpp"
	"${MPDir}/game/g_utils.cpp"
	"${MPDir}/game/g_weapon.cpp"
	"${MPDir}/game/jkg_bans.cpp"
	"${MPDir}/game/jkg_chatcmds.cpp"
	"${MPDir}/game/jkg_custombounds.cpp"
	"${MPDir}/game/jkg_damageareas.cpp"
	"${MPDir}/game/jkg_dynarrays.cpp"
	"${MPDir}/game/jkg_equip.cpp"
	"${MPDir}/game/jkg_gangwars.cpp"
	"${MPDir}/game/jkg_keypairs.cpp"
	"${MPDir}/game/jkg_playerisolation.cpp"
	"${MPDir}/game/jkg_rconbans.cpp"
	"${MPDir}/game/jkg_supremacy_flags.cpp"
	"${MPDir}/game/jkg_supremacy_flagsys.cpp"
	"${MPDir}/game/jkg_supremacy_spawnsys.cpp"
	"${MPDir}/game/jkg_team.cpp"
	"${MPDir}/game/jkg_trade.cpp"
	"${MPDir}/game/jkg_treasureclass.cpp"
	"${MPDir}/game/jkg_utilityfunc.cpp"
	"${MPDir}/game/NPC.cpp"
	"${MPDir}/game/NPC_AI_Atst.cpp"
	"${MPDir}/game/NPC_AI_Default.cpp"
	"${MPDir}/game/NPC_AI_Droid.cpp"
	"${MPDir}/game/NPC_AI_GalakMech.cpp"
	"${MPDir}/game/NPC_AI_Grenadier.cpp"
	"${MPDir}/game/NPC_AI_Howler.cpp"
	"${MPDir}/game/NPC_AI_Humanoid.cpp"
	"${MPDir}/game/NPC_AI_ImperialProbe.cpp"
	"${MPDir}/game/NPC_AI_Interrogator.cpp"
	"${MPDir}/game/NPC_AI_Mark1.cpp"
	"${MPDir}/game/NPC_AI_Mark2.cpp"
	"${MPDir}/game/NPC_AI_MineMonster.cpp"
	"${MPDir}/game/NPC_AI_Rancor.cpp"
	"${MPDir}/game/NPC_AI_Remote.cpp"
	"${MPDir}/game/NPC_AI_Seeker.cpp"
	"${MPDir}/game/NPC_AI_Sentry.cpp"
	"${MPDir}/game/NPC_AI_Sniper.cpp"
	"${MPDir}/game/NPC_AI_Stormtrooper.cpp"
	"${MPDir}/game/NPC_AI_Utils.cpp"
	"${MPDir}/game/NPC_AI_Wampa.cpp"
	"${MPDir}/game/NPC_behavior.cpp"
	"${MPDir}/game/NPC_combat.cpp"
	"${MPDir}/game/NPC_Conversations.cpp"
	"${MPDir}/game/NPC_CoverPoints.cpp"
	"${MPDir}/game/NPC_goal.cpp"
	"${MPDir}/game/NPC_misc.cpp"
	"${MPDir}/game/NPC_move.cpp"
	"${MPDir}/game/NPC_reactions.cpp"
	"${MPDir}/game/NPC_senses.cpp"
	"${MPDir}/game/NPC_sounds.cpp"
	"${MPDir}/game/NPC_spawn.cpp"
	"${MPDir}/game/NPC_stats.cpp"
	"${MPDir}/game/NPC_utils.cpp"
	"${MPDir}/game/NPC_stats.cpp"
	"${MPDir}/game/NPC_Waypointing.cpp"
	"${MPDir}/game/tri_coll_test.cpp"
	"${MPDir}/game/w_force.cpp"
	"${MPDir}/game/w_melee.cpp"
	"${MPDir}/game/w_saber.cpp"
	"${MPDir}/game/ai.h"
	"${MPDir}/game/ai_main.h"
	"${MPDir}/game/anims.h"
	"${MPDir}/game/b_local.h"
	"${MPDir}/game/b_public.h"
	"${MPDir}/game/bg_ammo.h"
	"${MPDir}/game/bg_armor.h"
	"${MPDir}/game/bg_buffs.h"
	"${MPDir}/game/bg_damage.h"
	"${MPDir}/game/bg_items.h"
	"${MPDir}/game/bg_jetpacks.h"
	"${MPDir}/game/bg_local.h"
	"${MPDir}/game/bg_npcnames.h"
	"${MPDir}/game/bg_public.h"
	"${MPDir}/game/bg_saga.h"
	"${MPDir}/game/bg_weapons.h"
	"${MPDir}/game/chars.h"
	"${MPDir}/game/g_ICARUScb.h"
	"${MPDir}/game/g_local.h"
	"${MPDir}/game/g_nav.h"
	"${MPDir}/game/g_public.h"
	"${MPDir}/game/g_team.h"
	"${MPDir}/game/g_xcvar.h"
	"${MPDir}/game/inv.h"
	"${MPDir}/game/jkg_bans.h"
	"${MPDir}/game/jkg_chatcmds.h"
	"${MPDir}/game/jkg_damageareas.h"
	"${MPDir}/game/jkg_dynarrays.h"
	"${MPDir}/game/jkg_gangwars.h"
	"${MPDir}/game/jkg_keypairs.h"
	"${MPDir}/game/jkg_navmesh_creator.h"
	"${MPDir}/game/jkg_utilityfunc.h"
	"${MPDir}/game/match.h"
	"${MPDir}/game/npc_headers.h"
	"${MPDir}/game/say.h"
	"${MPDir}/game/surfaceflags.h"
	"${MPDir}/game/teams.h"
	"${MPDir}/game/w_saber.h"
	"${MPDir}/game/z_global_defines.h"
	)
source_group("game" FILES ${MPGameGameFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameGameFiles})

set(MPGameCommonFiles
	"${MPDir}/qcommon/cppStringHelp.cpp"
	"${MPDir}/qcommon/cppStringHelp.h"
	"${MPDir}/qcommon/q_math.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	"${MPDir}/qcommon/disablewarnings.h"
	"${MPDir}/qcommon/q_shared.h"
	"${MPDir}/qcommon/q_platform.h"
	"${MPDir}/qcommon/tags.h"
	)
source_group("common" FILES ${MPGameCommonFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameCommonFiles})

set(MPGameCgameFiles
	"${MPDir}/cgame/animtable.h"
	"${MPDir}/cgame/cg_public.h"
	)
source_group("cgame" FILES ${MPGameCgameFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameCgameFiles})

set(MPGameRendererFiles
	"${MPDir}/rd-common/tr_types.h"
	)
source_group("rd-common" FILES ${MPGameRendererFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameRendererFiles})

set(MPGameBotlibFiles
	"${MPDir}/botlib/be_aas.h"
	"${MPDir}/botlib/be_ai_char.h"
	"${MPDir}/botlib/be_ai_chat.h"
	"${MPDir}/botlib/be_ai_gen.h"
	"${MPDir}/botlib/be_ai_goal.h"
	"${MPDir}/botlib/be_ai_move.h"
	"${MPDir}/botlib/be_ai_weap.h"
	"${MPDir}/botlib/be_ea.h"
	"${MPDir}/botlib/botlib.h"
	)
source_group("botlib" FILES ${MPGameBotlibFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameBotlibFiles})

set(MPGameG2Files
	"${MPDir}/ghoul2/G2.h"
	)
source_group("ghoul2" FILES ${MPGameG2Files})
set(MPGameFiles ${MPGameFiles} ${MPGameG2Files})

set(MPGameIcarusFiles
	"${MPDir}/icarus/interpreter.h"
	"${MPDir}/icarus/Q3_Interface.h"
	)
source_group("icarus" FILES ${MPGameIcarusFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameIcarusFiles})

set(MPGameUiFiles
	"${MPDir}/ui/menudef.h"
	)
source_group("ui" FILES ${MPGameUiFiles})
set(MPGameFiles ${MPGameFiles} ${MPGameUiFiles})

set(MPGameGLua
	"${MPDir}/GLua/glua.h"
	"${MPDir}/GLua/glua_bitstream.cpp"
	"${MPDir}/GLua/glua_bitwise.cpp"
	"${MPDir}/GLua/glua_cryptography.cpp"
	"${MPDir}/GLua/glua_cvar.cpp"
	"${MPDir}/GLua/glua_encoding.cpp"
	"${MPDir}/GLua/glua_engine.cpp"
	"${MPDir}/GLua/glua_entity.cpp"
	"${MPDir}/GLua/glua_files.cpp"
	"${MPDir}/GLua/glua_json.cpp"
	"${MPDir}/GLua/glua_main.cpp"
	"${MPDir}/GLua/glua_npc.cpp"
	"${MPDir}/GLua/glua_player.cpp"
	"${MPDir}/GLua/glua_prng.cpp"
	"${MPDir}/GLua/glua_sys.cpp"
	"${MPDir}/GLua/glua_vector.cpp"
	)
source_group("GLUA" FILES ${MPGameGLua})
set(MPGameFiles ${MPGameFiles} ${MPGameGLua})

set(MPGameLibBase128Header
	"${MPDir}/libraries/encoding/base128.h"
	)
source_group("Libraries\\Base128\\Header Files" FILES ${MPGameLibBase128Header})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBase128Header})

set(MPGameLibBase128Source
	"${MPDir}/libraries/encoding/base128.cpp"
	)
source_group("Libraries\\Base128\\Source Files" FILES ${MPGameLibBase128Source})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBase128Source})

set(MPGameLibBase64Header
	"${MPDir}/libraries/encoding/base64.h"
	)
source_group("Libraries\\Base64\\Header Files" FILES ${MPGameLibBase64Header})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBase64Header})

set(MPGameLibBase64Source
	"${MPDir}/libraries/encoding/base64.c"
	)
source_group("Libraries\\Base64\\Source Files" FILES ${MPGameLibBase64Source})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBase64Source})

set(MPGameLibBitstreamHeader
	"${MPDir}/libraries/encoding/bitstream.h"
	)
source_group("Libraries\\BitStream\\Header Files" FILES ${MPGameLibBitstreamHeader})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBitstreamHeader})

set(MPGameLibBitstreamSource
	"${MPDir}/libraries/encoding/bitstream.c"
	)
source_group("Libraries\\BitStream\\Source Files" FILES ${MPGameLibBitstreamSource})
set(MPGameFiles ${MPGameFiles} ${MPGameLibBitstreamSource})

set(MPGameLibJsonHeader
	"${MPDir}/libraries/json/cJSON.h"
	)
source_group("Libraries\\Json\\Header Files" FILES ${MPGameLibJsonHeader})
set(MPGameFiles ${MPGameFiles} ${MPGameLibJsonHeader})

set(MPGameLibJsonSource
	"${MPDir}/libraries/json/cJSON.cpp"
	)
source_group("Libraries\\Json\\Source Files" FILES ${MPGameLibJsonSource})
set(MPGameFiles ${MPGameFiles} ${MPGameLibJsonSource})


set(MPGameLuaHeader
	"${MPDir}/libraries/Lua/lapi.h"
	"${MPDir}/libraries/Lua/lauxlib.h"
	"${MPDir}/libraries/Lua/lcode.h"
	"${MPDir}/libraries/Lua/ldebug.h"
	"${MPDir}/libraries/Lua/ldo.h"
	"${MPDir}/libraries/Lua/lfunc.h"
	"${MPDir}/libraries/Lua/lgc.h"
	"${MPDir}/libraries/Lua/llex.h"
	"${MPDir}/libraries/Lua/llimits.h"
	"${MPDir}/libraries/Lua/lmem.h"
	"${MPDir}/libraries/Lua/lobject.h"
	"${MPDir}/libraries/Lua/lopcodes.h"
	"${MPDir}/libraries/Lua/lparser.h"
	"${MPDir}/libraries/Lua/lstate.h"
	"${MPDir}/libraries/Lua/lstring.h"
	"${MPDir}/libraries/Lua/ltable.h"
	"${MPDir}/libraries/Lua/ltm.h"
	"${MPDir}/libraries/Lua/lua.h"
	"${MPDir}/libraries/Lua/luaconf.h"
	"${MPDir}/libraries/Lua/lualib.h"
	"${MPDir}/libraries/Lua/lundump.h"
	"${MPDir}/libraries/Lua/lvm.h"
	"${MPDir}/libraries/Lua/lzio.h"
	"${MPDir}/qcommon/disablewarnings.h"
	)
source_group("Libraries\\Lua\\Header Files" FILES ${MPGameLuaHeader})
set(MPGameFiles ${MPGameFiles} ${MPGameLuaHeader})

set(MPGameLuaSource
	"${MPDir}/libraries/Lua/lapi.c"
	"${MPDir}/libraries/Lua/lauxlib.c"
	"${MPDir}/libraries/Lua/lbaselib.c"
	"${MPDir}/libraries/Lua/lcode.c"
	"${MPDir}/libraries/Lua/ldblib.c"
	"${MPDir}/libraries/Lua/ldebug.c"
	"${MPDir}/libraries/Lua/ldo.c"
	"${MPDir}/libraries/Lua/ldump.c"
	"${MPDir}/libraries/Lua/lfunc.c"
	"${MPDir}/libraries/Lua/lgc.c"
	"${MPDir}/libraries/Lua/linit.c"
	"${MPDir}/libraries/Lua/liolib.c"
	"${MPDir}/libraries/Lua/llex.c"
	"${MPDir}/libraries/Lua/lmathlib.c"
	"${MPDir}/libraries/Lua/lmem.c"
	"${MPDir}/libraries/Lua/loadlib.c"
	"${MPDir}/libraries/Lua/lobject.c"
	"${MPDir}/libraries/Lua/lopcodes.c"
	"${MPDir}/libraries/Lua/loslib.c"
	"${MPDir}/libraries/Lua/lparser.c"
	"${MPDir}/libraries/Lua/lstate.c"
	"${MPDir}/libraries/Lua/lstring.c"
	"${MPDir}/libraries/Lua/lstrlib.c"
	"${MPDir}/libraries/Lua/ltable.c"
	"${MPDir}/libraries/Lua/ltablib.c"
	"${MPDir}/libraries/Lua/ltm.c"
	"${MPDir}/libraries/Lua/luac.c"
	"${MPDir}/libraries/Lua/lundump.c"
	"${MPDir}/libraries/Lua/lvm.c"
	"${MPDir}/libraries/Lua/lzio.c"
	"${MPDir}/libraries/Lua/print.c"
	)
source_group("Libraries\\Lua\\Source Files" FILES ${MPGameLuaSource})
set(MPGameFiles ${MPGameFiles} ${MPGameLuaSource})

if(BuildSkipCryptography)
# Do nothing
else()
set(MPGameSSLHeader
	"${MPDir}/libraries/openssl/aes.h"
	"${MPDir}/libraries/openssl/asn1.h"
	"${MPDir}/libraries/openssl/asn1_mac.h"
	"${MPDir}/libraries/openssl/asn1t.h"
	"${MPDir}/libraries/openssl/bio.h"
	"${MPDir}/libraries/openssl/blowfish.h"
	"${MPDir}/libraries/openssl/bn.h"
	"${MPDir}/libraries/openssl/buffer.h"
	"${MPDir}/libraries/openssl/camellia.h"
	"${MPDir}/libraries/openssl/cast.h"
	"${MPDir}/libraries/openssl/cms.h"
	"${MPDir}/libraries/openssl/comp.h"
	"${MPDir}/libraries/openssl/conf.h"
	"${MPDir}/libraries/openssl/conf_api.h"
	"${MPDir}/libraries/openssl/crypto.h"
	"${MPDir}/libraries/openssl/des.h"
	"${MPDir}/libraries/openssl/des_old.h"
	"${MPDir}/libraries/openssl/dh.h"
	"${MPDir}/libraries/openssl/dsa.h"
	"${MPDir}/libraries/openssl/dso.h"
	"${MPDir}/libraries/openssl/dtls1.h"
	"${MPDir}/libraries/openssl/e_os2.h"
	"${MPDir}/libraries/openssl/ebcdic.h"
	"${MPDir}/libraries/openssl/ec.h"
	"${MPDir}/libraries/openssl/ecdh.h"
	"${MPDir}/libraries/openssl/ecdsa.h"
	"${MPDir}/libraries/openssl/engine.h"
	"${MPDir}/libraries/openssl/err.h"
	"${MPDir}/libraries/openssl/evp.h"
	"${MPDir}/libraries/openssl/hmac.h"
	"${MPDir}/libraries/openssl/idea.h"
	"${MPDir}/libraries/openssl/krb5_asn.h"
	"${MPDir}/libraries/openssl/kssl.h"
	"${MPDir}/libraries/openssl/lhash.h"
	"${MPDir}/libraries/openssl/md4.h"
	"${MPDir}/libraries/openssl/md5.h"
	"${MPDir}/libraries/openssl/mdc2.h"
	"${MPDir}/libraries/openssl/modes.h"
	"${MPDir}/libraries/openssl/obj_mac.h"
	"${MPDir}/libraries/openssl/objects.h"
	"${MPDir}/libraries/openssl/ocsp.h"
	"${MPDir}/libraries/openssl/opensslconf.h"
	"${MPDir}/libraries/openssl/opensslv.h"
	"${MPDir}/libraries/openssl/ossl_typ.h"
	"${MPDir}/libraries/openssl/pem.h"
	"${MPDir}/libraries/openssl/pem2.h"
	"${MPDir}/libraries/openssl/pkcs12.h"
	"${MPDir}/libraries/openssl/pkcs7.h"
	"${MPDir}/libraries/openssl/pqueue.h"
	"${MPDir}/libraries/openssl/rand.h"
	"${MPDir}/libraries/openssl/rc2.h"
	"${MPDir}/libraries/openssl/rc4.h"
	"${MPDir}/libraries/openssl/ripemd.h"
	"${MPDir}/libraries/openssl/rsa.h"
	"${MPDir}/libraries/openssl/safestack.h"
	"${MPDir}/libraries/openssl/seed.h"
	"${MPDir}/libraries/openssl/sha.h"
	"${MPDir}/libraries/openssl/ssl.h"
	"${MPDir}/libraries/openssl/ssl2.h"
	"${MPDir}/libraries/openssl/ssl23.h"
	"${MPDir}/libraries/openssl/ssl3.h"
	"${MPDir}/libraries/openssl/stack.h"
	"${MPDir}/libraries/openssl/symhacks.h"
	"${MPDir}/libraries/openssl/tls1.h"
	"${MPDir}/libraries/openssl/ts.h"
	"${MPDir}/libraries/openssl/txt_db.h"
	"${MPDir}/libraries/openssl/ui.h"
	"${MPDir}/libraries/openssl/ui_compat.h"
	"${MPDir}/libraries/openssl/whirlpool.h"
	"${MPDir}/libraries/openssl/x509.h"
	"${MPDir}/libraries/openssl/x509_vfy.h"
	"${MPDir}/libraries/openssl/x509v3.h"
	)
source_group("Libraries\\OpenSSL\\Header Files" FILES ${MPGameSSLHeader})
set(MPGameFiles ${MPGameFiles} ${MPGameSSLHeader})

if(WIN32)
	target_link_libraries(${MPGame} "${MPDir}/libraries/openssl/ssleay32.lib" "${MPDir}/libraries/openssl/libeay32.lib")
else()
	target_link_libraries(${MPGame} "ssl" "crypto")
endif()
endif() # End BuildSkipCryptography

add_library(${MPGame} SHARED ${MPGameFiles})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPGame} PROPERTIES PREFIX "")
endif()

if(MakeApplicationBundles AND BuildMPEngine)
	install(TARGETS ${MPGame}
		LIBRARY
		DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/JKG"
		COMPONENT ${JKAMPCoreComponent})
elseif(WIN32)
	install(TARGETS ${MPGame}
		RUNTIME
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
else()
	install(TARGETS ${MPGame}
		LIBRARY
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
endif()

set_target_properties(${MPGame} PROPERTIES COMPILE_DEFINITIONS "${MPGameDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPGame} APPEND PROPERTY COMPILE_OPTIONS ${JKG_VISIBILITY_FLAGS})

set_target_properties(${MPGame} PROPERTIES INCLUDE_DIRECTORIES "${MPGameIncludeDirectories}")
set_target_properties(${MPGame} PROPERTIES PROJECT_LABEL "Server Game Library (game)")
# no libraries used
if(MPGameLibraries)
	target_link_libraries(${MPGame} ${MPGameLibraries})
endif(MPGameLibraries)
