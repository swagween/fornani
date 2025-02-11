#include "fornani/sys/Window.hpp"

namespace fornani::sys {

namespace {
bool calculateAspectRatio(sf::RenderTarget const& target, float& aspectRatio) {
	// Get the size of the render target (width and height)
	sf::Vector2u const size = target.getSize();

	// Check for division by zero: if height is zero, calculation fails.
	if (size.y == 0) {
		aspectRatio = 0.0f; // Set to 0 as a fallback value.
		return false;
	}

	// Calculate the aspect ratio (width / height)
	aspectRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
	return true;
}

/**
 * Calculates the normalized viewport rectangle for letterboxing a view to a specific aspect ratio.
 *
 * @param windowWidth      The width of the window (or render target) in pixels.
 * @param windowHeight     The height of the window (or render target) in pixels.
 * @param targetAspectRatio The desired aspect ratio (width / height), e.g., 16.0f/9.0f.
 * @return An sf::FloatRect representing the normalized viewport (left, top, width, height)
 *         to be set on an sf::View.
 */
sf::FloatRect calculateLetterboxViewport(unsigned int windowWidth, unsigned int windowHeight, float targetAspectRatio) {
	// Fallback for zero dimensions.
	if (windowWidth == 0 || windowHeight == 0) return sf::FloatRect({0.f, 0.f}, {1.f, 1.f});

	// Calculate the current aspect ratio of the window.
	float windowAspect = static_cast<float>(windowWidth) / windowHeight;

	// Start with a full-viewport.
	float viewportX = 0.f;
	float viewportY = 0.f;
	float viewportWidth = 1.f;
	float viewportHeight = 1.f;

	// If the window is wider than the target, add letterboxing on the left and right.
	if (windowAspect > targetAspectRatio) {
		viewportWidth = targetAspectRatio / windowAspect;
		viewportX = (1.f - viewportWidth) / 2.f;
	}
	// If the window is taller than the target, add letterboxing on the top and bottom.
	else if (windowAspect < targetAspectRatio) {
		viewportHeight = windowAspect / targetAspectRatio;
		viewportY = (1.f - viewportHeight) / 2.f;
	}
	// If they are equal, no letterboxing is needed (full viewport is used).

	return sf::FloatRect({viewportX, viewportY}, {viewportWidth, viewportHeight});
}

} // namespace
Window::Window(Properties const& properties) : m_properties(properties) {}
Window::~Window() {}
bool Window::create() {
	auto mode = sf::Style::Default;
	auto state = sf::State::Windowed;
	switch (m_properties.mode) {
	case Mode::eFullscreen: {
		mode = sf::Style::None;
		state = sf::State::Fullscreen;
		m_fullscreen = true;
		break;
	}
	case Mode::eBorderlessWindowed: {
		mode = sf::Style::None;
		state = sf::State::Windowed;
		m_fullscreen = false;
		break;
	}
	case Mode::eWindowed: {
		mode = sf::Style::Default;
		state = sf::State::Windowed;
		m_fullscreen = false;
		break;
	}
	case Mode::eDefault: {
		mode = sf::Style::Default;
		state = sf::State::Windowed;
		m_fullscreen = false;
		break;
	}
	}

	auto const vid_mode = sf::VideoMode(m_properties.extent, m_properties.bit_depth);

	m_window->create(vid_mode, m_properties.title, mode, state);
	// Enable vsync
	m_window->setVerticalSyncEnabled(m_properties.vsync == Vsync::eON);

	// Everything went good so now we can state the window should not close now.
	m_window_should_close = false;

	return true;
}
bool Window::should_close() { return m_window_should_close; }
void Window::process_events() {
	// Poll for every possible event then invoke all subscribed functions. Not thread safe.
	while (auto const event = m_window->pollEvent()) {
		if (event->is<sf::Event::Closed>()) { Closed(); }
		ProcessEventsLoopTop(); // Handled second technically but might as well be considered first.
		if (auto const* ev = event->getIf<sf::Event::Resized>()) { Resized(ev->size); }
		if (event->is<sf::Event::FocusLost>()) { FocusLost(); }
		if (event->is<sf::Event::FocusGained>()) { FocusGained(); }
		if (auto const* ev = event->getIf<sf::Event::KeyPressed>()) { KeyPressed(ev->code, ev->scancode, ev->alt, ev->control, ev->shift, ev->system); }
		if (auto const* ev = event->getIf<sf::Event::KeyReleased>()) { KeyReleased(ev->code, ev->scancode, ev->alt, ev->control, ev->shift, ev->system); }
		if (auto const* ev = event->getIf<sf::Event::MouseWheelScrolled>()) { MouseWheelScrolled(ev->wheel, ev->delta, ev->position); }
		if (auto const* ev = event->getIf<sf::Event::MouseButtonPressed>()) { MouseButtonPressed(ev->button, ev->position); }
		if (auto const* ev = event->getIf<sf::Event::MouseButtonReleased>()) { MouseButtonReleased(ev->button, ev->position); }
		if (auto const* ev = event->getIf<sf::Event::MouseMoved>()) { MouseMoved(ev->position); }
		if (auto const* ev = event->getIf<sf::Event::MouseMovedRaw>()) { MouseMovedRaw(ev->delta); }
		if (event->is<sf::Event::MouseEntered>()) { MouseEntered(); }
		if (event->is<sf::Event::MouseLeft>()) { MouseExited(); }
		if (auto const* ev = event->getIf<sf::Event::JoystickButtonPressed>()) { JoystickButtonPressed(ev->joystickId, ev->button); }
		if (auto const* ev = event->getIf<sf::Event::JoystickButtonReleased>()) { JoystickButtonReleased(ev->joystickId, ev->button); }
		if (auto const* ev = event->getIf<sf::Event::JoystickMoved>()) { JoystickMoved(ev->joystickId, ev->axis, ev->position); }
		if (auto const* ev = event->getIf<sf::Event::JoystickConnected>()) { JoystickConnected(ev->joystickId); }
		if (auto const* ev = event->getIf<sf::Event::JoystickDisconnected>()) { JoystickDisconnected(ev->joystickId); }
	}
	PolledEvents();
}
void Window::setIcon(sf::Image const& icon) const { m_window->setIcon(icon.getSize(), icon.getPixelsPtr()); }

void Window::OnCreate() {}
void Window::onResize() {}
} // namespace fornani::sys