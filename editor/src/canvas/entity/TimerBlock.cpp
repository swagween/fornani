
#include "editor/canvas/entity/TimerBlock.hpp"

namespace pi {

TimerBlock::TimerBlock(fornani::automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "timer_blocks") {
	unserialize(in);
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	repeatable = true;
}

TimerBlock::TimerBlock(fornani::automa::ServiceProvider& svc, int id, int type) : Entity(svc, "timer_blocks", id), m_type{type} {
	set_texture_rect(sf::IntRect{{}, fornani::constants::i_resolution_vec});
	repeatable = true;
}

std::unique_ptr<Entity> TimerBlock::clone() const { return std::make_unique<TimerBlock>(*this); }

void TimerBlock::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = m_type;
}

void TimerBlock::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = in["type"].as<int>();
}

void TimerBlock::expose() {
	Entity::expose();
	ImGui::InputInt("Type", &m_type);
}

void TimerBlock::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 20});
	Entity::render(win, cam, size);
}

} // namespace pi
