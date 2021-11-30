#pragma once

class DODMGDeployer;
class Blackboard;

class DODMGDeployerState {
public:
	DODMGDeployerState(DODMGDeployer *context): context(context) {
	}

	virtual ~DODMGDeployerState() {
	}

	virtual void deploy(Blackboard& blackboard) = 0;

protected:
	DODMGDeployer *context;
};
