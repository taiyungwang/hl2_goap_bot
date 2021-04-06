#pragma once

#include <player/BotBuilder.h>

class HL2MPPlayer;
class HL2DMWorld;

class HL2DMBotBuilder: public BotBuilder {
public:
	HL2DMBotBuilder(): BotBuilder(nullptr) {
	}

private:

	void initWeapons(WeaponBuilderFactory& factory) const;

	void updatePlanner(GoalManager& planner, Blackboard& blackboard) const;

	BasePlayer* buildEntity(edict_t* ent) const;

	World* buildWorld() const;
};
