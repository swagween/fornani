
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/Gizmo.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/NumberDisplay.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/NineSlice.hpp>

namespace fornani::arms {
class Layout;
}

namespace fornani::gui {

enum class LayoutGizmoFlags : std::uint8_t { selected, item_select_mode, fixture_select_mode };

class LayoutGizmo : public Gizmo, public Flaggable<LayoutGizmoFlags> {
  public:
	LayoutGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	void toggle();
	void set_selected(bool const to);
	void set_layout(arms::Weapon& weapon);
	void set_selection(int const to) {
		if (to > m_selection.get_order() - 1) {
			m_selection.set(m_selection.get_order() - 1);
			return;
		}
		m_selection.set(to);
	}

	void enter_item_select_mode();
	void enter_fixture_select_mode();
	void exit_item_select_mode();
	void exit_fixture_select_mode();

	void close_menu();

  private:
	void open_slot_menu(arms::SlotStatus status);
	void handle_menu_selection(audio::Soundboard& soundboard);
	auto handle_activated_slot(audio::Soundboard& soundboard) -> bool;
	void handle_deactivated_slot(audio::Soundboard& soundboard);
	void exit_menu_if_finished();
	[[nodiscard]] auto make_menu(std::string_view key, std::initializer_list<std::string_view> options) -> MiniMenu;

	void debug();

  private:
	util::NineSlice m_board;
	Animatable m_item_sprite;
	Animatable m_slots;
	Animatable m_fixtures;
	util::RectPath m_path;
	std::vector<NumberDisplay> m_number_displays{};
	std::optional<MiniMenu> m_menu{};

	util::Circuit m_selection{arms::layout_size_v};
	std::size_t m_slot_index{};

	arms::Layout* m_current_layout{};
	arms::PinLocation m_current_pin{arms::PinLocation::top_left};
	std::optional<std::string> m_current_item{};
	std::vector<std::string> m_selectable_items{};
	std::size_t m_item_selection{};
	std::vector<arms::PinLocation> m_selectable_fixtures{};
	std::size_t m_fixture_selection{};

	player::Player* m_player;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::gui
