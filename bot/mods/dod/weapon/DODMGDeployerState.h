#pragma once

class DODMGDeployer;
class Bot;

class DODMGDeployerState {
public:
	DODMGDeployerState(DODMGDeployer *context): context(context) {
	}

	virtual ~DODMGDeployerState() {
	}

	virtual void deploy(Bot *self) = 0;

protected:
	DODMGDeployer *context;
};
