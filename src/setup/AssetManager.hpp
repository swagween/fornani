
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

		t_palette_nani.loadFromFile(finder.resource_path + "/image/character/nani_palette_default.png");
		t_palette_nanidiv.loadFromFile(finder.resource_path + "/image/character/nani_palette_divine.png");
		t_palette_naninight.loadFromFile(finder.resource_path + "/image/character/nani_palette_night.png");

		t_bryn_test.loadFromFile(finder.resource_path + "/image/portrait/bryn_test.png");
		t_ui_test.loadFromFile(finder.resource_path + "/image/gui/ui_test.png");

		t_frdog.loadFromFile(finder.resource_path + "/image/critter/frdog.png");
		t_hulmet.loadFromFile(finder.resource_path + "/image/critter/hulmet.png");
		t_tank.loadFromFile(finder.resource_path + "/image/critter/tank.png");
		texture_lookup.insert({"frdog", t_frdog});
		texture_lookup.insert({"hulmet", t_hulmet});
		texture_lookup.insert({"tank", t_tank});

		t_ui.loadFromFile(finder.resource_path + "/image/gui/simple_console.png");
		t_hud_orb_font.loadFromFile(finder.resource_path + "/image/gui/HUD_orb_font.png");
		t_hud_hearts.loadFromFile(finder.resource_path + "/image/gui/HUD_hearts.png");
		t_hud_pointer.loadFromFile(finder.resource_path + "/image/gui/HUD_pointer.png");
		t_hud_gun_color.loadFromFile(finder.resource_path + "/image/gui/HUD_gun_color.png");
		t_hud_gun_shadow.loadFromFile(finder.resource_path + "/image/gui/HUD_gun_shadow.png");

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

		t_heart.loadFromFile(finder.resource_path + "/image/item/hearts.png");
		t_orb.loadFromFile(finder.resource_path + "/image/item/orbs.png");

		t_alphabet.loadFromFile(finder.resource_path + "/image/gui/alphabet.png");

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

		t_large_animators.loadFromFile(finder.resource_path + "/image/animators/large_animators_01.png");
		t_small_animators.loadFromFile(finder.resource_path + "/image/animators/small_animators_01.png");

		// title stuff
		t_title.loadFromFile(finder.resource_path + "/image/gui/title.png");
		t_title_assets.loadFromFile(finder.resource_path + "/image/gui/title_assets.png");
		t_file_text.loadFromFile(finder.resource_path + "/image/gui/file_text.png");

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

		enem_death_1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/enemy_death.wav");
		enem_death_1.setBuffer(enem_death_1_buffer);

		heal_buffer.loadFromFile(finder.resource_path + "/audio/sfx/heal.wav");
		heal.setBuffer(heal_buffer);
		orb_1_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_1.wav");
		orb_1.setBuffer(orb_1_buffer);
		orb_5_buffer.loadFromFile(finder.resource_path + "/audio/sfx/orb_5.wav");
		orb_5.setBuffer(orb_5_buffer);

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

		brown_noise.openFromFile(finder.resource_path + "/audio/songs/brown_noise.wav");
		clay_statue.openFromFile(finder.resource_path + "/audio/songs/clay_statue.wav");
		abandoned.openFromFile(finder.resource_path + "/audio/songs/abandoned.wav");
		three_pipes.openFromFile(finder.resource_path + "/audio/songs/three_pipes.wav");
		dusken_cove.openFromFile(finder.resource_path + "/audio/songs/dusken_cove.wav");
	}

	// declare all the textures and sprites as members of the AssetManager

	// player and NPCs!
	sf::Texture t_nani{};

	//player palettes
	sf::Texture t_palette_nani{};
	sf::Texture t_palette_nanidiv{};
	sf::Texture t_palette_naninight{};

	// portraits!
	sf::Texture t_bryn_test{};
	sf::Sprite sp_bryn_test{};
	sf::Texture t_ui_test{};
	sf::Sprite sp_ui_test{};

	// critters and bosses!
	sf::Texture t_frdog{};
	sf::Texture t_hulmet{};
	sf::Texture t_tank{};

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

	sf::Texture t_alphabet{};
	std::vector<sf::Sprite> sp_alphabet{};

	// tiles!
	std::vector<sf::Texture> tilesets{};

	//save
	sf::Texture savepoint{};
	sf::Texture t_chest{};

	sf::Texture t_large_animators{};
	sf::Texture t_small_animators{};

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

	//items
	sf::Texture t_heart{};
	sf::Texture t_orb{};

	// condense these into a 2d vector later
	std::vector<sf::Sprite> sp_tileset_provisional{};

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
	sf::SoundBuffer tomahawk_flight_buffer{};
	sf::Sound tomahawk_flight;
	sf::SoundBuffer tomahawk_catch_buffer{};
	sf::Sound tomahawk_catch;
	sf::SoundBuffer pop_mid_buffer{};
	sf::Sound pop_mid;

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
	sf::Sound orb_1;
	sf::SoundBuffer orb_5_buffer{};
	sf::Sound orb_5;

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

	// songs!
	sf::Music brown_noise{};
	sf::Music clay_statue{};
	sf::Music abandoned{};
	sf::Music three_pipes{};
	sf::Music dusken_cove{};

	// other members
	int music_vol{24};

	data::ResourceFinder finder{};

	std::unordered_map<std::string_view, sf::Texture&> texture_lookup{};
};

} // namespace asset
