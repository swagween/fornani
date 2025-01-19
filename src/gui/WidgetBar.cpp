#include "WidgetBar.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/entities/packages/Health.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include <algorithm>

namespace gui {

void WidgetBar::set(automa::ServiceProvider& svc, int amount, sf::Vector2<int> dimensions, sf::Texture& texture, sf::Vector2<float> origin, float pad) {
	widgets.clear();
	for (auto i{0}; i < amount; ++i) {
		widgets.push_back(Widget(svc, texture, dimensions, i));
		widgets.back().position = {i * dimensions.x + i * pad, 0.f};
		widgets.back().gravitator.set_position(widgets.back().position);
		widgets.back().origin = origin;
	}
}

void WidgetBar::update(automa::ServiceProvider& svc, entity::Health& health, bool shake) {
	int i{};
	for (auto& widget : widgets) {
		if (shake) {
			auto randv = svc.random.random_vector_float(-16.f, 16.f);
			widget.gravitator.set_position(widget.position + randv);
			widget.shake();
		}
		widget.update(svc, health.get_max());
		widget.current_state = health.get_hp() > i ? State::neutral : health.get_taken_point() > i ? State::taken : State::gone;
		auto flashing = health.restored.running() && health.restored.get_cooldown() % 48 > 24 && health.get_hp() > i;
		widget.current_state = flashing ? State::added : widget.current_state;
		++i;
	}
}

void WidgetBar::render(sf::RenderWindow& win) {
	for (auto& widget : widgets) { widget.render(win); }
}

} // namespace gui
