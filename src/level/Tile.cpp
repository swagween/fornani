#include "Tile.hpp"
#include <iostream>

namespace world {

Tile::Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi) : index(i), scaled_position(static_cast<sf::Vector2<int>>(i)), position(p), value(val), one_d_index(odi) {
	set_type();
	bounding_box.set_position(p);
}

void Tile::update_polygon(sf::Vector2<float> cam) {
	polygon.setPointCount(bounding_box.vertices.size());
	for (size_t i{0}; i < bounding_box.vertices.size(); ++i) { polygon.setPoint(i, bounding_box.vertices.at(i)); }
	polygon.setPosition(-cam.x, -cam.y);
	polygon.setFillColor(sf::Color{40, 200, 130, 120});
	polygon.setOutlineColor(sf::Color(235, 232, 249, 140));
	polygon.setOutlineThickness(0);
}

void Tile::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setOutlineColor(sf::Color::Blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setSize(bounding_box.dimensions);
	drawbox.setOutlineThickness(-2);
	if (collision_check) {
		update_polygon(cam);
		if (!surrounded) {
			//win.draw(polygon);
		}
	}
	drawbox.setPosition(bounding_box.position);
	if (ramp_adjacent()) { 
		drawbox.setOutlineColor(sf::Color::Red);
		win.draw(drawbox);
	}
}

void Tile::set_type() {
	type = TileType::empty;
	if (value < 192 && value > 0) {
		type = TileType::solid;
		return;
	}
	if (value < 208 && value >= 192) {
		type = TileType::ceiling_ramp;
		return;
	}
	if (value < 224 && value >= 208) {
		type = TileType::ground_ramp;
		return;
	}
	if (value < 240 && value >= 236) {
		type = TileType::platform;
		return;
	}
	if (value < 244 && value >= 240) {
		type = TileType::death_spike;
		return;
	}
	if (value < 230 && value >= 228) {
		type = TileType::pushable;
		return;
	}
	if (value == 231) {
		type = TileType::spawner;
		return;
	}
	if (value < 248 && value >= 244) {
		type = TileType::breakable;
		return;
	}
	if (value >= 248) { type = TileType::spike; }
}

sf::Vector2<float> Tile::middle_point() { return {position.x + bounding_box.dimensions.x / 2, position.y + bounding_box.dimensions.y / 2}; }

} // namespace world
