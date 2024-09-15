
#pragma once

#include "../../utils/Shape.hpp"
#include "../../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}
namespace gui {
class Console;
}
namespace flfx {
class Transition;
}

namespace entity {

enum class PortalAttributes { activate_on_contact, already_open };
enum class PortalState { activated, ready, locked, unlocked };
enum class PortalRenderState { closed, open };

class Portal {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu32 = sf::Vector2<uint32_t>;

	Portal() = default;
	Portal(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, int src, int dest, bool activate_on_contact, bool locked = false, bool already_open = false, int key_id = 0);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos); // for debugging
	void handle_activation(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, int room_id, flfx::Transition& transition);
	void change_states(automa::ServiceProvider& svc, int room_id, flfx::Transition& transition);
	void close() { state = PortalRenderState::closed; }
	[[nodiscard]] auto get_source() const -> int { return meta.source_map_id; }
	[[nodiscard]] auto get_destination() const -> int { return meta.destination_map_id; }
	[[nodiscard]] auto activate_on_contact() const -> bool { return flags.attributes.test(PortalAttributes::activate_on_contact); }
	[[nodiscard]] auto already_open() const -> bool { return flags.attributes.test(PortalAttributes::already_open); }

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
	} meta{};

	struct {
		util::BitFlags<PortalAttributes> attributes{};
		util::BitFlags<PortalState> state{};
	} flags{};

	PortalRenderState state{};

	sf::Sprite sprite{};
	sf::IntRect lookup{};
};

} // namespace entity
