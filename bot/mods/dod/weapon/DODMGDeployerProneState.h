#pragma once

#include "DODMGDeployerStartState.h"
#include <memory>

class MoveStateContext;

class DODMGDeployerProneState: public DODMGDeployerStartState {
public:
	DODMGDeployerProneState(DODMGDeployer *ctx): DODMGDeployerStartState(ctx) {
	}

	void deploy(Bot *self) override;

private:
	std::shared_ptr<MoveStateContext> moveCtx;

	int targetIdx = 0;
};
