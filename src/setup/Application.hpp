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
	sf::Texture t_loading{};
	sf::Sprite loading{};
	dj::Json game_info{};
	dj::Json app_settings{};
	data::ResourceFinder finder{};
	Version metadata{};
	WindowManager window{};
};

} // namespace fornani
