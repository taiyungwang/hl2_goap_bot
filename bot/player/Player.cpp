#include "Player.h"

#include <move/Navigator.h>
#include <weapon/Arsenal.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <nav_mesh/nav_mesh.h>
#include <nav_mesh/nav_area.h>
#include <util/BasePlayer.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <in_buttons.h>

std::unordered_map<int, Player*> Player::players;

static ConVar mybotDangerAmt("mybot_danger_amount", "3.0f", 0,
		"Amount of 'danger' to add to an area for a team when a teammate is killed.");

extern IVEngineServer* engine;

Player* Player::getPlayer(edict_t* ent) {
	return getPlayer(engine->IndexOfEdict(ent));
}

Player::TeamCount Player::getTeamCount() {
	int count[] = {0, 0};
	for (auto pair: players) {
		int team = pair.second->getTeam();
		if (team == 1) {
			continue;
		}
		count[team % 2]++;
	}
	return std::make_tuple(count[0], count[1]);
}

Player *Player::getPlayerByUserId(int userId) {
	for (auto pair: players) {
		if (pair.second->getUserId() == userId) {
			return pair.second;
		}
	}
	return nullptr;
}

Player::Player(edict_t* ent, const std::shared_ptr<Arsenal>& arsenal) :
		ent(ent), arsenal(arsenal) {
	extern IPlayerInfoManager *playerinfomanager;
	listenForGameEvent({"player_spawn", "player_death"});
	info = playerinfomanager->GetPlayerInfo(ent);
	players[engine->IndexOfEdict(ent)] = this;
}

Player::~Player() {
	// Cant' rely on server index for server shutting down.
	for(auto player: players) {
		if (player.second == this) {
			players.erase(player.first);
			return;
		}
	}
}

const char* Player::getName() const {
	return info->GetName();
}

int Player::getTeam() const {
	return info->GetTeamIndex();
}

bool Player::isEnemy(const Player& other) const {
	return &other != this && other.inGame && (getTeam() == 0 || other.getTeam() != getTeam());
}

Vector Player::getCurrentPosition() const {
	return ent->GetCollideable()->GetCollisionOrigin();
}

const char* Player::getWeaponName() const {
	return info->GetWeaponName();
}

void Player::think() {
	if (inGame && (ent->IsFree() || info->IsDead() || info->GetHealth() <= 0)) {
		inGame = false;
	}
	if (inGame) {
		noiseRange = 0.0f;
		arsenal->update(ent);
		if (BasePlayer(ent).getVelocity()->Length() > 150.0f) {
			noiseRange = 100.0f;
		}
		int buttons = info->GetLastUserCommand().buttons;
		Weapon *currWeap = arsenal->getCurrWeapon();
		if (currWeap != nullptr
				&& (((buttons & IN_ATTACK) && !currWeap->getPrimary()->isSilent())
						|| ((buttons & IN_ATTACK2) && currWeap->getSecondary() != nullptr
								&& !currWeap->getSecondary()->isSilent()))) {
			noiseRange = 1000.0f;
		}
	}
}

int Player::getUserId() const {
	extern IVEngineServer* engine;
	return engine->GetPlayerUserId(ent);
}

Vector Player::getEyesPos() const {
	Vector pos;
	extern IServerGameClients* gameclients;
	gameclients->ClientEarPosition(ent, &pos);
	return pos;
}

int Player::getArmor() const {
	return info->GetArmorValue();
}

int Player::getHealth() const {
	return info->GetHealth();
}

int Player::getMaxHealth() const {
	return info->GetMaxHealth();
}

void Player::FireGameEvent(IGameEvent* event) {
	std::string name(event->GetName());
	int eventUserId = event->GetInt("userid");
	if (eventUserId != getUserId()) {
		return;
	}
	// bot owns this event.
	if (name == "player_spawn") {
		arsenal->reset();
		inGame = true;
	}  else if (name == "player_death") {
		inGame = false;
		auto area = getArea();
		if (area != nullptr) {
			area->IncreaseDanger(getTeam(), mybotDangerAmt.GetFloat());
		}
	}
}

CNavArea *Player::getArea() const {
	if (!inGame) {
		return nullptr;
	}
	extern CNavMesh* TheNavMesh;
	auto area = TheNavMesh->GetNavArea(ent, 0);
	if (area == nullptr) {
		area = Navigator::getArea(getCurrentPosition(), getTeam());
	}
	return area;
}

int Player::getClosestHidingSpot() const {
	int spot = -1;
	auto area = getArea();
	if (area != nullptr) {
		auto spots = area->GetHidingSpots();
		float closest = INFINITY;
		FOR_EACH_VEC(*spots, i) {
			float dist = (*spots)[i]->GetPosition().DistTo(getCurrentPosition());
			if (dist < closest) {
				spot = (*spots)[i]->GetID();
				closest = dist;
			}
		}
	}
	return spot;
}


QAngle Player::getFacingAngle() const {
	return info->GetLastUserCommand().viewangles;
}

QAngle Player::getAbsoluteAngle() const {
	return info->GetAbsAngles();
}
