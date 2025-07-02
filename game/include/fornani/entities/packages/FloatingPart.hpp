#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/particle/Gravitator.hpp"

#include <vector>

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::entity {

class FloatingPart {
  public:
	FloatingPart(sf::Texture const& tex, float force, float friction, sf::Vector2<float> offset = {}, int id = 0);
	FloatingPart(sf::Texture const& tex, sf::Vector2i dimensions, std::vector<anim::Parameters> params, std::vector<std::string_view> labels, float force, float friction, sf::Vector2<float> offset = {}, int id = 0);
	FloatingPart(sf::Color color, sf::Vector2f dimensions, float force, float friction, sf::Vector2<float> offset = {}, int id = 0);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, Direction direction, sf::Vector2<float> scale, sf::Vector2<float> position);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos) const { gravitator->set_position(pos); }
	void set_force(float force) const { gravitator->attraction_force = force; }
	void set_shield(sf::Vector2<float> dim = {}, sf::Vector2<float> pos = {});
	void set_hitbox(sf::Vector2<float> dim = {}, sf::Vector2<float> pos = {});
	void set_magnitude(float magnitude);
	void move(sf::Vector2<float> distance) const;

	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator->collider.bounding_box.get_position(); }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return gravitator->collider.get_center(); }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2<float> { return gravitator->collider.physics.velocity; }
	[[nodiscard]] auto get_id() const -> int { return m_id; }

	std::optional<sf::Sprite> sprite{};
	std::optional<anim::AnimatedSprite> animated_sprite{};

  private:
	int m_id{};
	std::unique_ptr<vfx::Gravitator> gravitator{};
	sf::Vector2<float> left{};
	sf::Vector2<float> right{};
	sf::Vector2<float> actual{};
	struct {
		float time{};
		float rate{0.03f};
		float magnitude{4.f};
	} m_movement{};
	bool init{};
	bool textured{};
	std::optional<sf::RectangleShape> drawbox{};
	std::optional<shape::Shape> hitbox{};
	std::optional<shape::Shape> shieldbox{};
	sf::RectangleShape debugbox{};

};

} // namespace fornani::entity
