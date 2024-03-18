#pragma once

#include "../Entity.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/BitFlags.hpp"
#include "../animation/Animation.hpp"
#include "../../utils/StateFunction.hpp"
#include "../packages/Health.hpp"
#include <string_view>

namespace player {
class Player;
}

namespace enemy {

enum class GeneralFlags { mobile, gravity };
enum class StateFlags { alive, alert, hostile, shot, vulnerable, hurt };
enum class Variant { beast, soldier, elemental, worker };
struct Attributes {
	float base_hp{};
	float base_damage{};
	float speed{};
	float loot_multiplier{};
	sf::Vector2<int> drop_range{};
};

struct Flags {
	util::BitFlags<GeneralFlags> general{};
	util::BitFlags<StateFlags> state{};
};

class Enemy : public entity::Entity {
  public:
	Enemy() = default;
	Enemy(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc, world::Map& map) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	virtual void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player){};
	[[nodiscard]] auto get_attributes() const -> Attributes { return attributes; }
	[[nodiscard]] auto get_flags() const -> Flags { return flags; }
	[[nodiscard]] auto get_collider() const -> shape::Collider { return collider; }
	[[nodiscard]] auto died() const -> bool { return health.is_dead(); }
	void set_position(sf::Vector2<float> pos) {
		collider.physics.position = pos;
		collider.sync_components();
	}
	void hurt() { flags.state.set(StateFlags::hurt); }

	entity::Health health{};
	anim::Animation animation{};

  protected:
	std::string_view label{};
	shape::Collider collider{};
	std::vector<anim::Parameters> animation_parameters{};
	Flags flags{};
	Attributes attributes{};

	struct {
		int id{};
		std::string_view variant{};
	} metadata{};

	struct {
		std::vector<shape::Shape> hurtbox_atlas{};
		std::vector<shape::Shape> hurtboxes{};
		sf::Vector2<float> alert_range{};
		sf::Vector2<float> hostile_range{};
	} physical{};
};

} // namespace enemy