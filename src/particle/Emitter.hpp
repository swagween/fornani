
#pragma once

#include <vector>
#include "Particle.hpp"

namespace vfx {

class Emitter {
  public:
	Emitter() = default;
	Emitter(sf::Vector2<float> position, sf::Vector2<float> dimensions, std::string_view type, sf::Color color, dir::Direction direction);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos);
	void set_dimensions(sf::Vector2<float> dim);
	void deactivate();
	[[nodiscard]] auto done() const -> bool { return particles.empty(); }

  private:
	std::vector<Particle> particles{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> particle_dimensions{3.f, 3.f}; // customize later
	sf::Vector2<float> position{};

	std::string_view type{};
	sf::Color color{};
	util::Cooldown cooldown{};
	dir::Direction direction{};
	bool active{true};
};

} // namespace vfx
