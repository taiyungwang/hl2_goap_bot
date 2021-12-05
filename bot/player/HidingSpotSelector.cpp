#include "HidingSpotSelector.h"

#include "Bot.h"
#include <move/Navigator.h>
#include <event/EventInfo.h>
#include <nav_mesh/nav_area.h>
#include <filesystem.h>
#include <eiface.h>
#include <random>
#include <initializer_list>

extern IFileSystem *filesystem;

extern CGlobalVars *gpGlobals;

extern NavAreaVector TheNavAreas;

static const char *ROOT_KEY = "HidingSpot Selector";
static const char *TIME_STAMP_KEY = "navFileTimeStamp";
static const char *SPOTS_KEY = "spots";
static const char *FILE_PREFIX = "addons/mybot/";

HidingSpotSelector::HidingSpotSelector(CommandHandler &commandHandler) : Receiver(commandHandler) {
	buildFromNavMesh();
	KeyValues *file = new KeyValues(ROOT_KEY);
	navFileTimeStamp = filesystem->GetFileTime(getNavFileName().c_str(), "MOD");
	if (filesystem->IsDirectory((std::string(FILE_PREFIX)).c_str(), "MOD")
			&& file->LoadFromFile(filesystem, getHidingSpotFileName().c_str(), "MOD")
			&& file->GetInt(TIME_STAMP_KEY) == navFileTimeStamp) {
		FOR_EACH_TRUE_SUBKEY(file->FindKey(SPOTS_KEY), i) {
			auto &spot = spots[i->GetInt()];
			FOR_EACH_TRUE_SUBKEY(i, j) {
				auto &score = spot.score[std::stoi(j->GetName())];
				score.success = j->GetFloat("success");
				score.fail = j->GetFloat("fail");
			}
		}
	}
	file->deleteThis();
}

bool HidingSpotSelector::receive(edict_t *sender, const CCommand &command) {
	if (std::string("nav_save") == command.Arg(0)) {
		buildFromNavMesh();
	}
	return false;
}

bool HidingSpotSelector::handle(EventInfo* eventInfo) {
	if (std::string("player_death") == eventInfo->getName()) {
		for (auto player: { std::make_pair(Player::getPlayer(eventInfo->getInt("attacker")), true),
			std::make_pair(Player::getPlayer(eventInfo->getInt("userid")), false) }) {
			if (std::get<0>(player) == nullptr) {
				continue;
			}
			int spot = std::get<0>(player)->getClosestHidingSpot();
			if (spot >= 0) {
				update(spot, std::get<0>(player)->getTeam(), std::get<1>(player));
			}
		}
	}
	return false;
}

int HidingSpotSelector::select(Vector &pos, int team) const {
	float max = 0.0f;
	int selected = -1;
	int scoreIdx = team > 1 ? team - 2 : 0;
	static std::default_random_engine generator;
	for (auto i : spots) {
		const auto &score = std::get<1>(i).score[scoreIdx];
		if (score.inUse && team > 1) {
			continue;
		}
		float x =  std::gamma_distribution<float>(score.success, 1.0f)(generator);
		float betaSample = x / (x + std::gamma_distribution<float>(score.fail, 1.0f)(generator));
		if (betaSample > max) {
			selected = std::get<0>(i);
			max = betaSample;
			pos = std::get<1>(i).pos;
		}
	}
	return selected;
}

void HidingSpotSelector::setInUse(int spot, int team, bool inUse) {
	if (team > 1) {
		spots.at(static_cast<unsigned int>(spot)).score[team > 1 ? team - 2 : 0].inUse =
				inUse;
	}
}

void HidingSpotSelector::update(int spot, int team, bool success) {
	auto &score = spots[static_cast<unsigned int>(spot)].score[
			team > 1 ? team - 2 : 0];
	(success ? score.success : score.fail) += 1.0f;
}

void HidingSpotSelector::save() {
	if (spots.empty()) {
		return;
	}
	auto dirName = std::string(FILE_PREFIX);
	if (!filesystem->IsDirectory(dirName.c_str(), "MOD")) {
		filesystem->CreateDirHierarchy(dirName.c_str(), "DEFAULT_WRITE_PATH");
	}
	if (filesystem->GetFileTime(getNavFileName().c_str(), "MOD") != navFileTimeStamp) {
		// file was edited, so current score is invalid.
		buildFromNavMesh();
	}
	KeyValues *file = new KeyValues(ROOT_KEY);
	file->SetInt(TIME_STAMP_KEY, filesystem->GetFileTime(getNavFileName().c_str(), "MOD"));
	auto spotsKV = file->FindKey(SPOTS_KEY, true);
	for (auto spot : spots) {
		auto spotKV = new KeyValues(std::to_string(std::get<0>(spot)).c_str());
		spotsKV->AddSubKey(spotKV);
		for (int i = 0; i < 2; i++) {
			auto scoreKv = new KeyValues(std::to_string(i).c_str());
			auto &score = std::get<1>(spot).score;
			spotKV->AddSubKey(scoreKv);
			scoreKv->SetFloat("success", score[i].success);
			scoreKv->SetFloat("fail", score[i].fail);
		}
	}
	file->SaveToFile(filesystem, getHidingSpotFileName().c_str(), "MOD");
	file->deleteThis();
}

std::string HidingSpotSelector::getNavFileName() {
	return std::string("maps/") + +gpGlobals->mapname.ToCStr() + ".nav";
}

std::string HidingSpotSelector::getHidingSpotFileName() {
	return std::string(FILE_PREFIX)
			+ gpGlobals->mapname.ToCStr() + "_hiding_spot_scores.vdf";
}

void HidingSpotSelector::buildFromNavMesh() {
	spots.clear();
	FOR_EACH_VEC(TheNavAreas, i)
	{
		const auto &hideSpots = *TheNavAreas[i]->GetHidingSpots();
		if (TheNavAreas[i]->IsBlocked(TEAM_ANY)) {
			continue;
		}
		FOR_EACH_VEC(hideSpots, j)
		{
			auto &spot = spots[hideSpots[j]->GetID()] = Spot();
			spot.pos = hideSpots[j]->GetPosition();
			for (int k = 0; k < 2; k++) {
				if (TheNavAreas[i]->IsBlocked(k)) {
					spot.score[i].inUse = true;
				}
			}
		}
	}
}

