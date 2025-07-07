
#pragma once

#include <vector>
#include "Particle.hpp"

namespace fornani::vfx {

class Emitter {
  public:
	Emitter() = default;
	Emitter(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color = sf::Color::Transparent, Direction direction = {});
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos);
	void set_dimensions(sf::Vector2f dim);
	void deactivate();
	[[nodiscard]] auto done() const -> bool { return particles.empty() && !cooldown.started(); }

  private:
	std::vector<Particle> particles{};
	sf::Vector2f dimensions{};
	sf::Vector2f particle_dimensions{3.f, 3.f}; // customize later
	sf::Vector2f position{};

	struct {
		int load{};
		float rate{};
	} variables{};

	std::string_view type{};
	sf::Color color{};
	sf::RectangleShape drawbox{}; // for debug
	util::Cooldown cooldown{};
	Direction direction{};
	bool active{true};
};

} // namespace vfx
