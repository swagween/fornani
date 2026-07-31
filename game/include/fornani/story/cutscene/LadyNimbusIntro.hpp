
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class LadyNimbusIntroFlags { pre_intro, intro_done, hang, main_scene };

class LadyNimbusIntro final : public Cutscene {
  public:
	explicit LadyNimbusIntro(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render_on_top(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::BitFlags<LadyNimbusIntroFlags> m_flags{};
	graphics::Background m_cloud_sea;
	Animatable m_greatwing;
	util::Cooldown m_intro;
	util::Cooldown m_pre_intro;
	util::Cooldown m_airship_movement;
	gui::TextWriter m_location_text;

	automa::ServiceProvider* m_services;
};

} // namespace fornani
