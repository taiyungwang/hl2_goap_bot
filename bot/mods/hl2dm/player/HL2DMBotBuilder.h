#pragma once

#include <player/BotBuilder.h>

class HL2MPPlayer;
class HL2DMWorld;

class HL2DMBotBuilder: public BotBuilder {

	void initWeapons(WeaponBuilderFactory& factory) const;

	void updatePlanner(Planner& planner, Blackboard& blackboard) const;

	EntityInstance* buildEntity(edict_t* ent) const;

	World* buildWorld() const;
};
