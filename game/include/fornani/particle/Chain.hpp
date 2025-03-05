
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Spring.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::player {
class Player;
}

namespace fornani::vfx {
class Chain {
  public:
	Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2<float> position, int num_links, bool reversed = false, float spacing = 0.f);
	Chain(automa::ServiceProvider& svc, sf::Texture const& tex, SpringParameters params, sf::Vector2<float> position, int num_links, bool reversed = false, float spacing = 0.f);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_gravity(float g) { grav = g; }
	void set_position(sf::Vector2f to_position);
	void set_end_position(sf::Vector2f to_position);
	void lock_ends();
	void set_texture_rect(sf::IntRect rect);
	void snap_to_axis(bool vert = true);
	std::vector<Spring> links{};
	std::vector<components::PhysicsComponent> joints{};
	[[nodiscard]] bool moving() const;

  private:
	sf::Vector2<float> root{};
	float external_dampen{0.05f};
	float grav{1.f};
	util::Cooldown intro{8};
	sf::Sprite m_sprite;
};

} // namespace fornani::vfx
