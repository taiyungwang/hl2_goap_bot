#include "SMGBuilder.h"

#include <player/Buttons.h>
#include <weapon/GrenadeLauncherFunction.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

class SMGGrenadeLauncher: public GrenadeLauncherFunction {
public:
	SMGGrenadeLauncher() {
		initialVelocity = 1500.0f;
	}

	void attack(Buttons& buttons, float distance) const {
		buttons.hold(IN_ATTACK2);
	}
};

std::shared_ptr<Weapon> SMGBuilder::build(edict_t* weap) const {
	auto weapon = FullAutoGunBuilder::build(weap);
	weapon->getPrimary()->getRange()[1] = 750.0f;
	weapon->setSecondary(std::make_shared<SMGGrenadeLauncher>());
	return weapon;
}
