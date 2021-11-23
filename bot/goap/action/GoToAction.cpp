#include "GoToAction.h"

#include <move/Navigator.h>
#include <move/NavMeshPathBuilderWithGoal.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <nav_mesh/nav_area.h>
#include <ivdebugoverlay.h>

float GoToAction::getCost() {
	return findTargetLoc() ? targetLoc.DistTo(blackboard.getSelf()->getCurrentPosition()) : INFINITY;
}

bool GoToAction::onPlanningFinished() {
	auto self = blackboard.getSelf();
	CNavArea* buildPathStartArea = Navigator::getArea(self);
	if (buildPathStartArea == nullptr) {
		Warning("Unable to get startArea.\n");
		return false;
	}
	Navigator::Path path;
	NavMeshPathBuilderWithGoal(*self, targetLoc, targetRadius).build(path,
			buildPathStartArea);
	if (!path.empty()) {
		blackboard.getNavigator()->getPath().swap(path);
		return true;
	}
	extern ConVar mybot_debug;
	if (mybot_debug.GetBool()) {
		Msg("Unable to build a path to goal.\n");
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(self->getCurrentPosition(),
				targetLoc, 255, 0, 0, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}

bool GoToAction::execute() {
	canAbort = !blackboard.isOnLadder();
	return blackboard.getNavigator()->step();
}

void GoToAction::init() {
	blackboard.getNavigator()->start(targetLoc, targetRadius, sprint);
}

bool GoToAction::goalComplete() {
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
	CUtlVector<CUtlKeyValuePair<edict_t*, float>> choices;
	FOR_EACH_LL(active, i) {
		choices.AddToTail();
		choices.Tail().m_key = active[i];
		choices.Tail().m_value = 1.0f / active[i]->GetCollideable()->GetCollisionOrigin().DistTo(blackboard.getSelf()->getCurrentPosition());
		totalDist += choices.Tail().m_value;
	}
	float totalProb = 0.0f;
	FOR_EACH_VEC(choices, i) {
		choices[i].m_value /= totalDist;
		if (i > 0) {
			choices[i].m_value += choices[i - 1].m_value;
		}
	}
	float choice = RandomFloat(0, 1.0f);
	FOR_EACH_VEC(choices, i) {
		if (choice < choices[i].m_value) {
			item = choices[i].m_key;
			break;
		}
	}
	return item;
}
