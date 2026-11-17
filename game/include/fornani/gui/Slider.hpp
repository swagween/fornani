
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani::audio {
class Soundboard;
}

namespace fornani::gui {

enum class SliderState { neutral, hovered, engaged };

class Slider {
  public:
	explicit Slider(float const size, sf::Vector2f const position = {});
	void handle_input(input::InputSystem& input, audio::Soundboard& soundboard);
	void update(float const percentage);
	void render(sf::RenderTarget& win);
	void set_position(sf::Vector2f const position);

	[[nodiscard]] auto get_output() const -> std::optional<float> { return m_state == SliderState::engaged ? std::optional<float>{m_output} : std::nullopt; }
	[[nodiscard]] auto is_neutral() const -> bool { return m_state == SliderState::neutral; }
	[[nodiscard]] auto is_hovered() const -> bool { return m_state == SliderState::hovered; }
	[[nodiscard]] auto is_engaged() const -> bool { return m_state == SliderState::engaged; }

  private:
	void set_geometry();

  private:
	sf::RectangleShape m_border{};
	sf::RectangleShape m_current{};
	sf::RectangleShape m_total{};
	sf::RectangleShape m_puck{};

	SliderState m_state{};

	float m_output{};

	float m_size{};
};

} // namespace fornani::gui
