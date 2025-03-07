
#pragma once

#include "fornani/entities/item/Item.hpp"

namespace fornani::item {

class KeyItem : public Item {
  public:
	KeyItem(dj::Json& source, std::string_view label);
};

} // namespace fornani::item
