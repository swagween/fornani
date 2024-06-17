#include "StatusBar.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"

namespace gui {

StatusBar::StatusBar(automa::ServiceProvider& svc, sf::Vector2<int> dim) : dimensions(dim) {
	gravitator = vfx::Gravitator({0, 0}, svc.styles.colors.bright_orange, 0.9f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.9f, 0.9f}, 1.0f);
	debug_rects.filled.setFillColor(svc.styles.colors.red);
	debug_rects.taken.setFillColor(svc.styles.colors.goldenrod);
	debug_rects.gone.setFillColor(svc.styles.colors.navy_blue);
	debug_rects.filled.setPosition({svc.constants.screen_dimensions.x * 0.5f, 60.f});
	debug_rects.filled.setSize({200.f, 20.f});
	debug_rects.filled.setOrigin(debug_rects.filled.getSize() * 0.5f);
	debug_rects.taken.setPosition({svc.constants.screen_dimensions.x * 0.5f, 60.f});
	debug_rects.taken.setSize({200.f, 20.f});
	debug_rects.taken.setOrigin(debug_rects.filled.getSize() * 0.5f);
	debug_rects.gone.setPosition({svc.constants.screen_dimensions.x * 0.5f, 60.f});
	debug_rects.gone.setSize({200.f, 20.f});
	debug_rects.gone.setOrigin(debug_rects.filled.getSize() * 0.5f);
	current_state = BarState::full;
}

void StatusBar::update(automa::ServiceProvider& svc, float max, float current) {
	gravitator.set_target_position(position);
	gravitator.update(svc);

	debug_rects.filled.setSize({current, 10.f});
	debug_rects.gone.setSize({max, 10.f});
	debug_rects.taken.setSize({current, 10.f});
	debug_rects.filled.setOrigin(debug_rects.gone.getSize() * 0.5f);
	debug_rects.gone.setOrigin(debug_rects.gone.getSize() * 0.5f);
	debug_rects.taken.setOrigin(debug_rects.gone.getSize() * 0.5f);
	current_state = current == max ? BarState::full : current_state;
	current_state = current <= 0 ? BarState::empty : current_state;
}
void StatusBar::render(sf::RenderWindow& win) {
	win.draw(debug_rects.gone);
	win.draw(debug_rects.taken);
	win.draw(debug_rects.filled);
}
} // namespace gui