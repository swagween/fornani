
#pragma once

#include <vector>
#include "Spark.hpp"

namespace fornani::vfx {

class Sparkler {
  public:
	Sparkler() = default;
	explicit Sparkler(automa::ServiceProvider& svc, sf::Vector2f dimensions = {2.f, 2.f}, sf::Color color = sf::Color::White, std::string_view type = "");
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos);
	void set_dimensions(sf::Vector2f dim);
	void activate();
	void deactivate();
	void set_color(sf::Color to_color);
	void set_rate(float to_rate);
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2f { return dimensions; }

  private:
	std::vector<Spark> sparkles{};
	sf::Vector2f dimensions{};
	sf::Vector2f position{};
	std::string_view type{};

	automa::ServiceProvider* m_services;

	struct {
		float rate{};
	} behavior{};

	sf::Color color{};
	sf::RectangleShape drawbox{}; // for debug
	bool active{true};
};

} // namespace fornani::vfx
