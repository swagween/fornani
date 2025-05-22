
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shape.hpp"

#include <optional>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}
namespace fornani::gui {
class Console;
}
namespace fornani::graphics {
class Transition;
}

namespace fornani::entity {

enum class PortalAttributes : std::uint8_t { activate_on_contact, already_open };
enum class PortalState : std::uint8_t { activated, ready, locked, unlocked };
enum class PortalRenderState : std::uint8_t { closed, open };
enum class PortalOrientation : std::uint8_t { top, bottom, left, right, central };

class Portal {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu32 = sf::Vector2<std::uint32_t>;

	Portal(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, int src, int dest, bool activate_on_contact, bool locked = false, bool already_open = false, int key_id = 0, int style = 0, sf::Vector2<int> map_dim = {});
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos);
	void handle_activation(automa::ServiceProvider& svc, player::Player& player, std::optional<std::unique_ptr<gui::Console>>& console, int room_id, graphics::Transition& transition);
	void change_states(automa::ServiceProvider& svc, int room_id, graphics::Transition& transition);
	void close() { state = PortalRenderState::closed; }
	[[nodiscard]] auto get_source() const -> int { return meta.source_map_id; }
	[[nodiscard]] auto get_destination() const -> int { return meta.destination_map_id; }
	[[nodiscard]] auto activate_on_contact() const -> bool { return flags.attributes.test(PortalAttributes::activate_on_contact); }
	[[nodiscard]] auto already_open() const -> bool { return flags.attributes.test(PortalAttributes::already_open); }
	[[nodiscard]] auto is_top_or_bottom() const -> bool { return is_bottom() || is_top(); }
	[[nodiscard]] auto is_left_or_right() const -> bool { return is_left() || is_right(); }
	[[nodiscard]] auto is_bottom() const -> bool { return meta.orientation == PortalOrientation::bottom; }
	[[nodiscard]] auto is_top() const -> bool { return meta.orientation == PortalOrientation::top; }
	[[nodiscard]] auto is_left() const -> bool { return meta.orientation == PortalOrientation::left; }
	[[nodiscard]] auto is_right() const -> bool { return meta.orientation == PortalOrientation::right; }

	Vec dimensions{};
	Vec position{};
	Vec offset{0.f, 32.f};
	Vecu32 scaled_dimensions{};
	Vecu32 scaled_position{};
	shape::Shape bounding_box{};

  private:
	struct {
		int source_map_id{};	  // where to place the player once they arrive (check all portals in the destination until you match)
		int destination_map_id{}; // where to send the player
		int key_id{};
		PortalOrientation orientation{};
	} meta{};

	struct {
		util::BitFlags<PortalAttributes> attributes{};
		util::BitFlags<PortalState> state{};
	} flags{};

	PortalRenderState state{};

	sf::Sprite sprite;
	sf::IntRect lookup{};
};

} // namespace fornani::entity
