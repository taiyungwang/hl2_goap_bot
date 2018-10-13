#pragma once

#include "FindPathAction.h"
#include <navmesh/nav_pathfind.h>

class FindCoverAction: public FindPathAction, public ISearchSurroundingAreasFunctor {
public:
	FindCoverAction(Blackboard& blackboard);

	bool precondCheck();

	bool postCondCheck();

	bool execute() {
		return true;
	}

	float getCost() const {
		return FindPathAction::getCost() * 2.0f;
	}

	bool operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar );

	void PostSearch( void );

private:
	CNavArea* hideArea = nullptr;

	edict_t* getTarget() const;
};
