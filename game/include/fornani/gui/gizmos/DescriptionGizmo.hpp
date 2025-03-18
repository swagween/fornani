
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/TextWriter.hpp"

#include <optional>

namespace fornani::gui {

class DescriptionGizmo : public Gizmo {
  public:
	/// @brief A text description for the highlighted item. Should be used for any gizmo that has item views.
	DescriptionGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement, sf::IntRect lookup, sf::FloatRect text_bounds);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	void write(automa::ServiceProvider& svc, std::string_view message);

  private:
	sf::Sprite m_sprite;
	std::optional<TextWriter> m_text{};
	sf::FloatRect m_bounds;
	sf::Vector2f m_text_offset{};
};

} // namespace fornani::gui
