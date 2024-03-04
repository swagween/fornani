
#pragma once

#include "MusicPlayer.hpp"
#include "../setup/ServiceLocator.hpp"

namespace audio {
void MusicPlayer::load(std::string song_name) {
	song_first.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_first.wav");
	song_loop.openFromFile(finder.resource_path + "/audio/songs/" + song_name + "_loop.wav");
}
void MusicPlayer::play_once() {
	song_first.setLoop(false);
	//song_first.play();
}
void MusicPlayer::play_looped() {
	song_first.setLoop(false);
	song_loop.setLoop(true);
	//song_first.play();
}
void MusicPlayer::update() {
	if (song_first.getStatus() == sf::SoundSource::Stopped) {
		song_first.setPlayingOffset(sf::Time::Zero);
		song_first.stop();
		//song_loop.play();
	}
}
void MusicPlayer::pause() {}
void MusicPlayer::stop() {}
void MusicPlayer::fade_out() {}
void MusicPlayer::fade_in() {}
} // namespace audio
