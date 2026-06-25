
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/particle/Spring.hpp>
#include <fornani/utils/BitFlags.hpp>
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

enum class ChainFlags { ignore_player_collision, linked };

class Chain {
  public:
	Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f, bool linked = false);
	Chain(automa::ServiceProvider& svc, sf::Texture const& tex, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f, bool linked = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool average = true);
	void set_gravity(float g) { m_grav = g; }
	void set_position(sf::Vector2f to_position);
	void set_end_position(sf::Vector2f to_position);
	void lock_ends();
	void set_texture_rect(sf::IntRect rect);
	void snap_to_axis(bool vert = true);
	void set_free(bool to) { m_free = to; }
	void simulate(automa::ServiceProvider& svc, int amount);
	float compute_resistance();
	std::vector<Spring> links{};
	std::vector<components::PhysicsComponent> joints{};
	[[nodiscard]] bool moving() const;
	[[nodiscard]] auto get_recoil_force() const -> sf::Vector2f { return -m_avg_velocity * m_resistance; }
	[[nodiscard]] auto get_percentage_colliding() const -> float;

	util::BitFlags<ChainFlags> flags{};

  private:
	sf::Vector2f m_root{};
	std::optional<sf::Vector2f> m_centroid{};
	sf::Vector2f m_avg_velocity{};
	float m_resistance{};
	float m_external_dampen;
	float m_grav{1.f};
	float m_radius{};
	int m_num_collisions{};
	bool m_free{};
	sf::Sprite m_sprite;
};

} // namespace fornani::vfx
