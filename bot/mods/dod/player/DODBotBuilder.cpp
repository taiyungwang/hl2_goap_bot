#include "DODBotBuilder.h"

#include "DODWorld.h"
#include <mods/dod/goap/action/DODBombTargetAction.h>
#include <mods/dod/goap/action/DODDefendPointAction.h>
#include <mods/dod/goap/action/DODUseFragGrenadeAction.h>
#include <mods/dod/goap/action/DODPickUpGrenadeAction.h>
#include <mods/dod/voice/DODVoiceMessage.h>
#include <mods/dod/util/DodPlayer.h>
#include <event/EventHandler.h>
#include <event/EventInfo.h>
#include <goap/action/AttackAction.h>
#include <player/Bot.h>
#include <player/Blackboard.h>
#include <voice/AreaClearVoiceMessage.h>
#include <voice/GrenadeVoiceMessage.h>
#include <move/Navigator.h>
#include <util/EntityClassManager.h>
#include <util/EntityClass.h>
#include <util/EntityVar.h>
#include <eiface.h>
#include <goap/GoalManager.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

static const int CLASS_COUNT = 6;
static const char *CLASSES[2][CLASS_COUNT] { { "cls_garand", "cls_tommy",
		"cls_bar", "cls_spring", "cls_30cal", "cls_bazooka" }, { "cls_mk98",
		"cls_mp40", "cls_mp44", "cls_k98s", "cls_mg42", "cls_pschreck" } };

class FireInTheHole: public VoiceMessage {
public:
	FireInTheHole(edict_t *sender) :
			VoiceMessage(sender) {

	}
};

DODBotBuilder::DODBotBuilder(GameManager *objectives,
		CommandHandler &commandHandler, const ArsenalBuilder &arsenalBuilder) :
		BotBuilder(objectives, commandHandler, arsenalBuilder) {
	Bot::setClasses(&CLASSES);
	teamPlay = true;
	voiceMessageSender.addMessage<AreaClearVoiceMessage>("voice_areaclear");
	voiceMessageSender.addMessage<GrenadeVoiceMessage>("voice_grenade");
	voiceMessageSender.addMessage<NeedBackupVoiceMessage>("voice_backup");
	voiceMessageSender.addMessage<DODVoiceMessage::FireInTheHole>("voice_fireinhole");
	voiceMessageSender.addMessage<DODVoiceMessage::NeedAmmo>("voice_needammo");
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
	class DODGetBombAction: public GoToEntityAction {
	public:
		DODGetBombAction(Blackboard &blackboard) :
				GoToEntityAction(blackboard, "dod_bomb_dispenser") {
			effects = { WorldProp::HAS_BOMB, true };
		}
	};
	planner.addAction<DODThrowLiveGrenadeAction>(0.97f);
	planner.addAction<DODPickUpGrenadeAction>(0.96f);
	planner.addAction<DODUseFragGrenadeAction>(0.92f);
	planner.addAction<DODUseRifleGrenadeAction>(0.92f);
	planner.addAction<DODUseSmokeGrenadeAction>(0.83f);
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
