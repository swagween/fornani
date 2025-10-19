
#pragma once

#include <editor/automa/EditorState.hpp>
#include <editor/dialogue/DialogueSuite.hpp>
#include <editor/metagrid/tool/MetagridTool.hpp>

namespace pi {

class DialogueEditor final : public EditorState {
  public:
	DialogueEditor(fornani::automa::ServiceProvider& svc);
	EditorStateType run(char** argv) override;
	void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) override;
	void logic() override;
	void render(sf::RenderWindow& win) override;

  private:
	void save();
	sf::Color m_background_color;
	sf::Vector2f m_dragged_position{};
	sf::Vector2f m_left_clicked_position{};
	sf::Vector2f m_right_clicked_position{};
	sf::Vector2f m_current_mouse_position{};
	std::unique_ptr<MetagridTool> m_tool;
	std::optional<DialogueSuite> m_suite{};
	std::optional<Node> m_current_node{};
	dj::Json* m_data;

	sf::Vector2f m_camera{-100.f, -100.f};
};

} // namespace pi
