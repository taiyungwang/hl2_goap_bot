#include "Player.h"

#include <move/Navigator.h>
#include <weapon/Weapon.h>
#include <weapon/WeaponFunction.h>
#include <weapon/WeaponBuilder.h>
#include <nav_mesh/nav_mesh.h>
#include <nav_mesh/nav_area.h>
#include <util/BaseCombatWeapon.h>
#include <util/BasePlayer.h>
#include <shareddefs.h>
#include <eiface.h>
#include <iplayerinfo.h>
#include <server_class.h>
#include <in_buttons.h>
#include <string>

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

Player::Player(edict_t* ent, const WeaponBuilders& weaponBuilders) :
		ent(ent), weaponBuilders(weaponBuilders) {
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
		if (BasePlayer(ent).getVelocity()->Length() > 150.0f) {
			noiseRange = 100.0f;
		}
		int buttons = info->GetLastUserCommand().buttons;
		auto currWeap = getCurrWeapon();
		if (currWeap && (((buttons & IN_ATTACK) && !currWeap->getPrimary()->isSilent())
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

int Player::getCurrWeaponIdx() const {
	int index = 0;
	forMyWeapons([&index](edict_t *weaponEnt) mutable -> bool {
		if (WEAPON_IS_ACTIVE == BaseCombatWeapon(weaponEnt).getWeaponState()) {
			index = engine->IndexOfEdict(weaponEnt);
			return true;
		}
		return false;
	});
	return index;
}

std::shared_ptr<Weapon> Player::getCurrWeapon() {
	int index = getCurrWeaponIdx();
	if (index == 0) {
		return nullptr;
	}
	if (index != currWeapIdx) {
		currWeapIdx = index;
		edict_t* weaponEnt = engine->PEntityOfEntIndex(currWeapIdx);
		currentWeapon = weaponBuilders.at(weaponEnt->GetClassName()).get()->build(weaponEnt);
	}
	return currentWeapon;
}

std::shared_ptr<Weapon> Player::getWeapon(int id) const {
	std::shared_ptr<Weapon> weapon;
	forMyWeapons([&weapon, id, this](edict_t *weaponEnt) mutable -> bool {
		if (id == engine->IndexOfEdict(weaponEnt)) {
			weapon = weaponBuilders.at(weaponEnt->GetClassName()).get()->build(weaponEnt);
			return true;
		}
		return false;
	});
	return weapon;
}

int Player::getWeaponId(const std::string& name) const {
	int weaponIdx = 0;
	forMyWeapons([&weaponIdx, name, this](edict_t *weaponEnt) mutable -> bool {
		if (name == weaponEnt->GetClassName()) {
			weaponIdx = engine->IndexOfEdict(weaponEnt);
			return true;
		}
		return false;
	});
	return weaponIdx;
}

std::shared_ptr<Weapon> Player::getWeapon(const std::string& name) const {
	int weaponIdx = getWeaponId(name);
	return weaponIdx > 0 ? weaponBuilders.at(name).get()->build(engine->PEntityOfEntIndex(weaponIdx)):
			std::shared_ptr<Weapon>();
}

int Player::getBestWeapon(edict_t *target) const {
	edict_t *best = nullptr;
	float targetDist = target == nullptr || target->IsFree() ? -1.0f:
			target->GetCollideable()->GetCollisionOrigin().DistTo(getEyesPos());
	bool underWater = BasePlayer(getEdict()).isUnderWater();
	forMyWeapons([&best, targetDist, underWater, target, this](edict_t *weaponEnt) mutable
			-> bool {
		auto weapon = weaponBuilders.at(weaponEnt->GetClassName()).get()->build(weaponEnt);
		std::shared_ptr<Weapon> bestWeap;
		if (best != nullptr) {
			bestWeap = weaponBuilders.at(best->GetClassName()).get()->build(best);
		}
		if (weapon && !weapon->isUnderWater() && !underWater
				&& !(targetDist < 0.0f && weapon->getPrimary()->isMelee() && !underWater)
				// TODO: assumes primary and secondary weapons have similar ranges.
				&& !weapon->isGrenade() && !weapon->isOutOfAmmo(getEdict())
				&& (target == nullptr
				 || !weapon->isClipEmpty() && weapon->isInRange(targetDist))
				&& !(best && weapon->getDamage(getEdict(), targetDist)
								<= bestWeap->getDamage(getEdict(), targetDist))) {
			best = weaponEnt;
		}
		return false;
	});
	return engine->IndexOfEdict(best);

}

void Player::forMyWeapons(const std::function<bool(edict_t*)>& func) const {
	CBaseHandle* weapList = BaseEntity(ent).getPtr<CBaseHandle>("m_hMyWeapons");
	for (int i = 0; i < MAX_WEAPONS && weapList[i].IsValid(); i++) {
		int entIdx = weapList[i].GetEntryIndex();
		extern IVEngineServer* engine;
		edict_t* weaponEnt = engine->PEntityOfEntIndex(entIdx);
		if (weaponEnt == nullptr || weaponEnt->IsFree()) {
			continue;
		}
		bool isWeapon = false;
		// sometimes the handle is pointing to an entity that is not an weapon
		// like when a round ends/begins in dod.
		for (SendTable *st = weaponEnt->m_pNetworkable->GetServerClass()->m_pTable;
				st != nullptr && !isWeapon; ) {
			if (std::string(st->GetName()) == "DT_BaseCombatWeapon") {
				isWeapon = true;
			} else if (st->GetNumProps() == 0
					|| std::string(st->GetProp(0)->GetName()) != "baseclass") {
				st = nullptr;
			} else {
				st = st->GetProp(0)->GetDataTable();
			}
		}
		if (!isWeapon) {
			continue;
		}
		int weapState = BaseCombatWeapon(weaponEnt).getWeaponState();
		if (weapState == WEAPON_NOT_CARRIED) {
			continue;
		}
		if (func(weaponEnt)) {
			break;
		}
	}
}

QAngle Player::getFacingAngle() const {
	return info->GetLastUserCommand().viewangles;
}

QAngle Player::getAbsoluteAngle() const {
	return info->GetAbsAngles();
}
