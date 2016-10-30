#============================================================================
# Copyright (C) 2013 - 2015, OpenJK contributors
# 
# This file is part of the OpenJK source code.
# 
# OpenJK is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#============================================================================

cmake_minimum_required(VERSION 2.8.8)

# Make sure the user is not executing this script directly
if(NOT InOpenJK)
	message(FATAL_ERROR "Use the top-level cmake script!")
endif(NOT InOpenJK)

set(MPUIIncludeDirectories "${MPDir}" "${OpenJKLibDir}" "${OpenJKLibDir}/libexpat/include")
if(WIN32)
	set(MPUILibraries "odbc32" "odbccp32") # what are these even?
endif(WIN32)
set(MPUIDefines ${MPSharedDefines} "UI_EXPORTS" "MISSIONPACK" "_USRDLL" "IN_UI" "XML_STATIC")

set(MPUIGameFiles
	"${MPDir}/game/animtable.cpp"
	"${MPDir}/game/bg_ammo.cpp"
	"${MPDir}/game/bg_constants.cpp"
	"${MPDir}/game/bg_saberData.cpp"
	"${MPDir}/game/bg_saberLoad.cpp"
	"${MPDir}/game/jkg_gangwars.cpp"
	"${MPDir}/game/bg_misc.cpp"
	"${MPDir}/game/bg_vehicleLoad.cpp"
	"${MPDir}/game/bg_weapons.cpp"
	"${MPDir}/game/bg_weapons_load.cpp"
	"${MPDir}/game/anims.h"
	"${MPDir}/game/bg_ammo.h"
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
	"${MPDir}/ui/jkg_ui_auxlib.h"
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

set(MPUILibExpat
	"${OpenJKLibDir}/libexpat/include/amigaconfig.h"
	"${OpenJKLibDir}/libexpat/include/ascii.h"
	"${OpenJKLibDir}/libexpat/include/asciitab.h"
	"${OpenJKLibDir}/libexpat/include/expat.h"
	"${OpenJKLibDir}/libexpat/include/expat_external.h"
	"${OpenJKLibDir}/libexpat/include/iasciitab.h"
	"${OpenJKLibDir}/libexpat/include/internal.h"
	"${OpenJKLibDir}/libexpat/include/latin1tab.h"
	"${OpenJKLibDir}/libexpat/include/macconfig.h"
	"${OpenJKLibDir}/libexpat/include/nametab.h"
	"${OpenJKLibDir}/libexpat/include/utf8tab.h"
	"${OpenJKLibDir}/libexpat/include/watcomconfig.h"
	"${OpenJKLibDir}/libexpat/include/winconfig.h"
	"${OpenJKLibDir}/libexpat/include/xmlrole.h"
	"${OpenJKLibDir}/libexpat/include/xmltok.h"
	"${OpenJKLibDir}/libexpat/include/xmltok_impl.h"
	)
source_group("Libraries\\Expat" FILES ${MPUILibExpat})
set(MPUIFiles ${MPUIFiles} ${MPUILibExpat})

add_library(${MPUI} SHARED ${MPUIFiles})
set(LibExpat
	#"${OpenJKLibDir}/libexpat/lib/libexpat.lib"
	"${OpenJKLibDir}/libexpat/lib/libexpatMT.lib"
	#"${OpenJKLibDir}/libexpat/lib/libexpatw.lib"
	#"${OpenJKLibDir}/libexpat/lib/libexpatwMT.lib"
	)
set(MPUILibraries ${MPUILibraries} ${LibExpat})

if(NOT MSVC)
	# remove "lib" prefix for .so/.dylib files
	set_target_properties(${MPUI} PROPERTIES PREFIX "")
endif()

if(MakeApplicationBundles AND BuildMPEngine)
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/JKG"
		COMPONENT ${JKAMPCoreComponent})
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/${MPEngine}.app/Contents/MacOS/base"
		COMPONENT ${JKAMPCoreComponent})
elseif(WIN32)
	install(TARGETS ${MPUI}
		RUNTIME
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
	if (WIN64)
		# Don't do this on 32-bit Windows to avoid overwriting
		# vanilla JKA's DLLs
		install(TARGETS ${MPUI}
			RUNTIME
			DESTINATION "${JKAInstallDir}/base"
			COMPONENT ${JKAMPCoreComponent})
	endif()
else()
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/JKG"
		COMPONENT ${JKAMPCoreComponent})
	install(TARGETS ${MPUI}
		LIBRARY
		DESTINATION "${JKAInstallDir}/base"
		COMPONENT ${JKAMPCoreComponent})
endif()

set_target_properties(${MPUI} PROPERTIES COMPILE_DEFINITIONS "${MPUIDefines}")

# Hide symbols not explicitly marked public.
set_property(TARGET ${MPUI} APPEND PROPERTY COMPILE_OPTIONS ${OPENJK_VISIBILITY_FLAGS})

set_target_properties(${MPUI} PROPERTIES INCLUDE_DIRECTORIES "${MPUIIncludeDirectories}")
set_target_properties(${MPUI} PROPERTIES PROJECT_LABEL "UI Library (uix86.dll)")
# no libraries used
if(MPUILibraries)
	target_link_libraries(${MPUI} ${MPUILibraries})
endif(MPUILibraries)
