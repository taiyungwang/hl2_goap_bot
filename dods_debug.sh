#!/usr/bin/env bash

echo 'Starting DoDS GDB server'
cd ~/.steam/steam/steamapps/common/Day\ of\ Defeat\ Source
LD_LIBRARY_PATH=bin/linux64
gdbserver localhost:12345 ./dod_linux64 -console +plugin_load addons/mybot/my_bot
cd -