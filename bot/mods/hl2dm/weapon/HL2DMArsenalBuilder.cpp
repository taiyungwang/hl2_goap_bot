#include "HL2DMArsenalBuilder.h"

#include "MagnumBuilder.h"
#include "CrossbowBuilder.h"
#include "AR2Builder.h"
#include "SMGBuilder.h"
#include "ShotgunFunction.h"
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/PistolBuilder.h>
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/SimpleWeaponBuilder.h>
#include <weapon/GrenadeBuilder.h>
#include <weapon/RPGBuilder.h>
#include <weapon/UtilityToolBuilder.h>

using namespace std;

HL2DMArsenalBuilder::HL2DMArsenalBuilder() {
	weaponBuilders["weapon_stunstick"] = make_shared<MeleeWeaponBuilder>();
	weaponBuilders["weapon_crowbar"] = make_shared<MeleeWeaponBuilder>();
	weaponBuilders["weapon_pistol"] = make_shared<PistolBuilder>(0.2f);
	weaponBuilders["weapon_smg1"] = make_shared<SMGBuilder>();
	weaponBuilders["weapon_ar2"] = make_shared<AR2Builder>();
	weaponBuilders["weapon_shotgun"] = make_shared<SimpleWeaponBuilder<ShotgunFunction>>();
	weaponBuilders["weapon_357"] = make_shared<MagnumBuilder>();
	weaponBuilders["weapon_crossbow"] = make_shared<CrossbowBuilder>();
	weaponBuilders["weapon_frag"] = make_shared<GrenadeBuilder>();
	weaponBuilders["weapon_rpg"] = make_shared<RPGBuilder>();
	weaponBuilders["weapon_physcannon"] = make_shared<UtilityToolBuilder>(768.0f);
}
