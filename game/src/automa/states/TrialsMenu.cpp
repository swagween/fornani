
#include <fornani/automa/states/TrialsMenu.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

TrialsMenu::TrialsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "trials") { m_parent_menu = MenuType::play; }

void TrialsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		switch (current_selection.get()) {
		case 0: break;
		}
	}

	for (auto& option : options) { option.update(svc, current_selection.get()); }
}

void TrialsMenu::frame_update(ServiceProvider& svc) {}

void TrialsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) { MenuState::render(svc, win); }

} // namespace fornani::automa
