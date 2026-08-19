
#pragma once

#include <fornani/automa/GameState.hpp>
#include <fornani/automa/Option.hpp>
#include <fornani/automa/StateController.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/graphics/MenuTheme.hpp>
#include <fornani/gui/Button.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani::automa {

struct DotInticator {
	components::PhysicsComponent physics{};
	sf::RectangleShape rect{};
};

enum class MenuStateFlags : std::uint8_t { option_hovered };

class MenuState : public GameState {
  public:
	MenuState(ServiceProvider& svc, player::Player& player, AppContext& ctx, std::string_view tag);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win);
	void clear_back_button() override { p_back_button.reset(); }

	void set_theme(ServiceProvider& svc, std::string_view theme);

	[[nodiscard]] auto is_mouse_hovering_option() const -> bool { return m_flags.test(MenuStateFlags::option_hovered); }
	[[nodiscard]] auto was_selected(input::InputSystem& input, bool minimenu = false) const -> bool;
	[[nodiscard]] auto went_back(input::InputSystem& input) const -> bool;

  protected:
	std::vector<Option> options{};
	std::optional<gui::Button> p_back_button{};
	util::Circuit current_selection{1};
	MenuType m_parent_menu{};
	bool m_input_authorized{true};
	AppContext* p_app_context;
	sf::RectangleShape p_backdrop{};
	ServiceProvider* p_services;
	TextJustification p_option_justification{TextJustification::center};

  private:
	std::array<DotInticator, 2> m_dot_indicators{};
	components::SteeringBehavior m_steering{};
	util::BitFlags<MenuStateFlags> m_flags{};
};

} // namespace fornani::automa
