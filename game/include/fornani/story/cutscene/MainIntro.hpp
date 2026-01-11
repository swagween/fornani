
#pragma once

#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class MainIntroFlags { rumble };

class MainIntro final : public Cutscene, public Flaggable<MainIntroFlags> {
  public:
	explicit MainIntro(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
