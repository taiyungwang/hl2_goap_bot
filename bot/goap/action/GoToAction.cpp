#include "GoToAction.h"

#include <move/Navigator.h>
#include <player/Blackboard.h>
#include <player/Bot.h>

float GoToAction::getCost() {
	return findTargetLoc() ? targetLoc.DistTo(blackboard.getSelf()->getCurrentPosition()) : INFINITY;
}

bool GoToAction::execute() {
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

