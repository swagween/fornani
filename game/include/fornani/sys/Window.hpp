#pragma once

#include "fornani/core/Event.hpp"
#include "fornani/ftl/MonoInstance.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/ConceptHelpers.hpp"

#include <SFML/Graphics.hpp>

namespace fornani::sys {

/**
 * @brief A window abstraction wrapping SFML's RenderWindow.
 *
 * This class integrates window management with Fornani's event and logging systems.
 * It supports various display modes and Vsync options, and provides a suite of events
 * for subscribers to observe window-related changes.
 */
class Window : public ftl::MonoInstance<Window> {
  public:
	static constexpr sf::Vector2u k_DefaultExtent = {1280, 720};
	static constexpr unsigned int k_DefaultBitDepth = 32;

	/**
	 * @brief Supported display modes for the window.
	 *
	 * These modes determine how the window is rendered on the screen.
	 */
	enum class Mode : uint8_t {
		eFullscreen,		 ///< The window occupies the entire screen.
		eBorderlessWindowed, ///< Fullscreen mode without window borders.
		eWindowed,			 ///< Fullscreen mode with stretching applied.
		eDefault,			 ///< The default display mode.
	};

	/**
	 * @brief Options for vertical synchronization.
	 *
	 * Vsync prevents screen tearing, though it may introduce input lag.
	 */
	enum class Vsync : uint8_t {
		eOFF = 0,	   ///< Vsync disabled.
		eON = 1,	   ///< Vsync enabled.
		eDefault = eON ///< The default Vsync setting.
	};

	/**
	 * @brief Optional properties for window creation.
	 *
	 * Each property is optional and can override the default settings.
	 *
	 * TODO: Might remove this not sure yet.
	 */
	struct OptionalProperties {
		std::optional<std::string> title;	///< Optional window title.
		std::optional<Mode> mode;			///< Optional display mode.
		std::optional<bool> resizable;		///< Optional flag to allow resizing.
		std::optional<Vsync> vsync;			///< Optional Vsync setting.
		std::optional<sf::Vector2u> extent; ///< Optional window dimensions.
	};

	/**
	 * @brief Default properties for window creation.
	 *
	 * These properties define the baseline configuration for a new window.
	 */
	struct Properties {
		std::string title = "Fornani";				///< Default window title.
		Mode mode = Mode::eDefault;					///< Default display mode. (Currently sf::Style::Default & sf::State::Windowed [may change])
		bool resizable = true;						///< Whether the window is resizable.
		Vsync vsync = Vsync::eDefault;				///< Default Vsync setting. (ON)
		sf::Vector2u extent = {k_DefaultExtent};	///< Default window size. (1280, 720)
		unsigned int bit_depth = k_DefaultBitDepth; ///< Default bit depth. (32)
		bool maintain_aspect_ratio{true};			///< Should the render frame maintain a consistent aspect ratio or not?
		bool integer_scaling{true};					///< Should the render frame be scaled using integer values?
		int frame_rate_limit{60};					///< Set fps limit. -1 means uncapped framerate.
	};

  protected:
	void updateRenderView();
	void calculateAspectRatio(sf::Vector2u windowSize);

	virtual void OnCreate();

	virtual void onResize();

	Properties m_properties; ///< Configuration properties for the window.

	// sf::RenderWindow m_handle;	 ///< The underlying SFML RenderWindow.
	std::unique_ptr<sf::RenderTarget> m_renderTarget;
	sf::View m_renderView;
	sf::RenderStates m_renderStates;

	// Window handle
	std::unique_ptr<sf::RenderWindow> m_window;

	bool m_fullscreen{false};		  ///< Whether the window is in fullscreen mode.
	bool m_window_should_close{true}; ///< Whether the window should close.

	io::Logger m_logger{"core"}; ///< Logger for window-related messages.

  public:
	explicit Window(Properties const& properties);

	// Disable copy and move semantics.
	Window(Window const&) = delete;
	Window& operator=(Window const&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	virtual bool create();

	virtual bool should_close();

	virtual void process_events();

	virtual void close();

	sf::Vector2u resize(sf::Vector2u const& extent);

	// virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	template <util::Derives<sf::RenderTarget> T, typename... ArgsT>
		requires util::ConstructibleUserType<T, ArgsT...>
	void setRenderTarget(ArgsT&&... args) {
		m_renderTarget = std::make_unique<T>(std::forward<ArgsT>(args)...);
	}

	sf::Vector2u const& get_extent() const;

	Mode get_window_mode() const;

	void setIcon(sf::Image const& icon) const;

	Properties const& get_properties() const { return m_properties; }

	//////////////////////////////////////////////////////////////////////////////
	/// Subscribable events
	//////////////////////////////////////////////////////////////////////////////

	core::Event<void()> PreClosed;
	core::Event<void()> Closed;
	core::Event<void()> PostClosed;
	core::Event<void()> ProcessEventsLoopTop;
	core::Event<void(sf::Vector2u)> Resized;
	core::Event<void()> FocusLost;
	core::Event<void()> FocusGained;
	using KeyEvent = core::Event<void(sf::Keyboard::Key, sf::Keyboard::Scancode, bool, bool, bool, bool)>;
	KeyEvent KeyPressed;
	KeyEvent KeyReleased;
	core::Event<void(sf::Mouse::Wheel, float, sf::Vector2i)> MouseWheelScrolled;
	core::Event<void(sf::Mouse::Button, sf::Vector2i)> MouseButtonPressed;
	core::Event<void(sf::Mouse::Button, sf::Vector2i)> MouseButtonReleased;
	core::Event<void(sf::Vector2i)> MouseMoved;
	core::Event<void(sf::Vector2i)> MouseMovedRaw;
	core::Event<void()> MouseEntered;
	core::Event<void()> MouseExited;
	core::Event<void(unsigned int, unsigned int)> JoystickButtonPressed;
	core::Event<void(unsigned int, unsigned int)> JoystickButtonReleased;
	core::Event<void(unsigned int, sf::Joystick::Axis, float)> JoystickMoved;
	core::Event<void(unsigned int)> JoystickConnected;
	core::Event<void(unsigned int)> JoystickDisconnected;
	core::Event<void()> PolledEvents;
};

} // namespace fornani::sys
