#pragma once

#include <utlmap.h>
#include <utlcommon.h>

enum class WorldProp {
	ENEMY_SIGHTED,
	MULTIPLE_ENEMY_SIGHTED,
	USING_BEST_WEAP,
	USING_DESIRED_WEAPON,
	IS_BLOCKED,
	WEAPON_LOADED,
	OUT_OF_AMMO,
	HURT,
	WEAPON_IN_RANGE,
	HEALTH_FULL,
	HEARD_AREA_CLEAR,
	EXPLOSIVE_NEAR,
	// DOD
	ROUND_STARTED,
	POINTS_DEFENDED,
	ALL_POINTS_CAPTURED,
	HAS_BOMB,
	BOMB_DEFUSED,
	HAS_LIVE_GRENADE,
	// HL2DM
	ARMOR_FULL,
	PROP_COUNT
};

typedef CUtlKeyValuePair<WorldProp, bool> GoalState;

/**
 * Map of world property types to their sates.
 */
typedef CUtlMap<WorldProp, bool> WorldState;
