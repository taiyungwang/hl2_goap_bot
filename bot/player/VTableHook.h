#pragma once

struct edict_t;

void hookPlayerRunCommand(edict_t *edict);

void unhookPlayerRunCommand();
