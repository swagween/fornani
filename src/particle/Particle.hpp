
#pragma once

#include <vector>
#include "../components/PhysicsComponent.hpp"
#include "../utils/Shape.hpp"

namespace vfx {

using Time = std::chrono::duration<float>;
int const default_lifespan = 50;
float const default_dim = 2.f;

class Particle {
  public:
	Particle() = default;
	Particle(components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric, float sz = 3.0f, dir::Direction dir_ = dir::Direction{});
	void update(float initial_force, float grav, float grav_variance);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void oscillate_between_colors(sf::Color dark, sf::Color bright);
	components::PhysicsComponent physics{};
	float lifespan{};
	float init_force{};
	float force_variance{};
	float angle_range{};
	float size{};
	dir::Direction direction{};
	shape::Shape bounding_box{};
	sf::Sprite sprite{};
	sf::RectangleShape dot{};
	sf::Color color{};

};

} // namespace vfx
