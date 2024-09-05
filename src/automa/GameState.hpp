
#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdio>
#include <memory>
#include <filesystem>
#include <optional>
#include "Option.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../entities/player/Player.hpp"
#include "../graphics/Background.hpp"
#include "../gui/Console.hpp"
#include "../gui/Portrait.hpp"
#include "../gui/HUD.hpp"
#include "../level/Map.hpp"
#include "../gui/InventoryWindow.hpp"
#include "../gui/PauseWindow.hpp"

namespace player {
class Player;
}

namespace automa {

// globals

enum class GameStateFlags { playtest };

enum class MenuSelection { play, options, quit, controls, tutorial, credits, settings };

constexpr inline float dot_force{0.9f};
constexpr inline float dot_fric{0.86f};
constexpr inline float dot_speed{200.f};
constexpr inline float title_letter_spacing{1.0f};

struct ServiceProvider;

struct Scene {
	struct {
		std::filesystem::path scene{};
		std::filesystem::path region{};
		std::filesystem::path room{};
		std::filesystem::path next_region{};
		std::filesystem::path next_room{};
	} paths{};
	std::string_view label{};
};

class GameState {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{{MenuSelection::play, 0},	   {MenuSelection::options, 1}, {MenuSelection::quit, 2},	 {MenuSelection::controls, 0},
															 {MenuSelection::tutorial, 2}, {MenuSelection::credits, 3}, {MenuSelection::settings, 1}};

	GameState() = default;
	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	GameState& operator=(GameState&&) = delete;
	virtual ~GameState() {}

	virtual void init(ServiceProvider& svc, int room_number, std::string room_name = ""){};
	virtual void handle_events(ServiceProvider& svc, sf::Event& event){};
	virtual void tick_update(ServiceProvider& svc){};
	virtual void frame_update(ServiceProvider& svc){};
	virtual void render(ServiceProvider& svc, sf::RenderWindow& win){};

	bool debug_mode{false};
	util::BitFlags<GameStateFlags> flags{};

	std::string_view scene{};
	gui::Console console{};
	gui::InventoryWindow inventory_window;
	gui::PauseWindow pause_window;

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};
	sf::Vector2<float> dot_pad{24.f, 8.f};

	player::Player* player;
	gui::HUD hud;
	sf::Font font{};
	sf::Font subtitle_font{};

	Scene target_folder{};
	std::vector<Option> options{};
	util::Circuit current_selection{1};
	float spacing{24.f};
	float top_buffer{80.f};
};

} // namespace automa
