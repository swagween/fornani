
#pragma once

#include "Soundboard.hpp"
#include <algorithm>
#include "../setup/ServiceLocator.hpp"

namespace audio {

void Soundboard::play_sounds() {

	// menu
	if (menu.test(Menu::forward_switch)) { svc::assetLocator.get().menu_next.play(); }
	if (menu.test(Menu::backward_switch)) { svc::assetLocator.get().menu_back.play(); }
	if (menu.test(Menu::select)) { svc::assetLocator.get().click.play(); }
	if (menu.test(Menu::shift)) { svc::assetLocator.get().menu_shift.play(); }

	// console
	if (console.test(Console::select)) { svc::assetLocator.get().click.play(); }
	if (console.test(Console::done)) { svc::assetLocator.get().menu_back.play(); }
	if (console.test(Console::next)) { svc::assetLocator.get().menu_next.play(); }
	if (console.test(Console::speech)) { repeat(svc::assetLocator.get().menu_shift, 5, 0.2f); }

	// world
	if (world.test(World::load)) { svc::assetLocator.get().load.play(); }
	if (world.test(World::save)) { svc::assetLocator.get().save.play(); }
	if (world.test(World::soft_sparkle)) { svc::assetLocator.get().soft_sparkle.play(); }

	// player
	if (player.test(Player::land)) { svc::assetLocator.get().landed.play(); }
	if (player.test(Player::jump)) { svc::assetLocator.get().jump.play(); }
	if (player.test(Player::step)) { randomize(svc::assetLocator.get().step, 0.1f); }
	if (player.test(Player::arms_switch)) { svc::assetLocator.get().arms_switch.play(); }
	if (player.test(Player::hurt)) { svc::assetLocator.get().hurt.play(); }

	// gun
	if (weapon.test(Weapon::bryns_gun)) { svc::assetLocator.get().bg_shot.play(); }
	if (weapon.test(Weapon::plasmer)) { svc::assetLocator.get().plasmer_shot.play(); }
	if (weapon.test(Weapon::clover)) { repeat(svc::assetLocator.get().pop_mid, 2, 0.3f);
	}
	if (weapon.test(Weapon::nova)) { svc::assetLocator.get().pop_mid.play(); }
	if (weapon.test(Weapon::tomahawk)) { repeat(svc::assetLocator.get().tomahawk_flight, 30, 0.4f);
	}
	if (weapon.test(Weapon::tomahawk_catch)) { svc::assetLocator.get().tomahawk_catch.play(); }
	if (weapon.test(Weapon::hook_probe)) { svc::assetLocator.get().sharp_click.play(); }

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
