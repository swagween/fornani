
#pragma once

#include <string>
#include "../utils/Shape.hpp"
#include "../components/PhysicsComponent.hpp"

namespace vfx {
struct SpringParameters {
	float dampen_factor{};
	float spring_constant{};
	float rest_length{};
};
class Spring {
  public:
	Spring() = default;
	Spring(SpringParameters params);
	void calculate();
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void calculate_force();
	void reverse_anchor_and_bob();
	void set_anchor(sf::Vector2<float> point);
	void set_bob(sf::Vector2<float> point);
	void set_rest_length(float point);
	void set_force(float force);
	sf::Vector2<float>& get_bob();
	sf::Vector2<float>& get_anchor();
	sf::Vector2<float> get_rope(int index);

	int num_links{8};

	struct {
		float extension{};
		sf::Vector2<float> spring_force{};
		components::PhysicsComponent physics{};
	} variables{};

  private:
	sf::Vector2<float> anchor{};
	sf::Vector2<float> bob{};
	sf::Vector2<float> coil{};

	SpringParameters params{};

	//drawables for debugging
	sf::CircleShape bob_shape{};
	sf::CircleShape anchor_shape{};
};

} // namespace vfx
