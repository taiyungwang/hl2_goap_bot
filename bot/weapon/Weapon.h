#pragma once

#include <memory>

class Bot;
class BaseEntity;
class WeaponFunction;
class Deployer;
class Reloader;
struct edict_t;

/**
 * Defines a weapon.
 */
class Weapon {
public:
	static float MELEE_RANGE;

	Weapon(edict_t* ent);

	void setPrimary(std::shared_ptr<WeaponFunction> func) {
		setWeaponFunc(0, func);
	}

	void setSecondary(std::shared_ptr<WeaponFunction> func) {
		setWeaponFunc(1, func);
	}

	int getWeaponState() const;

	WeaponFunction* getPrimary() const {
		return function[0].get();
	}

	WeaponFunction* getSecondary() const {
		return function[1].get();
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

	void undeploy(Bot *self);

	edict_t* getEdict() const {
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

	void setDeployable(const char *deployedVarName, float minDeployRange) {
		this->deployedVarName = deployedVarName;
		this->minDeployRange = minDeployRange;
	}

	float getMinDeployRange() const {
		return minDeployRange;
	}

	Deployer* getDeployer() const {
		return deployer.get();
	}

	void setDeployer(const std::shared_ptr<Deployer>& deployer) {
		this->deployer = deployer;
	}

	Reloader* getReloader() const {
		return reloader.get();
	}

	void setReloader(const std::shared_ptr<Reloader>& reloader) {
		this->reloader = reloader;
	}

private:
	std::shared_ptr<WeaponFunction> function[2];

	const char* deployedVarName = nullptr;

	float minDeployRange;

	bool underWater, grenade;

	edict_t* weap;

	std::shared_ptr<Deployer> deployer;

	std::shared_ptr<Reloader> reloader;

	void setWeaponFunc(int i, const std::shared_ptr<WeaponFunction>& func);

	template<typename Func>
	bool checkAmmo(const Func& getAmmo) const;
};
