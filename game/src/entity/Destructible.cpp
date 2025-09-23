
#include <fornani/entity/Destructible.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

Destructible::Destructible(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "destructibles") {
	unserialize(in);
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
}

Destructible::Destructible(automa::ServiceProvider& svc, int id) : Entity(svc, "destructibles", id) {
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = true;
}

std::unique_ptr<Entity> Destructible::clone() const { return std::make_unique<Destructible>(*this); }

void Destructible::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["inverse"] = m_inverse;
}

void Destructible::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_inverse = in["inverse"].as_bool();
}

void Destructible::expose() {
	Entity::expose();
	ImGui::Checkbox("Inverse", &m_inverse);
}

void Destructible::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace fornani
