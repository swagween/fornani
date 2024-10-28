#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"

namespace automa {

enum class MenuStatus { vendor };

class MenuController {
  public:
	void open_vendor_dialog(int id);
	[[nodiscard]] auto vendor_dialog_opened() -> bool { return status.consume(MenuStatus::vendor); }
	[[nodiscard]] auto get_menu_id() const -> int { return menu_id; }
  private:
	util::BitFlags<MenuStatus> status{};
	int menu_id{};
};

}
