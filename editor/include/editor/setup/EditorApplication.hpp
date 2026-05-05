#pragma once

#include <capo/engine.hpp>
#include <editor/automa/EditorContext.hpp>
#include <editor/automa/EditorState.hpp>
#include "fornani/io/Logger.hpp"
#include "fornani/setup/AppContext.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/WindowManager.hpp"

namespace pi {

class EditorApplication {
  public:
	explicit EditorApplication(char** argv);
	void run(char** argv);

  private:
	void shutdown();

  private:
	fornani::ResourceFinder m_finder;
	dj::Json game_info{};
	fornani::AppContext context;
	EditorContext editor_context{};
	dj::Json app_settings{};
	dj::Json user_data{};
	fornani::WindowManager window{};
	std::unique_ptr<capo::IEngine> m_engine{capo::create_engine()};

	fornani::automa::ServiceProvider m_services;

	std::unique_ptr<EditorState> m_state{};
	EditorStateType m_current_state{};

	fornani::io::Logger m_logger{"pioneer"};
};

} // namespace pi
