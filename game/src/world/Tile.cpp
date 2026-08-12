
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
	collision_check = true;
	if ((proj.get_collider().collides_with(bounding_box) && is_occupied())) {
		if (!is_collidable() || is_platform()) { return; }
		if (!proj.reflect()) {
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

void Tile::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto const size = bounding_box.get_dimensions();

	auto is_in = win.getViewport(win.getView()).contains(sf::Vector2i{bounding_box.get_position() - cam}) || win.getViewport(win.getView()).contains(sf::Vector2i{bounding_box.get_position() + bounding_box.get_dimensions() - cam});
	// if (!is_in) { return; }

	auto even = (one_d_index % 2 == 0 && index.y % 2 == 0) || (one_d_index % 2 == 1 && index.y % 2 == 1);
	auto const outline_color = even ? sf::Color{17, 230, 167, 180} : sf::Color{78, 230, 250, 180};

	auto color = outline_color;

	if (ramp_adjacent()) { color = sf::Color{240, 10, 7, 180}; }
	if (collision_check) { color = sf::Color{190, 215, 60, 180}; }

	sf::VertexArray draw{sf::PrimitiveType::TriangleFan, bounding_box.vertices.size() + 1};

	if (bounding_box.vertices.size() == 4) {
		auto tweak = sf::Vector2f{};
		if (is_platform()) { tweak.y = -24.f; }
		draw[0].position = bounding_box.vertices[0] - cam;
		draw[1].position = bounding_box.vertices[1] - cam;
		draw[2].position = bounding_box.vertices[2] - cam + tweak;
		draw[3].position = bounding_box.vertices[3] - cam + tweak;
		draw[4].position = bounding_box.vertices[0] - cam;
	} else if (bounding_box.vertices.size() == 3) {
		draw[0].position = bounding_box.vertices[0] - cam;
		draw[1].position = bounding_box.vertices[1] - cam;
		draw[2].position = bounding_box.vertices[2] - cam;
		draw[3].position = bounding_box.vertices[0] - cam;
	}

	for (auto& vertex : draw) { vertex.color = color; }

	if (is_collidable()) { win.draw(draw); }

	collision_check = false;
}

void Tile::draw(sf::RenderTexture& tex) {
	if (is_occupied()) { bounding_box.draw(tex); }
}

void Tile::set_type() { type = get_type_by_value(value); }

} // namespace fornani::world
