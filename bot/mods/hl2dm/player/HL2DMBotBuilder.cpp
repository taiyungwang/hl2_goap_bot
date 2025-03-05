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

protected:
	const std::string WEAPON_NAME;
};

class ItemAmmoBuilder: public ItemWeaponBuilder {
public:
	ItemAmmoBuilder(const char *weapon, int maxAmmo, bool secondary = false) : ItemWeaponBuilder(weapon),
		SECONDARY(secondary), MAX_AMMO(maxAmmo) {
	}

	bool need(const Bot &bot) const override {
		auto arsenal = bot.getArsenal();
		Weapon *weapon = arsenal.getWeapon(arsenal.getWeaponIdByName(WEAPON_NAME.c_str()));
		return weapon != nullptr && (SECONDARY ? weapon->getSecondary() : weapon->getPrimary())->getAmmo(bot.getEdict()) < MAX_AMMO;
	}

private:
	const bool SECONDARY;

	const int MAX_AMMO;
};

#define DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(weaponClass, weaponName, ammo, ammoName) \
	class ItemAmmo##weaponClass##Builder: public ItemAmmoBuilder {\
	public:\
		ItemAmmo##weaponClass##Builder() : ItemAmmoBuilder(weaponName, ammo) {} \
	};			\
	itemMap.addItemBuilder<ItemAmmo##weaponClass##Builder>(ammoName);\
	class Item##weaponClass##Builder: public ItemWeaponBuilder {\
	public:\
		Item##weaponClass##Builder(): ItemWeaponBuilder(weaponName) {\
		}\
	};\
	itemMap.addItemBuilder<Item##weaponClass##Builder>(weaponName);


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
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(Pistol, "pistol", 150, "item_ammo_pistol")
	itemMap.addItemBuilder<ItemAmmoPistolBuilder>("item_ammo_pistol_large");
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(AR2, "weapon_ar2", 60, "item_ammo_ar2")
	itemMap.addItemBuilder<ItemAmmoAR2Builder>("item_ammo_ar2_large");
	itemMap.addItemBuilder<ItemAmmoAR2AltBuilder>("item_ammo_ar2_altfire");
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(SMG, "weapon_smg1", 225, "item_ammo_smg1")
	itemMap.addItemBuilder<ItemAmmoSMGBuilder>("item_ammo_smg1_large");
	itemMap.addItemBuilder<ItemAmmoSmgGrenadeBuilder>("item_ammo_smg1_grenade");
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(Shotgun, "weapon_shotgun", 30, "item_box_buckshot")
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(Crossbow, "weapon_crossbow", 10, "item_ammo_crossbow")
	itemMap.addItemBuilder<ItemAmmoCrossbowBuilder>("item_ammo_crossbow_large");
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(Magnum, "weapon_357", 12, "item_ammo_357")
	itemMap.addItemBuilder<ItemAmmoMagnumBuilder>("item_ammo_357_large");
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(RPG, "weapon_rpg", 3, "item_rpg_round")
	DECL_AMMO_WEAPON_BUILDER_CLASSES_AND_ADD(Grenade, "weapon_grenade", 5, "weapon_frag")
}

void HL2DMBotBuilder::updatePlanner(GoalManager& planner,
		Blackboard& blackboard) const {
	planner.addAction<ThrowFragGrenadeAction>(0.92f);
	GetClosestNeededItemAction::setItemMap(&itemMap);
	planner.addAction<GetClosestNeededItemAction>(0.54f);
	planner.addAction<UseGravityGunAction>(0.71f);
}

World* HL2DMBotBuilder::buildWorld() const {
	return new HL2DMWorld();
}

void HL2DMBotBuilder::modHandleCommand(const CCommand &command, Bot* bot) const {
	bot->setPlayerClassVar<HL2MPPlayer>();
}
