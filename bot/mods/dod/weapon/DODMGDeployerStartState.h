#pragma once
#include "DODMGDeployerState.h"

class DODMGDeployerStartState: public DODMGDeployerState {
public:
	DODMGDeployerStartState(DODMGDeployer *context): DODMGDeployerState(context) {
	}

	void deploy(Blackboard& blackboard) override;

protected:
	int wait = 0;
};
