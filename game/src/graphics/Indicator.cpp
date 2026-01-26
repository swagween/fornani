
#include "fornani/graphics/Indicator.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::graphics {

Indicator::Indicator(automa::ServiceProvider& svc, IndicatorType type) : m_label{svc.text.fonts.title}, m_type{type} {
	m_label.setCharacterSize(16);
	m_label.setLetterSpacing(0.6f);
	if (type == IndicatorType::health) { color_fade = vfx::ColorFade({colors::ui_white, colors::red, colors::dark_fucshia}, 16, addition_time); }
	if (type == IndicatorType::orb) { color_fade = vfx::ColorFade({colors::ui_white, colors::goldenrod, colors::dark_orange}, 16, addition_time); }
	float fric{0.85f};
	m_steering.physics.set_friction_componentwise({fric, fric});
}

void Indicator::shift() { position += {0.f, -20.f}; }

void Indicator::update(automa::ServiceProvider& svc, sf::Vector2f pos) {
	m_steering.seek(pos);
	if (addition_limit.get() == fadeout_time) { fadeout.start(fadeout_time); }
	if (fadeout.running()) { m_steering.seek(pos + sf::Vector2f{60.f, 0.f}); }
	addition_limit.update();
	fadeout.update();
	color_fade.update();
	if (addition_limit.is_complete()) { variables.amount = 0; }
	position = m_steering.physics.position + offset;
	if (m_type == IndicatorType::orb) {
		if (variables.amount < 0.f) {
			color_fade.change_colors({colors::ui_white, colors::periwinkle, colors::navy_blue});
		} else {
			color_fade.change_colors({colors::ui_white, colors::goldenrod, colors::dark_orange});
		}
	}
}

void Indicator::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		return;
	} else if (!addition_limit.is_complete()) {
		m_label.setPosition(position + shadow - cam);
		m_type == IndicatorType::health || variables.amount < 0.f ? m_label.setFillColor(colors::ui_black) : m_label.setFillColor(colors::dark_fucshia);
		win.draw(m_label);
		m_label.setFillColor(color_fade.color());
		m_label.setPosition(position - cam);
		win.draw(m_label);
	}
}

void Indicator::add(float amount) {
	variables.amount += amount;
	std::string sign = variables.amount >= 0 ? "+" : "";
	auto round = static_cast<int>(variables.amount);
	m_label.setString(sign + std::to_string(round));
	addition_limit.start(addition_time);
	color_fade.start();
}

void Indicator::set_position(sf::Vector2f pos) {
	position = pos;
	m_steering.physics.position = pos;
}

} // namespace fornani::graphics
