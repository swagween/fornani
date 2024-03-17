
#pragma once
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

class EnemyCatalog {
  public:
	EnemyCatalog() = default;
	EnemyCatalog(automa::ServiceProvider& svc);
	void update();
	void push_enemy(automa::ServiceProvider& svc, int id);
	Enemy& get_enemy(int id);

	std::vector<int> enemy_pool{};
	std::unordered_map<int, Enemy> enemy_table{};

	Frdog frdog;
};

} // namespace enemy