#pragma once

#include <weapon/Deployer.h>
#include <vector.h>
#include <memory>

class DODMGDeployerState;

class DODMGDeployer: public Deployer {
public:
	DODMGDeployer(const Weapon& weapon): Deployer(weapon) {
	}

	bool execute(Blackboard& blackboard);

	void undeploy(Blackboard& blackboard);

	const Weapon &getWeapon() const {
		return weapon;
	}

	void setState(std::shared_ptr<DODMGDeployerState> state) {
		this->state = state;
	}

private:
	bool started = false;

	std::shared_ptr<DODMGDeployerState> state;
};
