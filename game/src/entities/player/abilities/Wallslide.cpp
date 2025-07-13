#include "fornani/entities/player/abilities/Wallslide.hpp"

namespace fornani::player {

void Wallslide::start() { states.set(WallslideState::wallsliding); }

void Wallslide::end() { states.reset(WallslideState::wallsliding); }

void Wallslide::update() { walljump_request.update(); }

void Wallslide::reset_triggers() { triggers = {}; }

void Wallslide::reset_all() {
	triggers = {};
	states = {};
}

void Wallslide::request_walljump() { walljump_request.start(walljump_request_time); }

} // namespace fornani::player
