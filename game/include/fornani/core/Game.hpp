
#pragma once

#include <capo/engine.hpp>
#include <fornani/automa/StateManager.hpp>
#include <fornani/core/FrameTracker.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/graphics/LoadingScreen.hpp>
#include <fornani/io/Loader.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/AppContext.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <filesystem>
#include <string_view>
#include <imgui-SFML.h>

namespace fornani {

class WindowManager;
enum class GameFlags : std::uint8_t { playtest, in_game, draw_cursor, waited };

class Game final {
  public:
	explicit Game(char** argv, WindowManager& window, AppContext& context, capo::IEngine& audio_engine);
	void run(capo::IEngine& audio_engine, bool demo = false, int room_id = 100, std::filesystem::path levelpath = std::filesystem::path{}, sf::Vector2f player_position = {});
	void set_file(int to) { services.editor_settings.save_file = to; }
	void shutdown();
	void draw_wallpaper();
	void set_language(AppContext& context, std::string_view label);

	util::BitFlags<GameFlags> flags{};

	[[nodiscard]] auto get_services() -> automa::ServiceProvider& { return services; }
	[[nodiscard]] auto get_context() -> AppContext& { return *m_context; }

  private:
	void playtester_portal(sf::RenderWindow& window);
	void take_screenshot(sf::Texture& screencap, bool pixel_perfect = true);
	void restart_trial(std::filesystem::path const& levelpath);

  private:
	std::optional<LoadingScreen> m_loading_screen{};
	AppContext* m_context;

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

	std::shared_ptr<Slot const> m_slot{std::make_shared<Slot const>()};
	std::optional<player::Player> player{};
	std::optional<automa::StateManager> game_state{};
	std::optional<std::unique_ptr<automa::StateManager>> m_game_menu{};
	std::unique_ptr<graphics::Background> m_background{};
	std::optional<Animatable> m_cursor{};
	util::Cooldown m_screencap_timer;

	FrameTracker m_frame_tracker{};
	sf::RectangleShape m_wallpaper{};

	io::Logger m_logger{"core"};

	bool m_zooming{};
};

} // namespace fornani
