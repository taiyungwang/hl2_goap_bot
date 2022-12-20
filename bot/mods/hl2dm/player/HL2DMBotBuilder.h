#pragma once

#include <mods/hl2dm/goap/action/ItemMap.h>
#include <player/BotBuilder.h>

class HL2MPPlayer;
class HL2DMWorld;

class HL2DMBotBuilder: public BotBuilder {
public:
	HL2DMBotBuilder(CommandHandler& commandHandler, const ArsenalBuilder &arsenalBuilder);

private:
	ItemMap itemMap;

	void modOnFrame() override {
		itemMap.buildMap();
	}

	void updatePlanner(GoalManager &planner, Blackboard &blackboard) const;

	BasePlayer* buildEntity(edict_t *ent) const;

	World* buildWorld() const;
};
