#pragma once

#include <capo/engine.hpp>
#include <memory>
#include <utility>
#include "fornani/core/Game.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"
#include "fornani/utils/Polymorphic.hpp"

namespace fornani {

class Application : public UniquePolymorphic {
  public:
	explicit Application(char** argv, char const* loading_screen = "/image/gui/loading.png") : m_finder(argv), m_metadata(m_game_info, m_finder) {};
	virtual void init(char** argv, std::pair<bool, bool> demo_fullscreen = {});
	virtual void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2f player_position = {});

  protected:
	ResourceFinder m_finder;
	dj::Json m_game_info{};
	dj::Json m_app_settings{};
	Version m_metadata;
	WindowManager m_window{};
	std::unique_ptr<capo::IEngine> m_engine{capo::create_engine()};
	io::Logger m_logger{"Application"};
};

} // namespace fornani
