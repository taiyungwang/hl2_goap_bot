#pragma once

#include <player/BotBuilder.h>
#include <event/EventHandler.h>
#include "DODObjectives.h"

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public EventHandler {
public:
	DODBotBuilder(GameManager* objectives);

	bool handle(EventInfo* event);

private:
	bool roundStarted = false;

	void initWeapons(WeaponBuilderFactory& factory) const;

	void updatePlanner(GoalManager& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;
};
