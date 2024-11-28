#include "HUD.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

HUD::HUD(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<int> pos) : position(pos) {
	auto f_distance_from_edge = 20.f;
	auto f_pad = 4.f;
	origins.hp = {f_distance_from_edge, svc.constants.screen_dimensions.y - f_distance_from_edge - (float)heart_dimensions.y};
	origins.orb = {f_distance_from_edge, origins.hp.y - f_pad - orb_text_dimensions.y};
	origins.ammo = {f_distance_from_edge, origins.orb.y - f_pad - f_ammo_dimensions.y};
	origins.gun = {f_distance_from_edge, origins.ammo.y - f_pad - pointer_dimensions.y - pointer_pad * 2};
	origins.shield = {f_distance_from_edge, origins.hp.y + f_pad + heart_dimensions.y};
	shield_discrepancy = vfx::Gravitator({0, 0}, svc.styles.colors.bright_orange, 0.1f);
	shield_discrepancy.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.9f, 0.9f}, 1.0f);
	shield_discrepancy.set_position({0.f, 0.f});

	health_bar.set(svc, player.health.get_limit(), heart_dimensions, svc.assets.t_hud_hearts, origins.hp, HP_pad);
	if (player.arsenal && player.hotbar) { ammo_bar.set(svc, player.equipped_weapon().ammo.get_capacity(), ammo_dimensions, svc.assets.t_hud_ammo, origins.ammo, AMMO_pad); }

	sprites.orb.setTexture(svc.assets.t_hud_orb_font);
	sprites.gun.setTexture(svc.assets.t_hud_gun_color);
	sprites.gun_shadow.setTexture(svc.assets.t_hud_gun_shadow);
	sprites.pointer.setTexture(svc.assets.t_hud_pointer);
	sprites.shield_icon.setTexture(svc.assets.t_hud_shield);
	sprites.shield_bit.setTexture(svc.assets.t_hud_shield);
	sprites.shield_icon.setTextureRect(sf::IntRect{{0, 0}, shield_dimensions});
	sprites.shield_icon.setPosition(origins.shield);
	sprites.shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x * 3, 0}, shield_bit_dimensions});
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {

	// widget bars
	auto& hp = player.health;
	health_bar.update(svc, hp, hp.flags.test(entity::HPState::hit));

	if (player.arsenal && player.hotbar) {
		if (player.switched_weapon()) { ammo_bar.set(svc, player.equipped_weapon().ammo.get_capacity(), ammo_dimensions, svc.assets.t_hud_ammo, origins.ammo, AMMO_pad); }
		player.hotbar->sync();
		auto& player_ammo = player.equipped_weapon().ammo;
		ammo_bar.update(svc, player_ammo.magazine, player.equipped_weapon().shot());
	}

	player.health.flags.reset(entity::HPState::hit);

	auto const& shield = player.controller.get_shield();
	auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
	shield_discrepancy.set_target_position({(float)amount, 0.f});
	if (shield.health.full()) { shield_discrepancy.set_position({(float)amount, 0.f}); }
	shield_discrepancy.update(svc);

	filled_hp_cells = static_cast<int>(std::ceil(player.health.get_hp()));
	num_orbs = player.wallet.get_balance();
	total_hp_cells = static_cast<int>(player.health.get_max());
	if (player.hotbar && player.arsenal) { gun_name = player.equipped_weapon().label; }
}

void HUD::render(player::Player& player, sf::RenderWindow& win) {

	// HEARTS
	health_bar.render(win);

	// ORB
	sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * 10, 0}, {orb_label_width, orb_text_dimensions.y}));
	sprites.orb.setPosition(corner_pad.x + origins.orb.x, corner_pad.y + origins.orb.y);
	win.draw(sprites.orb);

	digits = std::to_string(num_orbs);
	int ctr{0};
	for (auto& digit : digits) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * index, 0}, orb_text_dimensions));
			sprites.orb.setPosition(corner_pad.x + origins.orb.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), corner_pad.y + origins.orb.y);
			win.draw(sprites.orb);
		}
		ctr++;
	}


	// GUN
	if (player.hotbar && player.arsenal) {
		// AMMO
		ammo_bar.render(win);
		auto pointer_index{0};
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = player.hotbar.value().get_id(i);
			sprites.gun.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun_shadow.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun.setPosition(corner_pad.x + origins.gun.x + pointer_dimensions.x + gun_pad_horiz, corner_pad.y + origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert);
			sprites.gun_shadow.setPosition(corner_pad.x + origins.gun.x + pointer_dimensions.x + gun_pad_horiz + 2, corner_pad.y + origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert);
			win.draw(sprites.gun_shadow);
			if (i == player.hotbar.value().get_selection()) {
				win.draw(sprites.gun);
				pointer_index = i;
			}
		}
		arms::WEAPON_TYPE curr_type = player.equipped_weapon().type;
		sprites.pointer.setTextureRect(sf::IntRect({0, player.equipped_weapon().attributes.ui_color * pointer_dimensions.y}, pointer_dimensions));
		sprites.pointer.setPosition(corner_pad.x + origins.gun.x, corner_pad.y + origins.gun.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert));
		win.draw(sprites.pointer);
	}

	// SHIELD
	if (player.has_shield()) {
		auto const& shield = player.controller.get_shield();
		sprites.shield_icon.setTextureRect(sf::IntRect{{0, shield.hud_animation.get_frame() * shield_dimensions.y}, shield_dimensions});
		sprites.shield_icon.setPosition({corner_pad.x + origins.shield.x, corner_pad.y + origins.shield.y});
		win.draw(sprites.shield_icon);
		auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
		auto discrepancy_position = shield_discrepancy.collider.physics.position.x;

		for (auto i{shield_bar}; i >= 0; --i) {
			sprites.shield_bit.setPosition({corner_pad.x + origins.shield.x + shield_dimensions.x + shield_pad + i, corner_pad.y + origins.shield.y});
			if (i <= amount) {
				sprites.shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(sprites.shield_bit);
			} else if (i > amount && i < discrepancy_position) {
				sprites.shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x + shield_bit_dimensions.x * 1, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(sprites.shield_bit);
			} else if (i > discrepancy_position) {
				sprites.shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x + shield_bit_dimensions.x * 2, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(sprites.shield_bit);
			}
		}
		flags.set(HUDState::shield);
	} else {
		flags.reset(HUDState::shield);
	}
}

void HUD::set_corner_pad(automa::ServiceProvider& svc, bool file_preview) {
	corner_pad = {0.f, 0.f};
	if (file_preview) { corner_pad = {((float)svc.constants.screen_dimensions.x / 2.f) - 140.f, -60.f}; }
	if (flags.test(HUDState::shield)) { corner_pad = {0.f, -((float)shield_dimensions.y + PAD)}; }
	int ctr{};
}

} // namespace gui
