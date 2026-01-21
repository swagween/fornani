
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

namespace fornani::gui {
class Console;
}

namespace fornani::enemy {

struct Multispawn {
	sf::Vector2f spread{};
};

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	explicit EnemyCatalog(automa::ServiceProvider& svc);
	~EnemyCatalog() {}
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, int id, bool spawned = false, int variant = 0, sf::Vector2<int> start_direction = {-1, 0},
					Multispawn multispawn = {});

	std::vector<std::unique_ptr<Enemy>> enemies{};
};

} // namespace fornani::enemy
