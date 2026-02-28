
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/HazardTile.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/HazardMap.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

HazardMap::HazardMap(automa::ServiceProvider& svc, dj::Json const& in, sf::Vector2u texture_dimensions) {
	// metadata
	auto const& metadata = in["metadata"];
	m_tag = metadata["tag"].as_string();
	auto const& config = svc.data.hazards[m_tag];
	m_dimensions = sf::Vector2i{config["dimensions"][0].as<int>(), config["dimensions"][1].as<int>()};
	m_table_dimensions = sf::Vector2i{config["table_dimensions"][0].as<int>(), config["table_dimensions"][1].as<int>()};

	// tiles
	auto const& list = in["tiles"];
	if (!m_texture.resize(texture_dimensions)) { NANI_LOG_ERROR(m_logger, "Failed to set HazardMap texture size!"); }
	m_texture.clear(sf::Color::Transparent);
	for (auto const& tile : list.as_array()) {
		auto position = sf::Vector2<std::uint32_t>{tile["position"][0].as<std::uint32_t>(), tile["position"][1].as<std::uint32_t>()};
		auto direction = CardinalDirection{tile["direction"].as<int>()};
		auto lookup = sf::Vector2i{tile["lookup"][0].as<int>(), tile["lookup"][1].as<int>()};
		m_tiles.push_back(HazardTile{svc, m_tag, position, lookup, m_dimensions, m_table_dimensions, direction});

		// draw to texture
		m_texture.draw(m_tiles.back());
	}
	m_texture.display();
}

void HazardMap::serialize(dj::Json& out) {
	out["metadata"]["tag"] = m_tag;
	out["tiles"] = dj::Json::empty_array();
	for (auto const& tile : m_tiles) {
		auto entry = dj::Json{};
		entry["position"] = dj::Json::empty_array();
		entry["position"].push_back(tile.get_grid_position().x);
		entry["position"].push_back(tile.get_grid_position().y);
		entry["lookup"] = dj::Json::empty_array();
		entry["lookup"].push_back(tile.get_lookup().x);
		entry["lookup"].push_back(tile.get_lookup().y);
		entry["direction"] = tile.get_direction().as<int>();
		out["tiles"].push_back(entry);
	}
}

void HazardMap::add_tile(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i position, int value, CardinalDirection direction) {
	auto pos = sf::Vector2<std::uint32_t>{position};
	auto lookup = sf::Vector2i(value % m_table_dimensions.x, value / m_table_dimensions.x);
	m_tiles.push_back(HazardTile{svc, m_tag, pos, lookup, m_dimensions, m_table_dimensions, direction});
}

void HazardMap::update(player::Player& player, world::Map& map) {
	for (auto& hazard : m_tiles) { hazard.update(player, map); }
}

void HazardMap::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto sprite = sf::Sprite(m_texture.getTexture());
	sprite.setPosition(-cam);
	win.draw(sprite);
	for (auto& hazard : m_tiles) { hazard.render(win, cam); }
}

void HazardMap::render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f scale, sf::Vector2f origin) {
	auto sprite = sf::Sprite(m_texture.getTexture());
	sprite.setOrigin(origin);
	sprite.setScale(scale);
	sprite.setPosition(cam);
	win.draw(sprite);
}

} // namespace fornani::world
