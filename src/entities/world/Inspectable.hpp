#pragma once

#include <djson/json.hpp>
#include <string>
#include "../../utils/Shape.hpp"
#include "../animation/Animation.hpp"

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

enum class InspectableFlags { hovered, hovered_trigger, activated, destroy, engaged };

class Inspectable {
  public:
	using Vec = sf::Vector2<float>;
	using Vecu16 = sf::Vector2<uint32_t>;

	Inspectable() = default;
	Inspectable(automa::ServiceProvider& svc, Vecu16 dim, Vecu16 pos, std::string_view key, int room_id);
	void update(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, dj::Json& set);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos);
	void destroy() { flags.set(InspectableFlags::destroy); } 
	[[nodiscard]] auto destroyed() const -> bool { return flags.test(InspectableFlags::destroy); }
	[[nodiscard]] auto get_id() const -> std::string { return id; }

	Vec dimensions{};
	Vec position{};
	Vec offset{0.f, -36.f};
	Vecu16 scaled_dimensions{};
	Vecu16 scaled_position{};
	shape::Shape bounding_box{};

	bool activated{};
	bool activate_on_contact{};

	std::string key{};

  private:
	std::string id{};
	util::BitFlags<InspectableFlags> flags{};
	sf::Sprite sprite{};
	anim::Animation animation{};
	anim::Parameters params{0, 14, 18, 0};
};

} // namespace entity
