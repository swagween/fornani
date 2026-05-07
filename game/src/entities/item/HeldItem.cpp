
#include <fornani/entities/item/HeldItem.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::item {

HeldItem::HeldItem(automa::ServiceProvider& svc, int i, int lookup) : Animatable(svc, "held_items", {10, 10}), id{i} {
	push_and_set_animation("basic", {0, 4, 24, -1});
	set_channel(lookup);
	center();
}

void HeldItem::update() { tick(); }

void HeldItem::render(sf::RenderWindow& win, sf::Vector2f where) {
	set_position(where);
	win.draw(*this);
}

} // namespace fornani::item
