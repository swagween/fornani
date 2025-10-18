
#include "fornani/gui/gizmos/DescriptionGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

#include <numbers>

namespace fornani::gui {

DescriptionGizmo::DescriptionGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement, sf::IntRect lookup, sf::FloatRect text_bounds, sf::Vector2f start_position)
	: Gizmo("Map Info", false), m_sprite{svc.assets.get_texture("description_gizmo")}, m_bounds{text_bounds}, m_text_offset{text_bounds.position} {
	m_physics.position = start_position;
	m_placement = placement;
	m_sprite.setScale(constants::f_scale_vec);
	m_sprite.setTextureRect(lookup);
}

void DescriptionGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_steering.target(m_physics, m_placement + position, 0.003f);
	m_physics.simple_update();
	m_physics.position = m_placement + position;
	m_bounds.position = m_text_offset + m_physics.position + m_additional_offset;
	if (m_text) { m_text->update(); }
}

void DescriptionGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	if (!m_flags.test(DescriptionGizmoFlags::text_only)) {
		Gizmo::render(svc, win, player, shader, palette, cam);
		m_sprite.setPosition(m_physics.position - cam);
		shader.submit(win, palette, m_sprite);
	}
	if (m_text) { m_text->write_instant_message(win); }
}

bool DescriptionGizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

void DescriptionGizmo::set_offset(sf::Vector2f const offset) { m_additional_offset = offset; }

void DescriptionGizmo::adjust_bounds(sf::Vector2f const adjustment) { m_bounds.position -= adjustment; }

void DescriptionGizmo::write(automa::ServiceProvider& svc, std::string_view message, sf::Font& font) {
	if (!m_text) { m_text = TextWriter(svc, message, m_bounds); }
	m_text->set_font(font);
	m_text->load_single_message(message);
	m_text->set_bounds(m_bounds, true);
	m_text->set_font_color(colors::pioneer_red);
}

} // namespace fornani::gui
