#include "PluginAdaptor.h"

#include "event/EventInfo.h"
#include "mods/hl2dm/player/HL2DMBotBuilder.h"
#include "mods/hl2dm/weapon/HL2DMArsenalBuilder.h"
#include "mods/dod/player/DODBotBuilder.h"
#include "mods/dod/weapon/DODArsenalBuilder.h"
#include "goap/action/SnipeAction.h"
#include "player/Bot.h"
#include "player/GameManager.h"
#include "player/HidingSpotSelector.h"
#include "player/VTableHook.h"
#include <nav_mesh/nav_entities.h>
#include <util/EntityClassManager.h>
#include <util/EntityUtils.h>
#include <eiface.h>
#include <iplayerinfo.h>

CGlobalVars *gpGlobals = nullptr;
ConVar r_visualizetraces("r_visualizetraces", "0", FCVAR_CHEAT);

EntityClassManager *classManager = nullptr;
CNavMesh* TheNavMesh = nullptr;

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
		extern  ConVar snipeChance;
		snipeChance.SetValue("1.0");
		gameManager = new DODObjectives();
		arsenalBuilder = std::make_shared<DODArsenalBuilder>();
		botBuilder = new DODBotBuilder(gameManager, commandHandler, *arsenalBuilder.get());
		enableHook = true;
		TheNavMesh->addPlayerSpawnName("info_player_axis");
		TheNavMesh->addPlayerSpawnName("info_player_allies");
	} else {
		Msg("Mod not supported, %s.\n", modPath.c_str());
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
			hidingSpotSelector = std::make_shared<HidingSpotSelector>(commandHandler);
			SnipeAction::setSpotSelector(hidingSpotSelector.get());
			Msg("Loaded Navigation mesh.\n");
		}
		navMeshLoadAttempted = true;
	}
	while (!activationQ.IsEmpty()) {
		edict_t* ent = activationQ.RemoveAtHead();
		auto& players = Player::getPlayers();
		if (players.find(engine->IndexOfEdict(ent)) == players.end()) {
			new Player(ent, arsenalBuilder->build());
		} else if (enableHook) {
			hookPlayerRunCommand(ent);
		}
	}
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
	// TODO: this doesn't really work.
	if (Q_strcmp("nav_generate", event->getName()) == 0) {
		blockers.PurgeAndDeleteElements();
		return true;
	}
	return false;
}
