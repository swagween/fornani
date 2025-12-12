
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

namespace player {
class Player;
}

enum class MobileState { flip };

class Mobile : public Animatable {
  public:
	Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);
	virtual void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void face_player(player::Player& player);
	[[nodiscard]] bool player_behind(player::Player& player) const;
	[[nodiscard]] auto get_actual_direction() const -> Direction { return directions.actual; }
	[[nodiscard]] auto get_global_center() const -> sf::Vector2f { return collider.get_center(); }
	[[nodiscard]] auto get_collider() -> shape::Collider& { return collider; }

  protected:
	shape::Collider collider{};
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
