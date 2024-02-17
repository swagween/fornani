
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
	AssetManager(){};

	void importTextures() {
		t_nani.loadFromFile(finder.resource_path + "/image/character/nani.png");
		t_nani_unarmed.loadFromFile(finder.resource_path + "/image/character/nani_unarmed.png");
		t_nani_dark.loadFromFile(finder.resource_path + "/image/character/nani_dark.png");
		t_nani_red.loadFromFile(finder.resource_path + "/image/character/nani_red.png");

		t_bryn_test.loadFromFile(finder.resource_path + "/image/portrait/bryn_test.png");
		t_ui_test.loadFromFile(finder.resource_path + "/image/gui/ui_test.png");

		t_frdog.loadFromFile(finder.resource_path + "/image/critter/frdog.png");
		t_hulmet.loadFromFile(finder.resource_path + "/image/critter/hulmet.png");

		t_ui.loadFromFile(finder.resource_path + "/image/gui/ui.png");
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
		t_bryns_gun_projectile.loadFromFile(finder.resource_path + "/image/weapon/bg_proj.png");
		t_plasmer_projectile.loadFromFile(finder.resource_path + "/image/weapon/plasmer_proj.png");
		t_clover_projectile.loadFromFile(finder.resource_path + "/image/weapon/clover_proj.png");
		t_nova_projectile.loadFromFile(finder.resource_path + "/image/weapon/nova_proj.png");
		t_skycorps_ar_projectile.loadFromFile(finder.resource_path + "/image/weapon/skycorps_ar_proj.png");

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
	}

	void assignSprites() {

		sp_bryn_test.setTexture(t_bryn_test);
		sp_ui_test.setTexture(t_ui_test);

		for (int j = 0; j < 2; ++j) {
			for (int i = 0; i < 26; ++i) { sp_alphabet.push_back(sf::Sprite(t_alphabet, sf::IntRect({i * 12, j * 12}, {12, 12}))); }
		}

		// gun sprite order convention:          right : left : up_right : down_right : down_left : up_left
		// projectile sprite order convention:   right : left : up : down

		// guns and bullets!! (gotta do these all by hand)
		int bg_w = 18;
		int bg_h = 8;
		int bg_total_w = 36;
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({0, 0}, {bg_w, bg_h})));
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_w, 0}, {bg_w, bg_h})));
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({0, bg_h}, {bg_h, bg_w})));
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_h, bg_h}, {bg_h, bg_w})));
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_h * 2, bg_h}, {bg_h, bg_w})));
		sp_bryns_gun.push_back(sf::Sprite(t_bryns_gun, sf::IntRect({bg_total_w - bg_h, bg_h}, {bg_h, bg_w})));

		int plas_w = 22;
		int plas_h = 10;
		int plas_total_w = 44;
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({0, 0}, {plas_w, plas_h})));
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_w, 0}, {plas_w, plas_h})));
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({0, plas_h}, {plas_h, plas_w})));
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_h, plas_h}, {plas_h, plas_w})));
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_h * 2, plas_h}, {plas_h, plas_w})));
		sp_plasmer.push_back(sf::Sprite(t_plasmer, sf::IntRect({plas_total_w - plas_h, plas_h}, {plas_h, plas_w})));

		// clover
		int clov_w = 18;
		int clov_h = 10;
		int clov_total_w = 40;
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({0, 0}, {clov_w, clov_h})));
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_w, 0}, {clov_w, clov_h})));
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({0, clov_h}, {clov_h, clov_w})));
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_h, clov_h}, {clov_h, clov_w})));
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_h * 2, clov_h}, {clov_h, clov_w})));
		sp_clover.push_back(sf::Sprite(t_clover, sf::IntRect({clov_total_w - clov_h, clov_h}, {clov_h, clov_w})));

		int bg_proj_h = 12;
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({0, 0}, {4, bg_proj_h})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({4, 0}, {12, bg_proj_h})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({16, 0}, {14, bg_proj_h})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({30, 0}, {22, bg_proj_h})));

		/*int bg_proj_w = 22; int bg_proj_h = 12;
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({0, 0},                        {bg_proj_w, bg_proj_h})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({0, bg_proj_h},                {bg_proj_w, bg_proj_h})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({bg_proj_w, 0},                {bg_proj_h, bg_proj_w})));
		sp_bryns_gun_projectile.push_back(sf::Sprite(t_bryns_gun_projectile, sf::IntRect({bg_proj_w + bg_proj_w, 0},    {bg_proj_h, bg_proj_w})));*/

		int plas_proj_w = 20;
		int plas_proj_h = 10;
		sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({0, 0}, {plas_proj_w, plas_proj_h})));
		sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({0, plas_proj_h}, {plas_proj_w, plas_proj_h})));
		sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({plas_proj_w, 0}, {plas_proj_h, plas_proj_w})));
		sp_plasmer_projectile.push_back(sf::Sprite(t_plasmer_projectile, sf::IntRect({plas_proj_w + plas_proj_w, 0}, {plas_proj_h, plas_proj_w})));

		int cproj1_w = 6;
		int cproj1_h = 6;
		int cproj2_w = 8;
		int cproj2_h = 6;
		int cproj3_w = 8;
		int cproj3_h = 8;
		int cproj4_w = 14;
		int cproj4_h = 12;
		int cproj5_w = 8;
		int cproj5_h = 10;
		int cproj_total_height = 18;
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, 0}, {cproj1_w, cproj1_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, 0}, {cproj2_w, cproj1_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, 0}, {cproj3_w, cproj3_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h}, {cproj4_w, cproj4_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h}, {cproj5_w, cproj5_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj_total_height}, {cproj1_w, cproj1_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height}, {cproj2_w, cproj2_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height}, {cproj3_w, cproj3_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height}, {cproj4_w, cproj4_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height}, {cproj5_w, cproj5_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, 0}, {cproj1_w, cproj1_h})));

		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height * 2}, {cproj2_w, cproj1_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height * 2}, {cproj3_w, cproj3_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height * 2}, {cproj4_w, cproj4_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height * 2}, {cproj5_w, cproj5_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj_total_height * 3}, {cproj1_w, cproj1_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w, cproj_total_height * 3}, {cproj2_w, cproj2_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj1_w + cproj2_w, cproj_total_height * 3}, {cproj3_w, cproj3_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({0, cproj1_h + cproj_total_height * 3}, {cproj4_w, cproj4_h})));
		sp_clover_projectile.push_back(sf::Sprite(t_clover_projectile, sf::IntRect({cproj4_w, cproj3_h + cproj_total_height * 3}, {cproj5_w, cproj5_h})));

		// assign all the other sprites...
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

		brown_noise.openFromFile(finder.resource_path + "/audio/songs/brown_noise.wav");
		clay_statue.openFromFile(finder.resource_path + "/audio/songs/clay_statue.wav");
		abandoned.openFromFile(finder.resource_path + "/audio/songs/abandoned.wav");
		three_pipes.openFromFile(finder.resource_path + "/audio/songs/three_pipes.wav");
		dusken_cove.openFromFile(finder.resource_path + "/audio/songs/dusken_cove.wav");
	}

	// declare all the textures and sprites as members of the AssetManager

	// player and NPCs!
	sf::Texture t_nani{};
	sf::Texture t_nani_unarmed{};
	sf::Texture t_nani_dark{};
	sf::Texture t_nani_red{};

	// portraits!
	sf::Texture t_bryn_test{};
	sf::Sprite sp_bryn_test{};
	sf::Texture t_ui_test{};
	sf::Sprite sp_ui_test{};

	// critters and bosses!
	sf::Texture t_frdog{};
	sf::Texture t_hulmet{};

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
	sf::Texture t_tomahawk_projectile{};
	sf::Texture t_skycorps_ar_projectile{};

	std::vector<sf::Sprite> sp_bryns_gun{};
	std::vector<sf::Sprite> sp_bryns_gun_projectile{};
	std::vector<sf::Sprite> sp_plasmer{};
	std::vector<sf::Sprite> sp_plasmer_projectile{};
	std::vector<sf::Sprite> sp_clover{};
	std::vector<sf::Sprite> sp_clover_projectile{};

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

	// songs!
	sf::Music brown_noise{};
	sf::Music clay_statue{};
	sf::Music abandoned{};
	sf::Music three_pipes{};
	sf::Music dusken_cove{};

	// other members
	int music_vol{24};

	data::ResourceFinder finder{};
};

} // namespace asset
