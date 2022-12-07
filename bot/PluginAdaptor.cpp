#include "PluginAdaptor.h"

#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/hl2dm/weapon/HL2DMArsenalBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "mods/dod/weapon/DODArsenalBuilder.h"
#include "goap/action/SnipeAction.h"
#include "player/Bot.h"
#include "player/GameManager.h"
#include "player/HidingSpotSelector.h"
#include <nav_mesh/nav_entities.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <iplayerinfo.h>

ConVar r_visualizetraces("r_visualizetraces", "0", FCVAR_CHEAT);
ConVar mybot_debug("my_bot_debug", "0");
ConVar mybot_var("mybot_var", "0.5");
ConVar minPlayers("mybot_min_players", "-1");
static ConVar mybot_dod_playerruncommand_offset("mybot_dod_playerruncommand_offset",
#ifndef _WIN32
	"419"
#else
	"418"
#endif
);

CNavMesh* TheNavMesh = nullptr;

CUtlMap<int, NavEntity*> blockers;

extern IVEngineServer* engine;
extern IGameEventManager2* gameeventmanager;

PluginAdaptor::PluginAdaptor() {
	// TODO: consider moving constructor initializations into init callback.
	gameeventmanager->AddListener(this, "nav_generate", true);
	TheNavMesh = new CNavMesh;
	botBuilder = nullptr;
	SetDefLessFunc(blockers);
	char gameDir[MAX_PATH];
	engine->GetGameDir(gameDir, MAX_PATH);
	std::string modPath(gameDir);
	auto lastDelim = modPath.rfind('/');
	if (lastDelim == modPath.npos) {
		lastDelim = modPath.rfind('\\');
	}
	modPath = modPath.substr(lastDelim + 1);
	// TODO: make mod checking more stringent.
	if (modPath == "hl2mp") {
		arsenalBuilder = std::make_shared<HL2DMArsenalBuilder>();
		botBuilder = new HL2DMBotBuilder(commandHandler, *arsenalBuilder.get());
		TheNavMesh->addPlayerSpawnName("info_player_start");
	} else if (modPath == "dod") {
		gameManager = new DODObjectives();
		arsenalBuilder = std::make_shared<DODArsenalBuilder>();
		botBuilder = new DODBotBuilder(gameManager, commandHandler, *arsenalBuilder.get());
		hookOffset = mybot_dod_playerruncommand_offset.GetInt();
		TheNavMesh->addPlayerSpawnName("info_player_axis");
		TheNavMesh->addPlayerSpawnName("info_player_allies");
	} else {
		Msg("Mod not supported, %s.\n", modPath.c_str());
	}
}

PluginAdaptor::~PluginAdaptor() {
	delete TheNavMesh;
	TheNavMesh = nullptr;
	if (botBuilder != nullptr) {
		delete botBuilder;
		botBuilder = nullptr;
	}
	gameeventmanager->RemoveListener(this);
}

void PluginAdaptor::levelInit(const char* mapName) {
	navMeshLoadAttempted = false;
}

void PluginAdaptor::gameFrame(bool simulating) {
	if (!simulating) {
		return;
	}
	if (!navMeshLoadAttempted) {
		if (TheNavMesh->Load() == NAV_OK) {
			hidingSpotSelector = std::make_shared<HidingSpotSelector>(commandHandler);
			SnipeAction::setSpotSelector(hidingSpotSelector.get());
			Msg("Loaded Navigation mesh.\n");
		}
		navMeshLoadAttempted = true;
	}
	const auto& players = Player::getPlayers();
	newPlayers.remove_if([players, this](edict_t *ent) {
		if (players.find(engine->IndexOfEdict(ent)) == players.end()) {
			new Player(ent, arsenalBuilder->build());
			return true;
		}
		return false;
	});
	if (navMeshLoadAttempted && TheNavMesh != nullptr) {
		CUtlLinkedList<unsigned short> toRemove;
		FOR_EACH_MAP_FAST(blockers, i) {
			edict_t* blocker = blockers[i]->getEntity();
			if (blocker->IsFree()) {
				toRemove.AddToTail(i);
				blockers[i]->InputDisable();
				delete blockers[i];
			} else {
				blockers[i]->InputEnable();
			}
		}
		FOR_EACH_LL(toRemove, i) {
			blockers.RemoveAt(toRemove[i]);
		}
		TheNavMesh->Update();
		botBuilder->onFrame();

	}
	for (auto player: Player::getPlayers()) {
		player.second->think();
	}
	if (minPlayers.GetInt() < 0) {
		return;
	}
	auto count = Player::getTeamCount();
	extern CGlobalVars *gpGlobals;
	int botsToAdd = MIN(minPlayers.GetInt(), gpGlobals->maxClients - 1) - std::get<0>(count) - std::get<1>(count);
	if (botsToAdd > 0) {
		for (int i = 0; i < botsToAdd; i++) {
			const char *args[] = { "mybot_add_bot" };
			botBuilder->CommandCallback(CCommand(1, args));
		}
	} else if (botsToAdd < 0) {
		for (auto player : Player::getPlayers()) {
			Bot *bot = dynamic_cast<Bot*>(player.second);
			if (bot != nullptr) {
				int team = bot->getTeam();
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
			}
			if (std::get<0>(count) + std::get<1>(count) == minPlayers.GetInt()) {
				break;
			}
		}
	}
}

void PluginAdaptor::clientDisconnect(edict_t *pEntity) {
	delete Player::getPlayer(pEntity);
}

void PluginAdaptor::levelShutdown() {
	auto& players = Player::getPlayers();
	while(players.size() > 0) {
		delete players.begin()->second;
	}
	blockers.PurgeAndDeleteElements();
	if (gameManager != nullptr) {
		gameManager->endRound();
	}
	if (hidingSpotSelector) {
		hidingSpotSelector->save();
	}
	hidingSpotSelector = nullptr;
}

void PluginAdaptor::FireGameEvent(IGameEvent* event) {
// TODO: this doesn't really work.
	blockers.PurgeAndDeleteElements();
}
