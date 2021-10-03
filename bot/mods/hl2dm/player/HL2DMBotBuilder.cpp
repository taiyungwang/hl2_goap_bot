#include "HL2DMBotBuilder.h"

#include "HL2DMWorld.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
#include <mods/hl2dm/goap/action/ChargeArmorAction.h>
#include <mods/hl2dm/goap/action/GetBatteryAction.h>
#include <mods/hl2dm/goap/action/UseGravityGunAction.h>
#include <mods/hl2dm/util/HL2MPPlayer.h>


void HL2DMBotBuilder::updatePlanner(GoalManager& planner,
		Blackboard& blackboard) const {
	/**
	 * Defines the action for using a suit charger
	 */
	class ChargeHealthAction: public ChargeAction {
	public:
		ChargeHealthAction(Blackboard& blackboard) :
				ChargeAction("item_healthcharger", blackboard) {
			effects = {WorldProp::HEALTH_FULL, true};
		}

	private:
		bool isFinished() const {
			return blackboard.getSelf()->getHealth()
					>= blackboard.getSelf()->getMaxHealth();
		}
	};

	class GetHealthKitAction: public GetItemAction {
	public:
		GetHealthKitAction(Blackboard& blackboard) :
				GetItemAction("item_healthkit", blackboard) {
			effects = {WorldProp::HEALTH_FULL, true};
		}
	};

	planner.addAction<UseGravityGunAction>(0.7f);
	planner.addAction<ChargeHealthAction>(0.53f);
	planner.addAction<GetHealthKitAction>(0.52f);
	planner.addAction<ChargeArmorAction>(0.51f);
	planner.addAction<GetBatteryAction>(0.5f);
}

BasePlayer* HL2DMBotBuilder::buildEntity(edict_t* ent) const {
	return new HL2MPPlayer(ent);
}

World* HL2DMBotBuilder::buildWorld() const {
	return new HL2DMWorld();
}
