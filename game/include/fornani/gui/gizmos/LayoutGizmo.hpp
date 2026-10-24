
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/Gizmo.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/NineSlice.hpp>

namespace fornani::arms {
class Layout;
}

namespace fornani::gui {

enum class LayoutGizmoFlags : std::uint8_t { selected };

class LayoutGizmo : public Gizmo, public Flaggable<LayoutGizmoFlags> {
  public:
	LayoutGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	void set_layout(arms::Weapon& weapon);
	void set_selection(int const to) {
		if (to > m_selection.get_order() - 1) {
			m_selection.set(m_selection.get_order() - 1);
			return;
		}
		m_selection.set(to);
	}

  private:
	void debug();

  private:
	util::NineSlice m_board;
	Animatable m_slots;
	util::RectPath m_path;

	util::Circuit m_selection{arms::layout_size_v};

	arms::Layout* m_current_layout{};
};

} // namespace fornani::gui
