#include "Tile.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../weapon/Projectile.hpp"
#include "../level/Map.hpp"
#include <iostream>

namespace world {

Tile::Tile(sf::Vector2<uint32_t> i, sf::Vector2<float> p, uint32_t val, uint32_t odi) : index(i), value(val), one_d_index(odi) {
	set_type();
	bounding_box.set_position(p);
}

void Tile::on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj) {
	if (proj.stats.transcendent) { return; }
	if (!map.nearby(bounding_box, proj.bounding_box)) {
		return;
	} else {
		collision_check = true;
		if ((proj.bounding_box.overlaps(bounding_box) && is_occupied())) {
			if (!is_collidable()) { return; }
			if (!proj.stats.transcendent) {
				if (!proj.destruction_initiated()) {
					map.effects.push_back(entity::Effect(svc, proj.destruction_point + proj.physics.position, {}, proj.effect_type(), 2));
					if (proj.direction.lr == dir::LR::neutral) { map.effects.back().rotate(); }
				}
				proj.destroy(false);
			}
			if (proj.stats.spring && is_hookable()) {
				if (proj.hook.grapple_flags.test(arms::GrappleState::probing)) {
					proj.hook.spring.set_anchor(middle_point());
					proj.hook.grapple_triggers.set(arms::GrappleTriggers::found);
				}
				map.handle_grappling_hook(svc, proj);
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
		if (!surrounded) {

		}
	}
	if (ramp_adjacent()) { 

	}
	draw.setPosition(bounding_box.position - cam);
	if (is_solid()) { win.draw(draw); }
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

} // namespace world
