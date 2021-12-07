#pragma once
#include "DODMGDeployerState.h"

class DODMGDeployerStartState: public DODMGDeployerState {
public:
	DODMGDeployerStartState(DODMGDeployer *context): DODMGDeployerState(context) {
	}

	void deploy(Blackboard& blackboard) override;

protected:
	static const int PRONE_TIMEOUT, DEPLOY_TIMEOUT;

	int wait = 0;
};
