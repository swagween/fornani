#include <iostream>
#include "EnemyCatalog.hpp"
#include "../enemy/catalog/Frdog.hpp"

namespace enemy {

EnemyCatalog::EnemyCatalog(automa::ServiceProvider& svc) : frdog(svc) { enemy_table.insert({5, frdog}); }

void EnemyCatalog::update() {
	std::erase_if(enemy_pool, [this](auto const& e) { return get_enemy(e).died(); });
}

void EnemyCatalog::push_enemy(automa::ServiceProvider& svc, int id) { enemy_pool.push_back(id); }

Enemy& EnemyCatalog::get_enemy(int id) {
	
	return enemy_table.at(id);
	
}

} // namespace enemy
