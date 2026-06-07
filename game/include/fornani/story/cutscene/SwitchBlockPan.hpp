
#pragma once

#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/ID.hpp>

namespace fornani {

class SwitchBlockPan final : public Cutscene {
  public:
	explicit SwitchBlockPan(automa::ServiceProvider& svc, int block_id);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	int m_block_id{};
	util::Cooldown m_wait;
};

} // namespace fornani
