
#include <fornani/entities/world/Laser.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

Laser::Laser(automa::ServiceProvider& svc, sf::Vector2f position, LaserType type, int active, int cooldown, float size) : m_active{active}, m_cooldown{cooldown}, m_size{size} {
	m_drawbox.setFillColor(colors::ui_white);
	m_hitbox.set_position(position);
}

void Laser::update(automa::ServiceProvider& svc, player::Player& player, Map& map) { m_drawbox.setSize(calculate_size(map)); }

void Laser::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	m_drawbox.setPosition(m_hitbox.get_position() - cam);
	win.draw(m_drawbox);
}

sf::Vector2f Laser::calculate_size(Map& map) {
	auto pos = m_hitbox.get_position();
	auto& axis = m_direction == HV::horizontal ? pos.x : pos.y;
	auto& fixed = m_direction == HV::horizontal ? pos.y : pos.x;
	fixed = constants::f_cell_size * m_size;
	for (auto& cell : map.get_middleground()->grid.cells) {
		if (map.get_cell_at_position(pos).is_solid()) {
			axis += constants::f_cell_size;
			break;
		}
		axis += constants::f_cell_size;
	}
	return pos;
}

} // namespace fornani::world
