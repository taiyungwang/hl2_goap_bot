#include "SMGBuilder.h"

#include <player/Buttons.h>
#include <weapon/GrenadeLauncherFunction.h>
#include <weapon/Weapon.h>
#include <in_buttons.h>

class SMGGrenadeLauncher: public GrenadeLauncherFunction {
public:
	void attack(Buttons& buttons, float distance) const {
		buttons.hold(IN_ATTACK2);
	}
};

Weapon* SMGBuilder::build(edict_t* weap) {
	Weapon* weapon = FullAutoGunBuilder::build(weap);
	weapon->setSecondary(new SMGGrenadeLauncher());
	return weapon;
}
