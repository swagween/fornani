
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"

namespace audio {

enum class Menu { select, shift, forward_switch, backward_switch };
enum class Console { next, done, shift, select, speech, menu_open };
enum class World { load, save, soft_sparkle, soft_sparkle_high };
enum class Player { jump, step, land, arms_switch, shoot, hurt, dash };
enum class Weapon { bryns_gun, plasmer, nova, clover, tomahawk, tomahawk_catch };

//critters
enum class Frdog {hurt};
enum class Hulmet {hurt};

struct Soundboard {

	util::BitFlags<Menu> menu{};
	util::BitFlags<Console> console{};
	util::BitFlags<World> world{};
	util::BitFlags<Player> player{};
	util::BitFlags<Weapon> weapon{};

	util::BitFlags<Frdog> frdog{};
	util::BitFlags<Hulmet> hulmet{};

	void play_sounds();
	void load_audio();

	struct {
		float save{};
	} proximities{};

	
	data::ResourceFinder finder{};

	// sound effects!
	sf::SoundBuffer click_buffer{};
	sf::Sound click;
	sf::SoundBuffer sharp_click_buffer{};
	sf::Sound sharp_click;
	sf::SoundBuffer menu_shift_buffer{};
	sf::Sound menu_shift;
	sf::SoundBuffer menu_back_buffer{};
	sf::Sound menu_back;
	sf::SoundBuffer menu_next_buffer{};
	sf::Sound menu_next;

	sf::SoundBuffer arms_switch_buffer{};
	sf::Sound arms_switch;
	sf::SoundBuffer bg_shot_buffer{};
	sf::Sound bg_shot;
	sf::SoundBuffer plasmer_shot_buffer{};
	sf::Sound plasmer_shot;
	sf::SoundBuffer tomahawk_flight_buffer{};
	sf::Sound tomahawk_flight;
	sf::SoundBuffer tomahawk_catch_buffer{};
	sf::Sound tomahawk_catch;
	sf::SoundBuffer pop_mid_buffer{};
	sf::Sound pop_mid;

	sf::SoundBuffer jump_buffer{};
	sf::Sound jump;
	sf::SoundBuffer shatter_buffer{};
	sf::Sound shatter;
	sf::SoundBuffer step_buffer{};
	sf::Sound step;
	sf::SoundBuffer landed_buffer{};
	sf::Sound landed;
	sf::SoundBuffer hurt_buffer{};
	sf::Sound hurt;
	sf::SoundBuffer player_death_buffer{};
	sf::Sound player_death;
	sf::SoundBuffer enem_hit_buffer{};
	sf::Sound enem_hit;

	// save/load
	sf::SoundBuffer save_buffer{};
	sf::Sound save;
	sf::SoundBuffer load_buffer{};
	sf::Sound load;
	sf::SoundBuffer soft_sparkle_high_buffer{};
	sf::Sound soft_sparkle_high;
	sf::SoundBuffer soft_sparkle_buffer{};
	sf::Sound soft_sparkle;
};

} // namespace audio
