
#include "fornani/audio/MusicPlayer.hpp"
#include <ccmath/ext/clamp.hpp>
#include <filesystem>

namespace fornani::audio {

using namespace std::chrono_literals;

MusicPlayer::MusicPlayer(capo::IEngine& audio_engine) : m_jukebox{audio_engine}, m_ringtone{audio_engine} {}

void MusicPlayer::quick_play(ResourceFinder const& finder, std::string_view song_name) {
	m_jukebox.stop();
	if (is_off()) { return; }
	auto path = std::filesystem::path{finder.resource_path() + "/audio/songs/" + song_name.data() + ".xm"};
	m_ringtone.load_media(path);
	m_ringtone.play(false);
	m_ringtone.set_gain(m_jukebox.get_gain());
}

void MusicPlayer::load(ResourceFinder const& finder, std::string_view song_name) {
	if (is_off()) { return; }
	if (song_name.empty()) { return; }
	if (song_name == m_current_song) { return; }
	m_current_song = song_name;
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

void MusicPlayer::resume() { m_jukebox.play(); }

void MusicPlayer::fade_out(std::chrono::duration<float> duration) { m_jukebox.set_fade_in(duration, get_volume()); }

void MusicPlayer::fade_in(std::chrono::duration<float> duration) { m_jukebox.set_fade_out(duration); }

void MusicPlayer::turn_off() {
	stop();
	m_state = MusicPlayerState::off;
}

void MusicPlayer::turn_on() { m_state = MusicPlayerState::on; }

void MusicPlayer::set_volume(float vol) { m_jukebox.set_gain(ccm::ext::clamp(vol, 0.f, 1.f)); }

void MusicPlayer::adjust_volume(float delta) { set_volume(get_volume() + delta); }

} // namespace fornani::audio
