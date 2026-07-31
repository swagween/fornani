
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/TransparentStringHash.hpp>
#include <optional>

namespace fornani {

namespace player {
class Player;
}

enum class MobileState { flip };

struct MobileSound {
	int frame{};
	std::string tag{};
};

class Mobile : public Polymorphic {
  public:
	Mobile(automa::ServiceProvider& svc, world::Map& map, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec, bool include_collider = true);
	Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);
	virtual void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, bool tick = true);
	void register_collider(world::Map& map, sf::Vector2f dimensions);
	void face_player(player::Player& player);
	void face_movement() { directions.desired.set(directions.movement.lnr); }
	void set_direction(SimpleDirection to);
	void set_desired_direction(SimpleDirection to);
	void push_and_set_animation(std::string_view tag, anim::Parameters params) { p_animatable.push_and_set_animation(tag, params); }
	void push_animation(std::string_view tag, anim::Parameters params) { p_animatable.push_animation(tag, params); }
	void set_animation(std::string_view to) { p_animatable.set_animation(to); }
	[[nodiscard]] bool player_behind(player::Player& player) const;
	[[nodiscard]] auto get_desired_direction() const -> Direction { return directions.desired; }
	[[nodiscard]] auto get_actual_direction() const -> Direction { return directions.actual; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return collider.value().get().get_reference().get_center(); }
	[[nodiscard]] auto get_collider() const -> shape::Collider& { return collider.value().get().get_reference(); }

  protected:
	Animatable p_animatable;
	std::optional<shape::RegisteredCollider> owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> collider;
	std::unordered_map<std::string, MobileSound, TransparentHash, TransparentEqual> p_sounds{};
	void request_flip() { p_flags.set(MobileState::flip); }
	anim::Parameters const& get_params(std::string const& key);
	struct {
		Direction actual{};
		Direction desired{};
		Direction movement{};
		Direction input{};
	} directions{};

	util::BitFlags<MobileState> p_flags{};
};

} // namespace fornani
