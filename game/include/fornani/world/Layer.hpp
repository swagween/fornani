
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include "fornani/io/Logger.hpp"
#include "fornani/world/Grid.hpp"

#include <optional>

namespace fornani {
class LightShader;
class Palette;
} // namespace fornani

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::graphics {
class DayNightShifter;
}

namespace fornani::world {

enum class LayerType : std::uint8_t { background, middleground, foreground, reverse_obscuring, obscuring };
enum class LayerFlags : std::uint8_t { ignore_lighting };

struct LayerTexture {
	sf::RenderTexture day{};
	sf::RenderTexture twilight{};
	sf::RenderTexture night{};
};

class Layer {
  public:
	friend class Map;
	Layer(std::uint8_t o, sf::Vector2i partition, sf::Vector2<std::uint32_t> dim, dj::Json& source, float spacing, bool has_obscuring, bool has_reverse_obscuring, float parallax = 1.f, bool ignore_lighting = false)
		: m_render_order(o), collidable(o == partition.x), dimensions(dim), grid(dim, source, spacing), m_parallax{parallax} {
		auto order = static_cast<int>(o);
		if (order < partition.x) { m_type = LayerType::background; }
		if (order == partition.x) { m_type = LayerType::middleground; }
		if (order > partition.x) { m_type = LayerType::foreground; }
		if (order == partition.y - 2 && has_reverse_obscuring) { m_type = LayerType::reverse_obscuring; }
		if (order == partition.y - 1 && has_obscuring) { m_type = LayerType::obscuring; }
		ignore_lighting ? m_flags.set(LayerFlags::ignore_lighting) : m_flags.reset(LayerFlags::ignore_lighting);
	}
	[[nodiscard]] auto background() const -> bool { return m_type == LayerType::background; }
	[[nodiscard]] auto foreground() const -> bool { return m_type == LayerType::foreground; }
	[[nodiscard]] auto middleground() const -> bool { return m_type == LayerType::middleground; }
	[[nodiscard]] auto obscuring() const -> bool { return m_type == LayerType::obscuring; }
	[[nodiscard]] auto reverse_obscuring() const -> bool { return m_type == LayerType::reverse_obscuring; }
	[[nodiscard]] auto not_obscuring() const -> bool { return !obscuring() && !reverse_obscuring(); }
	[[nodiscard]] auto get_render_order() const -> std::uint8_t { return m_render_order; }
	[[nodiscard]] auto get_i_render_order() const -> int { return static_cast<int>(m_render_order); }
	[[nodiscard]] auto get_layer_type() const -> LayerType { return m_type; }
	[[nodiscard]] auto ignore_lighting() const -> bool { return m_flags.test(LayerFlags::ignore_lighting); }
	bool collidable{};
	sf::Vector2<std::uint32_t> dimensions{};

	Grid grid;

  private:
	void generate_textures(sf::Texture const& tex);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, graphics::DayNightShifter& shifter, float fade, sf::Vector2f cam, bool is_bg = false, bool day_night_shift = false) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, LightShader& shader, Palette& palette, graphics::DayNightShifter& shifter, float fade, sf::Vector2f cam, bool is_bg = false, bool day_night_shift = false) const;
	std::uint8_t m_render_order{};
	LayerType m_type{};
	LayerTexture m_texture{};
	std::optional<sf::RenderTexture> m_barrier{};
	float m_parallax;
	util::BitFlags<LayerFlags> m_flags{};

	io::Logger m_logger{"world"};
};
} // namespace fornani::world
