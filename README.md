# A Half-Life 2 Bot Driven by GOAP #

GOAP stands for Goal Oriented Action Planning--please visit [Jeff Orkin's page](http://alumni.media.mit.edu/~jorkin/goap.html) for more information on the GOAP architecture.  
The bot uses a [refactored version](https://github.com/taiyungwang/valve_source_plugin_navmesh) of [Valve's Navigation Mesh](https://developer.valvesoftware.com/wiki/Navigation_Meshes).  A bot traverses the mesh using an algorithm similar to [Valve's reactive path following technique](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf).  
The navigation mesh generated provided by Valve generates hiding spots based on various geometric features of the map.  The bots will randomly use these spots to camp/snipe, but the generated spots are not always placed in the most ideal positions.  For example, some spots are generated in the middle of a spawn area where an opponent will never visit.  This bot addresses this problem by treating the hiding spots as a [Multi-arm Bandit problem](https://en.wikipedia.org/wiki/Multi-armed_bandit) and tries to solve the problem using [Thompson Sampling](https://en.wikipedia.org/wiki/Thompson_sampling).
The bot currently supports Valve's [Day of Defeat Source](https://store.steampowered.com/app/300/Day_of_Defeat_Source/) and [Half-Life 2 Deathmatch](https://store.steampowered.com/app/320/HalfLife_2_Deathmatch/).

## Installing the Bot ##
Extract the tgz files into desired game directory, e.g. C:\program files (x86)\steam\steamapps\common\day of defeat source\dod

## Running the Bot from the Desktop (Listen Server) ##
On Windows, you would need to launch the game with the -insecure option.  For Linux, execute the following from console:
```
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/lib/i386-linux-gnu:$HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/usr/lib/i386-linux-gnu:bin $HOME/.local/share/Steam/steamapps/common/<game dir>/hl2_linux -game <mod dir>
```
Where "game dir" is either "Day of Defeat Source" or "Half-Life 2 Deathmatch" and "mod dir" is either dod or hl2mp respectively.

## Playing with Bots ##
* Generate a navigation mesh for the map if one has not been generated yet.  
  * See [here](https://developer.valvesoftware.com/wiki/Nav_Mesh), [here](https://steamcommunity.com/sharedfiles/filedetails/?id=485737805), and [here](https://steamcommunity.com/groups/p_nm/discussions/0/540741132001702464/) for for tutorials and tips on how to generate and edit navigation meshes.  
  * As mentioned earlier, the navigation mesh generator generates a large number of hide spots, and most of these are not appropriate for Day of Defeat Source.  While the bot does use machine learning to try to choose better hide areas, it might make sense to just manually remove the unwanted hide spots.  Keep in mind that reducing the number of hidespots also reduces the stocacity of the bot's pathing somewhat--the bots will seldom or never go to areas of the map that do not contain objectives unless hidespots are present.  To adjust the hide spots generated after the initial ```nav_generate```:
    * select all nav areas using ```nav_flood_select``` or add one area to selected set and use```nav_select_orphans```.
    * deselect the areas where hide spots are desired.
    * execute ```nav_dont_hide```.
    * execute ```nav_analyze```.
* To add a bot:  ```mybot_add_bot [name] [team] [class]``` Team and class are the numbers as indicated in the game menu for Day of Defeat Source, and are ignored in Half-Life 2 Deathmatch.
* The server variable ```mybot_min_players``` ensure that there will be a minimum number of players (Bots or real) at all times.
  * For example, setting this value to 30 in a map with no players will create 30 bots.  If a player joins, a bot will drop automatically.  Bots dropped and added will attempt to keep the team balanced.
  * Setting the value to the default, -1, will disable the algorithm.

## Compiling the Bot ##
The bot can be built on Windows (Visual Studio 2012 or higher) and LINUX with [CMake](https://cmake.org/) and [Valve's Source SDK](alliedmodders/hl2sdk). The toplevel CMakeList.txt assumes that hl2sdk and valve_plugin_navmesh are in the same subdirectory as this repo.  It assumes that [CxxTest](https://cxxtest.com/) is installed via apt in LINUX, and in Windows it is installed in the root of drive that is specified (defaults to C:). It is also assumed that [Steam](https://store.steampowered.com/about/) is installed within the HOMEPATH for the LINUX build environment.  Please see the [Valve Source SDK Wiki on compiling](https://developer.valvesoftware.com/wiki/Source_SDK_2013) for all other platform specific dependencies.  
