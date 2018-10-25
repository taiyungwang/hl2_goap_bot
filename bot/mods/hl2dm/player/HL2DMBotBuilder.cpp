#include "HL2DMBotBuilder.h"

#include "HL2DMWorld.h"
#include <goap/Planner.h>
#include <goap/action/DestroyObjectAction.h>
#include <goap/action/GoToAction.h>
#include <mods/hl2dm/goap/action/ChargeArmorAction.h>
#include <mods/hl2dm/goap/action/GetBatteryAction.h>
#include <mods/hl2dm/goap/action/UseGravityGunAction.h>
#include <mods/hl2dm/weapon/AR2Builder.h>
#include <mods/hl2dm/weapon/CrossbowBuilder.h>
#include <mods/hl2dm/weapon/ShotgunFunction.h>
#include <mods/hl2dm/weapon/SMGBuilder.h>
#include <mods/hl2dm/weapon/MagnumBuilder.h>
#include <mods/hl2dm/util/HL2MPPlayer.h>
#include <weapon/GrenadeBuilder.h>
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/SimpleWeaponBuilder.h>
#include <weapon/PistolBuilder.h>
#include <weapon/RPGBuilder.h>
#include <weapon/WeaponBuilderFactory.h>
#include <weapon/UtilityToolBuilder.h>


void HL2DMBotBuilder::updatePlanner(Planner& planner,
		Blackboard& blackboard) const {
	planner.addAction<ChargeArmorAction>(0.51f);
	planner.addAction<GetBatteryAction>(0.5f);
	planner.addAction<UseGravityGunAction>(0.9f);
	planner.addAction<DestroyObjectAction>(0.9f);
}

void HL2DMBotBuilder::initWeapons(WeaponBuilderFactory& weaponFac) const {
	weaponFac.addInstance("weapon_stunstick", new MeleeWeaponBuilder());
	weaponFac.addInstance("weapon_crowbar", new MeleeWeaponBuilder());
	weaponFac.addInstance("weapon_pistol", new PistolBuilder(0.2f));
	weaponFac.addInstance("weapon_smg1", new SMGBuilder());
	weaponFac.addInstance("weapon_ar2", new AR2Builder());
	weaponFac.addInstance("weapon_shotgun", new SimpleWeaponBuilder<ShotgunFunction>());
	weaponFac.addInstance("weapon_357", new MagnumBuilder());
	weaponFac.addInstance("weapon_crossbow", new CrossbowBuilder());
	weaponFac.addInstance("weapon_frag", new GrenadeBuilder());
	weaponFac.addInstance("weapon_rpg", new RPGBuilder());
	weaponFac.addInstance("weapon_physcannon", new UtilityToolBuilder(768.0f));
}

EntityInstance* HL2DMBotBuilder::buildEntity(edict_t* ent) const {
	return new HL2MPPlayer(ent);
}

World* HL2DMBotBuilder::buildWorld() const {
	return new HL2DMWorld();
}
