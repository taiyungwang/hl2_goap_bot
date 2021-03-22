#pragma once

#include <weapon/Deployer.h>

class MoveStateContext;
class Player;

class DODMGDeployer: public Deployer {
public:
	DODMGDeployer(const Weapon& weapon): Deployer(weapon) {
	}

	~DODMGDeployer();

	bool execute(Blackboard& blackboard);

	void undeploy(Blackboard& blackboard);

private:
	bool proneRequired = true, started = false;

	MoveStateContext* moveCtx = nullptr;

	const Player* target = nullptr;

	int animationCounter = -1;

	void start(Blackboard& blackboard);
};
