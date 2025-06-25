
#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <string_view>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/CircleCollider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Fader.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {
enum class ParticleType : std::uint8_t { animated, colliding };
class Particle {
  public:
	Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, Direction direction);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto done() const -> bool { return lifespan.is_almost_complete(); }

  private:
	sf::RectangleShape box{};
	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<int> sprite_dimensions{};
	util::Cooldown lifespan{};
	shape::CircleCollider collider;
	int frame{};
	sf::Sprite sprite;
	std::optional<util::Fader> fader{};
	anim::Animation animation{};
	util::BitFlags<ParticleType> flags{};
};

} // namespace fornani::vfx
