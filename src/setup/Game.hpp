#pragma once
#include <imgui.h>
#include <random>
#include "../automa/StateManager.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../audio/MusicPlayer.hpp"
#include "../utils/BitFlags.hpp"
#include <imgui-SFML.h>
#include <filesystem>

namespace fornani {

enum class GameFlags { playtest, in_game };

class Game {
  public:
	Game() = default;
	Game(char** argv);
	~Game() {}
	void run(sf::RenderWindow& window, sf::Texture& screencap, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2<float> player_position = {});

	void playtest_sync();
	void toggle_weapon(bool flag, int id);
	util::BitFlags<GameFlags> flags{};

  private:

	void debug_window(sf::RenderWindow& window);
	void playtester_portal(sf::RenderWindow& window);
	void take_screenshot(sf::Texture& screencap);
	bool debug();
	automa::ServiceProvider services{};

	struct {
		sf::Vector2<uint32_t> win_size{};
		float height_ratio{};
		float width_ratio{};
	} measurements{};

	struct {
		int draw_calls{};
	} trackers{};

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

	player::Player player;
	automa::StateManager game_state{};
	sf::RectangleShape background{};
};

} // namespace fornani