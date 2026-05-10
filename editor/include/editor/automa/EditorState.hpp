
#pragma once

#include <fornani/events/Subscription.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace pi {

struct EditorContext;

struct EditorInput {
	bool clicked{};
	bool held{};
	bool released{};

	void press() {
		clicked = true;
		held = true;
	}
	void release() {
		released = true;
		held = false;
	}
	void flush() {
		clicked = false;
		released = false;
	}
	void reset() {
		clicked = false;
		released = false;
		held = false;
	}
};

enum class EditorStateType { editor, metagrid, dialogue_editor };

class EditorState {
  public:
	EditorState(fornani::automa::ServiceProvider& svc, EditorContext& ctx);
	virtual EditorStateType run(char** argv);
	virtual void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win);
	virtual void logic();
	virtual void render(sf::RenderWindow& win);

	bool create_new_room();

	[[nodiscard]] auto control_held() const -> bool { return p_control.held; }
	[[nodiscard]] auto shift_held() const -> bool { return p_shift.held; }
	[[nodiscard]] auto left_mouse_clicked() const -> bool { return p_left_mouse.clicked; }
	[[nodiscard]] auto right_mouse_clicked() const -> bool { return p_right_mouse.clicked; }
	[[nodiscard]] auto left_mouse_held() const -> bool { return p_left_mouse.held; }
	[[nodiscard]] auto right_mouse_held() const -> bool { return p_right_mouse.held; }
	[[nodiscard]] auto any_mouse_clicked() const -> bool { return left_mouse_clicked() || right_mouse_clicked(); }
	[[nodiscard]] auto any_mouse_held() const -> bool { return left_mouse_held() || right_mouse_held(); }
	[[nodiscard]] auto any_mouse_released() const -> bool { return p_left_mouse.released || p_right_mouse.released; }

  protected:
	EditorContext* p_context;

	struct {
		fornani::util::Cooldown left_click{8};
		fornani::util::Cooldown left_release{8};
		fornani::util::Cooldown right_click{8};
		fornani::util::Cooldown right_release{8};
	} p_mouse_cooldowns{};

	char regbuffer[128] = "";
	char roombuffer[128] = "";
	int width{1};
	int height{1};
	int p_new_id{};

	EditorInput p_left_mouse{};
	EditorInput p_right_mouse{};
	EditorInput p_shift{};
	EditorInput p_control{};
	EditorInput p_alt{};

	sf::Vector2f p_current_mouse_position{};
	sf::Vector2f p_left_clicked_position{};
	sf::Vector2f p_right_clicked_position{};
	sf::Vector2f p_dragged_position{};
	sf::Vector2f p_camera{};
	fornani::automa::ServiceProvider* p_services;
	sf::Clock p_delta_clock{};
	std::shared_ptr<fornani::Slot const> p_slot{std::make_shared<fornani::Slot const>()};

	EditorStateType p_target_state{};

	sf::RectangleShape p_wallpaper{};

	fornani::io::Logger p_logger{"Editor"};

  private:
};

} // namespace pi
