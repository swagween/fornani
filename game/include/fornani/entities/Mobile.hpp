
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
#include <fornani/utils/Direction.hpp>
#include <optional>

namespace fornani {

namespace player {
class Player;
}

enum class MobileState { flip };

class Mobile : public Animatable {
  public:
	Mobile(automa::ServiceProvider& svc, world::Map& map, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec, bool include_collider = true);
	Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);
	virtual void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, bool tick = true);
	void register_collider(world::Map& map, sf::Vector2f dimensions);
	void face_player(player::Player& player);
	[[nodiscard]] bool player_behind(player::Player& player) const;
	[[nodiscard]] auto get_actual_direction() const -> Direction { return directions.actual; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return collider.value().get().get_reference().get_center(); }
	[[nodiscard]] auto get_collider() const -> shape::Collider& { return collider.value().get().get_reference(); }

  protected:
	std::optional<shape::RegisteredCollider> owned_collider;
	std::optional<std::reference_wrapper<shape::RegisteredCollider>> collider;
	void request_flip() { p_flags.set(MobileState::flip); }
	std::unordered_map<std::string, anim::Parameters> m_params;
	anim::Parameters const& get_params(std::string const& key);
	struct {
		Direction actual{};
		Direction desired{};
		Direction movement{};
	} directions{};

	util::BitFlags<MobileState> p_flags{};
};

} // namespace fornani
