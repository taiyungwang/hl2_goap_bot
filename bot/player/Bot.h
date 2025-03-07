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
class ITraceFilter;

typedef const char *(*PlayerClasses)[2][6];

class Bot: public Player, public CommandHandler::Receiver {
public:

	static void setClasses(PlayerClasses options) {
		CLASSES = options;
	}

	static void canSee(CGameTrace& result, const Vector& start, const Vector& end);

	Bot(edict_t *ent, const WeaponBuilders& arsenal,
			CommandHandler& commandHandler,
			const std::unordered_map<unsigned int, std::string> &messages);

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

	void FireGameEvent(IGameEvent* event) override;

	bool receive(edict_t* sender, const CCommand&) override;

	const CBotCmd &getCmd() const;

	void setHookEnabled(bool hookEnabled) {
		this->hookEnabled = hookEnabled;
	}

	void setDesiredClassId(int classId = -1) {
		this->desiredClassId = classId;
	}

	int getPlayerClass() const;

	void setWantToListen(bool wantToListen) {
		this->wantToListen = wantToListen;
	}

	bool canShoot(const Vector &vecAbsEnd) const;

	bool canSee(const Vector &vecAbsEnd) const;

	bool canSee(const Player& target) const {
		return canSee(target.getEyesPos());
	}

	bool canSee(edict_t* target) const;

	bool sendVoiceMessage(const unsigned int message);

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

	void setAimOffset(float aiming) {
		aimOffset = aiming;
	}

	void setNavigator(const std::shared_ptr<Navigator>& navigator) {
		this->navigator = navigator;
	}

	std::shared_ptr<Navigator> getNavigator() {
		return navigator;
	}

	void setResetPlanner(bool reset) {
		resetPlanner = reset;
	}

	bool isVoiceMessageType(unsigned int messageType, const char* message) const {
		return messages.at(messageType) == message;
	}

	void setHasRadio(bool radio) {
		hasRadio = radio;
	}

	bool isOnLadder() const;

	void consoleMsg(const std::string& message) const;

	void consoleWarn(const std::string& message) const;

	int getDesiredWeapon() const {
		return desiredWeapon;
	}

	void setDesiredWeapon(int weapon) {
		desiredWeapon = weapon;
	}

	int getBestWeapon() const;

private:
	static PlayerClasses CLASSES;

	const std::unordered_map<unsigned int, std::string> &messages;

	using SentMessage = std::tuple<bool, float, std::string>;

	std::list<SentMessage> sentMessages;

	Vision vision;

	BasePlayer *playerClassVar = nullptr;

	Blackboard *blackboard = nullptr;

	std::shared_ptr<Navigator> navigator;

	int desiredClassId = -1, desiredWeapon = 0;

	float aimOffset = 0.0f;

	GoalManager *planner = nullptr;

	World *world = nullptr;

	RotationManager rotation;

	Vector viewTarget;

	bool hookEnabled = false, wantToListen = true, resetPlanner = false,
			hasRadio = false;

	bool canShoot(CGameTrace &result, const Vector &vecAbsEnd) const;

	void listen();
};
