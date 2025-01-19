#include "fornani/level/Tile.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/weapon/Projectile.hpp"
#include "fornani/level/Map.hpp"
#include <iostream>

namespace world {

Tile::Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi) : index(i), value(val), one_d_index(odi), bounding_box({32.f, 32.f}, evaluate(val)) {
	set_type();
	bounding_box.set_position(p);
}

void Tile::on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (!map.nearby(bounding_box, proj.get_bounding_box())) {
		return;
	} else {
		collision_check = true;
		if ((proj.get_bounding_box().overlaps(bounding_box) && is_occupied())) {
			if (!is_collidable() || is_platform()) { return; }
			if (!proj.transcendent()) {
				if (!proj.destruction_initiated()) {
					map.effects.push_back(entity::Effect(svc, proj.get_destruction_point() + proj.get_position(), {}, proj.effect_type(), 2));
					if (proj.get_direction().lr == dir::LR::neutral) { map.effects.back().rotate(); }
				}
				proj.destroy(false);
			}
		}
	}
}

void Tile::update_polygon(sf::Vector2<float> cam) {}

void Tile::render(sf::RenderWindow& win, sf::Vector2<float> cam, sf::RectangleShape& draw) {
	draw.setSize({32.f, 32.f});
	draw.setFillColor(sf::Color{17, 230, 187, 127});
	if (collision_check) {
		draw.setFillColor(sf::Color{190, 255, 7, 180});
		if (!surrounded) {}
	}
	if (ramp_adjacent()) { draw.setFillColor(sf::Color{240, 155, 7, 180}); }
	if (covered()) { draw.setFillColor(sf::Color{0, 155, 130, 180}); }
	draw.setPosition(bounding_box.position - cam);
	if (is_solid() && !is_spike()) { win.draw(draw); }
	if (is_occupied()) { bounding_box.render(win, cam); }
	collision_check = false;
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
	if (value == 230) {
		type = TileType::target;
		return;
	}
	if (value == 231) {
		type = TileType::spawner;
		return;
	}
	if (value == 244) {
		type = TileType::fire;
		return;
	}
	if (value == 246) {
		type = TileType::checkpoint;
		return;
	}
	if (value == 247) {
		type = TileType::breakable;
		return;
	}
	if (value >= 248) { type = TileType::spike; }
}

} // namespace world
