
#pragma once

#include <SFML/Audio.hpp>
#include "../utils/BitFlags.hpp"
#include "../utils/Cooldown.hpp"
#include <unordered_map>

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class SoundboardState { on, off };

enum class Menu { select, shift, forward_switch, backward_switch };
enum class Console { next, done, shift, select, speech, menu_open };
enum class World { load, save, soft_sparkle, soft_sparkle_high, chest, breakable_shatter, breakable_hit, hard_hit, thud, small_crash, switch_press, block_toggle, wall_hit, pushable, door_open, door_unlock };
enum class Item { heal, orb_low, orb_medium, orb_high, orb_max, health_increase, gem, get, equip };
enum class Player { jump, land, arms_switch, shoot, hurt, dash, death, shield_drop, slide };
enum class Weapon { bryns_gun, plasmer, skycorps_ar, nova, clover, tomahawk, tomahawk_catch, hook_probe, staple, indie, gnat, energy_ball, wasp };
enum class Transmission { statics };
enum class Step { basic, grass };

// critters
enum class Frdog { hurt, death };
enum class Hulmet { hurt };
enum class Tank { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Thug { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Minigus { hurt_1, hurt_2, hurt_3, laugh, laugh_2, jump, land, step, punch, snap, build_invincibility, invincible, invincibility_lost, ok };
enum class Minigun { charge, reload, neutral, firing };
enum class Demon { hurt, signal, death, snort };

struct Soundboard {
	struct {
		util::BitFlags<Menu> menu{};
		util::BitFlags<Console> console{};
		util::BitFlags<World> world{};
		util::BitFlags<Item> item{};
		util::BitFlags<Player> player{};
		util::BitFlags<Weapon> weapon{};
		util::BitFlags<Transmission> transmission{};
		util::BitFlags<Step> step{};
		util::BitFlags<Step> land{};

		util::BitFlags<Frdog> frdog{};
		util::BitFlags<Hulmet> hulmet{};
		util::BitFlags<Tank> tank{};
		util::BitFlags<Thug> thug{};
		util::BitFlags<Minigus> minigus{};
		util::BitFlags<Minigun> minigun{};
		util::BitFlags<Demon> demon{};
	} flags{};

	SoundboardState status{SoundboardState::on};

	void play_sounds(automa::ServiceProvider& svc);
	void repeat(automa::ServiceProvider& svc, sf::Sound& sound, int frequency, float random_pitch_offset = 0.f);
	void randomize(automa::ServiceProvider& svc, sf::Sound& sound, float random_pitch_offset, float vol = 100.f);
	void play_at_volume(sf::Sound& sound, float vol);
	void turn_on() { status = SoundboardState::on; }
	void turn_off() { status = SoundboardState::off; }
	void play_step(int tile_value, int style_id, bool land = false);

	struct {
		float save{};
	} proximities{};

	struct {
		util::Cooldown hard_hit{18};
	} cooldowns{};

	std::unordered_map<int, std::unordered_map<int, Step>> get_step_sound{{1,
																		   {{1, Step::grass},
																			{208, Step::grass},
																			{209, Step::grass},
																			{210, Step::grass},
																			{211, Step::grass},
																			{212, Step::grass},
																			{213, Step::grass},
																			{214, Step::grass},
																			{215, Step::grass},
																			{220, Step::grass},
																			{221, Step::grass},
																			{222, Step::grass},
																			{223, Step::grass}}}};
};

} // namespace audio
