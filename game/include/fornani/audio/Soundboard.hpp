
#pragma once

#include <SFML/Audio.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "Sound.hpp"
#include <unordered_map>

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class SoundboardState { on, off };

enum class Menu { select, shift, forward_switch, backward_switch };
enum class Console { next, done, shift, select, speech, menu_open };
enum class World { load, save, soft_sparkle, soft_sparkle_high, chest, breakable_shatter, breakable_hit, hard_hit, thud, small_crash, switch_press, block_toggle, wall_hit, soft_tap, pushable, door_open, door_unlock };
enum class Item { heal, orb_low, orb_medium, orb_high, orb_max, health_increase, gem, get, equip };
enum class Player { jump, land, arms_switch, shoot, hurt, dash, death, shield_drop, slide, walljump, roll };
enum class Weapon { bryns_gun, wasp, skycorps_ar, tomahawk, tomahawk_catch, clover, nova, hook_probe, staple, indie, gnat, energy_ball, plasmer, underdog, peckett_710 };
enum class Arms { reload };
enum class Transmission { statics };
enum class Step { basic, grass };

// critters
enum class Enemy { hit_squeak, hit_high, hit_medium, hit_low };
enum class Frdog { hurt, death };
enum class Hulmet { hurt };
enum class Tank { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Thug { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Minigus { hurt_1, hurt_2, hurt_3, laugh, laugh_2, jump, land, step, punch, snap, build_invincibility, invincible, invincibility_lost, ok };
enum class Minigun { charge, reload, neutral, firing };
enum class Demon { hurt, signal, death, snort };
enum class Archer { hurt, flee, death };
enum class Beamstalk { hurt, death };
enum class Meatsquash { hurt, death, chomp, whip, swallow };

class Soundboard {
  public:
	Soundboard(automa::ServiceProvider& svc);
	void play_sounds(automa::ServiceProvider& svc, int echo_count = 0, int echo_rate = 1);
	void turn_on() { status = SoundboardState::on; }
	void turn_off() { status = SoundboardState::off; }
	void play_step(int tile_value, int style_id, bool land = false);
	[[nodiscard]] auto sound_pool_size() const -> size_t { return sound_pool.size(); }
	[[nodiscard]] auto number_of_playng_sounds() -> int;
	struct {
		util::BitFlags<Menu> menu{};
		util::BitFlags<Console> console{};
		util::BitFlags<World> world{};
		util::BitFlags<Item> item{};
		util::BitFlags<Player> player{};
		util::BitFlags<Weapon> weapon{};
		util::BitFlags<Arms> arms{};
		util::BitFlags<Transmission> transmission{};
		util::BitFlags<Step> step{};
		util::BitFlags<Step> land{};

		util::BitFlags<Enemy> enemy{};
		util::BitFlags<Frdog> frdog{};
		util::BitFlags<Hulmet> hulmet{};
		util::BitFlags<Tank> tank{};
		util::BitFlags<Thug> thug{};
		util::BitFlags<Minigus> minigus{};
		util::BitFlags<Minigun> minigun{};
		util::BitFlags<Demon> demon{};
		util::BitFlags<Archer> archer{};
		util::BitFlags<Beamstalk> beamstalk{};
		util::BitFlags<Meatsquash> meatsquash{};
	} flags{};
	
	void play(automa::ServiceProvider& svc, sf::SoundBuffer& buffer, float random_pitch_offset = 0.f, float vol = 100.f, int frequency = 0, float attenuation = 1.f, sf::Vector2<float> distance = {}, int echo_count = 0, int echo_rate = 64);

  private:
	void repeat(automa::ServiceProvider& svc, Sound& sound, int frequency, float random_pitch_offset = 0.f, float attenuation = 1.f, sf::Vector2<float> distance = {});
	void randomize(automa::ServiceProvider& svc, Sound& sound, float random_pitch_offset, float vol = 100.f, float attenuation = 1.f, sf::Vector2<float> distance = {});

	std::vector<Sound> sound_pool{};

	SoundboardState status{SoundboardState::on};

	struct {
		float save{};
	} proximities{};

	struct {
		util::Cooldown hard_hit{18};
	} cooldowns{};

	std::unordered_map<int, std::unordered_map<int, Step>> get_step_sound{{1,
																		   {{96, Step::grass},
																			{464, Step::grass},
																			{465, Step::grass},
																			{466, Step::grass},
																			{467, Step::grass},
																			{468, Step::grass},
																			{469, Step::grass},
																			{470, Step::grass},
																			{471, Step::grass},
																			{476, Step::grass},
																			{477, Step::grass},
																			{478, Step::grass},
																			{479, Step::grass},
																			{496, Step::grass},
																			{497, Step::grass}}}};
};

} // namespace audio
