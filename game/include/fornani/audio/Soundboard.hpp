
#pragma once

#include <SFML/Graphics.hpp>
#include <capo/engine.hpp>
#include <ccmath/ext/clamp.hpp>
#include <fornani/audio/Sound.hpp>
#include <unordered_map>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

enum class SoundboardState : std::uint8_t { on, off };

enum class Menu : std::uint8_t { select, shift, forward_switch, backward_switch, error };
enum class Pioneer : std::uint8_t { select, click, back, open, close, slot, chain, boot, buzz, fast_click, hard_slot, hum, sync, scan, drag, wires };
enum class Console : std::uint8_t { next, done, shift, select, speech, menu_open };
enum class World : std::uint8_t {
	load,
	save,
	chest,
	breakable_shatter,
	breakable_hit,
	hard_hit,
	thud,
	small_crash,
	switch_press,
	block_toggle,
	wall_hit,
	soft_tap,
	pushable_move,
	door_open,
	door_unlock,
	projectile_hit,
	clink,
	vibration,
	gem_hit_1,
	gem_hit_2
};
enum class Item : std::uint8_t { heal, orb_low, orb_medium, orb_high, orb_max, health_increase, gem, get, equip, vendor_sale };
enum class Player : std::uint8_t { jump, land, arms_switch, shoot, hurt, dash, death, shield_drop, slide, walljump, roll, wallslide, super_slide };
enum class Weapon : std::uint8_t { bryns_gun, wasp, skycorps_ar, tomahawk, tomahawk_catch, clover, nova, hook_probe, staple, indie, gnat, energy_ball, plasmer, underdog, peckett_710, pulse, demon_magic };
enum class Projectile : std::uint8_t { basic, shuriken, pulse };
enum class Arms : std::uint8_t { reload };
enum class Transmission : std::uint8_t { statics };
enum class Step : std::uint8_t { basic, grass };

// critters
enum class Enemy : std::uint8_t { hit_squeak, hit_high, hit_medium, hit_low, standard_death, jump_low, high_death, low_death };

enum class Frdog : std::uint8_t { hurt, death };
enum class Hulmet : std::uint8_t { hurt, alert };
enum class Tank : std::uint8_t { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Thug : std::uint8_t { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Minigun : std::uint8_t { charge, reload, neutral, firing };
enum class Demon : std::uint8_t { hurt, alert, death, snort, up_snort };
enum class Archer : std::uint8_t { hurt, flee, death };
enum class Beamstalk : std::uint8_t { hurt, death };
enum class Meatsquash : std::uint8_t { hurt, death, chomp, whip, swallow };
enum class Summoner : std::uint8_t { hurt_1, hurt_2, death, block_1, block_2, summon, hah };

enum class Minigus : std::uint8_t {
	hurt_1,
	hurt_2,
	hurt_3,
	laugh_1,
	laugh_2,
	jump,
	land,
	step,
	punch,
	snap,
	build,
	invincible,
	invincibility_lost,
	ok,
	crash,
	woob,
	getit,
	deepspeak,
	mother,
	momma,
	quick_breath,
	long_moan,
	poh,
	soda,
	doge,
	charge,
	pizza,
	greatidea,
	dontlookatme,
	grunt,
	exhale
};
enum class Lynx : std::uint8_t { prepare, shing, ping_1, ping_2, swipe_1, swipe_2, slam, hoah, defeat, hah, heuh, hiyyah, hnnyah, huh, hurt_1, hurt_2, hurt_3, hurt_4, huuyeah, nngyah, yyah, laugh, giggle };
enum class Miaag : std::uint8_t { growl, hiss, hurt, roar, chomp };

enum class NPC : std::uint8_t { minigus_greatidea, minigus_dontlookatme, minigus_laugh, minigus_getit, minigus_pizza, minigus_grunt };

class Soundboard {
  public:
	Soundboard(automa::ServiceProvider& svc);
	void play_sounds(capo::IEngine& engine, automa::ServiceProvider& svc, int echo_count = 0, int echo_rate = 1);
	void turn_on() { status = SoundboardState::on; }
	void turn_off() { status = SoundboardState::off; }
	void play_step(int tile_value, int style_id, bool land = false);
	void set_volume(float to) { m_volume_multiplier = ccm::ext::clamp(to, 0.f, 1.f); }
	void adjust_volume(float amount) { set_volume(m_volume_multiplier + amount); }

	[[nodiscard]] auto get_volume() const -> float { return m_volume_multiplier; }
	[[nodiscard]] auto sound_pool_size() const -> std::size_t { return sound_pool.size(); }
	[[nodiscard]] auto number_of_playng_sounds() -> int;
	struct {
		util::BitFlags<Menu> menu{};
		util::BitFlags<Console> console{};
		util::BitFlags<Pioneer> pioneer{};
		util::BitFlags<World> world{};
		util::BitFlags<Item> item{};
		util::BitFlags<Player> player{};
		util::BitFlags<Weapon> weapon{};
		util::BitFlags<Projectile> projectile{};
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
		util::BitFlags<Summoner> summoner{};
		util::BitFlags<Beamstalk> beamstalk{};
		util::BitFlags<Meatsquash> meatsquash{};
		util::BitFlags<Lynx> lynx{};
		util::BitFlags<Miaag> miaag{};
		util::BitFlags<NPC> npc{};
	} flags{};

	void play(capo::IEngine& engine, automa::ServiceProvider& svc, capo::Buffer const& buffer, float random_pitch_offset = 0.f, float vol = 100.f, int frequency = 0, float attenuation = 1.f, sf::Vector2f distance = {}, int echo_count = 0,
			  int echo_rate = 64);

  private:
	void repeat(automa::ServiceProvider& svc, Sound& sound, int frequency, float random_pitch_offset = 0.f, float attenuation = 1.f, sf::Vector2f distance = {});
	void randomize(automa::ServiceProvider& svc, Sound& sound, float random_pitch_offset, float vol = 100.f, float attenuation = 1.f, sf::Vector2f distance = {}, bool wait_until_over = false);
	void simple_repeat(capo::IEngine& engine, capo::Buffer const& buffer, std::string const& label, int fade = 16);
	void stop(std::string_view label);
	void fade_out(std::string_view label);

	std::vector<Sound> sound_pool{};
	float m_volume_multiplier{0.5f};

	SoundboardState status{SoundboardState::on};

	struct {
		float save{};
	} proximities{};

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

} // namespace fornani::audio
