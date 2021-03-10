#include "PlayerManager.h"

#include "Bot.h"
#include "BotBuilder.h"
#include <util/Exception.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <igameevents.h>
#include <threadtools.h>

extern IVEngineServer* engine;

static CThreadMutex playerLock;
static CThreadMutex edictQLock;

PlayerManager::PlayerManager(const BotBuilder* botBuilder) :
		botBuilder(botBuilder) {
	SetDefLessFunc(players);
}

PlayerManager::~PlayerManager() {
	players.PurgeAndDeleteElements();
	edictQ.RemoveAll();
}

void PlayerManager::addPlayer(edict_t *ent) {
	edictQLock.Lock();
	edictQ.Insert(ent);
	edictQLock.Unlock();
}

void PlayerManager::think() {
	while (!edictQ.IsEmpty()) {
		edictQLock.Lock();
		edict_t *ent = edictQ.RemoveAtTail();
		edictQLock.Unlock();
		extern IPlayerInfoManager *playerinfomanager;
		IPlayerInfo* info = playerinfomanager->GetPlayerInfo(ent);
		Player *player =
				info->IsFakeClient() ?
						botBuilder->build(players, ent) : new Player(ent);
		playerLock.Lock();
		players.Insert(engine->IndexOfEdict(ent), player);
		playerLock.Unlock();
	}
	FOR_EACH_MAP_FAST(players, i) {
		try {
			players[i]->think();
		} catch (const Exception& e) {
			Msg("Exception caught: %s\n", e.what());
		}
	}
}

Player* PlayerManager::getPlayer(edict_t *ent) {
	auto i = players.Find(engine->IndexOfEdict(ent));
	return players.IsValidIndex(i) ? players[i]: nullptr;
}


Player* PlayerManager::getPlayer(int userId) {
	FOR_EACH_MAP_FAST(players, i) {
		if (players[i]->getUserId() == userId) {
			return players[i];
		}
	}
	return nullptr;
}

void PlayerManager::removePlayer(edict_t *ent) {
	auto i = players.Find(engine->IndexOfEdict(ent));
	if (!players.IsValidIndex(i)) {
		Warning("Tried to remove invalid edict.\n");
		return;
	}
	playerLock.Lock();
	delete players[i];
	players.RemoveAt(i);
	playerLock.Unlock();
}

