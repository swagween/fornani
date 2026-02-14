
#pragma once

#include <fornani/gui/Gizmo.hpp>
#include <fornani/gui/gizmos/HotbarGizmo.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/CyclicLerp.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::gui {

enum class RotaryGizmoFlags { push_to_hotbar };

class RotaryGizmo final : public Gizmo, public Flaggable<RotaryGizmoFlags> {
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
	std::optional<std::unique_ptr<HotbarGizmo>> m_hotbar{};
	Animatable m_sprite;
	Animatable m_gun_display;
	Animatable m_gun_selector;
	Drawable m_dashboard_rail;
	util::Circuit m_selection;
	CyclicLerp m_frame_lerp;
	util::RectPath m_path;
	util::RectPath m_rail_path;
	float m_previous_position{};
	int m_gun_id{};
};

} // namespace fornani::gui
