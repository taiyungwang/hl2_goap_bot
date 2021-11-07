#pragma once

#include <IEngineTrace.h>

struct edict_t;

class FilterSelfAndEnemies: public CTraceFilterEntitiesOnly {
public:
	FilterSelfAndEnemies(edict_t* self) : self(self) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) override;

private:
	edict_t *self;
};
