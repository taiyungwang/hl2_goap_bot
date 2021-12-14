#pragma once

#include "ItemBuilder.h"

class ChargerBuilder: public ItemBuilder {
	std::shared_ptr<Item> build(edict_t *ent) const override;
};
