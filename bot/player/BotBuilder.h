#pragma once

#include <convar.h>
#include <utlhashtable.h>

class GameManager;
class Blackboard;
class Bot;
class BasePlayer;
class Player;
class GoalManager;
class World;
class HidingSpotSelector;
class ArsenalBuilder;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:

	BotBuilder(GameManager* objectives, const ArsenalBuilder& arsenalBuilder);

	virtual ~BotBuilder();

	void CommandCallback(const CCommand &command) {
		(this->**cmdCallbacks.GetPtr(command.Arg(0)))(command);
	}

	virtual void onNavMeshLoad();

	HidingSpotSelector* getHidingSpotSelector() const {
		return hidingSpotSelector;
	}

protected:
	GameManager* objectives;

	bool teamPlay = false;

	virtual void updatePlanner(GoalManager& planner,
			Blackboard& blackboard) const = 0;

	virtual BasePlayer* buildEntity(edict_t* ent) const;

	virtual World* buildWorld() const = 0;

	virtual void modHandleCommand(const CCommand &command, Bot* bot) const {
	}

private:
	const ArsenalBuilder& arsenalBuilder;

	typedef void (BotBuilder::*CmdFuncPtr)(const CCommand &command) const;

	HidingSpotSelector* hidingSpotSelector = nullptr;

	CUtlHashtable<const char*, CmdFuncPtr> cmdCallbacks;

	CUtlLinkedList<ConCommand*> commands;

	Bot* build(edict_t* ent) const;

	void addAllBots(const CCommand &command) const;

	void addBot(const CCommand &command) const;

	void kickAllBots(const CCommand &command) const;

	void kickAllExcept(const CCommand &command) const;

	void addCommand(const char* name, const char* description, CmdFuncPtr ptr);
};
