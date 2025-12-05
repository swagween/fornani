
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Laser.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Laser::Laser(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, LaserType type, util::BitFlags<LaserAttributes> attributes, CardinalDirection direction, int active, int cooldown, float size)
	: m_type{type}, m_attributes{attributes}, m_direction{direction}, m_active{active}, m_cooldown{cooldown}, m_size{size}, m_breadth{constants::f_cell_size}, m_spawn_point{position} {
	m_drawbox.setFillColor(colors::ui_white);
	m_hitbox.set_position(position);
	fire();
}

void Laser::update(automa::ServiceProvider& svc, player::Player& player, Map& map) {
	m_cooldown.update();
	m_active.update();
	if (m_attributes.test(LaserAttributes::infinite)) { fire(); }
	if (m_active.is_almost_complete()) { m_cooldown.start(); }
	auto size = calculate_size(map);
	auto hit_size = size;
	hit_size.x = m_direction.as_hv() == HV::vertical ? constants::f_cell_size : size.x;
	hit_size.y = m_direction.as_hv() == HV::horizontal ? constants::f_cell_size : size.y;
	m_drawbox.setSize(size);
	m_hitbox.set_dimensions(hit_size);

	// set dimensions based on cardinal direction
	m_hitbox.set_position(m_spawn_point);
	if (m_direction.left()) { m_hitbox.set_position(m_spawn_point - sf::Vector2f{m_hitbox.get_dimensions().x - constants::f_cell_size, 0.f}); }
	if (m_direction.up()) { m_hitbox.set_position(m_spawn_point - sf::Vector2f{0.f, m_hitbox.get_dimensions().y - constants::f_cell_size}); }

	for (auto& p : map.pushables) { handle_collision(p.collision_box, size); }
	for (auto& p : map.platforms) { handle_collision(p.bounding_box, size); }

	// calculate laser end and spawn an effect there
	if (svc.ticker.every_x_ticks(18)) {
		m_pulse.modulate(1);
		map.spawn_effect(svc, "dash", calculate_end_point() + random::random_vector_float(-16.f, 16.f));
	}

	m_direction.as_hv() == HV::horizontal ? m_drawbox.setOrigin({0.f, (m_breadth - constants::f_cell_size) * 0.5f}) : m_drawbox.setOrigin({(m_breadth - constants::f_cell_size) * 0.5f, 0.f});
	m_breadth = is_active() ? constants::f_cell_size + 4.f * m_pulse.get() : util::round_to_nearest(constants::f_cell_size * m_cooldown.get_quadratic_normalized(), 4.f);
	is_active() ? m_drawbox.setFillColor(colors::ui_white) : m_cooldown.get_normalized() > 0.6f ? m_drawbox.setFillColor(colors::goldenrod) : m_drawbox.setFillColor(colors::dark_goldenrod);
	if (player.collider.hurtbox.overlaps(m_hitbox) && is_active()) { player.hurt(); }
}

void Laser::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	m_drawbox.setPosition(m_hitbox.get_position() - cam);
	if (svc.greyblock_mode()) {
		m_hitbox.render(win, cam, colors::bright_purple);
	} else {
		win.draw(m_drawbox);
	}
}

sf::Vector2f Laser::calculate_size(Map& map) {
	auto ret = sf::Vector2f{};
	auto& axis = m_direction.as_hv() == HV::horizontal ? ret.x : ret.y;
	auto& fixed = m_direction.as_hv() == HV::horizontal ? ret.y : ret.x;
	fixed = m_breadth * m_size;
	auto current = m_spawn_point;
	while (true) {
		auto step = constants::f_cell_size * m_direction.as_vector();
		axis += constants::f_cell_size;
		current += step;
		if (map.get_cell_at_position(current).is_solid() || !map.within_bounds(current)) { return ret; }
	}
	return ret;
}

sf::Vector2f Laser::calculate_end_point() {
	auto ret = m_hitbox.get_center();
	switch (m_direction.get()) {
	case UDLR::up: ret.y -= m_hitbox.get_dimensions().y * 0.5f; break;
	case UDLR::down: ret.y += m_hitbox.get_dimensions().y * 0.5f; break;
	case UDLR::left: ret.x -= m_hitbox.get_dimensions().x * 0.5f; break;
	case UDLR::right: ret.x += m_hitbox.get_dimensions().x * 0.5f; break;
	default: break;
	}
	return ret;
}

void Laser::handle_collision(shape::Shape& obstacle, sf::Vector2f size) {
	if (auto coll = calculate_collision_point(obstacle)) {
		auto new_width = ccm::abs(m_spawn_point.x - coll->x) + constants::f_cell_size;
		auto new_height = ccm::abs(m_spawn_point.y - coll->y) + constants::f_cell_size;
		switch (m_direction.get()) {
		case UDLR::up:
			m_hitbox.set_position(sf::Vector2f{m_hitbox.get_position().x, coll->y});
			m_hitbox.set_dimensions({m_hitbox.get_dimensions().x, new_height});
			m_drawbox.setSize({size.x, new_height});
			break;
		case UDLR::down:
			m_hitbox.set_dimensions({m_hitbox.get_dimensions().x, new_height - constants::f_cell_size});
			m_drawbox.setSize({size.x, new_height - constants::f_cell_size});
			break;
		case UDLR::left:
			m_hitbox.set_position(sf::Vector2f{coll->x, m_hitbox.get_position().y});
			m_hitbox.set_dimensions({new_width, m_hitbox.get_dimensions().y});
			m_drawbox.setSize({new_width, size.y});
			break;
		case UDLR::right:
			m_hitbox.set_dimensions({new_width - constants::f_cell_size, m_hitbox.get_dimensions().y});
			m_drawbox.setSize({new_width - constants::f_cell_size, size.y});
			break;
		default: break;
		}
	}
}

std::optional<sf::Vector2f> Laser::calculate_collision_point(shape::Shape& other) {
	if (!other.overlaps(m_hitbox)) { return std::nullopt; }
	auto ret = sf::Vector2f{};
	switch (m_direction.get()) {
	case UDLR::up: ret.y = other.vertices[2].y; break;
	case UDLR::down: ret.y = other.vertices[0].y; break;
	case UDLR::left: ret.x = other.vertices[1].x; break;
	case UDLR::right: ret.x = other.vertices[0].x; break;
	default: break;
	}
	return ret;
}

} // namespace fornani::world
