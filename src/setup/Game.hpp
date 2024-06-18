#pragma once
#include <imgui.h>
#include <random>
#include "../automa/StateManager.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../audio/MusicPlayer.hpp"
#include "../utils/BitFlags.hpp"
#include <imgui-SFML.h>

namespace fornani {

enum class GameFlags { playtest, in_game };

class Game {
  public:
	Game() = default;
	Game(char** argv);
	void run();

  private:
	void debug_window();
	void playtester_portal();
	void take_screenshot();
	bool debug();
	void playtest_sync();

	util::BitFlags<GameFlags> flags{};

	struct automa::ServiceProvider services{};

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
		} weapons{};
	} playtest{};

	player::Player player;
	automa::StateManager game_state{};
	sf::RenderWindow window{};
	sf::Texture screencap{};
	sf::RectangleShape background{};
	audio::MusicPlayer music_player{};
};

} // namespace fornani