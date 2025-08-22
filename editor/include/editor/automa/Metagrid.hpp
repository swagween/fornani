
#pragma once

#include <editor/automa/EditorState.hpp>
#include <editor/metagrid/Workspace.hpp>

namespace pi {

class Metagrid final : public EditorState {
  public:
	Metagrid(fornani::automa::ServiceProvider& svc);
	EditorStateType run() override;
	void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) override;
	void logic() override;
	void render(sf::RenderWindow& win) override;

  private:
	sf::Color m_background_color{21, 21, 20};
	Workspace m_workspace;
	sf::Vector2f m_camera{};
	sf::Vector2f m_right_clicked_position{};
	sf::Vector2f m_current_mouse_position{};
	std::vector<Room> m_rooms{};

	std::optional<std::unique_ptr<Room>> m_highlighted_room{};
};

} // namespace pi
