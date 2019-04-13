#pragma once

#include "DODDefuseBombAction.h"

class DODBombTargetAction: public DODDefuseBombAction {
public:
	DODBombTargetAction(Blackboard& blackboard);

	bool execute();

private:
	bool isAvailable(int idx);

	bool isTeamMateActingOnBomb(DodPlayer& teammate) const;
};
