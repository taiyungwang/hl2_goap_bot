#pragma once

#include "Player.h"
#include "Vision.h"
#include "Buttons.h"
#include "CommandHandler.h"
#include "move/RotationManager.h"
#include <eiface.h>
#include <iplayerinfo.h>

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

	void setWorld(World *world);

	void setPlanner(GoalManager *planner) {
		this->planner = planner;
	}

	void FireGameEvent(IGameEvent* event) override;

	bool receive(edict_t* sender, const CCommand&) override;

	const CBotCmd &getCmd() const {
		return cmd;
	}

	CBotCmd &getCmd() {
		return cmd;
	}

	Buttons &getButtons() {
		return buttons;
	}

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

	template<typename T>
	void setNavigator() {
		navigator = std::make_unique<T>(this);
	}
	
	Navigator *getNavigator() const {
		return navigator.get();
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

	edict_t *getBlocker() const {
		return blocker;
	}

	void setBlocker(edict_t *blocker) {
		this->blocker = blocker;
	}

	edict_t *getTarget() const {
		return target;
	}

	void setTarget(edict_t *target) {
		this->target = target;
	}

private:
	static PlayerClasses CLASSES;

	const std::unordered_map<unsigned int, std::string> &messages;

	using SentMessage = std::tuple<bool, float, std::string>;

	std::list<SentMessage> sentMessages;

	Vision vision;

	Buttons buttons;

	CBotCmd cmd;

	BasePlayer *playerClassVar = nullptr;

	edict_t *blocker = nullptr,
			*target = nullptr;

	std::unique_ptr<Navigator> navigator;

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
