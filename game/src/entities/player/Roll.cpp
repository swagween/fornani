#include "fornani/entities/player/Roll.hpp"

namespace fornani::player {
void Roll::update() { roll_window.update(); }
void Roll::reset() { flags.set(RollFlags::can_roll); }
void Roll::roll() { flags.set(RollFlags::rolling); }
void Roll::break_out() { flags.reset(RollFlags::rolling); }
void Roll::request() {
	roll_window.start();
	flags.reset(RollFlags::can_roll);
}
} // namespace player
