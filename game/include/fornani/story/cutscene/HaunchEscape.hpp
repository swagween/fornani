
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/RectPath.hpp>

namespace fornani {

enum class HaunchEscapeFlags { done, over };

class HaunchEscape final : public Cutscene {
  public:
	explicit HaunchEscape(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::Cooldown m_intro;

	util::BitFlags<HaunchEscapeFlags> m_flags{};
};

} // namespace fornani
