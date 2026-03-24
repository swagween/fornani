
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class NightsideWall final : public Cutscene {
  public:
	explicit NightsideWall(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
};

} // namespace fornani
