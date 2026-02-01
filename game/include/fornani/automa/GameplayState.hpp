
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/events/Subscription.hpp>

namespace fornani::automa {

class GameplayState : public GameState {
  public:
	GameplayState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine) {};
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win) {};

  protected:
	void play_song_by_id(int id);
	std::shared_ptr<Slot const> p_slot{std::make_shared<Slot const>()};
	ServiceProvider* p_services;
};

} // namespace fornani::automa
