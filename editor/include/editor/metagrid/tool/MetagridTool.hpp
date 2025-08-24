
#pragma once

#include <editor/util/Constants.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace pi {

enum class MetagridToolType : std::uint8_t { cursor, move, END };

class MetagridTool : public fornani::UniquePolymorphic {
  public:
	MetagridTool(fornani::automa::ServiceProvider& svc, MetagridToolType type) : m_type{type}, p_sprite{svc.assets.get_texture("editor_tools")} {
		p_sprite.setTextureRect(sf::IntRect{{constants::tool_size_v * (static_cast<int>(type) + static_cast<int>(ToolType::END)), 0}, {constants::tool_size_v, constants::tool_size_v}});
		p_sprite.setScale({constants::ui_tool_scale_v, constants::ui_tool_scale_v});
	}
	virtual void handle_inputs(Room& room, sf::Vector2f const cam, sf::Vector2f const clicked_position) {}
	virtual void update(sf::Vector2f const position) { p_position = position; }
	virtual void render(sf::RenderWindow& win) {
		p_sprite.setPosition(p_position);
		win.draw(p_sprite);
	}
	void set_original_position(sf::Vector2f to) { p_original_position = to; }

	[[nodiscard]] auto is(MetagridToolType const test) const -> bool { return m_type == test; }
	[[nodiscard]] auto get_label() const -> std::string_view { return p_label; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return p_position; }
	[[nodiscard]] auto get_workspace_coordinates(sf::Vector2f const cam) const -> sf::Vector2i { return sf::Vector2i{(p_position - cam) / spacing_v}; }

  protected:
	sf::Vector2f p_position{};
	sf::Vector2f p_original_position{};
	sf::Sprite p_sprite;
	std::string p_label{};

  private:
	MetagridToolType m_type{};
};

} // namespace pi
