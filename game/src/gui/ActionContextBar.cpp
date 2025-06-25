#include "fornani/gui/ActionContextBar.hpp"
#include <SFML/Graphics.hpp>
#include <tracy/Tracy.hpp>

#include "fornani/service/ServiceProvider.hpp"
#include "fornani/gui/ActionControlIconQuery.hpp"
#include <format>
#include <string>

namespace fornani::gui {

ActionContextBar::ActionContextBar(automa::ServiceProvider& svc) { text.setPosition(sf::Vector2f{8.f, svc.window->f_screen_dimensions().y - 32.f}); }

void ActionContextBar::update(automa::ServiceProvider& svc) {
	ZoneScopedN("ActionContextBar::update");
	auto actions_queried_this_frame = svc.controller_map.actions_queried_this_update();

	text.clear_segments();
	for (auto const& action : actions_queried_this_frame) {
		// If an action has been queried this tick, it most likely does something when activated, so it is shown in the context bar
		auto sprite = get_action_control_icon(svc, action);

		sprite.setColor(colors::dark_grey);

		text.add_sprite(sprite);

		auto str = svc.controller_map.digital_action_name(action);
		text.add_text(str, svc.text.fonts.basic, 16, colors::dark_grey);
		text.add_spacing(15.f);
	}
}

void ActionContextBar::render(sf::RenderWindow& win) { win.draw(text); }

} // namespace gui