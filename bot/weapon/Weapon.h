#pragma once

class EntityVar;
class WeaponFunction;
class Deployer;
class Reloader;
struct edict_t;

/**
 * Defines a weapon.
 */
class Weapon {
public:

	Weapon(edict_t* ent);

	~Weapon();

	void setPrimary(WeaponFunction* func) {
		setWeaponFunc(0, func);
	}

	void setSecondary(WeaponFunction* func) {
		setWeaponFunc(1, func);
	}

	int getWeaponState() const;

	WeaponFunction* getPrimary() const {
		return function[0];
	}

	WeaponFunction* getSecondary() const {
		return function[1];
	}

	bool isGrenade() const {
		return grenade;
	}

	void setGrenade(bool grenade) {
		this->grenade = grenade;
	}

	bool isUnderWater() const {
		return underWater;
	}

	void setUnderWater(bool underWater) {
		this->underWater = underWater;
	}

	bool isDeployed() const;

	const edict_t* getEdict() const {
		return weap;
	}

	bool isOutOfAmmo(edict_t* player) const;

	bool isClipEmpty() const;
	/**
	 * Returns the damage rating based on ammo count,
	 * target distance, and function damage rating.
	 */
	float getDamage(edict_t* player, float dist) const;

	/**
	 * @return True if distance is within weapon range.
	 */
	bool isInRange(float distance) const;

	/**
	 * @return the appropriate weapon function given the distance.
	 */
	WeaponFunction* chooseWeaponFunc(edict_t* self, float dist) const;

	void setDeployable(EntityVar* deployableCheck, float minDeployRange) {
		this->deployedCheck = deployableCheck;
		this->minDeployRange = minDeployRange;
	}

	float getMinDeployRange() const {
		return minDeployRange;
	}

	Deployer* getDeployer() const {
		return deployer;
	}

	void setDeployer(Deployer* deployer) {
		this->deployer = deployer;
	}

	Reloader* getReloader() const {
		return reloader;
	}

	void setReloader(Reloader* reloader) {
		this->reloader = reloader;
	}

private:
	WeaponFunction* function[2];

	const char* weaponName;

	EntityVar* deployedCheck = nullptr;

	float minDeployRange;

	bool underWater, grenade;

	edict_t* weap;

	Deployer* deployer = nullptr;

	Reloader* reloader = nullptr;

	void setWeaponFunc(int i, WeaponFunction* func);

	template<typename Func>
	bool checkAmmo(const Func& getAmmo) const;
};
