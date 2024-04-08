
#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdio>
#include <memory>
#include "../components/PhysicsComponent.hpp"
#include "../entities/player/Player.hpp"
#include "../graphics/Background.hpp"
#include "../gui/Console.hpp"
#include "../gui/Portrait.hpp"
#include "../gui/HUD.hpp"
#include "../level/Map.hpp"
#include "../gui/InventoryWindow.hpp"

namespace player {
class Player;
}

namespace automa {

// globals

enum class STATE { STATE_NULL, STATE_INIT, STATE_EXIT, STATE_MENU, STATE_OPTIONS, STATE_FILE, STATE_MAIN, STATE_DOJO };

enum class MenuSelection { play, options, quit, controls, tutorial, credits };

constexpr inline float dot_force{0.9f};
constexpr inline float dot_fric{0.86f};
constexpr inline float dot_speed{200.f};

struct ServiceProvider;

struct Option {
	Option(ServiceProvider& svc, std::string_view lbl);
	sf::Text label{};
	sf::Vector2<float> position{};
	sf::Vector2<float> left_offset{};
	sf::Vector2<float> right_offset{};
	sf::Vector2<float> dot_offset{24.f, 2.f};
	int index{};
	bool highlighted{};
	bool selected{};
	void update(ServiceProvider& svc, int& selection);
};

class GameState {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{{MenuSelection::play, 0}, {MenuSelection::options, 1}, {MenuSelection::quit, 2}, {MenuSelection::controls, 0}, {MenuSelection::tutorial, 1}, {MenuSelection::credits, 2}};

	GameState() = default;
	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene, int id = 0);
	GameState& operator=(GameState&&) = delete;

	virtual void init(ServiceProvider& svc, std::string_view room = ""){};
	virtual void setTilesetTexture(ServiceProvider& svc, sf::Texture& t){};
	virtual void handle_events(ServiceProvider& svc, sf::Event& event){};
	virtual void tick_update(ServiceProvider& svc){};
	virtual void frame_update(ServiceProvider& svc){};
	virtual void render(ServiceProvider& svc, sf::RenderWindow& win){};
	void constrain_selection();

	STATE state = STATE::STATE_NULL;
	bool debug_mode{false};

	gui::Console console{};
	gui::InventoryWindow inventory_window;

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};
	sf::Vector2<float> dot_pad{24.f, 8.f};

	player::Player* player;
	gui::HUD hud;
	sf::Font font{};

	std::string_view scene{};
	std::vector<Option> options{};
	int current_selection{};
	float spacing{32.f};
	float top_buffer{80.f};
};

} // namespace automa
