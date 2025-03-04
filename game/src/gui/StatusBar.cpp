#include "fornani/gui/StatusBar.hpp"

#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

StatusBar::StatusBar(automa::ServiceProvider& svc, sf::Vector2<int> dim, float size) : dimensions(dim), size(size) {
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

void StatusBar::update(automa::ServiceProvider& svc, float current) {

	auto filled = std::lerp(0, size, current);
	auto f_filled = static_cast<float>(filled);

	gravitator.set_target_position(position);
	gravitator.update(svc);

	debug_rects.filled.setSize({f_filled, 10.f});
	debug_rects.gone.setSize({size, 10.f});
	debug_rects.taken.setSize({f_filled, 10.f});
	debug_rects.filled.setOrigin(debug_rects.gone.getSize() * 0.5f);
	debug_rects.gone.setOrigin(debug_rects.gone.getSize() * 0.5f);
	debug_rects.taken.setOrigin(debug_rects.gone.getSize() * 0.5f);
	current_state = filled == size ? BarState::full : current_state;
	current_state = filled <= 0 ? BarState::empty : current_state;
}
void StatusBar::render(sf::RenderWindow& win) {
	win.draw(debug_rects.gone);
	win.draw(debug_rects.taken);
	win.draw(debug_rects.filled);
}
} // namespace gui