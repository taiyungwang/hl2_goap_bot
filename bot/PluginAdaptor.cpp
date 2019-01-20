#include "PluginAdaptor.h"

#include "event/EventHandler.h"
#include "event/EventInfo.h"
#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "player/PlayerManager.h"
#include "util/EntityClassManager.h"
#include "navmesh/nav_mesh.h"

#include <eiface.h>
#include <iplayerinfo.h>

CGlobalVars *gpGlobals = nullptr;
ConVar r_visualizetraces("r_visualizetraces", "0", FCVAR_CHEAT);
PlayerManager *playerManager = nullptr;

EntityClassManager *classManager = nullptr;
CNavMesh* TheNavMesh = nullptr;
bool navMeshLoadAttempted;

ConVar mybot_debug("my_bot_debug", "0");

extern IPlayerInfoManager *playerinfomanager;

PluginAdaptor::PluginAdaptor() {
	// TODO: consider moving constructor initializations into init callback.
	extern IServerGameDLL *servergamedll;
	classManager = new EntityClassManager(servergamedll);
	gpGlobals = playerinfomanager->GetGlobalVars();
	TheNavMesh = new CNavMesh;
	playerManager = nullptr;
	extern IVEngineServer* engine;
	engine->GetGameDir(modPath, 256);
	// TODO: make mod checking more stringent.
	if (Q_stristr(modPath, "hl2mp")) {
		botBuilder = new HL2DMBotBuilder();
	} else if (Q_stristr(modPath, "dod")) {
		botBuilder = new DODBotBuilder();
		botBuilder->setEnableHook(true);
	} else {
		botBuilder = nullptr;
		Msg("Mod not supported, %s.\n", modPath);
	}
}

PluginAdaptor::~PluginAdaptor() {
	EventHandler::resetHandlers();
	delete TheNavMesh;
	TheNavMesh = nullptr;
	delete classManager;
	classManager = nullptr;
	delete botBuilder;
}

void PluginAdaptor::levelInit(const char* mapName) {
	navMeshLoadAttempted = false;
	if (botBuilder != nullptr) {
		playerManager = new PlayerManager(botBuilder);
		thinkers.AddToTail(playerManager);
	}
}

void PluginAdaptor::gameFrame(bool simulating) {
	if (!simulating) {
		return;
	}
	if (!navMeshLoadAttempted) {
		if (TheNavMesh->Load() == NAV_OK) {
			Msg("Loaded Navigation mesh.\n");
		}
		if (Q_stristr(modPath, "dod")) {
			TheNavMesh->SetPlayerSpawnName("info_player_axis");
			TheNavMesh->AddWalkableSeeds();
			TheNavMesh->SetPlayerSpawnName("info_player_allies");
			// allied walkable seed will be added if nav is generated.
		}
		navMeshLoadAttempted = true;
	}
	if (TheNavMesh != nullptr) {
		TheNavMesh->Update();
	}
	FOR_EACH_LL(thinkers, i)
	{
		thinkers[i]->think();
	}
}

void PluginAdaptor::clientActive(edict_t *pEntity) {
	playerManager->addPlayer(pEntity);
}

void PluginAdaptor::clientPutInServer(edict_t *pEntity) {

}

void PluginAdaptor::clientDisconnect(edict_t *pEntity) {
	playerManager->removePlayer(pEntity);
}

void PluginAdaptor::levelShutdown() {
	thinkers.RemoveAll();
	if (playerManager != nullptr) {
		delete playerManager;
		playerManager = nullptr;
	}
}

template
void PluginAdaptor::handEvent(IGameEvent* event);

template
void PluginAdaptor::handEvent(KeyValues* event);

template<typename T>
void PluginAdaptor::handEvent(T* event) {
	EventInfoWrapper<T> wrapper(event);
	EventHandler::notifyListeners(dynamic_cast<EventInfo*>(&wrapper));
}

