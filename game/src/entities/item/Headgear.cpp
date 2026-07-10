
#include <fornani/entities/item/Headgear.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::item {

Headgear::Headgear(automa::ServiceProvider& svc, int i, int lookup) : Animatable(svc, "headgear", {26, 26}), id{i} {
	set_channel(lookup);
	center();
}

void Headgear::update(int frame) { set_frame(frame); }

void Headgear::render(sf::RenderWindow& win, sf::Vector2f where) {
	set_position(where);
	win.draw(*this);
}

} // namespace fornani::item
