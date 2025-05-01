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

set(MPUIIncludeDirectories "${MPDir}" "${JKGLibDir}")
if(WIN32)
	set(MPUILibraries "odbc32" "odbccp32") # what are these even?
endif(WIN32)
set(MPUIDefines ${MPSharedDefines} "UI_EXPORTS" "MISSIONPACK" "_USRDLL" "IN_UI")

set(MPUIGameFiles
	"${MPDir}/game/animtable.cpp"
	"${MPDir}/game/bg_ammo.cpp"
	"${MPDir}/game/bg_constants.cpp"
	"${MPDir}/game/bg_damage.cpp"
	"${MPDir}/game/bg_saberData.cpp"
	"${MPDir}/game/bg_saberLoad.cpp"
	"${MPDir}/game/jkg_gangwars.cpp"
	"${MPDir}/game/bg_misc.cpp"
	"${MPDir}/game/bg_weapons.cpp"
	"${MPDir}/game/bg_weapons_load.cpp"
	"${MPDir}/game/anims.h"
	"${MPDir}/game/bg_ammo.h"
	"${MPDir}/game/bg_damage.h"
	"${MPDir}/game/bg_local.h"
	"${MPDir}/game/bg_public.h"
	"${MPDir}/game/bg_saga.h"
	"${MPDir}/game/bg_weapons.h"
	"${MPDir}/game/jkg_gangwars.h"
	"${MPDir}/game/surfaceflags.h"
	"${MPDir}/game/z_global_defines.h"
	)
source_group("game" FILES ${MPUIGameFiles})
set(MPUIFiles ${MPUIFiles} ${MPUIGameFiles})

set(MPUICommonFiles
	"${MPDir}/qcommon/cppStringHelp.cpp"
	"${MPDir}/qcommon/cppStringHelp.h"
	"${MPDir}/qcommon/q_math.cpp"
	"${MPDir}/qcommon/q_shared.cpp"
	"${MPDir}/qcommon/disablewarnings.h"
	"${MPDir}/qcommon/game_version.h"
	"${MPDir}/qcommon/q_shared.h"
	"${MPDir}/qcommon/q_platform.h"
	"${MPDir}/qcommon/qfiles.h"
	"${MPDir}/qcommon/tags.h"
	)
source_group("common" FILES ${MPUICommonFiles})
set(MPUIFiles ${MPUIFiles} ${MPUICommonFiles})

set(MPUIUIFiles
	"${MPDir}/ui/jkg_conversations.cpp"
	"${MPDir}/ui/jkg_gangwars_ui.cpp"
	"${MPDir}/ui/jkg_inventory.cpp"
	"${MPDir}/ui/jkg_loot.cpp"
	"${MPDir}/ui/jkg_partymanager.cpp"
	"${MPDir}/ui/jkg_pazaak.cpp"
	"${MPDir}/ui/jkg_shop.cpp"
	"${MPDir}/ui/jkg_slice.cpp"
	"${MPDir}/ui/ui_atoms.cpp"
	"${MPDir}/ui/ui_cvar.cpp"
	"${MPDir}/ui/ui_crossover.cpp"
	"${MPDir}/ui/ui_force.cpp"
	"${MPDir}/ui/ui_gameinfo.cpp"
	"${MPDir}/ui/ui_jkgscript.cpp"
	"${MPDir}/ui/ui_main.cpp"
	"${MPDir}/ui/ui_saber.cpp"
	"${MPDir}/ui/ui_servercmds.cpp"
	"${MPDir}/ui/ui_shared.cpp"
	"${MPDir}/ui/ui_stylesheets.cpp"
	"${MPDir}/ui/keycodes.h"
	"${CMAKE_SOURCE_DIR}/JKGalaxies/ui/menudef.h" #Fixme: Better location plz
	"${MPDir}/ui/ui_force.h"
	"${MPDir}/ui/jkg_conversations.h"
	"${MPDir}/ui/jkg_gangwars_ui.h"
	"${MPDir}/ui/jkg_inventory.h"
	"${MPDir}/ui/jkg_lootui.h"
	"${MPDir}/ui/jkg_partymanager.h"
	"${MPDir}/ui/jkg_pazaak.h"
	"${MPDir}/ui/jkg_slice.h"
	"${MPDir}/ui/ui_jkgscript.h"
	"${MPDir}/ui/ui_local.h"
	"${MPDir}/ui/ui_public.h"
	"${MPDir}/ui/ui_shared.h"
	"${MPDir}/ui/ui_xcvar.h"
	)
source_group("ui" FILES ${MPUIUIFiles})
set(MPUIFiles ${MPUIFiles} ${MPUIUIFiles})

set(MPUIRendererFiles
	"${MPDir}/rd-common/tr_types.h"
	)
source_group("rd-common" FILES ${MPUIRendererFiles})
set(MPUIFiles ${MPUIFiles} ${MPUIRendererFiles})

set(MPUILibBase128Header
	"${MPDir}/libraries/encoding/base128.h"
	"${MPDir}/libraries/encoding/base128.cpp"
	)
source_group("Libraries\\Base128" FILES ${MPUILibBase128Header})
set(MPUIFiles ${MPUIFiles} ${MPUILibBase128Header})

set(MPUILibBitstream
	"${MPDir}/libraries/encoding/bitstream.h"
	"${MPDir}/libraries/encoding/bitstream.c"
	)
source_group("Libraries\\BitStream" FILES ${MPUILibBitstream})
set(MPUIFiles ${MPUIFiles} ${MPUILibBitstream})

set(MPUILibJson
	"${MPDir}/libraries/json/cJSON.h"
	"${MPDir}/libraries/json/cJSON.cpp"
	)
source_group("Libraries\\Json" FILES ${MPUILibJson})
set(MPUIFiles ${MPUIFiles} ${MPUILibJson})

add_library(${MPUI} SHARED ${MPUIFiles})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPUI} PROPERTIES PREFIX "")
endif()

if(MakeApplicationBundles AND BuildMPEngine)
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/JKG"
		COMPONENT ${JKAMPCoreComponent})
elseif(WIN32)
	install(TARGETS ${MPUI}
		RUNTIME
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
else()
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
endif()

set_target_properties(${MPUI} PROPERTIES COMPILE_DEFINITIONS "${MPUIDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPUI} APPEND PROPERTY COMPILE_OPTIONS ${JKG_VISIBILITY_FLAGS})

set_target_properties(${MPUI} PROPERTIES INCLUDE_DIRECTORIES "${MPUIIncludeDirectories}")
set_target_properties(${MPUI} PROPERTIES PROJECT_LABEL "UI Library (uix86.dll)")
# no libraries used
if(MPUILibraries)
	target_link_libraries(${MPUI} ${MPUILibraries})
endif(MPUILibraries)
