
#pragma once

#include <fornani/automa/MenuState.hpp>
#include <fornani/graphics/Animatable.hpp>

namespace fornani::automa {

struct CourseListing {
	std::string label{};
	int id{};
};

struct TrialListing {
	sf::Text tag;
	sf::Text time;
	std::uint8_t star_rating{};
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
	std::vector<CourseListing> m_courses;
	util::Cooldown m_loading;
	Animatable m_stars;
};

} // namespace fornani::automa
