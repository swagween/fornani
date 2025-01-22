#pragma once

#include "fornani/setup/Version.hpp"
#include "editor/setup/ResourceFinder.hpp"
#include "editor/setup/WindowManager.hpp"

namespace pi {

class EditorApplication {
  public:
	explicit EditorApplication(char** argv);
	void init();
	void launch(char** argv);

private:
	dj::Json game_info{};
	dj::Json app_settings{};
	ResourceFinder finder{};
	fornani::Version metadata{};
	WindowManager window{};

};

} // namespace pi
