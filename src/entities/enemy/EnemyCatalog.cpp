#include <iostream>
#include "EnemyCatalog.hpp"
#include "../enemy/catalog/Frdog.hpp"
#include "../enemy/catalog/Tank.hpp"

namespace enemy {

EnemyCatalog::EnemyCatalog(automa::ServiceProvider& svc) {}

void EnemyCatalog::update() {
	std::erase_if(enemies, [this](auto const& e) { return e->died(); });
}

void EnemyCatalog::push_enemy(automa::ServiceProvider& svc, int id) {
	switch (id) {
	case 5: enemies.push_back(std::make_unique<Tank>(svc)); break;
	default: enemies.push_back(std::make_unique<Frdog>(svc)); break;
	}
}

} // namespace enemy
