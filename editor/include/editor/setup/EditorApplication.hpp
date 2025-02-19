#pragma once

#include "editor/setup/WindowManager.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"

namespace pi {

class EditorApplication {
  public:
	explicit EditorApplication(char** argv);
	void launch(char** argv);

  private:
	fornani::data::ResourceFinder finder;
	dj::Json game_info{};
	dj::Json app_settings{};
	fornani::Version metadata;
	WindowManager window{};

	fornani::io::Logger m_logger{"pioneer"};

};

} // namespace pi
