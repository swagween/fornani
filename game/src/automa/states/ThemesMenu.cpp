
#include "fornani/automa/states/ThemesMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

ThemesMenu::ThemesMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "themes") { m_parent_menu = MenuType::options; }

void ThemesMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
		for (auto& option : options) {
			if (current_selection.get() == option.index) {
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
				set_theme(svc, std::string{option.label.getString()});
			}
		}
	}
}

void ThemesMenu::frame_update(ServiceProvider& svc) {}

void ThemesMenu::render(ServiceProvider& svc, sf::RenderWindow& win) { MenuState::render(svc, win); }

} // namespace fornani::automa
