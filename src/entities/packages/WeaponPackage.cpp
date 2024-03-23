#include "WeaponPackage.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {
void WeaponPackage::update(automa::ServiceProvider& svc, world::Map& map) {}
void WeaponPackage::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {}
void WeaponPackage::shoot() {}
} // namespace entity