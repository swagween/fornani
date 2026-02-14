
#pragma once

#include <djson/json.hpp>
#include <fornani/automa/MenuState.hpp>
#include <vector>

namespace fornani::automa {

struct Credit {
	sf::Text name;
	sf::Text description;
	int line_breaks{};
};

class CreditsMenu final : public MenuState {
  public:
	CreditsMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void refresh(ServiceProvider& svc);

  private:
	dj::Json m_data{};
	std::vector<Credit> m_credits;
	util::Cooldown m_loading;
};

} // namespace fornani::automa
