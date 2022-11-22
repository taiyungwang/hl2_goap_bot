#include "SnipeAction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/Deployer.h>
#include <nav_mesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <in_buttons.h>
#include <worldsize.h>

HidingSpotSelector* SnipeAction::selector = nullptr;

ConVar snipeChance("mybot_snipe_chance", "0.5");

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
	targetRadius = 16.0f;
	facing.x = facing.z = facing.y = 0.0f;
}

bool SnipeAction::init() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	duration = 300;
	const Weapon *weapon = self->getArsenal().getCurrWeapon();
	if (!GoToAction::init()
			|| (weapon != nullptr && weapon->getMinDeployRange()
			> calculateFacing())) {
		selector->update(selectorId, team, false);
		return false;
	}
	return true;
}

bool SnipeAction::precondCheck() {
	int team = blackboard.getSelf()->getTeam();
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
	if (!GoToAction::goalComplete() || blackboard.isOnLadder()) {
		return true;
	}
	Vector aim;
	AngleVectors(facing, &aim);
	auto self = blackboard.getSelf();
	self->setViewTarget(aim * 100.0f + blackboard.getSelf()->getCurrentPosition());
	self->lookStraight();
	Weapon *weapon = blackboard.getSelf()->getArsenal().getCurrWeapon();
	Deployer* deployer = weapon->getDeployer();
	if (deployer != nullptr && self->getAimAccuracy() > 0.8f) {
		if (!deployer->execute(blackboard)) {
			return false;
		}
		if (!weapon->isDeployed()) {
			return true;
		}
	}
	if (crouch) {
		blackboard.getButtons().hold(IN_DUCK);
	}
	return --duration <= 0;
}

bool SnipeAction::goalComplete() {
	int team = blackboard.getSelf()->getTeam();
	selector->setInUse(selectorId, team, false);
	auto weapon = blackboard.getSelf()->getArsenal().getCurrWeapon();
	if (weapon != nullptr) {
		weapon->undeploy(blackboard);
	}
	if (GoToAction::goalComplete()) {
		// if we are at our location, and we didn't see an enemy, then count it as failure
		selector->update(selectorId, team, 
			blackboard.getSelf()->getVision().getTargetedPlayer() != 0);
		return true;
	}
	return false;
}

void SnipeAction::abort() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selector->setInUse(selectorId, team, false);
	if (self->getVision().getTargetedPlayer() != 0
		&& GoToAction::goalComplete()) {
		// if we see an enemy at our spot, then it's successful.
		selector->update(selectorId, team, true);
	}
	auto weapon = self->getArsenal().getCurrWeapon();
	if (weapon != nullptr) {
		weapon->undeploy(blackboard);
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
		blackboard.getSelf()->canSee(result, pos, target);
		if (!result.DidHit() && !result.startsolid) {
			furthest = MAX_COORD_FLOAT;
			facing.y = currFacing;
			break;
		}
		float dist = result.endpos.DistTo(pos);
		if (dist > furthest) {
			pos.z += HumanCrouchHeight - HumanEyeHeight;
			blackboard.getSelf()->canSee(result, pos, target);
			furthest = dist;
			facing.y = currFacing;
			crouch = !result.DidHit() || result.endpos.DistTo(pos) >= dist;
		}
	}
	return furthest;
}
