#pragma once
#include "Game.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"

namespace fornani {

class Application {
  public:
	explicit Application(char** argv, const char * loading_screen = "/image/gui/loading.png")  : m_finder(argv), m_metadata(m_game_info, m_finder), m_t_loading(m_finder.resource_path() + loading_screen), m_loading(m_t_loading) {};
	virtual ~Application() = default;
	virtual void init(char** argv){};

	virtual void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {}){};


  protected:

	data::ResourceFinder m_finder;
	dj::Json m_game_info{};
	dj::Json m_app_settings{};
	Version m_metadata;
	WindowManager m_window{};
	sf::Texture m_t_loading{};
	sf::Sprite m_loading;
};

} // namespace fornani
