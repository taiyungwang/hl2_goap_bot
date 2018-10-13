#include "GetBatteryAction.h"

#include <player/Blackboard.h>
#include <player/Player.h>

bool GetBatteryAction::precondCheck() {
	return !GoToItemAction::precondCheck()
			|| blackboard.getSelf()->getArmor() < 100;
}
