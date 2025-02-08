#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/utils/Direction.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::entity {

enum class State { flip };

class Entity {
  public:
	Entity() = default;
	virtual ~Entity() = default;
	explicit Entity(automa::ServiceProvider& svc);
	virtual void update(automa::ServiceProvider& svc, world::Map& map);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) = 0;
	void sprite_shake(automa::ServiceProvider& svc, int rate = 16, int energy = 4);
	[[nodiscard]] auto get_direction() const -> dir::Direction { return direction; }
	sf::RectangleShape drawbox{}; // for debug

  protected:
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> sprite_offset{};
	sf::Vector2<float> random_offset{};
	sf::Vector2<int> sprite_dimensions{};
	sf::Vector2<int> spritesheet_dimensions{};
	dir::Direction direction{};
	util::BitFlags<State> ent_state{};
};

} // namespace entity