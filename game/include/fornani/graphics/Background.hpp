
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/graphics/DayNightShifter.hpp"

#include <unordered_map>

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::bg {

enum class BackgroundAttributes : uint8_t { lock };

struct BackgroundLayer {
	int render_layer{};
	float scroll_speed{};
	float parallax{};
	std::vector<sf::Sprite> sprites{};
	components::PhysicsComponent physics{};
	sf::Vector2<float> final_position{};
};

class Background {
  public:
	Background() = default;
	Background(automa::ServiceProvider& svc, int bg_id);

	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void lock();
	void lock_horizontally();
	void lock_vertically();
	void debug();

	[[nodiscard]] auto locked_horizontally() const -> bool { return attributes.horizontal.test(BackgroundAttributes::lock); }
	[[nodiscard]] auto locked_vertically() const -> bool { return attributes.vertical.test(BackgroundAttributes::lock); }

  private:
	std::vector<BackgroundLayer> layers{};
	sf::Vector2<int> scroll_pane{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<int> start_offset{};
	bool* b_debug{};
	std::unordered_map<int, std::string_view> labels{};
	struct {
		util::BitFlags<BackgroundAttributes> vertical{};
		util::BitFlags<BackgroundAttributes> horizontal{};
	} attributes{};
	graphics::DayNightShifter shifter{};
};

} // namespace bg
