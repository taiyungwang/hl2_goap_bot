#pragma once

#include "DODMGDeployerStartState.h"

class DODMGDeployerStandState: public DODMGDeployerStartState {
public:
	DODMGDeployerStandState(DODMGDeployer *ctx) :
			DODMGDeployerStartState(ctx) {
	}

	void deploy(Bot *self) override;
};
