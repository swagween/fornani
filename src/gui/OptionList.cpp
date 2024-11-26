#include "OptionList.hpp"

#include <utility>
#include "SFML/Graphics/RenderStates.hpp"

namespace gui {

OptionList::OptionList(automa::ServiceProvider& svc) : option_font(svc.text.fonts.title), deselected_color(svc.styles.colors.dark_grey), hovering_color(svc.styles.colors.ui_white) {
	constexpr float dot_force{0.9f};
	constexpr float dot_fric{0.86f};
	constexpr float dot_speed{200.f};

	left_dot = vfx::Gravitator({0.f, 0.f}, svc.styles.colors.bright_orange, dot_force);
	right_dot = vfx::Gravitator({0.f, 0.f}, svc.styles.colors.bright_orange, dot_force);

	left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
	right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{dot_fric, dot_fric}, 1.0f);
	right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(dot_speed, dot_speed);
}

void OptionList::push_option(std::string_view label, std::function<void()> on_select, bool enabled) {
	auto text = sf::Text(std::string(label), option_font, 16);
	text.setFillColor(hovering_color);
	options.emplace_back(Option{.text = text, .enabled = enabled, .on_select = std::move(on_select)});

	// Reposition options
	float option_max_width = 0.f;
	for (auto const& option : options) {
		auto const width = option.text.getLocalBounds().width;
		if (width > option_max_width) { option_max_width = width; }
	}
	float current_y = 0.f;
	for (auto& option : options) {
		auto const width = option.text.getLocalBounds().width;
		option.text.setPosition({(option_max_width - width) / 2.f, current_y});
		current_y += option.text.getLocalBounds().height + vertical_spacing;
	}

	// Recalculate bounds
	bounds = sf::FloatRect({0.f, 0.f}, sf::Vector2f{option_max_width, current_y});

	// Set number of options available
	selected_option.set_order(options.size());
}

void OptionList::update(automa::ServiceProvider& svc, bool enable_input, bool update_action_ctx_bar) {
	if (selected_option.get() < options.size()) {
		options[selected_option.get()].text.setFillColor(deselected_color);
		if (enable_input) {
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
				selected_option.modulate(1);
				svc.soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
				selected_option.modulate(-1);
				svc.soundboard.flags.menu.set(audio::Menu::shift);
			}
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
				options[selected_option.get()].on_select();
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			}
		}
		options[selected_option.get()].text.setFillColor(hovering_color);

		constexpr float dot_spacing{24.f};
		auto selection_bounds = options.at(selected_option.get()).text.getGlobalBounds();
		auto left = selection_bounds.left;
		auto right = left + selection_bounds.width;
		auto middle_y = selection_bounds.top + selection_bounds.height / 2.f;

		left_dot.set_target_position({left - dot_spacing, middle_y});
		right_dot.set_target_position({right + dot_spacing, middle_y});
	}

	left_dot.update(svc);
	right_dot.update(svc);
}

void OptionList::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f extra_offset) {
	auto offset = (sf::Vector2f(win.getSize()) - bounds.getSize()) / 2.f + extra_offset;

	for (auto const& option : options) { win.draw(option.text, sf::RenderStates(sf::Transform().translate(offset))); }
	if (selected_option.get() < options.size()) {
		left_dot.render(svc, win, -offset, 0);
		right_dot.render(svc, win, -offset, 0);
	}
}
} // namespace gui