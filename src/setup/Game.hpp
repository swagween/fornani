#pragma once
#include <imgui.h>
#include <filesystem>
#include <random>
#include "../audio/MusicPlayer.hpp"
#include "../automa/StateManager.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/BitFlags.hpp"
#include <imgui-SFML.h>

namespace fornani {

class WindowManager;
enum class GameFlags { playtest, in_game };
enum class KeyboardFlags { control };

class Game {
  public:
	Game() = default;
	Game(char** argv, WindowManager& window);
	~Game() {}
	void run(bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {});
	void shutdown();

	void playtest_sync();
	void toggle_weapon(bool flag, int id);
	util::BitFlags<GameFlags> flags{};
	util::BitFlags<KeyboardFlags> key_flags{};

  private:
	void playtester_portal(sf::RenderWindow& window);
	void take_screenshot(sf::Texture& screencap);
	automa::ServiceProvider services;

	struct {
		sf::Vector2u win_size{};
		float height_ratio{};
		float width_ratio{};
	} measurements{};

	struct {
		bool m_musicplayer{};
		bool b_dash{};
		bool b_shield{};
		bool b_wallslide{};
		bool b_doublejump{};
		struct {
			bool bryn{};
			bool grenade{};
			bool plasmer{};
			bool tomahawk{};
			bool grapple{};
			bool staple_gun{};
			bool indie{};
			bool gnat{};
		} weapons{};
	} playtest{};

	struct {
		int sample{};
		int total{};
	} rng_test{};

	player::Player player;
	automa::StateManager game_state{};
	sf::RectangleShape background{};
};

} // namespace fornani