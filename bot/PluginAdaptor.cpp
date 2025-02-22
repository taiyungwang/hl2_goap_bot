#include "PluginAdaptor.h"

#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/hl2dm/weapon/HL2DMArsenalBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "mods/dod/weapon/DODArsenalBuilder.h"
#include "goap/action/SnipeAction.h"
#include "player/Bot.h"
#include "player/HidingSpotSelector.h"
#include <nav_mesh/nav_entities.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <iplayerinfo.h>

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
		arsenalBuilder = std::make_shared<DODArsenalBuilder>();
		botBuilder = new DODBotBuilder(commandHandler, *arsenalBuilder.get());
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
	while(players.size() > 0) {
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
