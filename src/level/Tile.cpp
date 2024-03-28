#include "Tile.hpp"

namespace world {

Tile::Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, lookup::TILE_TYPE t) : index(i), position(p), value(val), type(t) { bounding_box = shape::Shape(sf::Vector2<float>(lookup::unit_size_f, lookup::unit_size_f)); }

void Tile::update_polygon(sf::Vector2<float> cam) {

	polygon.setPointCount(bounding_box.vertices.size());
	for (size_t i{0}; i < bounding_box.vertices.size(); ++i) { polygon.setPoint(i, bounding_box.vertices.at(i)); }
	polygon.setPosition(-cam.x, -cam.y);
	polygon.setFillColor(sf::Color{40, 200, 130, 120});
	polygon.setOutlineColor(sf::Color(235, 232, 249, 140));
	polygon.setOutlineThickness(-1);
}

void Tile::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (collision_check) {
		update_polygon(cam);
		if (!surrounded) {
			win.draw(polygon);
		}
	}
}

bool Tile::is_occupied() const { return value > 0; }

bool Tile::is_collidable() const { return type != lookup::TILE_TYPE::TILE_CEILING_RAMP && type != lookup::TILE_TYPE::TILE_GROUND_RAMP && type != lookup::TILE_TYPE::TILE_SPIKES; }

bool Tile::is_solid() const { return type == lookup::TILE_TYPE::TILE_BASIC || type == lookup::TILE_TYPE::TILE_BREAKABLE || type == lookup::TILE_TYPE::TILE_CEILING_RAMP || type == lookup::TILE_TYPE::TILE_GROUND_RAMP; }

bool Tile::is_hookable() const { return type == lookup::TILE_TYPE::TILE_BASIC || type == lookup::TILE_TYPE::TILE_BREAKABLE; }

bool Tile::is_breakable() const { return type == lookup::TILE_TYPE::TILE_BREAKABLE; }

bool Tile::is_ramp() const { return type == lookup::TILE_TYPE::TILE_CEILING_RAMP || type == lookup::TILE_TYPE::TILE_GROUND_RAMP; }

sf::Vector2<float> Tile::middle_point() { return {position.x + bounding_box.dimensions.x / 2, position.y + bounding_box.dimensions.y / 2}; }

} // namespace world
