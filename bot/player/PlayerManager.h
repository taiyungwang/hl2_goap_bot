#pragma once

#include "goap/WorldCond.h"
#include "Thinker.h"
#include <utlqueue.h>

class Player;
class BotBuilder;
struct edict_t;

class PlayerManager: public Thinker {
public:
	PlayerManager(const BotBuilder* builder);

	virtual ~PlayerManager();

	void addPlayer(edict_t *ent);

	template<typename U>
	bool fireEvent(U* event);

	void removePlayer(edict_t *ent);

	Player* getPlayer(edict_t *ent);

	int getIdByName(const char* name) const;

	void think();

private:
	const BotBuilder* botBuilder;

	CUtlQueue<edict_t*> edictQ;

	CUtlMap<int, Player*> players;
};
