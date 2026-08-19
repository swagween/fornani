
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <string_view>

namespace fornani {
class QuestTable;
}

namespace fornani::audio {
class Soundboard;
}

namespace fornani::input {
class InputSystem;
}

namespace fornani::data {
class TextManager;
}

namespace fornani::gui {

enum class ResponseDialogFlags : std::uint8_t { hide_portrait };

struct ResponseIndicator {
	sf::Vector2f position{};
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	sf::RectangleShape shape{};
	void update();
};

class ResponseDialog final : public Flaggable<ResponseDialogFlags> {
  public:
	ResponseDialog(data::TextManager& text, dj::Json& source, QuestTable& quest_table, std::string_view key, int index = 0, sf::Vector2f start_position = {}, int stall = 32);

	/// @return true if dialog is still processing inputs, false when exit is requested
	bool handle_inputs(input::InputSystem& controller, audio::Soundboard& soundboard);
	void render(sf::RenderWindow& win);
	void update();
	void set_position(sf::Vector2f to_position);

	[[nodiscard]] auto get_selection() const -> int { return m_selection.get(); }
	[[nodiscard]] auto get_index() const -> int { return m_index; }
	[[nodiscard]] auto get_codes(std::size_t index) const -> std::optional<std::vector<MessageCode>>;

	std::vector<Message> responses{};

  private:
	void stylize(sf::Text& message) const;
	sf::Vector2f m_position{};
	ResponseIndicator m_indicator{};
	FontSpec* m_font;
	int m_index{};
	util::Circuit m_selection;
	util::Cooldown m_stall;

	io::Logger m_logger{"Console"};
};

} // namespace fornani::gui
