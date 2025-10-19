
#pragma once

#include <editor/metagrid/tool/MetagridTool.hpp>

namespace pi {

class Move final : public MetagridTool {
  public:
	Move(fornani::automa::ServiceProvider& svc) : MetagridTool{svc, MetagridToolType::move} { p_label = "Move"; }
	void handle_inputs(Room& room, sf::Vector2f const cam, sf::Vector2f const clicked_position) override {
		auto clicked_offset = clicked_position - p_original_position;
		room.set_position(get_workspace_coordinates(cam) - sf::Vector2i{clicked_offset / spacing_v});
	}
	void update(sf::Vector2f const position) override { MetagridTool::update(position); }
	void render(sf::RenderWindow& win) override { MetagridTool::render(win); }

  private:
};

} // namespace pi
