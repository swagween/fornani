
#include "fornani/graphics/DayNightShifter.hpp"
#include <fornani/core/Debug.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

namespace fornani::graphics {

void DayNightShifter::render(automa::ServiceProvider& svc, sf::RenderTarget& win, sf::Sprite& sprite, int ctr, float temp) {
	if (auto alpha = get_temperature_alpha(ctr, temp)) {
		sprite.setColor({255, 255, 255, *alpha});
		win.draw(sprite);
		++debug::draw_calls;
	} else {
		auto const cycle = svc.world_clock.as_trio();
		auto const from_cycle = svc.world_clock.get_previous_as_trio();
		bool const is_next = cycle == (from_cycle + 1) % 3;

		if (cycle == ctr) {
			std::uint8_t const alpha = !is_next ? 255 : util::get_uint8_from_normal(1.f - svc.world_clock.get_transition());

			sprite.setColor({255, 255, 255, alpha});
			win.draw(sprite);
			++debug::draw_calls;
		} else if (ctr == from_cycle && svc.world_clock.is_transitioning()) {
			std::uint8_t const alpha = is_next ? 255 : util::get_uint8_from_normal(svc.world_clock.get_transition());

			sprite.setColor({255, 255, 255, alpha});
			win.draw(sprite);
			++debug::draw_calls;
		}
	}

	sprite.setColor(sf::Color::White);
}

void DayNightShifter::render(automa::ServiceProvider& svc, sf::RenderTarget& win, sf::Sprite& sprite, int ctr, std::uint8_t const native_alpha, float temp) {
	if (auto alpha = get_temperature_alpha(ctr, temp, native_alpha)) {
		sprite.setColor({255, 255, 255, *alpha});
		win.draw(sprite);
		++debug::draw_calls;
	} else {
		auto const cycle = svc.world_clock.as_trio();
		auto const from_cycle = svc.world_clock.get_previous_as_trio();

		if (cycle == ctr) {
			std::uint8_t const alpha = from_cycle > cycle ? native_alpha : static_cast<std::uint8_t>(std::lerp(0.f, static_cast<float>(native_alpha), 1.f - svc.world_clock.get_transition()));

			sprite.setColor({255, 255, 255, alpha});
			win.draw(sprite);
			++debug::draw_calls;
		} else if (ctr == from_cycle && svc.world_clock.is_transitioning()) {
			std::uint8_t const alpha = from_cycle <= cycle ? native_alpha : static_cast<std::uint8_t>(std::lerp(0.f, static_cast<float>(native_alpha), svc.world_clock.get_transition()));

			sprite.setColor({255, 255, 255, alpha});
			win.draw(sprite);
			++debug::draw_calls;
		}
	}

	sprite.setColor(sf::Color::White);
}

auto DayNightShifter::get_temperature_alpha(int ctr, float temp, std::uint8_t native_alpha) const -> std::optional<std::uint8_t> {
	if (ctr != 0 && ctr != 1) { return std::nullopt; }

	float const blend = std::clamp(temp, 0.f, 1.f);
	float const alpha = ctr == 0 ? std::lerp(0.f, static_cast<float>(native_alpha), blend)	// twilight
								 : std::lerp(static_cast<float>(native_alpha), 0.f, blend); // day

	return static_cast<std::uint8_t>(alpha);
}

} // namespace fornani::graphics
