
#pragma once

#include <SFML/Graphics.hpp>
#include <capo/engine.hpp>
#include <ccmath/ext/clamp.hpp>
#include <fornani/audio/Sound.hpp>
#include <functional>
#include <unordered_map>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

template <typename Enum>
std::function<void(int)> make_int_setter(util::BitFlags<Enum>& flags) {
	return [&flags](int which) { flags.set(static_cast<Enum>(which)); };
}

enum class SoundboardState { on, off };

enum class Menu { select, shift, forward_switch, backward_switch, error };
enum class Pioneer { select, click, back, open, close, slot, chain, boot, buzz, fast_click, hard_slot, hum, sync, scan, drag, wires, forward, unhover };
enum class Console { next, done, shift, select, speech, menu_open, notification };
enum class World {
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
	big_crash,
	heavy_land,
	delay_crash,
	gem_hit_1,
	gem_hit_2,
	laser_charge,
	laser_hum,
	laser_cooldown,
	incinerite_explosion,
	splash
};
enum class Item { heal, orb_low, orb_medium, orb_high, orb_max, health_increase, gem, get, equip, vendor_sale, unequip, drop_spawn, orb_collide, heart_collide };
enum class Player { jump, land, arms_switch, shoot, hurt, dash, death, shield_drop, slide, walljump, roll, wallslide, super_slide, doublejump, turn_slide, dash_kick, gulp, dive };
enum class Weapon { bryns_gun, wasp, skycorps_ar, tomahawk, tomahawk_catch, clover, nova, hook_probe, staple, indie, gnat, energy_ball, plasmer, underdog, peckett_710, pulse, demon_magic };
enum class Projectile { basic, shuriken, pulse, hard_hit, critical_hit };
enum class Arms { reload, frag_grenade, whistle };
enum class Transmission { statics };
enum class Step { basic, grass };

// critters
enum class Enemy { hit_squeak, hit_high, hit_medium, hit_low, standard_death, jump_low, high_death, low_death, disappear };

enum class Beast { growl, hurt, gulp, snort, roar };

enum class Frdog { hurt, death };
enum class Crow { fly, flap, caw, death };
enum class Hulmet { hurt, alert, reload };
enum class Tank { alert_1, alert_2, hurt_1, hurt_2, death, step };
enum class Thug { alert_1, alert_2, hurt_1, hurt_2, death };
enum class Minigun { charge, reload, neutral, firing };
enum class Demon { hurt, alert, death, snort, up_snort };
enum class Archer { hurt, flee, death };
enum class Beamstalk { hurt, death };
enum class Beamsprout { hurt, death, shoot, charge };
enum class Meatsquash { hurt, death, chomp, whip, swallow, open };
enum class Summoner { hurt_1, hurt_2, death, block_1, block_2, summon, hah };
enum class Mastiff { bite, growl };

enum class NPCBryn { agh, ah_1, ah_2, chuckle, nani_1, nani_2, oh, ohh, sigh, whatsup, yah, yeah, yeahh, eagh, haha, hello, hey_1, hey_2, heyyy, hi, hmm, hmph, laugh_1, laugh_2, mm, oeugh };
enum class NPCGobe { oh, orewa };
enum class NPCLynx { hmph, hmm };
enum class NPCMirin { ah, oh, haha };
enum class NPCCarl { hah, eh };
enum class NPCGo { oh, mm };
enum class NPCBit { hey, hehe };
enum class NPCMinigus { greatidea, dontlookatme, laugh, getit, pizza, grunt };

enum class Minigus {
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
enum class Lynx { prepare, shing, ping_1, ping_2, swipe_1, swipe_2, slam, hoah, defeat, hah, heuh, hiyyah, hnnyah, huh, hurt_1, hurt_2, hurt_3, hurt_4, huuyeah, nngyah, yyah, laugh, giggle };
enum class Miaag { growl, hiss, hurt, roar, chomp };

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
		util::BitFlags<Beast> beast{};
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
		util::BitFlags<Mastiff> mastiff{};
		util::BitFlags<Beamsprout> beamsprout{};
		util::BitFlags<Crow> crow{};
	} flags{};

	struct {
		util::BitFlags<NPCBryn> bryn{};
		util::BitFlags<NPCGobe> gobe{};
		util::BitFlags<NPCLynx> lynx{};
		util::BitFlags<NPCMirin> mirin{};
		util::BitFlags<NPCGo> go{};
		util::BitFlags<NPCCarl> carl{};
		util::BitFlags<NPCBit> bit{};
		util::BitFlags<NPCMinigus> minigus{};
	} npc_flags{};

	std::unordered_map<std::string, std::function<void(int)>> npc_map;

	void play(capo::IEngine& engine, automa::ServiceProvider& svc, std::string const& label, float random_pitch_offset = 0.f, float vol = 100.f, int frequency = 0, float attenuation = 1.f, sf::Vector2f distance = {}, int echo_count = 0,
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
