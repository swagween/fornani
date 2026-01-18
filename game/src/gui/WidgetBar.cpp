
#include "fornani/gui/WidgetBar.hpp"
#include "fornani/entities/packages/Health.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::gui {

WidgetBar::WidgetBar(automa::ServiceProvider& svc, int amount, sf::Vector2i dimensions, std::string_view tag, sf::Vector2f origin, float pad, bool compress)
	: m_text{svc.text.fonts.title}, m_quantity{amount}, m_compress{compress}, m_position{origin} {
	m_text.setCharacterSize(16);
	for (auto i{0}; i < amount; ++i) { m_widgets.push_back(Widget(svc, tag, dimensions, i, origin + sf::Vector2f{i * dimensions.x * constants::f_scale_factor + i * pad, 0.f})); }
}

void WidgetBar::update(automa::ServiceProvider& svc, entity::Health& health, bool shake) {
	m_quantity = health.get_i_capacity();
	auto qty = "x" + std::to_string(health.get_i_quantity());
	m_text.setString(qty);
	for (auto [i, widget] : std::views::enumerate(m_widgets)) {
		i >= health.get_native_capacity() ? widget.set_type(WidgetType::secondary) : widget.set_type(WidgetType::basic);
		widget.set_offset(m_position);
		if (shake) {
			auto const randv = random::random_vector_float(-16.f, 16.f);
			widget.set_physics_offset(randv);
			widget.shake();
		}
		widget.update(svc, static_cast<int>(health.get_capacity()));
		auto taken = health.is_taken() && health.get_taken_point() > i;
		widget.set_state(health.get_quantity() > i ? WidgetState::neutral : taken ? WidgetState::taken : WidgetState::gone);
		auto const flashing = health.restored.running() && health.restored.get() % 48 > 24 && health.get_quantity() > i;
		if (flashing) { widget.set_state(WidgetState::added); }
	}
}

void WidgetBar::render(sf::RenderWindow& win) {
	if (m_quantity > 16 && m_compress) {
		auto& widget = m_widgets.at(0);
		widget.render(win);
		m_text.setPosition(widget.get_window_position() + sf::Vector2f{16.f, 12.f});
		m_text.setFillColor(colors::navy_blue);
		win.draw(m_text);
		m_text.setPosition(widget.get_window_position() + sf::Vector2f{14.f, 10.f});
		widget.is_state(WidgetState::neutral) ? m_text.setFillColor(colors::ui_white) : widget.is_state(WidgetState::taken) ? m_text.setFillColor(colors::dark_goldenrod) : m_text.setFillColor(colors::blue);
		if (!widget.is_state(WidgetState::gone)) { win.draw(m_text); }
	} else {
		for (auto& widget : m_widgets) { widget.render(win); }
	}
}

void WidgetBar::set_origin(sf::Vector2f const to) {
	for (auto& widget : m_widgets) { widget.set_origin(to); }
}

} // namespace fornani::gui
