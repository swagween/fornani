
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
	~EnemyCatalog() { std::cout << "EnemyCatalog destroyed.\n"; }
	void update();
	void push_enemy(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, int id);

	std::vector<std::unique_ptr<enemy::Enemy>> enemies{};
};

} // namespace enemy