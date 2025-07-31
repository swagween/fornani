
#include "fornani/audio/Ambience.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::audio {

Ambience::Ambience(capo::IEngine& audio_engine) : tracks{.open{audio_engine}, .closed{audio_engine}} {}

void Ambience::load(ResourceFinder& finder, std::string_view source) {
	if (source.empty()) { return; }
	tracks.open.turn_on();
	tracks.closed.turn_on();
	std::string source_str = source.data();
	tracks.open.load(finder.resource_path() + "/audio/ambience/" + source_str + "/open.xm");
	tracks.closed.load(finder.resource_path() + "/audio/ambience/" + source_str + "/closed.xm");
}

void Ambience::play() {
	tracks.open.play_looped();
	tracks.closed.play_looped();
}

void Ambience::set_balance(float balance) {
	tracks.open.update();
	tracks.closed.update();
	auto actual = balance * volume_multiplier;
	auto inverse = (100.f - balance) * volume_multiplier;
	tracks.open.set_volume(actual);
	tracks.closed.set_volume(inverse);
}

} // namespace fornani::audio
