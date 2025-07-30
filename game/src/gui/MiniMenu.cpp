
#include "fornani/gui/MiniMenu.hpp"
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string> opt, sf::Vector2f start_position, bool white)
	: m_nineslice{svc, (white ? svc.assets.get_texture("cream_console") : svc.assets.get_texture("blue_console")), {28, 28}, {1, 1}} {
	auto ctr{0};
	for (auto& o : opt) {
		options.push_back(automa::Option(svc, o, white));
		options.back().index = ctr;
		options.back().update(svc, selection.get());
		++ctr;
	}
	selection = util::Circuit(static_cast<int>(options.size()));
	svc.soundboard.flags.console.set(audio::Console::menu_open);
}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2f dim, sf::Vector2f at_position) {
	dimensions = dim;
	m_nineslice.set_position(at_position);
	m_nineslice.set_dimensions(dim);
	auto spacing = 22.f;
	auto ctr{0};
	auto span = options.size();
	auto top_buffer = span * spacing / 4.f; // center the options
	for (auto& option : options) {
		auto ypos = m_nineslice.get_position().y + ctr * spacing - top_buffer;
		option.position = {m_nineslice.get_global_center().x, ypos};
		option.update(svc, selection.get());
		++ctr;
	}
}

void MiniMenu::render(sf::RenderWindow& win, bool bg) {
	if (bg) { m_nineslice.render(win); }
	for (auto& option : options) { win.draw(option.label); }
}

void MiniMenu::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) {
		selection.modulate(-1);
		soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) {
		selection.modulate(1);
		soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) {
		m_flags.set(MiniMenuFlags::selected);
		soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		m_flags.set(MiniMenuFlags::closed);
		soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
}

sf::Vector2f MiniMenu::get_dimensions() const { return dimensions; }

} // namespace fornani::gui
