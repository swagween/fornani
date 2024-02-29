
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include <array>
#include "Weapon.hpp"

namespace arms {

using Key = WEAPON_TYPE;
constexpr static int max_weapons{19};

/*    const vfx::ElementBehavior spr = light_gun_spray, const ProjectileAnimation& pa = default_anim, RENDER_TYPE rt = RENDER_TYPE::SINGLE_SPRITE,
	sf::Vector2<int> dim = { 16, 16 }, sf::Vector2<float> proj_dim = { 28.0f, 12.0f }); */

inline Weapon bryns_gun{0};

//inline Weapon bryns_gun = Weapon(0, "bryn's gun", Key::BRYNS_GUN, bryns_gun_attributes, bryns_gun_stats, light_gun_spray, bg_anim, RENDER_TYPE::SINGLE_SPRITE, {18, 8}, {28.0f, 12.0f});
inline Weapon plasmer = Weapon(1, "plasmer", Key::PLASMER, plasmer_attributes, plasmer_stats, heavy_gun_spray, bg_anim, RENDER_TYPE::SINGLE_SPRITE, {22, 10}, {38.0f, 12.0f});
inline Weapon wasp = Weapon(2, "wasp", Key::WASP, wasp_attributes, wasp_stats);
inline Weapon blizzard = Weapon(3, "blizzard", Key::BLIZZARD, blizzard_attributes, blizzard_stats);
inline Weapon bismuth = Weapon(4, "bismuth", Key::BISMUTH, bismuth_attributes, bismuth_stats);
inline Weapon underdog = Weapon(5, "underdog", Key::UNDERDOG, underdog_attributes, underdog_stats, light_gun_spray, bg_anim, RENDER_TYPE::MULTI_SPRITE, {26, 10}, {32.0f, 16.0f});
inline Weapon electron = Weapon(6, "electron", Key::ELECTRON, electron_attributes, electron_stats);
inline Weapon clover = Weapon(7, "clover", Key::CLOVER, clover_attributes, clover_stats, bubble_spray, clover_anim, RENDER_TYPE::ANIMATED, {18, 10});
inline Weapon triton = Weapon(8, "triton", Key::TRITON, triton_attributes, triton_stats);
inline Weapon willet_585 = Weapon(9, "willet 585", Key::WILLET_585, willet_585_attributes, willet_585_stats);
inline Weapon quasar = Weapon(10, "quasar", Key::QUASAR, quasar_attributes, quasar_stats);
inline Weapon nova = Weapon(11, "nova", Key::NOVA, nova_attributes, nova_stats, powerful_gun_spray, nova_anim, RENDER_TYPE::SINGLE_SPRITE, {24, 10}, {28.0f, 12.0f});
inline Weapon venom = Weapon(12, "venom", Key::VENOM, venom_attributes, venom_stats);
inline Weapon twin = Weapon(13, "twin", Key::TWIN, twin_attributes, twin_stats);
inline Weapon carise = Weapon(14, "carise", Key::CARISE, carise_attributes, carise_stats);
inline Weapon stinger = Weapon(15, "stinger", Key::STINGER, stinger_attributes, stinger_stats);
inline Weapon tusk = Weapon(16, "tusk", Key::TUSK, tusk_attributes, tusk_stats);
inline Weapon tomahawk = Weapon(17, "tomahawk", Key::TOMAHAWK, tomahawk_attributes, tomahawk_stats);

// enemy
inline Weapon skycorps_ar = Weapon(18, "skycorps_ar", Key::SKYCORPS_AR, skycorps_ar_attributes, skycorps_ar_stats, light_gun_spray, bg_anim, RENDER_TYPE::SINGLE_SPRITE, {0, 0}, {14.0f, 14.0f});

struct Arsenal {

	Arsenal() = default;
	Arsenal& operator=(Arsenal&&) = delete;
	
	void init() {
		for (int i = 0; i < max_weapons; ++i) { armory.at(i) = Weapon(i); }
	}
	void push_to_loadout(int id) { loadout.push_back(armory.at(id)); }

	std::array<Weapon, max_weapons> armory{};
	std::vector<Weapon> loadout{};

};

} // namespace arms
