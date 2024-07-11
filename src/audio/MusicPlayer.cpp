
#pragma once

#include "MusicPlayer.hpp"
#include "../service/ServiceProvider.hpp"

namespace audio {

void MusicPlayer::load(std::string_view song_name) {
	if (global_off()) { return; }
	if (label == song_name && playing()) { return; }
	if (song_name == "") { return; }
	if (song_name == "none") {
		stop();
		return;
	}
	label = song_name;
	song_first.openFromFile(finder.resource_path + "/audio/songs/" + song_name.data() + "_first.wav");
	song_loop.openFromFile(finder.resource_path + "/audio/songs/" + song_name.data() + "_loop.wav");
	switch_on();
}
void MusicPlayer::play_once(float vol) {
	if (global_off()) { return; }
	if (switched_off()) { return; }
	if (playing()) { return; }
	volume.native = vol;
	song_first.setLoop(false);
	song_first.setVolume(volume.actual);
	song_first.play();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::play_looped(float vol) {
	if (global_off()) { return; }
	if (switched_off()) { return; }
	if (playing()) { return; }
	volume.native = vol;
	song_first.setLoop(false);
	song_loop.setLoop(true);
	song_first.setVolume(volume.actual);
	song_loop.setVolume(volume.actual);
	song_first.play();
	music_clock.restart();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::update() {
	if (global_off()) { return; }
	volume.actual = volume.native * volume.multiplier;
	set_volume(volume.actual);
	if (!flags.state.test(SongState::on)) {
		stop();
		return;
	}
	auto song_dt = (song_first.getDuration() - music_clock.getElapsedTime()).asMilliseconds();
	if (song_dt < 0) {
		if (song_loop.getStatus() == sf::SoundSource::Status::Playing) { return; }
		status = sf::SoundSource::Status::Stopped;
	}
	if (status != sf::SoundSource::Status::Playing) {
		song_loop.play();
		status = sf::SoundSource::Status::Playing;
	}
}

void MusicPlayer::pause() {
	song_first.pause();
	song_loop.pause();
	switch_off();
}
void MusicPlayer::stop() {
	song_first.stop();
	song_loop.stop();
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
	song_first.setVolume(vol);
	song_loop.setVolume(vol);
}
} // namespace audio
