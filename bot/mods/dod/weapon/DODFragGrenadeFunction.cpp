#include "DODFragGrenadeFunction.h"

#include <player/Buttons.h>
#include <in_buttons.h>

DODFragGrenadeFunction::DODFragGrenadeFunction() :
	GrenadeLauncherFunction() {
	range[1] = 1200.0f;
	zMultiplier = 600.0f;
}

void DODFragGrenadeFunction::attack(Buttons& buttons, float distance) const {
	buttons.hold(IN_ATTACK);
}
