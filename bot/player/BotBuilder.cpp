#include "BotBuilder.h"

#include "Blackboard.h"
#include "Bot.h"
#include "World.h"
#include "HidingSpotSelector.h"
#include <move/Navigator.h>
#include <goap/action/KillAction.h>
#include <goap/action/FindCoverAction.h>
#include <goap/action/ReloadWeaponAction.h>
#include <goap/action/SnipeAction.h>
#include <goap/action/SwitchToDesiredWeaponAction.h>
#include <goap/action/SwitchToBestLoadedWeaponAction.h>
#include <goap/action/FindCoverFromGrenadesAction.h>
#include <goap/action/SwitchWeaponAction.h>
#include <goap/GoalManager.h>
#include <weapon/ArsenalBuilder.h>
#include <util/BasePlayer.h>

extern IVEngineServer* engine;

BotBuilder::BotBuilder(GameManager* objectives, CommandHandler& commandHandler,
		const ArsenalBuilder& arsenalBuilder): objectives(objectives),
				commandHandler(commandHandler),
				arsenalBuilder(arsenalBuilder) {
	addCommand("mybot_add_bot", "Add a bot to the server", &BotBuilder::addBot);
	addCommand("mybot_add_all_bots", "Fill server with bots", &BotBuilder::addAllBots);
	addCommand("mybot_kick_all_bots", "Kicks all bots", &BotBuilder::kickAllBots);
	addCommand("mybot_kick_all_bots_except", "Kicks all bots except bot with given name",
			&BotBuilder::kickAllExcept);
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

void BotBuilder::addBot(const CCommand &command) {
	static int botCount = 0;
	extern IBotManager *botmanager;
	int team = rand() % 2 + 2;
	botCount %= 32;
	if (command.ArgC() > 2) {
		team = (atoi(command.Arg(2)) - 1) % 2 + 2;
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

void BotBuilder::kickAllBots(const CCommand &command) {
	for (auto player : Player::getPlayers()) {
		if (dynamic_cast<Bot*>(player.second) != nullptr) {
			engine->ServerCommand((CUtlString("kickid ") + player.second->getUserId() + "\n").Get());
		}
	}
}

void BotBuilder::kickAllExcept(const CCommand &command) {
	for (auto player : Player::getPlayers()) {
		if (dynamic_cast<Bot*>(player.second) != nullptr
				&& CUtlString(player.second->getName()) != command.Arg(1)) {
			engine->ServerCommand((CUtlString("kickid ") + player.second->getUserId() + "\n").Get());
		}
	}
}

class SwitchToBestInRangeWeaponAction: public SwitchToBestLoadedWeaponAction {
public:
	SwitchToBestInRangeWeaponAction(Blackboard &blackboard) :
			SwitchToBestLoadedWeaponAction(blackboard) {
		effects = { WorldProp::WEAPON_IN_RANGE, true };
	}
};

Bot* BotBuilder::build(edict_t* ent) {
	Bot* bot = new Bot(ent, arsenalBuilder.build(), commandHandler,
			voiceMessageSender);
	Blackboard *blackboard = new Blackboard(bot, buildEntity(ent));
	blackboard->setNavigator(new Navigator(*blackboard));
	bot->setBlackboard(blackboard);
	World* world = buildWorld();
	world->reset();
	bot->setInGame(world->getState(WorldProp::ROUND_STARTED));
	bot->setWorld(world);
	GoalManager *planner = new GoalManager(world->getStates(), *blackboard);
	planner->addAction<FindCoverFromGrenadesAction>(0.95f);
	planner->addAction<ReloadWeaponAction>(0.85f);
	planner->addAction<KillAction>(0.84f);
	planner->addAction<SwitchWeaponAction>(0.82f);
	planner->addAction<FindCoverAction>(0.81f);
	planner->addAction<SnipeAction>(0.7f, 0.7f);
	planner->addAction<SwitchToDesiredWeaponAction>(0.0f);
	planner->addAction<SwitchToBestLoadedWeaponAction>(0.0f);
	planner->addAction<SwitchToBestInRangeWeaponAction>(0.0f);
	updatePlanner(*planner, *blackboard);
	bot->setPlanner(planner);
	return modBuild(bot);
}

BasePlayer* BotBuilder::buildEntity(edict_t* ent) const {
	return new BasePlayer(ent);
}

void BotBuilder::addAllBots(const CCommand &command) {
	extern CGlobalVars *gpGlobals;
	int botsToAdd = gpGlobals->maxClients - 2 - Player::getPlayers().size();
	for (int i = 0; i < botsToAdd; i++) {
		const char* team = i % 2 == 0 ? "2" : "3";
		CUtlString name("Bot");
		name += i;
		if (command.ArgC() > 1) {
			const char* args[] = {"addbot", name.Get(), team, command.Arg(1) };
			addBot(CCommand(4, args));
		} else {
			const char* args[] = {"addbot", name.Get(), team };
			addBot(CCommand(3, args));
		}
	}
}

void BotBuilder::addCommand(const char* name, const char* description, CmdFuncPtr ptr) {
	commands.AddToTail(new ConCommand(name, this, description));
	cmdCallbacks.Insert(commands[commands.Tail()]->GetName(), ptr);
}
