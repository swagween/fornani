
#include "editor/canvas/entity/Destructible.hpp"

namespace pi {

Destructible::Destructible(dj::Json const& in) : Entity(in, "destructibles") { unserialize(in); }

std::unique_ptr<Entity> Destructible::clone() const { return std::make_unique<Destructible>(*this); }

void Destructible::serialize(dj::Json& out) { Entity::serialize(out); }

void Destructible::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void Destructible::expose() { Entity::expose(); }

void Destructible::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
