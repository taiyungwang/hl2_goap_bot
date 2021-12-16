#pragma once

#include <memory>

class Bot;
struct edict_t;

class Item {
public:
	Item(edict_t *ent) :
			ent(ent), charger(false) {
	}

	virtual ~Item() {
	}

	virtual bool isAvailable() const;

	edict_t *getEnt() const {
		return ent;
	}

	bool isCharger() const {
		return charger;
	}

protected:
	edict_t *ent = nullptr;
	bool charger;
};

class Charger: public Item {
public:
	Charger(edict_t *ent): Item(ent) {
		charger = true;
	}

	bool isAvailable() const override;
};

class ItemBuilder {
public:
	virtual ~ItemBuilder() {
	}

	virtual bool need(const Bot &bot) const = 0;

	virtual std::shared_ptr<Item> build(edict_t *ent) const {
		return std::make_shared<Item>(ent);
	}
};

class ChargerBuilder: public ItemBuilder {
public:
	std::shared_ptr<Item> build(edict_t *ent) const override {
		return std::make_shared<Charger>(ent);
	}
};
