#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shape.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}
namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

class Map;

enum class CheckpointState : std::uint8_t { reached };

class Checkpoint {
  public:
	Checkpoint(automa::ServiceProvider& svc, sf::Vector2f pos);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void unflag() { flags.reset(CheckpointState::reached); }
	[[nodiscard]] auto reached() const -> bool { return flags.test(CheckpointState::reached); }
	[[nodiscard]] auto position() const -> sf::Vector2f { return bounds.get_position(); }

  private:
	shape::Shape bounds{};
	util::BitFlags<CheckpointState> flags{};
};

} // namespace fornani::world
