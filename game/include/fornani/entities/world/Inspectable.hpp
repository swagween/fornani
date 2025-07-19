
#pragma once

#include <fornani/utils/Constants.hpp>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/IWorldPositionable.hpp"
#include "fornani/utils/Shape.hpp"

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

enum class InspectableAttributes : std::uint8_t { activate_on_contact, instant };
enum class InspectableFlags : std::uint8_t { hovered, hovered_trigger, activated, destroy, engaged };

class Inspectable : public IWorldPositionable {
  public:
	using Vecu32 = sf::Vector2<std::uint32_t>;

	Inspectable(automa::ServiceProvider& svc, dj::Json const& in, int room);
	void update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json const& set);
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
	util::BitFlags<InspectableAttributes> attributes{};
	util::BitFlags<InspectableFlags> flags{};
	sf::Sprite sprite;
	anim::Animation animation{};
	anim::Parameters params{0, 14, 18, 0};
	util::Cooldown m_indicator_cooldown{1600};
};

} // namespace fornani::entity
