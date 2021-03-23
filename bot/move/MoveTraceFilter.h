#pragma once

#include <util/UtilTrace.h>

class Player;

class MoveTraceFilter: public FilterList {
public:
	MoveTraceFilter(const Player& self, edict_t *target);

	bool ShouldHitEntity(IHandleEntity *pHandleEntity,
			int contentsMask);

private:
	int team;
};
