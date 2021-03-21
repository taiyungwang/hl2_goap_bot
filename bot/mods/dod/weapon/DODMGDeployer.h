#pragma once

#include <weapon/Deployer.h>

class DODMGDeployer: public Deployer {
public:
	DODMGDeployer(const Weapon& weapon): Deployer(weapon) {
	}

	bool execute(Blackboard& blackboard);

	void undeploy(Blackboard& blackboard);

private:
	bool proneRequired = true;

	int animationCounter = -1;

	void start(Blackboard& blackboard);
};
