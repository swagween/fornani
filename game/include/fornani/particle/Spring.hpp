
#pragma once

#include <optional>
#include "fornani/components/CircleSensor.hpp"
#include "fornani/components/PhysicsComponent.hpp"

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
	void simulate(float custom_grav = 1.5f, bool loose = false, bool sag = false);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void calculate_force();
	void reverse_anchor_and_bob();
	void set_anchor(sf::Vector2f point);
	void set_bob(sf::Vector2f point);
	void set_rest_length(float point);
	void set_force(float force);
	void lock() { locked = true; };
	sf::Vector2f& get_bob();
	sf::Vector2f& get_anchor();
	sf::Vector2f get_rope(int index);
	SpringParameters& get_params() { return params; }
	components::CircleSensor sensor{8.f};
	std::optional<Spring*> cousin{};
	[[nodiscard]] auto is_locked() const -> bool { return locked; }
	[[nodiscard]] auto get_equilibrium_point() const -> float { return params.grav / params.spring_constant; }

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
	sf::Vector2f coil{};
	float spring_max{64.f};
	bool locked{};

	SpringParameters params{};

	// drawables for debugging
	sf::CircleShape bob_shape{};
	sf::CircleShape anchor_shape{};
};

} // namespace fornani::vfx
