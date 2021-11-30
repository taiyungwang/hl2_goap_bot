#pragma once
#include "DODMGDeployerDeployState.h"

#include <memory>

class MoveStateContext;

class DODMGDeployerProneState: public DODMGDeployerDeployState {
public:
	DODMGDeployerProneState(DODMGDeployer *ctx): DODMGDeployerDeployState(ctx) {
	}

	void deploy(Blackboard& blackboard) override;

private:
	std::shared_ptr<MoveStateContext> moveCtx;
};
