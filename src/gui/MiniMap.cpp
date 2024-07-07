#include "MiniMap.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

	MiniMap::MiniMap(automa::ServiceProvider& svc) { background.setFillColor() }

void MiniMap::update(automa::ServiceProvider& svc) {
	view = sf::View(sf::FloatRect(0.0f, 0.0f, svc.constants.screen_dimensions.x * 2.f, svc.constants.screen_dimensions.y * 2.f));
	view.setViewport(sf::FloatRect(0.1f, 0.1f, svc.constants.screen_dimensions.x * 0.8f, svc.constants.screen_dimensions.y * 0.8f));
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {

	// render minimap
		win.setView(view);
		win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y}));
	
}

} // namespace gui
