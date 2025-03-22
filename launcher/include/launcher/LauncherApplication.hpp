#pragma once

#include "fornani/core/Application.hpp"

namespace launcher {

class LauncherApplication : public fornani::Application {
  public:
	LauncherApplication(char** argv) : Application(argv){};
};

} // namespace launcher
