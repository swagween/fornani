
#include "editor/canvas/entity/SavePoint.hpp"

namespace pi {

SavePoint::SavePoint(dj::Json const& in) : Entity(in, "save_point") { unserialize(in); }

SavePoint::SavePoint(int id) : Entity("save_point", id, {1, 1}) { unique = true; }

std::unique_ptr<Entity> SavePoint::clone() const { return std::make_unique<SavePoint>(*this); }

void SavePoint::serialize(dj::Json& out) { Entity::serialize(out); }

void SavePoint::unserialize(dj::Json const& in) { Entity::unserialize(in); }

void SavePoint::expose() { Entity::expose(); }

void SavePoint::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 10, 250, 100}) : drawbox.setFillColor(sf::Color{250, 10, 250, 60});
	Entity::render(win, cam, size);
}

} // namespace pi
