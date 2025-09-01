
#pragma once

#include <fornani/io/Logger.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace pi {

enum class EditorStateType : std::uint8_t { editor, metagrid, dialogue_editor };
enum class PressedKeys : std::uint8_t { control, shift, mouse_left, mouse_middle, mouse_right, space };

class EditorState {
  public:
	EditorState(fornani::automa::ServiceProvider& svc);
	virtual EditorStateType run(char** argv) { return EditorStateType::editor; }
	virtual void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {}
	virtual void logic() {}
	virtual void render(sf::RenderWindow& win);

  protected:
	fornani::util::BitFlags<PressedKeys> pressed_keys{};
	fornani::automa::ServiceProvider* p_services;
	sf::Clock p_delta_clock{};

	EditorStateType p_target_state{};

	sf::RectangleShape p_wallpaper{};

	fornani::io::Logger p_logger{"Editor"};

  private:
};

} // namespace pi
