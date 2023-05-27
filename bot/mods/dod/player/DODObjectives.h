#pragma once

#include <player/GameEventListener.h>
#include <utllinkedlist.h>
#include <unordered_map>
#include <vector>
#include <memory>

class DODObjective;
struct edict_t;

class DODObjectives: public GameEventListener {
public:
	DODObjectives();

	~DODObjectives() {
		endRound();
	}

	/**
	 * @return True if the current map is a detonation type map.
	 */
	bool isDetonation() const {
		return detonation;
	}

	const DODObjective* getObjective(edict_t*) const;

	const CUtlLinkedList<edict_t*>& getCtrlPts() const {
		return ctrlPts;
	}

	void FireGameEvent(IGameEvent* event);

private:
	bool detonation;

	std::unordered_map<edict_t*, int> ctrlPointsMap;

	CUtlLinkedList<edict_t*> ctrlPts;

	std::vector<std::shared_ptr<DODObjective>> objectives;

	void addCapTarget(const Vector &pos,
			const CUtlLinkedList<edict_t*> &targets);

	/**
	 * Callback for when a round starts
	 */
	void startRound();

	/**
	 * Callback for when a round ends.
	 */
	void endRound();
};
