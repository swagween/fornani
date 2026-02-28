
#pragma once

#include <djson/json.hpp>
#include <fornani/entities/world/HazardTile.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani::world {

class HazardMap {
  public:
	HazardMap() = default;
	HazardMap(automa::ServiceProvider& svc, dj::Json const& in, sf::Vector2u texture_dimensions);
	void serialize(dj::Json& out);
	void add_tile(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i position, int value, CardinalDirection direction);

	// gameplay functions
	void update(player::Player& player, world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f scale, sf::Vector2f origin);

  private:
	std::vector<HazardTile> m_tiles{};
	sf::RenderTexture m_texture{};
	std::string m_tag{};

	sf::Vector2i m_dimensions{};
	sf::Vector2i m_table_dimensions{};

	io::Logger m_logger{"Hazards"};
};

} // namespace fornani::world
