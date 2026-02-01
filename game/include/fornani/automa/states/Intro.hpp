
#pragma once

#include <fornani/automa/GameplayState.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/world/Camera.hpp>

namespace fornani::automa {

enum class IntroFlags { complete, established, cutscene_started, cutscene_over, console_message };

struct Nighthawk final : public Animatable {
	Nighthawk(ServiceProvider& svc) : Animatable(svc, "scenery_distant_nighthawk", {11, 11}) {}
	components::SteeringComponent steering;
	float z{};
};

class Intro final : public GameplayState, public Flaggable<IntroFlags> {
  public:
	Intro(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void toggle_pause_menu(ServiceProvider& svc);

	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};

  private:
	gui::TextWriter m_location_text;
	graphics::Background m_cloud_sea;
	graphics::Background m_cloud;
	Animatable m_airship;
	std::vector<Nighthawk> m_nighthawks;
	util::Cooldown m_intro_shot;
	util::Cooldown m_wait;
	util::Cooldown m_end_wait;
	util::Cooldown m_attack_fadeout;
	std::optional<LightShader> m_world_shader{};
};

} // namespace fornani::automa
