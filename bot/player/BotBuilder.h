#pragma once

#include <convar.h>
#include <utlmap.h>

class Blackboard;
class Bot;
class EntityInstance;
class Player;
class Planner;
class WeaponBuilderFactory;
class World;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:

	BotBuilder() {
		command = new ConCommand("mybot_add_bot", this,
				"Add a bot to the server");
	}

	virtual ~BotBuilder();

	Bot* build(const CUtlMap<int, Player*>& players, edict_t* ent) const;

	void setEnableHook(bool enableHook = false) {
		this->enableHook = enableHook;
	}

	void CommandCallback(const CCommand &command);

protected:
	bool enableHook = false;

	int classType = -1;

	virtual void initWeapons(WeaponBuilderFactory& factory) const = 0;

	virtual void updatePlanner(Planner& planner,
			Blackboard& blackboard) const = 0;

	virtual EntityInstance* buildEntity(edict_t* ent) const;

	virtual World* buildWorld() const = 0;

	virtual void modHandleCommand(const CCommand &command) {
	}

	virtual void update(Bot *bot) const {
	}

private:
	ConCommand* command;
};
