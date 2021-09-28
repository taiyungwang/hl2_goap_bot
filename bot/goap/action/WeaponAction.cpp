#include "WeaponAction.h"

#include <player/Blackboard.h>

WeaponAction::WeaponAction(Blackboard& blackboard) :
		Action(blackboard), arsenal(blackboard.getArsenal()) {
}
