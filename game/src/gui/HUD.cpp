#include "fornani/gui/HUD.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

constexpr auto f_distance_from_edge = 20.f;
constexpr auto f_pad = 4.f;
static bool init{};

HUD::HUD(automa::ServiceProvider& svc, player::Player& player)
	: sprites{.orb = sf::Sprite{svc.assets.get_texture("hud_orb_font")}, .gun = sf::Sprite{svc.assets.get_texture("hud_gun")}, .pointer = sf::Sprite{svc.assets.get_texture("hud_pointer")}},
	  m_reload_bar{svc, {64.f, 8.f}, {colors::blue, colors::goldenrod, colors::navy_blue}, false},
	  origins{.hp = sf::Vector2f{sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge}.x,
								 sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge}.y - f_heart_dimensions.y},
			  .orb = sf::Vector2f{sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge}.x, origins.hp.y - f_pad - orb_text_dimensions.y},
			  .ammo = sf::Vector2f{sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge}.x, origins.orb.y - 4.f * f_pad - f_ammo_dimensions.y},
			  .gun = sf::Vector2f{sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge}.x, origins.ammo.y - f_pad - pointer_dimensions.y - pointer_pad * 2}},
	  health_bar{svc, static_cast<int>(player.health.get_limit()), heart_dimensions, svc.assets.get_texture("hud_hearts"), origins.hp, static_cast<float>(HP_pad)} {
	orient(svc, player, false);
	init = true;
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {
	auto& hp = player.health;
	health_bar.update(svc, hp, hp.flags.test(entity::HPState::hit));

	if (player.arsenal && player.hotbar) {
		if (player.switched_weapon() || init) { ammo_bar = WidgetBar(svc, player.equipped_weapon().ammo.get_capacity(), ammo_dimensions, svc.assets.get_texture("hud_ammo"), origins.ammo, static_cast<float>(AMMO_pad), true); }
		init = false;
		player.hotbar->sync();
		auto& player_ammo = player.equipped_weapon().ammo;
		if (ammo_bar) {
			ammo_bar->update(svc, player_ammo.magazine, player.equipped_weapon().shot());
			m_reload_bar.set_dimensions({player.equipped_weapon().get_reload().get_native_time() * 0.2f, 4.f});
			m_reload_bar.update(svc, origins.ammo + sf::Vector2f{0.f, 32.f}, player.equipped_weapon().get_reload().get_inverse_normalized());
		}
	}
	player.health.flags.reset(entity::HPState::hit);
}

void HUD::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {

	// HEARTS
	health_bar.render(win);

	// ORB
	sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * 10, 0}, {orb_label_width, orb_text_dimensions.y}));
	sprites.orb.setPosition(origins.orb);
	win.draw(sprites.orb);

	digits = std::to_string(player.wallet.get_balance());
	int ctr{0};
	for (auto& digit : digits) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			sprites.orb.setTextureRect(sf::IntRect({orb_text_dimensions.x * index, 0}, orb_text_dimensions));
			sprites.orb.setPosition({origins.orb.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), origins.orb.y});
			win.draw(sprites.orb);
		}
		ctr++;
	}

	// GUN
	if (player.hotbar && player.arsenal) {
		if (ammo_bar) { ammo_bar->render(win); }
		m_reload_bar.render(win);
		auto pointer_index{0};
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = svc.data.get_gun_id_from_tag(player.hotbar.value().get_tag(i));
			sprites.gun.setTextureRect(sf::IntRect({gun_dimensions.x, gun_index * gun_dimensions.y}, gun_dimensions));
			sprites.gun.setPosition({origins.gun.x + pointer_dimensions.x + gun_pad_horiz + 2.f, origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert});
			win.draw(sprites.gun);
			if (i == player.hotbar.value().get_selection()) {
				sprites.gun.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
				sprites.gun.setPosition({origins.gun.x + pointer_dimensions.x + gun_pad_horiz, origins.gun.y - i * gun_dimensions.y - i * gun_pad_vert});
				win.draw(sprites.gun);
				pointer_index = i;
			}
		}
		sprites.pointer.setTextureRect(sf::IntRect({0, player.equipped_weapon().get_ui_color() * pointer_dimensions.y}, pointer_dimensions));
		sprites.pointer.setPosition({+origins.gun.x, +origins.gun.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert)});
		win.draw(sprites.pointer);
	}
}

void HUD::orient(automa::ServiceProvider& svc, player::Player& player, bool file_preview) {
	corner_pad = sf::Vector2f{};
	if (file_preview) { corner_pad = {(svc.window->f_screen_dimensions().x / 2.f) - 140.f, -60.f}; }
	auto f_distance_from_edge = 20.f;
	auto f_pad = 4.f;
	auto true_origin = sf::Vector2f{f_distance_from_edge, svc.window->i_screen_dimensions().y - f_distance_from_edge} + corner_pad;
	origins.hp = sf::Vector2f{true_origin.x, true_origin.y - f_heart_dimensions.y};
	origins.orb = sf::Vector2f{true_origin.x, origins.hp.y - f_pad - orb_text_dimensions.y};
	origins.ammo = sf::Vector2f{true_origin.x, origins.orb.y - 4.f * f_pad - f_ammo_dimensions.y};
	origins.gun = sf::Vector2f{true_origin.x, origins.ammo.y - f_pad - pointer_dimensions.y - pointer_pad * 2};
	health_bar = WidgetBar{svc, static_cast<int>(player.health.get_limit()), heart_dimensions, svc.assets.get_texture("hud_hearts"), origins.hp, static_cast<float>(HP_pad)};
}

} // namespace fornani::gui
