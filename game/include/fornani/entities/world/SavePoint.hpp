
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/IWorldPositionable.hpp>
#include <optional>
#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/Shape.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {
class Console;
}

namespace fornani::entity {

class SavePoint : public IWorldPositionable, Animatable {

  public:
	using Vecu32 = sf::Vector2<std::uint32_t>;

	explicit SavePoint(automa::ServiceProvider& svc, int id, Vecu32 position);

	void update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void save(automa::ServiceProvider& svc, player::Player& player);

  private:
	anim::Parameters m_anim_params;
	shape::Shape bounding_box{};
	shape::Shape proximity_box{};
	vfx::Sparkler sparkler{};
	bool activated{};
	bool can_activate{true};
	int m_id{};
	int intensity{};

	io::Logger m_logger{"Entity"};
};

} // namespace fornani::entity
