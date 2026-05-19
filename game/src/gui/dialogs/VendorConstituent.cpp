
#include <fornani/gui/dialogs/VendorConstituent.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

VendorConstituent::VendorConstituent(automa::ServiceProvider& svc, std::string_view label, sf::IntRect lookup, int speed, util::InterpolationType type)
	: Drawable(svc, "vendor_gizmo"), path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "vendor_" + std::string{label}, speed, type} {
	set_texture_rect(lookup);
}

void VendorConstituent::update() {
	path.update();
	set_position(path.get_position());
}

void VendorConstituent::render(sf::RenderWindow& win, LightShader& shader, Palette& palette) { shader.submit(win, palette, get_sprite()); }

} // namespace fornani::gui
