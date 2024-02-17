
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include "../setup/EnumLookups.hpp"
#include "../setup/LookupTables.hpp"
#include "../setup/ServiceLocator.hpp"

namespace gui {

enum HP_ELEMS { HP_LIGHT = 0, HP_FILLED = 1, HP_TAKEN = 2, HP_GONE = 3 };

inline int const distance_from_edge{20};
inline int const PAD{4};
inline int const HP_pad{2};
inline int const orb_pad{8};
inline int const gun_pad_horiz{12};
inline int const gun_pad_vert{4};
inline int const pointer_pad{4};
inline const sf::Vector2<int> heart_dimensions{18, 18};
inline const sf::Vector2<int> orb_text_dimensions{18, 16};
inline const sf::Vector2<int> gun_dimensions{66, 18};
inline const sf::Vector2<int> pointer_dimensions{14, 10};
inline const sf::Vector2<int> HP_origin{distance_from_edge, static_cast<int>(cam::screen_dimensions.y) - distance_from_edge - heart_dimensions.y};
inline const sf::Vector2<int> ORB_origin{distance_from_edge, HP_origin.y - PAD - orb_text_dimensions.y};
inline const sf::Vector2<int> GUN_origin{distance_from_edge, ORB_origin.y - PAD - pointer_dimensions.y - pointer_pad * 2};
inline int const orb_label_width{44};
inline int const orb_label_index{10};

int const num_heart_sprites = 4;
int const num_orb_chars = 11;
int const num_guns = 18; // to be changed later, obviously
int const num_colors = 6;

class HUD {

  public:
	HUD(sf::Vector2<int> pos) : position(pos) {
		update();
		for (int i = 0; i < num_heart_sprites; ++i) {
			sp_hearts.at(i).setTexture(svc::assetLocator.get().t_hud_hearts);
			sp_hearts.at(i).setTextureRect(sf::IntRect({heart_dimensions.x * i, 0}, heart_dimensions));
		}
		for (int i = 0; i < num_orb_chars; ++i) {
			sp_orb_text.at(i).setTexture(svc::assetLocator.get().t_hud_orb_font);
			if (i < 10) {
				sp_orb_text.at(i).setTextureRect(sf::IntRect({orb_text_dimensions.x * i, 0}, orb_text_dimensions));
			} else {
				sp_orb_text.at(i).setTextureRect(sf::IntRect({orb_text_dimensions.x * i, 0}, {orb_label_width, orb_text_dimensions.y}));
			}
		}
		for (int i = 0; i < num_guns; ++i) {
			sp_guns.at(i).setTexture(svc::assetLocator.get().t_hud_gun_color);
			sp_guns.at(i).setTextureRect(sf::IntRect({0, i * gun_dimensions.y}, gun_dimensions));
			sp_guns_shadow.at(i).setTexture(svc::assetLocator.get().t_hud_gun_shadow);
			sp_guns_shadow.at(i).setTextureRect(sf::IntRect({0, i * gun_dimensions.y}, gun_dimensions));
		}
		for (int i = 0; i < num_colors; ++i) {
			sp_pointer.at(i).setTexture(svc::assetLocator.get().t_hud_pointer);
			sp_pointer.at(i).setTextureRect(sf::IntRect({0, i * pointer_dimensions.y}, pointer_dimensions));
		}
	}

	void update() {
		filled_hp_cells = svc::playerLocator.get().player_stats.health;
		num_orbs = svc::playerLocator.get().player_stats.orbs;
		total_hp_cells = svc::playerLocator.get().player_stats.max_health;
		max_orbs = svc::playerLocator.get().player_stats.max_orbs;
		gun_name = svc::playerLocator.get().loadout.get_equipped_weapon().label;
		constrain();
	}

	void constrain() {
		if (filled_hp_cells > total_hp_cells) { filled_hp_cells = total_hp_cells; }
		if (filled_hp_cells < 0) { filled_hp_cells = 0; }
		if (num_orbs > max_orbs) { num_orbs = max_orbs; }
		if (num_orbs < 0) { num_orbs = 0; }
	}

	void render(sf::RenderWindow& win) {

		// HP
		for (int i = 0; i < total_hp_cells; ++i) {
			if (i < filled_hp_cells) {
				if (i == 0) {
					sp_hearts.at(HP_FILLED).setPosition(corner_pad.x + HP_origin.x + i * heart_dimensions.x, corner_pad.y + HP_origin.y);
				} else {
					sp_hearts.at(HP_FILLED).setPosition(corner_pad.x + HP_origin.x + i * heart_dimensions.x + i * HP_pad, corner_pad.y + HP_origin.y);
				}
				win.draw(sp_hearts.at(HP_FILLED));
				svc::counterLocator.get().at(svc::draw_calls)++;
			} else {
				sp_hearts.at(HP_GONE).setPosition(corner_pad.x + HP_origin.x + i * heart_dimensions.x + i * HP_pad, corner_pad.y + HP_origin.y);
				win.draw(sp_hearts.at(HP_GONE));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}
		}

		// ORB
		sp_orb_text.at(orb_label_index).setPosition(corner_pad.x + ORB_origin.x, corner_pad.y + ORB_origin.y);
		win.draw(sp_orb_text.at(orb_label_index));
		svc::counterLocator.get().at(svc::draw_calls)++;
		digits = std::to_string(num_orbs);
		int ctr{0};
		for (auto& digit : digits) {

			if (digit - '0' >= 0 && digit - '0' < 10) {
				sp_orb_text.at(digit - '0').setPosition(corner_pad.x + ORB_origin.x + orb_label_width + orb_pad + (orb_text_dimensions.x * ctr), corner_pad.y + ORB_origin.y);
				win.draw(sp_orb_text.at(digit - '0'));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}

			ctr++;
		}

		// GUN
		int pointer_index{0};
		;
		int loadout_size = svc::playerLocator.get().weapons_hotbar.size();
		for (int i = 0; i < loadout_size; ++i) {
			int gun_index = lookup::type_to_index.at(svc::playerLocator.get().weapons_hotbar.at(i));
			sp_guns.at(gun_index).setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			sp_guns_shadow.at(gun_index).setPosition(corner_pad.x + GUN_origin.x + pointer_dimensions.x + gun_pad_horiz + 2, corner_pad.y + GUN_origin.y - i * gun_dimensions.y - i * gun_pad_vert);
			if (svc::playerLocator.get().loadout.equipped_weapon == svc::playerLocator.get().weapons_hotbar.at(i)) {
				win.draw(sp_guns_shadow.at(gun_index));
				svc::counterLocator.get().at(svc::draw_calls)++;
				win.draw(sp_guns.at(gun_index));
				svc::counterLocator.get().at(svc::draw_calls)++;
				pointer_index = i;
			} else {
				win.draw(sp_guns_shadow.at(gun_index));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}
		}
		arms::WEAPON_TYPE curr_type = svc::playerLocator.get().loadout.get_equipped_weapon().type;
		sp_pointer.at(lookup::type_to_weapon.at(curr_type).attributes.ui_color).setPosition(corner_pad.x + GUN_origin.x, corner_pad.y + GUN_origin.y + pointer_pad - pointer_index * (gun_dimensions.y + gun_pad_vert));
		if (svc::playerLocator.get().weapons_hotbar.size() != 0) {
			win.draw(sp_pointer.at(lookup::type_to_weapon.at(curr_type).attributes.ui_color));
			svc::counterLocator.get().at(svc::draw_calls)++;
		}
	}

	void set_corner_pad(bool file_preview = false) { file_preview ? corner_pad = {((float)cam::screen_dimensions.x / 2) - 140, -60.f} : corner_pad = {0.f, 0.f}; }

	sf::Vector2<int> position{};
	int total_hp_cells{};
	int filled_hp_cells{};
	int max_orbs{};
	int num_orbs{};
	std::string_view gun_name{};
	std::string digits{};

	std::array<sf::Sprite, num_heart_sprites> sp_hearts{};
	std::array<sf::Sprite, num_orb_chars> sp_orb_text{};
	std::array<sf::Sprite, num_guns> sp_guns_shadow{};
	std::array<sf::Sprite, num_guns> sp_guns{};
	std::array<sf::Sprite, num_colors> sp_pointer{};

	sf::Vector2f corner_pad{}; // for rendering file preview
};

} // namespace gui
