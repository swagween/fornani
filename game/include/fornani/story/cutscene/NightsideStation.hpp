
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class NightsideStation final : public Cutscene {
  public:
	explicit NightsideStation(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
};

} // namespace fornani
