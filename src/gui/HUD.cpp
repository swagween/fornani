#include "HUD.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

HUD::HUD(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<int> pos) : position(pos) {
	HP_origin = {distance_from_edge, (int)svc.constants.screen_dimensions.y - distance_from_edge - heart_dimensions.y};
	origins.hp = {(float)distance_from_edge, svc.constants.screen_dimensions.y - (float)distance_from_edge - (float)heart_dimensions.y};
	ORB_origin = {distance_from_edge, HP_origin.y - PAD - orb_text_dimensions.y};
	GUN_origin = {distance_from_edge, ORB_origin.y - PAD - pointer_dimensions.y - pointer_pad * 2};
	SHIELD_origin = {distance_from_edge, HP_origin.y + PAD + heart_dimensions.y};
	shield_discrepancy = vfx::Gravitator({0, 0}, svc.styles.colors.bright_orange, 0.1f);
	shield_discrepancy.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.9f, 0.9f}, 1.0f);
	shield_discrepancy.set_position({0.f, 0.f});

	for (auto i{0}; i < player.health.get_limit(); ++i) {
		hearts.push_back(Widget(svc, heart_dimensions, i));
		hearts.back().sprite.setTexture(svc.assets.t_hud_hearts);
		hearts.back().position = {corner_pad.x + i * heart_dimensions.x + i * HP_pad, corner_pad.y};
		hearts.back().gravitator.set_position(hearts.back().position);
		hearts.back().origin = origins.hp;
	}

	sprites.orb.setTexture(svc.assets.t_hud_orb_font);
	sprites.gun.setTexture(svc.assets.t_hud_gun_color);
	sprites.gun_shadow.setTexture(svc.assets.t_hud_gun_shadow);
	sprites.pointer.setTexture(svc.assets.t_hud_pointer);
	sprites.shield_icon.setTexture(svc.assets.t_hud_shield);
	sprites.shield_bit.setTexture(svc.assets.t_hud_shield);
	sprites.shield_icon.setTextureRect(sf::IntRect{{0, 0}, shield_dimensions});
	sprites.shield_icon.setPosition(sf::Vector2<float>{SHIELD_origin});
	sprites.shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x * 3, 0}, shield_bit_dimensions});
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {

	// health widgets
	auto const& hp = player.health;
	int i{};
	for (auto& heart : hearts) {
		heart.position = {corner_pad.x + i * heart_dimensions.x + i * HP_pad, corner_pad.y};
		if (hp.flags.test(entity::HPState::hit)) {
			auto randx = svc.random.random_range_float(-16.f, 16.f);
			auto randy = svc.random.random_range_float(-16.f, 16.f);
			heart.gravitator.set_position(heart.position + sf::Vector2<float>{randx, randy});
		}
		heart.update(svc, player);
		heart.current_state = hp.get_hp() > i ? State::neutral : player.health.taken_point > i ? State::taken : State::gone;
		auto flashing = hp.restored.running() && hp.restored.get_cooldown() % 48 > 24 && hp.get_hp() > i;
		heart.current_state = flashing ? State::added : heart.current_state;
		++i;
	}

	player.health.flags.reset(entity::HPState::hit);

	auto const& shield = player.controller.get_shield();
	auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
	shield_discrepancy.set_target_position({(float)amount, 0.f});
	if (shield.health.full()) { shield_discrepancy.set_position({(float)amount, 0.f}); }
	shield_discrepancy.update(svc);

	filled_hp_cells = static_cast<int>(std::ceil(player.health.get_hp()));
	num_orbs = player.player_stats.orbs;
	total_hp_cells = static_cast<int>(player.health.get_max());
	max_orbs = player.player_stats.max_orbs;
	if (player.hotbar) { gun_name = player.equipped_weapon().label; }
}

void HUD::render(player::Player& player, sf::RenderWindow& win) {

	// HEARTS
	for (auto& heart : hearts) { heart.render(win); }

	// ORB
	sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * 10, 0}, {orb_label_width, orb_text_dimensions.y}));
	sprites.orb.setPosition(corner_pad.x + ORB_origin.x, corner_pad.y + ORB_origin.y);
	win.draw(sprites.orb);

	digits = std::to_string(num_orbs);
	int ctr{0};
	for (auto& digit : digits) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * index, 0}, orb_text_dimensions));
			sprites.orb.setPosition(corner_pad.x + ORB_origin.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), corner_pad.y + ORB_origin.y);
			win.draw(sprites.orb);
		}
		ctr++;
	}

	// GUN
	if (player.hotbar && player.arsenal) {
		auto pointer_index{0};
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = player.hotbar.value().get_id(i);
			sprites.gun.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun_shadow.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun.setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			sprites.gun_shadow.setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz + 2, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			win.draw(sprites.gun_shadow);
			if (i == player.hotbar.value().get_selection()) {
				win.draw(sprites.gun);
				pointer_index = i;
			}
		}
		arms::WEAPON_TYPE curr_type = player.equipped_weapon().type;
		sprites.pointer.setTextureRect(sf::IntRect({0, player.equipped_weapon().attributes.ui_color * pointer_dimensions.y}, pointer_dimensions));
		sprites.pointer.setPosition(corner_pad.x + GUN_origin.x, corner_pad.y + GUN_origin.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert));
		win.draw(sprites.pointer);
	}

	// SHIELD
	if (player.has_shield()) {
		auto const& shield = player.controller.get_shield();
		sprites.shield_icon.setTextureRect(sf::IntRect{{0, shield.hud_animation.get_frame() * shield_dimensions.y}, shield_dimensions});
		sprites.shield_icon.setPosition({corner_pad.x + SHIELD_origin.x, corner_pad.y + SHIELD_origin.y});
		win.draw(sprites.shield_icon);
		auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
		auto discrepancy_position = shield_discrepancy.collider.physics.position.x;

		for (auto i{shield_bar}; i >= 0; --i) {
			sprites.shield_bit.setPosition({corner_pad.x + SHIELD_origin.x + shield_dimensions.x + shield_pad + i, corner_pad.y + SHIELD_origin.y});
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
	for (auto& heart : hearts) {
		heart.position = {corner_pad.x + ctr * heart_dimensions.x + ctr * HP_pad, corner_pad.y};
		heart.gravitator.set_position(heart.position);
		++ctr;
	}
}

} // namespace gui
