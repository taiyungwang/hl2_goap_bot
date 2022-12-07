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
#include <voice/AreaClearVoiceMessage.h>
#include <voice/AffirmativeVoiceMessage.h>
#include <voice/GrenadeVoiceMessage.h>
#include <move/Navigator.h>
#include <eiface.h>
#include <goap/GoalManager.h>
#include <vstdlib/random.h>
#include <in_buttons.h>

static const int CLASS_COUNT = 6;
static const char *CLASSES[2][CLASS_COUNT] { { "cls_garand", "cls_tommy",
		"cls_bar", "cls_spring", "cls_30cal", "cls_bazooka" }, { "cls_mk98",
		"cls_mp40", "cls_mp44", "cls_k98s", "cls_mg42", "cls_pschreck" } };

extern IGameEventManager2* gameeventmanager;

class FireInTheHole: public VoiceMessage {
public:
	FireInTheHole(edict_t *sender) :
			VoiceMessage(sender) {

	}
};

DODBotBuilder::DODBotBuilder(GameManager *objectives,
		CommandHandler &commandHandler, const ArsenalBuilder &arsenalBuilder) :
		BotBuilder(objectives, commandHandler, arsenalBuilder) {
	gameeventmanager->AddListener(this, "dod_round_active", true);
	gameeventmanager->AddListener(this, "dod_round_win", true);
	gameeventmanager->AddListener(this, "dod_game_over", true);
	Bot::setClasses(&CLASSES);
	teamPlay = true;
	voiceMessageSender.addMessage<AreaClearVoiceMessage>("voice_areaclear");
	voiceMessageSender.addMessage<GrenadeVoiceMessage>("voice_grenade");
	voiceMessageSender.addMessage<NeedBackupVoiceMessage>("voice_backup");
	voiceMessageSender.addMessage<AffirmativeVoiceMessage>("voice_yessir");
	voiceMessageSender.addMessage<DODVoiceMessage::FireInTheHole>("voice_fireinhole");
	voiceMessageSender.addMessage<DODVoiceMessage::NeedAmmo>("voice_needammo");
	voiceMessageSender.addMessage<DODVoiceMessage::MGAheadVoiceMessage>("voice_mgahead");
	voiceMessageSender.addMessage<DODVoiceMessage::SniperAheadVoiceMessage>("voice_sniper");
	voiceMessageSender.addMessage<DODVoiceMessage::RocketAheadVoiceMessage>("voice_bazookaspotted");
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
	planner.addAction<DODDefuseBombAction>(0.64f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODDefendPointAction>(0.63f)->setObjectives(
			dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODBombTargetAction>(0.62f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<CapturePointAction>(0.61f)->setObjectives(dynamic_cast<DODObjectives*>(objectives));
	planner.addAction<DODGetBombAction>(0.0f);
}

DODBotBuilder::~DODBotBuilder() {
	gameeventmanager->RemoveListener(this);
}

void DODBotBuilder::FireGameEvent(IGameEvent* event) {
	std::string name(event->GetName());
	if (name == "dod_round_active") {
		objectives->startRound();
		roundStarted = true;
	} else if (name == "dod_game_over" || name == "dod_round_win") {
		roundStarted = false;
		objectives->endRound();
	}
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
