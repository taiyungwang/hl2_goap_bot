#pragma once

#include <weapon/WeaponBuilder.h>
#include <convar.h>
#include <string>
#include <list>

class Bot;
class BasePlayer;
class CommandHandler;
class GoalManager;
class World;
class HidingSpotSelector;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:
	BotBuilder(CommandHandler& commandHandler);

	virtual ~BotBuilder();

	void CommandCallback(const CCommand &command) override {
		(this->*cmdCallbacks.at(command.Arg(0)))(command);
	}

	void onFrame();

	void kickAllExcept(const CCommand &command);

	const WeaponBuilders &getWeaponBuilders() const {
		return weaponBuilders;
	}

protected:
	std::unordered_map<unsigned int, std::string> messages;

	CommandHandler& commandHandler;

	bool teamPlay = false;

	WeaponBuilders weaponBuilders;

	virtual void updatePlanner(GoalManager& planner,
			Bot *self) const = 0;

	virtual World* buildWorld() const = 0;

	virtual void modOnFrame() {}

	virtual void modHandleCommand(const CCommand &command, Bot* bot) const {
	}

	virtual Bot* modBuild(Bot *bot) {
		return bot;
	}

private:
	typedef void (BotBuilder::*CmdFuncPtr)(const CCommand &command);

	HidingSpotSelector* hidingSpotSelector = nullptr;

	std::unordered_map<std::string, CmdFuncPtr> cmdCallbacks;

	std::list<std::unique_ptr<ConCommand>> commands;

	Bot* build(edict_t* ent);

	void addAllBots(const CCommand &command);

	void addBot(const CCommand &command);

	void addCommand(const char* name, const char* description, CmdFuncPtr ptr);
};
