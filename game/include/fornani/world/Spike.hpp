#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/Collider.hpp"

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

enum class SpikeAttributes : std::uint8_t { no_collision };

class Spike {
  public:
	Spike(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2<float> position, sf::Vector2<int> direction, sf::Vector2<float> size);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }

  private:
	[[nodiscard]] auto i_size() const -> sf::Vector2<int> { return sf::Vector2<int>(static_cast<int>(size.x), static_cast<int>(size.y)); }
	sf::Vector2<float> size{};
	shape::Shape hitbox{};
	shape::Collider collider{};
	Direction facing{};
	util::BitFlags<SpikeAttributes> attributes{};
	sf::Vector2<float> grid_position{};
	sf::Vector2<float> offset{};
	sf::RectangleShape drawbox{};
	sf::Sprite sprite;
	bool soft_reset{};
};
} // namespace fornani::world
