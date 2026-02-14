
#pragma once

#include <capo/engine.hpp>
#include <fornani/automa/StateManager.hpp>
#include <fornani/core/FrameTracker.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <filesystem>
#include <imgui-SFML.h>

namespace fornani {

class WindowManager;
enum class GameFlags { playtest, in_game, draw_cursor };

class Game {
  public:
	Game(char** argv, WindowManager& window, Version& version, capo::IEngine& audio_engine);
	~Game() = default;
	void run(capo::IEngine& audio_engine, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2f player_position = {});
	void set_file(int to) { services.editor_settings.save_file = to; }
	void shutdown();

	util::BitFlags<GameFlags> flags{};

  private:
	void playtester_portal(sf::RenderWindow& window);
	void take_screenshot(sf::Texture& screencap);
	void restart_trial(std::filesystem::path const& levelpath);

	automa::ServiceProvider services;

	struct {
		sf::Vector2u win_size{};
		float height_ratio{};
		float width_ratio{};
	} measurements{};

	struct {
		int sample{};
		int total{};
	} rng_test{};

	player::Player player;
	automa::StateManager game_state;
	std::optional<std::unique_ptr<automa::StateManager>> m_game_menu;
	std::unique_ptr<graphics::Background> m_background{};

	FrameTracker m_frame_tracker{};

	io::Logger m_logger{"core"};
};

} // namespace fornani
