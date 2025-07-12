
#pragma once

#include "fornani/entities/item/Item.hpp"

namespace fornani::item {

class GizmoItem : public Item {
  public:
	GizmoItem(dj::Json& source, std::string_view label);
};

} // namespace fornani::item
