
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class PioneerBaseDebriefFlags : std::uint8_t { start, end };

class PioneerBaseDebrief final : public Cutscene {
  public:
	explicit PioneerBaseDebrief(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::BitFlags<PioneerBaseDebriefFlags> m_flags{};
};

} // namespace fornani
