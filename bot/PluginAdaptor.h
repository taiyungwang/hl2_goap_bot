#pragma once

#include "event/EventHandler.h"
#include <strtools.h>
#include <utlqueue.h>

class Thinker;
class GameManager;
class BotBuilder;
struct edict_t;

/**
 * Allows the use of either the VSDK Plugin or MetaMod Plugin.
 */
class PluginAdaptor : public EventHandler {
public:

	PluginAdaptor();

	~PluginAdaptor();

	const char *getPluginDescription(void) const {
		return "MyBot";
	}

	void levelInit(const char* mapName);

	void gameFrame(bool isSimulating);

	void clientActive(edict_t *pEntity) {
	}

	void clientPutInServer(edict_t *pEntity) {
		activationQ.Insert(pEntity);
	}

	void clientDisconnect(edict_t *pEntity);

	void levelShutdown();

	template<typename T>
	void handEvent(T* event);

	bool handle(EventInfo* event);

private:
	bool enableHook = false;

	GameManager* gameManager = nullptr;

	BotBuilder* botBuilder;

	CUtlQueue<edict_t*> activationQ;

	char modPath[256];
};

