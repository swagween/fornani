#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "../setup/EnumLookups.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Counter.hpp"
#include "../entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
class Projectile;
}

namespace world {

enum class BreakableAttributes { bulletproof };

class Breakable {
  public:
	Breakable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style = 0, int state = 4);
	void update(automa::ServiceProvider& svc);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power = 1);
	void on_smash(automa::ServiceProvider& svc, world::Map& map, int power = 1);
	void destroy() { state = 0; }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	[[nodiscard]] auto destroyed() const -> bool { return state <= 1; }

  private:
	shape::Collider collider{};
	util::BitFlags<BreakableAttributes> attributes{};
	sf::Sprite sprite{};
	int style{};
	int state{};
	float energy{};
	float dampen{0.1f};
	float hit_energy{8.f};
	sf::Vector2<float> random_offset{};

};
} // namespace world