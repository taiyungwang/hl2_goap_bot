#include "Weapon.h"

#include "WeaponFunction.h"
#include "Deployer.h"
#include "Reloader.h"
#include <util/BaseCombatWeapon.h>
#include <edict.h>

Weapon::Weapon(edict_t* ent) :
		weap(ent), weaponName(ent->GetNetworkable()->GetClassName()) {
	function[0] = function[1] = nullptr;
	grenade = underWater = false;
	minDeployRange = INFINITY;
}

Weapon::~Weapon() {
	for (int i = 0; i < 2; i++) {
		if (function[i] != nullptr) {
			delete function[i];
			function[i] = nullptr;
		}
	}
	if (deployer != nullptr) {
		delete deployer;
	}
	if (reloader != nullptr) {
		delete reloader;
	}
}

void Weapon::setWeaponFunc(int i, WeaponFunction* func) {
	BaseCombatWeapon combatWeap(weap);
	function[i] = func;
	function[i]->setClip(combatWeap.getClipIndex(i));
	function[i]->setClipId(combatWeap.getAmmoType(i));
}

int Weapon::getWeaponState() const {
	return BaseCombatWeapon(weap).getWeaponState();
}

bool Weapon::isDeployed() const {
	return deployedCheck != nullptr
			&& deployedCheck->get<bool>(weap);
}

void Weapon::undeploy(Blackboard& blackboard) {
	 if (deployer != nullptr) {
		 deployer->undeploy(blackboard);
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
			|| (function[1] != nullptr && function[1]->getRange()[0] < distance
					&& function[1]->getRange()[1] > distance);
}

WeaponFunction* Weapon::chooseWeaponFunc(edict_t* self, float dist) const {
	return function[1] != nullptr
			&& function[1]->getDamageRating(self, dist)
					> function[0]->getDamageRating(self, dist) ?
			function[1] : function[0];
}

template<typename Func>
bool Weapon::checkAmmo(const Func& getAmmo) const {
	return (!function[0]->isMelee() && getAmmo(function[0]) == 0)
		|| (function[1] != nullptr && !function[1]->isMelee() && getAmmo(function[1]) == 0);
}
