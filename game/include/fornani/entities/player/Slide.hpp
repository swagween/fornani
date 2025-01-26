
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Direction.hpp"
#include "fornani/utils/Cooldown.hpp"
#include <cmath>
#include <iostream>

namespace player {

enum class SlideFlags { break_out, started };

class Slide {
  public:
	void start() {
		flags.set(SlideFlags::started);
		begin_slide.start();
		friction.start();
	}
	void end() {
		post_slide.start();
		flags.reset(SlideFlags::started);
	}
	void break_out() { flags.set(SlideFlags::break_out); }
	void update();
	void calculate();
	void slide();
	[[nodiscard]] auto going() const -> bool { return begin_slide.get_cooldown() == begin_slide.get_native_time() - 1; }
	[[nodiscard]] auto can_exit() const -> bool { return begin_slide.is_complete(); }
	[[nodiscard]] auto can_begin() const -> bool { return post_slide.is_complete(); }
	[[nodiscard]] auto exhausted() const -> bool { return get_dampen() < slowness_limit; }
	[[nodiscard]] auto broke_out() -> bool { return flags.consume(SlideFlags::break_out); }
	[[nodiscard]] auto started() const -> bool { return flags.test(SlideFlags::started); }
	[[nodiscard]] auto get_speed() const -> float { return speed; }
	[[nodiscard]] auto get_dampen() const -> float { return begin_normal * multiplier; }
	dir::Direction direction{};

  private:
	util::Cooldown begin_slide{128};
	util::Cooldown friction{256};
	util::Cooldown post_slide{48};
	float speed{4.5f};
	float multiplier{1.5f};
	float slowness_limit{0.2f};
	float begin_normal{};
	util::BitFlags<SlideFlags> flags{};
};

} // namespace player
