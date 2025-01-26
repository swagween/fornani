#pragma once

#include "fornani/setup/Version.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "editor/setup/WindowManager.hpp"

namespace pi {

class EditorApplication {
  public:
	explicit EditorApplication(char** argv);
	void launch(char** argv);

  private:
	data::ResourceFinder finder;
	dj::Json game_info{};
	dj::Json app_settings{};
	fornani::Version metadata;
	WindowManager window{};
};

} // namespace pi
