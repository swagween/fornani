
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/world/Camera.hpp>

namespace fornani::automa {

enum class IntroFlags { complete, established };

class Intro final : public GameState, public Flaggable<IntroFlags> {
  public:
	Intro(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void toggle_pause_menu(ServiceProvider& svc);

	std::optional<std::unique_ptr<gui::PauseWindow>> pause_window{};

  private:
	gui::TextWriter m_location_text;
	world::Map map;
	graphics::Background m_cloud_sea;
	graphics::Background m_cloud;
	Animatable m_airship;
	util::Cooldown m_intro_shot;
	util::Cooldown m_wait;
	std::optional<LightShader> m_world_shader{};
};

} // namespace fornani::automa
