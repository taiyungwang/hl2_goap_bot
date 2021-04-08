#include "Reloader.h"

#include <player/Blackboard.h>
#include <player/Buttons.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

bool Reloader::execute(Blackboard& blackboard) {
	blackboard.getButtons().tap(IN_RELOAD);
	return !weapon.isClipEmpty();
}

