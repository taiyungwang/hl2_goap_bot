#pragma once

#include "GoToAction.h"
#include <navmesh/nav_pathfind.h>

class FindCoverAction: public GoToAction, public ISearchSurroundingAreasFunctor {
public:
	FindCoverAction(Blackboard& blackboard);

	bool precondCheck();

	float getCost() const {
		return GoToAction::getCost() * 2.0f;
	}

	bool isInterruptable() const {
		return false;
	}

	bool operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar );

	bool ShouldSearch( CNavArea *adjArea, CNavArea *currentArea, float travelDistanceSoFar );

	void PostSearch( void );

private:
	CNavArea* hideArea = nullptr, *currentArea = nullptr;

	edict_t* getTarget() const;
};
