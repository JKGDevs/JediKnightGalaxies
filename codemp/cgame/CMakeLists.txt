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

set(MPCGameIncludeDirectories "${MPDir}" "${MPDir}/libraries")
if(WIN32)
	set(MPCGameLibraries "odbc32" "odbccp32") # what are these even?
endif(WIN32)
set(MPCGameDefines ${MPSharedDefines} "_CGAME" )
set(MPCGameGameFiles
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
	"${MPDir}/game/bg_saga.h"
	"${MPDir}/game/bg_shields.cpp"
	"${MPDir}/game/bg_slidemove.cpp"
	"${MPDir}/game/bg_vmove.cpp"
	"${MPDir}/game/bg_weapons.cpp"
	"${MPDir}/game/bg_weapons_load.cpp"
	"${MPDir}/game/jkg_gangwars.cpp"
	"${MPDir}/game/anims.h"
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
	"${MPDir}/game/bg_shields.h"
	"${MPDir}/game/bg_weapons.h"
	"${MPDir}/game/surfaceflags.h"
	"${MPDir}/game/z_global_defines.h"
	)
source_group("game" FILES ${MPCGameGameFiles})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameGameFiles})

set(MPCGameCgameFiles
	"${MPDir}/cgame/cg_cinematics.cpp"
	"${MPDir}/cgame/cg_consolecmds.cpp"
	"${MPDir}/cgame/cg_crossover.cpp"
	"${MPDir}/cgame/cg_draw.cpp"
	"${MPDir}/cgame/cg_drawtools.cpp"
	"${MPDir}/cgame/cg_effects.cpp"
	"${MPDir}/cgame/cg_ents.cpp"
	"${MPDir}/cgame/cg_event.cpp"
	"${MPDir}/cgame/cg_info.cpp"
	"${MPDir}/cgame/cg_light.cpp"
	"${MPDir}/cgame/cg_localents.cpp"
	"${MPDir}/cgame/cg_main.cpp"
	"${MPDir}/cgame/cg_marks.cpp"
	"${MPDir}/cgame/cg_newDraw.cpp"
	"${MPDir}/cgame/cg_players.cpp"
	"${MPDir}/cgame/cg_playerstate.cpp"
	"${MPDir}/cgame/cg_predict.cpp"
	"${MPDir}/cgame/cg_scoreboard.cpp"
	"${MPDir}/cgame/cg_servercmds.cpp"
	"${MPDir}/cgame/cg_snapshot.cpp"
	"${MPDir}/cgame/cg_spawn.cpp"
	"${MPDir}/cgame/cg_turret.cpp"
	"${MPDir}/cgame/cg_view.cpp"
	"${MPDir}/cgame/cg_weaponinit.cpp"
	"${MPDir}/cgame/cg_weapons.cpp"
	"${MPDir}/cgame/fx_bryarpistol.cpp"
	"${MPDir}/cgame/jkg_autowaypoint.cpp"
	"${MPDir}/cgame/jkg_cg_damagetypes.cpp"
	"${MPDir}/cgame/jkg_cg_weaponholster.cpp"
	"${MPDir}/cgame/jkg_chatbox.cpp"
	"${MPDir}/cgame/jkg_chatcmds.cpp"
	"${MPDir}/cgame/jkg_cinbuild.cpp"
	"${MPDir}/cgame/jkg_custombounds.cpp"
	"${MPDir}/cgame/jkg_hud.cpp"
	"${MPDir}/cgame/jkg_minimaps.cpp"
	"${MPDir}/cgame/jkg_splinepaths.cpp"
	"${MPDir}/cgame/jkg_wpindicators.cpp"
	"${MPDir}/cgame/animtable.h"
	"${MPDir}/cgame/cg_local.h"
	"${MPDir}/cgame/cg_public.h"
	"${MPDir}/cgame/cg_weapons.h"
	"${MPDir}/cgame/cg_xcvar.h"
	"${MPDir}/cgame/fx_local.h"
	"${MPDir}/cgame/jkg_chatcmds.h"
	"${MPDir}/cgame/jkg_hud.h"
	"${MPDir}/cgame/jkg_navmesh_visualiser.h"
	"${CMAKE_SOURCE_DIR}/JKGalaxies/ui/menudef.h" #Fixme: Don't like the way JKGalaxies folder is included. 
	)
source_group("cgame" FILES ${MPCGameCgameFiles})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameCgameFiles})

set(MPCGameCommonFiles
	"${MPDir}/qcommon/cppStringHelp.cpp"
	"${MPDir}/qcommon/cppStringHelp.h"
	"${MPDir}/qcommon/q_math.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	"${MPDir}/qcommon/disablewarnings.h"
	"${MPDir}/qcommon/q_shared.h"
	"${MPDir}/qcommon/q_platform.h"
	"${MPDir}/qcommon/qfiles.h"
	"${MPDir}/qcommon/tags.h"
	)
source_group("common" FILES ${MPCGameCommonFiles})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameCommonFiles})

set(MPCGameUiFiles
	"${MPDir}/ui/ui_shared.cpp"
	"${MPDir}/ui/keycodes.h"
	"${MPDir}/ui/menudef.h"
	"${MPDir}/ui/ui_shared.h"
	)
source_group("ui" FILES ${MPCGameUiFiles})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameUiFiles})

set(MPCGameRendererFiles
	"${MPDir}/rd-common/tr_types.h"
	)
source_group("rd-common" FILES ${MPCGameRendererFiles})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameRendererFiles})

set(MPCGameGFiles
	"${MPDir}/ghoul2/G2.h"
	)
source_group("ghoul2" FILES ${MPCGameG2Files})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameG2Files})

set(MPCGameEncodingLibrary
	"${MPDir}/libraries/encoding/base128.cpp"
	"${MPDir}/libraries/encoding/base128.h"
	"${MPDir}/libraries/encoding/bitstream.c"
	"${MPDir}/libraries/encoding/bitstream.h"
	)
source_group("Libraries\\Encoding" FILES ${MPCGameEncodingLibrary})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameEncodingLibrary})

set(MPCGameJSONLibrary
	"${MPDir}/libraries/json/cJSON.cpp"
	"${MPDir}/libraries/json/cJSON.h"
	)
source_group("Libraries\\JSON" FILES ${MPCGameJSONLibrary})
set(MPCGameFiles ${MPCGameFiles} ${MPCGameJSONLibrary})
	
add_library(${MPCGame} SHARED ${MPCGameFiles})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPCGame} PROPERTIES PREFIX "")
endif()

if(MakeApplicationBundles AND BuildMPEngine)
	install(TARGETS ${MPCGame}
		LIBRARY
		DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/JKG"
		COMPONENT ${JKAMPCoreComponent})
elseif(WIN32)
	install(TARGETS ${MPCGame}
		RUNTIME
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
else()
	install(TARGETS ${MPCGame}
		LIBRARY
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
endif()

set_target_properties(${MPCGame} PROPERTIES COMPILE_DEFINITIONS "${MPCGameDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPCGame} APPEND PROPERTY COMPILE_OPTIONS ${JKG_VISIBILITY_FLAGS})

set_target_properties(${MPCGame} PROPERTIES INCLUDE_DIRECTORIES "${MPCGameIncludeDirectories}")
set_target_properties(${MPCGame} PROPERTIES PROJECT_LABEL "Client Side Module (cgame)")
# no libraries used
if(MPCGameLibraries)
	target_link_libraries(${MPCGame} ${MPCGameLibraries})
endif(MPCGameLibraries)
