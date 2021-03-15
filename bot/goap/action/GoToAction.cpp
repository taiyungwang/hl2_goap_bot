#include "GoToAction.h"

#include <move/Navigator.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

float GoToAction::getCost() {
	return findTargetLoc() ? targetLoc.DistTo(blackboard.getSelf()->getCurrentPosition()) : INFINITY;
}

bool GoToAction::execute() {
	canAbort = !blackboard.isOnLadder();
	return blackboard.getNavigator()->step();
}

bool GoToAction::precondCheck() {
	return blackboard.getNavigator()->buildPath(targetLoc, path);
}

void GoToAction::init() {
	blackboard.getNavigator()->start(&path, targetLoc, targetRadius);
}

bool GoToAction::postCondCheck() {
	return blackboard.getNavigator()->reachedGoal();
}

/**
 * Randomly choose a target based on its relative distance.
 */
edict_t* GoToAction::randomChoice(CUtlLinkedList<edict_t*>& active) {
	edict_t* item = nullptr;
	if (active.Count() == 0) {
		return item;
	}
	item = active[active.Tail()];
	if (active.Count() == 1) {
		return item;
	}
	float totalDist = 0.0f;
	CUtlLinkedList<float> prob;
	FOR_EACH_LL(active, i) {
		prob.AddToTail(1.0f / active[i]->GetCollideable()->GetCollisionOrigin().DistTo(blackboard.getSelf()->getCurrentPosition()));
		totalDist += prob[prob.Tail()];
	}
	float totalProb = 0.0f;
	FOR_EACH_LL(prob, i) {
		prob[i] /= totalDist;
		if (i > 0) {
			prob[i] += prob[i - 1];
		}
	}
	float choice = RandomFloat(0, 1.0f);
	FOR_EACH_LL(prob, i) {
		if (choice < prob[i]) {
			item = active[i];
			break;
		}
	}
	return item;
}
