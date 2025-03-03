
#include "fornani/audio/Soundboard.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::audio {

Soundboard::Soundboard(automa::ServiceProvider& /*svc*/) {}

void Soundboard::play_sounds(automa::ServiceProvider& svc, int echo_count, int echo_rate) {

	std::erase_if(sound_pool, [](auto& s) { return !s.is_running(); });
	for (auto& s : sound_pool) { s.update(svc); }

	// menu
	if (flags.menu.test(Menu::forward_switch)) { play(svc, svc.sounds.get_buffer("menu_next")); }
	if (flags.menu.test(Menu::backward_switch)) { play(svc, svc.sounds.get_buffer("menu_back")); }
	if (flags.menu.test(Menu::select)) { play(svc, svc.sounds.get_buffer("menu_click")); }
	if (flags.menu.test(Menu::shift)) { play(svc, svc.sounds.get_buffer("menu_shift")); }

	// console
	if (flags.console.test(Console::select)) { play(svc, svc.sounds.get_buffer("menu_click")); }
	if (flags.console.test(Console::done)) { play(svc, svc.sounds.get_buffer("menu_back")); }
	if (flags.console.test(Console::next)) { play(svc, svc.sounds.get_buffer("menu_next")); }
	if (flags.console.test(Console::shift)) { play(svc, svc.sounds.get_buffer("menu_shift")); }
	if (flags.console.test(Console::menu_open)) { play(svc, svc.sounds.get_buffer("menu_open")); }
	if (flags.console.test(Console::speech)) { play(svc, svc.sounds.get_buffer("menu_shift"), 0.2f, 100.f, 16); }

	// pioneer
	if (flags.pioneer.test(Pioneer::back)) { play(svc, svc.sounds.get_buffer("pioneer_back")); }
	if (flags.pioneer.test(Pioneer::click)) { play(svc, svc.sounds.get_buffer("pioneer_click")); }
	if (flags.pioneer.test(Pioneer::select)) { play(svc, svc.sounds.get_buffer("pioneer_select")); }
	if (flags.pioneer.test(Pioneer::open)) { play(svc, svc.sounds.get_buffer("pioneer_open")); }
	if (flags.pioneer.test(Pioneer::close)) { play(svc, svc.sounds.get_buffer("pioneer_close")); }
	if (flags.pioneer.test(Pioneer::slot)) { play(svc, svc.sounds.get_buffer("pioneer_slot")); }
	if (flags.pioneer.test(Pioneer::chain)) { play(svc, svc.sounds.get_buffer("pioneer_chain")); }
	if (flags.pioneer.test(Pioneer::drag)) { play(svc, svc.sounds.get_buffer("pioneer_drag")); }
	flags.pioneer.test(Pioneer::buzz) ? simple_repeat(svc.sounds.get_buffer("pioneer_buzz"), "pioneer_buzz") : stop("pioneer_buzz");
	flags.pioneer.test(Pioneer::hum) ? simple_repeat(svc.sounds.get_buffer("pioneer_hum"), "pioneer_hum") : stop("pioneer_hum");
	flags.pioneer.test(Pioneer::scan) ? simple_repeat(svc.sounds.get_buffer("pioneer_scan"), "pioneer_scan") : stop("pioneer_scan");
	if (flags.pioneer.test(Pioneer::hard_slot)) { play(svc, svc.sounds.get_buffer("pioneer_hard_slot")); }
	if (flags.pioneer.test(Pioneer::fast_click)) { play(svc, svc.sounds.get_buffer("pioneer_fast_click")); }
	if (flags.pioneer.test(Pioneer::sync)) { play(svc, svc.sounds.get_buffer("pioneer_sync"), 0.1f); }
	if (flags.pioneer.test(Pioneer::boot)) { play(svc, svc.sounds.get_buffer("pioneer_boot"), 0.f, 40.f); }

	// transmission
	if (flags.transmission.test(Transmission::statics)) { play(svc, svc.sounds.get_buffer("block_toggle")); }

	// always play console and menu sounds
	if (status == SoundboardState::off) {
		flags = {};
		proximities = {};
		return;
	}

	// world
	if (flags.world.test(World::load)) { play(svc, svc.sounds.get_buffer("load_game"), 0.f, 40.f); }
	if (flags.world.test(World::save)) { play(svc, svc.sounds.get_buffer("save_game")); }
	if (flags.world.test(World::chest)) { play(svc, svc.sounds.get_buffer("chest_open"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_shatter)) { play(svc, svc.sounds.get_buffer("breakable_shatter"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_hit)) { play(svc, svc.sounds.get_buffer("breakable_hit"), 0.1f); }
	if (flags.world.test(World::hard_hit)) { play(svc, svc.sounds.get_buffer("hard_hit"), 0.1f, 60.f, 0, 1.f, {}, echo_count, echo_rate); }

	if (!svc.in_game()) {
		flags = {};
		proximities = {};
		return;
	} // exit early if not in-game

	if (flags.world.test(World::wall_hit)) { play(svc, svc.sounds.get_buffer("wall_hit"), 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::thud)) { play(svc, svc.sounds.get_buffer("thud"), 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::switch_press)) { play(svc, svc.sounds.get_buffer("switch_press"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::block_toggle)) { play(svc, svc.sounds.get_buffer("block_toggle"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_open)) { play(svc, svc.sounds.get_buffer("door_open"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_unlock)) { play(svc, svc.sounds.get_buffer("door_unlock")); }
	flags.world.test(World::pushable_move) ? simple_repeat(svc.sounds.get_buffer("pushable_move"), "pushable_move") : stop("pushable_move");

	// tank
	if (flags.tank.test(Tank::alert_1)) { play(svc, svc.sounds.get_buffer("tank_alert_1")); }
	if (flags.tank.test(Tank::alert_2)) { play(svc, svc.sounds.get_buffer("tank_alert_2")); }
	if (flags.tank.test(Tank::hurt_1)) { play(svc, svc.sounds.get_buffer("tank_hurt_1")); }
	if (flags.tank.test(Tank::hurt_2)) { play(svc, svc.sounds.get_buffer("tank_hurt_2"), 0.f, 50.f); }
	if (flags.tank.test(Tank::death)) { play(svc, svc.sounds.get_buffer("tank_death")); }

	// thug
	if (flags.thug.test(Thug::alert_1)) { play(svc, svc.sounds.get_buffer("tank_alert_1")); }
	if (flags.thug.test(Thug::alert_2)) { play(svc, svc.sounds.get_buffer("tank_alert_2")); }
	if (flags.thug.test(Thug::hurt_1)) { play(svc, svc.sounds.get_buffer("tank_hurt_1")); }
	if (flags.thug.test(Thug::hurt_2)) { play(svc, svc.sounds.get_buffer("tank_hurt_2"), 0.f, 50.f); }
	if (flags.thug.test(Thug::death)) { play(svc, svc.sounds.get_buffer("tank_death")); }

	// demon
	if (flags.demon.test(Demon::hurt)) { play(svc, svc.sounds.get_buffer("demon_hurt")); }
	if (flags.demon.test(Demon::death)) { play(svc, svc.sounds.get_buffer("demon_death")); }
	if (flags.demon.test(Demon::snort)) { play(svc, svc.sounds.get_buffer("demon_snort"), 0.2f); }
	if (flags.demon.test(Demon::up_snort)) { play(svc, svc.sounds.get_buffer("demon_up_snort"), 0.2f); }
	if (flags.demon.test(Demon::alert)) { play(svc, svc.sounds.get_buffer("demon_alert")); }

	// general enemy
	if (flags.enemy.test(Enemy::hit_low)) { play(svc, svc.sounds.get_buffer("hit_low")); }
	if (flags.enemy.test(Enemy::hit_medium)) { play(svc, svc.sounds.get_buffer("hit_medium")); }
	if (flags.enemy.test(Enemy::hit_high)) { play(svc, svc.sounds.get_buffer("hit_high")); }
	if (flags.enemy.test(Enemy::hit_squeak)) { play(svc, svc.sounds.get_buffer("hit_squeak")); }
	if (flags.enemy.test(Enemy::standard_death)) { play(svc, svc.sounds.get_buffer("standard_death")); }

	// item
	if (flags.item.test(Item::heal)) { play(svc, svc.sounds.get_buffer("heal"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_low)) { play(svc, svc.sounds.get_buffer("orb_get_1"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_medium)) { play(svc, svc.sounds.get_buffer("orb_get_2"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_high)) { play(svc, svc.sounds.get_buffer("orb_get_3"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_max)) { play(svc, svc.sounds.get_buffer("orb_get_4"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::health_increase)) { play(svc, svc.sounds.get_buffer("health_increase")); }
	if (flags.item.test(Item::gem)) { play(svc, svc.sounds.get_buffer("gem_get"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::get)) { play(svc, svc.sounds.get_buffer("item_get")); }
	if (flags.item.test(Item::equip)) { play(svc, svc.sounds.get_buffer("item_equip")); }
	if (flags.item.test(Item::vendor_sale)) { play(svc, svc.sounds.get_buffer("vendor_sale")); }

	// player
	if (flags.player.test(Player::jump)) { play(svc, svc.sounds.get_buffer("nani_jump"), 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::hurt)) { play(svc, svc.sounds.get_buffer("nani_hurt"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::death)) { play(svc, svc.sounds.get_buffer("nani_death")); }
	if (flags.player.test(Player::slide)) { play(svc, svc.sounds.get_buffer("nani_slide"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::walljump)) { play(svc, svc.sounds.get_buffer("nani_walljump"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::roll)) { play(svc, svc.sounds.get_buffer("nani_roll"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	// steps
	if (flags.step.test(Step::basic)) { play(svc, svc.sounds.get_buffer("nani_steps"), 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.step.test(Step::grass)) { play(svc, svc.sounds.get_buffer("nani_steps_grass"), 0.3f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::basic)) { play(svc, svc.sounds.get_buffer("nani_landed"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::grass)) { play(svc, svc.sounds.get_buffer("nani_landed_grass"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// arms
	if (flags.player.test(Player::arms_switch)) { play(svc, svc.sounds.get_buffer("arms_switch"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.arms.test(Arms::reload)) { play(svc, svc.sounds.get_buffer("arms_reload"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// gun
	if (flags.weapon.test(Weapon::bryns_gun)) { play(svc, svc.sounds.get_buffer("arms_shot_bg"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::gnat)) { play(svc, svc.sounds.get_buffer("arms_shot_gnat"), 0.1f, 100.f, 2, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::wasp)) { play(svc, svc.sounds.get_buffer("arms_shot_wasp"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::skycorps_ar)) { play(svc, svc.sounds.get_buffer("arms_shot_skycorps_ar"), 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// enemy gun
	if (flags.weapon.test(Weapon::energy_ball)) { play(svc, svc.sounds.get_buffer("arms_shot_energy_ball"), 0.1f); }

	// reset flags
	flags = {};

	// reset proximities
	proximities = {};
}

void Soundboard::play(automa::ServiceProvider& svc, sf::SoundBuffer const& buffer, float random_pitch_offset, float vol, int frequency, float attenuation, sf::Vector2<float> distance, int echo_count, int echo_rate) {
	sound_pool.push_back(Sound(buffer, "standard", echo_count, echo_rate));
	frequency != 0 ? repeat(svc, sound_pool.back(), frequency, random_pitch_offset, attenuation, distance) : randomize(svc, sound_pool.back(), random_pitch_offset, vol, attenuation, distance);
}

void audio::Soundboard::simple_repeat(sf::SoundBuffer const& buffer, std::string const& label) {
	bool already_playing{};
	for (auto& sd : sound_pool) {
		if (sd.get_label() == label) { already_playing = true; }
	}
	if (!already_playing) {
		sound_pool.push_back(Sound(buffer, label));
		sound_pool.back().play(true);
	}
}

void audio::Soundboard::stop(std::string const& label) {
	std::erase_if(sound_pool, [label](auto const& s) { return s.get_label() == label; });
}

void Soundboard::repeat(automa::ServiceProvider& svc, Sound& sound, int frequency, float random_pitch_offset, float attenuation, sf::Vector2<float> distance) {
	if (frequency == -1) {
		randomize(svc, sound, random_pitch_offset, 100.f, attenuation, distance, true);
	} else if (svc.ticker.every_x_ticks(frequency)) {
		randomize(svc, sound, random_pitch_offset, 100.f, attenuation, distance);
	}
}

void Soundboard::randomize(automa::ServiceProvider& svc, Sound& sound, float random_pitch_offset, float vol, float attenuation, sf::Vector2<float> distance, bool wait_until_over) {
	auto random_pitch = random_pitch_offset == 0.f ? 0.f : util::Random::random_range_float(-random_pitch_offset, random_pitch_offset);
	sound.set_pitch(1.f + random_pitch);
	sound.set_volume(vol);
	auto scalar = util::magnitude(distance) / attenuation;
	sound.set_volume(vol - (scalar > vol ? vol : scalar));
	if (wait_until_over && sound.is_playing()) { return; }
	sound.play();
}

void Soundboard::play_step(int tile_value, int style_id, bool land) {
	auto& set = land ? flags.land : flags.step;
	if (!get_step_sound.contains(style_id)) {
		set.set(audio::Step::basic);
		return;
	}
	if (!get_step_sound.at(style_id).contains(tile_value)) {
		set.set(audio::Step::basic);
		return;
	}
	set.set(get_step_sound.at(style_id).at(tile_value));
}

auto Soundboard::number_of_playng_sounds() -> int { return static_cast<int>(sound_pool.size()); }

} // namespace fornani::audio
