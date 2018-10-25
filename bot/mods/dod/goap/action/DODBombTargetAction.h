#pragma once

#include "DODDefuseBombAction.h"

class DODBombTargetAction: public DODDefuseBombAction {
public:
	DODBombTargetAction(Blackboard& blackboard);

	bool execute() {
		return DODDefuseBombAction::execute()
			&& (!GoToAction::postCondCheck() || duration++ >= 1000);
	}

private:
	bool isAvailable(int idx) const;
};
