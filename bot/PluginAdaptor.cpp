#include "PluginAdaptor.h"

#include "event/EventInfo.h"
#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "player/Bot.h"
#include "player/GameManager.h"
#include "player/VTableHook.h"
#include <nav_mesh/nav_entities.h>
#include <util/EntityClassManager.h>
#include <util/EntityUtils.h>
#include <util/BaseEntity.h>
#include <eiface.h>
#include <iplayerinfo.h>

CGlobalVars *gpGlobals = nullptr;
ConVar r_visualizetraces("r_visualizetraces", "0", FCVAR_CHEAT);

EntityClassManager *classManager = nullptr;
CNavMesh* TheNavMesh = nullptr;
bool navMeshLoadAttempted;

ConVar mybot_debug("my_bot_debug", "0");
ConVar mybot_var("mybot_var", "0.5");

CUtlMap<int, NavEntity*> blockers;

extern IPlayerInfoManager *playerinfomanager;
extern IVEngineServer* engine;

PluginAdaptor::PluginAdaptor() {
	// TODO: consider moving constructor initializations into init callback.
	extern IServerGameDLL *servergamedll;
	classManager = new EntityClassManager(servergamedll);
	gpGlobals = playerinfomanager->GetGlobalVars();
	TheNavMesh = new CNavMesh;
	botBuilder = nullptr;
	SetDefLessFunc(Player::getPlayers());
	SetDefLessFunc(blockers);
	engine->GetGameDir(modPath, 256);
	// TODO: make mod checking more stringent.
	if (Q_stristr(modPath, "hl2mp")) {
		botBuilder = new HL2DMBotBuilder();
		TheNavMesh->addPlayerSpawnName("info_player_start");
	} else if (Q_stristr(modPath, "dod")) {
		gameManager = new DODObjectives();
		botBuilder = new DODBotBuilder(gameManager);
		enableHook = true;
		TheNavMesh->addPlayerSpawnName("info_player_axis");
		TheNavMesh->addPlayerSpawnName("info_player_allies");
	} else {
		Msg("Mod not supported, %s.\n", modPath);
	}
}

PluginAdaptor::~PluginAdaptor() {
	delete TheNavMesh;
	TheNavMesh = nullptr;
	delete classManager;
	classManager = nullptr;
	if (enableHook) {
		unhookPlayerRunCommand();
	}
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
			botBuilder->onNavMeshLoad();
			Msg("Loaded Navigation mesh.\n");
		}
		navMeshLoadAttempted = true;
	}
	while (!activationQ.IsEmpty()) {
		edict_t* ent = activationQ.RemoveAtHead();
		auto& players = Player::getPlayers();
		auto i = players.Find(engine->IndexOfEdict(ent));
		if (i == players.InvalidIndex()) {
			new Player(ent);
		} else if (enableHook) {
			hookPlayerRunCommand(ent);
		}
	}
	if (TheNavMesh != nullptr) {
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
	}
	auto& players = Player::getPlayers();
	FOR_EACH_MAP_FAST(players, i) {
		players[i]->think();
	}
}

void PluginAdaptor::clientDisconnect(edict_t *pEntity) {
	delete Player::getPlayer(pEntity);
}

void PluginAdaptor::levelShutdown() {
	auto& players = Player::getPlayers();
	while(players.Count() > 0) {
		delete players[players.FirstInorder()];
	}
	blockers.PurgeAndDeleteElements();
	if (gameManager != nullptr) {
		gameManager->endRound();
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

bool PluginAdaptor::handle(EventInfo* event) {
	if (Q_strcmp("nav_generate", event->getName()) == 0) {
		blockers.PurgeAndDeleteElements();
		return true;
	}
	return false;
}
