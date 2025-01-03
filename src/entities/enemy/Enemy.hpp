#pragma once

#include "../Entity.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/BitFlags.hpp"
#include "../animation/Animation.hpp"
#include "../../utils/StateFunction.hpp"
#include "../../utils/Math.hpp"
#include "../packages/Health.hpp"
#include "../packages/WeaponPackage.hpp"
#include "../packages/Caution.hpp"
#include "../packages/Attack.hpp"
#include "../packages/Shockwave.hpp"
#include "../packages/FloatingPart.hpp"
#include "../player/Indicator.hpp"
#include "../../audio/Soundboard.hpp"
#include <string_view>
#include <iostream>

namespace player {
class Player;
}

namespace world {
class Map;
}

namespace arns {
class Projectile;
}

namespace enemy {

enum class GeneralFlags { mobile, gravity, player_collision, hurt_on_contact, map_collision, post_death_render, no_loot, custom_sounds, uncrushable, foreground, spawned, transcendent, rare_drops, permadeath };
enum class StateFlags { alive, alert, hostile, shot, vulnerable, hurt, shaking, special_death_mode, invisible };
enum class Triggers { hostile, alert };
enum class Variant { beast, soldier, elemental, worker, guardian };
struct Attributes {
	float base_hp{};
	float base_damage{};
	float speed{};
	float loot_multiplier{};
	sf::Vector2<int> drop_range{};
	int rare_drop_id{};
	int respawn_distance{};
};

struct Flags {
	util::BitFlags<GeneralFlags> general{};
	util::BitFlags<StateFlags> state{};
	util::BitFlags<Triggers> triggers{};
};

class Enemy : public entity::Entity {
  public:
	Enemy() = default;
	virtual ~Enemy() {}
	Enemy(automa::ServiceProvider& svc, std::string_view label, bool spawned = false, int variant = 0, sf::Vector2<int> start_direction = {-1, 0});
	void set_external_id(std::pair<int, sf::Vector2<int>> code);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	virtual void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player){};
	virtual void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam){};
	virtual void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam){};
	void handle_player_collision(player::Player& player) const;
	void handle_collision(shape::Collider& other);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void on_crush(world::Map& map);
	[[nodiscard]] auto hostile() const -> bool { return flags.state.test(StateFlags::hostile); }
	[[nodiscard]] auto alert() const -> bool { return flags.state.test(StateFlags::alert); }
	[[nodiscard]] auto is_hurt() const -> bool { return flags.state.test(StateFlags::hurt); }
	[[nodiscard]] auto hostility_triggered() const -> bool { return flags.triggers.test(Triggers::hostile); }
	[[nodiscard]] auto alertness_triggered() const -> bool { return flags.triggers.test(Triggers::alert); }
	[[nodiscard]] auto get_attributes() const -> Attributes { return attributes; }
	[[nodiscard]] auto get_flags() const -> Flags { return flags; }
	[[nodiscard]] auto get_external_id() const -> int { return metadata.external_id; }
	[[nodiscard]] auto get_collider() -> shape::Collider& { return collider; }
	[[nodiscard]] auto get_secondary_collider() -> shape::Collider& { return secondary_collider; }
	[[nodiscard]] auto died() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto just_died() const -> bool { return health.is_dead() && post_death.get_cooldown() == afterlife; }
	[[nodiscard]] auto gone() const -> bool { return post_death.is_complete(); }
	[[nodiscard]] auto player_collision() const -> bool { return flags.general.test(GeneralFlags::player_collision); }
	[[nodiscard]] auto spawn_loot() const -> bool { return !flags.general.test(GeneralFlags::no_loot); }
	[[nodiscard]] auto is_foreground() const -> bool { return flags.general.test(GeneralFlags::foreground); }
	[[nodiscard]] auto is_transcendent() const -> bool { return flags.general.test(GeneralFlags::transcendent); }
	[[nodiscard]] auto permadeath() const -> bool { return flags.general.test(GeneralFlags::permadeath); }
	[[nodiscard]] bool player_behind(player::Player& player) const;
	void set_position(sf::Vector2<float> pos) {
		collider.physics.position = pos;
		collider.sync_components();
		health_indicator.set_position(pos);
	}
	void set_position_from_scaled(sf::Vector2<float> pos);
	void hurt() { flags.state.set(StateFlags::hurt); }
	void shake() { energy = hit_energy; }
	void stop_shaking() { flags.state.reset(StateFlags::shaking); }

	entity::Health health{};
	player::Indicator health_indicator{};
	anim::Animation animation{};
	struct {
		dir::Direction actual{};
		dir::Direction desired{};
		dir::Direction movement{};
	} directions{};

  protected:
	std::string_view label{};
	shape::Collider collider{};
	shape::Collider secondary_collider{};
	std::vector<anim::Parameters> animation_parameters{};
	Flags flags{};
	Attributes attributes{};
	util::Cooldown post_death{};
	int afterlife{200};

	util::Cooldown hurt_effect{};

	struct {
		int id{};
		std::string_view variant{};
		int external_id{};
	} metadata{};

	struct {
		std::vector<shape::Shape> hurtbox_atlas{};
		std::vector<shape::Shape> hurtboxes{};
		shape::Shape alert_range{};
		shape::Shape hostile_range{};
	} physical{};

	struct {
		int effect_type{};
		int effect_size{};
	} visual{};

	struct {
		audio::Enemy hit_flag{};
		util::Cooldown hurt_sound_cooldown{24};
	} sound{};

	// shake
	float energy{};
	float dampen{0.1f};
	float hit_energy{6.f};
};

} // namespace enemy