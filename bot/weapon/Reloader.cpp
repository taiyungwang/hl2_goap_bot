#include "Reloader.h"

#include <player/Bot.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

bool Reloader::execute(Bot *self) {
	self->getButtons().tap(IN_RELOAD);
	return !weapon.isClipEmpty();
}

