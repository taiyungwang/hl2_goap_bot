#include "HidingSpotSelector.h"

#include <navmesh/nav_area.h>

HidingSpotSelector::HidingSpotSelector() {
	extern NavAreaVector TheNavAreas;
	FOR_EACH_VEC(TheNavAreas, i) {
		const auto& hideSpots = *TheNavAreas[i]->GetHidingSpots();
		FOR_EACH_VEC(hideSpots, j) {
			spots.AddToTail();
			spots.Tail().pos = hideSpots[j]->GetPosition();
		}
	}
}

int HidingSpotSelector::select(Vector& pos, int team) {
	float max = 0.0f;
	int selected = -1;
	int scoreIdx = team > 1 ? team - 2 : 0;
	FOR_EACH_VEC(spots, i) {
		const auto& score = spots[i].score[scoreIdx];
		if (score.inUse) {
			continue;
		}
		// Beta sampling.
		float sample = tgammaf(score.success);
		sample /= sample + tgammaf(score.fail);
		if (sample > max) {
			selected = i;
			max = sample;
			pos = spots[i].pos;
		}
	}
	if (selected >= 0 && team > 1) {
		spots[selected].score[scoreIdx].inUse = true;
	}
	return selected;
}

void HidingSpotSelector::update(int spot, int team, bool success) {
	auto& score = spots[spot].score[team > 1 ? team - 2 : 0];
	success ? score.success += 1.0f : score.fail += 1.0f;
}

