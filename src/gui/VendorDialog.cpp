#include "VendorDialog.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include "../entities/player/Player.hpp"

namespace gui {

VendorDialog::VendorDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id) : vendor_id(vendor_id) {
	artwork.setTexture(svc.assets.t_vendor_artwork);
	flags.set(VendorDialogStatus::opened);
	artwork.setTextureRect(sf::IntRect{{0, (vendor_id - 1) * svc.constants.screen_dimensions.y}, {svc.constants.screen_dimensions}});
}

void VendorDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) { close(); }
}

void VendorDialog::render(sf::RenderWindow& win) { win.draw(artwork); }

void VendorDialog::close() { flags.reset(VendorDialogStatus::opened); }

} // namespace gui