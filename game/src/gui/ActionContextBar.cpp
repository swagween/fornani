#include "fornani/gui/ActionContextBar.hpp"
#include <SFML/Graphics.hpp>

#include <format>
#include <string>
#include "fornani/gui/ActionControlIconQuery.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

ActionContextBar::ActionContextBar(automa::ServiceProvider& svc) { text.setPosition(sf::Vector2f{8.f, svc.window->f_screen_dimensions().y - 32.f}); }

void ActionContextBar::update(automa::ServiceProvider& svc) {
	text.clear_segments();
	// TODO: fix this later
	// for (auto const& action : svc.input_system.get) {
	//	// If an action has been queried this tick, it most likely does something when activated, so it is shown in the context bar
	//	auto sprite = get_action_control_icon(svc, action);

	//	sprite.setColor(colors::dark_grey);

	//	text.add_sprite(sprite);

	//	auto str = input::str(action);
	//	text.add_text(str, svc.text.fonts.basic, 16, colors::dark_grey);
	//	text.add_spacing(15.f);
	//}
}

void ActionContextBar::render(sf::RenderWindow& win) { win.draw(text); }

} // namespace fornani::gui
