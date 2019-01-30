#include "Blackboard.h"

#include <player/Button.h>
#include <player/Bot.h>
#include <move/Navigator.h>
#include <weapon/Weapon.h>
#include <util/BaseEntity.h>
#include <util/UtilTrace.h>
#include <util/EntityUtils.h>

float Blackboard::clamp180(float angle) {
	if (angle > 180.0f) {
		angle -= 360.0f;
	} else if (angle < -180.0f) {
		angle += 360.0f;
	}
	return angle;
}

Blackboard::Blackboard(const CUtlMap<int, Player*>& players,
		Bot* self, EntityInstance* entInstance) : players(players),
		self(self), entInstance(entInstance) {
	reset();
}

void Blackboard::reset() {
	blocker = nullptr;
	targetedPlayer = nullptr;
	armory.reset();
}

Blackboard::~Blackboard() {
	delete entInstance;
	if (navigator != nullptr) {
		delete navigator;
	}
	reset();
}

float Blackboard::getAimAccuracy(const Vector& pos) const {
	return (pos - self->getEyesPos()).Normalized().Dot(self->getFacing());
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

