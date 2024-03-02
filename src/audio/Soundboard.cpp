
#pragma once

#include "Soundboard.hpp"
#include "../setup/ServiceLocator.hpp"

namespace audio {

void Soundboard::play_sounds() {

	//menu
	if (menu.test(Menu::forward_switch)) { svc::assetLocator.get().menu_next.play(); }
	if (menu.test(Menu::backward_switch)) { svc::assetLocator.get().menu_back.play(); }
	if (menu.test(Menu::select)) { svc::assetLocator.get().click.play(); }
	if (menu.test(Menu::shift)) { svc::assetLocator.get().menu_shift.play(); }

	//player
	if (player.test(Player::land)) { svc::assetLocator.get().landed.play(); }
	if (player.test(Player::jump)) { svc::assetLocator.get().jump.play(); }
	if (player.test(Player::step)) {
		float random_pitch = svc::randomLocator.get().random_range_float(0.f, 0.1f);
		svc::assetLocator.get().step.setPitch(1.0f + random_pitch);

		svc::assetLocator.get().step.setVolume(60);
		svc::assetLocator.get().step.play();
	}
	if (player.test(Player::arms_switch)) { svc::assetLocator.get().arms_switch.play(); }
	if (player.test(Player::hurt)) { svc::assetLocator.get().hurt.play(); }

	//gun
	if (weapon.test(Weapon::bryns_gun)) { svc::assetLocator.get().bg_shot.play(); }
	if (weapon.test(Weapon::plasmer)) { svc::assetLocator.get().plasmer_shot.play(); }
	if (weapon.test(Weapon::clover)) {
		float random_pitch = svc::randomLocator.get().random_range_float(-0.3f, 0.3f);
		svc::assetLocator.get().pop_mid.setPitch(1 + random_pitch);
		svc::assetLocator.get().pop_mid.play(); }
	if (weapon.test(Weapon::nova)) { svc::assetLocator.get().pop_mid.play(); }


	//reset flags
	menu = {};
	player = {};
	weapon = {};

}

} // namespace audio
