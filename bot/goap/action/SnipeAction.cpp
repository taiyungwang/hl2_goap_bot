#include "SnipeAction.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <player/Bot.h>
#include <player/HidingSpotSelector.h>
#include <weapon/Weapon.h>
#include <weapon/Deployer.h>
#include <navmesh/nav_mesh.h>
#include <util/UtilTrace.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

HidingSpotSelector* SnipeAction::selector = nullptr;

SnipeAction::SnipeAction(Blackboard& blackboard) : GoToAction(blackboard) {
	effects = {WorldProp::ENEMY_SIGHTED, true};
	targetRadius = 0.0f;
}

bool SnipeAction::precondCheck() {
	deployed = false;
	path.Clear();
	if (RandomInt(0, 1) == 0) {
		return false;
	}
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selectorId = selector->select(targetLoc, team);
	if (selectorId < 0) {
		return false;
	}
	GoToAction::precondCheck();
	if (path.Count() == 0) {
		// count unreachable spots as failures.
		selector->update(selectorId, team, false);
		return false;
	}
	Vector pos = targetLoc;
	pos.z += HumanCrouchHeight;
	float furthest = 0.0f;
	for (float currFacing = -180.0f; currFacing < 180.0f; currFacing += 20.0f) {
		QAngle angle(0.0f, currFacing, 0.0f);
		Vector aim;
		AngleVectors(angle, &aim);
		trace_t result;
		FilterSelf filter(self->getEdict()->GetIServerEntity());
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
	if (furthest < 1000000.0) {
		// count spots with short range as failures.
		selector->update(selectorId, team, false);
		return false;
	}
	duration = 300;
	return true;
}

bool SnipeAction::execute() {
	if (!GoToAction::postCondCheck() && !GoToAction::execute()) {
		return false;
	}
	if (!GoToAction::postCondCheck()) {
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
	if (path.Count() == 0) {
		// if we are at our location, and we didn't see an enemy, then count it as failure
		selector->update(selectorId, team, 
			blackboard.getTargetedPlayer() != nullptr);
	}
	selector->resetInUse(selectorId, team);
	return true;
}

void SnipeAction::abort() {
	auto self = blackboard.getSelf();
	int team = self->getTeam();
	selector->resetInUse(selectorId, team);
	if (self->isDead()) {
		// if we died, then it's probably not an ideal spot to try for.
		selector->update(selectorId, team, false);
	} else if (blackboard.getTargetedPlayer() != nullptr
		&& path.Count() == 0) {
		// if we see an enemy at our spot, then it's successful.
		selector->update(selectorId, team, true);
	}
}
