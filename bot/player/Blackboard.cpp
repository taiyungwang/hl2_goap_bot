#include "Blackboard.h"

#include <player/Button.h>
#include <player/Bot.h>
#include <move/Navigator.h>
#include <weapon/Weapon.h>
#include <util/BasePlayer.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>

Blackboard::Blackboard(Bot *self, BasePlayer *entInstance) :
		self(self), entInstance(entInstance) {
	reset();
}

void Blackboard::reset() {
	blocker = nullptr;
	targetedPlayer = nullptr;
}

Blackboard::~Blackboard() {
	delete entInstance;
	if (navigator != nullptr) {
		delete navigator;
	}
	reset();
}

float Blackboard::getAimAccuracy(const Vector& pos) const {
	return (pos - self->getEyesPos()).Normalized().Dot(getFacing());
}

bool Blackboard::isOnLadder() {
	return entInstance->isOnLadder();
}

float Blackboard::getTargetEntDistance() const {
	return targetedPlayer == nullptr ?
			INFINITY :
			(targetedPlayer->getEdict()->GetCollideable()->GetCollisionOrigin()
					- self->getCurrentPosition()).Length();
}

void Blackboard::lookStraight() {
	this->viewTarget.z = this->self->getEyesPos().z;
}

Vector Blackboard::getFacing() const {
	Vector facing;
	AngleVectors(self->getFacingAngle(), &facing);
	return facing;
}


class FilterSelfAndEnemies: public CTraceFilter {
public:
	FilterSelfAndEnemies(edict_t* self,
			edict_t* target) : self(self), target(target) {
	}

	virtual ~FilterSelfAndEnemies() {
	}

	bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) {
		auto& players = Player::getPlayers();
		if (target != nullptr && pHandleEntity == target->GetIServerEntity()) {
			return true;
		}
		if (pHandleEntity == self->GetIServerEntity()) {
			return false;
		}
		FOR_EACH_MAP_FAST(players, i) {
			if (!players[i]->isDead()
					&& players[i]->getEdict()->GetIServerEntity()
							== pHandleEntity) {
				return false;
			}
		}
		return true;
	}

private:
	edict_t* self, *target;
};

bool Blackboard::checkVisible(trace_t& result, const Vector &vecAbsEnd, edict_t* target) const {
	if (target == nullptr) {
		return false;
	}
	result.fraction = 0.0f;
	Vector start = self->getEyesPos();
	FilterSelfAndTarget filter(self->getEdict()->GetIServerEntity(),
			target->GetIServerEntity());
	UTIL_TraceLine(start, vecAbsEnd, MASK_ALL, &filter, &result);
	return !result.DidHit();
}

bool Blackboard::checkVisible(const Vector &vecAbsEnd, edict_t* target) const {
	trace_t result;
	return checkVisible(result, vecAbsEnd, target);
}
