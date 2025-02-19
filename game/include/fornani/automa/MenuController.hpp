#pragma once

#include "fornani/utils/BitFlags.hpp"

#include <cstdint>

namespace fornani::automa {

enum class MenuStatus : std::uint8_t { vendor };

class MenuController {
  public:
	void open_vendor_dialog(int id);
	void reset_vendor_dialog();
	[[nodiscard]] auto vendor_dialog_opened() -> bool { return status.consume(MenuStatus::vendor); }
	[[nodiscard]] auto get_menu_id() const -> int { return menu_id; }
  private:
	util::BitFlags<MenuStatus> status{};
	int menu_id{};
};
} // namespace fornani::automa
