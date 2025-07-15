#pragma once

#include <capo/engine.hpp>
#include "fornani/io/Logger.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"

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
	fornani::WindowManager window{};
	std::unique_ptr<capo::IEngine> m_engine{capo::create_engine()};

	fornani::io::Logger m_logger{"pioneer"};
};

} // namespace pi
