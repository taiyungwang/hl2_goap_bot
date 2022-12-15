#include "HL2DMBotBuilder.h"

#include "HL2DMWorld.h"
#include <mods/hl2dm/goap/action/UseGravityGunAction.h>
#include <mods/hl2dm/goap/action/GetClosestNeededItemAction.h>
#include <mods/hl2dm/goap/action/Item.h>
#include <mods/hl2dm/util/HL2MPPlayer.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <goap/action/GoToAction.h>
#include <goap/action/ThrowGrenadeAction.h>
#include <goap/GoalManager.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>

class HealthChargerBuilder: public ChargerBuilder {
public:
	bool need(const Bot& bot) const override {
		return bot.getHealth() < 100;
	}
};

class HealthKitBuilder: public ItemBuilder {
public:
	bool need(const Bot& bot) const override {
		return bot.getHealth() < 100;
	}
};

class BatteryBuilder: public ItemBuilder {
public:
	bool need(const Bot& bot) const override {
		return bot.getArmor() < 100;
	}
};

class SuitChargerBuilder: public ChargerBuilder {
public:
	bool need(const Bot& bot) const override {
		return bot.getArmor() < 100;
	}
};

class ThrowFragGrenadeAction: public ThrowGrenadeAction {
public:
	ThrowFragGrenadeAction(Blackboard &blackboard) :
			ThrowGrenadeAction(blackboard) {
	}

private:
	bool canUse(const char* weaponName) const override {
		return std::string("weapon_frag") == weaponName;
	}
};

class ItemWeaponBuilder: public ItemBuilder {
public:
	ItemWeaponBuilder(const char *weapon): WEAPON_NAME(weapon) {
	}

	bool need(const Bot &bot) const override {
		auto arsenal = bot.getArsenal();
		return nullptr == arsenal.getWeapon(arsenal.getWeaponIdByName(WEAPON_NAME.c_str()));
	}

private:
	const std::string WEAPON_NAME;
};

class ItemAmmoBuilder: public ItemBuilder {
public:
	ItemAmmoBuilder(const char *weapon, int maxAmmo, bool secondary = false) :
			WEAPON_NAME(weapon), MAX_AMMO(maxAmmo), SECONDARY(secondary) {
	}

	bool need(const Bot &bot) const override {
		auto arsenal = bot.getArsenal();
		Weapon *weapon = arsenal.getWeapon(
				arsenal.getWeaponIdByName(WEAPON_NAME.c_str()));
		return weapon != nullptr
				&& ((!SECONDARY && weapon->getPrimary()->getAmmo(bot.getEdict()) < MAX_AMMO)
						||(SECONDARY && weapon->getSecondary()->getAmmo(bot.getEdict()) < MAX_AMMO));
	}

private:
	const bool SECONDARY;

	const std::string WEAPON_NAME;

	const int MAX_AMMO;
};

#define DECL_AMMO_BUILDER_CLASS(weaponClass, weaponName, ammo) \
class ItemAmmo##weaponClass##Builder: public ItemAmmoBuilder {\
public:\
	ItemAmmo##weaponClass##Builder() : ItemAmmoBuilder(weaponName, ammo) {} \
};

#define DECL_AMMO_WEAPON_BUILDER_CLASSES(weaponClass, weaponName, ammo) \
DECL_AMMO_BUILDER_CLASS(weaponClass, weaponName, ammo)\
	\
class Item##weaponClass##Builder: public ItemWeaponBuilder {\
public:\
	Item##weaponClass##Builder(): ItemWeaponBuilder(weaponName) {\
	}\
};

DECL_AMMO_WEAPON_BUILDER_CLASSES(Pistol, "pistol", 150)
DECL_AMMO_WEAPON_BUILDER_CLASSES(AR2, "weapon_ar2", 60)
DECL_AMMO_WEAPON_BUILDER_CLASSES(SMG, "weapon_smg1", 225)
DECL_AMMO_WEAPON_BUILDER_CLASSES(Shotgun, "weapon_shotgun", 30)
DECL_AMMO_WEAPON_BUILDER_CLASSES(RPG, "weapon_rpg", 3)
DECL_AMMO_WEAPON_BUILDER_CLASSES(Crossbow, "weapon_crossbow", 10)
DECL_AMMO_WEAPON_BUILDER_CLASSES(Magnum, "weapon_357", 12)
DECL_AMMO_BUILDER_CLASS(Grenade, "weapon_grenade", 5)

class ItemAmmoSmgGrenadeBuilder: public ItemAmmoBuilder {
public:
	ItemAmmoSmgGrenadeBuilder() : ItemAmmoBuilder("weapon_smg1", 3, true) {
	}
};

class ItemAmmoAR2AltBuilder: public ItemAmmoBuilder {
public:
	ItemAmmoAR2AltBuilder() : ItemAmmoBuilder("weapon_ar2", 3, true) {
	}
};

HL2DMBotBuilder::HL2DMBotBuilder(CommandHandler& commandHandler, const ArsenalBuilder &arsenalBuilder) :
		BotBuilder(commandHandler, arsenalBuilder) {
	itemMap.addItemBuilder<HealthKitBuilder>("item_healthkit");
	itemMap.addItemBuilder<HealthKitBuilder>("item_healthvial");
	itemMap.addItemBuilder<HealthChargerBuilder>("item_healthcharger");
	itemMap.addItemBuilder<BatteryBuilder>("item_battery");
	itemMap.addItemBuilder<SuitChargerBuilder>("item_suitcharger");
	itemMap.addItemBuilder<ItemAmmoPistolBuilder>("item_ammo_pistol");
	itemMap.addItemBuilder<ItemAmmoPistolBuilder>("item_ammo_pistol_large");
	itemMap.addItemBuilder<ItemPistolBuilder>("weapon_pistol");
	itemMap.addItemBuilder<ItemAmmoAR2Builder>("item_ammo_ar2");
	itemMap.addItemBuilder<ItemAmmoAR2Builder>("item_ammo_ar2_large");
	itemMap.addItemBuilder<ItemAmmoAR2AltBuilder>("item_ammo_ar2_altfire");
	itemMap.addItemBuilder<ItemSMGBuilder>("weapon_smg1");
	itemMap.addItemBuilder<ItemAmmoSMGBuilder>("item_ammo_smg1");
	itemMap.addItemBuilder<ItemAmmoSMGBuilder>("item_ammo_smg1_large");
	itemMap.addItemBuilder<ItemAmmoSmgGrenadeBuilder>("item_ammo_smg1_grenade");
	itemMap.addItemBuilder<ItemShotgunBuilder>("weapon_shotgun");
	itemMap.addItemBuilder<ItemAmmoShotgunBuilder>("item_box_buckshot");
	itemMap.addItemBuilder<ItemCrossbowBuilder>("weapon_crossbow");
	itemMap.addItemBuilder<ItemAmmoCrossbowBuilder>("item_ammo_crossbow");
	itemMap.addItemBuilder<ItemAmmoCrossbowBuilder>("item_ammo_crossbow_large");
	itemMap.addItemBuilder<ItemMagnumBuilder>("weapon_357");
	itemMap.addItemBuilder<ItemAmmoMagnumBuilder>("item_ammo_357");
	itemMap.addItemBuilder<ItemAmmoMagnumBuilder>("item_ammo_357_large");
	itemMap.addItemBuilder<ItemRPGBuilder>("weapon_rpg");
	itemMap.addItemBuilder<ItemAmmoMagnumBuilder>("item_rpg_round");
	itemMap.addItemBuilder<ItemAmmoGrenadeBuilder>("weapon_frag");
}

void HL2DMBotBuilder::updatePlanner(GoalManager& planner,
		Blackboard& blackboard) const {
	planner.addAction<ThrowFragGrenadeAction>(0.92f);
	GetClosestNeededItemAction::setItemMap(&itemMap);
	planner.addAction<GetClosestNeededItemAction>(0.54f);
	planner.addAction<UseGravityGunAction>(0.71f);
}

BasePlayer* HL2DMBotBuilder::buildEntity(edict_t* ent) const {
	return new HL2MPPlayer(ent);
}

World* HL2DMBotBuilder::buildWorld() const {
	return new HL2DMWorld();
}
