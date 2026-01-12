
#include "fornani/audio/Ambience.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::audio {

Ambience::Ambience(capo::IEngine& audio_engine) : tracks{.open{audio_engine}, .closed{audio_engine}}, m_volume_multiplier{0.3f}, m_in_game_multiplier{1.f} {}

void Ambience::load(ResourceFinder& finder, std::string_view source) {
	if (source.empty()) { source = "none"; }
	tracks.open.turn_on();
	tracks.closed.turn_on();
	tracks.open.set_volume(0.f);
	tracks.closed.set_volume(0.f);
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
	auto actual = balance * m_volume_multiplier * m_in_game_multiplier;
	auto inverse = (1.f - balance) * m_volume_multiplier * m_in_game_multiplier;
	tracks.open.set_volume(actual);
	tracks.closed.set_volume(inverse);
}

} // namespace fornani::audio
