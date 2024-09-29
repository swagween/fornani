
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string_view>
#include <optional>
#include "../utils/CircleCollider.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/Fader.hpp"
#include "../entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace vfx {
enum class ParticleType{ animated, colliding };
class Particle {
  public:
	Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, dir::Direction direction);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto done() const -> bool { return lifespan.is_complete(); }

  private:
	sf::RectangleShape box{};
	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<int> sprite_dimensions{};
	util::Cooldown lifespan{};
	shape::CircleCollider collider;
	int frame{};
	sf::Sprite sprite{};
	std::optional<util::Fader> fader{};
	anim::Animation animation{};
	util::BitFlags<ParticleType> flags{};
};

} // namespace vfx
