#pragma once

#include "GoToAction.h"
#include <nav_mesh/nav_pathfind.h>
#include <unordered_map>

class FindCoverAction: public GoToAction, public ISearchSurroundingAreasFunctor {
public:
	FindCoverAction(Blackboard& blackboard);

	bool isInterruptable() const {
		return false;
	}

	bool execute() override;

	bool operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar );

	bool ShouldSearch( CNavArea *adjArea, CNavArea *currentArea, float travelDistanceSoFar );

	void PostSearch( void );

protected:
	CNavArea* hideArea = nullptr, *startArea = nullptr;

	float maxRange = INFINITY;

	std::unordered_map<CNavArea*, edict_t*> areasToAvoid;

	virtual void setAvoidAreas();

	virtual void getAvoidPosition(Vector& pos, edict_t* avoid) const;

	virtual bool waitInCover() const {
		return false;
	}

private:
	bool findTargetLoc();
};
