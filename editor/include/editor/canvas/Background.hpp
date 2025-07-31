
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#include "fornani/io/Logger.hpp"

namespace fornani {
class ResourceFinder;
}

namespace pi {

enum class Backdrop { black, night, dusk, woods, canopy, END };
class BackgroundType {
  public:
	BackgroundType(Backdrop type) : type(type) {
		switch (type) {
		case Backdrop::black: label = "black"; break;
		case Backdrop::night: label = "night"; break;
		case Backdrop::dusk: label = "dusk"; break;
		case Backdrop::woods: label = "woods"; break;
		case Backdrop::canopy: label = "canopy"; break;
		default: label = "<none>"; break;
		}
	}
	[[nodiscard]] auto get_label_char() const -> char const* { return label.c_str(); };
	[[nodiscard]] auto get_label() const -> std::string { return label; };
	[[nodiscard]] auto get_type() const -> Backdrop { return type; };
	[[nodiscard]] auto get_i_type() const -> int { return static_cast<int>(type); };

  private:
	Backdrop type{};
	std::string label{};
};

class Canvas;

struct BackgroundLayer {
	BackgroundLayer(sf::Texture const& texture, int index, float speed, float parallax);
	int render_layer{};
	float scroll_speed{};
	float parallax{};
	sf::Sprite sprite;
	sf::Vector2f position{};
	sf::Vector2f velocity{};
	sf::Vector2f final_position{};
};

class Background {
  public:
	Background(fornani::ResourceFinder& finder, Backdrop backdrop);

	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2f& campos);
	void debug();

	BackgroundType type;

  private:
	std::vector<BackgroundLayer> layers{};
	sf::Vector2<int> scroll_pane{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<int> start_offset{};
	bool* b_debug{};
	sf::Texture texture{};

	fornani::io::Logger m_logger{"pioneer"};
};

} // namespace pi
