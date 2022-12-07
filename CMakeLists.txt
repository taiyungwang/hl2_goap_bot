cmake_minimum_required (VERSION 2.8.12)

project (PluginBot)

#OS specific settings
add_definitions(-DRAD_TELEMETRY_DISABLED)

set(DRIVE c CACHE STRING "User specified drive for Windows")
set (CXX_DIR "" CACHE STRING "CXX Path")
set( steam_root "${DRIVE}:/Program Files (x86)/steam" )
set(HL2SDK_DIR "${PROJECT_SOURCE_DIR}/../hl2sdk")
set (HL2SDK_LIBS ${HL2SDK_DIR}/lib/public)
set (VSP_NAVMESH ${PROJECT_SOURCE_DIR}/../valve_source_plugin_navmesh)
set (MMS_DIR ${PROJECT_SOURCE_DIR}/../metamod-source/core)

set (LEGACY_STDIO legacy_stdio_definitions)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if (${UNIX})
	set (SRC_HOME $ENV{HOME})
	set (HL2SDK_LIBS ${HL2SDK_LIBS}/linux32)
	add_definitions(-D_LINUX -DLINUX -D_GLIBCXX_USE_CXX11_ABI=0) 
	set( steam_root "${SRC_HOME}/.local/share/Steam" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	set( UNDEF "-Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE" )
	add_link_options(-static-libgcc -static-libstdc++)
	set (TIER1_SRC tier1.cpp convar.cpp KeyValues.cpp utlstring.cpp strtools.cpp memstack.cpp bitbuf.cpp
	 strtools_unicode.cpp qsort_s.cpp utlbuffer.cpp checksum_crc.cpp characterset.cpp generichash.cpp
	 interface.cpp)
	list(TRANSFORM TIER1_SRC PREPEND ${HL2SDK_DIR}/tier1/)
	add_library(tier1 ${TIER1_SRC})
	file(GLOB MATHLIB_SRC ${HL2SDK_DIR}/mathlib/*.cpp)
	add_library(mathlib ${MATHLIB_SRC})
else ()
	set (CXX_DIR "${DRIVE}:/cxxtest-4.4")
	set (CXX_TEST_GEN "${CXX_DIR}/bin/cxxtestgen")		
endif (${UNIX})

#compiler specific settings
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set (LEGACY_STDIO "")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_definitions(-DCOMPILER_GCC -DGNUC)
	else()
		add_definitions(-D__clang__ )
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-dangling-else -Wno-c++11-narrowing -Wno-inconsistent-missing-override")
	endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set( ARCH_FLAGS "-mtune=i686 -march=pentium3 -mmmx -m32 -fpermissive" )
	add_definitions(-D_POSIX -DPOSIX -DGNUC -DNO_HOOK_MALLOC -DNO_MALLOC_OVERRIDE)
	add_definitions(-DSWDS -D_finite=finite -Dstricmp=strcasecmp  -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -D_alloca=alloca)	
else () # MSVC
	add_compile_options(/MTd /MP)
	add_link_options(/NODEFAULTLIB:libcmt)
endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}" )
SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${ARCH_FLAGS}" )

include_directories(${PROJECT_SOURCE_DIR} ${VSP_NAVMESH} ${HL2SDK_DIR}/common utils/common ${HL2SDK_DIR}/public ${HL2SDK_DIR}/public/mathlib ${HL2SDK_DIR}/public/tier0 ${HL2SDK_DIR}/public/tier1 ${HL2SDK_DIR}/public/game/server ${HL2SDK_DIR}/public/engine ${HL2SDK_DIR}/game/shared ${HL2SDK_DIR}/game/server ${CMAKE_CURRENT_SOURCE_DIR}/bot)

link_directories(${HL2SDK_LIBS})
add_library(public STATIC ${HL2SDK_DIR}/public/collisionutils.cpp)
add_subdirectory(${VSP_NAVMESH}/util valve_source_plugin_navmesh/util)
add_subdirectory(${VSP_NAVMESH}/nav_mesh valve_source_plugin_navmesh/nav_mesh)
add_subdirectory(${PROJECT_SOURCE_DIR}/bot)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set (CXX_TEST_GEN ${CXX_DIR}/bin/cxxtestgen)
enable_testing()
include(CTest)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(PLUGIN_NAME my_bot)
add_library(${PLUGIN_NAME} SHARED  VSPlugin.cpp)
target_link_libraries(${PLUGIN_NAME}  
	 bot_main dod_player dod_weapon dod_goap_actions hl2dm_player hl2dm_weapon hl2dm_goap_actions player 
	 voice goap goap_actions weapon_sys bot_move valve_navmesh plugin_util bot_util
	 tier1 mathlib vstdlib public tier0 ${LEGACY_STDIO})

add_library(${PLUGIN_NAME}_mm SHARED MMSPlugin.cpp)
target_include_directories(${PLUGIN_NAME}_mm PRIVATE ${MMS_DIR} ${MMS_DIR}/sourcehook)
target_compile_definitions(${PLUGIN_NAME}_mm PRIVATE SOURCE_ENGINE=4 
	SE_EPISODEONE=1 SE_DARKMESSIAH=2 SE_ORANGEBOX=3 SE_ORANGEBOXVALVE=4 SE_LEFT4DEAD=5 SE_LEFT4DEAD2=6 
	SE_ALIENSWARM)
target_link_libraries(${PLUGIN_NAME}_mm
	 bot_main dod_player dod_weapon dod_goap_actions hl2dm_player hl2dm_weapon hl2dm_goap_actions player 
	 voice goap goap_actions weapon_sys bot_move valve_navmesh plugin_util bot_util
	 tier1 mathlib vstdlib public tier0 ${LEGACY_STDIO})
#find installed mods on system.
set (mods "tf;dod;hl2mp")
set( steam_dir steamapps/common )
FILE(GLOB games ${steam_root}/${steam_dir}/*)
foreach(GAME ${games})
	foreach(mod ${mods})
		if (EXISTS ${GAME}/${mod})
			set( src_dir "${GAME}" )
			set( install_dir ${GAME}/${mod}/addons/mybot )
	        install (TARGETS ${PLUGIN_NAME} DESTINATION ${install_dir})
	        install (TARGETS ${PLUGIN_NAME}_mm DESTINATION ${install_dir})
		endif (EXISTS ${GAME}/${mod})
	endforeach(mod ${mods})
endforeach(GAME ${games})
if (NOT DEFINED src_dir)
    message (WARNING "No supported mods found.")
endif (NOT DEFINED src_dir)

