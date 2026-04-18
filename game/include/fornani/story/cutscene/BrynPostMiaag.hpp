#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class BrynPostMiaag final : public Cutscene {
  public:
	explicit BrynPostMiaag(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::Counter m_failsafe{};
};

} // namespace fornani
