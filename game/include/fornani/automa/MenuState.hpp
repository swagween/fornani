
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/automa/StateController.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>

namespace fornani::automa {

struct DotInticator {
	components::PhysicsComponent physics{};
	sf::RectangleShape rect{};
};

class MenuState : public GameState {
  public:
	MenuState(ServiceProvider& svc, player::Player& player, std::string_view scene);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win);

  protected:
	std::vector<Option> options{};
	util::Circuit current_selection{1};
	MenuType m_parent_menu{};
	bool m_input_authorized{true};

  private:
	std::array<DotInticator, 2> m_dot_indicators{};
	components::SteeringBehavior m_steering{};
};

} // namespace fornani::automa
