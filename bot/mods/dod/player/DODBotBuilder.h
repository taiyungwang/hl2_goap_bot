#pragma once

#include <player/BotBuilder.h>
#include <event/EventHandler.h>
#include "DODObjectives.h"

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public EventHandler {
public:
	DODBotBuilder();

	bool handle(EventInfo* event);

private:
	bool roundStarted = false;

	DODObjectives objectives;

	void initWeapons(WeaponBuilderFactory& factory) const;

	void updatePlanner(Planner& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command);

	void update(Bot *bot) const;
};
