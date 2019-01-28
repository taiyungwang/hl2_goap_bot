#pragma once

#include <weapon/Deployer.h>

class DODMGDeployer: public Deployer {
public:
	DODMGDeployer(const Weapon& weapon): Deployer(weapon) {
	}

	bool execute(Blackboard& blackboard);

private:

	void reset(Blackboard& blackboard);

	int position = 3, animationCounter = -1;
};
