#pragma once

#include <voice/VoiceMessageSender.h>
#include <convar.h>
#include <utlhashtable.h>

class GameManager;
class Blackboard;
class Bot;
class BasePlayer;
class CommandHandler;
class Player;
class GoalManager;
class World;
class HidingSpotSelector;
class ArsenalBuilder;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:
	BotBuilder(GameManager* objectives, CommandHandler& commandHandler,
			const ArsenalBuilder& arsenalBuilder);

	virtual ~BotBuilder();

	void CommandCallback(const CCommand &command) {
		(this->**cmdCallbacks.GetPtr(command.Arg(0)))(command);
	}

protected:
	GameManager* objectives;

	bool teamPlay = false;

	VoiceMessageSender voiceMessageSender;

	virtual void updatePlanner(GoalManager& planner,
			Blackboard& blackboard) const = 0;

	virtual BasePlayer* buildEntity(edict_t* ent) const;

	virtual World* buildWorld() const = 0;

	virtual void modHandleCommand(const CCommand &command, Bot* bot) const {
	}

	virtual Bot* modBuild(Bot *bot, Blackboard& blackboard) {
		return bot;
	}

private:
	const ArsenalBuilder& arsenalBuilder;

	typedef void (BotBuilder::*CmdFuncPtr)(const CCommand &command);

	HidingSpotSelector* hidingSpotSelector = nullptr;

	CUtlHashtable<const char*, CmdFuncPtr> cmdCallbacks;

	CUtlLinkedList<ConCommand*> commands;

	CommandHandler& commandHandler;

	Bot* build(edict_t* ent);

	void addAllBots(const CCommand &command);

	void addBot(const CCommand &command);

	void kickAllBots(const CCommand &command);

	void kickAllExcept(const CCommand &command);

	void addCommand(const char* name, const char* description, CmdFuncPtr ptr);
};
