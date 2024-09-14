#pragma once
#include <SFML/Graphics.hpp>
#include "../../particle/Gravitator.hpp"

namespace entity {

class FloatingPart {
  public:
	FloatingPart() = default;
	FloatingPart(sf::Texture& tex, float force, float friction, sf::Vector2<float> offset);
	void update(automa::ServiceProvider& svc, dir::Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos) { gravitator->set_position(pos); }
	void set_force(float force) { gravitator->attraction_force = force; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator->collider.bounding_box.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return gravitator->collider.physics.velocity; }

  private:
	std::unique_ptr<vfx::Gravitator> gravitator{};
	sf::Sprite sprite{};
	sf::Vector2<float> left{};
	sf::Vector2<float> right{};
	sf::Vector2<float> actual{};
	struct {
		float time{};
		float rate{0.02f};
		float magnitude{6.f};
	} movement{};
	bool init{};
};

} // namespace entity