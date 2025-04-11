#include "fornani/audio/MusicPlayer.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Tracy.hpp"

namespace fornani::audio {

void MusicPlayer::load(data::ResourceFinder const& finder, std::string_view const song_name) {
	if (global_off()) { return; }
	if (label == song_name && playing()) { return; }
	if (song_name.empty()) { return; }
	if (song_name == "none") {
		stop();
		return;
	}
	label = song_name;
	song_first = sf::Music(finder.resource_path() + "/audio/songs/" + song_name.data() + "_first.ogg");
	song_loop = sf::Music(finder.resource_path() + "/audio/songs/" + song_name.data() + "_loop.ogg");
	switch_on();
	flags.state.reset(SongState::looping);
}

void MusicPlayer::simple_load(std::string_view source) {
	if (global_off()) { return; }
	if (label == source && playing()) { return; }
	if (source.empty()) { return; }
	if (source == "none") {
		stop();
		return;
	}
	label = source;
	std::string const path = source.data();
	song_first = sf::Music(path + ".ogg");
	song_loop = sf::Music(path + ".ogg");
	switch_on();
	flags.state.reset(SongState::looping);
}

void MusicPlayer::play_once(float vol) {
	if (global_off()) { return; }
	if (switched_off()) { return; }
	if (playing()) { return; }
	if (!song_first) { return; }
	volume.native = vol;
	song_first->setLooping(false);
	song_first->setVolume(volume.actual);
	song_first->play();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::play_looped(float vol) {
	if (global_off()) { return; }
	if (switched_off()) { return; }
	if (playing()) { return; }
	if (!song_first || !song_loop) { return; }
	volume.native = vol;
	song_first->setLooping(false);
	song_loop->setLooping(true);
	song_first->setVolume(volume.actual);
	song_loop->setVolume(volume.actual);
	song_first->play();
	music_clock.restart();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::update() {
	NANI_ZoneScopedN("MusicPlayer::update");
	if (global_off()) { return; }
	if (!song_first || !song_loop) { return; }
	volume.actual = volume.native * volume.multiplier;
	set_volume(volume.actual);
	if (!flags.state.test(SongState::on)) {
		stop();
		return;
	}
	last_dt = music_tick.getElapsedTime().asMicroseconds();
	music_tick.restart();
	auto song_dt = (song_first->getDuration() - music_clock.getElapsedTime()).asMicroseconds();
	if (song_dt < (last_dt * 2) && !flags.state.test(SongState::looping) && song_first->getStatus() != sf::Sound::Status::Playing) {
		song_loop->play();
		flags.state.set(SongState::looping);
		music_clock.restart();
	}
	if (flags.state.test(SongState::looping)) {
		song_dt = (song_loop->getDuration() - music_clock.getElapsedTime()).asMicroseconds();
		if (song_dt < (last_dt * 2)) {
			song_loop->play();
			flags.state.set(SongState::looping);
			music_clock.restart();
		}
	}
}

void MusicPlayer::pause() {
	if (!song_first || !song_loop) { return; }
	song_first->pause();
	song_loop->pause();
	switch_off();
}

void MusicPlayer::stop() {
	if (!song_first || !song_loop) { return; }
	song_first->stop();
	song_loop->stop();
	switch_off();
}

void MusicPlayer::fade_out() {}
void MusicPlayer::fade_in() {}
void MusicPlayer::switch_off() { flags.state.reset(SongState::on); }
void MusicPlayer::switch_on() { flags.state.set(SongState::on); }

void MusicPlayer::turn_off() {
	flags.player.reset(MusicPlayerState::on);
	stop();
}

void MusicPlayer::turn_on() {
	flags.player.set(MusicPlayerState::on);
	pause();
}

void MusicPlayer::set_volume(float vol) {
	if (!song_first || !song_loop) { return; }
	song_first->setVolume(vol);
	song_loop->setVolume(vol);
}

} // namespace fornani::audio
