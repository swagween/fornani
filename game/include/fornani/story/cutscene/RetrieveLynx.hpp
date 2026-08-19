#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class RetrieveLynxFlags : std::uint8_t { pre_intro, intro_done, hang, main_scene, gus_landed, arrived, holding_lynx };

class RetrieveLynx final : public Cutscene {
  public:
	explicit RetrieveLynx(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam) override;
	void render_on_top(sf::RenderWindow& win, sf::Vector2f cam) override;

  private:
	util::BitFlags<RetrieveLynxFlags> m_flags{};
	graphics::Background m_cloud_sea;
	Animatable m_greatwing;
	Drawable m_nighthawk;
	components::SteeringComponent m_nighthawk_steering{};
	components::SteeringComponent m_gus_steering{};
	util::Cooldown m_intro;
	util::Cooldown m_pre_intro;
	util::Cooldown m_airship_movement;
	gui::TextWriter m_location_text;
	std::vector<entity::Effect> m_smoke_effects{};

	sf::Vector2f m_lynx_target{};

	automa::ServiceProvider* m_services;
};

} // namespace fornani
