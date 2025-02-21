#pragma once

#include "player/CommandHandler.h"
#include "player/GameEventListener.h"
#include <memory>

class Thinker;
class BotBuilder;
class ArsenalBuilder;
class VoiceCommand;
class HidingSpotSelector;
class CBasePlayer;
struct edict_t;

/**
 * Allows the use of either the VSDK Plugin or MetaMod Plugin.
 */
class PluginAdaptor : public GameEventListener {
public:
	static int getPlayerruncommandOffset();

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

	void FireGameEvent(IGameEvent* event);

	void clientCommand(edict_t* player, const CCommand& command) {
		commandHandler.handle(player, command);
	}

	std::list<edict_t*>& getNewPlayers() {
		return newPlayers;
	}

private:
	bool navMeshLoadAttempted = false;

	CommandHandler commandHandler;

	BotBuilder* botBuilder;

	std::shared_ptr<ArsenalBuilder> arsenalBuilder;

	std::shared_ptr<VoiceCommand> voiceCommand;

	std::shared_ptr<HidingSpotSelector> hidingSpotSelector;

	std::list<edict_t*> newPlayers;
};

