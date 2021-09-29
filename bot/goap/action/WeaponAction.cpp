#include "WeaponAction.h"

#include <player/Blackboard.h>
#include <player/Bot.h>

WeaponAction::WeaponAction(Blackboard& blackboard) :
		Action(blackboard), arsenal(blackboard.getSelf()->getArsenal()) {
}
