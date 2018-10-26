#include "Bot.h"

#include "Blackboard.h"
#include "Vision.h"
#include "World.h"
#include <event/EventInfo.h>
#include <goap/Planner.h>
#include <goap/action/GoToAction.h>
#include <move/Navigator.h>
#include <weapon/Armory.h>
#include <weapon/Weapon.h>
#include <navmesh/nav_mesh.h>
#include <util/Exception.h>
#include <util/EntityInstance.h>
#include <in_buttons.h>

PlayerClasses Bot::CLASSES = nullptr;

Bot::Bot(edict_t* ent) :
		EventHandler(), Player(ent) {
	extern IPlayerInfoManager *playerinfomanager;
}

Bot::~Bot() {
	delete blackboard;
	delete world;
	delete planner;
	if (playerClassVar != nullptr) {
		delete playerClassVar;
	}
}

void Bot::think() {
	CBotCmd& cmd = blackboard->getCmd();
	if (isDead()) {
		targeter.reset();
		Vector pos = getCurrentPosition();
		extern CNavMesh* TheNavMesh;
		TheNavMesh->IncreaseDangerNearby(getTeam(), 5.0f,
				Navigator::getCurrentArea(pos), pos, 100.0f);
		inGame = false;
		planner->resetPlanning(true);
		blackboard->getButtons().tap(IN_ATTACK);
	} else if (getPlayerClass() != desiredClassId) {
		extern IServerPluginHelpers* helpers;
		helpers->ClientCommand(getEdict(), (*CLASSES)[getTeam() - 2][desiredClassId]);
	} else {
		if ((resetPlanner || world->think(*blackboard))
				&& !blackboard->isOnLadder()) {
			planner->resetPlanning(false);
			resetPlanner = false;
		}
		Vision::updateVisiblity(*blackboard);
		if (inGame) {
			cmd.Reset();
			Armory& amory = blackboard->getArmory();
			try {
				amory.update(*blackboard);
			} catch (const Exception& ex) {
				Warning("Exception caught while updating weapons: %s\n",
						ex.what());
			}
			int currentWeapon = amory.getCurrWeaponIdx();
			if (currentWeapon > 0
					&& amory.getWeapon(currentWeapon)->isOutOfAmmo(getEdict())) {
				world->updateState(WorldProp::USING_BEST_WEAP, false);
			}
			planner->execute();
			cmd.viewangles = getAngle();
			QAngle angle;
			VectorAngles(blackboard->getViewTarget() - getEyesPos(), angle);
			targeter.updateAngle(cmd.viewangles, angle);
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
	extern IBotManager *botmanager;
	if (!hookEnabled) {
		botmanager->GetBotController(getEdict())->RunPlayerMove(&cmd);
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
			resetPlanner = inGame = true;
			blackboard->reset();
			world->reset();
			return true;
		}
		if (name == "player_hurt") {
			int attacker = event->getInt("attacker");
			if (event->getInt("attacker") == getUserId()) {
				return false;
			}
			world->updateState(WorldProp::HURT, true);
			auto& players = blackboard->getPlayers();
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

