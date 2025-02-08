
#pragma once

#include "fornani/utils/Collider.hpp"
#include "fornani/components/SteeringBehavior.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::vfx {

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
	void demagnetize(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos, int history = 0);
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return collider.physics.position; }

	shape::Collider collider{};
	Vecu16 scaled_position{};
	Vec dimensions{};

	sf::Color color{};
	sf::RectangleShape box{};
	float attraction_force{};

  private:
	components::SteeringBehavior steering{};
};

} // namespace vfx
