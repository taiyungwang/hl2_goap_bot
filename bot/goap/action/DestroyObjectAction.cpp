#include "DestroyObjectAction.h"

#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <util/BaseEntity.h>
#include <navmesh/nav_area.h>
#include <convar.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>
#include <edict.h>

DestroyObjectAction::DestroyObjectAction(Blackboard& blackboard) :
		Action(blackboard) {
	maxWaitTime = waitTime = 0;
	moveCtx = new MoveStateContext(blackboard);
	crouch = false;
	effects = {WorldProp::IS_BLOCKED, false};
	precond.Insert(WorldProp::WEAPON_LOADED, true);
	precond.Insert(WorldProp::NEED_TO_DEPLOY_WEAPON, false);
}

DestroyObjectAction::~DestroyObjectAction() {
	delete moveCtx;
}

bool DestroyObjectAction::precondCheck() {
	edict_t* blocker = blackboard.getBlocker();
	if (blocker == nullptr) {
		return false;
	}
	const char* className = blocker->GetClassName();
	return Q_stristr(className, "breakable") != nullptr
			|| Q_strcmp(className, "player") == 0
			|| isBreakable(blocker);
}

bool DestroyObjectAction::execute() {
	Weapon* weapon = blackboard.getArmory().getCurrWeapon();
	if (postCondCheck() || weapon->isClipEmpty()) {
		return true;
	}
	const Player* self = blackboard.getSelf();
	edict_t* selfEnt = self->getEdict();
	edict_t* targetEnt = getTargetedEdict();
	Vector targetLoc = targetEnt->GetCollideable()->GetCollisionOrigin();
	Vector eyes = self->getEyesPos();
	float dist = targetLoc.DistTo(self->getCurrentPosition());
	Buttons& buttons = blackboard.getButtons();
	if (weapon->isDeployable() && !weapon->isDeployed()
			&& weapon->getMinDeployRange() < dist) {
		buttons.tap(IN_ATTACK2);
		return false;
	}
	WeaponFunction* weapFunc = weapon->chooseWeaponFunc(selfEnt, dist);
	if (!weapFunc->isExplosive()) {
		targetLoc.z += (targetEnt->GetCollideable()->OBBMaxs().z - targetLoc.z)
				* 0.5f;
	}
	getAdjustedTargetPos(targetLoc, weapFunc);
	if ((!weapFunc->isMelee() && weapFunc->getClip() != -1
			&& weapFunc->getClip() < 1) || !weapFunc->isInRange(dist)) {
		return true;
	}
	maxWaitTime = 10.0f * dist / weapFunc->getRange()[1];
	blackboard.setViewTarget(targetLoc);
	extern ConVar mybot_debug;
	if (!crouch) {
		crouch = (weapFunc->isMelee()
				&& fabs(targetLoc.z - eyes.z) > HumanCrouchHeight);
	}
	if (crouch) {
		buttons.hold(IN_DUCK);
	}
	if (weapFunc->isMelee()) {
		moveCtx->setGoal(
				(targetLoc - self->getCurrentPosition()).Normalized()
						* (dist - 25.0f) + self->getCurrentPosition());
		CNavArea* area = Navigator::getArea(selfEnt);
		moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
		if (moveCtx->isStuck()) {
			return true;
		}
		buttons.hold(IN_SPEED);
	}
	if (fabs(blackboard.getAimAccuracy(targetLoc))
			> 1.0f - 10.0f / (dist == 0.0f ? 0.0001f : dist) || weapFunc->isMelee()) {
		if (!isVisible(targetLoc, targetEnt)) {
			// aim for feet
			if (!crouch) {
				// try crouching
				crouch = true;
			} else {
				// target blocked.
				return true;
			}
			return false; 		// adjusted aim is blocked
		}
		if (waitTime-- <= 0) {
			waitTime = maxWaitTime;
			weapFunc->attack(buttons, dist);
		}
	}
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(eyes, targetLoc, 255, 0, 255, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
		debugoverlay->AddLineOverlay(eyes, eyes + self->getFacing() * dist, 0, 255, 0, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}

bool DestroyObjectAction::postCondCheck() {
	bool isDestroyed = blackboard.getBlocker() == nullptr
			|| BaseEntity(blackboard.getBlocker()).isDestroyedOrUsed()
			|| blackboard.getBlocker()->GetCollideable()->GetCollisionOrigin().DistTo(
					blackboard.getSelf()->getCurrentPosition()) > 130.0f;
	if (isDestroyed) {
		blackboard.setBlocker(nullptr);
	}
	return isDestroyed;
}

edict_t* DestroyObjectAction::getTargetedEdict() const {
	return blackboard.getBlocker();
}

void DestroyObjectAction::getAdjustedTargetPos(Vector& targetLoc,
		WeaponFunction* weapFunc) const {
	targetLoc = weapFunc->getAim(targetLoc, blackboard.getSelf()->getEyesPos());
}
