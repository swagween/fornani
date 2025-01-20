#pragma once
#include "Game.hpp"
#include "../setup/ResourceFinder.hpp"
#include "../setup/Version.hpp"
#include "../setup/WindowManager.hpp"

namespace fornani {

class Application {
  public:
	Application() = default;
	Application(char** argv);
	~Application() {}
	void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {});

  private:
	data::ResourceFinder finder;
	dj::Json game_info{};
	dj::Json app_settings{};
	Version metadata{};
	WindowManager window{};
	sf::Texture t_loading{};
	sf::Sprite loading;
};

} // namespace fornani
