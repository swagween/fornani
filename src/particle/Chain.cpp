
#include "Chain.hpp"
#include "../service/ServiceProvider.hpp"
#include <numbers>

namespace vfx {

Chain::Chain(SpringParameters params, int num_links) {
	for (int i{0}; i < num_links; ++i) { links.push_back(Spring({params})); }
}

void Chain::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	for (auto& link : links) { link.update(svc); }
}

void Chain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& link : links) { link.render(win, cam); }
}

} // namespace vfx