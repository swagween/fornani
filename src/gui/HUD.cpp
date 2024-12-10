#include "HUD.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

HUD::HUD(automa::ServiceProvider& svc, player::Player& player) {
	orient(svc, player, false);
	sprites.orb.setTexture(svc.assets.t_hud_orb_font);
	sprites.gun.setTexture(svc.assets.t_hud_gun_color);
	sprites.gun_shadow.setTexture(svc.assets.t_hud_gun_shadow);
	sprites.pointer.setTexture(svc.assets.t_hud_pointer);
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {
	auto& hp = player.health;
	health_bar.update(svc, hp, hp.flags.test(entity::HPState::hit));

	if (player.arsenal && player.hotbar) {
		if (player.switched_weapon()) { ammo_bar.set(svc, player.equipped_weapon().ammo.get_capacity(), ammo_dimensions, svc.assets.t_hud_ammo, origins.ammo, static_cast<float>(AMMO_pad));
		}
		player.hotbar->sync();
		auto& player_ammo = player.equipped_weapon().ammo;
		ammo_bar.update(svc, player_ammo.magazine, player.equipped_weapon().shot());
	}
	player.health.flags.reset(entity::HPState::hit);
}

void HUD::render(player::Player& player, sf::RenderWindow& win) {

	// HEARTS
	health_bar.render(win);

	// ORB
	sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * 10, 0}, {orb_label_width, orb_text_dimensions.y}));
	sprites.orb.setPosition(origins.orb.x, origins.orb.y);
	win.draw(sprites.orb);

	digits = std::to_string(player.wallet.get_balance());
	int ctr{0};
	for (auto& digit : digits) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * index, 0}, orb_text_dimensions));
			sprites.orb.setPosition(origins.orb.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), origins.orb.y);
			win.draw(sprites.orb);
		}
		ctr++;
	}

	// GUN
	if (player.hotbar && player.arsenal) {
		ammo_bar.render(win);
		auto pointer_index{0};
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = player.hotbar.value().get_id(i);
			sprites.gun.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun_shadow.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun.setPosition(origins.gun.x + pointer_dimensions.x + gun_pad_horiz, origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert);
			sprites.gun_shadow.setPosition(origins.gun.x + pointer_dimensions.x + gun_pad_horiz + 2, origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert);
			win.draw(sprites.gun_shadow);
			if (i == player.hotbar.value().get_selection()) {
				win.draw(sprites.gun);
				pointer_index = i;
			}
		}
		arms::WeaponType curr_type = player.equipped_weapon().get_type();
		sprites.pointer.setTextureRect(sf::IntRect({0, player.equipped_weapon().get_ui_color() * pointer_dimensions.y}, pointer_dimensions));
		sprites.pointer.setPosition(+origins.gun.x, +origins.gun.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert));
		win.draw(sprites.pointer);
	}
}

void HUD::orient(automa::ServiceProvider& svc, player::Player& player, bool file_preview) {
	auto corner_pad{sf::Vector2<float>{}};
	if (file_preview) { corner_pad = {(svc.constants.f_screen_dimensions.x / 2.f) - 140.f, -60.f}; }
	auto f_distance_from_edge = 20.f;
	auto f_pad = 4.f;
	auto true_origin = sf::Vector2<float>{f_distance_from_edge, svc.constants.screen_dimensions.y - f_distance_from_edge} + corner_pad;
	origins.hp = sf::Vector2<float>{true_origin.x, true_origin.y - f_heart_dimensions.y};
	origins.orb = sf::Vector2<float>{true_origin.x, origins.hp.y - f_pad - orb_text_dimensions.y};
	origins.ammo = sf::Vector2<float>{true_origin.x, origins.orb.y - f_pad - f_ammo_dimensions.y};
	origins.gun = sf::Vector2<float>{true_origin.x, origins.ammo.y - f_pad - pointer_dimensions.y - pointer_pad * 2};
	health_bar.set(svc, static_cast<int>(player.health.get_limit()), heart_dimensions, svc.assets.t_hud_hearts, origins.hp, static_cast<float>(HP_pad));
	if (player.arsenal && player.hotbar) { ammo_bar.set(svc, player.equipped_weapon().ammo.get_capacity(), ammo_dimensions, svc.assets.t_hud_ammo, origins.ammo, static_cast<float>(AMMO_pad)); }
}

} // namespace gui
