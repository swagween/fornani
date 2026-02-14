
#pragma once

#include <fornani/io/Logger.hpp>
#include <optional>
#include "fornani/audio/Soundboard.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/weapon/Ammo.hpp"
#include "fornani/weapon/Projectile.hpp"

namespace fornani::arms {

enum class WeaponState { unlocked, equipped, reloading };
enum class WeaponAttributes { automatic, no_reload };
enum class InventoryState { reserve, hotbar };
enum class UIFlags { selected };

struct WeaponSpecifications {
	int cooldown_time{};
	int reload_time{};
	int multishot{};
	float recoil{};
};

struct Offsets {
	struct {
		sf::Vector2f global{};
		sf::Vector2f barrel{};
	} render{};
	struct {
		sf::Vector2f barrel{};
	} gameplay{};
};

struct WeaponModifiers {
	float reload_multiplier{1.f};
};

struct EmitterAttributes {
	sf::Vector2f dimensions{};
	std::string type{};
	sf::Color color{};
};

class Weapon : public Animatable {
  public:
	explicit Weapon(automa::ServiceProvider& svc, std::string_view tag, bool enemy = false);

	void update(automa::ServiceProvider& svc, Direction to_direction);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void render_ui(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f position);

	void equip();
	void unequip();
	void unlock();
	void lock();
	void shoot();
	void shoot(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f target);
	void decrement_projectiles();

	bool is_equipped() const;
	bool is_unlocked() const;
	bool cooling_down() const;
	bool can_shoot() const;

	void set_position(sf::Vector2f pos);
	void force_position(sf::Vector2f pos);
	void set_barrel_point(sf::Vector2f point);
	void set_orientation(Direction to_direction);
	void set_team(Team team);
	void set_firing_direction(Direction to_direction);
	void reset();

	void set_hotbar() { inventory_state = InventoryState::hotbar; }
	void set_reserved() { inventory_state = InventoryState::reserve; }
	void select() { flags.ui.set(UIFlags::selected); }
	void deselect() { flags.ui.reset(UIFlags::selected); }
	void set_reload_multiplier(float const to) { m_modifiers.reload_multiplier = to; }
	void reduce_reload_time(float percentage);

	[[nodiscard]] auto selected() const -> bool { return flags.ui.test(UIFlags::selected); }
	[[nodiscard]] auto shot() const -> bool { return cooldowns.cooldown.just_started(); }
	[[nodiscard]] auto automatic() const -> bool { return attributes.test(WeaponAttributes::automatic); }
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_sound_id() const -> int { return static_cast<int>(m_audio.shoot); }
	[[nodiscard]] auto get_active_projectiles() const -> int { return active_projectiles.get_count(); }
	[[nodiscard]] auto get_inventory_state() const -> int { return static_cast<int>(inventory_state); }
	[[nodiscard]] auto get_ui_position() const -> sf::Vector2f { return visual.ui.getPosition(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return metadata.description; }
	[[nodiscard]] auto multishot() const -> bool { return specifications.multishot != 0; }
	[[nodiscard]] auto get_barrel_point() const -> sf::Vector2f { return offsets.gameplay.barrel; }
	[[nodiscard]] auto get_cooldown() const -> int { return cooldowns.cooldown.get(); }
	[[nodiscard]] auto get_firing_direction() & -> Direction& { return firing_direction; }
	[[nodiscard]] auto get_global_offset() const -> sf::Vector2f { return offsets.render.global; };
	[[nodiscard]] auto get_recoil() const -> float { return specifications.recoil; }
	[[nodiscard]] auto get_multishot() const -> int { return specifications.multishot; }
	[[nodiscard]] auto get_tag() const -> std::string_view { return metadata.tag; }
	[[nodiscard]] auto get_audio_tag() const -> std::string_view { return metadata.audio_tag; }
	[[nodiscard]] auto get_label() const -> std::string_view { return metadata.label; }
	[[nodiscard]] auto get_type() const -> ProjectileType { return projectile.get_type(); }
	[[nodiscard]] auto get_recoil_force() const -> sf::Vector2f { return sf::Vector2f{-specifications.recoil, 0.f}; }
	[[nodiscard]] auto get_reload() const -> util::Cooldown { return cooldowns.reload; }
	[[nodiscard]] auto get_reload_time() const -> float { return cooldowns.reload.get_native_time() * m_modifiers.reload_multiplier; }
	[[nodiscard]] auto get_reload_multiplier() const -> float { return m_modifiers.reload_multiplier; }
	[[nodiscard]] auto get_reload_time_inverse_normalized() const -> float { return cooldowns.reload.get_inverse_normalized() * m_modifiers.reload_multiplier; }

	Projectile projectile;
	Ammo ammo{};
	EmitterAttributes emitter{};
	std::optional<EmitterAttributes> secondary_emitter{};

  private:
	struct {
		int id{};
		std::string tag{};
		std::string audio_tag{};
		std::string label{};
		std::string description{};
	} metadata{};

	Offsets offsets{};
	Direction firing_direction{};
	WeaponSpecifications specifications{};
	util::BitFlags<WeaponAttributes> attributes{};

	struct {
		components::PhysicsComponent physics{};
		components::SteeringBehavior steering{};
		sf::Vector2f final_position{};
	} physical{};

	struct {
		sf::Sprite ui;
		sf::Vector2f position{};
		sf::Vector2<int> dimensions{};
		std::vector<sf::Vector2f> anchor_points{};
		int texture_lookup{};
	} visual;

	struct {
		util::BitFlags<WeaponState> state{};
		util::BitFlags<UIFlags> ui{};
	} flags{};

	struct {
		audio::Weapon shoot{};
	} m_audio{};

	util::Counter active_projectiles{};
	InventoryState inventory_state{};

	struct {
		util::Cooldown cooldown{};
		util::Cooldown reload{};
		util::Cooldown shoot_effect{};
	} cooldowns{};

	WeaponModifiers m_modifiers{};

	io::Logger m_logger{"Arms"};
};

} // namespace fornani::arms
