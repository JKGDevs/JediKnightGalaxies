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

# For checks in subdirectories
set(InJKG TRUE)

option(BuildDestinationExternal "Copy final build files to directory specified by 'Binary Destination'" OFF)
set(BinaryDestination ${CMAKE_CURRENT_BINARY_DIR} CACHE STRING "Binary Destination")

# Project name
set(ProjectName "Jedi Knight Galaxies" CACHE STRING "Project Name")
project(${ProjectName})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Customizable options
option(BuildPortableVersion "Build portable version (does not read or write files from your user/home directory" OFF)
option(BuildSkipCryptography "Remove unused Cryptography." ON)

option(BuildMPEngine "Whether to create projects for the MP client (jkgalaxies.exe)" ON)
option(BuildMPDed "Whether to create projects for the MP dedicated server (jkgalaxiesded.exe)" ON)
option(BuildMPGame "Whether to create projects for the MP server-side gamecode (jampgamex86.dll)" ON)
option(BuildMPCGame "Whether to create projects for the MP clientside gamecode (cgamex86.dll)" ON)
option(BuildMPUI "Whether to create projects for the MP UI code (uix86.dll)" ON)

# Configure the use of bundled libraries.  By default, we assume the user is on
# a platform that does not require any bundling.
#
# Note that we always use the bundled copy of minizip, since it is modified to
# use Z_Malloc.

set(UseInternalOpenALDefault OFF)
set(UseInternalZlibDefault   OFF)
set(UseInternalPNGDefault    OFF)
set(UseInternalJPEGDefault   OFF)
set(UseInternalSDL2Default   OFF)

if(WIN32)
  set(UseInternalOpenALDefault ON)
  set(UseInternalZlibDefault   ON)
  set(UseInternalPNGDefault    ON)
  set(UseInternalJPEGDefault   ON)
  set(UseInternalSDL2Default   ON)
endif()

if(APPLE)
  set(UseInternalJPEGDefault ON)
endif()

option(UseInternalOpenAL "If set, use bundled OpenAL."  ${UseInternalOpenALDefault})
option(UseInternalZlib   "If set, use bundled zlib."    ${UseInternalZlibDefault})
option(UseInternalPNG    "If set, use bundled libpng."  ${UseInternalPNGDefault})
option(UseInternalJPEG   "If set, use bundled libjpeg." ${UseInternalJPEGDefault})
option(UseInternalSDL2   "If set, use bundled SDL2."    ${UseInternalSDL2Default})

# This option won't appear on non-Apple platforms.
if(APPLE)
  option(MakeApplicationBundles "Whether to build .app application bundles for engines built" ON)
endif()

# Custom CMake Modules needed
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/CMakeModules")

Include(CheckTypeSize)
check_type_size("void*" CMAKE_SIZEOF_VOID_P)



# ${Architecture} must match ARCH_STRING in q_platform.h,
# and is used in DLL names (jagamex86.dll, jagamex86.dylib, jagamei386.so).
if(WIN32)
	set(X86 ON)
	if(CMAKE_SIZEOF_VOID_P MATCHES "8")
		set(Architecture "x86_64")
		set(WIN64 TRUE)
	else()
		set(Architecture "x86")
		set(WIN64 FALSE)
	endif()
else()
	set(X86 OFF)
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
		set(Architecture "arm")
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
		set(X86 ON)
		if(APPLE)
			set(Architecture "x86")
		else()
			# e.g. Linux
			set(Architecture "i386")
		endif()
	elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^x86.64$")
		set(X86 ON)
		set(Architecture "x86_64")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc")
		set(Architecture "ppc")
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "powerpc64")
		set(Architecture "ppc64")
	else()
		set(Architecture "${CMAKE_SYSTEM_PROCESSOR}")
	endif()
endif()

message("Architecture is ${Architecture}")

# Current Git SHA1 hash
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
#this information is shown later on, see codemp\CMakeLists.txt
#message("Git revision is ${GIT_SHA1}")
#message("Git refspec is ${GIT_REFSPEC}")

# Binary names
set(MPEngine "jkgalaxies.${Architecture}")
set(MPVanillaRenderer "rd-galaxies_${Architecture}")
set(MPDed "jkgalaxiesded.${Architecture}")
set(MPGame "game${Architecture}")
set(MPCGame "cgame${Architecture}")
set(MPUI "ui${Architecture}")
# Library names
set(MPBotLib "botlib")
set(SharedLib "shared")

# Paths
set(MPDir "${CMAKE_SOURCE_DIR}/codemp")
set(JKGLibDir "${MPDir}/libraries")
set(SharedDir ${CMAKE_SOURCE_DIR}/shared)

include(InstallConfig)

# Operating settings
if(WIN64)
	set(SharedDefines ${SharedDefines} "WIN64")
endif()

if (APPLE)
	set(SharedDefines "MACOS_X")
endif()

if (NOT WIN32 AND NOT APPLE)
	set(SharedDefines "ARCH_STRING=\"${Architecture}\"")
endif()

# Compiler settings
if(MSVC)

	set(SharedDefines ${SharedDefines} "NOMINMAX")
	set(SharedDefines ${SharedDefines} "_CRT_SECURE_NO_WARNINGS")
	set(SharedDefines ${SharedDefines} "_SCL_SECURE_NO_WARNINGS")
	set(SharedDefines ${SharedDefines} "_CRT_NONSTDC_NO_DEPRECATE")

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	# We don't try to control symbol visibility under MSVC.
	set(JKG_VISIBILITY_FLAGS "")

	# additional flags for debug configuration
	#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
	#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

elseif (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang"))
	# I hope this doesn't come back to bite me in the butt later on.
	# Realistically though, can the C and CXX compilers be different?

	# Visibility can't be set project-wide -- it needs to be specified on a
	# per-target basis.  This is primarily due to the bundled copy of ZLib.
	# ZLib explicitly declares symbols hidden, rather than defaulting to hidden.
	#
	# Note that -fvisibility=hidden is stronger than -fvisibility-inlines-hidden.
	set(JKG_VISIBILITY_FLAGS "-fvisibility=hidden")

	# removes the -rdynamic flag at linking (which causes crashes for some reason)
	set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

	# additional flags for debug configuration
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")

	if (X86)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	endif()

	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

	# enable somewhat modern C++
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	
	if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-comment")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char")
		if (X86)
			# "x86 vm will crash without -mstackrealign since MMX
			# instructions will be used no matter what and they
			# corrupt the frame pointer in VM calls"
			# -ioquake3 Makefile
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mstackrealign")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpmath=sse")
		endif()

		if(WIN32)
			# Link libgcc and libstdc++ statically
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
		endif()
	elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-comment")
	endif()

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")
		if (X86)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mstackrealign")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse")
		endif()
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings")
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-writable-strings")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
	endif()
else()
	message(ERROR "Unsupported compiler")
endif()

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message("Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#if (NOT CMAKE_BUILD_TYPE)
#	message("No build type selected, default to RELEASE")
#	set(CMAKE_BUILD_TYPE "RELEASE")
#endif()

if(CMAKE_BUILD_TYPE MATCHES "DEBUG" OR CMAKE_BUILD_TYPE MATCHES "Debug")
	# CMake already defines _DEBUG for MSVC.
	if (NOT MSVC)
		set(SharedDefines ${SharedDefines} "_DEBUG")
	endif()
else()
	set(SharedDefines ${SharedDefines} "FINAL_BUILD")
endif()



# Settings
if(BuildPortableVersion)
	set(SharedDefines ${SharedDefines} "_PORTABLE_VERSION")
endif()

if(BuildSkipCryptography)
	set(SharedDefines ${SharedDefines} "NO_CRYPTOGRAPHY")
endif()

set(JKGDir "${CMAKE_SOURCE_DIR}/JKGalaxies")
if(UseInternalJPEG)
  add_subdirectory(${JKGLibDir}/jpeg-9a)
else()
  find_package(JPEG REQUIRED)
endif()

if(UseInternalZlib)
  add_subdirectory(${JKGLibDir}/zlib)
else()
  find_package(ZLIB REQUIRED)
endif()

if(UseInternalPNG)
  add_subdirectory(${JKGLibDir}/libpng)
else()
  find_package(PNG REQUIRED)
endif()

# Always use bundled minizip (sets MINIZIP_{LIBRARIES,INCLUDE_DIR})
add_subdirectory(${JKGLibDir}/minizip)


#    Add projects
add_subdirectory(${MPDir})
