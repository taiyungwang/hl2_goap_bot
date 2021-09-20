# A Half-Life 2 Bot Driven by GOAP #

GOAP stands for Goal Oriented Action Planning--please visit [Jeff Orkin's page](http://alumni.media.mit.edu/~jorkin/goap.html) for more information on the GOAP architecture.  
The bot uses a [refactored version](https://github.com/taiyungwang/valve_source_plugin_navmesh) of [Valve's Navigation Mesh](https://developer.valvesoftware.com/wiki/Navigation_Meshes).  A bot traverses the mesh using an algorithm similar to [Valve's reactive path following technique](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf).  
The navigation mesh generated provided by Valve generates hiding spots based on various geometric features of the map.  However the generated spots are not always placed in the most ideal positions.  For example, some spots are generated in the middle of a spawn area where an opponent will never visit.  One way to address this problem is for a map maker to manually remove these invalid spots, however that can be very labor intensive.  This bot addresses this problem by treating the hiding spots as a [Multi-arm Bandit problem](https://en.wikipedia.org/wiki/Multi-armed_bandit) and tries to solve the problem using [Thompson Sampling](https://en.wikipedia.org/wiki/Thompson_sampling).
The bot currently supports Valve's [Day of Defeat Source](https://en.wikipedia.org/wiki/Day_of_Defeat:_Source) and [Half-Life 2 Deathmatch](https://en.wikipedia.org/wiki/Half-Life_2:_Deathmatch).  This bot is still in the Alpha stage--please see the TODO section for future work needed.

## Compiling the Bot ##
The bot can be built on Windows and LINUX with [CMake](https://cmake.org/) and [Valve's Source SDK](alliedmodders/hl2sdk). The toplevel CMakeList.txt assumes that hl2sdk and valve_plugin_navmesh are in the same subdirectory as this repo.  It assumes that [CxxTest](https://cxxtest.com/) is installed via apt in LINUX, and in Windows it is installed in the root of drive that is specified (defaults to C:). It assumed that [Steam](https://store.steampowered.com/about/) is installed within the HOMEPATH for the LINUX build environment.  Please see the [Valve Source SDK Wiki on compiling](https://developer.valvesoftware.com/wiki/Source_SDK_2013) for all other platform specific dependencies.  

## Installing the Bot ##
Extract the tgz files into desired game directory, e.g. C:\program files (x86)\steam\steamapps\common\day of defeat source\dod

## Running the Bot ##
On Windows, you would need to launch the game with the -insecure option.  For Linux, execute the following from console:
```
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/lib/i386-linux-gnu:$HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/usr/lib/i386-linux-gnu:bin $HOME/.local/share/Steam/steamapps/common/<game dir>/hl2_linux -game <mod dir>
```
Where "game dir" is either "Day of Defeat Source" or "Half-Life 2 Deathmatch" and "mod dir" is either dod or hl2mp respectively.

## TODO ##
* Make the bots more player friendly.  Currently the bots are nearly 100% accurate with their aim, and have nearly no latency when reacting.  
* Add a ability for bots to "listen" for things like gun fire and footsteps.
* Add action for bots to drop ammo for allies
* Add a framework in HL2DM for bots to determine which items to pick up.
* Add logic for bots to pick up a substitue weapon when current out of ammo for primary weapon.
* Save the Thompson Sampling results after each game.
* Refactor aiming behavior defined in the GOAP Action classes into an aiming subsystem.
