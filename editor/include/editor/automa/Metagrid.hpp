
#pragma once

#include <editor/automa/EditorState.hpp>
#include <editor/metagrid/Workspace.hpp>
#include <editor/metagrid/tool/MetagridTool.hpp>

namespace pi {

enum class MetagridFlags { context_menu, hide_cell };

class Metagrid final : public EditorState {
  public:
	Metagrid(fornani::automa::ServiceProvider& svc, EditorContext& ctx);
	EditorStateType run(char** argv) override;
	void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) override;
	void logic() override;
	void render(sf::RenderWindow& win) override;

  private:
	sf::Color m_background_color;
	Workspace m_workspace;
	sf::RectangleShape m_current_cell{};
	std::vector<Room> m_rooms{};
	std::unique_ptr<MetagridTool> m_tool;
	std::optional<Room*> m_highlighted_room{};
	fornani::util::BitFlags<MetagridFlags> m_flags{};
};

} // namespace pi
