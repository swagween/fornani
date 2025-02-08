#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/Collider.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

enum class BreakableAttributes { bulletproof };

class Breakable {
  public:
	Breakable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style = 0, int state = 4);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power = 1);
	void on_smash(automa::ServiceProvider& svc, world::Map& map, int power = 1);
	void destroy() { state = 0; }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto destroyed() const -> bool { return state <= 1; }
	shape::Collider collider{};

  private:
	util::BitFlags<BreakableAttributes> attributes{};
	sf::Sprite sprite;
	int style{};
	int state{};
	float energy{};
	float dampen{0.1f};
	float hit_energy{8.f};
	sf::Vector2<float> random_offset{};

};
} // namespace world