
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <filesystem>
#include <string>

#include "../graphics/FLColor.hpp"
#include "../setup/EnumLookups.hpp"
#include "ResourceFinder.hpp"

namespace asset {

const uint8_t TILE_WIDTH = 32;
int const NANI_SPRITESHEET_WIDTH = 22;
int const NANI_SPRITESHEET_HEIGHT = 10;
const uint8_t NANI_SPRITE_WIDTH = 48;
inline char const* styles[lookup::NUM_STYLES];

class AssetManager {
  public:
	AssetManager() = default;

	void importTextures() {
		t_nani.loadFromFile(finder.resource_path + "/image/character/nani.png");
		t_portraits.loadFromFile(finder.resource_path + "/image/character/portraits.png");
		t_bryn.loadFromFile(finder.resource_path + "/image/character/bryn.png");
		t_gobe.loadFromFile(finder.resource_path + "/image/character/gobe.png");
		npcs.insert({"bryn", t_bryn});
		npcs.insert({"gobe", t_gobe});

		t_palette_nani.loadFromFile(finder.resource_path + "/image/character/nani_palette_default.png");
		t_palette_nanidiv.loadFromFile(finder.resource_path + "/image/character/nani_palette_divine.png");
		t_palette_naninight.loadFromFile(finder.resource_path + "/image/character/nani_palette_night.png");

		t_bryn_test.loadFromFile(finder.resource_path + "/image/portrait/bryn_test.png");

		t_frdog.loadFromFile(finder.resource_path + "/image/critter/frdog.png");
		t_hulmet.loadFromFile(finder.resource_path + "/image/critter/hulmet.png");
		t_tank.loadFromFile(finder.resource_path + "/image/critter/tank.png");
		t_eyebot.loadFromFile(finder.resource_path + "/image/critter/eyebot.png");
		t_eyebit.loadFromFile(finder.resource_path + "/image/critter/eyebit.png");
		t_minigus.loadFromFile(finder.resource_path + "/image/boss/minigus.png");
		t_minigus_inv.loadFromFile(finder.resource_path + "/image/boss/minigus_inv.png");
		t_minigus_red.loadFromFile(finder.resource_path + "/image/boss/minigus_red.png");
		t_minigus_blue.loadFromFile(finder.resource_path + "/image/boss/minigus_blue.png");
		texture_lookup.insert({"frdog", t_frdog});
		texture_lookup.insert({"hulmet", t_hulmet});
		texture_lookup.insert({"tank", t_tank});
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
		weapon_textures.insert({"soda_gun", t_soda_gun});
		projectile_textures.insert({"soda_gun", t_soda_gun_projectile});

		t_items.loadFromFile(finder.resource_path + "/image/item/items.png");
		t_shield.loadFromFile(finder.resource_path + "/image/entity/shield.png");

		t_heart.loadFromFile(finder.resource_path + "/image/item/hearts.png");
		t_orb.loadFromFile(finder.resource_path + "/image/item/orbs.png");

		t_twinkle.loadFromFile(finder.resource_path + "/image/vfx/twinkle.png");
		t_small_flash.loadFromFile(finder.resource_path + "/image/vfx/small_flash.png");
		particle_textures.insert({"twinkle", t_twinkle});

		t_alphabet.loadFromFile(finder.resource_path + "/image/gui/alphabet.png");

		t_platforms.loadFromFile(finder.resource_path + "/image/tile/platforms.png");
		t_platform_night.loadFromFile(finder.resource_path + "/image/tile/platform_night.png");
		t_platform_abandoned.loadFromFile(finder.resource_path + "/image/tile/platform_abandoned.png");
		platform_lookup.insert({0, t_platform_night});
		platform_lookup.insert({1, t_platform_abandoned});

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
		effect_lookup.insert({0, t_small_explosion});
		effect_lookup.insert({1, t_large_explosion});
		effect_lookup.insert({2, t_wall_hit});
		effect_lookup.insert({3, t_huge_explosion});
		effect_lookup.insert({4, t_mini_flash});
		effect_lookup.insert({5, t_medium_flash});

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

	void load_audio() {
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
		enem_hit_buffer.loadFromFile(finder.resource_path + "/audio/sfx/rainy_ouch.wav");
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

		b_heavy_land.loadFromFile(finder.resource_path + "/audio/sfx/deep/heavy_land.wav");
		b_delay_crash.loadFromFile(finder.resource_path + "/audio/sfx/deep/delay_crash.wav");
		b_delay_high.loadFromFile(finder.resource_path + "/audio/sfx/deep/delay_high.wav");
		b_laser.loadFromFile(finder.resource_path + "/audio/sfx/laser1.wav");
		b_gun_charge.loadFromFile(finder.resource_path + "/audio/sfx/gun_charge.wav");
		b_minigus_build.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_build.wav");
		b_minigus_invincibility.loadFromFile(finder.resource_path + "/audio/sfx/minigus/mg_inv.wav");
		b_soda.loadFromFile(finder.resource_path + "/audio/sfx/soda.wav");

		save_buffer.loadFromFile(finder.resource_path + "/audio/sfx/save_point.wav");
		save.setBuffer(save_buffer);
		load_buffer.loadFromFile(finder.resource_path + "/audio/sfx/load_game.wav");
		load.setBuffer(load_buffer);
		soft_sparkle_high_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle_high.wav");
		soft_sparkle_high.setBuffer(soft_sparkle_high_buffer);
		soft_sparkle_buffer.loadFromFile(finder.resource_path + "/audio/sfx/soft_sparkle.wav");
		soft_sparkle.setBuffer(soft_sparkle_buffer);
		chest_buffer.loadFromFile(finder.resource_path + "/audio/sfx/chest.wav");
		chest.setBuffer(chest_buffer);
	}

	// player and NPCs!
	sf::Texture t_nani{};
	sf::Texture t_portraits{};
	sf::Texture t_bryn{};
	sf::Texture t_gobe{};

	//player palettes
	sf::Texture t_palette_nani{};
	sf::Texture t_palette_nanidiv{};
	sf::Texture t_palette_naninight{};

	// portraits!
	sf::Texture t_bryn_test{};
	sf::Sprite sp_bryn_test{};
	sf::Texture t_ui_test{};
	sf::Sprite sp_ui_test{};

	//items
	sf::Texture t_items{};

	// critters and bosses!
	sf::Texture t_frdog{};
	sf::Texture t_hulmet{};
	sf::Texture t_tank{};
	sf::Texture t_eyebot{};
	sf::Texture t_eyebit{};
	sf::Texture t_minigus{};
	sf::Texture t_minigus_inv{};
	sf::Texture t_minigus_red{};
	sf::Texture t_minigus_blue{};

	// gui
	sf::Texture t_ui{};
	sf::Texture t_title{};
	sf::Texture t_title_assets{};
	sf::Texture t_file_text{};

	sf::Texture t_hud_orb_font{};
	sf::Texture t_hud_hearts{};
	sf::Texture t_hud_gun_color{};
	sf::Texture t_hud_gun_shadow{};
	sf::Texture t_hud_pointer{};
	sf::Texture t_hud_shield{};
	sf::Texture t_selector{};
	sf::Texture t_console_outline{};

	sf::Texture t_alphabet{};
	std::vector<sf::Sprite> sp_alphabet{};

	// tiles!
	std::vector<sf::Texture> tilesets{};
	sf::Texture t_platforms{};
	sf::Texture t_platform_night{};
	sf::Texture t_platform_abandoned{};

	//save
	sf::Texture savepoint{};

	//entities
	sf::Texture t_chest{};
	sf::Texture t_shield{};

	sf::Texture t_large_animators{};
	sf::Texture t_small_animators{};

	//vfx
	sf::Texture t_huge_explosion{};
	sf::Texture t_large_explosion{};
	sf::Texture t_small_explosion{};
	sf::Texture t_wall_hit{};
	sf::Texture t_twinkle{};
	sf::Texture t_small_flash{};
	sf::Texture t_mini_flash{};
	sf::Texture t_medium_flash{};

	// load scrollingbackground sheets
	sf::Texture t_bg_opensky{};
	sf::Texture t_bg_overcast{};
	sf::Texture t_bg_dusk{};
	sf::Texture t_bg_night{};
	sf::Texture t_bg_dawn{};
	sf::Texture t_bg_sunrise{};
	sf::Texture t_bg_rosyhaze{};
	sf::Texture t_bg_slime{};
	sf::Texture t_bg_black{};
	sf::Texture t_bg_navy{};
	sf::Texture t_bg_dirt{};
	sf::Texture t_bg_gear{};
	sf::Texture t_bg_library{};
	sf::Texture t_bg_granite{};
	sf::Texture t_bg_ruins{};
	sf::Texture t_bg_crevasse{};
	sf::Texture t_bg_deep{};
	sf::Texture t_bg_grove{};

	// load the guns and bullets!
	sf::Texture t_bryns_gun{};
	sf::Texture t_bryns_gun_projectile{};
	sf::Texture t_plasmer{};
	sf::Texture t_plasmer_projectile{};
	sf::Texture t_clover{};
	sf::Texture t_clover_projectile{};
	sf::Texture t_wasp_projectile{};
	sf::Texture t_blizzard_projectile{};
	sf::Texture t_bismuth_projectile{};
	sf::Texture t_underdog_projectile{};
	sf::Texture t_electron_projectile{};
	sf::Texture t_triton_projectile{};
	sf::Texture t_willet_585_projectile{};
	sf::Texture t_quasar_projectile{};
	sf::Texture t_nova{};
	sf::Texture t_nova_projectile{};
	sf::Texture t_venom_projectile{};
	sf::Texture t_twin_projectile{};
	sf::Texture t_carise_projectile{};
	sf::Texture t_stinger_projectile{};
	sf::Texture t_tusk_projectile{};
	sf::Texture t_tomahawk{};
	sf::Texture t_tomahawk_projectile{};
	sf::Texture t_skycorps_ar_projectile{};
	sf::Texture t_grappling_hook{};
	sf::Texture t_rope{};
	sf::Texture t_hook{};
	sf::Texture t_grenade_launcher{};
	sf::Texture t_grenade{};
	sf::Texture t_minigun{};
	sf::Texture t_soda_gun{};
	sf::Texture t_soda_gun_projectile{};

	//items
	sf::Texture t_heart{};
	sf::Texture t_orb{};

	// sound effects!
	sf::SoundBuffer click_buffer{};
	sf::Sound click;
	sf::SoundBuffer sharp_click_buffer{};
	sf::Sound sharp_click;
	sf::SoundBuffer menu_shift_buffer{};
	sf::Sound menu_shift;
	sf::SoundBuffer menu_back_buffer{};
	sf::Sound menu_back;
	sf::SoundBuffer menu_next_buffer{};
	sf::Sound menu_next;

	sf::SoundBuffer arms_switch_buffer{};
	sf::Sound arms_switch;
	sf::SoundBuffer bg_shot_buffer{};
	sf::Sound bg_shot;
	sf::SoundBuffer plasmer_shot_buffer{};
	sf::Sound plasmer_shot;
	sf::SoundBuffer skycorps_ar_buffer{};
	sf::Sound skycorps_ar_shot;
	sf::SoundBuffer tomahawk_flight_buffer{};
	sf::Sound tomahawk_flight;
	sf::SoundBuffer tomahawk_catch_buffer{};
	sf::Sound tomahawk_catch;
	sf::SoundBuffer pop_mid_buffer{};
	sf::Sound pop_mid;
	sf::SoundBuffer bubble_buffer{};
	sf::Sound bubble;

	sf::SoundBuffer jump_buffer{};
	sf::Sound jump;
	sf::SoundBuffer shatter_buffer{};
	sf::Sound shatter;
	sf::SoundBuffer step_buffer{};
	sf::Sound step;
	sf::SoundBuffer landed_buffer{};
	sf::Sound landed;
	sf::SoundBuffer hurt_buffer{};
	sf::Sound hurt;
	sf::SoundBuffer player_death_buffer{};
	sf::Sound player_death;
	sf::SoundBuffer enem_hit_buffer{};
	sf::Sound enem_hit;
	sf::SoundBuffer enem_death_1_buffer{};
	sf::Sound enem_death_1;
	sf::SoundBuffer heal_buffer{};
	sf::Sound heal;
	sf::SoundBuffer orb_1_buffer{};
	sf::Sound orb_1{};
	sf::SoundBuffer orb_2_buffer{};
	sf::Sound orb_2{};
	sf::SoundBuffer orb_3_buffer{};
	sf::Sound orb_3{};
	sf::SoundBuffer orb_4_buffer{};
	sf::Sound orb_4{};

	sf::SoundBuffer tank_alert1_buffer{};
	sf::Sound tank_alert_1;
	sf::SoundBuffer tank_alert2_buffer{};
	sf::Sound tank_alert_2;
	sf::SoundBuffer tank_hurt1_buffer{};
	sf::Sound tank_hurt_1;
	sf::SoundBuffer tank_hurt2_buffer{};
	sf::Sound tank_hurt_2;
	sf::SoundBuffer tank_death_buffer{};
	sf::Sound tank_death;

	//minigus
	sf::SoundBuffer b_minigus_jump{};
	sf::Sound minigus_jump{};
	sf::SoundBuffer b_minigus_land{};
	sf::Sound minigus_land{};
	sf::SoundBuffer b_minigus_punch{};
	sf::Sound minigus_punch{};
	sf::SoundBuffer b_minigus_step{};
	sf::Sound minigus_step{};
	sf::SoundBuffer b_minigus_snap{};
	sf::Sound minigus_snap{};
	sf::SoundBuffer b_minigus_build{};
	sf::Sound minigus_build{};
	sf::SoundBuffer b_minigus_invincibility{};
	sf::Sound minigus_invincibility{};
	sf::SoundBuffer b_minigus_lose_inv{};
	sf::Sound minigus_lose_inv{};

	sf::SoundBuffer b_minigus_laugh{};
	sf::SoundBuffer b_minigus_laugh_2{};
	sf::SoundBuffer b_minigus_hurt_1{};
	sf::SoundBuffer b_minigus_hurt_2{};
	sf::SoundBuffer b_minigus_hurt_3{};
	sf::SoundBuffer b_minigus_grunt{};
	sf::SoundBuffer b_minigus_aww{};
	sf::SoundBuffer b_minigus_babyimhome{};
	sf::SoundBuffer b_minigus_deepspeak{};
	sf::SoundBuffer b_minigus_doge{};
	sf::SoundBuffer b_minigus_dontlookatme{};
	sf::SoundBuffer b_minigus_exhale{};
	sf::SoundBuffer b_minigus_getit{};
	sf::SoundBuffer b_minigus_greatidea{};
	sf::SoundBuffer b_minigus_itsagreatday{};
	sf::SoundBuffer b_minigus_long_death{};
	sf::SoundBuffer b_minigus_long_moan{};
	sf::SoundBuffer b_minigus_momma{};
	sf::SoundBuffer b_minigus_mother{};
	sf::SoundBuffer b_minigus_ok_1{};
	sf::SoundBuffer b_minigus_ok_2{};
	sf::SoundBuffer b_minigus_pizza{};
	sf::SoundBuffer b_minigus_poh{};
	sf::SoundBuffer b_minigus_quick_breath{};
	sf::SoundBuffer b_minigus_thatisverysneeze{};
	sf::SoundBuffer b_minigus_whatisit{};
	sf::SoundBuffer b_minigus_woob{};

	sf::SoundBuffer b_heavy_land{};
	sf::SoundBuffer b_delay_crash{};
	sf::SoundBuffer b_delay_high{};
	sf::SoundBuffer b_laser{};
	sf::SoundBuffer b_gun_charge{};
	sf::SoundBuffer b_soda{};

	//minigun
	sf::SoundBuffer b_minigun_neutral{};
	sf::Sound minigun_neutral{};
	sf::SoundBuffer b_minigun_charge{};
	sf::Sound minigun_charge{};
	sf::SoundBuffer b_minigun_reload{};
	sf::Sound minigun_reload{};
	sf::SoundBuffer b_minigun_firing{};
	sf::Sound minigun_firing{};

	//save/load
	sf::SoundBuffer save_buffer{};
	sf::Sound save;
	sf::SoundBuffer load_buffer{};
	sf::Sound load;
	sf::SoundBuffer soft_sparkle_high_buffer{};
	sf::Sound soft_sparkle_high;
	sf::SoundBuffer soft_sparkle_buffer{};
	sf::Sound soft_sparkle;
	sf::SoundBuffer chest_buffer{};
	sf::Sound chest{};

	// other members
	int music_vol{24};

	data::ResourceFinder finder{};

	std::unordered_map<std::string_view, sf::Texture&> texture_lookup{};
	std::unordered_map<int, sf::Texture&> background_lookup{};
	std::unordered_map<int, sf::Texture&> effect_lookup{};
	std::unordered_map<int, sf::Texture&> platform_lookup{};
	std::unordered_map<std::string_view, sf::Texture&> weapon_textures{};
	std::unordered_map<std::string_view, sf::Texture&> particle_textures{};
	std::unordered_map<std::string_view, sf::Texture&> npcs{};
	std::unordered_map<std::string_view, sf::Texture&> projectile_textures{};
};

} // namespace asset
