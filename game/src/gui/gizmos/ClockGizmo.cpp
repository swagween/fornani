
#include "fornani/gui/gizmos/ClockGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

ClockGizmo::ClockGizmo(automa::ServiceProvider& svc, world::Map& map) : Gizmo("Minimap"), m_sprites{.clock{sf::Sprite{svc.assets.get_texture("clock_gizmo")}}, .hand{sf::Sprite{svc.assets.get_texture("clock_hand")}}} {
	m_physics.position = sf::Vector2f{100.f, 100.f};
	m_sprites.clock.setScale(svc.constants.texture_scale);
	m_sprites.hand.setOrigin({4.f, 4.f});
	m_sprites.clock.setOrigin(m_sprites.clock.getLocalBounds().getCenter());
}

void ClockGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::update(svc, player, map);
	auto current_time{svc.world_clock.get_normalized_time()};
	auto angle{(util::f_pi * 2.f) * -current_time - util::f_pi * 0.5f}; // start at (0, 1); midnight
	auto direction{util::get_direction_from_angle(angle)};
	m_sprites.clock.setPosition(m_physics.position);
	m_rotator.handle_rotation(m_sprites.hand, direction, 4);
	m_sprites.hand.setScale(svc.constants.texture_scale);
	m_sprites.hand.setTextureRect(sf::IntRect{{0, m_rotator.get_sprite_angle_index() * 8}, {8, 8}});
	m_sprites.hand.setPosition(m_sprites.clock.getPosition() + direction * 32.f);
}

void ClockGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Gizmo::render(svc, win, cam);
	win.draw(m_sprites.clock);
	win.draw(m_sprites.hand);
}

bool ClockGizmo::handle_inputs(config::ControllerMap& controller) { return Gizmo::handle_inputs(controller); }

} // namespace fornani::gui
