
#pragma once

#include <fornani/entities/world/Hazard.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani {

class HazardTile : public IWorldPositionable, public Drawable {
  public:
	HazardTile(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2u position, sf::Vector2i lookup, sf::Vector2i dimensions, sf::Vector2i table_dimensions, CardinalDirection direction = {});
	void update(player::Player& player, world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);

	[[nodiscard]] auto get_lookup() const -> sf::Vector2i { return m_lookup; }
	[[nodiscard]] auto get_direction() const -> CardinalDirection { return m_direction; }

  private:
	std::vector<Hazard> m_hazards{};
	sf::Vector2i m_lookup;
	CardinalDirection m_direction;
};

} // namespace fornani
