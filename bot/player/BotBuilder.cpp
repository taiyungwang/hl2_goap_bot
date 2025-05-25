#include "BotBuilder.h"

#include "Bot.h"
#include "World.h"
#include "HidingSpotSelector.h"
#include <move/Navigator.h>
#include <goap/action/KillAction.h>
#include <goap/action/FindCoverAction.h>
#include <goap/action/ReloadWeaponAction.h>
#include <goap/action/SnipeAction.h>
#include <goap/action/FindCoverFromGrenadesAction.h>
#include <goap/action/SwitchWeaponAction.h>
#include <goap/GoalManager.h>

extern IVEngineServer* engine;
extern CGlobalVars *gpGlobals;

static ConVar minPlayers("mybot_min_players", "-1");

BotBuilder::BotBuilder(CommandHandler& commandHandler): commandHandler(commandHandler) {
	addCommand("mybot_add_bot", "Add a bot to the server", &BotBuilder::addBot);
	addCommand("mybot_add_all_bots", "Fill server with bots", &BotBuilder::addAllBots);
	addCommand("mybot_kick_all_bots_except", "Kicks all bots except bot with given name",
			&BotBuilder::kickAllExcept);
}

BotBuilder::~BotBuilder() {
	cmdCallbacks.clear();
	commands.clear();
	if (hidingSpotSelector != nullptr) {
		delete hidingSpotSelector;
	}
}

void BotBuilder::addBot(const CCommand &command) {
	static int botCount = 0;
	extern IBotManager *botmanager;
	int team = 0;
	extern ICvar* g_pCVar;
	if (teamPlay || g_pCVar->FindVar("mp_teamplay")->GetBool()) {
		auto count = Player::getTeamCount();
		team = std::get<0>(count) > std::get<1>(count) ? 3 : 2;
	}
	botCount %= gpGlobals->maxClients;
	if (command.ArgC() > 2) {
		team = (atoi(command.Arg(2)) - 1) % 2 + 2;
	}
	edict_t *pEdict = botmanager->CreateBot(
			command.ArgC() > 1 ?
					command.Arg(1) :
					(std::string("Bot") + std::to_string(botCount++)).c_str());
	if (pEdict == nullptr) {
		Warning("IBotManager::CreateBot failed");
		botCount--;
		return;
	}
	extern IPlayerInfoManager *playerinfomanager;
	playerinfomanager->GetPlayerInfo(pEdict)->ChangeTeam(team);
	modHandleCommand(command, build(pEdict));
}

void BotBuilder::kickAllExcept(const CCommand &command) {
	for (auto player : Player::getPlayers()) {
		if (dynamic_cast<Bot*>(player.second) != nullptr
				&& std::string(player.second->getName()) != command.Arg(1)) {
			engine->ServerCommand((std::string("kickid ") + std::to_string(player.second->getUserId())
					+ "\n").c_str());
		}
	}
}

Bot* BotBuilder::build(edict_t* ent) {
	Bot* bot = new Bot(ent, weaponBuilders, commandHandler,
			messages);
	bot->setNavigator<Navigator>();
	World* world = buildWorld();
	world->reset();
	bot->setWorld(world);
	GoalManager *planner = new GoalManager(world->getStates(), bot);
	planner->addAction<FindCoverFromGrenadesAction>(0.95f);
	planner->addAction<ReloadWeaponAction>(0.86f);
	planner->addAction<KillAction>(0.85f);
	planner->addAction<SwitchWeaponAction>(0.83f);
	planner->addAction<FindCoverAction>(0.81f);
	planner->addAction<SnipeAction>(0.7f);
	updatePlanner(*planner, bot);
	bot->setPlanner(planner);
	return modBuild(bot);
}

void BotBuilder::onFrame() {
	modOnFrame();
	if (minPlayers.GetInt() < 0) {
		return;
	}
	auto count = Player::getTeamCount();
	int botsToAdd = MIN(minPlayers.GetInt(), gpGlobals->maxClients - 1) - std::get<0>(count) - std::get<1>(count);
	if (botsToAdd > 0) {
		for (int i = 0; i < botsToAdd; i++) {
			const char *args[] = { "mybot_add_bot" };
			addBot(CCommand(1, args));
		}
	} else if (botsToAdd < 0) {
		for (auto player : Player::getPlayers()) {
			int team = player.second->getTeam();
			if (team < 2) {
				continue;
			}
			bool team3Less = std::get<1>(count) < std::get<0>(count);
			if ((team == 2 && team3Less) || (team == 3 && !team3Less)) {
				engine->ServerCommand((std::string("kickid ") + std::to_string(player.second->getUserId())
						+ "\n").c_str());
				if (team == 2) {
					std::get<0>(count)--;
				} else {
					std::get<1>(count)--;
				}
			}
			if (std::get<0>(count) + std::get<1>(count) == minPlayers.GetInt()) {
				break;
			}
		}
	}
}

void BotBuilder::addAllBots(const CCommand &command) {
	extern CGlobalVars *gpGlobals;
	int botsToAdd = gpGlobals->maxClients - 2 - Player::getPlayers().size();
	for (int i = 0; i < botsToAdd; i++) {
		const char* team = i % 2 == 0 ? "2" : "3";
		std::string name = std::string("Bot") + std::to_string(i);
		if (command.ArgC() > 1) {
			const char* args[] = {"addbot", name.c_str(), team, command.Arg(1) };
			addBot(CCommand(4, args));
		} else {
			const char *args[] = { "addbot", name.c_str(), team };
			addBot(CCommand(3, args));
		}
	}
}

void BotBuilder::addCommand(const char* name, const char* description, CmdFuncPtr ptr) {
	commands.push_back(std::make_unique<ConCommand>(name, this, description));
	cmdCallbacks[commands.back()->GetName()] = ptr;
}
