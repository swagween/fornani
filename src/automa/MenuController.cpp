
#include "MenuController.hpp"

namespace automa {

void MenuController::open_vendor_dialog(int id) {
	status.set(MenuStatus::vendor);
	menu_id = id;
}

} // namespace automa
