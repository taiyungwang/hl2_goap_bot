cmake_minimum_required (VERSION 2.8.7)

project (PluginBot)

#OS specific settings
add_definitions(-DRAD_TELEMETRY_DISABLED)

set( steam_root "c:/Program Files (x86)/steam" )
set(HL2SDK_DIR "${PROJECT_SOURCE_DIR}/../hl2sdk")
set (HL2SDK_LIBS ${HL2SDK_DIR}/lib/public)
set (VSP_NAVMESH ${PROJECT_SOURCE_DIR}/../plugin_navmesh)
find_library(VSP_NAVMESH_LIB valve_navmesh ${PROJECT_BINARY_DIR}/plugin_navmesh/nav_mesh/)
find_library(VSP_UTIL_LIB plugin_util ~/workspace/plugin_navmesh/util)
set (CXX_TEST_GEN cxxtestgen)

set(tier2 tier2)
file(GLOB TIER1_SRC ${HL2SDK_DIR}/tier1/*.cpp)
add_library(tier1 ${TIER1_SRC})
file(GLOB MATHLIB_SRC ${HL2SDK_DIR}/mathlib/*.cpp)
add_library(mathlib ${MATHLIB_SRC})

if (${UNIX})
	set (SRC_HOME $ENV{HOME})
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")
	set(tier2 :tier2.a)
	if( ${APPLE} )
		set (HL2SDK_LIBS ${HL2SDK_LIBS}/osx32)
		add_definitions(-D_OSX -DOSX)
		set( steam_root "${SRC_HOME}/Library/Application Support" )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
	else ()
		set (CXX_DIR "${SRC_HOME}/cxxtest/")
		set (CXX_TEST_GEN "${CXX_DIR}/bin/cxxtestgen")	
		set (HL2SDK_LIBS ${HL2SDK_LIBS}/linux32)
		add_definitions(-D_LINUX -DLINUX) 
		set( steam_root "${SRC_HOME}/.local/share/Steam" )
	endif( ${APPLE} )
	link_directories(${steam_root}/ubuntu12_32/steam-runtime/i386/usr/lib/i386-linux-gnu ${steam_root}/ubuntu12_32)
	set( UNDEF "-Usprintf -Ustrncpy -UPROTECTED_THINGS_ENABLE" )
else ()
	if(MSVC)
		add_compile_options(
			$<$<CONFIG:>:/MT> #---------|
			$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
			$<$<CONFIG:Release>:/MT> #--|
			-arch:SSE -GS- -TP -W4 -fp:fast -GF -Zc:forScope -RTC1 -GR -Gd -Oy- -Fa -Fo -Fp -MP -Od -TP
		)
		string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
		add_link_options(/NODEFAULTLIB:libc /NODEFAULTLIB:libcd /NODEFAULTLIB:libcmtd /SUBSYSTEM:WINDOWS /SAFESEH:NO)
		add_definitions(-DCOMPILER_MSVC -DCOMPILER_MSVC32 -D_CRT_NO_VA_START_VALIDATION -D_CRT_SECURE_NO_DEPRECATE 
			-D_CRT_NONSTDC_NO_DEPRECATE)
	endif()
	set (CXX_DIR "c:/cxxtest")
	set (CXX_TEST_GEN "${CXX_DIR}/bin/cxxtestgen")		
	set (LEGACY_STDIO legacy_stdio_definitions)
	add_definitions(-DWIN32 -D_WIN32 -DWINDOWS)
endif (${UNIX})

#compiler specific settings
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_definitions(-DCOMPILER_GCC -DGNUC)
	else()
		add_definitions(-D__clang__ )
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-dangling-else -Wno-c++11-narrowing -Wno-inconsistent-missing-override")
	endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set( ARCH_FLAGS "-mtune=i686 -march=pentium3 -mmmx -m32 -fpermissive" )
	add_definitions(-D_POSIX -DPOSIX -DGNUC -DNO_HOOK_MALLOC -DNO_MALLOC_OVERRIDE)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-non-virtual-dtor -Wno-invalid-offsetof" )
	add_definitions(-DSWDS -D_finite=finite -Dstricmp=strcasecmp  -Dstrnicmp=strncasecmp -D_vsnprintf=vsnprintf -D_alloca=alloca)	
endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}" )
SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${ARCH_FLAGS}" )

include_directories(${PROJECT_SOURCE_DIR} ${VSP_NAVMESH} ${HL2SDK_DIR}/common utils/common ${HL2SDK_DIR}/public ${HL2SDK_DIR}/public/mathlib ${HL2SDK_DIR}/public/tier0 ${HL2SDK_DIR}/public/tier1 ${HL2SDK_DIR}/public/game/server ${HL2SDK_DIR}/public/engine ${HL2SDK_DIR}/game/shared ${HL2SDK_DIR}/game/server)

#find installed mods on system.
set (mods "tf;dod;hl2mp")
set( steam_dir steamapps/common )
FILE(GLOB games ${steam_root}/${steam_dir}/*)
foreach(GAME ${games})
	foreach(mod ${mods})
		if (EXISTS ${GAME}/${mod})
			set( src_dir "${GAME}" )
			list(APPEND mod_dirs "${GAME}/${mod}")
		endif (EXISTS ${GAME}/${mod})
	endforeach(mod ${mods})
endforeach(GAME ${games})
link_directories("${src_dir}/bin" ${HL2SDK_LIBS})	
if (NOT DEFINED src_dir)
    message ("No supported mods found.")
    return()
endif (NOT DEFINED src_dir)

set(PLUGIN_NAME my_bot)

add_library(public STATIC ${HL2SDK_DIR}/public/collisionutils.cpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/bot)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(${PLUGIN_NAME} SHARED  VSPlugin.cpp)
target_link_libraries(${PLUGIN_NAME}  
	 bot_main dod_player dod_weapon dod_goap_actions hl2dm_player hl2dm_weapon hl2dm_goap_actions player goap goap_actions
	 weapon_sys event bot_move ${VSP_NAVMESH_LIB} ${VSP_UTIL_LIB}
	 tier1 mathlib vstdlib public tier0 ${tier2} ${LEGACY_STDIO})

foreach(mod_dir ${mod_dirs})
	if (EXISTS ${mod_dir}/addons)
	    set( install_dir ${mod_dir}/addons )
        install (TARGETS ${PLUGIN_NAME} DESTINATION ${install_dir})
	endif (EXISTS ${mod_dir}/addons)
endforeach(mod_dir ${mod_dirs})
