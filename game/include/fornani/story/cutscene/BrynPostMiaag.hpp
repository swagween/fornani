#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class BrynPostMiaag final : public Cutscene {
  public:
	explicit BrynPostMiaag(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;
};

} // namespace fornani
