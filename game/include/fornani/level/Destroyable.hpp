#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/utils/BitFlags.hpp"
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

class Map;

enum class DestroyerState { detonated };

class Destroyable {
  public:
	Destroyable(automa::ServiceProvider& svc, sf::Vector2<int> pos, int quest_id, int style_id = 0);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) const;
	shape::Shape& get_bounding_box();
	[[nodiscard]] auto detonated() const -> bool { return flags.test(DestroyerState::detonated); }
  private:
	int quest_id{};
	shape::Collider collider{};
	sf::Vector2<int> position{};
	util::BitFlags<DestroyerState> flags{};
	sf::Sprite sprite;
};
} // namespace world