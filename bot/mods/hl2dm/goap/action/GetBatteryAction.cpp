#include "GetBatteryAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool GetBatteryAction::findTargetLoc() {
	return blackboard.getSelf()->getArmor() < 100 && GoToEntityAction::findTargetLoc();
}
