
#pragma once

#include <editor/automa/EditorState.hpp>
#include <editor/metagrid/Workspace.hpp>
#include <editor/metagrid/tool/MetagridTool.hpp>

namespace pi {

class Metagrid final : public EditorState {
  public:
	Metagrid(fornani::automa::ServiceProvider& svc);
	EditorStateType run(char** argv) override;
	void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) override;
	void logic() override;
	void render(sf::RenderWindow& win) override;

  private:
	sf::Color m_background_color{21, 21, 20};
	Workspace m_workspace;
	sf::Vector2f m_camera{};
	sf::Vector2f m_dragged_position{};
	sf::Vector2f m_left_clicked_position{};
	sf::Vector2f m_right_clicked_position{};
	sf::Vector2f m_current_mouse_position{};
	std::vector<Room> m_rooms{};
	std::unique_ptr<MetagridTool> m_tool;
	std::optional<Room*> m_highlighted_room{};
};

} // namespace pi
