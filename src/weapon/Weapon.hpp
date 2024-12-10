
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include "../utils/BitFlags.hpp"
#include "Projectile.hpp"
#include "Ammo.hpp"
#include "../entities/animation/AnimatedSprite.hpp"
#include <optional>

namespace arms {

enum class WeaponAttributes { automatic };
enum class WeaponState { unlocked, equipped, reloading };
enum class InventoryState { reserve, hotbar };
enum class UIFlags { selected };
enum class UIColor { white, periwinkle, green, orange, fucshia, purple, mythic };
struct WeaponSpecifications {
	int cooldown_time{};
	int multishot{};
	float recoil{};
};

struct Offsets {
	struct {
		sf::Vector2<float> global{};
		sf::Vector2<int> stock{};
		sf::Vector2<int> grip{};
		sf::Vector2<float> barrel{};
	} render{};
	struct {
		sf::Vector2<float> barrel{};
	} gameplay{};
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
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
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
	void set_barrel_point(sf::Vector2<float> point);
	void set_orientation(dir::Direction to_direction);
	void set_team(Team team);
	void set_firing_direction(dir::Direction to_direction);
	void reset();

	void set_hotbar() { inventory_state = InventoryState::hotbar; }
	void set_reserved() { inventory_state = InventoryState::reserve; }
	void select() { flags.ui.set(UIFlags::selected); }
	void deselect() { flags.ui.reset(UIFlags::selected); }

	[[nodiscard]] auto selected() const -> bool { return flags.ui.test(UIFlags::selected); }
	[[nodiscard]] auto shot() const -> bool { return cooldowns.cooldown.just_started(); }
	[[nodiscard]] auto automatic() const -> bool { return attributes.test(WeaponAttributes::automatic); }
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_active_projectiles() const -> int { return active_projectiles.get_count(); }
	[[nodiscard]] auto get_inventory_state() const -> int { return static_cast<int>(inventory_state); }
	[[nodiscard]] auto get_ui_position() const -> sf::Vector2<float> { return visual.ui.getPosition(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return metadata.description; }
	[[nodiscard]] auto multishot() const -> bool { return specifications.multishot != 0; }
	[[nodiscard]] auto get_barrel_point() const -> sf::Vector2<float> { return offsets.gameplay.barrel; }
	[[nodiscard]] auto get_cooldown() const -> int { return cooldowns.cooldown.get_cooldown(); }
	[[nodiscard]] auto get_firing_direction() & -> dir::Direction& { return firing_direction; }
	[[nodiscard]] auto get_global_offset() const -> sf::Vector2<float> { return offsets.render.global; };
	[[nodiscard]] auto get_recoil() const -> float { return specifications.recoil; }
	[[nodiscard]] auto get_multishot() const -> int { return specifications.multishot; }
	[[nodiscard]] auto get_label() const -> std::string_view { return metadata.label; }
	[[nodiscard]] auto get_type() const -> ProjectileType { return projectile.get_type(); }
	[[nodiscard]] auto get_ui_color() const -> int { return static_cast<int>(visual.color); }

	Projectile projectile;
	Ammo ammo{};
	EmitterAttributes emitter{};
	std::optional<EmitterAttributes> secondary_emitter{};

  private:
	struct {
		int id{};
		std::string_view label{};
		std::string_view description{};
	} metadata{};

	Offsets offsets{};
	dir::Direction firing_direction{};
	WeaponSpecifications specifications{};
	util::BitFlags<WeaponAttributes> attributes{};

	struct {
		sf::Sprite sprite{};
		anim::AnimatedSprite animation{};
		sf::Vector2<float> position{};
		sf::Vector2<int> dimensions{};
		std::vector<sf::Vector2<float>> anchor_points{};
		sf::Sprite ui{};
		UIColor color{};
		int texture_lookup{};
	} visual{};

	struct {
		util::BitFlags<WeaponState> state{};
		util::BitFlags<UIFlags> ui{};
	} flags{};

	util::Counter active_projectiles{};
	InventoryState inventory_state{};

	struct {
		util::Cooldown cooldown{};
		util::Cooldown reload{};
		util::Cooldown down_time{};
	} cooldowns{};
};

} // namespace arms
