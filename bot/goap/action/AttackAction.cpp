#include "AttackAction.h"

#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <player/Vision.h>
#include <weapon/Arsenal.h>
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
	Bot* self = blackboard.getSelf();
	Vector targetLoc = self->getViewTarget();
	float dist = targetLoc.DistTo(self->getCurrentPosition());
	Weapon* weapon = self->getArsenal().getCurrWeapon();
	if (weapon == nullptr || dur-- < 1 || weapon->isClipEmpty()) {
		return true;
	}
	edict_t* selfEnt = self->getEdict();
	WeaponFunction* weapFunc = weapon->chooseWeaponFunc(selfEnt, dist);
	if (!weapFunc->isInRange(dist)) {
		return true;
	}
	if (weapon->getDeployer() != nullptr && weapon->getMinDeployRange() < dist) {
		if (!weapon->getDeployer()->execute(blackboard)) {
			return false;
		}
		if (!weapon->isDeployed()) {
			return true;
		}
	}
	edict_t* targetEnt = getTargetedEdict();
	if (targetEnt == nullptr || targetDestroyed()) {
		return true;
	}
	if (weapFunc->isExplosive()) {
		targetLoc = targetEnt->GetCollideable()->GetCollisionOrigin();
	}
	Vector eyes = self->getEyesPos();
	if (adjustAim) {
		targetLoc = weapFunc->getAim(targetLoc, eyes);
		adjustAim = false;
	}
	bool crouch = weapFunc->isMelee()
			&& eyes.z - targetLoc.z > 20.0f
			&& (moveCtx->getTraceResult().startsolid
				|| dist < HalfHumanWidth * 2.0f);
	Buttons& buttons = blackboard.getButtons();
	self->setWantToListen(false);
	self->setViewTarget(targetLoc);
	extern ConVar mybot_debug;
	if (crouch) {
		buttons.hold(IN_DUCK);
	} else if (weapFunc->isMelee()) {
		moveCtx->setGoal(targetLoc);
		CNavArea* area = Navigator::getArea(selfEnt, self->getTeam());
		moveCtx->traceMove(crouch);
		moveCtx->move(area == nullptr ? NAV_MESH_INVALID: area->GetAttributes());
		// can't sprint and attack
		if (!moveCtx->getTraceResult().startsolid) {
			buttons.hold(IN_SPEED);
		}
	}
	if (mybot_debug.GetBool()) {
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(eyes, targetLoc, 255, 0, 255, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
		debugoverlay->AddLineOverlay(eyes, eyes + self->getFacing() * dist, 0, 255, 0, true,
		NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	weapFunc->attack(buttons, dist);
	return false;
}

bool AttackAction::goalComplete() {
	abort();
	blackboard.setBlocker(nullptr);
	return targetDestroyed();
}


void AttackAction::abort() {
	auto self = blackboard.getSelf();
	self->setAiming(false);
	Weapon* weapon = self->getArsenal().getCurrWeapon();
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
