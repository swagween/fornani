#include "Health.hpp"
#include "../../service/ServiceProvider.hpp"
#include <algorithm>

namespace entity {

void Health::set_max(float max) {
	max_hp = max;
	hp = max;
}

void Health::update(automa::ServiceProvider& svc, world::Map& map) { hp = std::clamp(hp, 0.f, max_hp); }

void Health::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	drawbox.setFillColor(svc.styles.colors.dark_orange);
	drawbox.setSize({max_hp, 4});
	win.draw(drawbox);
	drawbox.setFillColor(svc.styles.colors.green);
	drawbox.setSize({hp, 4});
	win.draw(drawbox);
};

void Health::inflict(float amount) { hp -= amount; }

} // namespace entity