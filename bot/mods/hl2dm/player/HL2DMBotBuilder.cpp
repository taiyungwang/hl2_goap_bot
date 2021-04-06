#include "HL2DMBotBuilder.h"

#include "HL2DMWorld.h"
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <goap/action/GoToAction.h>
#include <goap/GoalManager.h>
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


void HL2DMBotBuilder::updatePlanner(GoalManager& planner,
		Blackboard& blackboard) const {
	/**
	 * Defines the action for using a suit charger
	 */
	class ChargeHealthAction: public ChargeAction {
	public:
		ChargeHealthAction(Blackboard& blackboard) :
				ChargeAction("item_healthcharger", blackboard) {
			effects = {WorldProp::HEALTH_FULL, true};
		}

	private:
		bool isFinished() const {
			return blackboard.getSelf()->getHealth()
					>= blackboard.getSelf()->getMaxHealth();
		}
	};

	class GetHealthKitAction: public GetItemAction {
	public:
		GetHealthKitAction(Blackboard& blackboard) :
				GetItemAction("item_healthkit", blackboard) {
			effects = {WorldProp::HEALTH_FULL, true};
		}
	};

	planner.addAction<UseGravityGunAction>(0.7f);
	planner.addAction<ChargeHealthAction>(0.53f);
	planner.addAction<GetHealthKitAction>(0.52f);
	planner.addAction<ChargeArmorAction>(0.51f);
	planner.addAction<GetBatteryAction>(0.5f);
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

BasePlayer* HL2DMBotBuilder::buildEntity(edict_t* ent) const {
	return new HL2MPPlayer(ent);
}

World* HL2DMBotBuilder::buildWorld() const {
	return new HL2DMWorld();
}
