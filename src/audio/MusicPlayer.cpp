
#pragma once

#include "MusicPlayer.hpp"
#include "../setup/ServiceLocator.hpp"

namespace audio {
void MusicPlayer::load(std::string song_name) {
	song_first.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_first.wav");
	song_loop.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_loop.ogg");
}
void MusicPlayer::play_once() {
	if (state_flags.test(SongState::off)) {
		stop();
		return;
	}
	song_first.setLoop(false);
	song_first.setVolume(60);
	song_first.play();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::play_looped() {
	if (state_flags.test(SongState::off)) {
		stop();
		return;
	}
	song_first.setLoop(false);
	song_loop.setLoop(true);
	song_first.setVolume(60);
	song_loop.setVolume(60);
	song_first.play();
	music_clock.restart();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::update() {
	if (state_flags.test(SongState::off)) {
		stop();
		return;
	}
	auto song_dt = (song_first.getDuration() - music_clock.getElapsedTime()).asMilliseconds();
	if (song_dt < svc::tickerLocator.get().tick_rate) {
		if (song_loop.getStatus() == sf::SoundSource::Status::Playing) { return; }
		status = sf::SoundSource::Status::Stopped;
	}
	if (status != sf::SoundSource::Status::Playing) {
		std::chrono::milliseconds wait_time = std::chrono::milliseconds{(int)(svc::tickerLocator.get().tick_rate - song_dt)};
		std::this_thread::sleep_for(wait_time * 20);
		song_loop.play();
		status = sf::SoundSource::Status::Playing;
	}
}
void MusicPlayer::pause() {}
void MusicPlayer::stop() {
	song_first.stop();
	song_loop.stop();
}
void MusicPlayer::fade_out() {}
void MusicPlayer::fade_in() {}
void MusicPlayer::turn_off() { state_flags.set(SongState::off); }
void MusicPlayer::turn_on() { state_flags.reset(SongState::off); }
} // namespace audio
