#include "BotBuilder.h"

#include "Blackboard.h"
#include "Bot.h"
#include "World.h"
#include "VTableHook.h"
#include "HidingSpotSelector.h"
#include <move/Navigator.h>
#include <goap/action/AttackAction.h>
#include <goap/action/FindCoverAction.h>
#include <goap/action/ReloadWeaponAction.h>
#include <goap/action/SnipeAction.h>
#include <goap/action/SwitchToDesiredWeaponAction.h>
#include <goap/action/SwitchToBestLoadedWeaponAction.h>
#include <goap/action/SwitchWeaponAction.h>
#include <goap/Planner.h>
#include <util/BaseEntity.h>

BotBuilder::~BotBuilder() {
	delete command;
	if (enableHook) {
		unhookPlayerRunCommand();
	}
	if (hidingSpotSelector != nullptr) {
		delete hidingSpotSelector;
	}
}

void BotBuilder::CommandCallback(const CCommand &command) {
	static int botCount = 0;
	extern IBotManager *botmanager;
	int team = rand() % 2 + 2;
	edict_t *pEdict = botmanager->CreateBot(
			command.ArgC() > 1 ?
					command.Arg(1) :
					(CUtlString("Bot") + botCount++).Get());
	if (pEdict == nullptr) {
		Warning("IBotManager::CreateBot failed");
		botCount--;
		return;
	}
	if (command.ArgC() > 2) {
		team = atoi(command.Arg(2)) % 2 + 2;
	}
	extern IPlayerInfoManager *playerinfomanager;
	playerinfomanager->GetPlayerInfo(pEdict)->ChangeTeam(team);
	modHandleCommand(command);
}

void BotBuilder::onNavMeshLoad() {
	hidingSpotSelector = new HidingSpotSelector();
	SnipeAction::setSpotSelector(hidingSpotSelector);
}


Bot* BotBuilder::build(const CUtlMap<int, Player*>& players,
		edict_t* ent) const {
	Bot* bot = new Bot(ent);
	if (enableHook) {
		hookPlayerRunCommand(ent);
	}
	Blackboard *blackboard = new Blackboard(players, bot, buildEntity(ent));
	blackboard->setNavigator(new Navigator(*blackboard));
	bot->setBlackboard(blackboard);
	World* world = buildWorld();
	world->reset();
	bot->setInGame(world->getState(WorldProp::ROUND_STARTED));
	bot->setWorld(world);
	bot->setHookEnabled(enableHook);
	Planner *planner = new Planner(world->getStates(), *blackboard);
	planner->addAction<SwitchToDesiredWeaponAction>(0.0f);
	planner->addAction<FindCoverAction>(0.1f);
	planner->addAction<AttackAction>(0.9f);
	planner->addAction<SwitchWeaponAction>(0.81f);
	planner->addAction<SwitchToBestLoadedWeaponAction>(0.0f);
	planner->addAction<ReloadWeaponAction>(0.82f);
	planner->addAction<SnipeAction>(0.4f);
	updatePlanner(*planner, *blackboard);
	bot->setPlanner(planner);
	initWeapons(blackboard->getArmory().getWeaponFactory());
	bot->setDesiredClassId(classType);
	update(bot);
	return bot;
}

EntityInstance* BotBuilder::buildEntity(edict_t* ent) const {
	return new BaseEntity(ent);
}
