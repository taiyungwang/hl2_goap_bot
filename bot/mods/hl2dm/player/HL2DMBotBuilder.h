#pragma once

#include <mods/hl2dm/goap/action/ItemMap.h>
#include <player/BotBuilder.h>

class HL2MPPlayer;
class HL2DMWorld;

class HL2DMBotBuilder: public BotBuilder {
public:
	HL2DMBotBuilder(CommandHandler& commandHandler);

private:
	ItemMap itemMap;

	void modOnFrame() override {
		itemMap.buildMap();
	}

	void updatePlanner(GoalManager &planner, Bot *bot) const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;

	World* buildWorld() const;
};
