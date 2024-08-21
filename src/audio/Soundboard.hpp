
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "../utils/BitFlags.hpp"
#include <unordered_map>

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class SoundboardState { on, off };

enum class Menu { select, shift, forward_switch, backward_switch };
enum class Console { next, done, shift, select, speech, menu_open };
enum class World { load, save, soft_sparkle, soft_sparkle_high, chest, breakable_shatter, breakable_hit, hard_hit, thud, small_crash };
enum class Item { heal, orb_low, orb_medium, orb_high, orb_max };
enum class Player { jump, step, land, arms_switch, shoot, hurt, dash, death, shield_drop };
enum class Weapon { bryns_gun, plasmer, skycorps_ar, nova, clover, tomahawk, tomahawk_catch, hook_probe, staple, indie, gnat };

// critters
enum class Frdog { hurt, death };
enum class Hulmet { hurt };
enum class Tank { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Thug { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Minigus { hurt_1, hurt_2, hurt_3, laugh, laugh_2, jump, land, step, punch, snap, build_invincibility, invincible, invincibility_lost, ok };
enum class Minigun { charge, reload, neutral, firing };

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
		util::BitFlags<Tank> tank{};
		util::BitFlags<Thug> thug{};
		util::BitFlags<Minigus> minigus{};
		util::BitFlags<Minigun> minigun{};
	} flags{};

	SoundboardState status{SoundboardState::on};

	void play_sounds(automa::ServiceProvider& svc);
	void repeat(automa::ServiceProvider& svc, sf::Sound& sound, int frequency, float random_pitch_offset = 0.f);
	void randomize(automa::ServiceProvider& svc, sf::Sound& sound, float random_pitch_offset, float vol = 100.f);
	void play_at_volume(sf::Sound& sound, float vol);
	void turn_on() { status = SoundboardState::on; }
	void turn_off() { status = SoundboardState::off; }

	struct {
		float save{};
	} proximities{};

	std::unordered_map<std::string_view, Weapon> gun_sounds{
		{"bryn's gun", Weapon::bryns_gun},		{"plasmer", Weapon::plasmer},		{"skycorps ar", Weapon::skycorps_ar}, {"nova", Weapon::nova},		  {"clover", Weapon::clover}, {"tomahawk", Weapon::tomahawk},
		{"grappling hook", Weapon::hook_probe}, {"grenade launcher", Weapon::nova}, {"minigun", Weapon::plasmer},		  {"staple gun", Weapon::staple}, {"indie", Weapon::indie},	  {"gnat", Weapon::gnat}};
};

} // namespace audio
