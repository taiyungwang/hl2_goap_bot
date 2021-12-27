#pragma once

#include <unordered_map>
#include <tuple>

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
	HEARD_NEED_AMMO,
	// HL2DM
	NEED_ITEM,
	PROP_COUNT
};

using GoalState = std::tuple<WorldProp, bool>;

struct EnumClassHash {
	template<typename T>
	std::size_t operator()(T t) const {
		return static_cast<std::size_t>(t);
	}
};

template<typename Key>
using HashType = typename std::conditional<std::is_enum<Key>::value, EnumClassHash, std::hash<Key>>::type;

template <typename Key, typename T>
using MyUnorderedMap = std::unordered_map<Key, T, HashType<Key>>;
/**
 * Map of world property types to their sates.
 */
using WorldState = MyUnorderedMap<WorldProp, bool>;
