#pragma once

#include "DODMGDeployerDeployState.h"

class DODMGDeployerStandState: public DODMGDeployerDeployState {
public:
	DODMGDeployerStandState(DODMGDeployer *context): DODMGDeployerDeployState(context) {
	}

	void deploy(Blackboard& blackboard) override;
};
