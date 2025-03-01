#include "GoToAction.h"

#include <move/Navigator.h>
#include <move/NavMeshPathBuilderWithGoal.h>
#include <player/Blackboard.h>
#include <player/Bot.h>
#include <nav_mesh/nav_area.h>
#include <ivdebugoverlay.h>

float GoToAction::getCost() {
	return targetLoc.DistTo(blackboard.getSelf()->getCurrentPosition());
}

bool GoToAction::init() {
	auto self = blackboard.getSelf();
	Navigator::Path path;
	int team = self->getTeam();
	extern ConVar mybot_debug;
	CNavArea *goal = Navigator::getArea(targetLoc, team);
	if (goal == nullptr) {
		if (mybot_debug.GetBool()) {
			Msg("Unable to find area for goal.\n");
			extern IVDebugOverlay *debugoverlay;
			debugoverlay->AddLineOverlay(self->getCurrentPosition(),
					targetLoc, 255, 0, 0, true,
					NDEBUG_PERSIST_TILL_NEXT_SERVER);
		}
		return false;
	}
	NavMeshPathBuilderWithGoal(team, goal).build(path,
			self->getArea());
	if (!path.empty()) {
		self->getNavigator()->getPath().swap(path);
		self->getNavigator()->start(targetLoc, targetRadius, sprint);
		return true;
	}
	if (mybot_debug.GetBool()) {
		Msg("Unable to find a path for goal.\n");
		extern IVDebugOverlay *debugoverlay;
		debugoverlay->AddLineOverlay(self->getCurrentPosition(),
				targetLoc, 255, 0, 0, true,
				NDEBUG_PERSIST_TILL_NEXT_SERVER);
	}
	return false;
}

bool GoToAction::execute() {
	canAbort = !blackboard.getSelf()->isOnLadder();
	return blackboard.getSelf()->getNavigator()->step();
}

bool GoToAction::goalComplete() {
	return blackboard.getSelf()->getNavigator()->reachedGoal();
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
		auto *collide = active[i]->GetCollideable();
		if (collide == nullptr) {
			continue;
		}
		choices.AddToTail();
		choices.Tail().m_key = active[i];
		choices.Tail().m_value = 1.0f / collide->GetCollisionOrigin().DistTo(blackboard.getSelf()->getCurrentPosition());
		totalDist += choices.Tail().m_value;
	}
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
