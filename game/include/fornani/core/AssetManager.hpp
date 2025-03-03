
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

namespace fornani::core {

class AssetManager {
  public:
	explicit AssetManager(data::ResourceFinder const& finder);

	sf::Texture& get_background(int id);
	sf::Texture& get_scenery(int style);

	sf::Texture const& get_tileset(std::string const& label);
	sf::Texture const& get_texture(std::string const& label);

	// TODO: Dear god we need to clean this up...
	// working on it :)

	sf::Texture t_null{};

	sf::Texture t_inv_test{};

	// player and NPCs!
	sf::Texture t_nani{};
	sf::Texture t_portraits{};
	sf::Texture t_bryn{};
	sf::Texture t_gobe{};
	sf::Texture t_dr_go{};
	sf::Texture t_mirin{};
	sf::Texture t_carl{};
	sf::Texture t_bit{};
	sf::Texture t_lady_nimbus{};
	sf::Texture t_justin{};

	// player palettes
	sf::Texture t_palette_nani{};
	sf::Texture t_palette_nanidiv{};
	sf::Texture t_palette_naninight{};

	// wardrobe
	sf::Texture t_wardrobe_base{};
	sf::Texture t_wardrobe_green_pants{};
	sf::Texture t_wardrobe_blue_shirt{};
	sf::Texture t_wardrobe_default_hair{};
	sf::Texture t_wardrobe_red_jeans{};
	sf::Texture t_wardrobe_chalcedony_tee{};
	sf::Texture t_wardrobe_chalcedony_skirt{};
	sf::Texture t_wardrobe_ponytail{};
	sf::Texture t_wardrobe_punk_hair{};
	sf::Texture t_wardrobe_punk_shirt{};
	sf::Texture t_wardrobe_punk_pants{};

	// items
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
	sf::Texture t_demon{};
	sf::Texture t_demon_spear{};
	sf::Texture t_demon_sword{};
	sf::Texture t_demon_shield{};
	sf::Texture t_caster{};
	sf::Texture t_caster_wand{};
	sf::Texture t_caster_scepter{};
	sf::Texture t_archer{};
	sf::Texture t_archer_bow{};
	sf::Texture t_archer_arrow{};
	sf::Texture t_beamstalk{};
	sf::Texture t_meatsquash{};
	sf::Texture t_fork_imp{};
	sf::Texture t_knife_imp{};
	sf::Texture t_imp_fork{};
	sf::Texture t_imp_knife{};

	// gui
	sf::Texture t_title{};
	sf::Texture t_title_assets{};
	sf::Texture t_file_text{};
	sf::Texture t_portrait_window{};
	sf::Texture t_controller_button_icons{};

	// inventory window
	sf::Texture t_dashboard{};

	sf::Texture t_vendor_artwork{};
	sf::Texture t_vendor_ui{};

	sf::Texture t_hud_orb_font{};
	sf::Texture t_hud_hearts{};
	sf::Texture t_hud_ammo{};
	sf::Texture t_hud_gun{};
	sf::Texture t_hud_pointer{};
	sf::Texture t_hud_shield{};
	sf::Texture t_selector{};
	sf::Texture t_indicator{};
	sf::Texture t_sticker{};

	sf::Texture t_alphabet{};

	// tiles!
	std::vector<sf::Texture> tilesets{};
	sf::Texture t_platforms{};
	sf::Texture t_platform_night{};
	sf::Texture t_platform_abandoned{};
	sf::Texture t_platform_firstwind{};
	sf::Texture t_platform_overturned{};

	// world stuff
	sf::Texture t_portals{};
	sf::Texture t_vine{};
	sf::Texture t_vine_large{};
	sf::Texture t_grass{};
	sf::Texture t_grass_large{};
	sf::Texture t_treasure_ball{};
	sf::Texture t_spawnable_platform{};
	sf::Texture t_big_spike{};

	// animators
	sf::Texture t_automatic_animators_firstwind{};

	// save
	sf::Texture savepoint{};

	// entities
	sf::Texture t_chest{};
	sf::Texture t_shield{};
	sf::Texture t_large_animators{};
	sf::Texture t_small_animators{};
	sf::Texture t_breakables{};
	sf::Texture t_pushables{};
	sf::Texture t_inspectable{};
	sf::Texture t_switches{};
	sf::Texture t_switch_blocks{};
	sf::Texture t_fader{};

	// vfx
	sf::Texture t_huge_explosion{};
	sf::Texture t_large_explosion{};
	sf::Texture t_small_explosion{};
	sf::Texture t_wall_hit{};
	sf::Texture t_twinkle{};
	sf::Texture t_wasp_effect{};
	sf::Texture t_green_beam_effect{};
	sf::Texture t_peckett_effect{};
	sf::Texture t_bg_effect{};
	sf::Texture t_small_flash{};
	sf::Texture t_mini_flash{};
	sf::Texture t_medium_flash{};
	sf::Texture t_inv_hit{};
	sf::Texture t_puff{};
	sf::Texture t_bullet_hit{};
	sf::Texture t_doublejump{};
	sf::Texture t_dust{};
	sf::Texture t_hit_flash{};

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
	sf::Texture t_bg_woods{};
	sf::Texture t_bg_canopy{};

	// scenery
	sf::Texture t_overturned_scenery{};
	sf::Texture t_firefly{};
	sf::Texture t_dragonfly{};
	sf::Texture t_fire{};
	sf::Texture t_smoke{};

	// load the guns and bullets!
	sf::Texture t_gun{};
	sf::Texture t_bryns_gun_projectile{};
	sf::Texture t_plasmer_projectile{};
	sf::Texture t_clover_projectile{};
	sf::Texture t_wasp_projectile{};
	sf::Texture t_tomahawk{};
	sf::Texture t_skycorps_ar_projectile{};
	sf::Texture t_rope{};
	sf::Texture t_hook{};
	sf::Texture t_grenade{};
	sf::Texture t_minigun{};
	sf::Texture t_soda_gun_projectile{};
	sf::Texture t_staple{};
	sf::Texture t_indie_projectile{};
	sf::Texture t_gnat_projectile{};
	sf::Texture t_energy_ball_projectile{};
	sf::Texture t_peckett_projectile{};
	sf::Texture t_arrow_projectile{};
	sf::Texture t_archer_arrow_projectile{};
	sf::Texture t_green_beam{};

	// items
	sf::Texture t_heart{};
	sf::Texture t_orb{};
	sf::Texture t_gem{};

	// other members
	int music_vol{24};

	std::unordered_map<std::string_view, sf::Texture&> texture_lookup{};
	std::unordered_map<int, sf::Texture&> scenery_lookup{};
	std::unordered_map<int, sf::Texture&> background_lookup{};
	std::unordered_map<int, sf::Texture&> effect_lookup{};
	std::unordered_map<int, sf::Texture&> platform_lookup{};
	std::unordered_map<int, sf::Texture&> animator_lookup{};
	std::unordered_map<std::string_view, sf::Texture&> particle_textures{};
	std::unordered_map<std::string_view, sf::Texture&> npcs{};
	std::unordered_map<std::string_view, sf::Texture&> projectile_textures{};

  private:
	std::unordered_map<std::string, sf::Texture> m_textures{};
	fornani::io::Logger m_logger{"core"};
};

} // namespace fornani::core
