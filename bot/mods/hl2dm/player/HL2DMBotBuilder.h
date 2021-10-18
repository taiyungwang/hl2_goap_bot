#pragma once

#include <player/BotBuilder.h>

class HL2MPPlayer;
class HL2DMWorld;

class HL2DMBotBuilder: public BotBuilder {
public:
	HL2DMBotBuilder(CommandHandler& commandHandler, const ArsenalBuilder &arsenalBuilder) :
			BotBuilder(nullptr, commandHandler, arsenalBuilder) {
	}

private:
	void updatePlanner(GoalManager &planner, Blackboard &blackboard) const;

	BasePlayer* buildEntity(edict_t *ent) const;

	World* buildWorld() const;
};
