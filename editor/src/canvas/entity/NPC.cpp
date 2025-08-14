
#include "editor/canvas/entity/NPC.hpp"

namespace pi {

NPC::NPC(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "npcs") {
	unserialize(in);
	repeatable = false;
}

NPC::NPC(fornani::automa::ServiceProvider& svc, int id, std::string_view label) : Entity(svc, "npcs", id, {1, 1}), m_label{label} { repeatable = false; }

std::unique_ptr<Entity> NPC::clone() const { return std::make_unique<NPC>(*this); }

void NPC::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["label"] = m_label;
}

void NPC::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_label = in["label"].as_string();
}

void NPC::expose() { Entity::expose(); }

void NPC::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 80, 250, 60}) : drawbox.setFillColor(sf::Color::Transparent);
	Entity::render(win, cam, size);
}

} // namespace pi
