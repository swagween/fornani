#pragma once

#include "fornani/core/Application.hpp"

namespace launcher {

class LauncherApplication : public fornani::Application {
  public:
	LauncherApplication(char** argv, char const* loading_screen = "/image/gui/loading.png") : Application(argv, loading_screen){};
};

} // namespace game
