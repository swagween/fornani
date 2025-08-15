
#include "editor/canvas/entity/Animator.hpp"

namespace pi {

Animator::Animator(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "animators") {
	unserialize(in);
	repeatable = true;
	set_texture_rect(sf::IntRect{{16 * get_id(), 0}, {16, 16}});
}

Animator::Animator(fornani::automa::ServiceProvider& svc, int id, std::string_view label) : Entity(svc, "animators", id, {1, 1}), m_label{label} {
	repeatable = true;
	set_texture_rect(sf::IntRect{{16 * id, 0}, {16, 16}});
}

std::unique_ptr<Entity> Animator::clone() const { return std::make_unique<Animator>(*this); }

void Animator::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["label"] = m_label;
	out["foreground"] = m_foreground;
}

void Animator::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_label = in["label"].as_string();
	m_foreground = in["foreground"].as_bool();
}

void Animator::expose() { Entity::expose(); }

void Animator::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{120, 250, 250, 60}) : drawbox.setFillColor(sf::Color::Transparent);
	Entity::render(win, cam, size);
}

} // namespace pi
