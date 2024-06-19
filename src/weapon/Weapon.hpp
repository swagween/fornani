
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include "../utils/BitFlags.hpp"
#include "Projectile.hpp"
#include "Ammo.hpp"

namespace arms {

enum COLOR_CODE { WHITE = 0, PERIWINKLE = 1, GREEN = 2, ORANGE = 3, FUCSHIA = 4, PURPLE = 5 };
enum class GunState { unlocked, equipped, cooling_down, reloading };

struct WeaponAttributes {
	bool automatic{};
	bool boomerang{};
	bool grenade{};
	int rate{};
	int cooldown_time{};
	float recoil{};
	COLOR_CODE ui_color{};
	std::array<float, 2> barrel_position{};
	int back_offset{};
};

class Weapon {

  public:
	Weapon() = default;
	Weapon(automa::ServiceProvider& svc, std::string_view label, int id);

	void update(dir::Direction to_direction);
	void render_back(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos);
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

	[[nodiscard]] auto get_id() const -> int { return id; }

	WeaponAttributes attributes{};

	Projectile projectile;

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
	std::string_view label{};

	sf::Sprite sp_gun{};
	sf::Sprite sp_gun_back{};

	int active_projectiles{};
	util::Cooldown cooldown{};
	dir::Direction firing_direction{};

  private:
	util::BitFlags<GunState> flags{};
	Ammo ammo{};
	int id{};
};

} // namespace arms
