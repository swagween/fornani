
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

namespace entity {

enum class PortalAttributes { activate_on_contact };
enum class PortalState { activated, ready, locked, unlocked };

class Portal {

  public:
	using Vec = sf::Vector2<float>;
	using Vecu32 = sf::Vector2<uint32_t>;

	Portal() = default;
	Portal(automa::ServiceProvider& svc, Vecu32 dim, Vecu32 pos, int src, int dest, bool activate_on_contact, bool locked = false, int key_id = 0);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, Vec campos); // for debugging
	void handle_activation(automa::ServiceProvider& svc, player::Player& player, gui::Console& console, int room_id, bool& fade_out, bool& done);
	void change_states(automa::ServiceProvider& svc, int room_id, bool& fade_out, bool& done) const;
	[[nodiscard]] auto get_source() const -> int { return meta.source_map_id; }
	[[nodiscard]] auto get_destination() const -> int { return meta.destination_map_id; }
	[[nodiscard]] auto activate_on_contact() const -> bool { return flags.attributes.test(PortalAttributes::activate_on_contact); }

	Vec dimensions{};
	Vec position{};
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
};

} // namespace entity
