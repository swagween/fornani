
#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <cstdio>
#include <memory>
#include "../components/PhysicsComponent.hpp"
#include "../entities/player/Player.hpp"
#include "../graphics/Background.hpp"
#include "../gui/HUD.hpp"
#include "../level/Map.hpp"
#include "../setup/LookupTables.hpp"

namespace automa {

// globals

enum class STATE { STATE_NULL, STATE_INIT, STATE_EXIT, STATE_MENU, STATE_OPTIONS, STATE_FILE, STATE_MAIN, STATE_DOJO };

enum class MenuSelection { new_game, load_game, options };

inline MenuSelection selection{};
inline int file_selection{0};

constexpr inline float dot_force{0.09f};
constexpr inline float dot_fric{0.87f};
constexpr inline float dot_speed{20.f};

class GameState {

  public:
	using Time = std::chrono::duration<float>;

	GameState() = default;
	GameState(int id) {}
	GameState& operator=(GameState&&) = delete;

	template <typename T>
	class StateID;

	virtual void init(std::string const& load_path){};
	virtual void setTilesetTexture(sf::Texture& t){};
	virtual void handle_events(sf::Event& event){};
	virtual void tick_update(){};
	virtual void frame_update(){};
	virtual void render(sf::RenderWindow& win){};

	STATE state = STATE::STATE_NULL;
	bool debug_mode{false};

	gui::HUD hud{{20, 20}};

	vfx::Attractor left_dot{};
	vfx::Attractor right_dot{};
	sf::Vector2<float> dot_pad{24.f, 8.f};
};

} // namespace automa
