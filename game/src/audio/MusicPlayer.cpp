
#include "fornani/audio/MusicPlayer.hpp"
#include <ccmath/ext/clamp.hpp>
#include <filesystem>

namespace fornani::audio {

MusicPlayer::MusicPlayer(capo::IEngine& audio_engine) : m_jukebox{audio_engine} {}

void MusicPlayer::load(data::ResourceFinder const& finder, std::string_view song_name) {
	if (is_off()) { return; }
	// if (song_name.empty()) { return; }
	auto path = std::filesystem::path{finder.resource_path() + "/audio/songs/" + song_name.data() + ".xm"};
	m_jukebox.load_media(path);
}

void MusicPlayer::load(std::string_view path) {
	if (is_off()) { return; }
	m_jukebox.load_media(std::filesystem::path{path});
}

void MusicPlayer::play_once() {
	if (is_off()) { return; }
	m_jukebox.play(false);
}

void MusicPlayer::play_looped() {
	if (is_off()) { return; }
	m_jukebox.play();
}

void MusicPlayer::update() {}

void MusicPlayer::pause() { m_jukebox.pause(); }

void MusicPlayer::stop() { m_jukebox.stop(); }

void MusicPlayer::fade_out() {}

void MusicPlayer::fade_in() {}

void MusicPlayer::turn_off() {
	stop();
	m_state = MusicPlayerState::off;
}

void MusicPlayer::turn_on() { m_state = MusicPlayerState::on; }

void MusicPlayer::set_volume(float vol) {}

void MusicPlayer::set_volume_multiplier(float to) { m_volume_multiplier = ccm::ext::clamp(to, 0.f, 1.f); }

} // namespace fornani::audio
