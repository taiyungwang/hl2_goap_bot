#include "ChargeArmorAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool ChargeArmorAction::isFinished() const {
	return blackboard.getSelf()->getArmor() >= 200;
}

bool ChargeArmorAction::findTargetLoc() {
	return blackboard.getSelf()->getArmor() < 200
			&& GoToEntityAction::findTargetLoc();
}
