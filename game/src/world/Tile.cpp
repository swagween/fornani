#include "fornani/world/Tile.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/weapon/Projectile.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Tile::Tile(sf::Vector2<std::uint32_t> i, sf::Vector2f p, std::uint32_t val, std::uint32_t odi, float spacing) : index(i), value(val), one_d_index(odi), bounding_box({32.f, 32.f}, evaluate(val)), m_spacing(spacing) {
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
					map.effects.push_back(entity::Effect(svc, "wall_hit", proj.get_destruction_point() + proj.get_position(), {}, proj.effect_type()));
					if (proj.get_direction().lnr == LNR::neutral) { map.effects.back().rotate(); }
				}
				proj.destroy(false);
			}
		}
	}
}

void Tile::render(sf::RenderWindow& win, sf::RectangleShape& draw, sf::Vector2f cam) {
	draw.setSize({32.f, 32.f});
	draw.setFillColor(sf::Color::Transparent);
	draw.setOutlineThickness(-2.f);
	one_d_index % 2 == 0 ? draw.setOutlineColor(sf::Color{17, 230, 187, 45}) : draw.setOutlineColor(sf::Color{38, 230, 220, 45});
	if (collision_check) { draw.setOutlineColor(sf::Color{190, 255, 7, 180}); }
	if (covered()) { draw.setOutlineColor(sf::Color{0, 155, 130, 180}); }
	if (ramp_adjacent()) { draw.setOutlineColor(sf::Color{240, 10, 7, 180}); }
	draw.setPosition(bounding_box.get_position() - cam);
	if (is_occupied()) { win.draw(draw); }
	collision_check = false;
}

void Tile::draw(sf::RenderTexture& tex) {
	if (is_occupied()) { bounding_box.draw(tex); }
}

void Tile::set_type() {
	type = TileType::empty;
	if (value < special_index_v && value > 0) { type = TileType::solid; }
	if ((value < special_index_v + 16 && value >= special_index_v) || (value >= ceiling_single_ramp && value <= ceiling_single_ramp + 3)) { type = TileType::ceiling_ramp; }
	if ((value < special_index_v + 32 && value >= special_index_v + 16) || (value >= floor_single_ramp && value <= floor_single_ramp + 3)) { type = TileType::ground_ramp; }
	if (value < special_index_v + 48 && value >= special_index_v + 44) { type = TileType::platform; }
	if (value < special_index_v + 38 && value >= special_index_v + 36) { type = TileType::pushable; }
	if (value == special_index_v + 38) { type = TileType::target; }
	if (value == special_index_v + 39) { type = TileType::spawner; }
	if (value == special_index_v + 40) { type = TileType::home; }
	if (value == special_index_v + 52) { type = TileType::bonfire; }
	if (value == special_index_v + 53) { type = TileType::campfire; }
	if (value == special_index_v + 54) { type = TileType::checkpoint; }
	if (value == special_index_v + 55) { type = TileType::breakable; }
	if (value == special_index_v + 62) { type = TileType::big_spike; }
	if (value == special_index_v + 63) { type = TileType::spike; }
}

} // namespace fornani::world
