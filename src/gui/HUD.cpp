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

	for (int i = 0; i < num_heart_sprites; ++i) {
		sp_hearts.at(i).setTexture(svc.assets.t_hud_hearts);
		sp_hearts.at(i).setTextureRect(sf::IntRect({heart_dimensions.x * i, 0}, heart_dimensions));
	}
	for (int i = 0; i < num_orb_chars; ++i) {
		sp_orb_text.at(i).setTexture(svc.assets.t_hud_orb_font);
		if (i < 10) {
			sp_orb_text.at(i).setTextureRect(sf::IntRect({orb_text_dimensions.x * i, 0}, orb_text_dimensions));
		} else {
			sp_orb_text.at(i).setTextureRect(sf::IntRect({orb_text_dimensions.x * i, 0}, {orb_label_width, orb_text_dimensions.y}));
		}
	}
	for (int i = 0; i < num_guns; ++i) {
		sp_guns.at(i).setTexture(svc.assets.t_hud_gun_color);
		sp_guns.at(i).setTextureRect(sf::IntRect({0, i * gun_dimensions.y}, gun_dimensions));
		sp_guns_shadow.at(i).setTexture(svc.assets.t_hud_gun_shadow);
		sp_guns_shadow.at(i).setTextureRect(sf::IntRect({0, i * gun_dimensions.y}, gun_dimensions));
	}
	for (int i = 0; i < num_colors; ++i) {
		sp_pointer.at(i).setTexture(svc.assets.t_hud_pointer);
		sp_pointer.at(i).setTextureRect(sf::IntRect({0, i * pointer_dimensions.y}, pointer_dimensions));
	}
	shield_icon.setTexture(svc.assets.t_hud_shield);
	shield_bit.setTexture(svc.assets.t_hud_shield);
	shield_icon.setTextureRect(sf::IntRect{{0, 0}, shield_dimensions});
	shield_icon.setPosition(sf::Vector2<float>{SHIELD_origin});
	shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x * 3, 0}, shield_bit_dimensions});
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
	// shield_discrepancy.set_position({std::clamp(shield_discrepancy.collider.physics.position.x, 0.f, shield.health.get_max()), 0.f});
	auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
	shield_discrepancy.set_target_position({(float)amount, 0.f});
	if (shield.health.full()) { shield_discrepancy.set_position({(float)amount, 0.f}); }
	shield_discrepancy.update(svc);

	filled_hp_cells = static_cast<int>(std::ceil(player.health.get_hp()));
	num_orbs = player.player_stats.orbs;
	total_hp_cells = static_cast<int>(player.health.get_max());
	max_orbs = player.player_stats.max_orbs;
	if (!player.arsenal.empty()) { gun_name = player.equipped_weapon().value().get()->label; }
}

void HUD::render(player::Player& player, sf::RenderWindow& win) {

	for (auto& heart : hearts) { heart.render(win); }

	// ORB
	sp_orb_text.at(orb_label_index).setPosition(corner_pad.x + ORB_origin.x, corner_pad.y + ORB_origin.y);
	win.draw(sp_orb_text.at(orb_label_index));

	digits = std::to_string(num_orbs);
	int ctr{0};
	for (auto& digit : digits) {
		if (digit - '0' >= 0 && digit - '0' < 10) {
			sp_orb_text.at(digit - '0').setPosition(corner_pad.x + ORB_origin.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), corner_pad.y + ORB_origin.y);
			win.draw(sp_orb_text.at(digit - '0'));
		}
		ctr++;
	}

	// GUN
	if (!player.arsenal.empty()) {
		auto pointer_index{0};
		auto loadout_size = player.arsenal.size();
		for (int i = 0; i < loadout_size; ++i) {
			int gun_index = player.arsenal.get_weapon_at(i).value().get()->get_id();
			sp_guns.at(gun_index).setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			sp_guns_shadow.at(gun_index).setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz + 2, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			if (i == player.arsenal.get_index()) {
				win.draw(sp_guns_shadow.at(gun_index));

				win.draw(sp_guns.at(gun_index));

				pointer_index = i;
			} else {
				win.draw(sp_guns_shadow.at(gun_index));
			}
		}
		arms::WEAPON_TYPE curr_type = player.equipped_weapon().value().get()->type;
		sp_pointer.at(player.equipped_weapon().value().get()->attributes.ui_color).setPosition(corner_pad.x + GUN_origin.x, corner_pad.y + GUN_origin.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert));
		win.draw(sp_pointer.at(player.equipped_weapon().value().get()->attributes.ui_color));
	}

	// SHIELD
	if (player.has_shield()) {
		auto const& shield = player.controller.get_shield();
		shield_icon.setTextureRect(sf::IntRect{{0, shield.hud_animation.get_frame() * shield_dimensions.y}, shield_dimensions});
		shield_icon.setPosition({corner_pad.x + SHIELD_origin.x, corner_pad.y + SHIELD_origin.y});
		win.draw(shield_icon);
		auto amount = std::lerp(0, num_bits, shield.health.get_hp() / shield.health.get_max());
		auto discrepancy_position = shield_discrepancy.collider.physics.position.x;

		for (auto i{shield_bar}; i >= 0; --i) {
			shield_bit.setPosition({corner_pad.x + SHIELD_origin.x + shield_dimensions.x + shield_pad + i, corner_pad.y + SHIELD_origin.y});
			if (i <= amount) {
				shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(shield_bit);
			} else if (i > amount && i < discrepancy_position) {
				shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x + shield_bit_dimensions.x * 1, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(shield_bit);
			} else if (i > discrepancy_position) {
				shield_bit.setTextureRect(sf::IntRect{{shield_dimensions.x + shield_bit_dimensions.x * 2, shield.hud_animation.get_frame() * shield_bit_dimensions.y}, shield_bit_dimensions});
				win.draw(shield_bit);
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
