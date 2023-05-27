#pragma once

#include <player/BotBuilder.h>
#include "DODObjectives.h"

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public GameEventListener {
public:
	DODBotBuilder(CommandHandler& commandHandler,
			const ArsenalBuilder& arsenalBuilder);

	void FireGameEvent(IGameEvent* event);

private:
	DODObjectives objectives;

	void updatePlanner(GoalManager& planner, Blackboard& blackboard) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;

	Bot *modBuild(Bot *bot, Blackboard& blackboard) override;
};
