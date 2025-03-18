
#pragma once

#include "fornani/entities/item/Item.hpp"

namespace fornani::item {

class ApparelItem : public Item {
  public:
	ApparelItem(dj::Json& source, std::string_view label);
};

} // namespace fornani::item
