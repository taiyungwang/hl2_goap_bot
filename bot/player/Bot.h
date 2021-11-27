#pragma once

#include "Player.h"
#include "Vision.h"
#include "CommandHandler.h"
#include "move/RotationManager.h"

class Blackboard;
class GoalManager;
class Action;
class BasePlayer;
class Goal;
class Navigator;
class World;
class CBotCmd;
class CGameTrace;
class VoiceMessageSender;
class ITraceFilter;

typedef const char *(*PlayerClasses)[2][6];

class Bot: public Player, public CommandHandler::Receiver {
public:

	static void setClasses(PlayerClasses options) {
		CLASSES = options;
	}

	static void canSee(CGameTrace& result, const Vector& start, const Vector& end);

	Bot(edict_t *ent, const std::shared_ptr<Arsenal> &arsenal,
			CommandHandler& commandHandler,
			VoiceMessageSender &voiceMessageSender) :
			Player(ent, arsenal), Receiver(commandHandler),
			voiceMessageSender(voiceMessageSender) {
	}

	~Bot();

	template<typename T>
	void setPlayerClassVar() {
		playerClassVar = new T(getEdict());
	}

	void think();

	void setBlackboard(Blackboard *blackboard) {
		this->blackboard = blackboard;
	}

	void setWorld(World *world);

	void setPlanner(GoalManager *planner) {
		this->planner = planner;
	}

	bool handle(EventInfo *event);

	bool receive(edict_t* sender, const CCommand&) override;

	CBotCmd* getCmd() const;

	void setHookEnabled(bool hookEnabled) {
		this->hookEnabled = hookEnabled;
	}

	void setInGame(bool inGame = false) {
		this->inGame = inGame;
	}

	void setDesiredClassId(int classId = -1) {
		this->desiredClassId = classId;
	}

	int getPlayerClass() const;

	void setWantToListen(bool wantToListen) {
		this->wantToListen = wantToListen;
	}

	bool canShoot(const Vector &vecAbsEnd) const;

	bool canSee(const Player& target) const;

	bool canSee(edict_t* target) const;

	VoiceMessageSender& getVoiceMessageSender() {
		return voiceMessageSender;
	}

	Vision& getVision() {
		return vision;
	}

	void lookStraight();

	Vector getFacing() const;

	float getAimAccuracy() const;

	void setViewTarget(const Vector& target) {
		this->viewTarget = target;
	}

	const Vector& getViewTarget() const {
		return viewTarget;
	}

	void traceMove(CGameTrace &traceResult, const Vector &start,
			const Vector &goal, bool crouch, const ITraceFilter &filter) const;

	void setAiming(bool aiming) {
		this->aiming = aiming;
	}

	void setNavigator(const std::shared_ptr<Navigator>& navigator) {
		this->navigator = navigator;
	}

	std::shared_ptr<Navigator> getNavigator() {
		return navigator;
	}

private:
	static PlayerClasses CLASSES;

	VoiceMessageSender &voiceMessageSender;

	Vision vision;

	BasePlayer *playerClassVar = nullptr;

	Blackboard *blackboard = nullptr;

	std::shared_ptr<Navigator> navigator;

	int desiredClassId = -1;

	GoalManager *planner = nullptr;

	World *world = nullptr;

	RotationManager rotation;

	Vector viewTarget;

	bool hookEnabled = false, resetPlanner = false, wantToListen = true, aiming = false;

	bool canShoot(CGameTrace &result, const Vector &vecAbsEnd) const;

	void listen();
};
