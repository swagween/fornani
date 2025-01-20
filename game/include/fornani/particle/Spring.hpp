
#pragma once

#include <string>
#include "../utils/Shape.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../components/CircleSensor.hpp"
#include <optional>

namespace vfx {
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
	Spring(SpringParameters params, sf::Vector2<float> anchor, sf::Vector2<float> bob);
	void calculate();
	void update(automa::ServiceProvider& svc, float custom_grav = 1.5f, sf::Vector2<float> external_force = {}, bool loose = false, bool sag = false);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void calculate_force();
	void reverse_anchor_and_bob();
	void set_anchor(sf::Vector2<float> point);
	void set_bob(sf::Vector2<float> point);
	void set_rest_length(float point);
	void set_force(float force);
	void lock() { locked = true; };
	sf::Vector2<float>& get_bob();
	sf::Vector2<float>& get_anchor();
	sf::Vector2<float> get_rope(int index);
	SpringParameters& get_params() { return params; }
	components::CircleSensor sensor{8.f};
	std::optional<Spring*> cousin{};
	[[nodiscard]] auto is_locked() const -> bool { return locked; }
	[[nodiscard]] auto get_equilibrium_point() const -> float { return params.grav / params.spring_constant; }

	int num_links{8};

	struct {
		float extension{};
		sf::Vector2<float> spring_force{};
		components::PhysicsComponent bob_physics{};
		components::PhysicsComponent anchor_physics{};
	} variables{};

  private:
	sf::Vector2<float> anchor{};
	sf::Vector2<float> bob{};
	sf::Vector2<float> coil{};
	float spring_max{64.f};
	bool locked{};

	SpringParameters params{};

	//drawables for debugging
	sf::CircleShape bob_shape{};
	sf::CircleShape anchor_shape{};
};

} // namespace vfx
