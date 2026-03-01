
#pragma once

#include <djson/json.hpp>
#include <fornani/entities/world/HazardTile.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani::world {

struct HazardMapProperties {
	std::string tag{};
	sf::Vector2u texture_dimensions{};
	sf::Vector2i dimensions{};
	sf::Vector2i table_dimensions{};

	sf::Vector2i get_lookup(int index);
};

class HazardMap {
  public:
	HazardMap(automa::ServiceProvider& svc, sf::Vector2u texture_dimensions, std::string_view tag);
	HazardMap(automa::ServiceProvider& svc, dj::Json const& in, sf::Vector2u texture_dimensions);
	void serialize(dj::Json& out);
	void add_tile(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i position, int value, CardinalDirection direction);
	void remove_tile(sf::Vector2u position);

	// gameplay functions
	void update(player::Player& player, world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f scale, sf::Vector2f origin);

  private:
	void refresh_texture();

  private:
	std::vector<HazardTile> m_tiles{};
	sf::RenderTexture m_texture{};
	HazardMapProperties m_properties{};

	io::Logger m_logger{"Hazards"};
};

} // namespace fornani::world
