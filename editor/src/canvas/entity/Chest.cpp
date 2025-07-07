
#include "editor/canvas/entity/Chest.hpp"

namespace pi {

Chest::Chest() : Entity("chests") { repeatable = false; }

Chest::Chest(int type, int modifier, int id) : Entity("chests", id, {1, 1}), m_type{type}, m_content_modifier{modifier} { repeatable = false; }

std::unique_ptr<Entity> Chest::clone() const { return std::make_unique<Chest>(*this); }

void Chest::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = m_type;
	out["modifier"] = m_content_modifier;
}

void Chest::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["type"].as<int>();
	m_content_modifier = in["modifier"].as<int>();
}

void Chest::expose() { Entity::expose(); }

void Chest::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{250, 120, 80, 80}) : drawbox.setFillColor(sf::Color{250, 120, 80, 10});
	Entity::render(win, cam, size);
}

} // namespace pi
