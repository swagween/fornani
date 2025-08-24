
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/particle/Spring.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <vector>

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
	Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f);
	Chain(automa::ServiceProvider& svc, sf::Texture const& tex, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_gravity(float g) { m_grav = g; }
	void set_position(sf::Vector2f to_position);
	void set_end_position(sf::Vector2f to_position);
	void lock_ends();
	void set_texture_rect(sf::IntRect rect);
	void snap_to_axis(bool vert = true);
	void simulate(automa::ServiceProvider& svc, int amount);
	std::vector<Spring> links{};
	std::vector<components::PhysicsComponent> joints{};
	[[nodiscard]] bool moving() const;

  private:
	sf::Vector2f m_root{};
	float m_external_dampen;
	float m_grav{1.f};
	sf::Sprite m_sprite;
};

} // namespace fornani::vfx
