#pragma once

#include <utllinkedlist.h>

class Thinker;
class BotBuilder;
struct edict_t;

/**
 * Allows the use of either the VSDK Plugin or MetaMod Plugin.
 */
class PluginAdaptor {
public:

	PluginAdaptor();

	~PluginAdaptor();

	const char *getPluginDescription(void) const {
		return "MyBot";
	}

	void levelInit(const char* mapName);

	void gameFrame(bool isSimulating);

	void clientActive(edict_t *pEntity);

	void clientPutInServer(edict_t *pEntity);

	void clientDisconnect(edict_t *pEntity);

	void levelShutdown();

	template<typename T>
	void handEvent(T* event);

private:
	BotBuilder* botBuilder;

	CUtlLinkedList<Thinker*> thinkers;

	char modPath[256];
};

