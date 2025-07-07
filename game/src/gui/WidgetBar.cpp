#include "fornani/gui/WidgetBar.hpp"

#include "fornani/entities/packages/Health.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::gui {

WidgetBar::WidgetBar(automa::ServiceProvider& svc, int amount, sf::Vector2<int> dimensions, sf::Texture const& texture, sf::Vector2f origin, float pad, bool compress)
	: m_text{svc.text.fonts.title}, m_quantity{amount}, m_compress{compress} {
	m_text.setCharacterSize(16);
	m_position = origin;
	m_widgets.clear();
	for (auto i{0}; i < amount; ++i) {
		m_widgets.push_back(Widget(svc, texture, dimensions, i));
		m_widgets.back().position = {i * dimensions.x + i * pad, 0.f};
		m_widgets.back().gravitator.set_position(m_widgets.back().position);
		m_widgets.back().origin = origin;
	}
}

void WidgetBar::update(automa::ServiceProvider& svc, entity::Health& health, bool shake) {
	int i{};
	m_quantity = health.get_i_max();
	auto qty = "x" + std::to_string(health.get_i_hp());
	m_text.setString(qty);
	for (auto& widget : m_widgets) {
		if (shake) {
			auto const randv = util::random::random_vector_float(-16.f, 16.f);
			widget.gravitator.set_position(widget.position + randv);
			widget.shake();
		}
		widget.update(svc, static_cast<int>(health.get_max()));
		widget.current_state = health.get_hp() > i ? WidgetState::neutral : health.get_taken_point() > i ? WidgetState::taken : WidgetState::gone;
		auto const flashing = health.restored.running() && health.restored.get() % 48 > 24 && health.get_hp() > i;
		widget.current_state = flashing ? WidgetState::added : widget.current_state;
		++i;
	}
}

void WidgetBar::render(sf::RenderWindow& win) {
	if (m_quantity > 16 && m_compress) {
		auto& widget = m_widgets.at(0);
		widget.render(win);
		m_text.setPosition(widget.get_position() + sf::Vector2f{16.f, 12.f});
		m_text.setFillColor(colors::navy_blue);
		win.draw(m_text);
		m_text.setPosition(widget.get_position() + sf::Vector2f{14.f, 10.f});
		widget.current_state == WidgetState::neutral ? m_text.setFillColor(colors::ui_white) : widget.current_state == WidgetState::taken ? m_text.setFillColor(colors::dark_goldenrod) : m_text.setFillColor(colors::blue);
		if (widget.current_state != WidgetState::gone) { win.draw(m_text); }
	} else {
		for (auto& widget : m_widgets) { widget.render(win); }
	}
}

} // namespace fornani::gui
