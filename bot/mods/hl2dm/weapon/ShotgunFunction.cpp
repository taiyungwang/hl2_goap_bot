#include "ShotgunFunction.h"

#include <player/Buttons.h>
#include<in_buttons.h>

void ShotgunFunction::attack(Buttons& buttons, float distance) const {
	buttons.tap(distance > 200.0f ? IN_ATTACK : IN_ATTACK2);
}
