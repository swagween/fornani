#include "fornani/audio/Soundboard.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"

namespace audio {

Soundboard::Soundboard(automa::ServiceProvider& /*svc*/) {
	//for (int i{0}; i < 64; ++i) { sound_pool.push_back(Sound(svc.assets.click_buffer)); }
}

void Soundboard::play_sounds(automa::ServiceProvider& svc, int echo_count, int echo_rate) {

	std::erase_if(sound_pool, [](auto& s) { return !s.is_running(); });
	for (auto& s : sound_pool) { s.update(svc); }

	// menu
	if (flags.menu.test(Menu::forward_switch)) { play(svc, svc.assets.menu_next_buffer); }
	if (flags.menu.test(Menu::backward_switch)) { play(svc, svc.assets.menu_back_buffer); }
	if (flags.menu.test(Menu::select)) { play(svc, svc.assets.click_buffer); }
	if (flags.menu.test(Menu::shift)) { play(svc, svc.assets.menu_shift_buffer); }

	// console
	if (flags.console.test(Console::select)) { play(svc, svc.assets.click_buffer); }
	if (flags.console.test(Console::done)) { play(svc, svc.assets.menu_back_buffer); }
	if (flags.console.test(Console::next)) { play(svc, svc.assets.menu_next_buffer); }
	if (flags.console.test(Console::shift)) { play(svc, svc.assets.menu_shift_buffer); }
	if (flags.console.test(Console::menu_open)) { play(svc, svc.assets.menu_open_buffer); }
	if (flags.console.test(Console::speech)) { play(svc, svc.assets.menu_shift_buffer, 0.2f, 100.f, 16); }

	// transmission
	if (flags.transmission.test(Transmission::statics)) { play(svc, svc.assets.b_small_crash); }

	// always play console and menu sounds
	if (status == SoundboardState::off) {
		flags = {};
		proximities = {};
		return;
	}

	// world
	if (flags.world.test(World::load)) { play(svc, svc.assets.load_buffer, 0.f, 40.f); }
	if (flags.world.test(World::save)) { play(svc, svc.assets.save_buffer);
	}
	if (flags.world.test(World::soft_sparkle)) { play(svc, svc.assets.soft_sparkle_buffer);
	}
	if (flags.world.test(World::soft_sparkle_high)) { play(svc, svc.assets.soft_sparkle_high_buffer);
	}
	if (flags.world.test(World::chest)) { play(svc, svc.assets.chest_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_shatter)) { play(svc, svc.assets.shatter_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::breakable_hit)) { play(svc, svc.assets.b_breakable_hit, 0.1f); }
	cooldowns.hard_hit.update();
	if (flags.world.test(World::hard_hit) && !cooldowns.hard_hit.running()) {
		play(svc, svc.assets.b_enemy_hit_inv, 0.1f, 60.f, 0, 1.f, {}, echo_count, echo_rate);
		cooldowns.hard_hit.start();
	}

	if (!svc.in_game()) {
		flags = {};
		proximities = {};
		return;
	} // exit early if not in-game

	if (flags.world.test(World::wall_hit)) { play(svc, svc.assets.b_wall_hit, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::soft_tap)) { play(svc, svc.assets.b_soft_tap, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::thud)) { play(svc, svc.assets.b_thud, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::small_crash)) { play(svc, svc.assets.b_small_crash, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::switch_press)) { play(svc, svc.assets.b_switch_press, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate);
	}
	if (flags.world.test(World::block_toggle)) { play(svc, svc.assets.b_block_toggle, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_open)) { play(svc, svc.assets.b_door_open, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.world.test(World::door_unlock)) { play(svc, svc.assets.b_door_unlock); }
	if (flags.world.test(World::pushable)) { play(svc, svc.assets.b_heavy_move, 0.f, 100.f, 80); }

	// frdog
	if (flags.frdog.test(Frdog::death)) { play(svc, svc.assets.enem_death_1_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	// tank
	if (flags.tank.test(Tank::alert_1)) { play(svc, svc.assets.tank_alert1_buffer); }
	if (flags.tank.test(Tank::alert_2)) { play(svc, svc.assets.tank_alert2_buffer); }
	if (flags.tank.test(Tank::hurt_1)) { play(svc, svc.assets.tank_hurt1_buffer); }
	if (flags.tank.test(Tank::hurt_2)) { play(svc, svc.assets.tank_hurt2_buffer, 0.f, 50.f);
	}
	if (flags.tank.test(Tank::death)) { play(svc, svc.assets.tank_death_buffer); }
	// thug
	if (flags.thug.test(Thug::alert_1)) { play(svc, svc.assets.tank_alert1_buffer); }
	if (flags.thug.test(Thug::alert_2)) { play(svc, svc.assets.tank_alert2_buffer); }
	if (flags.thug.test(Thug::hurt_1)) { play(svc, svc.assets.tank_hurt1_buffer); }
	if (flags.thug.test(Thug::hurt_2)) { play(svc, svc.assets.tank_hurt2_buffer, 0.f, 50.f);
	}
	if (flags.thug.test(Thug::death)) { play(svc, svc.assets.tank_death_buffer); }

	// demon
	if (flags.demon.test(Demon::hurt)) { play(svc, svc.assets.enem_hit_buffer); }
	if (flags.demon.test(Demon::death)) { play(svc, svc.assets.enem_death_1_buffer); }
	if (flags.demon.test(Demon::snort)) { play(svc, svc.assets.b_demon_snort, 0.2f); }

	// general enemy
	if (flags.enemy.test(Enemy::hit_low)) { play(svc, svc.assets.b_enemy_hit_low); }
	if (flags.enemy.test(Enemy::hit_medium)) { play(svc, svc.assets.b_enemy_hit_medium); }
	if (flags.enemy.test(Enemy::hit_high)) { play(svc, svc.assets.b_enemy_hit_high); }
	if (flags.enemy.test(Enemy::hit_squeak)) { play(svc, svc.assets.b_enemy_hit_squeak); }

	// item
	if (flags.item.test(Item::heal)) { play(svc, svc.assets.heal_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_low)) { play(svc, svc.assets.orb_1_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_medium)) { play(svc, svc.assets.orb_2_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_high)) { play(svc, svc.assets.orb_3_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::orb_max)) { play(svc, svc.assets.orb_4_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::health_increase)) { play(svc, svc.assets.b_health_increase); }
	if (flags.item.test(Item::gem)) { play(svc, svc.assets.b_upward_get, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.item.test(Item::get)) { play(svc, svc.assets.b_upward_get); }
	if (flags.item.test(Item::equip)) { play(svc, svc.assets.arms_switch_buffer); }

	// player
	if (flags.player.test(Player::jump)) { play(svc, svc.assets.jump_buffer, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::arms_switch)) { play(svc, svc.assets.arms_switch_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::hurt)) { play(svc, svc.assets.hurt_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::death)) { play(svc, svc.assets.player_death_buffer); }
	if (flags.player.test(Player::shield_drop)) { play(svc, svc.assets.bubble_buffer, 0.2f, 60.f); }
	if (flags.player.test(Player::slide)) { play(svc, svc.assets.slide_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::walljump)) { play(svc, svc.assets.b_walljump, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.player.test(Player::roll)) { play(svc, svc.assets.b_roll, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// steps
	if (flags.step.test(Step::basic)) { play(svc, svc.assets.step_buffer, 0.1f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.step.test(Step::grass)) { play(svc, svc.assets.grass_step_buffer, 0.3f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::basic)) { play(svc, svc.assets.landed_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.land.test(Step::grass)) { play(svc, svc.assets.landed_grass_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// arms
	if (flags.arms.test(Arms::reload)) { play(svc, svc.assets.b_reload, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// gun
	if (flags.weapon.test(Weapon::bryns_gun)) { play(svc, svc.assets.bg_shot_buffer, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::plasmer)) { play(svc, svc.assets.plasmer_shot_buffer); }
	if (flags.weapon.test(Weapon::skycorps_ar)) { play(svc, svc.assets.skycorps_ar_buffer); }
	if (flags.weapon.test(Weapon::clover)) { play(svc, svc.assets.pop_mid_buffer, 0.3f, 100.f, 0); }
	if (flags.weapon.test(Weapon::nova)) { play(svc, svc.assets.pop_mid_buffer); }
	if (flags.weapon.test(Weapon::indie)) { play(svc, svc.assets.b_nova); }
	if (flags.weapon.test(Weapon::staple)) { play(svc, svc.assets.b_staple); }
	if (flags.weapon.test(Weapon::gnat)) { play(svc, svc.assets.b_gnat, 0.1f, 100.f, 2, 1.f, {}, echo_count, echo_rate); }
	if (flags.weapon.test(Weapon::tomahawk)) { play(svc, svc.assets.tomahawk_flight_buffer, 0.05f, 100.f, 32); }
	if (flags.weapon.test(Weapon::tomahawk_catch)) { play(svc, svc.assets.tomahawk_catch_buffer); }
	if (flags.weapon.test(Weapon::hook_probe)) { play(svc, svc.assets.sharp_click_buffer); }
	if (flags.weapon.test(Weapon::energy_ball)) { play(svc, svc.assets.b_energy_shot, 0.1f); }
	if (flags.weapon.test(Weapon::wasp)) { play(svc, svc.assets.b_wasp, 0.f, 100.f, 0, 1.f, {}, echo_count, echo_rate); }

	// reset flags
	flags = {};

	// reset proximities
	proximities = {};
}

void Soundboard::play(automa::ServiceProvider& svc, sf::SoundBuffer& buffer, float random_pitch_offset, float vol, int frequency, float attenuation, sf::Vector2<float> distance, int echo_count, int echo_rate) {
	sound_pool.push_back(Sound(buffer, echo_count, echo_rate));
	frequency != 0 ? repeat(svc, sound_pool.back(), frequency, random_pitch_offset, attenuation, distance) : randomize(svc, sound_pool.back(), random_pitch_offset, vol, attenuation, distance);
}

void Soundboard::repeat(automa::ServiceProvider& svc, Sound& sound, int frequency, float random_pitch_offset, float attenuation, sf::Vector2<float> distance) {
	if (svc.ticker.every_x_ticks(frequency)) { randomize(svc, sound, random_pitch_offset, 100.f, attenuation, distance); }
}

void Soundboard::randomize(automa::ServiceProvider& svc, Sound& sound, float random_pitch_offset, float vol, float attenuation, sf::Vector2<float> distance) {
	auto random_pitch = random_pitch_offset == 0.f ? 0.f : svc.random.random_range_float(-random_pitch_offset, random_pitch_offset);
	sound.set_pitch(1.f + random_pitch);
	sound.set_volume(vol);
	auto scalar = util::magnitude(distance) / attenuation;
	sound.set_volume(vol - (scalar > vol ? vol : scalar));
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

} // namespace audio
