#pragma once

#include <player/BotBuilder.h>
#include "DODObjectives.h"

class HL2MPPlayer;
class HL2DMWorld;

class DODBotBuilder: public BotBuilder, public GameEventListener {
public:
	DODBotBuilder(CommandHandler& commandHandler);

	void FireGameEvent(IGameEvent* event);

private:
	DODObjectives objectives;

	void updatePlanner(GoalManager& planner, Bot *self) const;

	World* buildWorld() const;

	void modHandleCommand(const CCommand &command, Bot* bot) const;

	Bot *modBuild(Bot *bot) override;

	template<typename _Tp, typename... _Args>
	void addPair(const char* name1, const char* name2, _Args&&... args);
};
