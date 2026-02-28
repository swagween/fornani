
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/utils/NeighborSet.hpp>

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

enum class SpikeAttributes { no_collision, soft_reset, random };
enum class SpikeShape { peak, corner, valley };

class Spike : public Animatable {
  public:
	Spike(automa::ServiceProvider& svc, sf::Texture const& texture, sf::Vector2f position, NeighborSet neighbors, sf::Vector2f size, int style, bool random = false);
	void set_shape(NeighborSet neighbors);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, std::optional<Palette>& palette, sf::Vector2f cam);
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }

  private:
	[[nodiscard]] auto i_size() const -> sf::Vector2i { return sf::Vector2i{size}; }
	[[nodiscard]] auto is_small() const -> bool { return i_size().x == 1; }
	sf::Vector2f size{};
	shape::Shape hitbox{};
	shape::Collider collider{};
	CardinalDirection facing{};
	util::BitFlags<SpikeAttributes> attributes{};
	SpikeShape m_shape{};
	sf::Vector2f grid_position{};
	sf::Vector2f offset{};
	sf::RectangleShape drawbox{};
	bool soft_reset{};
};
} // namespace fornani::world
