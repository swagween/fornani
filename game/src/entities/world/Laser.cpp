
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Laser.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Laser::Laser(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, LaserType type, HV direction, int active, int cooldown, float size)
	: m_direction{direction}, m_active{active}, m_cooldown{cooldown}, m_size{size}, m_breadth{constants::f_cell_size}, m_spawn_point{position} {
	m_drawbox.setFillColor(colors::ui_white);
	m_hitbox.set_position(position);
	fire();
}

void Laser::update(automa::ServiceProvider& svc, player::Player& player, Map& map) {
	m_cooldown.update();
	m_active.update();
	if (m_active.is_almost_complete()) { m_cooldown.start(); }
	auto size = calculate_size(map);
	if (svc.ticker.every_x_ticks(18)) { m_pulse.modulate(1); }
	auto hit_size = size;
	hit_size.x = m_direction == HV::vertical ? constants::f_cell_size : size.x;
	hit_size.y = m_direction == HV::horizontal ? constants::f_cell_size : size.y;
	m_drawbox.setSize(size);
	m_hitbox.set_dimensions(hit_size);
	m_direction == HV::horizontal ? m_drawbox.setOrigin({0.f, (m_breadth - constants::f_cell_size) * 0.5f}) : m_drawbox.setOrigin({(m_breadth - constants::f_cell_size) * 0.5f, 0.f});
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
	auto& axis = m_direction == HV::horizontal ? ret.x : ret.y;
	auto& fixed = m_direction == HV::horizontal ? ret.y : ret.x;
	fixed = m_breadth * m_size;
	auto current = m_spawn_point;
	auto& curr = m_direction == HV::horizontal ? current.x : current.y;
	while (true) {
		axis += constants::f_cell_size;
		curr += constants::f_cell_size;
		if (map.get_cell_at_position(current).is_solid() || !map.within_bounds(current)) { return ret; }
	}
	return ret;
}

} // namespace fornani::world
