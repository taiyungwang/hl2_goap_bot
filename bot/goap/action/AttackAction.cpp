#include "AttackAction.h"

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

AttackAction::AttackAction(Blackboard& blackboard) :
		Action(blackboard) {
	moveCtx = new MoveStateContext(blackboard);
	crouch = false;
	effects = {WorldProp::IS_BLOCKED, false};
	precond.Insert(WorldProp::WEAPON_IN_RANGE, true);
	precond.Insert(WorldProp::WEAPON_LOADED, true);
}

AttackAction::~AttackAction() {
	delete moveCtx;
}

bool AttackAction::precondCheck() {
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

bool AttackAction::execute() {
	const Player* self = blackboard.getSelf();
	Vector targetLoc = blackboard.getViewTarget();
	float dist = targetLoc.DistTo(self->getCurrentPosition());
	Weapon* weapon = blackboard.getArmory().getCurrWeapon();
	if (weapon == nullptr) {
		return true;
	}
	if (!weapon->isDeployed()&& weapon->getMinDeployRange() < dist) {
		return weapon->getDeployer()->execute(blackboard);
	}
	if (dur-- < 1 || targetDestroyed() || weapon->isClipEmpty()) {
		return true;
	}
	edict_t* targetEnt = getTargetedEdict();
	Vector eyes = self->getEyesPos();
	Buttons& buttons = blackboard.getButtons();
	edict_t* selfEnt = self->getEdict();
	WeaponFunction* weapFunc = weapon->chooseWeaponFunc(selfEnt, dist);
	if (weapFunc->isExplosive()) {
		targetLoc = targetEnt->GetCollideable()->GetCollisionOrigin();
	}
	if (weapFunc->isMelee() && !crouch) {
		moveCtx->setGoal(
				(targetLoc - self->getCurrentPosition()).Normalized()
						* (dist - 25.0f) + self->getCurrentPosition());
		CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
		moveCtx->traceMove(crouch);
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
	if (weapFunc->isMelee() || blackboard.getAimAccuracy(targetLoc)
			> 1.0f - 10.0f / MAX(dist, 0.1f)) {
		if ((!UTIL_IsVisible(targetLoc, blackboard, targetEnt)
				|| (weapFunc->isMelee() && eyes.z - targetLoc.z > 20.0f))
				&& !crouch) {
			// try crouching
			crouch = true;
		}
	}
	weapFunc->attack(buttons, dist);
	return false;
}

bool AttackAction::goalComplete() {
	if (targetDestroyed()) {
		blackboard.setBlocker(nullptr);
		abort();
		return true;
	}
	return false;
}


void AttackAction::abort() {
	Weapon* weapon = blackboard.getArmory().getCurrWeapon();
	if (weapon != nullptr) {
		weapon->undeploy(blackboard);
	}
	moveCtx->stop();
}

bool AttackAction::targetDestroyed() const {
	return blackboard.getBlocker() == nullptr
			|| BaseEntity(blackboard.getBlocker()).isDestroyedOrUsed()
			|| blackboard.getBlocker()->GetCollideable()->GetCollisionOrigin().DistTo(
					blackboard.getSelf()->getCurrentPosition()) > 130.0f;
}

edict_t* AttackAction::getTargetedEdict() const {
	return blackboard.getBlocker();
}
