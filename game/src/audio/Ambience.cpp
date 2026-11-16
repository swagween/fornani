
#include <fornani/audio/Ambience.hpp>
#include <fornani/audio/SoundMixer.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::audio {

Ambience::Ambience(capo::IEngine& audio_engine, SoundMixer& mixer) : tracks{.open{audio_engine, mixer}, .closed{audio_engine, mixer}}, m_mixer{&mixer} {}

void Ambience::load(ResourceFinder& finder, std::string_view source) {
	if (current_track == source) { return; }
	if (source.empty()) { source = "none"; }
	tracks.open.turn_on();
	tracks.closed.turn_on();
	tracks.open.volume.set_dynamic(0.0);
	tracks.closed.volume.set_dynamic(0.0);
	std::string source_str = source.data();
	tracks.open.load(finder.resource_path() + "/audio/ambience/" + source_str + "/open.xm");
	tracks.closed.load(finder.resource_path() + "/audio/ambience/" + source_str + "/closed.xm");
	current_track = source_str;
}

void Ambience::play() {
	tracks.open.play_looped();
	tracks.closed.play_looped();
}

void Ambience::set_balance(double balance) {
	tracks.open.update();
	tracks.closed.update();
	auto actual = balance * m_mixer->mix(volume.get());
	auto inverse = (1.0 - balance) * m_mixer->mix(volume.get());
	tracks.open.set_gain(static_cast<float>(actual));
	tracks.closed.set_gain(static_cast<float>(inverse));
}

} // namespace fornani::audio
