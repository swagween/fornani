
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
	~AssetManager() {}

	void import_textures();
	void load_audio();

	// player and NPCs!
	sf::Texture t_nani{};
	sf::Texture t_portraits{};
	sf::Texture t_bryn{};
	sf::Texture t_gobe{};
	sf::Texture t_dr_go{};
	sf::Texture t_mirin{};
	sf::Texture t_carl{};
	sf::Texture t_bit{};

	//player palettes
	sf::Texture t_palette_nani{};
	sf::Texture t_palette_nanidiv{};
	sf::Texture t_palette_naninight{};

	//items
	sf::Texture t_items{};
	sf::Texture t_guns{};

	// critters and bosses!
	sf::Texture t_frdog{};
	sf::Texture t_hulmet{};
	sf::Texture t_tank{};
	sf::Texture t_thug{};
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
	sf::Texture t_indicator{};

	sf::Texture t_alphabet{};
	std::vector<sf::Sprite> sp_alphabet{};

	// tiles!
	std::vector<sf::Texture> tilesets{};
	sf::Texture t_platforms{};
	sf::Texture t_platform_night{};
	sf::Texture t_platform_abandoned{};
	sf::Texture t_platform_firstwind{};

	//animators
	sf::Texture t_automatic_animators_firstwind{};

	//save
	sf::Texture savepoint{};

	//entities
	sf::Texture t_chest{};
	sf::Texture t_shield{};

	sf::Texture t_large_animators{};
	sf::Texture t_small_animators{};
	sf::Texture t_breakables{};
	sf::Texture t_inspectable{};
	sf::Texture t_switches{};
	sf::Texture t_switch_blocks{};
	sf::Texture t_fader{};

	//vfx
	sf::Texture t_huge_explosion{};
	sf::Texture t_large_explosion{};
	sf::Texture t_small_explosion{};
	sf::Texture t_wall_hit{};
	sf::Texture t_twinkle{};
	sf::Texture t_small_flash{};
	sf::Texture t_mini_flash{};
	sf::Texture t_medium_flash{};
	sf::Texture t_inv_hit{};
	sf::Texture t_puff{};
	sf::Texture t_bullet_hit{};

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
	sf::Texture t_staple_gun{};
	sf::Texture t_staple{};
	sf::Texture t_indie{};
	sf::Texture t_indie_projectile{};
	sf::Texture t_gnat{};
	sf::Texture t_gnat_projectile{};

	//items
	sf::Texture t_heart{};
	sf::Texture t_orb{};

	sf::SoundBuffer player_death_buffer{};
	sf::SoundBuffer enem_hit_buffer{};
	sf::SoundBuffer enem_death_1_buffer{};
	sf::SoundBuffer heal_buffer{};
	sf::SoundBuffer orb_1_buffer{};
	sf::SoundBuffer orb_2_buffer{};
	sf::SoundBuffer orb_3_buffer{};
	sf::SoundBuffer orb_4_buffer{};

	// sound effects!
	sf::SoundBuffer click_buffer{};
	sf::SoundBuffer sharp_click_buffer{};
	sf::SoundBuffer menu_shift_buffer{};
	sf::SoundBuffer menu_back_buffer{};
	sf::SoundBuffer menu_next_buffer{};
	sf::SoundBuffer menu_open_buffer{};

	sf::SoundBuffer arms_switch_buffer{};
	sf::SoundBuffer bg_shot_buffer{};
	sf::SoundBuffer plasmer_shot_buffer{};
	sf::SoundBuffer skycorps_ar_buffer{};
	sf::SoundBuffer tomahawk_flight_buffer{};
	sf::SoundBuffer tomahawk_catch_buffer{};
	sf::SoundBuffer pop_mid_buffer{};
	sf::SoundBuffer bubble_buffer{};
	sf::SoundBuffer b_nova{};
	sf::SoundBuffer b_staple{};
	sf::SoundBuffer b_gnat{};

	sf::SoundBuffer jump_buffer{};
	sf::SoundBuffer shatter_buffer{};
	sf::SoundBuffer step_buffer{};
	sf::SoundBuffer landed_buffer{};
	sf::SoundBuffer hurt_buffer{};

	sf::SoundBuffer tank_alert1_buffer{};
	sf::SoundBuffer tank_alert2_buffer{};
	sf::SoundBuffer tank_hurt1_buffer{};
	sf::SoundBuffer tank_hurt2_buffer{};
	sf::SoundBuffer tank_death_buffer{};

	//minigus
	sf::SoundBuffer b_minigus_invincibility{};
	sf::SoundBuffer b_minigus_lose_inv{};
	sf::SoundBuffer b_minigus_build{};
	sf::SoundBuffer b_minigus_punch{};
	sf::SoundBuffer b_minigus_jump{};
	sf::SoundBuffer b_minigus_step{};
	sf::SoundBuffer b_minigus_land{};

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
	sf::SoundBuffer b_breakable_hit{};

	//minigun
	sf::SoundBuffer b_minigun_neutral{};
	sf::SoundBuffer b_minigun_charge{};
	sf::SoundBuffer b_minigun_reload{};
	sf::SoundBuffer b_minigun_firing{};

	sf::SoundBuffer b_enemy_hit_low{};
	sf::SoundBuffer b_enemy_hit_medium{};
	sf::SoundBuffer b_enemy_hit_high{};
	sf::SoundBuffer b_enemy_hit_squeak{};
	sf::SoundBuffer b_enemy_hit_inv{};

	//save/load
	sf::SoundBuffer save_buffer{};
	sf::SoundBuffer load_buffer{};
	sf::SoundBuffer soft_sparkle_high_buffer{};
	sf::SoundBuffer soft_sparkle_buffer{};
	sf::SoundBuffer chest_buffer{};

	sf::Sound landed{};
	sf::Sound step{};
	sf::Sound shatter{};
	sf::Sound jump{};
	sf::Sound arms_switch{};
	sf::Sound bg_shot{};
	sf::Sound plasmer_shot{};
	sf::Sound skycorps_ar_shot{};
	sf::Sound tomahawk_flight{};
	sf::Sound tomahawk_catch{};
	sf::Sound pop_mid{};
	sf::Sound bubble{};
	sf::Sound nova_shot{};
	sf::Sound staple{};
	sf::Sound gnat{};

	sf::Sound minigun_neutral{};
	sf::Sound minigun_charge{};
	sf::Sound minigun_reload{};
	sf::Sound minigun_firing{};
	sf::Sound chest{};
	sf::Sound save{};
	sf::Sound load{};
	sf::Sound soft_sparkle_high{};
	sf::Sound soft_sparkle{};
	sf::Sound click{};
	sf::Sound tank_alert_1{};
	sf::Sound tank_alert_2{};
	sf::Sound tank_hurt_1{};
	sf::Sound tank_hurt_2{};
	sf::Sound tank_death{};
	sf::Sound hurt{};
	sf::Sound player_death{};
	sf::Sound enem_hit{};
	sf::Sound enem_death_1{};
	sf::Sound heal{};
	sf::Sound orb_1{};
	sf::Sound orb_2{};
	sf::Sound orb_3{};
	sf::Sound orb_4{};
	sf::Sound menu_next{};
	sf::Sound menu_back{};
	sf::Sound sharp_click{};
	sf::Sound menu_shift{};
	sf::Sound menu_open{};

	sf::Sound breakable_hit{};
	sf::Sound breakable_shatter{};

	// other members
	int music_vol{24};

	data::ResourceFinder finder{};

	std::unordered_map<std::string_view, sf::Texture&> texture_lookup{};
	std::unordered_map<int, sf::Texture&> background_lookup{};
	std::unordered_map<int, sf::Texture&> effect_lookup{};
	std::unordered_map<int, sf::Texture&> platform_lookup{};
	std::unordered_map<int, sf::Texture&> animator_lookup{};
	std::unordered_map<std::string_view, sf::Texture&> weapon_textures{};
	std::unordered_map<std::string_view, sf::Texture&> particle_textures{};
	std::unordered_map<std::string_view, sf::Texture&> npcs{};
	std::unordered_map<std::string_view, sf::Texture&> projectile_textures{};
};

} // namespace asset
