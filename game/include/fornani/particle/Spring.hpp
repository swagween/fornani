
#pragma once

#include <fornani/components/CircleSensor.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/RingBuffer.hpp>
#include <optional>

namespace fornani::vfx {
struct SpringParameters {
	float dampen_factor{};
	float spring_constant{};
	float rest_length{};
	float grav{1.f};
	float mass{1.f};
};
class Spring {
  public:
	Spring() = default;
	Spring(SpringParameters params);
	Spring(SpringParameters params, sf::Vector2f anchor, sf::Vector2f bob);
	void calculate();
	void update(automa::ServiceProvider& svc, float custom_grav = 1.5f, sf::Vector2f external_force = {}, bool loose = false, bool sag = false);
	void update_constrained(automa::ServiceProvider& svc, float custom_grav = 1.5f, sf::Vector2f external_force = {});
	void simulate(float custom_grav = 1.5f, bool loose = false, bool sag = false);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void calculate_force();
	void reverse_anchor_and_bob();
	void set_anchor(sf::Vector2f point);
	void set_bob(sf::Vector2f point);
	void set_rest_length(float point);
	void set_force(float force);
	void lock() { locked = true; };
	void set_channel(int to) { m_channel = to; }
	sf::Vector2f get_bob() const { return bob; };
	sf::Vector2f get_anchor() const { return anchor; };
	sf::Vector2f& get_bob();
	sf::Vector2f& get_anchor();
	sf::Vector2f get_rope(int index);
	SpringParameters& get_params() { return params; }
	void fade(int time);
	components::CircleSensor sensor{8.f};
	std::optional<Spring*> cousin{};

	[[nodiscard]] auto get_fade() -> util::Cooldown& { return m_fade; }
	[[nodiscard]] auto is_locked() const -> bool { return locked; }
	[[nodiscard]] auto get_channel() const -> int { return m_channel; }
	[[nodiscard]] auto get_equilibrium_point() const -> float { return params.grav / params.spring_constant; }
	[[nodiscard]] auto get_average_bob_position() const -> sf::Vector2f { return m_bob_positions.average(); }
	[[nodiscard]] auto get_direction_vector() const -> sf::Vector2f;

	int num_links{8};

	struct {
		float extension{};
		sf::Vector2f spring_force{};
		components::PhysicsComponent bob_physics{};
		components::PhysicsComponent anchor_physics{};
	} variables{};

  private:
	sf::Vector2f anchor{};
	sf::Vector2f bob{};
	sf::Vector2f m_average_bob{};
	RingBuffer<sf::Vector2f> m_bob_positions;
	sf::Vector2f coil{};
	float spring_max{64.f};
	bool locked{};
	int m_channel{};
	util::Cooldown m_fade;

	SpringParameters params{};

	// drawables for debugging
	sf::CircleShape bob_shape{};
	sf::CircleShape anchor_shape{};
};

} // namespace fornani::vfx
