#pragma once

#include "fornani/core/Game.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"

#include <utility>

namespace fornani {

class Application {
  public:
	explicit Application(char** argv) : m_finder(argv), m_metadata(m_game_info, m_finder){};
	virtual ~Application() = default;
	virtual void init(char** argv, std::pair<bool, bool> demo_fullscreen = {});

	virtual void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {});

  protected:
	data::ResourceFinder m_finder;
	dj::Json m_game_info{};
	dj::Json m_app_settings{};
	Version m_metadata;
	WindowManager m_window{};
	io::Logger m_logger{"Application"};
};

} // namespace fornani
