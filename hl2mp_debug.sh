#!/usr/bin/env bash

echo 'Starting DoDS GDB server'
cd ~/.steam/steam/steamapps/common/Half-Life\ 2\ Deathmatch
LD_LIBRARY_PATH=bin/linux64
gdbserver localhost:12345 ./hl2mp_linux64 -console +plugin_load addons/mybot/my_bot
cd -