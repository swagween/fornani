#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shape.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}
namespace arms {
class Projectile;
}

namespace world {

class Map;

enum class CheckpointState { reached };

class Checkpoint {
  public:
	Checkpoint(automa::ServiceProvider& svc, sf::Vector2<float> pos);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void unflag() { flags.reset(CheckpointState::reached); }
	[[nodiscard]] auto reached() const -> bool { return flags.test(CheckpointState::reached); }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return bounds.get_position(); }
  private:
	shape::Shape bounds{};
	util::BitFlags<CheckpointState> flags{};
};

} // namespace world
