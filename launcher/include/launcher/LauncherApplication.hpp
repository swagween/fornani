#pragma once

#include "fornani/setup/Application.hpp"

namespace game {
class LauncherApplication : public fornani::Application {
public:
	LauncherApplication(char** argv, const char* loading_screen = "/image/gui/loading.png") : Application(argv, loading_screen) {};
	void init(char** argv) override;
	void launch(char** argv, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {}) override;

private:
	fornani::Logger m_logger{"Application"};
}; // class LauncherApplication
}
