
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/graphics/DayNightShifter.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <unordered_map>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::graphics {

enum class BackgroundAttributes { vertical_lock, horizontal_lock };

struct BackgroundLayer {
	int render_layer{};
	float scroll_speed{};
	float parallax{};
	std::array<sf::RenderTexture, num_cycles()> textures{};
	components::PhysicsComponent physics{};
	sf::Vector2f final_position{};
};

class Background {
  public:
	Background() = default;
	Background(automa::ServiceProvider& svc, std::string_view label);

	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void lock();
	void lock_horizontally();
	void lock_vertically();
	void debug();

	[[nodiscard]] auto locked_horizontally() const -> bool { return m_attributes.test(BackgroundAttributes::horizontal_lock); }
	[[nodiscard]] auto locked_vertically() const -> bool { return m_attributes.test(BackgroundAttributes::vertical_lock); }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }

  private:
	std::vector<BackgroundLayer> layers{};
	sf::Vector2i scroll_pane{};
	sf::Vector2i dimensions{};
	util::BitFlags<BackgroundAttributes> m_attributes{};
	graphics::DayNightShifter shifter{};

	std::string m_label;
};

} // namespace fornani::graphics
