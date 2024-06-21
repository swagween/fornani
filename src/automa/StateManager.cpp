
#include "StateManager.hpp"

namespace automa {
StateManager::StateManager() {
	g_current_state = std::make_unique<MainMenu>();
}

StateManager::~StateManager() {}

auto StateManager::get_current_state() -> automa::GameState& {
	assert(g_current_state);
	return *g_current_state;
}

auto StateManager::set_current_state(std::unique_ptr<automa::GameState> gameState) -> automa::GameState& {
	g_current_state = std::move(gameState);
	return get_current_state();
}

}
