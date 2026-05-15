
#include <fornani/gui/dialogs/BuilderDialog.hpp>

namespace fornani::gui {

fornani::gui::BuilderDialog::BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id) : IDialog(svc, map, player, vendor_id, "vendor") {}

void BuilderDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	IDialog::update(svc, map, player, context);
	if (early_tick_return()) { return; }
}

void BuilderDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) {
	IDialog::render(svc, win, player, map, shader);
	if (early_render_return()) { return; }
}

void BuilderDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {}

} // namespace fornani::gui
