
#include "fornani/entities/item/types/KeyItem.hpp"

namespace fornani::item {

KeyItem::KeyItem(dj::Json& source, std::string_view label) : Item(source, label, ItemType::key) {}

} // namespace fornani::item
