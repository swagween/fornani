#include "Soundboard.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"

namespace audio {

void Soundboard::play_sounds(automa::ServiceProvider& svc) {

	// menu
	if (flags.menu.test(Menu::forward_switch)) { svc.assets.menu_next.play(); }
	if (flags.menu.test(Menu::backward_switch)) { svc.assets.menu_back.play(); }
	if (flags.menu.test(Menu::select)) { svc.assets.click.play(); }
	if (flags.menu.test(Menu::shift)) { svc.assets.menu_shift.play(); }

	// console
	if (flags.console.test(Console::select)) { svc.assets.click.play(); }
	if (flags.console.test(Console::done)) { svc.assets.menu_back.play(); }
	if (flags.console.test(Console::next)) { svc.assets.menu_next.play(); }
	if (flags.console.test(Console::shift)) { svc.assets.menu_shift.play(); }
	if (flags.console.test(Console::menu_open)) { svc.assets.menu_open.play(); }
	if (flags.console.test(Console::speech)) { repeat(svc, svc.assets.menu_shift, 16, 0.2f); }

	// always play console and menu sounds
	if (status == SoundboardState::off) {
		flags = {};
		proximities = {};
		return;
	}

	// world
	if (flags.world.test(World::load)) { svc.assets.load.play(); }
	if (flags.world.test(World::save)) { svc.assets.save.play(); }
	if (flags.world.test(World::soft_sparkle)) { svc.assets.soft_sparkle.play(); }
	if (flags.world.test(World::chest)) { svc.assets.chest.play(); }
	if (flags.world.test(World::breakable_shatter)) { svc.assets.breakable_shatter.play(); }
	if (flags.world.test(World::breakable_hit)) { randomize(svc, svc.assets.breakable_hit, 0.1f); }
	if (flags.world.test(World::hard_hit)) { randomize(svc, svc.assets.hard_hit, 0.1f); }
	if (flags.world.test(World::thud)) { randomize(svc, svc.assets.thud, 0.1f); }
	if (flags.world.test(World::small_crash)) { randomize(svc, svc.assets.small_crash, 0.1f); }

	//frdog
	if (flags.frdog.test(Frdog::death)) { svc.assets.enem_death_1.play(); }
	//tank
	if (flags.tank.test(Tank::alert_1)) { svc.assets.tank_alert_1.play(); }
	if (flags.tank.test(Tank::alert_2)) { svc.assets.tank_alert_2.play(); }
	if (flags.tank.test(Tank::hurt_1)) { svc.assets.tank_hurt_1.play(); }
	if (flags.tank.test(Tank::hurt_2)) { play_at_volume(svc.assets.tank_hurt_2, 50); }
	if (flags.tank.test(Tank::death)) { svc.assets.tank_death.play(); }
	// thug
	if (flags.thug.test(Thug::alert_1)) { svc.assets.tank_alert_1.play(); }
	if (flags.thug.test(Thug::alert_2)) { svc.assets.tank_alert_2.play(); }
	if (flags.thug.test(Thug::hurt_1)) { svc.assets.tank_hurt_1.play(); }
	if (flags.thug.test(Thug::hurt_2)) { play_at_volume(svc.assets.tank_hurt_2, 50); }
	if (flags.thug.test(Thug::death)) { svc.assets.tank_death.play(); }

	//minigus

	//item
	if (flags.item.test(Item::heal)) { svc.assets.heal.play(); }
	if (flags.item.test(Item::orb_low)) { svc.assets.orb_1.play(); }
	if (flags.item.test(Item::orb_medium)) { svc.assets.orb_2.play(); }
	if (flags.item.test(Item::orb_high)) { svc.assets.orb_3.play(); }
	if (flags.item.test(Item::orb_max)) { svc.assets.orb_4.play(); }

	// player
	if (flags.player.test(Player::land)) { svc.assets.landed.play(); }
	if (flags.player.test(Player::jump)) { randomize(svc, svc.assets.jump, 0.1f); }
	if (flags.player.test(Player::step)) { randomize(svc, svc.assets.step, 0.1f); }
	if (flags.player.test(Player::arms_switch)) { svc.assets.arms_switch.play(); }
	if (flags.player.test(Player::hurt)) { svc.assets.hurt.play(); }
	if (flags.player.test(Player::death)) { svc.assets.player_death.play(); }
	if (flags.player.test(Player::shield_drop)) { randomize(svc, svc.assets.bubble, 0.2f, 60); }

	// gun
	if (flags.weapon.test(Weapon::bryns_gun)) { svc.assets.bg_shot.play(); }
	if (flags.weapon.test(Weapon::plasmer)) { svc.assets.plasmer_shot.play(); }
	if (flags.weapon.test(Weapon::skycorps_ar)) { svc.assets.skycorps_ar_shot.play(); }
	if (flags.weapon.test(Weapon::clover)) { repeat(svc, svc.assets.pop_mid, 2, 0.3f); }
	if (flags.weapon.test(Weapon::nova)) { svc.assets.pop_mid.play(); }
	if (flags.weapon.test(Weapon::indie)) { svc.assets.nova_shot.play(); }
	if (flags.weapon.test(Weapon::staple)) { svc.assets.staple.play(); }
	if (flags.weapon.test(Weapon::gnat)) { repeat(svc, svc.assets.gnat, 2, 0.1f); }
	if (flags.weapon.test(Weapon::tomahawk)) { repeat(svc, svc.assets.tomahawk_flight, -1, 0.1f); }
	if (flags.weapon.test(Weapon::tomahawk_catch)) {
		svc.assets.tomahawk_catch.play();
		svc.assets.tomahawk_flight.stop();
	}
	if (flags.weapon.test(Weapon::hook_probe)) { svc.assets.sharp_click.play(); }

	// reset flags
	flags = {};

	// reset proximities
	proximities = {};
}

void Soundboard::repeat(automa::ServiceProvider& svc, sf::Sound& sound, int frequency, float random_pitch_offset) {
	if (frequency == -1) {
		if (sound.getStatus() != sf::SoundSource::Playing) { randomize(svc, sound, random_pitch_offset); }
		return;
	}
	if (svc.ticker.every_x_ticks(frequency)) { randomize(svc, sound, random_pitch_offset); }
}

void Soundboard::randomize(automa::ServiceProvider& svc, sf::Sound& sound, float random_pitch_offset, float vol) {
	float random_pitch = svc.random.random_range_float(-random_pitch_offset, random_pitch_offset);
	sound.setPitch(1.f + random_pitch);
	sound.setVolume(vol);
	sound.play();
}

void Soundboard::play_at_volume(sf::Sound& sound, float vol) {
	sound.setVolume(vol);
	sound.play();
}

} // namespace audio
