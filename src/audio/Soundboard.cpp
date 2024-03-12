
#pragma once

#include "Soundboard.hpp"
#include <algorithm>
#include "../setup/ServiceLocator.hpp"
#include "../service/ServiceProvider.hpp"

namespace audio {

void Soundboard::play_sounds(automa::ServiceProvider& svc) {

	// menu
	if (menu.test(Menu::forward_switch)) { svc.assets.menu_next.play(); }
	if (menu.test(Menu::backward_switch)) { svc.assets.menu_back.play(); }
	if (menu.test(Menu::select)) { svc.assets.click.play(); }
	if (menu.test(Menu::shift)) { svc.assets.menu_shift.play(); }

	// console
	if (console.test(Console::select)) { svc.assets.click.play(); }
	if (console.test(Console::done)) { svc.assets.menu_back.play(); }
	if (console.test(Console::next)) { svc.assets.menu_next.play(); }
	if (console.test(Console::speech)) { repeat(svc.assets.menu_shift, 5, 0.2f); }

	// world
	if (world.test(World::load)) { svc.assets.load.play(); }
	if (world.test(World::save)) { svc.assets.save.play(); }
	if (world.test(World::soft_sparkle)) { svc.assets.soft_sparkle.play(); }

	// player
	if (player.test(Player::land)) { svc.assets.landed.play(); }
	if (player.test(Player::jump)) { svc.assets.jump.play(); }
	if (player.test(Player::step)) { randomize(svc.assets.step, 0.1f); }
	if (player.test(Player::arms_switch)) { svc.assets.arms_switch.play(); }
	if (player.test(Player::hurt)) { svc.assets.hurt.play(); }

	// gun
	if (weapon.test(Weapon::bryns_gun)) { svc.assets.bg_shot.play(); }
	if (weapon.test(Weapon::plasmer)) { svc.assets.plasmer_shot.play(); }
	if (weapon.test(Weapon::clover)) { repeat(svc.assets.pop_mid, 2, 0.3f);
	}
	if (weapon.test(Weapon::nova)) { svc.assets.pop_mid.play(); }
	if (weapon.test(Weapon::tomahawk)) { repeat(svc.assets.tomahawk_flight, 30, 0.4f);
	}
	if (weapon.test(Weapon::tomahawk_catch)) { svc.assets.tomahawk_catch.play(); }
	if (weapon.test(Weapon::hook_probe)) { svc.assets.sharp_click.play(); }

	// reset flags
	menu = {};
	console = {};
	world = {};
	player = {};
	weapon = {};

	// reset proximities
	proximities = {};
}

void Soundboard::repeat(sf::Sound& sound, int frequency, float random_pitch_offset) {
	if (svc::tickerLocator.get().every_x_frames(frequency)) { randomize(sound, random_pitch_offset); }
}

void Soundboard::randomize(sf::Sound& sound, float random_pitch_offset) {
	float random_pitch = svc::randomLocator.get().random_range_float(-random_pitch_offset, random_pitch_offset);
	sound.setPitch(1.f + random_pitch);
	sound.play();
}

} // namespace audio
