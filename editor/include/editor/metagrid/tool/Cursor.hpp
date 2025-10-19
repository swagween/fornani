
#pragma once

#include <editor/metagrid/tool/MetagridTool.hpp>

namespace pi {

class Cursor final : public MetagridTool {
  public:
	Cursor(fornani::automa::ServiceProvider& svc) : MetagridTool{svc, MetagridToolType::cursor} { p_label = "Cursor"; }
	void handle_inputs(Room& room, sf::Vector2f const cam, sf::Vector2f const clicked_position) override {}
	void update(sf::Vector2f const position) override { MetagridTool::update(position); }
	void render(sf::RenderWindow& win) override { MetagridTool::render(win); }

  private:
};

} // namespace pi
