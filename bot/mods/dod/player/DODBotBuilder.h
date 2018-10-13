#pragma once

#include <player/BotBuilder.h>
#include <event/EventHandler.h>

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public EventHandler {
public:
	DODBotBuilder();

	~DODBotBuilder();

	bool handle(EventInfo* event);

private:
	bool roundStarted = false;

	void initWeapons(WeaponBuilderFactory& factory) const;

	void updatePlanner(Planner& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command);

	void update(Bot *bot) const;
};
