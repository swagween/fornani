
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class PioneerBaseDebrief final : public Cutscene {
  public:
	explicit PioneerBaseDebrief(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
