#pragma once

#include "GoToAction.h"

#include <move/NavMeshPathBuilder.h>
#include <unordered_map>

class FindCoverAction: public GoToAction, private NavMeshPathBuilder {
public:
	FindCoverAction(Blackboard& blackboard);

	bool precondCheck() override;

	bool onPlanningFinished() {
		return true;
	}

	bool execute() override {
		return GoToAction::execute() && !waitInCover();
	}

protected:
	float maxRange = INFINITY;

	std::unordered_map<CNavArea*, edict_t*> areasToAvoid;

	virtual void setAvoidAreas();

	virtual void getAvoidPosition(Vector& pos, edict_t* avoid) const;

	virtual bool waitInCover() const {
		return false;
	}

private:
	float getHeuristicCost(CNavArea *area) const override;

	bool foundGoal(CNavArea *area) override;

	bool shouldSearch(CNavArea *area) const override;

};
