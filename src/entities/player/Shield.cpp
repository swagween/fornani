#include "Shield.hpp"

namespace player {

void Shield::start() {
	timer.start(stats.time);
	states.set(ShieldState::shielding);
}

void Shield::end() { states.reset(ShieldState::shielding); }

void Shield::update() { timer.update(); }

void Shield::reset_triggers() { triggers = {}; }

void Shield::reset_all() {
	triggers = {};
	states = {};
}

} // namespace player
