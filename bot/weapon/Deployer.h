#pragma once

class Blackboard;
class Weapon;

class Deployer {
public:
	Deployer(const Weapon& weapon): weapon(weapon) {
	}

	virtual ~Deployer() {
	}

	virtual bool deploy(Blackboard& blackboard);

protected:
	const Weapon& weapon;
};
