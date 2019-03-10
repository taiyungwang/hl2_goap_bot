#include "HidingSpotSelector.h"

#include <navmesh/nav_area.h>

HidingSpotSelector::HidingSpotSelector() {
	extern NavAreaVector TheNavAreas;
	FOR_EACH_VEC(TheNavAreas, i) {
		const auto& hideSpots = *TheNavAreas[i]->GetHidingSpots();
		if (TheNavAreas[i]->IsBlocked(TEAM_ANY)) {
			continue;
		}
		FOR_EACH_VEC(hideSpots, j) {
			spots.AddToTail();
			spots.Tail().pos = hideSpots[j]->GetPosition();
			for (int i = 0; i < 2; i++) {
				if (TheNavAreas[i]->IsBlocked(i)) {
					spots.Tail().score[i].inUse = true;
				}
			}
		}
	}
}

/**
 * Borrowed from https://jamesmccaffrey.wordpress.com/2017/11/01/more-on-sampling-from-the-beta-distribution-using-c/
 */
float beta_sample(float a, float b) {
	float alpha = a + b,
			beta =
			MIN(a, b) <= 1.0f ?
					MAX(1.0f / a, 1.0f / b) :
					sqrtf((alpha - 2.0f) / (2.0f * a * b - alpha)),
			w = 0.0f,
			gamma = a + 1.0f / beta;
	for (;;) {
		float u1 = RandomFloat(0, 1.0f),
				u2 = RandomFloat(0, 1.0f),
				v = beta * logf(u1 / (1.0f - u1));
		w = a * exp(v);
		if (alpha * logf(alpha / (b + w)) + (gamma * v) - 1.3862944
				>= logf(u1 * u1 * u2)) {
			break;
		}
	}
	return w / (b + w);
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
		float sample = beta_sample(score.success, score.fail);
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

