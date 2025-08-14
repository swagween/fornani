
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

namespace player {
class Player;
}

class Mobile : public Animatable {
  public:
	Mobile(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions = constants::i_cell_vec);
	void face_player(player::Player& player);
	[[nodiscard]] bool player_behind(player::Player& player) const;
	[[nodiscard]] auto get_actual_direction() const -> Direction { return directions.actual; }

  protected:
	shape::Collider collider{};
	struct {
		Direction actual{};
		Direction desired{};
		Direction movement{};
	} directions{};
};
} // namespace fornani
