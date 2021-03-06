#include "Bot.h"

#include "Blackboard.h"
#include "Vision.h"
#include "World.h"
#include <event/EventInfo.h>
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
#include <move/Navigator.h>
#include <move/RotationManager.h>
#include <weapon/Armory.h>
#include <weapon/Weapon.h>
#include <nav_mesh/nav_area.h>
#include <util/SimpleException.h>
#include <util/BasePlayer.h>
#include <ivdebugoverlay.h>
#include <in_buttons.h>

PlayerClasses Bot::CLASSES = nullptr;

ConVar mybot_rot_speed("mybot_rot_speed", "0.5", 0,
		"determines rotational acceleration rate in degrees");
ConVar mybot_mimic("mybot_mimic", "0");

Bot::~Bot() {
	delete blackboard;
	delete world;
	delete planner;
	if (playerClassVar != nullptr) {
		delete playerClassVar;
	}
}

void Bot::think() {
	try {
		CBotCmd& cmd = blackboard->getCmd();
		if (isDead()) {
			despawn();
		} else if (getPlayerClass() != desiredClassId) {
			extern IServerPluginHelpers* helpers;
			helpers->ClientCommand(getEdict(), (*CLASSES)[getTeam() - 2][desiredClassId]);
		} else {
			if ((resetPlanner || world->think(*blackboard))
					&& !blackboard->isOnLadder()) {
				planner->resetPlanning(false);
				resetPlanner = false;
			}
			vision.updateVisiblity(*blackboard);
			if (inGame) {
				cmd.Reset();
				Armory& amory = blackboard->getArmory();
				amory.update(*blackboard);
				int currentWeapon = amory.getCurrWeaponIdx();
				if (currentWeapon > 0
						&& amory.getWeapon(currentWeapon)->isOutOfAmmo(getEdict())) {
					world->updateState(WorldProp::USING_BEST_WEAP, false);
				}
				planner->execute();
				extern ConVar mybot_debug;
				if (mybot_debug.GetBool()) {
					extern IVDebugOverlay *debugoverlay;
					debugoverlay->AddLineOverlay(getEyesPos(), blackboard->getViewTarget(), 0,
							255, 0, true,
							NDEBUG_PERSIST_TILL_NEXT_SERVER);
				}
				VectorAngles(blackboard->getViewTarget() - getEyesPos(), cmd.viewangles);
				rotation.getUpdatedPosition(cmd.viewangles, getFacingAngle(),
						mybot_rot_speed.GetFloat());
				if (cmd.weaponselect != 0) {
					world->updateState(WorldProp::USING_BEST_WEAP, true);
				}
			} else {
				blackboard->getButtons().tap(IN_ATTACK);
			}
		}
		cmd.buttons = blackboard->getButtons().getPressed();
		extern CGlobalVars *gpGlobals;
		cmd.tick_count = gpGlobals->tickcount;
		if (mybot_mimic.GetBool()) {
			auto& players = Player::getPlayers();
			cmd = players[players.Find(1)]->getInfo()->GetLastUserCommand();
		}

		extern IBotManager *botmanager;
		if (!hookEnabled) {
			botmanager->GetBotController(getEdict())->RunPlayerMove(&cmd);
		}
	} catch (const Exception& e) {
		Error("%s: %s", getName(), e.what());
	}
}

bool Bot::handle(EventInfo* event) {
	CUtlString name(event->getName());
	int eventUserId = event->getInt("userid");
	if (name == "player_death" || name == "player_disconnect") {
		auto targeted = blackboard->getTargetedPlayer();
		extern IVEngineServer* engine;
		if (targeted != nullptr && eventUserId == targeted->getUserId()) {
			blackboard->setTargetedPlayer(nullptr);
		}
		return false;
	}
	// bot owns this event.
	if (eventUserId == getUserId()) {
		if (name == "player_spawn") {
			despawn();
			CNavArea* area = blackboard->getNavigator()->getLastArea();
			if (area != nullptr) {
				area->IncreaseDanger(getTeam(), 1.0f);
			}
			resetPlanner = inGame = true;
			blackboard->reset();
			world->reset();
			return true;
		}
		if (name == "player_death" && inGame) {
			despawn();
			return false;
		}
		if (name == "player_hurt") {
			int attacker = event->getInt("attacker");
			if (event->getInt("attacker") == getUserId()) {
				return false;
			}
			world->updateState(WorldProp::HURT, true);
			auto& players = Player::getPlayers();
			FOR_EACH_MAP_FAST(players, i) {
				if (players[i]->getUserId() == attacker) {
					blackboard->setViewTarget(players[i]->getEyesPos());
					blackboard->setTargetedPlayer(players[i]);
					break;
				}
			}
			return true;
		}
	}
	return false;
}

void Bot::setWorld(World* world) {
	this->world = world;
}

CBotCmd* Bot::getCmd() const {
	return blackboard == nullptr ? nullptr : &blackboard->getCmd();
}

int Bot::getPlayerClass() const {
	return playerClassVar == nullptr ? -1 : playerClassVar->getPlayerClass();
}


void Bot::despawn() {
	inGame = false;
	planner->resetPlanning(true);
	blackboard->getButtons().tap(IN_ATTACK);
}
