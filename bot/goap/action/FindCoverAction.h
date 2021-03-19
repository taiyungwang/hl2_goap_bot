#pragma once

#include "GoToAction.h"
#include <nav_mesh/nav_pathfind.h>

class FindCoverAction: public GoToAction, public ISearchSurroundingAreasFunctor {
public:
	FindCoverAction(Blackboard& blackboard);

	bool isInterruptable() const {
		return false;
	}

	bool execute() {
		return getTarget() == nullptr || GoToAction::execute();
	}


	float getCost();

	bool onPlanningFinished();

	bool operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar );

	bool ShouldSearch( CNavArea *adjArea, CNavArea *currentArea, float travelDistanceSoFar );

	void PostSearch( void );

private:
	CNavArea* hideArea = nullptr, *currentArea = nullptr;

	edict_t* getTarget() const;

	bool findTargetLoc();
};
