#include "PluginAdaptor.h"

#include "event/EventInfo.h"
#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "player/PlayerManager.h"
#include "util/EntityClassManager.h"
#include "util/EntityUtils.h"
#include "util/BaseEntity.h"
#include "navmesh/nav_entities.h"

#include <eiface.h>
#include <iplayerinfo.h>

CGlobalVars *gpGlobals = nullptr;
ConVar r_visualizetraces("r_visualizetraces", "0", FCVAR_CHEAT);
PlayerManager *playerManager = nullptr;

EntityClassManager *classManager = nullptr;
CNavMesh* TheNavMesh = nullptr;
bool navMeshLoadAttempted;

ConVar mybot_debug("my_bot_debug", "0");
ConVar mybot_var("mybot_var", "0.5");

CUtlVector<NavEntity*> blockers;

extern IPlayerInfoManager *playerinfomanager;
extern IVEngineServer* engine;

PluginAdaptor::PluginAdaptor() {
	// TODO: consider moving constructor initializations into init callback.
	extern IServerGameDLL *servergamedll;
	classManager = new EntityClassManager(servergamedll);
	gpGlobals = playerinfomanager->GetGlobalVars();
	TheNavMesh = new CNavMesh;
	playerManager = nullptr;
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
			botBuilder->onNavMeshLoad();
			Msg("Loaded Navigation mesh.\n");
			for (int i = gpGlobals->maxClients; i < gpGlobals->maxEntities; i++) {
				edict_t* ent = engine->PEntityOfEntIndex(i);
				if (ent != nullptr && !ent->IsFree() && ent->GetIServerEntity() != nullptr
						&& FClassnameIs(ent, "prop_dynamic")) {
					blockers.AddToTail(new CFuncNavBlocker(ent));
				}
			}
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
		static unsigned int poll = 0;
		if (poll-- < 1) {
			poll = 120;
			FOR_EACH_VEC(blockers, i) {
				if (!FClassnameIs(blockers[i]->getEntity(), "prop_dynamic")
						|| !BaseEntity(blockers[i]->getEntity()).isDestroyedOrUsed()) {
					blockers[i]->InputEnable();
				} else {
					blockers[i]->InputDisable();
				}
			}
		}
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
	blockers.Purge();
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

bool PluginAdaptor::handle(EventInfo* event) {
	if (Q_strcmp("nav_generate", event->getName()) == 0) {
		blockers.Purge();
		return true;
	}
	return false;
}
