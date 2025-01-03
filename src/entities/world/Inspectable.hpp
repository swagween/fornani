#pragma once

#include <djson/json.hpp>
#include <string>
#include <optional>
#include "../../utils/Shape.hpp"
#include "../animation/Animation.hpp"
#include "../../utils/Circuit.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {
class Console;
}

namespace entity {

enum class InspectableAttributes { activate_on_contact };
enum class InspectableFlags { hovered, hovered_trigger, activated, destroy, engaged };

class Inspectable {
  public:
	using Vec = sf::Vector2<float>;
	using Vecu32 = sf::Vector2<uint32_t>;

	Inspectable(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, std::string_view key = "", int room_id = 0, int alternates = 0, int native = 0, bool aoc = false);
	void update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, dj::Json& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos);
	void destroy() { flags.set(InspectableFlags::destroy); } 
	[[nodiscard]] auto destroyed() const -> bool { return flags.test(InspectableFlags::destroy); }
	[[nodiscard]] auto get_id() const -> std::string { return id; }

	Vec dimensions{};
	Vec position{};
	Vec offset{0.f, -36.f};
	Vecu32 scaled_dimensions{};
	Vecu32 scaled_position{};
	shape::Shape bounding_box{};

	std::string key{};
	int alternates{};
	int current_alt{};

  private:
	std::string id{};
	int native_id{};
	util::BitFlags<InspectableAttributes> attributes{};
	util::BitFlags<InspectableFlags> flags{};
	sf::Sprite sprite;
	anim::Animation animation{};
	anim::Parameters params{0, 14, 18, 0};
};

} // namespace entity
