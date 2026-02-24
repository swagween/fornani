
#include <ccmath/ext/clamp.hpp>
#include <fornani/audio/MusicPlayer.hpp>
#include <fornani/utils/Constants.hpp>
#include <filesystem>

namespace fornani::audio {

using namespace std::chrono_literals;
constexpr auto global_volume_damp_v = 0.45f; // music is just too loud in general

MusicPlayer::MusicPlayer(capo::IEngine& audio_engine) : m_jukebox{audio_engine}, m_ringtone{audio_engine}, m_volume_multiplier{0.5f} {
	m_name_from_id.insert({0, "none"});
	m_name_from_id.insert({1, "glitchified"});
	m_filter.hi_target = 80.f;
	m_filter.lo_target = 40.f;
}

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
	m_jukebox.set_gain(ccm::ext::clamp(m_volume_multiplier * global_volume_damp_v, 0.f, 1.f));
	m_current_song = song_name;
	auto path = std::filesystem::path{finder.resource_path() + "/audio/songs/" + song_name.data() + ".xm"};
	m_jukebox.load_media(path);
}

void MusicPlayer::load(std::string_view path) {
	if (is_off()) { return; }
	m_jukebox.load_media(std::filesystem::path{path});
}

void MusicPlayer::play_song_by_id(ResourceFinder const& finder, int id) {
	if (!m_name_from_id.contains(id)) { return; }
	m_jukebox.stop();
	if (is_off()) { return; }
	auto path = std::filesystem::path{finder.resource_path() + "/audio/songs/" + m_name_from_id.at(id).data() + ".xm"};
	m_ringtone.load_media(path);
	m_ringtone.play();
	m_ringtone.set_gain(m_jukebox.get_gain());
}

void MusicPlayer::play_once() {
	if (is_off()) { return; }
	m_jukebox.play(false);
}

void MusicPlayer::play_looped() {
	if (is_off()) { return; }
	m_jukebox.play();
}

void MusicPlayer::update() {
	if (!m_jukebox.has_file()) { return; }
	auto balance = m_balance;
	auto hi = m_filter.hi_target * balance;
	auto lo = (juke::sample_rate_v - m_filter.lo_target) * balance;
	auto hi_cutoff = std::clamp(hi, constants::small_value, juke::sample_rate_v);
	auto lo_cutoff = std::clamp(juke::sample_rate_v - lo, constants::small_value, juke::sample_rate_v);
	m_jukebox.set_cutoff(juke::FilterType::high, hi_cutoff, juke::sample_rate_v);
	m_jukebox.set_cutoff(juke::FilterType::low, lo_cutoff, juke::sample_rate_v);
}

void MusicPlayer::pause() { m_jukebox.pause(); }

void MusicPlayer::stop() { m_jukebox.stop(); }

void MusicPlayer::resume() { m_jukebox.play(); }

void MusicPlayer::fade_out(std::chrono::duration<float> duration) { m_jukebox.set_fade_in(duration, m_jukebox.get_gain()); }

void MusicPlayer::fade_in(std::chrono::duration<float> duration) { m_jukebox.set_fade_in(duration, 1.f); }

void MusicPlayer::turn_off() {
	stop();
	m_state = MusicPlayerState::off;
}

void MusicPlayer::turn_on() { m_state = MusicPlayerState::on; }

void MusicPlayer::adjust_volume(float delta) {
	m_volume_multiplier = std::clamp(m_volume_multiplier + delta, 0.f, 1.f);
	m_jukebox.set_gain(std::clamp(m_volume_multiplier * global_volume_damp_v, 0.f, 1.f));
}

auto MusicPlayer::get_volume() const -> float { return m_jukebox.get_gain() / global_volume_damp_v; }

} // namespace fornani::audio
