
#include "fornani/graphics/DayNightShifter.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

namespace fornani::graphics {

void DayNightShifter::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr) {
	auto cycle = static_cast<int>(svc.world_clock.get_time_of_day());
	auto from_cycle = static_cast<int>(svc.world_clock.get_previous_time_of_day());
	if (cycle == ctr) { // new time of day sprite
		std::uint8_t alpha = from_cycle > cycle ? 255 : util::get_uint8_from_normal(1.f - svc.world_clock.get_transition());
		sprite.setColor({255, 255, 255, alpha});
		win.draw(sprite);
	} else if (ctr == from_cycle && svc.world_clock.is_transitioning()) { // old time of day sprite
		std::uint8_t alpha = from_cycle <= cycle ? 255 : util::get_uint8_from_normal(svc.world_clock.get_transition());
		sprite.setColor({255, 255, 255, alpha});
		win.draw(sprite);
	}
	sprite.setColor(sf::Color::White);
}

void DayNightShifter::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr, std::uint8_t const native_alpha) {
	auto cycle = static_cast<int>(svc.world_clock.get_time_of_day());
	auto from_cycle = static_cast<int>(svc.world_clock.get_previous_time_of_day());
	if (cycle == ctr) {
		std::uint8_t alpha = from_cycle > cycle ? native_alpha : std::lerp(0, native_alpha, 1.f - svc.world_clock.get_transition());
		sprite.setColor({255, 255, 255, alpha});
		win.draw(sprite);
	} else if (ctr == from_cycle && svc.world_clock.is_transitioning()) {
		std::uint8_t alpha = from_cycle <= cycle ? native_alpha : std::lerp(0, native_alpha, svc.world_clock.get_transition());
		sprite.setColor({255, 255, 255, alpha});
		win.draw(sprite);
	}
	sprite.setColor(sf::Color::White);
}

} // namespace graphics
