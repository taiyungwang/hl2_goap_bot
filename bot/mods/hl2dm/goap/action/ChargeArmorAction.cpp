#include "ChargeArmorAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

bool ChargeArmorAction::isFinished() const {
	return blackboard.getSelf()->getArmor() >= 200;
}
