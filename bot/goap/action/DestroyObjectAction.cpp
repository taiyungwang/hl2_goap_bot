#include "DestroyObjectAction.h"

#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <weapon/Deployer.h>
#include <nav_mesh/nav_area.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>
#include <util/BaseEntity.h>
#include <convar.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>
#include <edict.h>

DestroyObjectAction::DestroyObjectAction(Blackboard& blackboard) :
		Action(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
	crouch = false;
	effects = {WorldProp::IS_BLOCKED, false};
	precond.Insert(WorldProp::WEAPON_IN_RANGE, true);
	precond.Insert(WorldProp::WEAPON_LOADED, true);
}

DestroyObjectAction::~DestroyObjectAction() {
	delete moveCtx;
}

bool DestroyObjectAction::precondCheck() {
	edict_t* blocker = blackboard.getBlocker();
	if (blocker == nullptr) {
		return false;
	}
	dur = 200;
	const char* className = blocker->GetClassName();
	adjustAim = true;
	return Q_stristr(className, "breakable") != nullptr
			|| Q_strcmp(className, "player") == 0
			|| isBreakable(blocker);
}

bool DestroyObjectAction::execute() {
	if (dur-- < 1) {
		return true;
	}
	Weapon* weapon = blackboard.getArmory().getCurrWeapon();
	if (goalComplete() || weapon->isClipEmpty()) {
		return true;
	}
	const Player* self = blackboard.getSelf();
	edict_t* selfEnt = self->getEdict();
	edict_t* targetEnt = getTargetedEdict();
	if (targetEnt == nullptr || targetEnt->IsFree()) {
		return true;
	}
	Vector targetLoc = blackboard.getViewTarget();
	Vector eyes = self->getEyesPos();
	float dist = targetLoc.DistTo(self->getCurrentPosition());
	Buttons& buttons = blackboard.getButtons();
	if (weapon->getDeployer() != nullptr
			&& weapon->getMinDeployRange() < dist
			&& !weapon->getDeployer()->execute(blackboard)) {
		return false;
	}
	WeaponFunction* weapFunc = weapon->chooseWeaponFunc(selfEnt, dist);
	if (weapFunc->isExplosive()) {
		targetLoc = targetEnt->GetCollideable()->GetCollisionOrigin();
	}
	if (weapFunc->isMelee() && !crouch) {
		moveCtx->setGoal(
				(targetLoc - self->getCurrentPosition()).Normalized()
						* (dist - 25.0f) + self->getCurrentPosition());
		CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
		moveCtx->traceMove();
		moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
		buttons.hold(IN_SPEED);
	}
	if (adjustAim) {
		targetLoc = weapFunc->getAim(targetLoc, eyes);
		adjustAim = false;
	}
	if ((!weapFunc->isMelee() && weapFunc->getClip() != -1
			&& weapFunc->getClip() < 1) || !weapFunc->isInRange(dist)) {
		return true;
	}
	blackboard.setViewTarget(targetLoc);
	extern ConVar mybot_debug;
	if (crouch) {
		buttons.hold(IN_DUCK);
	}
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(eyes, targetLoc, 255, 0, 255, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
		debugoverlay->AddLineOverlay(eyes, eyes + blackboard.getFacing() * dist, 0, 255, 0, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	if (weapFunc->isMelee() || fabs(blackboard.getAimAccuracy(targetLoc))
			> 1.0f - 10.0f / MAX(dist, 0.1f)) {
		if ((!UTIL_IsVisible(targetLoc, blackboard, targetEnt)
				|| (weapFunc->isMelee() && eyes.z - targetLoc.z > 20.0f))
				&& !crouch) {
			// try crouching
			crouch = true;
			return false;
		}
		weapFunc->attack(buttons, dist);
	}
	return false;
}

bool DestroyObjectAction::goalComplete() {
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
