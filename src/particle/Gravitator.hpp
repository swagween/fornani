
#pragma once

#include <string>
#include "../utils/Collider.hpp"

namespace automa {
struct ServiceProvider;
}

namespace vfx {

class Gravitator {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Gravitator() = default;
	Gravitator(Vec pos, sf::Color col, float agf, Vec size = {4.f, 4.f});
	void update(automa::ServiceProvider& svc);
	void add_force(sf::Vector2<float> force);
	void set_position(Vec new_position);
	void set_target_position(Vec new_position);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos, int history = 0);
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return collider.physics.position; }

	shape::Collider collider{};
	Vecu16 scaled_position{};
	Vec dimensions{};

	sf::Color color{};
	sf::RectangleShape box{};
	float attraction_force{};
};

} // namespace vfx
