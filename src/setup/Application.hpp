#pragma once
#include "Game.hpp"
#include "../setup/ResourceFinder.hpp"
#include "../setup/Version.hpp"

namespace fornani {

class Application {
  public:
	Application() = default;
	Application(char** argv);
	~Application() {}
	void launch(char** argv);

  private:
	sf::RenderWindow window{};
	sf::Texture screencap{};
	sf::Texture t_loading{};
	sf::Sprite loading{};
	dj::Json game_info{};
	dj::Json app_settings{};
	data::ResourceFinder finder{};
	Version metadata{};
	struct {
		sf::Vector2<int> screen_dimensions{};
		sf::View game_view{};
		sf::VideoMode mode{};
		sf::FloatRect game_port{};
	} window_utilities{};
	struct {
		bool fullscreen{};
	} settings{};
};

} // namespace fornani
