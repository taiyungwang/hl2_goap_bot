# A Half-Life 2 Bot Driven by GOAP #

GOAP stands for Goal Oriented Action Planning--please visit [Jeff Orkin's page](http://alumni.media.mit.edu/~jorkin/goap.html) for more information on the GOAP architecture.  
The bot uses a [refactored version](https://github.com/taiyungwang/valve_source_plugin_navmesh) of [Valve's Navigation Mesh](https://developer.valvesoftware.com/wiki/Navigation_Meshes).  A bot traverses the mesh using an algorithm similar to [Valve's reactive path following technique](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf).  
The navigation mesh generated provided by Valve generates hiding spots based on various geometric features of the map.  However the generated spots are not always placed in the most ideal positions.  For example, some spots are generated in the middle of a spawn area where an opponent will never visit.  One way to address this problem is for a map maker to manually remove these invalid spots, however that can be very labor intensive.  This bot addresses this problem by treating the hiding spots as a [Multi-arm Banit problem](https://en.wikipedia.org/wiki/Multi-armed_bandit).  It solves the problem using [Thompson Sampling](https://en.wikipedia.org/wiki/Thompson_sampling).
The bot currently supports Valve's [Day of Defeat Source](https://en.wikipedia.org/wiki/Day_of_Defeat:_Source) and [Half-Life 2 Deathmatch](https://en.wikipedia.org/wiki/Half-Life_2:_Deathmatch).  This bot is still in the Alpha stage--please see the TODO section for future work needed.

## Compiling the Bot ##
The bot is built on Windows, LINUX, and Mac OS X with [CMake](https://cmake.org/) and [Valve's Source SDK](alliedmodders/hl2sdk). The toplevel CMakeList.txt file needs to be edited to point to the appropriate location of the plugin navigation mesh, the Valve Source SDK, and CxxTest. It assumed that [Steam](https://store.steampowered.com/about/) is installed within the HOMEPATH for the OS X and LINUX build environment.  When building against Windows, the location of Steam in the CMakeList.txt may need to be updated.  Building also requires [CxxTest](https://cxxtest.com/). Please see the [Valve Source SDK Wiki on compiling](https://developer.valvesoftware.com/wiki/Source_SDK_2013) for all other platform specific dependencies.  

## TODO ##
* Make bots less accurate.  Currently the bots are nearly 100% accurate with their aim.  
* Add a framework in HL2DM for bots to determine which items to pick up.
* Add logic for DOD bots to pick up a substitue weapon when current out of ammo for primary weapon.
* Refactor aiming behavior defined in the GOAP Action classes into an aiming subsystem.
* Save the Thompson Sampling results after each game.
