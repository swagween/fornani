
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include "../utils/BitFlags.hpp"
#include "Projectile.hpp"
#include "Ammo.hpp"
#include <optional>

namespace arms {

enum COLOR_CODE { WHITE = 0, PERIWINKLE = 1, GREEN = 2, ORANGE = 3, FUCSHIA = 4, PURPLE = 5, MYTHIC = 6 };
enum class GunState { unlocked, equipped, cooling_down, reloading };
enum class InventoryState { reserve, hotbar };
enum class UIFlags { selected };

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
	int multishot{};
};

struct EmitterAttributes {
	sf::Vector2<float> dimensions{};
	std::string_view type{};
	sf::Color color{};
};

class Weapon {

  public:
	Weapon(automa::ServiceProvider& svc, int id);

	void update(automa::ServiceProvider& svc, dir::Direction to_direction);
	void render_back(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos);
	void render_ui(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> position);

	void equip();
	void unequip();
	void unlock();
	void lock();
	void shoot();
	void decrement_projectiles();

	bool is_equipped() const;
	bool is_unlocked() const;
	bool cooling_down() const;
	bool can_shoot() const;

	void set_position(sf::Vector2<float> pos);
	void set_orientation(dir::Direction to_direction);
	void reset();

	void set_hotbar() { inventory_state = InventoryState::hotbar; }
	void set_reserved() { inventory_state = InventoryState::reserve; }
	void select() { ui_flags.set(UIFlags::selected); }
	void deselect() { ui_flags.reset(UIFlags::selected); }

	[[nodiscard]] auto selected() const -> bool { return ui_flags.test(UIFlags::selected); }
	[[nodiscard]] auto shot() const -> bool { return cooldown.just_started(); }
	[[nodiscard]] auto get_id() const -> int { return id; }
	[[nodiscard]] auto get_active_projectiles() const -> int { return active_projectiles.get_count(); }
	[[nodiscard]] auto get_inventory_state() const -> int { return static_cast<int>(inventory_state); }
	[[nodiscard]] auto get_ui_position() const -> sf::Vector2<float> { return sprites.ui.getPosition(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return metadata.description; }
	[[nodiscard]] auto multishot() const -> bool { return attributes.multishot != 0; }

	Ammo ammo{};
	WeaponAttributes attributes{};
	EmitterAttributes emitter{};
	std::optional<EmitterAttributes> secondary_emitter{};
	Projectile projectile;

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
	struct {
		sf::Sprite ui{};
	} sprites{};

	util::Cooldown cooldown{};
	dir::Direction firing_direction{};

  private:
	struct {
		std::string_view description{};
	} metadata{};
	util::BitFlags<GunState> flags{};
	util::BitFlags<UIFlags> ui_flags{};
	int id{};
	util::Counter active_projectiles{};
	InventoryState inventory_state{};
	struct {
		util::Cooldown reload{};
		util::Cooldown down_time{};
	} cooldowns{};
};

} // namespace arms
