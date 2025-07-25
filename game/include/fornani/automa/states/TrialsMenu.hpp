
#pragma once

#include <fornani/automa/MenuState.hpp>

namespace fornani::automa {

struct TrialListing {
	sf::Text tag;
	sf::Text time;
};

class TrialsMenu final : public MenuState {
  public:
	TrialsMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

  private:
	void switch_selections(ServiceProvider& svc);
	std::vector<TrialListing> m_listings;
	util::Cooldown m_loading;
};

} // namespace fornani::automa
