
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include <fornani/gui/console/TextWriter.hpp>
#include "fornani/utils/BitFlags.hpp"

#include <optional>

namespace fornani::gui {

enum class DescriptionGizmoFlags { text_only };

class DescriptionGizmo : public Gizmo {
  public:
	/// @brief A text description for the highlighted item. Should be used for any gizmo that has item views.
	DescriptionGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement, sf::IntRect lookup, sf::FloatRect text_bounds, sf::Vector2f start_position);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

	void write(automa::ServiceProvider& svc, std::string_view message, sf::Font& font);
	void set_offset(sf::Vector2f const offset = {});
	void adjust_bounds(sf::Vector2f const adjustment);
	void flush() { m_text = {}; }

	void set_text_only(bool flag) { flag ? m_flags.set(DescriptionGizmoFlags::text_only) : m_flags.reset(DescriptionGizmoFlags::text_only); }

  private:
	util::BitFlags<DescriptionGizmoFlags> m_flags{};
	sf::Sprite m_sprite;
	std::optional<TextWriter> m_text{};
	sf::FloatRect m_bounds;
	sf::Vector2f m_text_offset{};
	sf::Vector2f m_additional_offset{};
};

} // namespace fornani::gui
