#pragma once

#include <convar.h>
#include <unordered_map>
#include <string>
#include <list>
#include <memory>

class Blackboard;
class Bot;
class BasePlayer;
class CommandHandler;
class GoalManager;
class World;
class HidingSpotSelector;
class ArsenalBuilder;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:
	BotBuilder(CommandHandler& commandHandler,
			const ArsenalBuilder& arsenalBuilder);

	virtual ~BotBuilder();

	void CommandCallback(const CCommand &command) {
		(this->*cmdCallbacks.at(command.Arg(0)))(command);
	}

	void onFrame();

	void kickAllExcept(const CCommand &command);

protected:
	std::unordered_map<unsigned int, std::string> messages;

	CommandHandler& commandHandler;

	virtual void updatePlanner(GoalManager& planner,
			Blackboard& blackboard) const = 0;

	virtual BasePlayer* buildEntity(edict_t* ent) const;

	virtual World* buildWorld() const = 0;

	virtual void modOnFrame() {}

	virtual void modHandleCommand(const CCommand &command, Bot* bot) const {
	}

	virtual Bot* modBuild(Bot *bot, Blackboard& blackboard) {
		return bot;
	}

private:
	const ArsenalBuilder& arsenalBuilder;

	typedef void (BotBuilder::*CmdFuncPtr)(const CCommand &command);

	HidingSpotSelector* hidingSpotSelector = nullptr;

	std::unordered_map<std::string, CmdFuncPtr> cmdCallbacks;

	std::list<std::shared_ptr<ConCommand>> commands;

	Bot* build(edict_t* ent);

	void addAllBots(const CCommand &command);

	void addBot(const CCommand &command);

	void addCommand(const char* name, const char* description, CmdFuncPtr ptr);
};
