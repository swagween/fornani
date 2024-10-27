#include "ActionContextBar.hpp"
#include <SFML/Graphics.hpp>

#include "../service/ServiceProvider.hpp"

#include <format>
#include <string>

namespace gui {

ActionContextBar::ActionContextBar(automa::ServiceProvider& svc) {
	text.setFont(svc.text.fonts.title);
	text.setFillColor(svc.styles.colors.dark_grey);
	text.setCharacterSize(16);
	text.setPosition(sf::Vector2f{0.f, svc.constants.f_screen_dimensions.y - 20.f});
}

void ActionContextBar::update(automa::ServiceProvider& svc) {
	std::string text_string{};
	for (auto const& action : svc.controller_map.actions_queried_this_update()) {
		// If an action has been queried this tick, it most likely does something when activated, so it is shown in the context bar
		text_string += std::format("[{}] {} ", svc.controller_map.digital_action_source_name(action), svc.controller_map.digital_action_name(action));
	}

	text.setString(text_string);
}

void ActionContextBar::render(sf::RenderWindow& win) { win.draw(text); }

} // namespace gui