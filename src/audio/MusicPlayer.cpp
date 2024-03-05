
#pragma once

#include "MusicPlayer.hpp"
#include "../setup/ServiceLocator.hpp"

namespace audio {
void MusicPlayer::load(std::string song_name) {
	song_first.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_first.wav");
	song_loop.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_loop.ogg");
}
void MusicPlayer::play_once() {
	song_first.setLoop(false);
	song_first.setVolume(60);
	song_first.play();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::play_looped() {
	song_first.setLoop(false);
	song_loop.setLoop(true);
	song_first.setVolume(60);
	song_loop.setVolume(60);
	song_first.play();
	status = sf::SoundSource::Status::Playing;
}
void MusicPlayer::update() {
	if (song_first.getStatus() == sf::SoundSource::Stopped) {
		if (song_loop.getStatus() == sf::SoundSource::Status::Playing) { return; }
		song_first.setPlayingOffset(sf::Time::Zero);
		song_first.stop();
		status = sf::SoundSource::Status::Stopped;
	}
	if (status != sf::SoundSource::Status::Playing) {
		song_loop.play();
		status = sf::SoundSource::Status::Playing;
	}
}
void MusicPlayer::pause() {}
void MusicPlayer::stop() {}
void MusicPlayer::fade_out() {}
void MusicPlayer::fade_in() {}
} // namespace audio
