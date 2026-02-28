
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/HazardTile.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

HazardTile::HazardTile(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2u position, sf::Vector2i lookup, sf::Vector2i dimensions, sf::Vector2i table_dimensions, CardinalDirection direction)
	: IWorldPositionable{position, {4, 4}}, Drawable{svc, "hazard_" + std::string{tag}}, m_lookup{lookup}, m_direction{direction} {
	set_texture_rect(sf::IntRect{lookup.componentWiseMul(dimensions), dimensions});
	center();
	set_rotation(direction.as_angle());
	auto one_d_index = lookup.y * table_dimensions.x + lookup.x;

	// read hazard positions from hazards.json
	auto const& in_data = svc.data.hazards[tag]["sensors"][one_d_index];
	auto center = sf::Vector2f{dimensions / 2};
	for (auto const& entry : in_data.as_array()) { m_hazards.push_back(Hazard{entry, get_world_position(), center, direction.as_angle()}); }
	Drawable::set_position(get_global_center());
}

void HazardTile::update(player::Player& player, world::Map& map) {
	for (auto& hazard : m_hazards) { hazard.update(player, map); }
}

void HazardTile::render(sf::RenderWindow& win, sf::Vector2f cam) {
	for (auto& hazard : m_hazards) { hazard.render(win, cam); }
}

} // namespace fornani
