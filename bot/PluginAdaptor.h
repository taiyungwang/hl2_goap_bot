#pragma once

#include "event/EventHandler.h"
#include "player/CommandHandler.h"
#include <strtools.h>
#include <memory>

class Thinker;
class GameManager;
class BotBuilder;
class ArsenalBuilder;
class VoiceCommand;
class HidingSpotSelector;
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

	void clientPutInServer(edict_t *pEntity) {
		newPlayers.push_back(pEntity);
	}

	void clientDisconnect(edict_t *pEntity);

	void levelShutdown();

	template<typename T>
	void handEvent(T* event);

	bool handle(EventInfo* event);

	void clientCommand(edict_t* player, const CCommand& command) {
		commandHandler.handle(player, command);
	}

	std::list<edict_t*>& getNewPlayers() {
		return newPlayers;
	}

	int getHookOffset() const {
		return hookOffset;
	}

private:
	bool navMeshLoadAttempted = false;

	int hookOffset = -1;

	GameManager* gameManager = nullptr;

	CommandHandler commandHandler;

	BotBuilder* botBuilder;

	std::shared_ptr<ArsenalBuilder> arsenalBuilder;

	std::shared_ptr<VoiceCommand> voiceCommand;

	std::shared_ptr<HidingSpotSelector> hidingSpotSelector;

	std::list<edict_t*> newPlayers;
};

