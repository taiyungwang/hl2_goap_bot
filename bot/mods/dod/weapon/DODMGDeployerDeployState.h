#pragma once

#include "DODMGDeployerState.h"

class DODMGDeployerDeployState: public DODMGDeployerState {
public:
	DODMGDeployerDeployState(DODMGDeployer *context) :
			DODMGDeployerState(context) {
	}

	virtual void deploy(Blackboard& blackboard) override;

protected:
	int frames = 0;
};
