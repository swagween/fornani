#pragma once

#include <utility>

#include "Window.hpp"

namespace fornani::sys {

struct ApplicationOptions {
	Window::Properties win_props;
	std::optional<Window::OptionalProperties> opt_win_props;
};

class Application {
  public:
	Application();
	virtual ~Application() = default;

	/**
	 * @brief Prepares the application for execution
	 */
	virtual bool prepare(ApplicationOptions const& options);

	/**
	 * @brief Updates the application
	 * @param delta_time The time since the last update
	 */
	virtual void update(float delta_time);

	/**
	 * @brief Handles cleaning up the application
	 */
	virtual void release();

	/**
	 * @brief Handles resizing of the window
	 * @param new_extent New extent of the window
	 */
	virtual bool resize(sf::Vector2u new_extent);

	/**
	 * @brief Handles input events of the window
	 * @param input_event The input event object
	 */
	// virtual void input_event(InputEvent const& input_event);

	[[nodiscard]] std::string const& get_name() const;

	void set_name(std::string const& name);

	[[nodiscard]] bool should_close() const { return requested_close; }

	// request the app to close
	// does not guarantee that the app will close immediately
	void close() { requested_close = true; }

  protected:
	float m_fps{0.0f};

	float m_frame_time{0.0f}; // In ms

	uint32_t m_frame_count{0};

	uint32_t m_last_frame_count{0};

	///
	/// Members owned by Application
	///

	std::unique_ptr<Window> m_window;

	io::Logger m_logger{"system"};

  private:
	std::string m_name;

	bool requested_close{false};
};

} // namespace fornani::sys
