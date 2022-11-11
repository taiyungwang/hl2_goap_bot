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

set (BUILD_DIR "")

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
	add_link_options(-static-libgcc  -static-libstdc++)	
	file(GLOB TIER1_SRC ${HL2SDK_DIR}/tier1/*.cpp)
	add_library(tier1 ${TIER1_SRC})
	file(GLOB MATHLIB_SRC ${HL2SDK_DIR}/mathlib/*.cpp)
	add_library(mathlib ${MATHLIB_SRC})
else ()
	set (CXX_DIR "${DRIVE}:/cxxtest-4.4")
	set (CXX_TEST_GEN "${CXX_DIR}/bin/cxxtestgen")		
	add_definitions(-DWIN32 -D_WIN32 -DWINDOWS)
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
	if (DEFINED CMAKE_BUILD_TYPE)
		set (BUILD_DIR ${CMAKE_BUILD_TYPE})
	else()
		set (BUILD_DIR "Debug")
	endif ()
	add_compile_options(
		$<$<CONFIG:>:/MT> #---------|
		$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
		$<$<CONFIG:Release>:/MT> #--|
		-arch:SSE -GS- -TP -W4 -fp:fast -GF -Zc:forScope -RTC1 -GR -Gd -Oy- -Fa -Fo -Fp -MP -Od -TP
	)
	string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	add_link_options($<$<CONFIG:Debug>:/NODEFAULTLIB:libcmt> 
		$<$<CONFIG:Release>:/NODEFAULTLIB:libcmtd> 
		/SAFESEH:NO)
	add_definitions(-DCOMPILER_MSVC -DCOMPILER_MSVC32 -D_CRT_NO_VA_START_VALIDATION -D_CRT_SECURE_NO_DEPRECATE 
		-D_CRT_NONSTDC_NO_DEPRECATE)
endif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}" )
SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${ARCH_FLAGS}" )

include_directories(${PROJECT_SOURCE_DIR} ${VSP_NAVMESH} ${HL2SDK_DIR}/common utils/common ${HL2SDK_DIR}/public ${HL2SDK_DIR}/public/mathlib ${HL2SDK_DIR}/public/tier0 ${HL2SDK_DIR}/public/tier1 ${HL2SDK_DIR}/public/game/server ${HL2SDK_DIR}/public/engine ${HL2SDK_DIR}/game/shared ${HL2SDK_DIR}/game/server)
link_directories(${HL2SDK_LIBS})	

add_library(public STATIC ${HL2SDK_DIR}/public/collisionutils.cpp)

find_library(VSP_NAVMESH_LIB valve_navmesh ${PROJECT_BINARY_DIR}/../valve_source_plugin_navmesh/nav_mesh/${BUILD_DIR})
find_library(VSP_UTIL_LIB plugin_util ${PROJECT_BINARY_DIR}/../valve_source_plugin_navmesh/util/${BUILD_DIR})

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
	 voice goap goap_actions weapon_sys event bot_move ${VSP_NAVMESH_LIB} ${VSP_UTIL_LIB}
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
		endif (EXISTS ${GAME}/${mod})
	endforeach(mod ${mods})
endforeach(GAME ${games})
if (NOT DEFINED src_dir)
    message (WARNING "No supported mods found.")
endif (NOT DEFINED src_dir)

