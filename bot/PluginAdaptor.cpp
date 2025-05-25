#include "PluginAdaptor.h"

#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "goap/action/SnipeAction.h"
#include "player/Bot.h"
#include "player/HidingSpotSelector.h"
#include <nav_mesh/nav_entities.h>
#include <util/common_util.h>
#include <eiface.h>

ConVar mybot_debug("my_bot_debug", "0");
ConVar mybot_var("mybot_var", "0.5");
static ConVar playerruncommand_offset("mybot_playerruncommand_offset", "-1");

CNavMesh* TheNavMesh = nullptr;

std::unordered_map<int, CFuncNavBlocker> blockers;

extern IVEngineServer* engine;

int PluginAdaptor::getPlayerruncommandOffset() {
	return playerruncommand_offset.GetInt();
}

PluginAdaptor::PluginAdaptor() {
	// TODO: consider moving constructor initializations into init callback.
	listenForGameEvent({"nav_generate"});
	TheNavMesh = new CNavMesh;
	botBuilder = nullptr;
	// TODO: make mod checking more stringent.
	if (isGameName("hl2mp")) {
		botBuilder = new HL2DMBotBuilder(commandHandler);
		TheNavMesh->addPlayerSpawnName("info_player_start");
	} else if (isGameName("dod")) {
		botBuilder = new DODBotBuilder(commandHandler);
		playerruncommand_offset.SetValue(
#ifdef PLATFORM_WINDOWS_PC
	"425"
#else
	"426"
#endif
				);
		TheNavMesh->addPlayerSpawnName("info_player_axis");
		TheNavMesh->addPlayerSpawnName("info_player_allies");
	} else {
		Msg("Mod not supported, \n");
	}
}

PluginAdaptor::~PluginAdaptor() {
	delete TheNavMesh;
	TheNavMesh = nullptr;
	if (botBuilder != nullptr) {
		delete botBuilder;
		botBuilder = nullptr;
	}
}

void PluginAdaptor::levelInit(const char* mapName) {
	navMeshLoadAttempted = false;
}

void PluginAdaptor::gameFrame(bool isSimulating) {
	if (!isSimulating) {
		return;
	}
	if (!navMeshLoadAttempted) {
		if (TheNavMesh->Load() == NAV_OK) {
			hidingSpotSelector = std::make_unique<HidingSpotSelector>(commandHandler);
			SnipeAction::setSpotSelector(hidingSpotSelector.get());
			Msg("Loaded Navigation mesh.\n");
		}
		navMeshLoadAttempted = true;
	}
	const auto& players = Player::getPlayers();
	newPlayers.remove_if([players, this](edict_t *ent) {
		if (players.find(engine->IndexOfEdict(ent)) == players.end()) {
			new Player(ent, botBuilder->getWeaponBuilders());
			return true;
		}
		return false;
	});
	if (TheNavMesh != nullptr) {
		for (auto itr = blockers.begin(); itr != blockers.end();) {
			if (itr->second.getEntity()->IsFree()) {
				itr->second.InputDisable();
				itr = blockers.erase(itr);
			} else {
				itr++->second.InputEnable();
			}
		}
		TheNavMesh->Update();
		botBuilder->onFrame();

	}
	for (auto player: Player::getPlayers()) {
		player.second->think();
	}
}

void PluginAdaptor::clientDisconnect(edict_t *pEntity) {
	delete Player::getPlayer(pEntity);
}

void PluginAdaptor::levelShutdown() {
	auto& players = Player::getPlayers();
	while(!players.empty()) {
		delete players.begin()->second;
	}
	blockers.clear();
	if (hidingSpotSelector) {
		hidingSpotSelector->save();
	}
	hidingSpotSelector = nullptr;
}

void PluginAdaptor::FireGameEvent(IGameEvent* event) {
// TODO: this doesn't really work.
	blockers.clear();
}
