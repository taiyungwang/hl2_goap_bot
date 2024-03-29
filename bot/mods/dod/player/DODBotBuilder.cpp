#include "DODBotBuilder.h"

#include "DODWorld.h"
#include <mods/dod/goap/action/DODBombTargetAction.h>
#include <mods/dod/goap/action/DODDefendPointAction.h>
#include <mods/dod/goap/action/DODUseFragGrenadeAction.h>
#include <mods/dod/goap/action/DODPickUpGrenadeAction.h>
#include <mods/dod/goap/action/GiveAmmoAction.h>
#include <mods/dod/voice/DODVoiceMessage.h>
#include <mods/dod/util/DodPlayer.h>
#include <goap/action/AttackAction.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <voice/VoiceMessage.h>
#include <move/Navigator.h>
#include <eiface.h>
#include <goap/GoalManager.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

static const int CLASS_COUNT = 6;
static const char *CLASSES[2][CLASS_COUNT] { { "cls_garand", "cls_tommy",
		"cls_bar", "cls_spring", "cls_30cal", "cls_bazooka" }, { "cls_mk98",
		"cls_mp40", "cls_mp44", "cls_k98s", "cls_mg42", "cls_pschreck" } };

DODBotBuilder::DODBotBuilder(CommandHandler &commandHandler,
		const ArsenalBuilder &arsenalBuilder) : BotBuilder(commandHandler, arsenalBuilder) {
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
}

void DODBotBuilder::FireGameEvent(IGameEvent* event) {
	DODWorld::setRoundStarted(std::string(event->GetName()) == "dod_round_active");
}

void DODBotBuilder::updatePlanner(GoalManager &planner,
		Blackboard &blackboard) const {

	class DODDestroyObjectAction: public AttackAction {
	public:
		DODDestroyObjectAction(Blackboard &blackboard) :
				AttackAction(blackboard) {
		}
	private:
		bool isBreakable(edict_t *object) const {
			return Q_stristr(object->GetClassName(), "physics") != nullptr;
		}
	};

	class DODGetBombAction: public GoToEntityWithGivenNameAction {
	public:
		DODGetBombAction(Blackboard &blackboard) :
			GoToEntityWithGivenNameAction(blackboard, "dod_bomb_dispenser") {
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
	DODNavigator(Blackboard &blackboard) :
			Navigator(blackboard) {
	}

private:
	bool unproned = false;

	bool checkCanMove() {
		if (!Navigator::checkCanMove()) {
			return false;
		}
		if (DodPlayer(blackboard.getSelf()->getEdict()).isProne()) {
			if (!unproned) {
				blackboard.getButtons().tap(IN_ALT1);
				unproned = true;
			}
			return true;
		}
		unproned = false;
		return true;
	}
};

Bot *DODBotBuilder::modBuild(Bot *bot, Blackboard& blackboard) {
	bot->setNavigator(std::make_shared<DODNavigator>(blackboard));
	bot->getVision().setMiniMapRange(500.0f);
	bot->getVision().addClassName("grenade_frag_ger");
	bot->getVision().addClassName("grenade_frag_us");
	bot->getVision().addClassName("grenade_riflegren_ger");
	bot->getVision().addClassName("grenade_riflegren_us");
	return bot;
}
