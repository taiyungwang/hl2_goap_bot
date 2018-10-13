#pragma once

#include <IEngineTrace.h>

class Blackboard;
class Player;
struct edict_t;

class FilterSelfAndEnemies: public CTraceFilter {
public:
	// It does have a base, but we'll never network anything below here..

	FilterSelfAndEnemies(const Blackboard& blackboard, edict_t* self,
			edict_t* target) :
			blackboard(blackboard), self(self), target(target) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask);

private:
	const Blackboard& blackboard;
	edict_t* self, *target;
};

bool UTIL_IsVisible(const Vector &vecAbsEnd,
		Blackboard& blackboard, edict_t* target);

class Vision {
public:
	static void updateVisiblity(Blackboard& blackboard);
};
