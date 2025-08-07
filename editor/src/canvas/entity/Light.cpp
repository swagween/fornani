
#include "editor/canvas/entity/Light.hpp"

namespace pi {

Light::Light(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "lights") {
	unserialize(in);
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	repeatable = true;
	m_textured = false;
}

Light::Light(fornani::automa::ServiceProvider& svc, int id, std::string_view type) : Entity(svc, "lights", id), m_type{type} {
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	repeatable = true;
	m_textured = false;
}

std::unique_ptr<Entity> Light::clone() const { return std::make_unique<Light>(*this); }

void Light::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["label"] = m_type;
}

void Light::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["label"].as_string();
}

void Light::expose() { Entity::expose(); }

void Light::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 20});
	Entity::render(win, cam, size);
}

} // namespace pi
