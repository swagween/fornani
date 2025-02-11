
#include "fornani/sys/Application.hpp"

#include <SFML/Graphics.hpp>
#include <libassert/assert.hpp>

#include <iostream>

#include "fornani/utils/Tracy.hpp"

namespace fornani::sys {

Application::Application() : m_name{"DefaultFornaniApp"} {}
bool Application::prepare(ApplicationOptions const& options) {
	NANI_LOG_INFO(m_logger, "Preparing application...");
	Window::Properties const windowProperties = {
		.title = options.win_props.title,
		.vsync = options.win_props.vsync,
		.extent = options.win_props.extent,
		.mode = options.win_props.mode,
		.resizable = options.win_props.resizable,
		.bit_depth = options.win_props.bit_depth,
	};
	m_window = std::make_unique<Window>(windowProperties);

	NANI_LOG_INFO(m_logger, "Application prepared!");
}
} // namespace fornani::sys
