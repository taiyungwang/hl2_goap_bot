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

HidingSpotSelector* SnipeAction::selector = nullptr;

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
	precond.Insert(WorldProp::USING_BEST_WEAP, true);
	targetRadius = 16.0f;
	facing.x = facing.z = facing.y = 0.0f;
}

bool SnipeAction::onPlanningFinished() {
	int team = blackboard.getSelf()->getTeam();
	if (!GoToAction::onPlanningFinished()) {
		selector->update(selectorId, team, false);
		return false;
	}
	calculateFacing();
	duration = 300;
	return true;
}

bool SnipeAction::findTargetLoc() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selectorId = selector->select(targetLoc, team);
	if (selectorId >= 0) {
		selector->setInUse(selectorId, blackboard.getSelf()->getTeam(), true);
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
	if (deployer != nullptr && self->getAimAccuracy() > 0.8f
			&& !deployer->execute(blackboard)) {
		return false;
	}
	if (deployer == nullptr || weapon->isDeployed()) {
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

void SnipeAction::calculateFacing() {
	Vector pos = targetLoc;
	pos.z += HumanCrouchHeight;
	float furthest = 0.0f;
	for (float currFacing = -180.0f; currFacing < 180.0f; currFacing += 20.0f) {
		QAngle angle(0.0f, currFacing, 0.0f);
		Vector aim;
		AngleVectors(angle, &aim);
		trace_t result;
		UTIL_TraceLine(pos, pos + aim * 2000.0f,
				CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP,
				NULL, COLLISION_GROUP_NONE, &result);
		if (!result.DidHit() && !result.startsolid) {
			furthest = 4000000.0f;
			facing.y = currFacing;
			break;
		}
		float dist = result.endpos.DistToSqr(pos);
		if (dist > furthest) {
			furthest = dist;
			facing.y = currFacing;
		}
	}
}
