
#include "../../service/ServiceProvider.hpp"
#include <iostream>
#include "Fireflies.hpp"

namespace vfx {

void vfx::Fireflies::update(automa::ServiceProvider& svc) {
	for (auto& fly : flies) { fly.update(svc); }
}

void Fireflies::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {}

} // namespace vfx
