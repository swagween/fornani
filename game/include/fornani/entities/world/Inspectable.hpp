
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
	Inspectable(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, std::string_view key = "", int room_id = 0, int alternates = 0, int native = 0, bool aoc = false, bool instant = false);
	void update(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, dj::Json const& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void destroy() { flags.set(InspectableFlags::destroy); }
	[[nodiscard]] auto destroyed() const -> bool { return flags.test(InspectableFlags::destroy); }
	[[nodiscard]] auto get_id() const -> std::string { return id; }

  private:
	shape::Shape bounding_box{};
	sf::Vector2f offset{0.f, -36.f};
	std::string id{};
	std::string key{};
	int native_id{};
	int alternates{};
	int current_alt{};
	util::BitFlags<InspectableAttributes> attributes{};
	util::BitFlags<InspectableFlags> flags{};
	sf::Sprite sprite;
	anim::Animation animation{};
	anim::Parameters params{0, 14, 18, 0};
	util::Cooldown m_indicator_cooldown{4000};
};

} // namespace fornani::entity
