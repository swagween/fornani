#pragma once

#include "fornani/automa/GameState.hpp"
#include "fornani/automa/StateController.hpp"

namespace fornani {
class Game;
} // namespace fornani

namespace fornani::player {
class Player;
} // namespace fornani::player

namespace fornani::automa {

struct ServiceProvider;

class StateManager {

  public:
	StateManager(ServiceProvider& svc, player::Player& player, MenuType type);
	StateManager& operator=(StateManager&&) = delete;
	void process_state(ServiceProvider& svc, player::Player& player, Game& game);
	void return_to_main_menu(ServiceProvider& svc, player::Player& player);
	void print_stats(ServiceProvider& svc, player::Player& player);

	GameState& get_current_state() const;
	GameState& set_current_state(std::unique_ptr<GameState> gameState);

	std::unique_ptr<GameState> g_current_state{};
};

} // namespace fornani::automa
