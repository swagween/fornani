#pragma once
#include "AssetManager.hpp"

namespace asset {

void AssetManager::import_textures() {
	t_nani.loadFromFile(finder.resource_path + "/image/character/nani.png");
	t_portraits.loadFromFile(finder.resource_path + "/image/character/portraits.png");
	t_bryn.loadFromFile(finder.resource_path + "/image/character/bryn.png");
	t_gobe.loadFromFile(finder.resource_path + "/image/character/gobe.png");
	t_dr_go.loadFromFile(finder.resource_path + "/image/character/dr_go.png");
	t_carl.loadFromFile(finder.resource_path + "/image/character/carl.png");
	t_mirin.loadFromFile(finder.resource_path + "/image/character/mirin.png");
	t_bit.loadFromFile(finder.resource_path + "/image/character/bit.png");
	npcs.insert({"bryn", t_bryn});
	npcs.insert({"gobe", t_gobe});
	npcs.insert({"dr_go", t_dr_go});
	npcs.insert({"carl", t_carl});
	npcs.insert({"mirin", t_mirin});
	npcs.insert({"bit", t_bit});

	t_palette_nani.loadFromFile(finder.resource_path + "/image/character/nani_palette_default.png");
	t_palette_nanidiv.loadFromFile(finder.resource_path + "/image/character/nani_palette_divine.png");
	t_palette_naninight.loadFromFile(finder.resource_path + "/image/character/nani_palette_night.png");

	t_frdog.loadFromFile(finder.resource_path + "/image/critter/frdog.png");
	t_hulmet.loadFromFile(finder.resource_path + "/image/critter/hulmet.png");
	t_tank.loadFromFile(finder.resource_path + "/image/critter/tank.png");
	t_thug.loadFromFile(finder.resource_path + "/image/critter/thug.png");
	t_eyebot.loadFromFile(finder.resource_path + "/image/critter/eyebot.png");
	t_eyebit.loadFromFile(finder.resource_path + "/image/critter/eyebit.png");
	t_minigus.loadFromFile(finder.resource_path + "/image/boss/minigus.png");
	t_minigus_inv.loadFromFile(finder.resource_path + "/image/boss/minigus_inv.png");
	t_minigus_red.loadFromFile(finder.resource_path + "/image/boss/minigus_red.png");
	t_minigus_blue.loadFromFile(finder.resource_path + "/image/boss/minigus_blue.png");
	texture_lookup.insert({"frdog", t_frdog});
	texture_lookup.insert({"hulmet", t_hulmet});
	texture_lookup.insert({"tank", t_tank});
	texture_lookup.insert({"thug", t_thug});
	texture_lookup.insert({"eyebot", t_eyebot});
	texture_lookup.insert({"eyebit", t_eyebit});
	texture_lookup.insert({"minigus", t_minigus});

	t_ui.loadFromFile(finder.resource_path + "/image/gui/simple_console.png");
	t_hud_orb_font.loadFromFile(finder.resource_path + "/image/gui/HUD_orb_font.png");
	t_hud_hearts.loadFromFile(finder.resource_path + "/image/gui/HUD_hearts.png");
	t_hud_pointer.loadFromFile(finder.resource_path + "/image/gui/HUD_pointer.png");
	t_hud_gun_color.loadFromFile(finder.resource_path + "/image/gui/HUD_gun_color.png");
	t_hud_gun_shadow.loadFromFile(finder.resource_path + "/image/gui/HUD_gun_shadow.png");
	t_hud_shield.loadFromFile(finder.resource_path + "/image/gui/HUD_shield.png");
	t_selector.loadFromFile(finder.resource_path + "/image/gui/selector.png");
	t_console_outline.loadFromFile(finder.resource_path + "/image/gui/console_outline.png");

	// guns and bullets!
	t_bryns_gun.loadFromFile(finder.resource_path + "/image/weapon/bg.png");
	t_plasmer.loadFromFile(finder.resource_path + "/image/weapon/plasmer.png");
	t_clover.loadFromFile(finder.resource_path + "/image/weapon/clover.png");
	t_nova.loadFromFile(finder.resource_path + "/image/weapon/nova.png");
	t_tomahawk.loadFromFile(finder.resource_path + "/image/weapon/tomahawk.png");
	t_bryns_gun_projectile.loadFromFile(finder.resource_path + "/image/weapon/bg_proj.png");
	t_plasmer_projectile.loadFromFile(finder.resource_path + "/image/weapon/plasmer_proj.png");
	t_clover_projectile.loadFromFile(finder.resource_path + "/image/weapon/clover_proj.png");
	t_nova_projectile.loadFromFile(finder.resource_path + "/image/weapon/nova_proj.png");
	t_skycorps_ar_projectile.loadFromFile(finder.resource_path + "/image/weapon/skycorps_ar_proj.png");
	t_tomahawk_projectile.loadFromFile(finder.resource_path + "/image/weapon/tomahawk_projectile.png");
	t_grappling_hook.loadFromFile(finder.resource_path + "/image/weapon/grappling_hook.png");
	t_rope.loadFromFile(finder.resource_path + "/image/weapon/rope.png");
	t_hook.loadFromFile(finder.resource_path + "/image/weapon/hook.png");
	t_grenade_launcher.loadFromFile(finder.resource_path + "/image/weapon/grenade_launcher.png");
	t_grenade.loadFromFile(finder.resource_path + "/image/weapon/grenade.png");
	t_minigun.loadFromFile(finder.resource_path + "/image/boss/minigun.png");
	t_soda_gun.loadFromFile(finder.resource_path + "/image/weapon/soda_gun.png");
	t_soda_gun_projectile.loadFromFile(finder.resource_path + "/image/weapon/soda.png");
	t_staple_gun.loadFromFile(finder.resource_path + "/image/weapon/staple_gun.png");
	t_staple.loadFromFile(finder.resource_path + "/image/weapon/staple.png");
	t_indie.loadFromFile(finder.resource_path + "/image/weapon/indie.png");
	t_indie_projectile.loadFromFile(finder.resource_path + "/image/weapon/indie_proj.png");
	t_gnat.loadFromFile(finder.resource_path + "/image/weapon/gnat.png");
	t_gnat_projectile.loadFromFile(finder.resource_path + "/image/weapon/gnat_proj.png");

	weapon_textures.insert({"bryn's gun", t_bryns_gun});
	projectile_textures.insert({"bryn's gun", t_bryns_gun_projectile});
	weapon_textures.insert({"plasmer", t_plasmer});
	weapon_textures.insert({"skycorps ar", t_plasmer});
	projectile_textures.insert({"skycorps ar", t_skycorps_ar_projectile});
	projectile_textures.insert({"plasmer", t_plasmer_projectile});
	weapon_textures.insert({"nova", t_nova});
	projectile_textures.insert({"nova", t_nova_projectile});
	weapon_textures.insert({"tomahawk", t_tomahawk});
	projectile_textures.insert({"tomahawk", t_tomahawk_projectile});
	weapon_textures.insert({"clover", t_clover});
	projectile_textures.insert({"clover", t_clover_projectile});
	weapon_textures.insert({"grappling hook", t_grappling_hook});
	projectile_textures.insert({"grappling hook", t_hook});
	weapon_textures.insert({"grenade launcher", t_grenade_launcher});
	projectile_textures.insert({"grenade launcher", t_hook});
	weapon_textures.insert({"minigun", t_minigun});
	projectile_textures.insert({"minigun", t_skycorps_ar_projectile});
	weapon_textures.insert({"soda gun", t_soda_gun});
	projectile_textures.insert({"soda gun", t_soda_gun_projectile});
	weapon_textures.insert({"staple gun", t_staple_gun});
	projectile_textures.insert({"staple gun", t_staple});
	weapon_textures.insert({"indie", t_indie});
	projectile_textures.insert({"indie", t_indie_projectile});
	weapon_textures.insert({"gnat", t_gnat});
	projectile_textures.insert({"gnat", t_gnat_projectile});

	t_items.loadFromFile(finder.resource_path + "/image/item/items.png");
	t_guns.loadFromFile(finder.resource_path + "/image/item/guns.png");
	t_shield.loadFromFile(finder.resource_path + "/image/entity/shield.png");

	t_heart.loadFromFile(finder.resource_path + "/image/item/hearts.png");
	t_orb.loadFromFile(finder.resource_path + "/image/item/orbs.png");

	t_fader.loadFromFile(finder.resource_path + "/image/vfx/fader.png");

	t_twinkle.loadFromFile(finder.resource_path + "/image/vfx/twinkle.png");
	t_small_flash.loadFromFile(finder.resource_path + "/image/vfx/small_flash.png");
	particle_textures.insert({"twinkle", t_twinkle});

	t_alphabet.loadFromFile(finder.resource_path + "/image/gui/alphabet.png");

	t_platforms.loadFromFile(finder.resource_path + "/image/tile/platforms.png");
	t_platform_night.loadFromFile(finder.resource_path + "/image/tile/platform_night.png");
	t_platform_abandoned.loadFromFile(finder.resource_path + "/image/tile/platform_abandoned.png");
	t_platform_firstwind.loadFromFile(finder.resource_path + "/image/tile/platform_firstwind.png");
	t_platform_overturned.loadFromFile(finder.resource_path + "/image/tile/platform_overturned.png");
	platform_lookup.insert({0, t_platform_night});
	platform_lookup.insert({1, t_platform_abandoned});
	platform_lookup.insert({2, t_platform_firstwind});
	platform_lookup.insert({3, t_platform_overturned});
	t_automatic_animators_firstwind.loadFromFile(finder.resource_path + "/image/tile/automatic_animators_firstwind.png");
	animator_lookup.insert({0, t_automatic_animators_firstwind});

	t_breakables.loadFromFile(finder.resource_path + "/image/tile/breakables.png");
	t_pushables.loadFromFile(finder.resource_path + "/image/tile/pushables.png");
	t_inspectable.loadFromFile(finder.resource_path + "/image/entity/inspectable.png");
	t_switches.loadFromFile(finder.resource_path + "/image/tile/switches.png");
	t_switch_blocks.loadFromFile(finder.resource_path + "/image/tile/switch_blocks.png");
	t_indicator.loadFromFile(finder.resource_path + "/image/entity/arrow.png");

	// backgrounds
	t_bg_dusk.loadFromFile(finder.resource_path + "/image/background/dusk.png");
	t_bg_opensky.loadFromFile(finder.resource_path + "/image/background/opensky.png");
	t_bg_overcast.loadFromFile(finder.resource_path + "/image/background/overcast.png");
	t_bg_night.loadFromFile(finder.resource_path + "/image/background/night.png");
	t_bg_dawn.loadFromFile(finder.resource_path + "/image/background/dawn.png");
	t_bg_sunrise.loadFromFile(finder.resource_path + "/image/background/sunrise.png");
	t_bg_rosyhaze.loadFromFile(finder.resource_path + "/image/background/rosyhaze.png");
	t_bg_slime.loadFromFile(finder.resource_path + "/image/background/slime.png");
	t_bg_dirt.loadFromFile(finder.resource_path + "/image/background/dirt.png");
	t_bg_grove.loadFromFile(finder.resource_path + "/image/background/glade.png");

	background_lookup.insert({0, t_bg_dusk});
	background_lookup.insert({1, t_bg_sunrise});
	background_lookup.insert({2, t_bg_opensky});
	background_lookup.insert({3, t_bg_rosyhaze});
	background_lookup.insert({4, t_bg_dawn});
	background_lookup.insert({5, t_bg_night});
	background_lookup.insert({6, t_bg_overcast});
	background_lookup.insert({7, t_bg_slime});
	background_lookup.insert({8, t_bg_black});
	background_lookup.insert({9, t_bg_navy});
	background_lookup.insert({10, t_bg_dirt});
	background_lookup.insert({11, t_bg_gear});
	background_lookup.insert({12, t_bg_library});
	background_lookup.insert({13, t_bg_granite});
	background_lookup.insert({14, t_bg_ruins});
	background_lookup.insert({15, t_bg_crevasse});
	background_lookup.insert({16, t_bg_deep});
	background_lookup.insert({17, t_bg_grove});

	t_large_animators.loadFromFile(finder.resource_path + "/image/animators/large_animators_01.png");
	t_small_animators.loadFromFile(finder.resource_path + "/image/animators/small_animators_01.png");

	t_huge_explosion.loadFromFile(finder.resource_path + "/image/entity/huge_explosion.png");
	t_large_explosion.loadFromFile(finder.resource_path + "/image/entity/large_explosion.png");
	t_small_explosion.loadFromFile(finder.resource_path + "/image/entity/small_explosion.png");
	t_wall_hit.loadFromFile(finder.resource_path + "/image/entity/wall_hit.png");
	t_mini_flash.loadFromFile(finder.resource_path + "/image/entity/small_flash.png");
	t_medium_flash.loadFromFile(finder.resource_path + "/image/entity/medium_flash.png");
	t_inv_hit.loadFromFile(finder.resource_path + "/image/entity/inv_hit.png");
	t_puff.loadFromFile(finder.resource_path + "/image/entity/puff.png");
	t_bullet_hit.loadFromFile(finder.resource_path + "/image/entity/bullet_hit.png");
	t_doublejump.loadFromFile(finder.resource_path + "/image/entity/doublejump.png");
	t_dust.loadFromFile(finder.resource_path + "/image/entity/dust.png");
	effect_lookup.insert({0, t_small_explosion});
	effect_lookup.insert({1, t_large_explosion});
	effect_lookup.insert({2, t_wall_hit});
	effect_lookup.insert({3, t_huge_explosion});
	effect_lookup.insert({4, t_mini_flash});
	effect_lookup.insert({5, t_medium_flash});
	effect_lookup.insert({6, t_inv_hit});
	effect_lookup.insert({7, t_puff});
	effect_lookup.insert({8, t_bullet_hit});
	effect_lookup.insert({9, t_doublejump});
	effect_lookup.insert({10, t_dust});

	// title stuff
	t_title.loadFromFile(finder.resource_path + "/image/gui/title.png");

	// load all the other textures...

	// load tilesets programatically (filenames had better be right...)
	for (int i = 0; i < lookup::NUM_STYLES; ++i) {
		char const* next = lookup::get_style_string.at(lookup::get_style.at(i));
		styles[i] = next;
	}
	for (int i = 0; i < lookup::NUM_STYLES; ++i) {
		tilesets.push_back(sf::Texture());
		std::string style = lookup::get_style_string.at(lookup::get_style.at(i));
		tilesets.back().loadFromFile(finder.resource_path + "/image/tile/" + style + "_tiles.png");
	}

	savepoint.loadFromFile(finder.resource_path + "/image/entity/savepoint.png");
	t_chest.loadFromFile(finder.resource_path + "/image/entity/chest.png");
}

void AssetManager::load_audio() {
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
	menu_open_buffer.loadFromFile(finder.resource_path + "/audio/sfx/gui_upward_select.wav");
	menu_open.setBuffer(menu_open_buffer);
	sharp_click_buffer.loadFromFile(finder.resource_path + "/audio/sfx/click.wav");
	sharp_click.setBuffer(sharp_click_buffer);
	arms_switch_buffer.loadFromFile(finder.resource_path + "/audio/sfx/arms_switch.wav");
	arms_switch.setBuffer(arms_switch_buffer);
	bg_shot_buffer.loadFromFile(finder.resource_path + "/audio/sfx/bg_shot.wav");
	bg_shot.setBuffer(bg_shot_buffer);
	skycorps_ar_buffer.loadFromFile(finder.resource_path + "/audio/sfx/skycorps_ar_shot.wav");
	skycorps_ar_shot.setBuffer(skycorps_ar_buffer);
	plasmer_shot_buffer.loadFromFile(finder.resource_path + "/audio/sfx/plasmer_shot.wav");
	plasmer_shot.setBuffer(plasmer_shot_buffer);
	tomahawk_flight_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tomahawk_flight.wav");
	tomahawk_flight.setBuffer(tomahawk_flight_buffer);
	tomahawk_catch_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tomahawk_catch.wav");
	tomahawk_catch.setBuffer(tomahawk_catch_buffer);
	pop_mid_buffer.loadFromFile(finder.resource_path + "/audio/sfx/clover.wav");
	pop_mid.setBuffer(pop_mid_buffer);
	b_nova.loadFromFile(finder.resource_path + "/audio/sfx/nova_shot.wav");
	nova_shot.setBuffer(b_nova);
	b_staple.loadFromFile(finder.resource_path + "/audio/sfx/staple.wav");
	staple.setBuffer(b_staple);
	b_gnat.loadFromFile(finder.resource_path + "/audio/sfx/gnat.wav");
	gnat.setBuffer(b_gnat);
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
	enem_hit_buffer.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_medium.wav");
	enem_hit.setBuffer(enem_hit_buffer);
	bubble_buffer.loadFromFile(finder.resource_path + "/audio/sfx/mid_pop.wav");
	bubble.setBuffer(bubble_buffer);

	enem_death_1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/enemy_death.wav");
	enem_death_1.setBuffer(enem_death_1_buffer);

	heal_buffer.loadFromFile(finder.resource_path + "/audio/sfx/heal.wav");
	heal.setBuffer(heal_buffer);
	orb_1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_1.wav");
	orb_1.setBuffer(orb_1_buffer);
	orb_2_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_2.wav");
	orb_2.setBuffer(orb_2_buffer);
	orb_3_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_3.wav");
	orb_3.setBuffer(orb_3_buffer);
	orb_4_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_4.wav");
	orb_4.setBuffer(orb_4_buffer);

	tank_alert1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tank_alert_1.wav");
	tank_alert_1.setBuffer(tank_alert1_buffer);
	tank_alert2_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tank_alert_2.wav");
	tank_alert_2.setBuffer(tank_alert2_buffer);
	tank_hurt1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tank_hurt_1.wav");
	tank_hurt_1.setBuffer(tank_hurt1_buffer);
	tank_hurt2_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tank_hurt_2.wav");
	tank_hurt_2.setBuffer(tank_hurt2_buffer);
	tank_death_buffer.loadFromFile(finder.resource_path + "/audio/sfx/tank_death.wav");
	tank_death.setBuffer(tank_death_buffer);

	// minigus

	b_minigus_laugh.loadFromFile(finder.resource_path + "/audio/sfx/minigus/minigus_laugh.wav");
	b_minigus_laugh_2.loadFromFile(finder.resource_path + "/audio/sfx/minigus/minigus_laugh_2.wav");
	b_minigus_hurt_1.loadFromFile(finder.resource_path + "/audio/sfx/minigus/minigus_hurt.wav");
	b_minigus_hurt_2.loadFromFile(finder.resource_path + "/audio/sfx/minigus/minigus_hurt_2.wav");
	b_minigus_hurt_3.loadFromFile(finder.resource_path + "/audio/sfx/minigus/minigus_hurt_3.wav");
	b_minigus_grunt.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_grunt.wav");
	b_minigus_aww.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_aww.wav");
	b_minigus_babyimhome.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_babyimhome.wav");
	b_minigus_deepspeak.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_deepspeak.wav");
	b_minigus_doge.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_doge.wav");
	b_minigus_dontlookatme.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_dontlookatme.wav");
	b_minigus_exhale.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_exhale.wav");
	b_minigus_getit.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_getit.wav");
	b_minigus_greatidea.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_greatidea.wav");
	b_minigus_itsagreatday.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_itsagreatday.wav");
	b_minigus_long_death.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_long_death.wav");
	b_minigus_long_moan.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_long_moan.wav");
	b_minigus_momma.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_momma.wav");
	b_minigus_mother.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_mother.wav");
	b_minigus_ok_1.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_ok.wav");
	b_minigus_ok_2.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_okayyy.wav");
	b_minigus_pizza.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_pizza.wav");
	b_minigus_poh.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_poh.wav");
	b_minigus_quick_breath.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_quick_breath.wav");
	b_minigus_thatisverysneeze.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_thatisverysneeze.wav");
	b_minigus_whatisit.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_whatisit.wav");
	b_minigus_woob.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_woob.wav");

	b_mirin_ah.loadFromFile(finder.resource_path + "/audio/sfx/mirin/mirin_ah.wav");
	b_mirin_oh.loadFromFile(finder.resource_path + "/audio/sfx/mirin/mirin_oh.wav");
	b_mirin_laugh.loadFromFile(finder.resource_path + "/audio/sfx/mirin/mirin_laugh.wav");
	vs_mirin.push_back(b_mirin_ah);
	vs_mirin.push_back(b_mirin_oh);
	vs_mirin.push_back(b_mirin_laugh);
	npc_sounds.insert({"mirin", vs_mirin});

	b_heavy_land.loadFromFile(finder.resource_path + "/audio/sfx/deep/heavy_land.wav");
	b_delay_crash.loadFromFile(finder.resource_path + "/audio/sfx/deep/delay_crash.wav");
	b_delay_high.loadFromFile(finder.resource_path + "/audio/sfx/deep/delay_high.wav");
	b_laser.loadFromFile(finder.resource_path + "/audio/sfx/laser1.wav");
	b_gun_charge.loadFromFile(finder.resource_path + "/audio/sfx/gun_charge.wav");
	b_minigus_build.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_build.wav");
	b_minigus_invincibility.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_inv.wav");
	b_soda.loadFromFile(finder.resource_path + "/audio/sfx/soda.wav");
	b_breakable_hit.loadFromFile(finder.resource_path + "/audio/sfx/breakable_hit.wav");

	b_enemy_hit_low.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_low.wav");
	b_enemy_hit_medium.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_medium.wav");
	b_enemy_hit_high.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_high.wav");
	b_enemy_hit_squeak.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_squeak.wav");
	b_enemy_hit_inv.loadFromFile(finder.resource_path + "/audio/sfx/enemy/hit_inv.wav");
	b_wall_hit.loadFromFile(finder.resource_path + "/audio/sfx/wall_hit.wav");
	b_thud.loadFromFile(finder.resource_path + "/audio/sfx/thud.wav");
	b_small_crash.loadFromFile(finder.resource_path + "/audio/sfx/small_crash.wav");
	b_heavy_move.loadFromFile(finder.resource_path + "/audio/sfx/heavy_move.wav");
	hard_hit.setBuffer(b_enemy_hit_inv);
	wall_hit.setBuffer(b_wall_hit);
	thud.setBuffer(b_thud);
	small_crash.setBuffer(b_small_crash);
	heavy_move.setBuffer(b_heavy_move);

	save_buffer.loadFromFile(finder.resource_path + "/audio/sfx/save_point.wav");
	save.setBuffer(save_buffer);
	load_buffer.loadFromFile(finder.resource_path + "/audio/sfx/load_game.wav");
	load.setBuffer(load_buffer);
	soft_sparkle_high_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle_high.wav");
	soft_sparkle_high.setBuffer(soft_sparkle_high_buffer);
	soft_sparkle_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle.wav");
	soft_sparkle.setBuffer(soft_sparkle_buffer);
	chest_buffer.loadFromFile(finder.resource_path + "/audio/sfx/chest.wav");
	b_switch_press.loadFromFile(finder.resource_path + "/audio/sfx/switch_press.wav");
	b_block_toggle.loadFromFile(finder.resource_path + "/audio/sfx/block_toggle.wav");
	chest.setBuffer(chest_buffer);
	breakable_hit.setBuffer(b_breakable_hit);
	breakable_shatter.setBuffer(shatter_buffer);
	switch_press.setBuffer(b_switch_press);
	block_toggle.setBuffer(b_block_toggle);
}

} // namespace data
