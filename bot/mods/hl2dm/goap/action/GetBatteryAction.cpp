#include "GetBatteryAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool GetBatteryAction::precondCheck() {
	return !GoToItemAction::precondCheck()
			|| blackboard.getSelf()->getArmor() < 100;
}
