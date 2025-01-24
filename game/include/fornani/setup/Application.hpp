#pragma once
#include "Game.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Version.hpp"
#include "fornani/setup/WindowManager.hpp"

namespace fornani {

class Application {
  public:
	explicit Application(char** argv, const char * loading_screen = "/image/gui/loading.png")  : finder(argv), metadata(game_info, finder), t_loading(finder.resource_path() + loading_screen), loading(t_loading) {};
	virtual ~Application() = default;
	virtual void init(char** argv);
	virtual void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {});

  protected:
	data::ResourceFinder finder;
	dj::Json game_info{};
	dj::Json app_settings{};
	Version metadata;
	WindowManager window{};
	sf::Texture t_loading{};
	sf::Sprite loading;
};

} // namespace fornani
