#include "BotBuilder.h"

#include "Blackboard.h"
#include "Bot.h"
#include "World.h"
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
#include <util/BasePlayer.h>

BotBuilder::BotBuilder(GameManager* objectives): objectives(objectives) {
	addCommand("mybot_add_bot",  "Add a bot to the server", &BotBuilder::addBot);
	addCommand("mybot_add_all_bots",  "Fill server with bots", &BotBuilder::addAllBots);
	extern ICvar* cVars;
	teamPlay = cVars->FindVar("mp_teamplay")->GetBool();
}

BotBuilder::~BotBuilder() {
	cmdCallbacks.RemoveAll();
	commands.PurgeAndDeleteElements();
	if (hidingSpotSelector != nullptr) {
		delete hidingSpotSelector;
	}
}

void BotBuilder::addBot(const CCommand &command) const {
	static int botCount = 0;
	extern IBotManager *botmanager;
	int team = rand() % 2 + 2;

	botCount %= 32;
	if (command.ArgC() > 2) {
		team = atoi(command.Arg(2)) % 2 + 2;
	}
	edict_t *pEdict = botmanager->CreateBot(
			command.ArgC() > 1 ?
					command.Arg(1) :
					(CUtlString("Bot") + botCount++).Get());
	if (pEdict == nullptr) {
		Warning("IBotManager::CreateBot failed");
		botCount--;
		return;
	}
	extern IPlayerInfoManager *playerinfomanager;
	playerinfomanager->GetPlayerInfo(pEdict)->ChangeTeam(team);
	modHandleCommand(command, build(pEdict));
}

void BotBuilder::onNavMeshLoad() {
	hidingSpotSelector = new HidingSpotSelector();
	SnipeAction::setSpotSelector(hidingSpotSelector);
}

class SwitchToBestInRangeWeaponAction: public SwitchToBestLoadedWeaponAction {
public:
	SwitchToBestInRangeWeaponAction(Blackboard &blackboard) :
			SwitchToBestLoadedWeaponAction(blackboard) {
		effects = { WorldProp::WEAPON_IN_RANGE, true };
	}
};

Bot* BotBuilder::build(edict_t* ent) const {
	Bot* bot = new Bot(ent);
	Blackboard *blackboard = new Blackboard(bot, buildEntity(ent));
	blackboard->setNavigator(new Navigator(*blackboard));
	bot->setBlackboard(blackboard);
	World* world = buildWorld();
	world->reset();
	bot->setInGame(world->getState(WorldProp::ROUND_STARTED));
	bot->setWorld(world);
	Planner *planner = new Planner(world->getStates(), *blackboard);
	planner->addAction<ReloadWeaponAction>(0.85f);
	planner->addAction<AttackAction>(0.84f);
	planner->addAction<SwitchWeaponAction>(0.82f);
	planner->addAction<FindCoverAction>(0.81f);
	planner->addAction<SnipeAction>(0.4f, 0.4f);
	planner->addAction<SwitchToDesiredWeaponAction>(0.0f);
	planner->addAction<SwitchToBestLoadedWeaponAction>(0.0f);
	planner->addAction<SwitchToBestInRangeWeaponAction>(0.0f);
	updatePlanner(*planner, *blackboard);
	bot->setPlanner(planner);
	initWeapons(blackboard->getArmory().getWeaponFactory());
	return bot;
}

BasePlayer* BotBuilder::buildEntity(edict_t* ent) const {
	return new BasePlayer(ent);
}

void BotBuilder::addAllBots(const CCommand &command) const {
	extern CGlobalVars *gpGlobals;
	for (int i = 0; i < gpGlobals->maxClients - 2; i++) {
		const char* team = i % 2 == 0 ? "2" : "3";
		if (command.ArgC() > 1) {
			const char* args[] = {"addbot", "Bot", team, command.Arg(1) };
			addBot(CCommand(4, args));
		} else {
			const char* args[] = {"addbot", "Bot", team };
			addBot(CCommand(3, args));
		}
	}
}

void BotBuilder::addCommand(const char* name, const char* description, CmdFuncPtr ptr) {
	commands.AddToTail(new ConCommand(name, this, description));
	cmdCallbacks.Insert(commands[commands.Tail()]->GetName(), ptr);
}
