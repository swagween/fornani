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
	data::ResourceFinder finder{};
	Version metadata{};
};

} // namespace fornani
