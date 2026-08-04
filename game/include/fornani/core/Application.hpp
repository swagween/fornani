
#pragma once

#include <capo/engine.hpp>
#include <fornani/core/Game.hpp>
#include <fornani/setup/AppContext.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/setup/WindowManager.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <memory>
#include <utility>

namespace fornani {

struct DemoFlags {
	bool is_demo{};
	bool value{};
};

class Application : public UniquePolymorphic {
  public:
	explicit Application(char** argv, char const* loading_screen = "/image/gui/loading.png") : m_finder(argv), m_context{.settings{m_finder}, .localization{m_finder}, .version{m_game_info, m_finder}}, m_window{{960, 512}} {};
	virtual void init(char** argv, DemoFlags demo_fullscreen = {});
	virtual void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2f player_position = {});
	void set_file(int to) { m_file = to; };

  protected:
	ResourceFinder m_finder;
	dj::Json m_game_info{};
	AppContext m_context;
	sf::Color m_backdrop_color{};
	WindowManager m_window;
	std::unique_ptr<capo::IEngine> m_engine{capo::create_engine()};
	int m_file{};
	DemoFlags m_demo_flags{};
	io::Logger m_logger{"Application"};
};

} // namespace fornani
