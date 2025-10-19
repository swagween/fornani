
#include <fornani/automa/MenuState.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

constexpr auto dot_buffer_v = 16.f;

MenuState::MenuState(ServiceProvider& svc, player::Player& player, std::string_view scene) : GameState(svc, player, scene) {
	auto const& in_data = svc.data.menu["options"];
	for (auto& entry : in_data[scene].as_array()) { options.push_back(Option(svc, entry.as_string())); }
	if (!options.empty()) { current_selection = util::Circuit(static_cast<int>(options.size())); }

	top_buffer = svc.data.menu["config"][scene]["top_buffer"].as<float>();
	int ctr{};
	for (auto& option : options) {
		option.position = {svc.window->i_screen_dimensions().x * 0.5f, top_buffer + ctr * spacing};
		option.index = ctr;
		option.update(svc, current_selection.get());
		++ctr;
	}

	auto dot_size = 4.f;
	auto which = 0;
	auto center = options.at(current_selection.get()).label.getGlobalBounds().getCenter();
	auto width_offset = options.at(current_selection.get()).label.getLocalBounds().size.x * 0.5f;
	auto offset = sf::Vector2f{width_offset + dot_buffer_v, 0.f};
	for (auto& dot : m_dot_indicators) {
		auto target = which == 0 ? center - offset : center + offset;
		dot.physics.position = target;
		dot.physics.set_global_friction(0.85f);
		dot.rect.setPosition(dot.physics.position);
		dot.rect.setFillColor(colors::bright_orange);
		dot.rect.setSize({dot_size, dot_size});
		dot.rect.setOrigin(dot.rect.getSize() * 0.5f);
		++which;
	}
}

void MenuState::tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);
	svc.controller_map.set_action_set(config::ActionSet::Menu);
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered && m_input_authorized) {
		current_selection.modulate(1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered && m_input_authorized) {
		current_selection.modulate(-1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered && m_input_authorized) {
		svc.state_controller.submenu = m_parent_menu;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	auto which = 0;
	auto center = options.at(current_selection.get()).label.getGlobalBounds().getCenter();
	auto width_offset = options.at(current_selection.get()).label.getLocalBounds().size.x * 0.5f;
	auto offset = sf::Vector2f{width_offset + dot_buffer_v, 0.f};
	for (auto& dot : m_dot_indicators) {
		auto target = which == 0 ? center - offset : center + offset;
		m_steering.seek(dot.physics, target);
		dot.physics.simple_update();
		dot.rect.setPosition(dot.physics.position);
		++which;
	}
}

void MenuState::render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win) {
	for (auto& dot : m_dot_indicators) { win.draw(dot.rect); }
	for (auto& option : options) { win.draw(option.label); }
}

} // namespace fornani::automa
