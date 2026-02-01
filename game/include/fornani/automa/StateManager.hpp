
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/automa/StateController.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {
class Game;
} // namespace fornani

namespace fornani::player {
class Player;
} // namespace fornani::player

namespace fornani::automa {

struct ServiceProvider;

enum class StateManagerFlags { return_to_main_menu };

class StateManager {

  public:
	StateManager(ServiceProvider& svc, player::Player& player, MenuType type);
	StateManager& operator=(StateManager&&) = delete;
	void process_state(ServiceProvider& svc, player::Player& player, Game& game);
	void return_to_main_menu();
	void print_stats(ServiceProvider& svc, player::Player& player);
	void reload_save(ServiceProvider& svc, int id);

	GameState& get_current_state() const;
	GameState& set_current_state(std::unique_ptr<GameState> gameState);

	std::unique_ptr<GameState> g_current_state{};

  private:
	std::shared_ptr<Slot const> m_slot{std::make_shared<Slot const>()};
	player::Player* m_player;
	SubscriptionGroup m_subscriptions{};
	util::BitFlags<StateManagerFlags> m_flags{};
	io::Logger m_logger{"StateManager"};
};

} // namespace fornani::automa
