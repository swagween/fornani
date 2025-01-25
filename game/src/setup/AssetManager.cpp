#include "fornani/setup/AssetManager.hpp"

namespace asset {
AssetManager::AssetManager(data::ResourceFinder& finder) {

	if (!t_null.loadFromFile(finder.resource_path() + "/image/app/null.png")) { std::cout << "Failed to load texture.\n"; };

	// TODO: Replace this with actually handling the results passed instead of doing this hacky work around.
	[[maybe_unused]] bool res = false;

	res = t_nani.loadFromFile(finder.resource_path() + "/image/character/nani.png");
	res = t_portraits.loadFromFile(finder.resource_path() + "/image/character/portraits.png");
	res = t_bryn.loadFromFile(finder.resource_path() + "/image/character/bryn.png");
	res = t_gobe.loadFromFile(finder.resource_path() + "/image/character/gobe.png");
	res = t_dr_go.loadFromFile(finder.resource_path() + "/image/character/dr_go.png");
	res = t_carl.loadFromFile(finder.resource_path() + "/image/character/carl.png");
	res = t_mirin.loadFromFile(finder.resource_path() + "/image/character/mirin.png");
	res = t_bit.loadFromFile(finder.resource_path() + "/image/character/bit.png");
	res = t_lady_nimbus.loadFromFile(finder.resource_path() + "/image/character/lady_nimbus.png");
	res = t_justin.loadFromFile(finder.resource_path() + "/image/character/justin.png");
	npcs.insert({"bryn", t_bryn});
	npcs.insert({"gobe", t_gobe});
	npcs.insert({"dr_go", t_dr_go});
	npcs.insert({"carl", t_carl});
	npcs.insert({"mirin", t_mirin});
	npcs.insert({"bit", t_bit});
	npcs.insert({"lady_nimbus", t_lady_nimbus});
	npcs.insert({"justin", t_justin});

	res = t_vendor_artwork.loadFromFile(finder.resource_path() + "/image/gui/vendor_artwork.png");
	res = t_vendor_ui.loadFromFile(finder.resource_path() + "/image/gui/vendor_interface.png");

	res = t_palette_nani.loadFromFile(finder.resource_path() + "/image/character/nani_palette_default.png");
	res = t_palette_nanidiv.loadFromFile(finder.resource_path() + "/image/character/nani_palette_divine.png");
	res = t_palette_naninight.loadFromFile(finder.resource_path() + "/image/character/nani_palette_night.png");

	res = t_wardrobe_base.loadFromFile(finder.resource_path() + "/image/wardrobe/base.png");
	res = t_wardrobe_green_pants.loadFromFile(finder.resource_path() + "/image/wardrobe/green_pants.png");
	res = t_wardrobe_blue_shirt.loadFromFile(finder.resource_path() + "/image/wardrobe/blue_shirt.png");
	res = t_wardrobe_default_hair.loadFromFile(finder.resource_path() + "/image/wardrobe/default_hair.png");
	res = t_wardrobe_red_jeans.loadFromFile(finder.resource_path() + "/image/wardrobe/red_jeans.png");
	res = t_wardrobe_chalcedony_tee.loadFromFile(finder.resource_path() + "/image/wardrobe/chalcedony_tee.png");
	res = t_wardrobe_chalcedony_skirt.loadFromFile(finder.resource_path() + "/image/wardrobe/chalcedony_skirt.png");
	res = t_wardrobe_ponytail.loadFromFile(finder.resource_path() + "/image/wardrobe/ponytail.png");
	res = t_wardrobe_punk_hair.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_hair.png");
	res = t_wardrobe_punk_shirt.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_shirt.png");
	res = t_wardrobe_punk_pants.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_pants.png");

	res = t_frdog.loadFromFile(finder.resource_path() + "/image/critter/frdog.png");
	res = t_hulmet.loadFromFile(finder.resource_path() + "/image/critter/hulmet.png");
	res = t_tank.loadFromFile(finder.resource_path() + "/image/critter/tank.png");
	res = t_thug.loadFromFile(finder.resource_path() + "/image/critter/thug.png");
	res = t_eyebot.loadFromFile(finder.resource_path() + "/image/critter/eyebot.png");
	res = t_eyebit.loadFromFile(finder.resource_path() + "/image/critter/eyebit.png");
	res = t_minigus.loadFromFile(finder.resource_path() + "/image/boss/minigus.png");
	res = t_minigus_inv.loadFromFile(finder.resource_path() + "/image/boss/minigus_inv.png");
	res = t_minigus_red.loadFromFile(finder.resource_path() + "/image/boss/minigus_red.png");
	res = t_minigus_blue.loadFromFile(finder.resource_path() + "/image/boss/minigus_blue.png");
	res = t_demon.loadFromFile(finder.resource_path() + "/image/critter/demon.png");
	res = t_demon_shield.loadFromFile(finder.resource_path() + "/image/critter/demon_shield.png");
	res = t_demon_spear.loadFromFile(finder.resource_path() + "/image/critter/demon_spear.png");
	res = t_demon_sword.loadFromFile(finder.resource_path() + "/image/critter/demon_sword.png");
	res = t_caster.loadFromFile(finder.resource_path() + "/image/critter/caster.png");
	res = t_caster_scepter.loadFromFile(finder.resource_path() + "/image/critter/caster_scepter.png");
	res = t_caster_wand.loadFromFile(finder.resource_path() + "/image/critter/caster_wand.png");
	res = t_archer.loadFromFile(finder.resource_path() + "/image/critter/archer.png");
	res = t_archer_bow.loadFromFile(finder.resource_path() + "/image/critter/archer_bow.png");
	res = t_archer_arrow.loadFromFile(finder.resource_path() + "/image/weapon/arrow.png");
	res = t_beamstalk.loadFromFile(finder.resource_path() + "/image/critter/beamstalk.png");
	res = t_meatsquash.loadFromFile(finder.resource_path() + "/image/critter/meatsquash.png");
	texture_lookup.insert({"frdog", t_frdog});
	texture_lookup.insert({"hulmet", t_hulmet});
	texture_lookup.insert({"tank", t_tank});
	texture_lookup.insert({"thug", t_thug});
	texture_lookup.insert({"eyebot", t_eyebot});
	texture_lookup.insert({"eyebit", t_eyebit});
	texture_lookup.insert({"minigus", t_minigus});
	texture_lookup.insert({"demon", t_demon});
	texture_lookup.insert({"caster", t_caster});
	texture_lookup.insert({"archer", t_archer});
	texture_lookup.insert({"beamstalk", t_beamstalk});
	texture_lookup.insert({"meatsquash", t_meatsquash});

	res = t_ui.loadFromFile(finder.resource_path() + "/image/gui/blue_console.png");
	res = t_hud_orb_font.loadFromFile(finder.resource_path() + "/image/gui/HUD_orb_font.png");
	res = t_hud_hearts.loadFromFile(finder.resource_path() + "/image/gui/HUD_hearts.png");
	res = t_hud_ammo.loadFromFile(finder.resource_path() + "/image/gui/HUD_ammo.png");
	res = t_hud_pointer.loadFromFile(finder.resource_path() + "/image/gui/HUD_pointer.png");
	res = t_hud_gun.loadFromFile(finder.resource_path() + "/image/gui/HUD_gun.png");
	res = t_hud_shield.loadFromFile(finder.resource_path() + "/image/gui/HUD_shield.png");
	res = t_selector.loadFromFile(finder.resource_path() + "/image/gui/selector.png");
	res = t_console_outline.loadFromFile(finder.resource_path() + "/image/gui/console_outline.png");
	res = t_sticker.loadFromFile(finder.resource_path() + "/image/gui/sticker.png");
	res = t_controller_button_icons.loadFromFile(finder.resource_path() + "/image/gui/controller_button_icons.png");

	// guns and bullets!
	res = t_gun.loadFromFile(finder.resource_path() + "/image/weapon/gun.png");
	res = t_bryns_gun_projectile.loadFromFile(finder.resource_path() + "/image/weapon/bg_proj.png");
	res = t_plasmer_projectile.loadFromFile(finder.resource_path() + "/image/weapon/plasmer_proj.png");
	res = t_clover_projectile.loadFromFile(finder.resource_path() + "/image/weapon/clover_proj.png");
	res = t_skycorps_ar_projectile.loadFromFile(finder.resource_path() + "/image/weapon/skycorps_ar_proj.png");
	res = t_tomahawk.loadFromFile(finder.resource_path() + "/image/weapon/tomahawk.png");
	res = t_wasp_projectile.loadFromFile(finder.resource_path() + "/image/weapon/wasp_proj.png");
	res = t_rope.loadFromFile(finder.resource_path() + "/image/weapon/rope.png");
	res = t_hook.loadFromFile(finder.resource_path() + "/image/weapon/hook.png");
	res = t_grenade.loadFromFile(finder.resource_path() + "/image/weapon/grenade.png");
	res = t_minigun.loadFromFile(finder.resource_path() + "/image/boss/minigun.png");
	res = t_soda_gun_projectile.loadFromFile(finder.resource_path() + "/image/weapon/soda.png");
	res = t_staple.loadFromFile(finder.resource_path() + "/image/weapon/staple.png");
	res = t_indie_projectile.loadFromFile(finder.resource_path() + "/image/weapon/indie_proj.png");
	res = t_gnat_projectile.loadFromFile(finder.resource_path() + "/image/weapon/gnat_proj.png");
	res = t_energy_ball_projectile.loadFromFile(finder.resource_path() + "/image/weapon/energy_ball.png");
	res = t_peckett_projectile.loadFromFile(finder.resource_path() + "/image/weapon/peckett_710_proj.png");
	res = t_arrow_projectile.loadFromFile(finder.resource_path() + "/image/weapon/arrow.png");
	res = t_archer_arrow_projectile.loadFromFile(finder.resource_path() + "/image/weapon/archer_arrow.png");
	res = t_green_beam.loadFromFile(finder.resource_path() + "/image/weapon/green_beam.png");

	projectile_textures.insert({"Bryn's Gun", t_bryns_gun_projectile});
	projectile_textures.insert({"Skycorps AR", t_skycorps_ar_projectile});
	projectile_textures.insert({"Plasmer", t_plasmer_projectile});
	projectile_textures.insert({"Tomahawk", t_tomahawk});
	projectile_textures.insert({"Clover", t_clover_projectile});
	projectile_textures.insert({"Grappling Hook", t_hook});
	projectile_textures.insert({"Grenade Launcher", t_hook});
	projectile_textures.insert({"Minigun", t_skycorps_ar_projectile});
	projectile_textures.insert({"Soda Gun", t_soda_gun_projectile});
	projectile_textures.insert({"Staple Gun", t_staple});
	projectile_textures.insert({"Indie", t_indie_projectile});
	projectile_textures.insert({"Gnat", t_gnat_projectile});
	projectile_textures.insert({"Energy Ball", t_energy_ball_projectile});
	projectile_textures.insert({"Wasp", t_wasp_projectile});
	projectile_textures.insert({"Underdog", t_wasp_projectile});
	projectile_textures.insert({"Peckett 710", t_peckett_projectile});
	projectile_textures.insert({"Longbow", t_arrow_projectile});
	projectile_textures.insert({"Demon Bow", t_archer_arrow_projectile});
	projectile_textures.insert({"Green Beam", t_green_beam});

	res = t_items.loadFromFile(finder.resource_path() + "/image/item/items.png");
	res = t_guns.loadFromFile(finder.resource_path() + "/image/item/guns.png");
	res = t_shield.loadFromFile(finder.resource_path() + "/image/entity/shield.png");

	res = t_heart.loadFromFile(finder.resource_path() + "/image/item/hearts.png");
	res = t_orb.loadFromFile(finder.resource_path() + "/image/item/orbs.png");
	res = t_gem.loadFromFile(finder.resource_path() + "/image/item/gems.png");

	res = t_fader.loadFromFile(finder.resource_path() + "/image/vfx/fader.png");
	res = t_firefly.loadFromFile(finder.resource_path() + "/image/vfx/firefly.png");
	res = t_dragonfly.loadFromFile(finder.resource_path() + "/image/vfx/dragonfly.png");
	res = t_fire.loadFromFile(finder.resource_path() + "/image/vfx/fire.png");

	res = t_twinkle.loadFromFile(finder.resource_path() + "/image/vfx/twinkle.png");
	res = t_wasp_effect.loadFromFile(finder.resource_path() + "/image/vfx/wasp_effect.png");
	res = t_peckett_effect.loadFromFile(finder.resource_path() + "/image/vfx/peckett_effect.png");
	res = t_bg_effect.loadFromFile(finder.resource_path() + "/image/vfx/bryns_gun_effect.png");
	res = t_small_flash.loadFromFile(finder.resource_path() + "/image/vfx/small_flash.png");
	res = t_smoke.loadFromFile(finder.resource_path() + "/image/vfx/smoke.png");
	res = t_green_beam_effect.loadFromFile(finder.resource_path() + "/image/vfx/green_beam_effect.png");
	particle_textures.insert({"twinkle", t_twinkle});
	particle_textures.insert({"wasp", t_wasp_effect});
	particle_textures.insert({"smoke", t_smoke});
	particle_textures.insert({"green_beam", t_green_beam_effect});
	particle_textures.insert({"peckett", t_peckett_effect});
	particle_textures.insert({"bryns_gun_smoke", t_bg_effect});

	res = t_alphabet.loadFromFile(finder.resource_path() + "/image/gui/alphabet.png");
	res = t_blue_console.loadFromFile(finder.resource_path() + "/image/gui/blue_console.png");
	res = t_cream_console.loadFromFile(finder.resource_path() + "/image/gui/cream_console.png");
	res = t_portrait_window.loadFromFile(finder.resource_path() + "/image/gui/portrait_window.png");

	res = t_platforms.loadFromFile(finder.resource_path() + "/image/tile/platforms.png");
	res = t_platform_night.loadFromFile(finder.resource_path() + "/image/tile/platform_night.png");
	res = t_platform_abandoned.loadFromFile(finder.resource_path() + "/image/tile/platform_abandoned.png");
	res = t_platform_firstwind.loadFromFile(finder.resource_path() + "/image/tile/platform_firstwind.png");
	res = t_platform_overturned.loadFromFile(finder.resource_path() + "/image/tile/platform_overturned.png");
	platform_lookup.insert({0, t_platform_night});
	platform_lookup.insert({1, t_platform_abandoned});
	platform_lookup.insert({2, t_platform_firstwind});
	platform_lookup.insert({3, t_platform_overturned});
	res = t_automatic_animators_firstwind.loadFromFile(finder.resource_path() + "/image/tile/automatic_animators_firstwind.png");
	animator_lookup.insert({0, t_automatic_animators_firstwind});

	res = t_breakables.loadFromFile(finder.resource_path() + "/image/tile/breakables.png");
	res = t_pushables.loadFromFile(finder.resource_path() + "/image/tile/pushables.png");
	res = t_inspectable.loadFromFile(finder.resource_path() + "/image/entity/inspectable.png");
	res = t_switches.loadFromFile(finder.resource_path() + "/image/tile/switches.png");
	res = t_switch_blocks.loadFromFile(finder.resource_path() + "/image/tile/switch_blocks.png");
	res = t_indicator.loadFromFile(finder.resource_path() + "/image/entity/arrow.png");
	res = t_portals.loadFromFile(finder.resource_path() + "/image/tile/portals.png");
	res = t_vine.loadFromFile(finder.resource_path() + "/image/tile/vine.png");
	res = t_vine_large.loadFromFile(finder.resource_path() + "/image/tile/vine_large.png");
	res = t_grass.loadFromFile(finder.resource_path() + "/image/tile/grass.png");
	res = t_big_spike.loadFromFile(finder.resource_path() + "/image/tile/big_spike.png");

	// backgrounds
	res = t_bg_dusk.loadFromFile(finder.resource_path() + "/image/background/dusk.png");
	res = t_bg_opensky.loadFromFile(finder.resource_path() + "/image/background/opensky.png");
	res = t_bg_overcast.loadFromFile(finder.resource_path() + "/image/background/overcast.png");
	res = t_bg_night.loadFromFile(finder.resource_path() + "/image/background/night.png");
	res = t_bg_dawn.loadFromFile(finder.resource_path() + "/image/background/dawn.png");
	res = t_bg_sunrise.loadFromFile(finder.resource_path() + "/image/background/sunrise.png");
	res = t_bg_rosyhaze.loadFromFile(finder.resource_path() + "/image/background/rosyhaze.png");
	res = t_bg_slime.loadFromFile(finder.resource_path() + "/image/background/slime.png");
	res = t_bg_dirt.loadFromFile(finder.resource_path() + "/image/background/dirt.png");
	res = t_bg_grove.loadFromFile(finder.resource_path() + "/image/background/glade.png");
	res = t_bg_woods.loadFromFile(finder.resource_path() + "/image/background/woods.png");

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
	background_lookup.insert({18, t_bg_woods});

	res = t_overturned_scenery.loadFromFile(finder.resource_path() + "/image/background/overturned_scenery.png");
	scenery_lookup.insert({1, t_overturned_scenery});

	res = t_large_animators.loadFromFile(finder.resource_path() + "/image/animators/large_animators.png");
	res = t_small_animators.loadFromFile(finder.resource_path() + "/image/animators/small_animators.png");

	res = t_treasure_ball.loadFromFile(finder.resource_path() + "/image/entity/treasure_ball.png");
	res = t_spawnable_platform.loadFromFile(finder.resource_path() + "/image/entity/spawnable_platform.png");
	res = t_huge_explosion.loadFromFile(finder.resource_path() + "/image/entity/huge_explosion.png");
	res = t_large_explosion.loadFromFile(finder.resource_path() + "/image/entity/large_explosion.png");
	res = t_small_explosion.loadFromFile(finder.resource_path() + "/image/entity/small_explosion.png");
	res = t_wall_hit.loadFromFile(finder.resource_path() + "/image/entity/wall_hit.png");
	res = t_mini_flash.loadFromFile(finder.resource_path() + "/image/entity/small_flash.png");
	res = t_medium_flash.loadFromFile(finder.resource_path() + "/image/entity/medium_flash.png");
	res = t_inv_hit.loadFromFile(finder.resource_path() + "/image/entity/inv_hit.png");
	res = t_puff.loadFromFile(finder.resource_path() + "/image/entity/puff.png");
	res = t_bullet_hit.loadFromFile(finder.resource_path() + "/image/entity/bullet_hit.png");
	res = t_doublejump.loadFromFile(finder.resource_path() + "/image/entity/doublejump.png");
	res = t_dust.loadFromFile(finder.resource_path() + "/image/entity/dust.png");
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
	res = t_title.loadFromFile(finder.resource_path() + "/image/gui/title.png");

	// load all the other textures...
	for (int i = 0; i < static_cast<size_t>(lookup::Style::END); ++i) {
		tilesets.push_back(sf::Texture());
		std::string style = lookup::get_style_string.at(static_cast<lookup::Style>(i));
		tilesets.back().loadFromFile(finder.resource_path() + "/image/tile/" + style + "_tiles.png");
	}

	res = savepoint.loadFromFile(finder.resource_path() + "/image/entity/savepoint.png");
	res = t_chest.loadFromFile(finder.resource_path() + "/image/entity/chest.png");


	res = click_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/heavy_click.wav");
	res = sharp_click_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/click.wav");
	res = menu_shift_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/menu_shift_1.wav");
	res = menu_back_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/menu_shift_2.wav");
	res = menu_next_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/menu_shift_3.wav");
	res = menu_open_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/gui_upward_select.wav");
	res = sharp_click_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/click.wav");
	res = arms_switch_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/arms_switch.wav");
	res = bg_shot_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/bg_shot.wav");
	res = b_wasp.loadFromFile(finder.resource_path() + "/audio/sfx/wasp_shot.wav");
	res = skycorps_ar_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/skycorps_ar_shot.wav");
	res = plasmer_shot_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/plasmer_shot.wav");
	res = tomahawk_flight_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tomahawk_flight.wav");
	res = tomahawk_catch_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tomahawk_catch.wav");
	res = pop_mid_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/clover.wav");
	res = b_nova.loadFromFile(finder.resource_path() + "/audio/sfx/nova_shot.wav");
	res = b_staple.loadFromFile(finder.resource_path() + "/audio/sfx/staple.wav");
	res = b_gnat.loadFromFile(finder.resource_path() + "/audio/sfx/gnat.wav");
	res = jump_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/jump.wav");
	res = slide_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/slide.wav");
	res = b_walljump.loadFromFile(finder.resource_path() + "/audio/sfx/walljump.wav");
	res = b_roll.loadFromFile(finder.resource_path() + "/audio/sfx/roll.wav");

	res = b_reload.loadFromFile(finder.resource_path() + "/audio/sfx/reload.wav");

	res = shatter_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/shatter.wav");
	res = step_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/steps.wav");
	res = grass_step_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/grass_steps.wav");
	res = landed_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/landed.wav");
	res = landed_grass_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/landed_grass.wav");
	res = hurt_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/hurt.wav");
	res = player_death_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/player_death.wav");
	res = enem_hit_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_medium.wav");
	res = bubble_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/mid_pop.wav");

	res = enem_death_1_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/enemy_death.wav");

	res = heal_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/heal.wav");
	res = b_health_increase.loadFromFile(finder.resource_path() + "/audio/sfx/health_increase.wav");
	res = orb_1_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/orb_1.wav");
	res = orb_2_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/orb_2.wav");
	res = orb_3_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/orb_3.wav");
	res = orb_4_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/orb_4.wav");
	res = b_upward_get.loadFromFile(finder.resource_path() + "/audio/sfx/upward_get.wav");

	res = tank_alert1_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tank_alert_1.wav");
	res = tank_alert2_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tank_alert_2.wav");
	res = tank_hurt1_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tank_hurt_1.wav");
	res = tank_hurt2_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tank_hurt_2.wav");
	res = tank_death_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/tank_death.wav");
	res = b_demon_snort.loadFromFile(finder.resource_path() + "/audio/sfx/demon/snort.wav");

	// minigus

	res = b_minigus_laugh.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/minigus_laugh.wav");
	res = b_minigus_laugh_2.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/minigus_laugh_2.wav");
	res = b_minigus_hurt_1.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/minigus_hurt.wav");
	res = b_minigus_hurt_2.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/minigus_hurt_2.wav");
	res = b_minigus_hurt_3.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/minigus_hurt_3.wav");
	res = b_minigus_grunt.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_grunt.wav");
	res = b_minigus_aww.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_aww.wav");
	res = b_minigus_babyimhome.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_babyimhome.wav");
	res = b_minigus_deepspeak.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_deepspeak.wav");
	res = b_minigus_doge.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_doge.wav");
	res = b_minigus_dontlookatme.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_dontlookatme.wav");
	res = b_minigus_exhale.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_exhale.wav");
	res = b_minigus_getit.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_getit.wav");
	res = b_minigus_greatidea.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_greatidea.wav");
	res = b_minigus_itsagreatday.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_itsagreatday.wav");
	res = b_minigus_long_death.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_long_death.wav");
	res = b_minigus_long_moan.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_long_moan.wav");
	res = b_minigus_momma.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_momma.wav");
	res = b_minigus_mother.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_mother.wav");
	res = b_minigus_ok_1.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_ok.wav");
	res = b_minigus_ok_2.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_okayyy.wav");
	res = b_minigus_pizza.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_pizza.wav");
	res = b_minigus_poh.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_poh.wav");
	res = b_minigus_quick_breath.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_quick_breath.wav");
	res = b_minigus_thatisverysneeze.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_thatisverysneeze.wav");
	res = b_minigus_whatisit.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_whatisit.wav");
	res = b_minigus_woob.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_woob.wav");

	res = b_mirin_ah.loadFromFile(finder.resource_path() + "/audio/sfx/mirin/mirin_ah.wav");
	res = b_mirin_oh.loadFromFile(finder.resource_path() + "/audio/sfx/mirin/mirin_oh.wav");
	res = b_mirin_laugh.loadFromFile(finder.resource_path() + "/audio/sfx/mirin/mirin_laugh.wav");
	res = b_carl_huh.loadFromFile(finder.resource_path() + "/audio/sfx/carl/carl_huh.wav");
	res = b_carl_eh.loadFromFile(finder.resource_path() + "/audio/sfx/carl/carl_eh.wav");
	res = b_carl_and.loadFromFile(finder.resource_path() + "/audio/sfx/carl/carl_and.wav");
	vs_mirin.push_back(b_mirin_ah);
	vs_mirin.push_back(b_mirin_oh);
	vs_mirin.push_back(b_mirin_laugh);
	npc_sounds.insert({"mirin", vs_mirin});
	vs_hologus.push_back(b_minigus_pizza);
	vs_hologus.push_back(b_minigus_dontlookatme);
	vs_hologus.push_back(b_minigus_babyimhome);
	vs_hologus.push_back(b_minigus_laugh);
	vs_hologus.push_back(b_minigus_itsagreatday);
	vs_hologus.push_back(b_minigus_ok_1);
	npc_sounds.insert({"hologus", vs_hologus});
	vs_carl.push_back(b_carl_huh);
	vs_carl.push_back(b_carl_eh);
	vs_carl.push_back(b_carl_and);
	npc_sounds.insert({"carl", vs_carl});

	res = b_heavy_land.loadFromFile(finder.resource_path() + "/audio/sfx/deep/heavy_land.wav");
	res = b_delay_crash.loadFromFile(finder.resource_path() + "/audio/sfx/deep/delay_crash.wav");
	res = b_delay_high.loadFromFile(finder.resource_path() + "/audio/sfx/deep/delay_high.wav");
	res = b_laser.loadFromFile(finder.resource_path() + "/audio/sfx/laser1.wav");
	res = b_energy_shot.loadFromFile(finder.resource_path() + "/audio/sfx/energy_shot.wav");
	res = b_gun_charge.loadFromFile(finder.resource_path() + "/audio/sfx/gun_charge.wav");
	res = b_minigus_build.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_build.wav");
	res = b_minigus_invincibility.loadFromFile(finder.resource_path() + "/audio/sfx/minigus/mg_inv.wav");
	res = b_soda.loadFromFile(finder.resource_path() + "/audio/sfx/soda.wav");
	res = b_breakable_hit.loadFromFile(finder.resource_path() + "/audio/sfx/breakable_hit.wav");

	res = b_enemy_hit_low.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_low.wav");
	res = b_enemy_hit_medium.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_medium.wav");
	res = b_enemy_hit_high.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_high.wav");
	res = b_enemy_hit_squeak.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_squeak.wav");
	res = b_enemy_hit_inv.loadFromFile(finder.resource_path() + "/audio/sfx/enemy/hit_inv.wav");
	res = b_wall_hit.loadFromFile(finder.resource_path() + "/audio/sfx/wall_hit.wav");
	res = b_soft_tap.loadFromFile(finder.resource_path() + "/audio/sfx/soft_tap.wav");
	res = b_thud.loadFromFile(finder.resource_path() + "/audio/sfx/thud.wav");
	res = b_small_crash.loadFromFile(finder.resource_path() + "/audio/sfx/small_crash.wav");
	res = b_heavy_move.loadFromFile(finder.resource_path() + "/audio/sfx/heavy_move.wav");
	res = b_door_open.loadFromFile(finder.resource_path() + "/audio/sfx/door_open.wav");
	res = b_door_unlock.loadFromFile(finder.resource_path() + "/audio/sfx/door_unlock.wav");

	res = save_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/save_point.wav");
	res = load_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/load_game.wav");
	res = soft_sparkle_high_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/soft_sparkle_high.wav");
	res = soft_sparkle_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/soft_sparkle.wav");
	res = chest_buffer.loadFromFile(finder.resource_path() + "/audio/sfx/chest.wav");
	res = b_switch_press.loadFromFile(finder.resource_path() + "/audio/sfx/switch_press.wav");
	res = b_block_toggle.loadFromFile(finder.resource_path() + "/audio/sfx/block_toggle.wav");
}

sf::Texture& AssetManager::get_background(int id) {
	if (!background_lookup.contains(id)) { return background_lookup.at(0); }
	return background_lookup.at(id);
}

sf::Texture& AssetManager::get_scenery(int style) {
	if (!scenery_lookup.contains(style)) { return scenery_lookup.at(1); }
	return scenery_lookup.at(style);
}

} // namespace data
