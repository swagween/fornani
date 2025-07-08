
#include "editor/canvas/entity/Bed.hpp"

namespace pi {

Bed::Bed(dj::Json const& in) : Entity(in, "beds") { unserialize(in); }

Bed::Bed(int id, bool flipped) : Entity("beds", id), m_flipped{flipped} {}

std::unique_ptr<Entity> Bed::clone() const { return std::make_unique<Bed>(*this); }

void Bed::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["flipped"] = m_flipped;
}

void Bed::unserialize(dj::Json const& in) { m_flipped = in["flipped"].as_bool(); }

void Bed::expose() {
	Entity::expose();
	ImGui::Checkbox("Flipped", &m_flipped);
}

void Bed::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 60, 120, 180}) : drawbox.setFillColor(sf::Color{60, 60, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
