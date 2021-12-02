#pragma once

#include "DODMGDeployerStartState.h"
#include <vector.h>
#include <memory>

class MoveStateContext;

class DODMGDeployerProneState: public DODMGDeployerStartState {
public:
	DODMGDeployerProneState(DODMGDeployer *ctx): DODMGDeployerStartState(ctx) {
	}

	void deploy(Blackboard& blackboard) override;

protected:
	static const int PRONE_TIMEOUT, DEPLOY_TIMEOUT;

private:
	std::shared_ptr<MoveStateContext> moveCtx;

	int targetIdx = 0;

	Vector viewTarget;
};
