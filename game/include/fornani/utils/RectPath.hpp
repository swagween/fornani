
#pragma once

#include "fornani/utils/Cooldown.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <vector>

namespace fornani::data {
class ResourceFinder;
}
namespace fornani::util {

struct PathSection {
	std::string label{};
	std::vector<sf::FloatRect> path{};
};

class RectPath {
  public:
	RectPath(data::ResourceFinder& finder, std::filesystem::path source, std::string_view type, int interpolation = 64);
	void update();
	void set_section(std::string_view to_section);
	void reset();
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_current_position; }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2f { return m_current_dimensions; }
	[[nodiscard]] auto finished() const -> bool;
  private:
	std::vector<PathSection> m_sections{};
	int m_current_section{};
	int m_current_step{};
	float m_scale{};
	sf::Vector2<float> m_current_position{};
	sf::Vector2<float> m_current_dimensions{};
	util::Cooldown m_interpolation;
};

} // namespace fornani::util
