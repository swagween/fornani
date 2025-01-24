
#pragma once

#include "fornani/audio/Ambience.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace audio {

void Ambience::load(data::ResourceFinder& finder, std::string_view source) {
	tracks.open.turn_on();
	tracks.closed.turn_on();
	std::string source_str = source.data();
	tracks.open.simple_load(finder.resource_path() + "/audio/ambience/" + source_str + "/open");
	tracks.closed.simple_load(finder.resource_path() + "/audio/ambience/" + source_str + "/closed");
}

void audio::Ambience::play() {
	tracks.open.switch_on();
	tracks.closed.switch_on();
	tracks.open.play_looped(5);
	tracks.closed.play_looped(5);
}

void Ambience::set_balance(float balance) {
	tracks.open.update();
	tracks.closed.update();
	auto actual = balance * volume_multiplier;
	auto inverse = (100.f - balance) * volume_multiplier;
	tracks.open.set_volume(actual);
	tracks.closed.set_volume(inverse);
}

} // namespace audio
