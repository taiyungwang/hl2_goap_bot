#pragma once

class Bot;
class Weapon;

/**
 * Performs an action on a given weapon such as reload or deploy.
 */
class WeaponActor {
public:
	WeaponActor(const Weapon& weapon): weapon(weapon) {
	}

	virtual ~WeaponActor() {
	}

	/**
	 * Executes an action against the given weapon.
	 * @return True if action is complete.
	 */
	virtual bool execute(Bot *self) = 0;

protected:
	const Weapon& weapon;
};
