
#pragma once

#include <fornani/gui/dialogs/IDialog.hpp>

namespace fornani::gui {

class BuilderDialog final : public IDialog {
  public:
	BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) override;
	void refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map);

  private:
};

} // namespace fornani::gui
