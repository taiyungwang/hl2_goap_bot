#pragma once

#include "Item.h"
#include <unordered_map>
#include <list>
#include <string>
#include <set>

class Bot;
class Vector;
class CNavArea;
struct edict_t;

class ItemMap {
public:
	std::shared_ptr<Item> getClosestNeededItem(CNavArea *area,
			const Bot &self) const;

	template<typename T>
	void addItemBuilder(const char *itemName) {
		builders[itemName] = std::make_shared<T>();
	}

	void buildMap();

private:
	std::unordered_map<int, std::list<std::shared_ptr<Item>>> items;

	std::unordered_map<std::string, std::shared_ptr<ItemBuilder>> builders;

	std::set<int> ignored;
};
