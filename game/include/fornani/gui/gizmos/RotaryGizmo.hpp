
#pragma once

#include <fornani/gui/Gizmo.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/CyclicLerp.hpp>

namespace fornani::gui {

class RotaryGizmo final : public Gizmo {
  public:
	explicit RotaryGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;

	void debug();

  private:
	Animatable m_sprite;
	util::Circuit m_selection;
	CyclicLerp m_frame_lerp;
	util::RectPath m_path;
	float m_previous_position{};
};

} // namespace fornani::gui
