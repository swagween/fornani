
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include "../utils/BitFlags.hpp"
#include "Projectile.hpp"

namespace arms {

enum COLOR_CODE { WHITE = 0, PERIWINKLE = 1, GREEN = 2, ORANGE = 3, FUCSHIA = 4, PURPLE = 5 };

enum class GunState { unlocked, equipped, cooling_down, reloading };

inline std::unordered_map<int, arms::WEAPON_TYPE> index_to_type{{0, arms::WEAPON_TYPE::BRYNS_GUN},		{1, arms::WEAPON_TYPE::PLASMER},	{2, arms::WEAPON_TYPE::SKYCORPS_AR}, {3, arms::WEAPON_TYPE::TOMAHAWK},
																{4, arms::WEAPON_TYPE::GRAPPLING_HOOK}, {5, arms::WEAPON_TYPE::UNDERDOG},	{6, arms::WEAPON_TYPE::ELECTRON},	 {7, arms::WEAPON_TYPE::CLOVER},
																{8, arms::WEAPON_TYPE::TRITON},			{9, arms::WEAPON_TYPE::WILLET_585}, {10, arms::WEAPON_TYPE::QUASAR},	 {11, arms::WEAPON_TYPE::NOVA},
																{12, arms::WEAPON_TYPE::VENOM},			{13, arms::WEAPON_TYPE::TWIN},		{14, arms::WEAPON_TYPE::CARISE},	 {15, arms::WEAPON_TYPE::STINGER},
																{16, arms::WEAPON_TYPE::TUSK},			{17, arms::WEAPON_TYPE::WASP},		{18, arms::WEAPON_TYPE::BLIZZARD},	 {19, arms::WEAPON_TYPE::BISMUTH}};


struct WeaponAttributes {
	bool automatic{};
	bool boomerang{};
	int rate{};
	int cooldown_time{};
	float recoil{};
	COLOR_CODE ui_color{};
	std::array<float, 2> barrel_position{};
};

inline std::unordered_map<arms::WEAPON_TYPE, sf::Color> spray_color{
	{arms::WEAPON_TYPE::BRYNS_GUN, flcolor::periwinkle},  {arms::WEAPON_TYPE::PLASMER, flcolor::fucshia}, {arms::WEAPON_TYPE::WASP, flcolor::goldenrod},
	{arms::WEAPON_TYPE::BLIZZARD, flcolor::blue},		  {arms::WEAPON_TYPE::BISMUTH, flcolor::fucshia}, {arms::WEAPON_TYPE::UNDERDOG, flcolor::orange},
	{arms::WEAPON_TYPE::ELECTRON, flcolor::white},		  {arms::WEAPON_TYPE::CLOVER, flcolor::green},	  {arms::WEAPON_TYPE::TRITON, flcolor::goldenrod},
	{arms::WEAPON_TYPE::WILLET_585, flcolor::periwinkle}, {arms::WEAPON_TYPE::QUASAR, flcolor::green},	  {arms::WEAPON_TYPE::NOVA, flcolor::white},
	{arms::WEAPON_TYPE::VENOM, flcolor::goldenrod},		  {arms::WEAPON_TYPE::TWIN, flcolor::fucshia},	  {arms::WEAPON_TYPE::CARISE, flcolor::blue},
	{arms::WEAPON_TYPE::STINGER, flcolor::goldenrod},	  {arms::WEAPON_TYPE::TUSK, flcolor::white},	  {arms::WEAPON_TYPE::TOMAHAWK, flcolor::white},

	{arms::WEAPON_TYPE::SKYCORPS_AR, flcolor::goldenrod}, {arms::WEAPON_TYPE::STINGER, flcolor::goldenrod}, {arms::WEAPON_TYPE::TUSK, flcolor::white},
	{arms::WEAPON_TYPE::GRAPPLING_HOOK, flcolor::ui_white}

};

class Weapon {

  public:
	Weapon() = default;
	Weapon(automa::ServiceProvider& svc, int id);

	void update(dir::Direction to_direction);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos);

	void equip();
	void unequip();
	void unlock();
	void lock();
	void shoot();

	bool is_equipped() const;
	bool is_unlocked() const;
	bool cooling_down() const;
	bool can_shoot() const;

	void set_position(sf::Vector2<float> pos);
	void set_orientation(dir::Direction to_direction);

	int get_id();

	WeaponAttributes attributes{};

	Projectile projectile{};

	//spray
	sf::Vector2<float> emitter_dimensions{};
	std::string_view emitter_type{};
	sf::Color emitter_color{};

	sf::Vector2<float> sprite_position{};
	sf::Vector2<float> gun_offset{};
	std::vector<sf::Vector2<float>> anchor_points{};
	sf::Vector2<float> barrel_point{};

	WEAPON_TYPE type{};
	sf::Vector2<int> sprite_dimensions{};
	sf::Vector2<int> sprite_offset{};
	std::string label{};

	sf::Sprite sp_gun{};

	int active_projectiles{};
	util::Cooldown cooldown{};
	dir::Direction firing_direction{};

  private:
	util::BitFlags<GunState> flags{};
	int id{};
};

} // namespace arms
