#pragma once
#include <SFML/Graphics.hpp>
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}
namespace player {
class Player;
}

namespace gui {
enum class VendorDialogStatus { opened };
class VendorDialog {
  public:
	VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(sf::RenderWindow& win);
	void close();
	[[nodiscard]] auto is_open() const -> bool { return flags.test(VendorDialogStatus::opened); }
  private:
	util::BitFlags<VendorDialogStatus> flags{};
	sf::Sprite artwork{};
	int vendor_id{};
};

} // namespace gui