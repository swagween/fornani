
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class Menu { select, shift, forward_switch, backward_switch };
enum class Console { next, done, shift, select, speech, menu_open };
enum class World { load, save, soft_sparkle, soft_sparkle_high, chest };
enum class Item { heal, orb_1, orb_5 };
enum class Player { jump, step, land, arms_switch, shoot, hurt, dash };
enum class Weapon { bryns_gun, plasmer, nova, clover, tomahawk, tomahawk_catch, hook_probe };

//critters
enum class Frdog {hurt, death};
enum class Hulmet {hurt};

struct Soundboard {
	struct {
		util::BitFlags<Menu> menu{};
		util::BitFlags<Console> console{};
		util::BitFlags<World> world{};
		util::BitFlags<Item> item{};
		util::BitFlags<Player> player{};
		util::BitFlags<Weapon> weapon{};

		util::BitFlags<Frdog> frdog{};
		util::BitFlags<Hulmet> hulmet{};
	} flags{};

	void play_sounds(automa::ServiceProvider& svc);
	void repeat(automa::ServiceProvider& svc, sf::Sound& sound, int frequency, float random_pitch_offset = 0.f);
	void randomize(automa::ServiceProvider& svc, sf::Sound& sound, float random_pitch_offset);

	struct {
		float save{};
	} proximities{};
};

} // namespace audio
