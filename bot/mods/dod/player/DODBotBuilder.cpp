#include "DODBotBuilder.h"

#include "DODWorld.h"
#include <event/EventHandler.h>
#include <event/EventInfo.h>
#include <goap/Planner.h>
#include <goap/action/DestroyObjectAction.h>
#include <mods/dod/goap/action/DODBombTargetAction.h>
#include <mods/dod/goap/action/DODDefendPointAction.h>
#include <mods/dod/goap/action/DODUseFragGrenadeAction.h>
#include <mods/dod/weapon/DODSMGBuilder.h>
#include <mods/dod/weapon/DODAssaultRifleBuilder.h>
#include <mods/dod/weapon/DODMGBuilder.h>
#include <mods/dod/weapon/DODMGDeployer.h>
#include <mods/dod/weapon/DODFragGrenadeFunction.h>
#include <mods/dod/util/DodPlayer.h>
#include <weapon/PistolBuilder.h>
#include <weapon/WeaponBuilderFactory.h>
#include <weapon/SimpleWeaponBuilder.h>
#include <weapon/MeleeWeaponBuilder.h>
#include <weapon/UtilityToolBuilder.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <move/Navigator.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>
#include <eiface.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

static const int CLASS_COUNT = 6;
static const char *CLASSES[2][CLASS_COUNT] { { "cls_garand", "cls_tommy",
		"cls_bar", "cls_spring", "cls_30cal", "cls_bazooka" }, { "cls_mk98",
		"cls_mp40", "cls_mp44", "cls_k98s", "cls_mg42", "cls_pschreck" } };

DODBotBuilder::DODBotBuilder(GameManager* objectives): BotBuilder(objectives) {
	Bot::setClasses(&CLASSES);
	teamPlay = true;
}

void DODBotBuilder::updatePlanner(Planner &planner,
		Blackboard &blackboard) const {

	class DODDestroyObjectAction: public DestroyObjectAction {
	public:
		DODDestroyObjectAction(Blackboard &blackboard) :
				DestroyObjectAction(blackboard) {
		}
	private:
		bool isBreakable(edict_t *object) const {
			return Q_stristr(object->GetClassName(), "physics") != nullptr;
		}

	};
	class DODGetBombAction: public GoToEntityAction {
	public:
		DODGetBombAction(Blackboard &blackboard) :
				GoToEntityAction(blackboard, "dod_bomb_dispenser") {
			effects = { WorldProp::HAS_BOMB, true };
		}
	};
	class DODNavigator: public Navigator {
	public:
		DODNavigator(Blackboard &blackboard) :
				Navigator(blackboard) {
		}

	private:
		bool checkCanMove() {
			if (!Navigator::checkCanMove()) {
				return false;
			}
			if (DodPlayer(blackboard.getSelf()->getEdict()).isProne()) {
				if (!unproned) {
					blackboard.getButtons().tap(IN_ALT1);
					unproned = true;
				}
				return false;
			}
			unproned = false;
			return true;
		}

		bool unproned = false;
	};
	// TODO: hacky way of overriding the default navigator
	delete blackboard.getNavigator();
	blackboard.setNavigator(new DODNavigator(blackboard));
	planner.addAction<DODUseFragGrenadeAction>(0.92f);
	planner.addAction<DODUseRifleGrenadeAction>(0.92f);
	planner.addAction<DODUseSmokeGrenadeAction>(0.91f);
	planner.addAction<DODDestroyObjectAction>(0.7f);
	planner.addAction<DODDefuseBombAction>(0.64f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODDefendPointAction>(0.63f, 0.2f)->setObjectives(
			dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODBombTargetAction>(0.62f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<CapturePointAction>(0.61f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODGetBombAction>(0.0f);
}

bool DODBotBuilder::handle(EventInfo *event) {
	CUtlString name(event->getName());
	if (name == "dod_round_active") {
		objectives->startRound();
		roundStarted = true;
		return false;
	}
	if (name == "dod_game_over" || name == "dod_round_win") {
		roundStarted = false;
		objectives->endRound();
		return false;
	}
	return false;
}

void DODBotBuilder::initWeapons(WeaponBuilderFactory &weaponFac) const {
	class GrenadeLauncherBuilder: public SimpleWeaponBuilder<
			GrenadeLauncherFunction> {
	public:
		GrenadeLauncherBuilder(float zMultiplier) :
				zMultiplier(zMultiplier) {
		}

		Weapon* build(edict_t *weap) {
			Weapon *weapon =
					SimpleWeaponBuilder<GrenadeLauncherFunction>::build(weap);
			weapon->setGrenade(true);
			dynamic_cast<GrenadeLauncherFunction*>(weapon->getPrimary())->setZMultiplier(
					zMultiplier);
			weapon->getPrimary()->getRange()[1] = 1200.0f;
			return weapon;
		}

	private:
		float zMultiplier;
	};

	class AntiTankBuilder: public DeployableWeaponBuilder<Reloader> {
	public:
		AntiTankBuilder() :
				DeployableWeaponBuilder<Reloader>(0.9f, 500.0f, 2000.0f,
						"CDODBaseRocketWeapon", "m_bDeployed") {
		}

		Weapon* build(edict_t *weap) {
			Weapon *weapon = DeployableWeaponBuilder<Reloader>::build(weap);
			weapon->getPrimary()->setExplosive(true);
			return weapon;
		}
	};

	class C96Builder: public PistolBuilder {
	public:
		C96Builder() :
				PistolBuilder(0.2f) {
		}

		Weapon* build(edict_t *weap) {
			Weapon *weapon = PistolBuilder::build(weap);
			weapon->getPrimary()->setFullAuto(true);
			return weapon;
		}
	};
	weaponFac.addInstance("weapon_riflegren_us",
			new GrenadeLauncherBuilder(2.0f));
	weaponFac.addInstance("weapon_riflegren_ger",
			new GrenadeLauncherBuilder(2.0f));
	weaponFac.addInstance("weapon_amerknife", new MeleeWeaponBuilder());
	weaponFac.addInstance("weapon_spade", new MeleeWeaponBuilder());
	weaponFac.addInstance("weapon_garand",
			new DeployableWeaponBuilder<Reloader>(0.8f, 100.0f, 1600.0f,
					"CWeaponGarand", "m_bZoomed", 1000.0f));
	weaponFac.addInstance("weapon_k98",
			new DeployableWeaponBuilder<Reloader>(0.8f, 100.0f, 1600.0f,
					"CWeaponK98", "m_bZoomed", 1000.0f));
	weaponFac.addInstance("weapon_thompson", new DODSMGBuilder());
	weaponFac.addInstance("weapon_mp40", new DODSMGBuilder());
	weaponFac.addInstance("weapon_colt", new PistolBuilder(0.2f));
	weaponFac.addInstance("weapon_p38", new PistolBuilder(0.2f));
	weaponFac.addInstance("weapon_smoke_us",
			new GrenadeLauncherBuilder(400.0f));
	weaponFac.addInstance("weapon_smoke_ger",
			new GrenadeLauncherBuilder(400.0f));
	weaponFac.addInstance("weapon_frag_us", new GrenadeLauncherBuilder(600.0f));
	weaponFac.addInstance("weapon_frag_ger",
			new GrenadeLauncherBuilder(600.0f));
	weaponFac.addInstance("weapon_bar", new DODAssaultRifleBuilder());
	weaponFac.addInstance("weapon_mp44", new DODAssaultRifleBuilder());
	weaponFac.addInstance("weapon_spring",
			new DeployableWeaponBuilder<Reloader>(0.8f, 500.0f, 3600.0f,
					"CDODSniperWeapon", "m_bZoomed"));
	weaponFac.addInstance("weapon_k98_scoped",
			new DeployableWeaponBuilder<Reloader>(0.8f, 500.0f, 3600.0f,
					"CDODSniperWeapon", "m_bZoomed"));
	weaponFac.addInstance("weapon_mg42", new DODMGBuilder());
	weaponFac.addInstance("weapon_30cal", new DODMGBuilder());
	weaponFac.addInstance("weapon_pschreck", new AntiTankBuilder());
	weaponFac.addInstance("weapon_bazooka", new AntiTankBuilder());
	weaponFac.addInstance("weapon_m1carbine", new PistolBuilder(0.4f));
	weaponFac.addInstance("weapon_c96", new C96Builder());
	weaponFac.addInstance("weapon_basebomb", new UtilityToolBuilder());

}

World* DODBotBuilder::buildWorld() const {
	return new DODWorld(roundStarted);
}

void DODBotBuilder::modHandleCommand(const CCommand &command, Bot* bot) const {
	bot->setDesiredClassId(
			command.ArgC() > 3 ?
					atoi(command.Arg(3)) % CLASS_COUNT - 1 :
					RandomInt(0, CLASS_COUNT - 1));
	bot->setPlayerClassVar<DodPlayer>();
	bot->setHookEnabled(true);
}
