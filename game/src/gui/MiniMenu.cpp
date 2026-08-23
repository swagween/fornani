
#include "fornani/gui/MiniMenu.hpp"
#include "fornani/automa/GameState.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

constexpr auto menu_spacing_v = 22.f;

MiniMenu::MiniMenu(automa::ServiceProvider& svc, std::vector<std::string> opt, sf::Vector2f start_position, MenuTheme& theme)
	: m_nineslice{svc, (theme.label == "mini_white" ? svc.assets.get_texture("cream_minimenu") : svc.assets.get_texture("blue_minimenu")), {13, 13}, {1, 1}}, m_theme{&theme}, m_open{8} {
	m_nineslice.set_position(start_position + random::random_vector_float({-16.f, -8.f}, {-8.f, 8.f}));
	options.reserve(opt.size());
	for (auto [i, option] : std::views::enumerate(opt)) {
		options.push_back(automa::Option(svc, theme, option));
		options.back().index = i;
	}

	selection = util::Circuit(static_cast<int>(options.size()));
	svc.soundboard.flags.console.set(audio::Console::menu_open);
	m_open.start();
}

void MiniMenu::update(automa::ServiceProvider& svc, sf::Vector2f at_position) {
	m_open.update();

	auto buffer = 84.f;
	at_position.y = std::clamp(at_position.y, buffer, svc.window->f_screen_dimensions().y - buffer - get_dimensions().y);
	m_attributes.test(MiniMenuAttributes::no_ease) ? m_nineslice.set_position(at_position) : m_nineslice.target_position(at_position, 0.003f);
	m_flags.reset(MiniMenuFlags::option_hovered);

	auto enclosing_rect = sf::FloatRect{};
	for (auto [i, option] : std::views::enumerate(options)) {
		option.update(selection.get());

		enclosing_rect.size.x = std::max(enclosing_rect.size.x, option.label.getGlobalBounds().size.x);
		enclosing_rect.size.y += menu_spacing_v;

		if (option.label.getGlobalBounds().contains(svc.input_system.get_mouse_position()) && svc.input_system.is_mouse_active()) {
			if (selection.get() != i) { svc.soundboard.play_sound("menu_shift"); }
			selection.set(i);
			m_flags.set(MiniMenuFlags::option_hovered);
		}
	}
	auto const border = sf::Vector2f{16.f, 26.f};
	auto to_dim = enclosing_rect.size - m_nineslice.get_f_corner_dimensions() * 2.f + border;
	to_dim.x = std::max(to_dim.x, 2.f);
	to_dim.y = std::max(to_dim.y, 2.f);
	m_nineslice.set_dimensions(to_dim);
}

void MiniMenu::render(sf::RenderWindow& win, bool bg) {
	if (bg) { m_nineslice.render(win); }
	auto const center = m_nineslice.get_global_center().y;
	auto const offset = (static_cast<float>(options.size()) - 1.f) * menu_spacing_v * 0.5f;
	for (auto [i, option] : std::views::enumerate(options)) {
		auto const ypos = center - offset + static_cast<float>(i) * menu_spacing_v;
		option.position = {m_nineslice.get_global_center().x, ypos};
		if (m_open.running()) { continue; }
		win.draw(option.label);
	}
}

void MiniMenu::render(sf::RenderWindow& win, sf::Vector2f cam) {
	m_nineslice.render(win, cam);
	auto const center = m_nineslice.get_global_center().y;
	auto const offset = (static_cast<float>(options.size()) - 1.f) * menu_spacing_v * 0.5f;
	for (auto [i, option] : std::views::enumerate(options)) {
		auto const ypos = center - offset + static_cast<float>(i) * menu_spacing_v;
		option.position = sf::Vector2f{m_nineslice.get_global_center().x, ypos} - cam;
		option.update(selection.get());
		if (m_open.running()) { continue; }
		win.draw(option.label);
	}
}

void MiniMenu::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.menu_move(input::MoveDirection::up)) {
		selection.modulate(-1);
		soundboard.play_sound("menu_shift");
	}
	if (controller.menu_move(input::MoveDirection::down)) {
		selection.modulate(1);
		soundboard.play_sound("menu_shift");
	}
	if (controller.digital(input::DigitalAction::menu_select).triggered || (controller.is_mouse_active() && is_mouse_hovering_option() && controller.left_clicked())) {
		m_flags.set(MiniMenuFlags::selected);
		soundboard.flags.menu.set(audio::Menu::forward_switch);
	}
	if (controller.digital(input::DigitalAction::menu_back).triggered) {
		m_flags.set(MiniMenuFlags::closed);
		soundboard.play_sound("menu_back");
	}
}

sf::Vector2f MiniMenu::get_dimensions() const { return m_nineslice.get_f_dimensions(); }

} // namespace fornani::gui
