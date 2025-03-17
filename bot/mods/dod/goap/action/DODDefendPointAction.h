#pragma once

#include <goap/action/SnipeAction.h>

class DODObjectives;
class DODObjective;

class DODDefendPointAction: public SnipeAction {
public:
	DODDefendPointAction(Bot *self);

	void setObjectives(const DODObjectives* objectives) {
		this->objectives = objectives;
	}

	bool precondCheck() override;

	bool execute() {
		return isTargetValid() && SnipeAction::execute();
	}

	float getChanceToExec() const;

private:
	const DODObjectives* objectives = nullptr;

	const DODObjective* target = nullptr;

	Vector guardTarget;

	bool isTargetValid() const;
};
