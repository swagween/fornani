#pragma once

#include "fornani/core/Event.hpp"
#include "fornani/ftl/MonoInstance.hpp"
#include "fornani/io/Logger.hpp"

#include <SFML/Graphics.hpp>
#include <functional>

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
		bool maintain_aspect_ratio{true};
	};

	/**
	 * @brief Constructs a new Window.
	 *
	 * Initializes the window with the specified properties.
	 *
	 * @param properties The configuration for the window.
	 */
	explicit Window(Properties const& properties);

	/**
	 * @brief Destructs the Window.
	 *
	 * Cleans up window resources.
	 */
	~Window() override;

	// Disable copy and move semantics.
	Window(Window const&) = delete;
	Window& operator=(Window const&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	/**
	 * @brief Initializes all initial code needed for startup of windowing.
	 *
	 * @return true if a close request has been issued; false otherwise.
	 */
	virtual bool create();

	/**
	 * @brief Checks whether the window should be closed.
	 *
	 * @return true if a close request has been issued; false otherwise.
	 */
	virtual bool should_close();

	/**
	 * @brief Processes all pending window events.
	 *
	 * Polls events from the underlying system and dispatches them to subscribers.
	 */
	virtual void process_events();

	/**
	 * @brief Signals that the window should close.
	 *
	 * Requests termination of the window's event loop.
	 */
	virtual void close();

	/**
	 * @brief Attempts to resize the window.
	 *
	 * This operation may not always change the window size.
	 *
	 * @param extent The desired new dimensions for the window.
	 * @return sf::Vector2u The actual dimensions of the window after resizing.
	 */
	sf::Vector2u resize(sf::Vector2u const& extent);

	/**
	 * @brief Gets the current dimensions of the window.
	 *
	 * @return const sf::Vector2u& A reference to the window's size.
	 */
	sf::Vector2u const& get_extent() const;

	/**
	 * @brief Retrieves the current display mode of the window.
	 *
	 * @return Mode The current window mode.
	 */
	Mode get_window_mode() const;

	void setIcon(sf::Image const& icon) const;

	/**
	 * @brief Accesses the window configuration properties.
	 *
	 * @return const Properties& A reference to the window's properties.
	 */
	Properties const& get_properties() const { return m_properties; }

	//////////////////////////////////////////////////////////////////////////////
	/// Subscribable events
	//////////////////////////////////////////////////////////////////////////////

	/**
	 * @brief Event fired when the window is closed.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->Closed += []() {
	 *     std::cout << "Window closed (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->Closed += std::bind_front(&MyClass::onClosed, this);
	 * @endcode
	 */
	core::Event<void()> Closed;

	/**
	 * @brief Event fired at the beginning of each event processing loop.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->ProcessEventsLoopTop += []() {
	 *     std::cout << "Starting event loop iteration (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->ProcessEventsLoopTop += std::bind_front(&MyClass::onLoopTop, this);
	 * @endcode
	 */
	core::Event<void()> ProcessEventsLoopTop;

	/**
	 * @brief Event fired when the window is resized.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Vector2u).
	 *
	 * @param newExtent The new dimensions of the window.
	 *
	 * @code
	 * // Lambda approach:
	 * window->Resized += [](sf::Vector2u newExtent) {
	 *     std::cout << "Resized (lambda): " << newExtent.x << "x" << newExtent.y << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->Resized += std::bind_front(&MyClass::onResized, this);
	 * @endcode
	 */
	core::Event<void(sf::Vector2u)> Resized;

	/**
	 * @brief Event fired when the window loses focus.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->FocusLost += []() {
	 *     std::cout << "Focus lost (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->FocusLost += std::bind_front(&MyClass::onFocusLost, this);
	 * @endcode
	 */
	core::Event<void()> FocusLost;

	/**
	 * @brief Event fired when the window gains focus.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->FocusGained += []() {
	 *     std::cout << "Focus gained (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->FocusGained += std::bind_front(&MyClass::onFocusGained, this);
	 * @endcode
	 */
	core::Event<void()> FocusGained;

	/**
	 * @brief Type alias for keyboard events.
	 *
	 * Subscribers must provide a function matching the signature:
	 * void(sf::Keyboard::Key, sf::Keyboard::Scancode, bool, bool, bool, bool).
	 *
	 * Since this event has more than two parameters, only the std::bind_front approach is shown.
	 *
	 * @code
	 * // std::bind_front approach:
	 * InputHandler handler;
	 * window->KeyPressed += std::bind_front(&InputHandler::processKey, &handler);
	 * @endcode
	 */
	using KeyEvent = core::Event<void(sf::Keyboard::Key, sf::Keyboard::Scancode, bool, bool, bool, bool)>;

	/**
	 * @brief Event fired when a key is pressed.
	 *
	 * Subscribers must provide a function matching the signature defined in KeyEvent.
	 * (Only std::bind_front example is provided due to multiple parameters.)
	 *
	 * @code
	 * InputHandler handler;
	 * window->KeyPressed += std::bind_front(&InputHandler::processKey, &handler);
	 * @endcode
	 */
	KeyEvent KeyPressed;

	/**
	 * @brief Event fired when a key is released.
	 *
	 * Subscribers must provide a function matching the signature defined in KeyEvent.
	 * (Only std::bind_front example is provided due to multiple parameters.)
	 *
	 * @code
	 * InputHandler handler;
	 * window->KeyReleased += std::bind_front(&InputHandler::processKeyRelease, &handler);
	 * @endcode
	 */
	KeyEvent KeyReleased;

	/**
	 * @brief Event fired when the mouse wheel is scrolled.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Mouse::Wheel, float, sf::Vector2i).
	 * (Only std::bind_front example is provided due to more than two parameters.)
	 *
	 * @code
	 * InputHandler handler;
	 * window->MouseWheelScrolled += std::bind_front(&InputHandler::onMouseWheelScrolled, &handler);
	 * @endcode
	 */
	core::Event<void(sf::Mouse::Wheel, float, sf::Vector2i)> MouseWheelScrolled;

	/**
	 * @brief Event fired when a mouse button is pressed.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Mouse::Button, sf::Vector2i).
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseButtonPressed += [](sf::Mouse::Button button, sf::Vector2i pos) {
	 *     std::cout << "Mouse button pressed (lambda): " << static_cast<int>(button)
	 *               << " at (" << pos.x << ", " << pos.y << ")" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseButtonPressed += std::bind_front(&MyClass::onMouseButtonPressed, this);
	 * @endcode
	 */
	core::Event<void(sf::Mouse::Button, sf::Vector2i)> MouseButtonPressed;

	/**
	 * @brief Event fired when a mouse button is released.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Mouse::Button, sf::Vector2i).
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseButtonReleased += [](sf::Mouse::Button button, sf::Vector2i pos) {
	 *     std::cout << "Mouse button released (lambda): " << static_cast<int>(button)
	 *               << " at (" << pos.x << ", " << pos.y << ")" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseButtonReleased += std::bind_front(&MyClass::onMouseButtonReleased, this);
	 * @endcode
	 */
	core::Event<void(sf::Mouse::Button, sf::Vector2i)> MouseButtonReleased;

	/**
	 * @brief Event fired when the mouse is moved.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Vector2i).
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseMoved += [](sf::Vector2i pos) {
	 *     std::cout << "Mouse moved (lambda) to (" << pos.x << ", " << pos.y << ")" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseMoved += std::bind_front(&MyClass::onMouseMoved, this);
	 * @endcode
	 */
	core::Event<void(sf::Vector2i)> MouseMoved;

	/**
	 * @brief Event fired when raw mouse movement is detected.
	 *
	 * Subscribers must provide a function matching the signature: void(sf::Vector2i).
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseMovedRaw += [](sf::Vector2i pos) {
	 *     std::cout << "Raw mouse moved (lambda) to (" << pos.x << ", " << pos.y << ")" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseMovedRaw += std::bind_front(&MyClass::onMouseMovedRaw, this);
	 * @endcode
	 */
	core::Event<void(sf::Vector2i)> MouseMovedRaw;

	/**
	 * @brief Event fired when the mouse enters the window.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseEntered += []() {
	 *     std::cout << "Mouse entered (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseEntered += std::bind_front(&MyClass::onMouseEntered, this);
	 * @endcode
	 */
	core::Event<void()> MouseEntered;

	/**
	 * @brief Event fired when the mouse exits the window.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->MouseExited += []() {
	 *     std::cout << "Mouse exited (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->MouseExited += std::bind_front(&MyClass::onMouseExited, this);
	 * @endcode
	 */
	core::Event<void()> MouseExited;

	/**
	 * @brief Event fired when a joystick button is pressed.
	 *
	 * Subscribers must provide a function matching the signature: void(unsigned int, unsigned int).
	 *
	 * @code
	 * // Lambda approach:
	 * window->JoystickButtonPressed += [](unsigned int joystickId, unsigned int button) {
	 *     std::cout << "Joystick button pressed (lambda): ID=" << joystickId
	 *               << ", Button=" << button << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->JoystickButtonPressed += std::bind_front(&MyClass::onJoystickButtonPressed, this);
	 * @endcode
	 */
	core::Event<void(unsigned int, unsigned int)> JoystickButtonPressed;

	/**
	 * @brief Event fired when a joystick button is released.
	 *
	 * Subscribers must provide a function matching the signature: void(unsigned int, unsigned int).
	 *
	 * @code
	 * // Lambda approach:
	 * window->JoystickButtonReleased += [](unsigned int joystickId, unsigned int button) {
	 *     std::cout << "Joystick button released (lambda): ID=" << joystickId
	 *               << ", Button=" << button << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->JoystickButtonReleased += std::bind_front(&MyClass::onJoystickButtonReleased, this);
	 * @endcode
	 */
	core::Event<void(unsigned int, unsigned int)> JoystickButtonReleased;

	/**
	 * @brief Event fired when a joystick axis is moved.
	 *
	 * Subscribers must provide a function matching the signature: void(unsigned int, sf::Joystick::Axis, float).
	 * (Only std::bind_front example is provided due to more than two parameters.)
	 *
	 * @code
	 * // std::bind_front approach:
	 * window->JoystickMoved += std::bind_front(&MyClass::onJoystickMoved, this);
	 * @endcode
	 */
	core::Event<void(unsigned int, sf::Joystick::Axis, float)> JoystickMoved;

	/**
	 * @brief Event fired when a joystick is connected.
	 *
	 * Subscribers must provide a function matching the signature: void(unsigned int).
	 *
	 * @code
	 * // Lambda approach:
	 * window->JoystickConnected += [](unsigned int joystickId) {
	 *     std::cout << "Joystick connected (lambda): ID=" << joystickId << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->JoystickConnected += std::bind_front(&MyClass::onJoystickConnected, this);
	 * @endcode
	 */
	core::Event<void(unsigned int)> JoystickConnected;

	/**
	 * @brief Event fired when a joystick is disconnected.
	 *
	 * Subscribers must provide a function matching the signature: void(unsigned int).
	 *
	 * @code
	 * // Lambda approach:
	 * window->JoystickDisconnected += [](unsigned int joystickId) {
	 *     std::cout << "Joystick disconnected (lambda): ID=" << joystickId << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->JoystickDisconnected += std::bind_front(&MyClass::onJoystickDisconnected, this);
	 * @endcode
	 */
	core::Event<void(unsigned int)> JoystickDisconnected;

	/**
	 * @brief Event fired after all events have been processed.
	 *
	 * Subscribers must provide a function matching the signature: void().
	 *
	 * @code
	 * // Lambda approach:
	 * window->PolledEvents += []() {
	 *     std::cout << "All events processed (lambda)" << std::endl;
	 * };
	 *
	 * // std::bind_front approach:
	 * window->PolledEvents += std::bind_front(&MyClass::onPolledEvents, this);
	 * @endcode
	 */
	core::Event<void()> PolledEvents;

  protected:
	virtual void OnCreate();

	virtual void onResize();

	Properties m_properties; ///< Configuration properties for the window.

	// sf::RenderWindow m_handle;	 ///< The underlying SFML RenderWindow.
	std::unique_ptr<sf::RenderTarget> m_render_target;
	sf::View m_renderView;
	sf::RenderStates m_renderStates;

	std::unique_ptr<sf::Window> m_window;

	bool m_fullscreen{false};		  ///< Whether the window is in fullscreen mode.
	bool m_window_should_close{true}; ///< Whether the window should close.

	io::Logger m_logger{"core"}; ///< Logger for window-related messages.
};

} // namespace fornani::sys
