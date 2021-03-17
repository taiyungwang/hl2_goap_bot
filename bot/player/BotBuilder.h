#pragma once

#include <convar.h>
#include <utlmap.h>

class Blackboard;
class Bot;
class BasePlayer;
class Player;
class Planner;
class WeaponBuilderFactory;
class World;
class HidingSpotSelector;
struct edict_t;

class BotBuilder: public ICommandCallback {
public:

	BotBuilder() {
		command = new ConCommand("mybot_add_bot", this,
				"Add a bot to the server");
	}

	virtual ~BotBuilder();

	void CommandCallback(const CCommand &command);

	virtual void onNavMeshLoad();

	HidingSpotSelector* getHidingSpotSelector() const {
		return hidingSpotSelector;
	}

protected:
	virtual void initWeapons(WeaponBuilderFactory& factory) const = 0;

	virtual void updatePlanner(Planner& planner,
			Blackboard& blackboard) const = 0;

	virtual BasePlayer* buildEntity(edict_t* ent) const;

	virtual World* buildWorld() const = 0;

	virtual void modHandleCommand(const CCommand &command, Bot* bot) {
	}

private:
	ConCommand* command;

	HidingSpotSelector* hidingSpotSelector = nullptr;

	friend class Bot;

	Bot* build(edict_t* ent) const;
};
