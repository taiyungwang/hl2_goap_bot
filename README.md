# hl2_goap_bot
#A Half-Life 2 Bot driven by GOAP#

GOAP stands for Goal Oriented Action Planning.  Please visit [Jeff Orkin's page](http://alumni.media.mit.edu/~jorkin/goap.html) for more information on GOAP.  The bot uses a [refactored version of Valve's Navigation Mesh](taiyungwang/valve_source_plugin_navmesh).  It also uses [Thompson sampling](https://en.wikipedia.org/wiki/Thompson_sampling) to determine which Hide spots from the navigation mesh to use. 

##Compiling the Bot##
The bot is compiled with CMake
##TODO##
*Make bots less accurate.  Currently the bots are nearly 100% accurate with their aim.  
*Remove the copied Navigation mesh from Valve SDK.
*Add a framework in HL2DM for bots to determine which items to pick up.
*Add logic for bots to pick up a substitue weapon when current out of ammo for primary weapon.
*Refactor aiming behavior defined in the GOAP Action classes into an aiming subsystem.
