#include "fornani/sys/Window.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Window/VideoMode.hpp>

namespace fornani::sys {

Window::Window(Properties const& properties) : m_properties(properties) {}
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

	sf::Window

		auto const vid_mode = sf::VideoMode(m_properties.extent, m_properties.bit_depth);

	setRenderTarget<sf::RenderWindow>(vid_mode, m_properties.title, mode, state);

	auto const tempWindow = dynamic_cast<sf::RenderWindow*>(m_renderTarget.get());
	m_window = std::unique_ptr<sf::RenderWindow>(tempWindow);

	calculateAspectRatio(m_renderTarget->getSize());

	// Enable vsync
	m_window->setVerticalSyncEnabled(m_properties.vsync == Vsync::eON);

	// m_window->create(vid_mode, m_properties.title, mode, state);
	//  Enable vsync
	// m_window->setVerticalSyncEnabled(m_properties.vsync == Vsync::eON);

	// Everything went good so now we can state the window should not close now.
	m_window_should_close = false;

	return true;
}
bool Window::should_close() { return m_window_should_close; }
void Window::process_events() {
	// Poll for every possible event then invoke all subscribed functions. Not thread safe.
	while (auto const event = m_window->pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			// TODO: Double check this makes sense.
			PreClosed(); // Fired before the window is closed.
			Closed();
			PostClosed(); // Fired after the window is closed.
		}
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

void Window::updateRenderView() {
	m_renderView.setSize({static_cast<float>(m_properties.extent.x), static_cast<float>(m_properties.extent.y)});
	m_renderView.setCenter(m_renderView.getSize() * 0.5f);
}
void Window::calculateAspectRatio(sf::Vector2u const windowSize) {
	if (!m_properties.maintain_aspect_ratio) {
		m_renderView = sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)}));

		return;
	}

	// Compute aspect ratio
	float const targetWidth = static_cast<float>(m_properties.extent.x);
	float const targetHeight = static_cast<float>(m_properties.extent.y);

	float scaleX = static_cast<float>(windowSize.x) / targetWidth;
	float scaleY = static_cast<float>(windowSize.y) / targetHeight;

	// Enforce integer scaling if enabled (prevents half-pixel issues)
	if (m_properties.integer_scaling) {
		float const scaleFactor = std::floor(std::min(scaleX, scaleY)); // Round down to nearest integer scale
		scaleX = scaleFactor;
		scaleY = scaleFactor;
	}

	float const finalWidth = targetWidth * scaleX;
	float const finalHeight = targetHeight * scaleY;

	// Calculate letterboxing (black bars)
	float const offsetX = (windowSize.x - finalWidth) / 2.0f;
	float const offsetY = (windowSize.y - finalHeight) / 2.0f;

	// Set viewport (normalized coordinates)
	sf::FloatRect const viewportRect({offsetX / static_cast<float>(windowSize.x), offsetY / static_cast<float>(windowSize.y)}, {finalWidth / static_cast<float>(windowSize.x), finalHeight / static_cast<float>(windowSize.y)});

	m_renderView.setViewport(viewportRect);
}
void Window::OnCreate() {}
void Window::onResize() {}
} // namespace fornani::sys