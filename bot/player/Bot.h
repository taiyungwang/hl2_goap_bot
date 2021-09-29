#pragma once

#include "Player.h"
#include "Vision.h"
#include "move/RotationManager.h"
#include <utlvector.h>
#include <utlqueue.h>

class Blackboard;
class GoalManager;
class Action;
class BasePlayer;
class Goal;
class Arsenal;
class World;
class CBotCmd;

typedef const char* (*PlayerClasses)[2][6];

class Bot: public Player {
public:

	static void setClasses(PlayerClasses options) {
		CLASSES = options;
	}

	Bot(edict_t* ent): Player(ent) {
	}

	~Bot();

	template<typename T>
	void setPlayerClassVar() {
		playerClassVar = new T(getEdict());
	}

	void think();

	void setBlackboard(Blackboard* blackboard) {
		this->blackboard = blackboard;
	}

	void setWorld(World* world);

	void setPlanner(GoalManager* planner) {
		this->planner = planner;
	}

	bool handle(EventInfo* event);

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

private:
	static PlayerClasses CLASSES;

	Vision vision;

	BasePlayer* playerClassVar = nullptr;

	Blackboard* blackboard = nullptr;

	int desiredClassId = -1;

	GoalManager* planner = nullptr;

	World* world = nullptr;

	RotationManager rotation;

	bool hookEnabled = false, resetPlanner = false, wantToListen = true;

	void despawn();

	void listen();
};
