
#include "fornani/gui/gizmos/DescriptionGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

DescriptionGizmo::DescriptionGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement, sf::IntRect lookup, sf::FloatRect text_bounds)
	: Gizmo("Map Info", false), m_sprite{svc.assets.get_texture("description_gizmo")}, m_bounds{text_bounds}, m_text_offset{text_bounds.position} {
	m_physics.position = sf::Vector2f{374.f, 1600.f};
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_sprite.setTextureRect(lookup);
}

void DescriptionGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_steering.target(m_physics, m_placement + position, 0.003f);
	m_physics.simple_update();
	m_physics.position = m_placement + position;
	if (m_text) { m_text->update(); }
}

void DescriptionGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	m_sprite.setPosition(m_physics.position - cam);
	win.draw(m_sprite);
	m_bounds.position = m_text_offset + m_physics.position - cam;
	if (m_text) { m_text->write_instant_message(win); }
}

bool DescriptionGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

void DescriptionGizmo::write(automa::ServiceProvider& svc, std::string_view message) {
	if (!m_text) { m_text = TextWriter(svc, message, m_bounds); }
	m_text->load_single_message(message);
	m_text->set_bounds(m_bounds, true);
	m_text->set_font_color(svc.styles.colors.pioneer_red);
}

} // namespace fornani::gui
