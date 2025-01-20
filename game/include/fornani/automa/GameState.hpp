
#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdio>
#include <memory>
#include <filesystem>
#include <optional>
#include "Option.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Background.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/gui/HUD.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/gui/PauseWindow.hpp"

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

	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	GameState& operator=(GameState&&) = delete;
	virtual ~GameState() {}

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc){};
	virtual void frame_update([[maybe_unused]] ServiceProvider& svc){};
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win){};

	bool debug_mode{false};
	util::BitFlags<GameStateFlags> flags{};

	std::string_view scene{};
	gui::Console console;
	gui::InventoryWindow inventory_window;
	gui::PauseWindow pause_window;

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};
	sf::Vector2<float> dot_pad{24.f, 8.f};

	player::Player* player;
	gui::HUD hud;

	Scene target_folder{};
	std::vector<Option> options{};
	util::Circuit current_selection{1};
	float spacing{24.f};
	float top_buffer{80.f};
};

} // namespace automa
