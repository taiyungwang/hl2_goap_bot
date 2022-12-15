#pragma once

#include <player/BotBuilder.h>
#include "DODObjectives.h"
#include <set>

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder {
public:
	DODBotBuilder(CommandHandler& commandHandler,
			const ArsenalBuilder& arsenalBuilder);

private:
	DODObjectives objectives;

	std::set<std::string> liveGrenades;

	void updatePlanner(GoalManager& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;

	Bot *modBuild(Bot *bot, Blackboard& blackboard) override;
};
