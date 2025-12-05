
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/utils/Constants.hpp>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/IWorldPositionable.hpp"
#include "fornani/utils/Shape.hpp"
#include <fornani/io/Logger.hpp>
#include <djson/json.hpp>

#include <optional>
#include <string>

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

enum class InspectableAttributes { activate_on_contact, instant };
enum class InspectableFlags { hovered, hovered_trigger, activated, destroy, engaged, can_engage };

class Inspectable : public IWorldPositionable {
  public:
	Inspectable(automa::ServiceProvider& svc, dj::Json const& in, int room, int index);
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player);
	void destroy() { flags.set(InspectableFlags::destroy); }
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void destroy_by_id(int id) {
		if (id == native_id) { flags.set(InspectableFlags::destroy); }
	}
	[[nodiscard]] auto destroyed() const -> bool { return flags.test(InspectableFlags::destroy); }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto get_id() const -> int { return native_id; }

  private:
	shape::Shape bounding_box{};
	sf::Vector2f offset{0.f, -36.f};
	std::string m_label{};
	std::string key{};
	int native_id{};
	int alternates{};
	int current_alt{};
	int m_index{};
	util::BitFlags<InspectableAttributes> attributes{};
	util::BitFlags<InspectableFlags> flags{};
	sf::Sprite sprite;
	anim::Animation animation{};
	anim::Parameters params{0, 14, 18, 0};
	util::Cooldown m_indicator_cooldown{1600};

	io::Logger m_logger{"Inspectable"};
};

} // namespace fornani::entity
