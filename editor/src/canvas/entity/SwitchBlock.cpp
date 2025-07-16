
#include "editor/canvas/entity/SwitchBlock.hpp"

namespace pi {

SwitchBlock::SwitchBlock(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "switch_blocks") {
	unserialize(in);
	set_texture_rect(sf::IntRect{{fornani::constants::i_cell_resolution * m_type, 0}, fornani::constants::i_resolution_vec});
	repeatable = true;
}

SwitchBlock::SwitchBlock(fornani::automa::ServiceProvider& svc, int id, int type) : Entity(svc, "switch_blocks", id), m_type{type} {
	set_texture_rect(sf::IntRect{{fornani::constants::i_cell_resolution * type, 0}, fornani::constants::i_resolution_vec});
	repeatable = true;
}

std::unique_ptr<Entity> SwitchBlock::clone() const { return std::make_unique<SwitchBlock>(*this); }

void SwitchBlock::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = m_type;
}

void SwitchBlock::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["type"].as<int>();
}

void SwitchBlock::expose() {
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

void SwitchBlock::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
}

} // namespace pi
