#include "DODArsenalBuilder.h"

#include "DODSMGBuilder.h"
#include "DODAssaultRifleBuilder.h"
#include "DODMGBuilder.h"
#include <weapon/SimpleWeaponBuilder.h>
#include <weapon/GrenadeLauncherFunction.h>
#include <weapon/DeployableWeaponBuilder.h>
#include <weapon/PistolBuilder.h>
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/UtilityToolBuilder.h>
#include "DODLiveGrenadeBuilder.h"

using namespace std;

const std::set<std::string> DODLiveGrenadeBuilder::NAMES {
		"weapon_frag_ger_live", "weapon_frag_us_live",
		"weapon_riflegren_ger_live", "weapon_riflegren_us_live" };

class GrenadeLauncherBuilder: public SimpleWeaponBuilder<GrenadeLauncherFunction> {
public:
	GrenadeLauncherBuilder(float velocity) :
		velocity(velocity) {
	}

	virtual std::shared_ptr<Weapon> build(edict_t *weap) const override {
		auto weapon = SimpleWeaponBuilder<GrenadeLauncherFunction>::build(weap);
		weapon->setGrenade(true);
		dynamic_cast<GrenadeLauncherFunction*>(weapon->getPrimary())->setInitialVelocity(
				velocity);
		weapon->getPrimary()->getRange()[1] = 2500.0f;
		return weapon;
	}

private:
	float velocity;
};

class AntiTankBuilder: public DeployableWeaponBuilder<Reloader> {
public:
	AntiTankBuilder() :
			DeployableWeaponBuilder<Reloader>(0.9f, 500.0f, 2000.0f,
					"CDODBaseRocketWeapon", "m_bDeployed") {
	}

	std::shared_ptr<Weapon> build(edict_t *weap) const {
		auto weapon = DeployableWeaponBuilder<Reloader>::build(weap);
		weapon->getPrimary()->setExplosive(true);
		return weapon;
	}
};

class C96Builder: public PistolBuilder {
public:
	C96Builder() :
			PistolBuilder(0.2f) {
	}

	std::shared_ptr<Weapon> build(edict_t *weap) const {
		auto weapon = PistolBuilder::build(weap);
		weapon->getPrimary()->setFullAuto(true);
		return weapon;
	}
};

DODArsenalBuilder::DODArsenalBuilder() {
	addPair<GrenadeLauncherBuilder>("weapon_riflegren_us", "weapon_riflegren_ger", 2000.0f);
	addPair<MeleeWeaponBuilder>("weapon_amerknife", "weapon_spade");
	addPair<DeployableWeaponBuilder<Reloader>>("weapon_garand", "weapon_k98",
			0.8f, 100.0f, 2000.0f, "CDODSniperWeapon", "m_bZoomed", 1000.0f);
	addPair<DODSMGBuilder>("weapon_thompson", "weapon_mp40");
	addPair<PistolBuilder>("weapon_colt", "weapon_p38", 0.2f);
	addPair<GrenadeBuilder>("weapon_smoke_us", "weapon_smoke_ger");
	addPair<GrenadeBuilder>("weapon_frag_us", "weapon_frag_ger");
	addPair<DODAssaultRifleBuilder>("weapon_bar", "weapon_mp44");
	addPair<DeployableWeaponBuilder<Reloader>>("weapon_spring", "weapon_k98_scoped",
			0.8f, 1000.0f, 3600.0f, "CDODSniperWeapon", "m_bZoomed", 1000.0f);
	addPair<DODMGBuilder>("weapon_30cal", "weapon_mg42");
	addPair<AntiTankBuilder>("weapon_bazooka", "weapon_pschreck");
	weaponBuilders["weapon_m1carbine"] = make_shared<PistolBuilder>(0.4f);
	weaponBuilders["weapon_c96"] = make_shared<C96Builder>();
	weaponBuilders["weapon_basebomb"] = make_shared<UtilityToolBuilder>();
	for (const auto grenade: DODLiveGrenadeBuilder::NAMES) {
		weaponBuilders[grenade] = make_shared<DODLiveGrenadeBuilder>();
	}
}

template<typename _Tp, typename... _Args>
void DODArsenalBuilder::addPair(const char* name1, const char* name2, _Args&&... args) {
	weaponBuilders[name1] = weaponBuilders[name2] = make_shared<_Tp>(args...);
}
