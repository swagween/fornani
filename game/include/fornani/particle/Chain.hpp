
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/SpriteRotator.hpp>
#include <fornani/particle/Spring.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <vector>

namespace fornani::vfx {

enum class ChainFlags { ignore_player_collision, linked, broken, no_collision };

struct ChainParameters {
	float resistance{};
	float tensile_strength{};
	float rigidity{};
	float external_dampen{};
	float gravity{};
};

class Chain {
  public:
	Chain(automa::ServiceProvider& svc, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f, bool linked = false);
	Chain(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i dim, SpringParameters params, sf::Vector2f position, int num_links, bool reversed = false, float spacing = 0.f, bool linked = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool average = true);
	void set_gravity(float g) { parameters.gravity = g; }
	void set_position(sf::Vector2f to_position);
	void set_end_position(sf::Vector2f to_position);
	void lock_ends();
	void snap_to_axis(bool vert = true);
	void set_free(bool to) { m_free = to; }
	void simulate(automa::ServiceProvider& svc, int amount);
	void set_num_angles(int to) { m_num_angles = to; }
	void break_all();
	float compute_resistance();

	sf::Vector2f get_tangent(std::size_t i) const;
	[[nodiscard]] bool moving() const;
	[[nodiscard]] auto get_recoil_force() const -> sf::Vector2f { return -m_avg_velocity * parameters.resistance; }
	[[nodiscard]] auto get_percentage_colliding() const -> float;
	[[nodiscard]] auto contains_point(sf::Vector2f test) const -> bool;

  public:
	std::vector<Spring> links{};
	std::vector<components::PhysicsComponent> joints{};
	util::BitFlags<ChainFlags> flags{};
	std::optional<Animatable> sprite{};
	ChainParameters parameters{};

  private:
	sf::Vector2f m_root{};
	std::optional<sf::Vector2f> m_centroid{};
	sf::Vector2f m_avg_velocity{};
	vfx::SpriteRotator m_rotator{};
	float m_radius{};
	int m_num_collisions{};
	int m_num_angles{};
	bool m_free{};
};

} // namespace fornani::vfx
