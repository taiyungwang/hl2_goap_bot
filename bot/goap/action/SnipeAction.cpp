#include "SnipeAction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <weapon/Weapon.h>
#include <weapon/Deployer.h>
#include <nav_mesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

HidingSpotSelector* SnipeAction::selector = nullptr;

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
	targetRadius = 16.0f;
}

bool SnipeAction::precondCheck() {
	deployed = false;
	const Bot* self = blackboard.getSelf();
	int team = self->getTeam();
	if (RandomInt(1, 2) == 1) {
		return false;
	}
	if (!GoToAction::precondCheck()) {
		selector->update(selectorId, team, false);
		return false;
	}
	Vector pos = targetLoc;
	pos.z += HumanCrouchHeight;
	float furthest = 0.0f;
	FilterSelf filter(self->getEdict()->GetIServerEntity());
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
			facing = currFacing;
			break;
		}
		float dist = result.endpos.DistToSqr(pos);
		if (dist > furthest) {
			furthest = dist;
			facing = currFacing;
		}
	}
	duration = 300;
	return true;
}
void SnipeAction::init() {
	GoToAction::init();
	selector->setInUse(selectorId, blackboard.getSelf()->getTeam(), true);
}


bool SnipeAction::findTargetLoc() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selectorId = selector->select(targetLoc, team);
	return selectorId >= 0;
}

bool SnipeAction::execute() {
	if (!GoToAction::postCondCheck()) {
		if (!GoToAction::execute()) {
			return false;
		}
		return true;
	}
	QAngle angle(0.0f, facing, 0.0f);
	Vector aim;
	AngleVectors(angle, &aim);
	blackboard.setViewTarget(aim * 100.0f + blackboard.getSelf()->getCurrentPosition());
	Deployer* deployer = blackboard.getArmory().getCurrWeapon()->getDeployer();
	if (deployer == nullptr) {
		blackboard.getButtons().hold(IN_DUCK);
		deployed = true;
	} else if (!deployed && blackboard.getAimAccuracy(blackboard.getViewTarget()) > 0.8f) {
		deployed = deployer->execute(blackboard);
	}
	blackboard.lookStraight();
	return deployed && --duration <= 0;
}

bool SnipeAction::postCondCheck() {
	int team = blackboard.getSelf()->getTeam();
	selector->setInUse(selectorId, team, false);
	if (GoToAction::postCondCheck()) {
		// if we are at our location, and we didn't see an enemy, then count it as failure
		selector->update(selectorId, team, 
			blackboard.getTargetedPlayer() != nullptr);
		return true;
	}
	return false;
}

void SnipeAction::abort() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selector->setInUse(selectorId, team, false);
	if (blackboard.getTargetedPlayer() != nullptr
		&& GoToAction::postCondCheck()) {
		// if we see an enemy at our spot, then it's successful.
		selector->update(selectorId, team, true);
	}
}
