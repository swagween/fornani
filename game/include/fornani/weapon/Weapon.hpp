
#pragma once

#include <optional>
#include "Ammo.hpp"
#include "Projectile.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::arms {

enum class WeaponState : std::uint8_t { unlocked, equipped, reloading };
enum class WeaponAttributes : std::uint8_t { automatic };
enum class InventoryState : std::uint8_t { reserve, hotbar };
enum class UIFlags : std::uint8_t { selected };
enum class UIColor : std::uint8_t { white, periwinkle, green, orange, fucshia, purple, mythic };

struct WeaponSpecifications {
	int cooldown_time{};
	int multishot{};
	float recoil{};
};

struct Offsets {
	struct {
		sf::Vector2<float> global{};
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
	Weapon(automa::ServiceProvider& svc, int id, bool enemy = false);

	void update(automa::ServiceProvider& svc, Direction to_direction);
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
	void force_position(sf::Vector2<float> pos);
	void set_barrel_point(sf::Vector2<float> point);
	void set_orientation(Direction to_direction);
	void set_team(Team team);
	void set_firing_direction(Direction to_direction);
	void reset();

	void set_hotbar() { inventory_state = InventoryState::hotbar; }
	void set_reserved() { inventory_state = InventoryState::reserve; }
	void select() { flags.ui.set(UIFlags::selected); }
	void deselect() { flags.ui.reset(UIFlags::selected); }

	[[nodiscard]] auto selected() const -> bool { return flags.ui.test(UIFlags::selected); }
	[[nodiscard]] auto shot() const -> bool { return cooldowns.cooldown.just_started(); }
	[[nodiscard]] auto automatic() const -> bool { return attributes.test(WeaponAttributes::automatic); }
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_sound_id() const -> int { return audio.shoot; }
	[[nodiscard]] auto get_active_projectiles() const -> int { return active_projectiles.get_count(); }
	[[nodiscard]] auto get_inventory_state() const -> int { return static_cast<int>(inventory_state); }
	[[nodiscard]] auto get_ui_position() const -> sf::Vector2<float> { return visual.ui.getPosition(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return metadata.description; }
	[[nodiscard]] auto multishot() const -> bool { return specifications.multishot != 0; }
	[[nodiscard]] auto get_barrel_point() const -> sf::Vector2<float> { return offsets.gameplay.barrel; }
	[[nodiscard]] auto get_cooldown() const -> int { return cooldowns.cooldown.get_cooldown(); }
	[[nodiscard]] auto get_firing_direction() & -> Direction& { return firing_direction; }
	[[nodiscard]] auto get_global_offset() const -> sf::Vector2<float> { return offsets.render.global; };
	[[nodiscard]] auto get_recoil() const -> float { return specifications.recoil; }
	[[nodiscard]] auto get_multishot() const -> int { return specifications.multishot; }
	[[nodiscard]] auto get_label() const -> std::string_view { return metadata.label; }
	[[nodiscard]] auto get_type() const -> ProjectileType { return projectile.get_type(); }
	[[nodiscard]] auto get_ui_color() const -> int { return static_cast<int>(visual.color); }
	[[nodiscard]] auto get_recoil_force() const -> sf::Vector2<float> { return sf::Vector2<float>{-specifications.recoil, 0.f}; }

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
	Direction firing_direction{};
	WeaponSpecifications specifications{};
	util::BitFlags<WeaponAttributes> attributes{};

	struct {
		components::PhysicsComponent physics{};
		components::SteeringBehavior steering{};
		sf::Vector2<float> final_position{};
	} physical{};

	struct {
		sf::Sprite sprite;
		sf::Sprite ui;
		sf::Vector2<float> position{};
		sf::Vector2<int> dimensions{};
		std::vector<sf::Vector2<float>> anchor_points{};
		UIColor color{};
		int texture_lookup{};
	} visual;

	struct {
		util::BitFlags<WeaponState> state{};
		util::BitFlags<UIFlags> ui{};
	} flags{};

	struct {
		int shoot{};
	} audio{};

	util::Counter active_projectiles{};
	InventoryState inventory_state{};

	struct {
		util::Cooldown cooldown{};
		util::Cooldown reload{};
		util::Cooldown down_time{};
	} cooldowns{};
};

} // namespace fornani::arms
