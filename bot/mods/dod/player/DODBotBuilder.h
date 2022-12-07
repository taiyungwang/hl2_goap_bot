#pragma once

#include <player/BotBuilder.h>
#include "DODObjectives.h"
#include <igameevents.h>
#include <set>

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public IGameEventListener2 {
public:
	DODBotBuilder(GameManager* objectives, CommandHandler& commandHandler,
			const ArsenalBuilder& arsenalBuilder);
	
	~DODBotBuilder();

	void FireGameEvent(IGameEvent* event);

private:
	std::set<std::string> liveGrenades;

	bool roundStarted = false;

	void updatePlanner(GoalManager& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;

	Bot *modBuild(Bot *bot, Blackboard& blackboard) override;
};
