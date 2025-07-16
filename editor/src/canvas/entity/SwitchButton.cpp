
#include "editor/canvas/entity/SwitchButton.hpp"

namespace pi {

SwitchButton::SwitchButton(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "switch_buttons") {
	unserialize(in);
	set_texture_rect(sf::IntRect{{fornani::constants::i_cell_resolution * m_type, 0}, {fornani::constants::i_cell_resolution, 8}});
	set_origin({0.f, -8.f});
}

SwitchButton::SwitchButton(fornani::automa::ServiceProvider& svc, int id, int type) : Entity(svc, "switch_buttons", id), m_type{type} {
	set_texture_rect(sf::IntRect{{fornani::constants::i_cell_resolution * type, 0}, {fornani::constants::i_cell_resolution, 8}});
	set_origin({0.f, -8.f});
}

std::unique_ptr<Entity> SwitchButton::clone() const { return std::make_unique<SwitchButton>(*this); }

void SwitchButton::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = m_type;
}

void SwitchButton::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["type"].as<int>();
}

void SwitchButton::expose() {
	Entity::expose();
	static int type{m_type};
	static char const* types[4] = {"toggler", "permanent", "movable", "alternator"};

	auto ctr{0};
	if (ImGui::BeginCombo("Type", types[type])) {
		for (auto const& t : types) {
			if (ImGui::Selectable(t)) { type = ctr; }
			++ctr;
		}
		ImGui::EndCombo();
	}

	m_type = type;
}

void SwitchButton::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
