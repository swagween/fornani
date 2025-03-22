#pragma once

#include "fornani/story/Cutscene.hpp"

namespace fornani {

class LadyNimbusIntro final : public Cutscene {
  public:
	virtual ~LadyNimbusIntro() = default;
	explicit LadyNimbusIntro(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;
};

} // namespace fornani
