
#pragma once
#include "../enemy/catalog/Frdog.hpp"

namespace gui {
class Console;
}

namespace enemy {

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	EnemyCatalog(automa::ServiceProvider& svc);
	~EnemyCatalog() {}
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, int id, bool spawned = false, int variant = 0, sf::Vector2<int> start_direction = {-1, 0});

	std::vector<std::unique_ptr<enemy::Enemy>> enemies{};
};

} // namespace enemy