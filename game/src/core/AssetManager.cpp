#include "fornani/core/AssetManager.hpp"

namespace fornani::core {

AssetManager::AssetManager(data::ResourceFinder const& finder) {

	// doing stuff the new way will be done up here. the goal is to slowly but surely get rid of the bottom

	namespace fs = std::filesystem;
	auto p_folder{fs::path{"image"}};
	auto p_gui{p_folder / fs::path{"gui"}};
	auto p_app{p_folder / fs::path{"app"}};
	auto image_dir = fs::path{finder.resource_path()} / "image";

	// null texture for lookup failures
	if (!t_null.loadFromFile(finder.resource_path() / p_app / fs::path{"null.png"})) { NANI_LOG_WARN(m_logger, "Failed to load texture."); };

	// recursively load textures
	for (auto const& image_genre : fs::recursive_directory_iterator(image_dir)) {
		if (!image_genre.is_directory()) { continue; }
		for (auto const& image : fs::recursive_directory_iterator(image_genre)) {
			if (image.path().extension() != ".png") { continue; }
			auto image_str = image.path().filename().string();
			m_textures.insert({image_str.substr(0, image_str.find('.')), sf::Texture{image.path()}});
		}
	}

	/////////////////////// old stuff below here, let's try to destroy it //////////////////////////////////////

	// TODO: This manner of loading assets is extremely cumbersome.
	//		 We honestly should move this into an unordered_map or something.

	if (!t_dashboard.loadFromFile(finder.resource_path() + "/image/gui/dashboard.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/dashboard.png] from file.", finder.resource_path());

	if (!t_nani.loadFromFile(finder.resource_path() + "/image/character/nani.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/nani.png] from file.", finder.resource_path());
	if (!t_portraits.loadFromFile(finder.resource_path() + "/image/character/portraits.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/portraits.png] from file.", finder.resource_path());
	if (!t_bryn.loadFromFile(finder.resource_path() + "/image/character/bryn.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/bryn.png] from file.", finder.resource_path());
	if (!t_gobe.loadFromFile(finder.resource_path() + "/image/character/gobe.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/gobe.png] from file.", finder.resource_path());
	if (!t_dr_go.loadFromFile(finder.resource_path() + "/image/character/dr_go.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/dr_go.png] from file.", finder.resource_path());
	if (!t_carl.loadFromFile(finder.resource_path() + "/image/character/carl.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/carl.png] from file.", finder.resource_path());
	if (!t_mirin.loadFromFile(finder.resource_path() + "/image/character/mirin.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/mirin.png] from file.", finder.resource_path());
	if (!t_bit.loadFromFile(finder.resource_path() + "/image/character/bit.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/bit.png] from file.", finder.resource_path());
	if (!t_lady_nimbus.loadFromFile(finder.resource_path() + "/image/character/lady_nimbus.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/lady_nimbus.png] from file.", finder.resource_path());
	if (!t_justin.loadFromFile(finder.resource_path() + "/image/character/justin.png")) NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/justin.png] from file.", finder.resource_path());
	npcs.insert({"bryn", t_bryn});
	npcs.insert({"gobe", t_gobe});
	npcs.insert({"dr_go", t_dr_go});
	npcs.insert({"carl", t_carl});
	npcs.insert({"mirin", t_mirin});
	npcs.insert({"bit", t_bit});
	npcs.insert({"lady_nimbus", t_lady_nimbus});
	npcs.insert({"justin", t_justin});

	if (!t_vendor_artwork.loadFromFile(finder.resource_path() + "/image/gui/vendor_artwork.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/vendor_artwork.png] from file.", finder.resource_path()); }
	if (!t_vendor_ui.loadFromFile(finder.resource_path() + "/image/gui/vendor_interface.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/vendor_interface.png] from file.", finder.resource_path()); }
	if (!t_inv_test.loadFromFile(finder.resource_path() + "/image/gui/inv_test.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/inv_test.png] from file.", finder.resource_path()); }

	if (!t_palette_nani.loadFromFile(finder.resource_path() + "/image/character/nani_palette_default.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/nani_palette_default.png] from file.", finder.resource_path());
	}
	if (!t_palette_nanidiv.loadFromFile(finder.resource_path() + "/image/character/nani_palette_divine.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/nani_palette_divine.png] from file.", finder.resource_path());
	}
	if (!t_palette_naninight.loadFromFile(finder.resource_path() + "/image/character/nani_palette_night.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/character/nani_palette_night.png] from file.", finder.resource_path());
	}

	if (!t_wardrobe_base.loadFromFile(finder.resource_path() + "/image/wardrobe/base.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/base.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_green_pants.loadFromFile(finder.resource_path() + "/image/wardrobe/green_pants.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/green_pants.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_blue_shirt.loadFromFile(finder.resource_path() + "/image/wardrobe/blue_shirt.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/blue_shirt.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_default_hair.loadFromFile(finder.resource_path() + "/image/wardrobe/default_hair.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/default_hair.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_red_jeans.loadFromFile(finder.resource_path() + "/image/wardrobe/red_jeans.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/red_jeans.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_chalcedony_tee.loadFromFile(finder.resource_path() + "/image/wardrobe/chalcedony_tee.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/chalcedony_tee.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_chalcedony_skirt.loadFromFile(finder.resource_path() + "/image/wardrobe/chalcedony_skirt.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/chalcedony_skirt.png] from file.", finder.resource_path());
	}
	if (!t_wardrobe_ponytail.loadFromFile(finder.resource_path() + "/image/wardrobe/ponytail.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/ponytail.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_punk_hair.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_hair.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/punk_hair.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_punk_shirt.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_shirt.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/punk_shirt.png] from file.", finder.resource_path()); }
	if (!t_wardrobe_punk_pants.loadFromFile(finder.resource_path() + "/image/wardrobe/punk_pants.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/wardrobe/punk_pants.png] from file.", finder.resource_path()); }

	if (!t_frdog.loadFromFile(finder.resource_path() + "/image/critter/frdog.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/frdog.png] from file.", finder.resource_path()); }
	if (!t_hulmet.loadFromFile(finder.resource_path() + "/image/critter/hulmet.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/hulmet.png] from file.", finder.resource_path()); }
	if (!t_tank.loadFromFile(finder.resource_path() + "/image/critter/tank.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/tank.png] from file.", finder.resource_path()); }
	if (!t_thug.loadFromFile(finder.resource_path() + "/image/critter/thug.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/thug.png] from file.", finder.resource_path()); }
	if (!t_eyebot.loadFromFile(finder.resource_path() + "/image/critter/eyebot.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/eyebot.png] from file.", finder.resource_path()); }
	if (!t_eyebit.loadFromFile(finder.resource_path() + "/image/critter/eyebit.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/eyebit.png] from file.", finder.resource_path()); }
	if (!t_minigus.loadFromFile(finder.resource_path() + "/image/boss/minigus.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/boss/minigus.png] from file.", finder.resource_path()); }
	if (!t_minigus_inv.loadFromFile(finder.resource_path() + "/image/boss/minigus_inv.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/boss/minigus_inv.png] from file.", finder.resource_path()); }
	if (!t_minigus_red.loadFromFile(finder.resource_path() + "/image/boss/minigus_red.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/boss/minigus_red.png] from file.", finder.resource_path()); }
	if (!t_minigus_blue.loadFromFile(finder.resource_path() + "/image/boss/minigus_blue.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/boss/minigus_blue.png] from file.", finder.resource_path()); }
	if (!t_demon.loadFromFile(finder.resource_path() + "/image/critter/demon.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/demon.png] from file.", finder.resource_path()); }
	if (!t_demon_shield.loadFromFile(finder.resource_path() + "/image/critter/demon_shield.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/demon_shield.png] from file.", finder.resource_path()); }
	if (!t_demon_spear.loadFromFile(finder.resource_path() + "/image/critter/demon_spear.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/demon_spear.png] from file.", finder.resource_path()); }
	if (!t_demon_sword.loadFromFile(finder.resource_path() + "/image/critter/demon_sword.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/demon_sword.png] from file.", finder.resource_path()); }
	if (!t_caster.loadFromFile(finder.resource_path() + "/image/critter/caster.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/caster.png] from file.", finder.resource_path()); }
	if (!t_caster_scepter.loadFromFile(finder.resource_path() + "/image/critter/caster_scepter.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/caster_scepter.png] from file.", finder.resource_path()); }
	if (!t_caster_wand.loadFromFile(finder.resource_path() + "/image/critter/caster_wand.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/caster_wand.png] from file.", finder.resource_path()); }
	if (!t_archer.loadFromFile(finder.resource_path() + "/image/critter/archer.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/archer.png] from file.", finder.resource_path()); }
	if (!t_archer_bow.loadFromFile(finder.resource_path() + "/image/critter/archer_bow.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/archer_bow.png] from file.", finder.resource_path()); }
	if (!t_archer_arrow.loadFromFile(finder.resource_path() + "/image/weapon/arrow.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/arrow.png] from file.", finder.resource_path()); }
	if (!t_beamstalk.loadFromFile(finder.resource_path() + "/image/critter/beamstalk.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/beamstalk.png] from file.", finder.resource_path()); }
	if (!t_meatsquash.loadFromFile(finder.resource_path() + "/image/critter/meatsquash.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/meatsquash.png] from file.", finder.resource_path()); }
	if (!t_fork_imp.loadFromFile(finder.resource_path() + "/image/critter/fork_imp.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/fork_imp.png] from file.", finder.resource_path()); }
	if (!t_knife_imp.loadFromFile(finder.resource_path() + "/image/critter/knife_imp.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/knife_imp.png] from file.", finder.resource_path()); }
	if (!t_imp_fork.loadFromFile(finder.resource_path() + "/image/critter/imp_fork.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/imp_fork.png] from file.", finder.resource_path()); }
	if (!t_imp_knife.loadFromFile(finder.resource_path() + "/image/critter/imp_knife.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/critter/imp_knife.png] from file.", finder.resource_path()); }

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
	texture_lookup.insert({"imp", t_fork_imp});

	if (!t_hud_orb_font.loadFromFile(finder.resource_path() + "/image/gui/HUD_orb_font.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_orb_font.png] from file.", finder.resource_path()); }
	if (!t_hud_hearts.loadFromFile(finder.resource_path() + "/image/gui/HUD_hearts.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_hearts.png] from file.", finder.resource_path()); }
	if (!t_hud_ammo.loadFromFile(finder.resource_path() + "/image/gui/HUD_ammo.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_ammo.png] from file.", finder.resource_path()); }
	if (!t_hud_pointer.loadFromFile(finder.resource_path() + "/image/gui/HUD_pointer.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_pointer.png] from file.", finder.resource_path()); }
	if (!t_hud_gun.loadFromFile(finder.resource_path() + "/image/gui/HUD_gun.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_gun.png] from file.", finder.resource_path()); }
	if (!t_hud_shield.loadFromFile(finder.resource_path() + "/image/gui/HUD_shield.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/HUD_shield.png] from file.", finder.resource_path()); }
	if (!t_selector.loadFromFile(finder.resource_path() + "/image/gui/selector.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/selector.png] from file.", finder.resource_path()); }
	if (!t_sticker.loadFromFile(finder.resource_path() + "/image/gui/sticker.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/sticker.png] from file.", finder.resource_path()); }
	if (!t_controller_button_icons.loadFromFile(finder.resource_path() + "/image/gui/controller_button_icons.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/controller_button_icons.png] from file.", finder.resource_path());
	}

	// guns and bullets!
	if (!t_gun.loadFromFile(finder.resource_path() + "/image/weapon/gun.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/gun.png] from file.", finder.resource_path()); }
	if (!t_bryns_gun_projectile.loadFromFile(finder.resource_path() + "/image/weapon/bg_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/bg_proj.png] from file.", finder.resource_path()); }
	if (!t_plasmer_projectile.loadFromFile(finder.resource_path() + "/image/weapon/plasmer_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/plasmer_proj.png] from file.", finder.resource_path()); }
	if (!t_clover_projectile.loadFromFile(finder.resource_path() + "/image/weapon/clover_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/clover_proj.png] from file.", finder.resource_path()); }
	if (!t_skycorps_ar_projectile.loadFromFile(finder.resource_path() + "/image/weapon/skycorps_ar_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/skycorps_ar_proj.png] from file.", finder.resource_path()); }
	if (!t_tomahawk.loadFromFile(finder.resource_path() + "/image/weapon/tomahawk.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/tomahawk.png] from file.", finder.resource_path()); }
	if (!t_wasp_projectile.loadFromFile(finder.resource_path() + "/image/weapon/wasp_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/wasp_proj.png] from file.", finder.resource_path()); }
	if (!t_rope.loadFromFile(finder.resource_path() + "/image/weapon/rope.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/rope.png] from file.", finder.resource_path()); }
	if (!t_hook.loadFromFile(finder.resource_path() + "/image/weapon/hook.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/hook.png] from file.", finder.resource_path()); }
	if (!t_grenade.loadFromFile(finder.resource_path() + "/image/weapon/grenade.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/grenade.png] from file.", finder.resource_path()); }
	if (!t_minigun.loadFromFile(finder.resource_path() + "/image/boss/minigun.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/boss/minigun.png] from file.", finder.resource_path()); }
	if (!t_soda_gun_projectile.loadFromFile(finder.resource_path() + "/image/weapon/soda.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/soda.png] from file.", finder.resource_path()); }
	if (!t_staple.loadFromFile(finder.resource_path() + "/image/weapon/staple.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/staple.png] from file.", finder.resource_path()); }
	if (!t_indie_projectile.loadFromFile(finder.resource_path() + "/image/weapon/indie_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/indie_proj.png] from file.", finder.resource_path()); }
	if (!t_gnat_projectile.loadFromFile(finder.resource_path() + "/image/weapon/gnat_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/gnat_proj.png] from file.", finder.resource_path()); }
	if (!t_energy_ball_projectile.loadFromFile(finder.resource_path() + "/image/weapon/energy_ball.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/energy_ball.png] from file.", finder.resource_path()); }
	if (!t_peckett_projectile.loadFromFile(finder.resource_path() + "/image/weapon/peckett_710_proj.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/peckett_710_proj.png] from file.", finder.resource_path()); }
	if (!t_arrow_projectile.loadFromFile(finder.resource_path() + "/image/weapon/arrow.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/arrow.png] from file.", finder.resource_path()); }
	if (!t_archer_arrow_projectile.loadFromFile(finder.resource_path() + "/image/weapon/archer_arrow.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/archer_arrow.png] from file.", finder.resource_path()); }
	if (!t_green_beam.loadFromFile(finder.resource_path() + "/image/weapon/green_beam.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/weapon/green_beam.png] from file.", finder.resource_path()); }

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

	if (!t_items.loadFromFile(finder.resource_path() + "/image/item/items.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/item/items.png] from file.", finder.resource_path()); }
	if (!t_guns.loadFromFile(finder.resource_path() + "/image/item/guns.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/item/guns.png] from file.", finder.resource_path()); }
	if (!t_shield.loadFromFile(finder.resource_path() + "/image/entity/shield.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/shield.png] from file.", finder.resource_path()); }

	if (!t_heart.loadFromFile(finder.resource_path() + "/image/item/hearts.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/item/hearts.png] from file.", finder.resource_path()); }
	if (!t_orb.loadFromFile(finder.resource_path() + "/image/item/orbs.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/item/orbs.png] from file.", finder.resource_path()); }
	if (!t_gem.loadFromFile(finder.resource_path() + "/image/item/gems.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/item/gems.png] from file.", finder.resource_path()); }

	if (!t_fader.loadFromFile(finder.resource_path() + "/image/vfx/fader.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/fader.png] from file.", finder.resource_path()); }
	if (!t_firefly.loadFromFile(finder.resource_path() + "/image/vfx/firefly.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/firefly.png] from file.", finder.resource_path()); }
	if (!t_dragonfly.loadFromFile(finder.resource_path() + "/image/vfx/dragonfly.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/dragonfly.png] from file.", finder.resource_path()); }
	if (!t_fire.loadFromFile(finder.resource_path() + "/image/vfx/fire.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/fire.png] from file.", finder.resource_path()); }

	if (!t_twinkle.loadFromFile(finder.resource_path() + "/image/vfx/twinkle.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/twinkle.png] from file.", finder.resource_path()); }
	if (!t_wasp_effect.loadFromFile(finder.resource_path() + "/image/vfx/wasp_effect.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/wasp_effect.png] from file.", finder.resource_path()); }
	if (!t_peckett_effect.loadFromFile(finder.resource_path() + "/image/vfx/peckett_effect.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/peckett_effect.png] from file.", finder.resource_path()); }
	if (!t_bg_effect.loadFromFile(finder.resource_path() + "/image/vfx/bryns_gun_effect.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/bryns_gun_effect.png] from file.", finder.resource_path()); }
	if (!t_small_flash.loadFromFile(finder.resource_path() + "/image/vfx/small_flash.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/small_flash.png] from file.", finder.resource_path()); }
	if (!t_hit_flash.loadFromFile(finder.resource_path() + "/image/vfx/hit_flash.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/hit_flash.png] from file.", finder.resource_path()); }
	if (!t_smoke.loadFromFile(finder.resource_path() + "/image/vfx/smoke.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/smoke.png] from file.", finder.resource_path()); }
	if (!t_green_beam_effect.loadFromFile(finder.resource_path() + "/image/vfx/green_beam_effect.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/vfx/green_beam_effect.png] from file.", finder.resource_path()); }

	particle_textures.insert({"twinkle", t_twinkle});
	particle_textures.insert({"wasp", t_wasp_effect});
	particle_textures.insert({"smoke", t_smoke});
	particle_textures.insert({"green_beam", t_green_beam_effect});
	particle_textures.insert({"peckett", t_peckett_effect});
	particle_textures.insert({"bryns_gun_smoke", t_bg_effect});

	if (!t_alphabet.loadFromFile(finder.resource_path() + "/image/gui/alphabet.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/alphabet.png] from file.", finder.resource_path()); }
	if (!t_portrait_window.loadFromFile(finder.resource_path() + "/image/gui/portrait_window.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/portrait_window.png] from file.", finder.resource_path()); }

	if (!t_platforms.loadFromFile(finder.resource_path() + "/image/tile/platforms.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/platforms.png] from file.", finder.resource_path()); }
	if (!t_platform_night.loadFromFile(finder.resource_path() + "/image/tile/platform_night.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/platform_night.png] from file.", finder.resource_path()); }
	if (!t_platform_abandoned.loadFromFile(finder.resource_path() + "/image/tile/platform_abandoned.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/platform_abandoned.png] from file.", finder.resource_path()); }
	if (!t_platform_firstwind.loadFromFile(finder.resource_path() + "/image/tile/platform_firstwind.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/platform_firstwind.png] from file.", finder.resource_path()); }
	if (!t_platform_overturned.loadFromFile(finder.resource_path() + "/image/tile/platform_overturned.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/platform_overturned.png] from file.", finder.resource_path()); }
	platform_lookup.insert({0, t_platform_night});
	platform_lookup.insert({1, t_platform_abandoned});
	platform_lookup.insert({2, t_platform_firstwind});
	platform_lookup.insert({3, t_platform_overturned});
	if (!t_automatic_animators_firstwind.loadFromFile(finder.resource_path() + "/image/tile/automatic_animators_firstwind.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/automatic_animators_firstwind.png] from file.", finder.resource_path());
	}
	animator_lookup.insert({0, t_automatic_animators_firstwind});

	if (!t_breakables.loadFromFile(finder.resource_path() + "/image/tile/breakables.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/breakables.png] from file.", finder.resource_path()); }
	if (!t_pushables.loadFromFile(finder.resource_path() + "/image/tile/pushables.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/pushables.png] from file.", finder.resource_path()); }
	if (!t_inspectable.loadFromFile(finder.resource_path() + "/image/entity/inspectable.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/inspectable.png] from file.", finder.resource_path()); }
	if (!t_switches.loadFromFile(finder.resource_path() + "/image/tile/switches.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/switches.png] from file.", finder.resource_path()); }
	if (!t_switch_blocks.loadFromFile(finder.resource_path() + "/image/tile/switch_blocks.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/switch_blocks.png] from file.", finder.resource_path()); }
	if (!t_indicator.loadFromFile(finder.resource_path() + "/image/entity/arrow.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/arrow.png] from file.", finder.resource_path()); }
	if (!t_portals.loadFromFile(finder.resource_path() + "/image/tile/portals.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/portals.png] from file.", finder.resource_path()); }
	if (!t_vine.loadFromFile(finder.resource_path() + "/image/tile/vine.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/vine.png] from file.", finder.resource_path()); }
	if (!t_vine_large.loadFromFile(finder.resource_path() + "/image/tile/vine_large.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/vine_large.png] from file.", finder.resource_path()); }
	if (!t_grass.loadFromFile(finder.resource_path() + "/image/tile/grass.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/grass.png] from file.", finder.resource_path()); }
	if (!t_big_spike.loadFromFile(finder.resource_path() + "/image/tile/big_spike.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/tile/big_spike.png] from file.", finder.resource_path()); }

	// backgrounds
	if (!t_bg_dusk.loadFromFile(finder.resource_path() + "/image/background/dusk.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/dusk.png] from file.", finder.resource_path()); }
	if (!t_bg_opensky.loadFromFile(finder.resource_path() + "/image/background/opensky.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/opensky.png] from file.", finder.resource_path()); }
	if (!t_bg_overcast.loadFromFile(finder.resource_path() + "/image/background/overcast.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/overcast.png] from file.", finder.resource_path()); }
	if (!t_bg_night.loadFromFile(finder.resource_path() + "/image/background/night.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/night.png] from file.", finder.resource_path()); }
	if (!t_bg_dawn.loadFromFile(finder.resource_path() + "/image/background/dawn.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/dawn.png] from file.", finder.resource_path()); }
	if (!t_bg_sunrise.loadFromFile(finder.resource_path() + "/image/background/sunrise.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/sunrise.png] from file.", finder.resource_path()); }
	if (!t_bg_rosyhaze.loadFromFile(finder.resource_path() + "/image/background/rosyhaze.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/rosyhaze.png] from file.", finder.resource_path()); }
	if (!t_bg_slime.loadFromFile(finder.resource_path() + "/image/background/slime.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/slime.png] from file.", finder.resource_path()); }
	if (!t_bg_dirt.loadFromFile(finder.resource_path() + "/image/background/dirt.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/dirt.png] from file.", finder.resource_path()); }
	if (!t_bg_grove.loadFromFile(finder.resource_path() + "/image/background/glade.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/glade.png] from file.", finder.resource_path()); }
	if (!t_bg_woods.loadFromFile(finder.resource_path() + "/image/background/woods.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/woods.png] from file.", finder.resource_path()); }
	if (!t_bg_canopy.loadFromFile(finder.resource_path() + "/image/background/canopy.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/canopy.png] from file.", finder.resource_path()); }

	background_lookup.insert({0, t_bg_dusk});
	background_lookup.insert({1, t_bg_sunrise});
	background_lookup.insert({2, t_bg_opensky});
	background_lookup.insert({3, t_bg_woods});
	background_lookup.insert({4, t_bg_canopy});

	// these will change
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

	if (!t_overturned_scenery.loadFromFile(finder.resource_path() + "/image/background/overturned_scenery.png")) {
		NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/background/overturned_scenery.png] from file.", finder.resource_path());
	}
	scenery_lookup.insert({1, t_overturned_scenery});

	if (!t_large_animators.loadFromFile(finder.resource_path() + "/image/animators/large_animators.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/animators/large_animators.png] from file.", finder.resource_path()); }
	if (!t_small_animators.loadFromFile(finder.resource_path() + "/image/animators/small_animators.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/animators/small_animators.png] from file.", finder.resource_path()); }

	if (!t_treasure_ball.loadFromFile(finder.resource_path() + "/image/entity/treasure_ball.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/treasure_ball.png] from file.", finder.resource_path()); }
	if (!t_spawnable_platform.loadFromFile(finder.resource_path() + "/image/entity/spawnable_platform.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/spawnable_platform.png] from file.", finder.resource_path()); }
	if (!t_huge_explosion.loadFromFile(finder.resource_path() + "/image/entity/huge_explosion.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/huge_explosion.png] from file.", finder.resource_path()); }
	if (!t_large_explosion.loadFromFile(finder.resource_path() + "/image/entity/large_explosion.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/large_explosion.png] from file.", finder.resource_path()); }
	if (!t_small_explosion.loadFromFile(finder.resource_path() + "/image/entity/small_explosion.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/small_explosion.png] from file.", finder.resource_path()); }
	if (!t_wall_hit.loadFromFile(finder.resource_path() + "/image/entity/wall_hit.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/wall_hit.png] from file.", finder.resource_path()); }
	if (!t_mini_flash.loadFromFile(finder.resource_path() + "/image/entity/small_flash.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/small_flash.png] from file.", finder.resource_path()); }
	if (!t_medium_flash.loadFromFile(finder.resource_path() + "/image/entity/medium_flash.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/medium_flash.png] from file.", finder.resource_path()); }
	if (!t_inv_hit.loadFromFile(finder.resource_path() + "/image/entity/inv_hit.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/inv_hit.png] from file.", finder.resource_path()); }
	if (!t_puff.loadFromFile(finder.resource_path() + "/image/entity/puff.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/puff.png] from file.", finder.resource_path()); }
	if (!t_bullet_hit.loadFromFile(finder.resource_path() + "/image/entity/bullet_hit.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/bullet_hit.png] from file.", finder.resource_path()); }
	if (!t_doublejump.loadFromFile(finder.resource_path() + "/image/entity/doublejump.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/doublejump.png] from file.", finder.resource_path()); }
	if (!t_dust.loadFromFile(finder.resource_path() + "/image/entity/dust.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/dust.png] from file.", finder.resource_path()); }

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
	effect_lookup.insert({11, t_hit_flash});

	// title stuff
	if (!t_title.loadFromFile(finder.resource_path() + "/image/gui/title.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/gui/title.png] from file.", finder.resource_path()); }

	// load all the other textures...
	for (int i = 0; i < static_cast<std::size_t>(lookup::Style::END); ++i) {
		tilesets.emplace_back();
		if (std::string style = lookup::get_style_string.at(static_cast<lookup::Style>(i)); !tilesets.back().loadFromFile(finder.resource_path() + "/image/tile/" + style + "_tiles.png")) {
			NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/til/{}_tiles.png] from file.", finder.resource_path(), style);
		}
	}

	if (!savepoint.loadFromFile(finder.resource_path() + "/image/entity/savepoint.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/savepoint.png] from file.", finder.resource_path()); }
	if (!t_chest.loadFromFile(finder.resource_path() + "/image/entity/chest.png")) { NANI_LOG_WARN(m_logger, "Failed to load asset [{}/image/entity/chest.png] from file.", finder.resource_path()); }
}

sf::Texture& AssetManager::get_background(int id) {
	if (!background_lookup.contains(id)) { return background_lookup.at(0); }
	return background_lookup.at(id);
}

sf::Texture& AssetManager::get_scenery(int style) {
	if (!scenery_lookup.contains(style)) { return scenery_lookup.at(1); }
	return scenery_lookup.at(style);
}

sf::Texture const& AssetManager::get_tileset(std::string const& label) { return get_texture(label + "_tiles"); }

sf::Texture const& AssetManager::get_texture(std::string const& label) { return m_textures.contains(label) ? m_textures.at(label) : t_null; }

} // namespace fornani::core
