#pragma once

#include "../Entity.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/BitFlags.hpp"
#include "../behavior/Animation.hpp"
#include "../../utils/StateFunction.hpp"
#include <string_view>

namespace enemy {

enum class GeneralFlags { mobile, gravity };
enum class StateFlags { alive, alert, hostile };
enum class Variant { beast, soldier, elemental, worker };
struct Attributes {
	float base_hp{};
	float base_damage{};
	float speed{};
	float loot_multiplier{};
};

class Enemy : public entity::Entity {
  public:
	Enemy() = default;
	Enemy(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	virtual void unique_update(automa::ServiceProvider& svc, world::Map& map){};
	[[nodiscard]] auto get_attributes() const -> Attributes { return attributes; }
	[[nodiscard]] auto get_collider() const -> shape::Collider { return collider; }
	void set_position(sf::Vector2<float> pos) { collider.physics.position = pos; }

  protected:
	std::string_view label{};
	util::BitFlags<GeneralFlags> general_flags{};
	shape::Collider collider{};
	anim::Animation animation{};
	std::vector<anim::Parameters> animation_parameters{};

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

	Attributes attributes{};
};

} // namespace enemy