#include "SnipeAction.h"

#include <player/Buttons.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <weapon/Weapon.h>
#include <weapon/Deployer.h>
#include <nav_mesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>
#include <worldsize.h>

HidingSpotSelector* SnipeAction::selector = nullptr;

ConVar snipeChance("mybot_snipe_chance", "0.5");

SnipeAction::SnipeAction(Bot* self) : GoToAction(self) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
	targetRadius = 16.0f;
	facing.x = facing.z = facing.y = 0.0f;
}

bool SnipeAction::init() {
	int team = self->getTeam();
	duration = 300;
	const auto weapon = self->getCurrWeapon();
	if (!GoToAction::init()
			|| (weapon && weapon->getMinDeployRange()
			> calculateFacing())) {
		selector->update(selectorId, team, false);
		return false;
	}
	return true;
}

bool SnipeAction::precondCheck() {
	if (selector == nullptr) {
		return false;
	}
	int team = self->getTeam();
	selectorId = selector->select(targetLoc, team);
	if (selectorId >= 0) {
		selector->setInUse(selectorId, team, true);
	}
	return selectorId >= 0;
}

bool SnipeAction::execute() {
	if (!GoToAction::execute()) {
		return false;
	}
	if (!GoToAction::goalComplete() || self->isOnLadder()) {
		return true;
	}
	Vector aim;
	AngleVectors(facing, &aim);
	self->setViewTarget(aim * 100.0f + self->getCurrentPosition());
	self->lookStraight();
	auto weapon = self->getCurrWeapon();
	if (!weapon) {
		return true;
	}
	Deployer* deployer = weapon->getDeployer();
	if (deployer != nullptr && self->getAimAccuracy() > 0.8f) {
		if (!deployer->execute(self)) {
			return false;
		}
		if (!weapon->isDeployed()) {
			return true;
		}
	}
	if (crouch) {
		self->getButtons().hold(IN_DUCK);
	}
	return --duration <= 0;
}

bool SnipeAction::goalComplete() {
	int team = self->getTeam();
	selector->setInUse(selectorId, team, false);
	auto weapon = self->getCurrWeapon();
	if (weapon) {
		weapon->undeploy(self);
	}
	if (GoToAction::goalComplete()) {
		// if we are at our location, and we didn't see an enemy, then count it as failure
		selector->update(selectorId, team, 
			self->getVision().getTargetedPlayer() != 0);
		return true;
	}
	return false;
}

void SnipeAction::abort() {
	int team = self->getTeam();
	selector->setInUse(selectorId, team, false);
	if (self->getVision().getTargetedPlayer() != 0
		&& GoToAction::goalComplete()) {
		// if we see an enemy at our spot, then it's successful.
		selector->update(selectorId, team, true);
	}
	auto weapon = self->getCurrWeapon();
	if (weapon != nullptr) {
		weapon->undeploy(self);
	}
}

float SnipeAction::getChanceToExec() const {
	return snipeChance.GetFloat();
}

float SnipeAction::calculateFacing() {
	Vector pos = targetLoc;
	pos.z += HumanEyeHeight;
	float furthest = 0.0f;
	for (float currFacing = -180.0f; currFacing < 180.0f; currFacing += 20.0f) {
		QAngle angle(0.0f, currFacing, 0.0f);
		Vector aim;
		AngleVectors(angle, &aim);
		trace_t result;
		Vector target = pos + aim * MAX_COORD_FLOAT;
		self->canSee(result, pos, target);
		if (!result.DidHit() && !result.startsolid) {
			furthest = MAX_COORD_FLOAT;
			facing.y = currFacing;
			break;
		}
		float dist = result.endpos.DistTo(pos);
		if (dist > furthest) {
			pos.z += HumanCrouchHeight - HumanEyeHeight;
			self->canSee(result, pos, target);
			furthest = dist;
			facing.y = currFacing;
			crouch = !result.DidHit() || result.endpos.DistTo(pos) >= dist;
		}
	}
	return furthest;
}
