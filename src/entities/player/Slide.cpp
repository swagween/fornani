#include "Slide.hpp"

namespace player {

void Slide::update() {
	begin_slide.update();
	post_slide.update();
	friction.update();
}

void Slide::calculate() { begin_normal = static_cast<float>(friction.get_cooldown()) / static_cast<float>(friction.get_native_time()); }

void Slide::slide() { flags.reset(SlideFlags::break_out); }

} // namespace player
