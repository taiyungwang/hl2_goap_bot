#pragma once

#include <goap/action/SnipeAction.h>

class DODObjectives;
class DODObjective;

class DODDefendPointAction: public SnipeAction {
public:
	DODDefendPointAction(Blackboard& blackboard);

	void setObjectives(const DODObjectives* objectives) {
		this->objectives = objectives;
	}

	bool execute();

private:
	const DODObjectives* objectives = nullptr;

	const DODObjective* target = nullptr;

	Vector guardTarget;

	bool findTargetLoc();

	bool isTargetValid() const;
};
