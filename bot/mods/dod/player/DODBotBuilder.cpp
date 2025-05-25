#include "DODBotBuilder.h"

#include "DODWorld.h"
#include <mods/dod/goap/action/DODBombTargetAction.h>
#include <mods/dod/goap/action/DODDefendPointAction.h>
#include <mods/dod/goap/action/DODUseFragGrenadeAction.h>
#include <mods/dod/goap/action/DODPickUpGrenadeAction.h>
#include <mods/dod/goap/action/GiveAmmoAction.h>
#include <mods/dod/weapon/DODSMGBuilder.h>
#include <mods/dod/weapon/DODAssaultRifleBuilder.h>
#include <mods/dod/weapon/DODMGBuilder.h>
#include <mods/dod/weapon/DODLiveGrenadeBuilder.h>
#include <mods/dod/voice/DODVoiceMessage.h>
#include <mods/dod/util/DodPlayer.h>
#include <goap/action/AttackAction.h>
#include <player/Bot.h>
#include <weapon/SimpleWeaponBuilder.h>
#include <weapon/GrenadeLauncherFunction.h>
#include <weapon/DeployableWeaponBuilder.h>
#include <weapon/PistolBuilder.h>
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/UtilityToolBuilder.h>
#include <voice/VoiceMessage.h>
#include <move/Navigator.h>
#include <move/MoveStateContext.h>
#include <eiface.h>
#include <goap/GoalManager.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

static constexpr int CLASS_COUNT = 6;
static const char *CLASSES[2][CLASS_COUNT] { { "cls_garand", "cls_tommy",
		"cls_bar", "cls_spring", "cls_30cal", "cls_bazooka" }, { "cls_mk98",
		"cls_mp40", "cls_mp44", "cls_k98s", "cls_mg42", "cls_pschreck" } };

const std::set<std::string> DODLiveGrenadeBuilder::NAMES {
		"weapon_frag_ger_live", "weapon_frag_us_live",
		"weapon_riflegren_ger_live", "weapon_riflegren_us_live" };

class GrenadeLauncherBuilder: public SimpleWeaponBuilder<GrenadeLauncherFunction> {
public:
	explicit GrenadeLauncherBuilder(float velocity) :
		velocity(velocity) {
	}

	std::shared_ptr<Weapon> build(edict_t *weap) const override {
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

class AntiTankBuilder final : public DeployableWeaponBuilder<Reloader> {
public:
	AntiTankBuilder() :
			DeployableWeaponBuilder<Reloader>(0.9f, 500.0f, 2000.0f,
					"m_bDeployed") {
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

	std::shared_ptr<Weapon> build(edict_t *weap) const override {
		auto weapon = PistolBuilder::build(weap);
		weapon->getPrimary()->setFullAuto(true);
		return weapon;
	}
};

DODBotBuilder::DODBotBuilder(CommandHandler &commandHandler) : BotBuilder(commandHandler) {
	Bot::setClasses(&CLASSES);
	messages[VoiceMessage::ENEMY_SIGHTED] = "voice_enemyahead";
	messages[VoiceMessage::AREA_CLEAR] = "voice_areaclear";
	messages[VoiceMessage::NEED_BACKUP] = "voice_backup";
	messages[VoiceMessage::AFFIRMATIVE] = "voice_yessir";
	messages[DODVoiceMessage::GRENADE] = "voice_grenade";
	messages[DODVoiceMessage::FIRE_IN_THE_HOLE] = "voice_fireinhole";
	messages[DODVoiceMessage::NEED_AMMO] = "voice_needammo";
	messages[DODVoiceMessage::MG_AHEAD] = "voice_mgahead";
	messages[DODVoiceMessage::SNIPER] = "voice_sniper";
	messages[DODVoiceMessage::ROCKET_AHEAD] = "voice_bazookaspotted";
	listenForGameEvent({"dod_round_active", "dod_round_win", "dod_game_over"});
	teamPlay = true;
	addPair<GrenadeLauncherBuilder>("weapon_riflegren_us", "weapon_riflegren_ger", 2000.0f);
	addPair<MeleeWeaponBuilder>("weapon_amerknife", "weapon_spade");
	addPair<DeployableWeaponBuilder<Reloader>>("weapon_garand", "weapon_k98",
			0.8f, Weapon::MELEE_RANGE, 2000.0f, "m_bZoomed", 1000.0f);
	addPair<DODSMGBuilder>("weapon_thompson", "weapon_mp40");
	addPair<PistolBuilder>("weapon_colt", "weapon_p38", 0.2f);
	addPair<GrenadeBuilder>("weapon_smoke_us", "weapon_smoke_ger");
	addPair<GrenadeBuilder>("weapon_frag_us", "weapon_frag_ger");
	addPair<DODAssaultRifleBuilder>("weapon_bar", "weapon_mp44");
	addPair<DeployableWeaponBuilder<Reloader>>("weapon_spring", "weapon_k98_scoped",
			0.8f, 1000.0f, 3600.0f, "m_bZoomed", 1000.0f);
	addPair<DODMGBuilder>("weapon_30cal", "weapon_mg42");
	addPair<AntiTankBuilder>("weapon_bazooka", "weapon_pschreck");
	weaponBuilders["weapon_m1carbine"] = std::make_shared<PistolBuilder>(0.4f);
	weaponBuilders["weapon_c96"] = std::make_shared<C96Builder>();
	weaponBuilders["weapon_basebomb"] = std::make_shared<UtilityToolBuilder>();
	for (const auto& grenade: DODLiveGrenadeBuilder::NAMES) {
		weaponBuilders[grenade] = std::make_shared<DODLiveGrenadeBuilder>();
	}
}

void DODBotBuilder::FireGameEvent(IGameEvent* event) {
	DODWorld::setRoundStarted(std::string(event->GetName()) == "dod_round_active");
}

void DODBotBuilder::updatePlanner(GoalManager &planner,
		Bot *bot) const {

	class DODDestroyObjectAction: public AttackAction {
	public:
		explicit DODDestroyObjectAction(Bot *bot) :
				AttackAction(bot) {
		}
	private:
		bool isBreakable(edict_t *object) const override {
			return Q_stristr(object->GetClassName(), "physics") != nullptr;
		}
	};

	class DODGetBombAction: public GoToEntityWithGivenNameAction {
	public:
		DODGetBombAction(Bot *bot) :
			GoToEntityWithGivenNameAction(self, "dod_bomb_dispenser") {
			effects = { WorldProp::HAS_BOMB, true };
			allItemsVisible = true;
		}
	private:
		bool isAvailable(edict_t* ent) override {
			return true;
		}
	};
	planner.addAction<DODThrowLiveGrenadeAction>(0.97f);
	planner.addAction<DODPickUpGrenadeAction>(0.96f);
	planner.addAction<GiveAmmoAction>(0.95f, commandHandler);
	planner.addAction<DODUseFragGrenadeAction>(0.92f);
	planner.addAction<DODUseRifleGrenadeAction>(0.92f);
	planner.addAction<DODUseSmokeGrenadeAction>(0.84f);
	planner.addAction<DODDestroyObjectAction>(0.82f);
	planner.addAction<DODDefuseBombAction>(0.64f)->setObjectives(&objectives);
	planner.addAction<DODDefendPointAction>(0.63f)->setObjectives(&objectives);
	planner.addAction<DODBombTargetAction>(0.62f)->setObjectives(&objectives);
	planner.addAction<CapturePointAction>(0.61f)->setObjectives(&objectives);
	planner.addAction<DODGetBombAction>(0.0f);
}

World* DODBotBuilder::buildWorld() const {
	return new DODWorld();
}

void DODBotBuilder::modHandleCommand(const CCommand &command, Bot* bot) const {
	bot->setDesiredClassId(
			command.ArgC() > 3 ?
					atoi(command.Arg(3)) % CLASS_COUNT - 1 :
					RandomInt(0, CLASS_COUNT - 1));
	bot->setPlayerClassVar<DodPlayer>();
	bot->setHookEnabled(true);
}

class DODNavigator: public Navigator {
public:
	DODNavigator(Bot *bot) :
			Navigator(bot) {
	}

private:
	bool unproned = false;

	bool checkCanMove() {
		if (!Navigator::checkCanMove()) {
			return false;
		}
		if (DodPlayer(moveCtx->getSelf()->getEdict()).isProne()) {
			if (!unproned) {
				moveCtx->getSelf()->getButtons().tap(IN_ALT1);
				unproned = true;
			}
			return true;
		}
		unproned = false;
		return true;
	}
};

Bot *DODBotBuilder::modBuild(Bot *bot) {
	bot->setNavigator<DODNavigator>();
	bot->getVision().setMiniMapRange(500.0f);
	bot->getVision().addClassName("grenade_frag_ger");
	bot->getVision().addClassName("grenade_frag_us");
	bot->getVision().addClassName("grenade_riflegren_ger");
	bot->getVision().addClassName("grenade_riflegren_us");
	return bot;
}

template<typename _Tp, typename... _Args>
void DODBotBuilder::addPair(const char* name1, const char* name2, _Args&&... args) {
	weaponBuilders[name1] = weaponBuilders[name2] = std::make_shared<_Tp>(args...);
}
