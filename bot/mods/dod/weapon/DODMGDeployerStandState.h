#pragma once

#include "DODMGDeployerProneState.h"

class DODMGDeployerStandState: public DODMGDeployerProneState {
public:
	DODMGDeployerStandState(DODMGDeployer *context) :
			DODMGDeployerProneState(context) {
	}

	void deploy(Blackboard &blackboard) override;
};
