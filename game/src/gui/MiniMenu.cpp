
#include "fornani/gui/MiniMenu.hpp"
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string> opt, sf::Vector2f start_position, automa::MenuTheme& theme)
	: m_nineslice{svc, (theme.label == "mini_white" ? svc.assets.get_texture("cream_console") : svc.assets.get_texture("blue_console")), {28, 28}, {1, 1}}, m_theme{&theme} {
	m_nineslice.set_position(start_position + random::random_vector_float({-16.f, -8.f}, {-8.f, 8.f}));
	options.reserve(opt.size());
	for (auto [i, option] : std::views::enumerate(opt)) {
		options.push_back(automa::Option(svc, theme, option));
		options.back().index = i;
	}

	selection = util::Circuit(static_cast<int>(options.size()));
	svc.soundboard.flags.console.set(audio::Console::menu_open);
}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2f dim, sf::Vector2f at_position) {
	dimensions = dim;
	auto buffer = 84.f;
	at_position.y = std::clamp(at_position.y, buffer, svc.window->f_screen_dimensions().y - buffer);
	m_attributes.test(MiniMenuAttributes::no_ease) ? m_nineslice.set_position(at_position) : m_nineslice.target_position(at_position, 0.003f);
	auto spacing = 22.f;
	auto ctr{0};
	auto span = options.size();
	auto top_buffer = (span - 1) * spacing / 2.f;
	auto largest_option = sf::Vector2f{};
	for (auto& option : options) {
		auto ypos = m_nineslice.get_position().y + ctr * spacing - top_buffer;
		option.position = {m_nineslice.get_global_center().x, ypos};
		option.update(selection.get());
		if (auto to = option.label.getLocalBounds().size.x > largest_option.x) { largest_option.x = to; }
		if (ctr > 2) { largest_option.y += option.label.getLocalBounds().size.y; }
		++ctr;
	}
	m_nineslice.set_dimensions(largest_option);
}

void MiniMenu::render(sf::RenderWindow& win, bool bg) {
	if (bg) { m_nineslice.render(win); }
	for (auto& option : options) { win.draw(option.label); }
}

void MiniMenu::render(sf::RenderWindow& win, sf::Vector2f cam) {
	m_nineslice.render(win, cam);
	auto spacing = 22.f;
	auto span = options.size();
	auto top_buffer = (span - 1) * spacing / 2.f;
	for (auto [i, option] : std::views::enumerate(options)) {
		auto ypos = m_nineslice.get_position().y + i * spacing - top_buffer;
		option.position = sf::Vector2f{m_nineslice.get_global_center().x, ypos} - cam;
		option.update(selection.get());
		win.draw(option.label);
	}
}

void MiniMenu::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.menu_move(input::MoveDirection::up)) {
		selection.modulate(-1);
		soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.menu_move(input::MoveDirection::down)) {
		selection.modulate(1);
		soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (controller.digital(input::DigitalAction::menu_select).triggered) {
		m_flags.set(MiniMenuFlags::selected);
		soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	if (controller.digital(input::DigitalAction::menu_back).triggered) {
		m_flags.set(MiniMenuFlags::closed);
		soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
}

sf::Vector2f MiniMenu::get_dimensions() const { return dimensions; }

} // namespace fornani::gui
