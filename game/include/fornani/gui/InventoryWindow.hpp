
#pragma once
#include "Console.hpp"
#include "MiniMap.hpp"
#include "MiniMenu.hpp"
#include "Selector.hpp"
#include "WardrobeWidget.hpp"
#include "fornani/io/Logger.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::item {
class Item;
}

namespace fornani::gui {

class InventoryWindow {
	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
