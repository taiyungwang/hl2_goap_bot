#include "Weapon.h"

#include "WeaponFunction.h"
#include "Deployer.h"
#include "Reloader.h"
#include <util/BaseCombatWeapon.h>
#include <edict.h>

float Weapon::MELEE_RANGE = 50.0f;

Weapon::Weapon(edict_t* ent) :
		weap(ent) {
	function[0] = function[1] = nullptr;
	grenade = underWater = false;
	minDeployRange = 0.0f;
}

void Weapon::setWeaponFunc(int i, const std::shared_ptr<WeaponFunction>& func) {
	BaseCombatWeapon combatWeap(weap);
	function[i] = func;
	function[i]->setClip(combatWeap.getClipIndex(i));
	function[i]->setClipId(combatWeap.getAmmoType(i));
}

int Weapon::getWeaponState() const {
	return BaseCombatWeapon(weap).getWeaponState();
}

bool Weapon::isDeployed() const {
	return deployedVarName != nullptr && BaseCombatWeapon(weap).get<bool>(deployedVarName, false);
}

void Weapon::undeploy(Bot *self) {
	 if (deployer) {
		 deployer->undeploy(self);
	 }
}

bool Weapon::isOutOfAmmo(edict_t* player) const {
	return checkAmmo([player](WeaponFunction* weapFunc) {
		return weapFunc->getAmmo(player);
	});
}

bool Weapon::isClipEmpty() const {
	return checkAmmo([](WeaponFunction* weapFunc) {
		return weapFunc->getClip();
	});
}

float Weapon::getDamage(edict_t* player, float dist) const {
	return MAX(function[0]->getDamageRating(player, dist),
			function[1] == nullptr ?
					0.0f : function[1]->getDamageRating(player, dist));
}

bool Weapon::isInRange(float distance) const {
	return (function[0]->getRange()[0] < distance
					&& function[0]->getRange()[1] > distance)
			|| (function[1]
					&& function[1]->getRange()[0] < distance
					&& function[1]->getRange()[1] > distance);
}

WeaponFunction* Weapon::chooseWeaponFunc(edict_t* self, float dist) const {
	return function[1] && function[1]->getDamageRating(self, dist)
					> function[0]->getDamageRating(self, dist) ?
			function[1].get() : function[0].get();
}

template<typename Func>
bool Weapon::checkAmmo(const Func& getAmmo) const {
	return (!function[0]->isMelee() && getAmmo(function[0].get()) == 0)
		|| (function[1] && !function[1]->isMelee() && getAmmo(function[1].get()) == 0);
}
