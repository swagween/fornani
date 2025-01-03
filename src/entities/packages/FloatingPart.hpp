#pragma once
#include <SFML/Graphics.hpp>
#include "../../particle/Gravitator.hpp"

namespace player {
class Player;
}

namespace world {
class Map;
}

namespace entity {

class FloatingPart {
  public:
	FloatingPart() = default;
	FloatingPart(sf::Texture& tex, float force, float friction, sf::Vector2<float> offset);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, dir::Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos) { gravitator->set_position(pos); }
	void set_force(float force) { gravitator->attraction_force = force; }
	void set_shield(sf::Vector2<float> dim = {}, sf::Vector2<float> pos = {});
	void set_hitbox(sf::Vector2<float> dim = {}, sf::Vector2<float> pos = {});
	void move(sf::Vector2<float> distance);
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator->collider.bounding_box.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return gravitator->collider.physics.velocity; }
	sf::Sprite sprite;

  private:
	std::unique_ptr<vfx::Gravitator> gravitator{};
	sf::Vector2<float> left{};
	sf::Vector2<float> right{};
	sf::Vector2<float> actual{};
	struct {
		float time{};
		float rate{0.03f};
		float magnitude{4.f};
	} movement{};
	bool init{};
	std::optional<shape::Shape> hitbox{};
	std::optional<shape::Shape> shieldbox{};
	sf::RectangleShape debugbox{};
};

} // namespace entity