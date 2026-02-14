
#include "fornani/world/Tile.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/weapon/Projectile.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Tile::Tile(sf::Vector2<std::uint32_t> i, sf::Vector2f p, std::uint32_t val, std::uint32_t odi, float spacing, std::uint8_t chunk_id, sf::Vector2<std::uint32_t> dim)
	: index(i), value(val), one_d_index(odi), bounding_box({32.f, 32.f}, evaluate(val)), m_spacing(spacing), m_chunk_id{chunk_id} {
	set_type();
	bounding_box.set_position(p);

	if (index.x == 0 || index.y == 0 || index.x == dim.x - 1 || index.y == dim.y - 1) { flags.set(TileState::border); }
}

void Tile::on_hit(automa::ServiceProvider& svc, player::Player& player, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	// if (!map.nearby(bounding_box, proj.get_collider())) { return; }
	collision_check = true;
	if ((proj.get_collider().collides_with(bounding_box) && is_occupied())) {
		if (!is_collidable() || is_platform()) { return; }
		if (!proj.reflect()) {
			if (!proj.destruction_initiated()) {
				map.effects.push_back(entity::Effect(svc, "wall_hit", proj.get_destruction_point() + proj.get_position(), {}, proj.effect_type()));
				if (proj.get_direction().lnr == LNR::neutral) { map.effects.back().rotate(); }
			}
			proj.destroy(false);
		} else {
			auto direction = sf::Vector2i{};
			direction.x = proj.get_collider().get_local_center().x < bounding_box.get_center().x ? -1 : 1;
			direction.y = proj.get_collider().get_local_center().y < bounding_box.get_center().y ? -1 : 1;
			auto side_collision = std::abs(proj.get_collider().get_local_center().x - bounding_box.get_center().x) > std::abs(proj.get_collider().get_local_center().y - bounding_box.get_center().y);
			if (side_collision) {
				direction.y = 0;
			} else {
				direction.x = 0;
			}
			proj.bounce_off_surface(direction);
			svc.soundboard.flags.item.set(audio::Item::gem);
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

void Tile::set_type() { type = get_type_by_value(value); }

} // namespace fornani::world
