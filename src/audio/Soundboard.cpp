
#pragma once

#include "Soundboard.hpp"
#include <algorithm>

namespace audio {

void Soundboard::play_sounds() {

	// menu
	if (menu.test(Menu::forward_switch)) { menu_next.play(); }
	if (menu.test(Menu::backward_switch)) { menu_back.play(); }
	if (menu.test(Menu::select)) { click.play(); }
	if (menu.test(Menu::shift)) { menu_shift.play(); }

	// console
	if (console.test(Console::select)) { sharp_click.play(); }
	if (console.test(Console::done)) { click.play(); }

	// world
	if (world.test(World::load)) { load.play(); }
	if (world.test(World::save)) { save.play(); }
	if (world.test(World::soft_sparkle)) { soft_sparkle.play(); }

	// player
	if (player.test(Player::land)) { landed.play(); }
	if (player.test(Player::jump)) { jump.play(); }
	if (player.test(Player::step)) {
		// float random_pitch = svc.randomLocator.get().random_range_float(0.f, 0.1f);
		// step.setPitch(1.0f + random_pitch);

		step.setVolume(60);
		step.play();
	}
	if (player.test(Player::arms_switch)) { arms_switch.play(); }
	if (player.test(Player::hurt)) { hurt.play(); }

	// gun
	if (weapon.test(Weapon::bryns_gun)) { bg_shot.play(); }
	if (weapon.test(Weapon::plasmer)) { plasmer_shot.play(); }
	if (weapon.test(Weapon::clover)) {
		// float random_pitch = svc.randomLocator.get().random_range_float(-0.3f, 0.3f);
		// pop_mid.setPitch(1 + random_pitch);
		pop_mid.play();
	}
	if (weapon.test(Weapon::nova)) { pop_mid.play(); }
	if (weapon.test(Weapon::tomahawk)) {
		// float random_pitch = svc.randomLocator.get().random_range_float(0.f, 0.4f);
		// tomahawk_flight.setPitch(1.0f + random_pitch);
		// if (svc.tickerLocator.get().every_x_frames(40)) { tomahawk_flight.play(); }
	}
	if (weapon.test(Weapon::tomahawk_catch)) { tomahawk_catch.play(); }

	// reset flags
	menu = {};
	console = {};
	world = {};
	player = {};
	weapon = {};

	// reset proximities
	proximities = {};
}

void Soundboard::load_audio() {
	click_buffer.loadFromFile(finder.resource_path + "/audio/sfx/heavy_click.wav");
	click.setBuffer(click_buffer);
	sharp_click_buffer.loadFromFile(finder.resource_path + "/audio/sfx/click.wav");
	sharp_click.setBuffer(sharp_click_buffer);
	menu_shift_buffer.loadFromFile(finder.resource_path + "/audio/sfx/menu_shift_1.wav");
	menu_shift.setBuffer(menu_shift_buffer);
	menu_back_buffer.loadFromFile(finder.resource_path + "/audio/sfx/menu_shift_2.wav");
	menu_back.setBuffer(menu_back_buffer);
	menu_next_buffer.loadFromFile(finder.resource_path + "/audio/sfx/menu_shift_3.wav");
	menu_next.setBuffer(menu_next_buffer);
	sharp_click_buffer.loadFromFile(finder.resource_path + "/audio/sfx/click.wav");
	sharp_click.setBuffer(sharp_click_buffer);
	arms_switch_buffer.loadFromFile(finder.resource_path + "/audio/sfx/arms_switch.wav");
	arms_switch.setBuffer(arms_switch_buffer);
	bg_shot_buffer.loadFromFile(finder.resource_path + "/audio/sfx/bg_shot.wav");
	bg_shot.setBuffer(bg_shot_buffer);
	plasmer_shot_buffer.loadFromFile(finder.resource_path + "/audio/sfx/plasmer_shot.wav");
	plasmer_shot.setBuffer(plasmer_shot_buffer);
	tomahawk_flight_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tomahawk_flight.wav");
	tomahawk_flight.setBuffer(tomahawk_flight_buffer);
	tomahawk_catch_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tomahawk_catch.wav");
	tomahawk_catch.setBuffer(tomahawk_catch_buffer);
	pop_mid_buffer.loadFromFile(finder.resource_path + "/audio/sfx/clover.wav");
	pop_mid.setBuffer(pop_mid_buffer);
	jump_buffer.loadFromFile(finder.resource_path + "/audio/sfx/jump.wav");
	jump.setBuffer(jump_buffer);
	shatter_buffer.loadFromFile(finder.resource_path + "/audio/sfx/shatter.wav");
	shatter.setBuffer(shatter_buffer);
	step_buffer.loadFromFile(finder.resource_path + "/audio/sfx/steps.wav");
	step.setBuffer(step_buffer);
	landed_buffer.loadFromFile(finder.resource_path + "/audio/sfx/landed.wav");
	landed.setBuffer(landed_buffer);
	hurt_buffer.loadFromFile(finder.resource_path + "/audio/sfx/hurt.wav");
	hurt.setBuffer(hurt_buffer);
	player_death_buffer.loadFromFile(finder.resource_path + "/audio/sfx/player_death.wav");
	player_death.setBuffer(player_death_buffer);
	enem_hit_buffer.loadFromFile(finder.resource_path + "/audio/sfx/enemy_hit.wav");
	enem_hit.setBuffer(enem_hit_buffer);
	save_buffer.loadFromFile(finder.resource_path + "/audio/sfx/save_point.wav");
	save.setBuffer(save_buffer);
	load_buffer.loadFromFile(finder.resource_path + "/audio/sfx/load_game.wav");
	load.setBuffer(load_buffer);
	soft_sparkle_high_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle_high.wav");
	soft_sparkle_high.setBuffer(soft_sparkle_high_buffer);
	soft_sparkle_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle.wav");
	soft_sparkle.setBuffer(soft_sparkle_buffer);
}

} // namespace audio
