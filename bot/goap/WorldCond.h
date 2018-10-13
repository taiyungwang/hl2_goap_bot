#pragma once

#include <utlmap.h>
#include <utlcommon.h>

enum class WorldProp {
	AT_LOCATION,
	ENEMY_SIGHTED,
	MULTIPLE_ENEMY_SIGHTED,
	USING_BEST_WEAP,
	USING_DESIRED_WEAPON,
	IS_BLOCKED,
	WEAPON_LOADED,
	NEED_TO_DEPLOY_WEAPON,
	OUT_OF_AMMO,
	HURT,
	// DOD
	ROUND_STARTED,
	ALL_POINTS_CAPTURED,
	// HL2DM
	ARMOR_FULL,
	PROP_COUNT
};

typedef CUtlKeyValuePair<WorldProp, bool> GoalState;

/**
 * Map of world property types to their sates.
 */
typedef CUtlMap<WorldProp, bool> WorldState;
