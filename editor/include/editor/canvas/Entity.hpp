
#pragma once

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace pi {

class Entity : public fornani::Drawable {
  public:
	explicit Entity(fornani::automa::ServiceProvider& svc, dj::Json const& in, std::string_view label);
	explicit Entity(fornani::automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim = {1, 1});

	virtual std::unique_ptr<Entity> clone() const;
	virtual void serialize(dj::Json& out);
	virtual void unserialize(dj::Json const& in);
	virtual void expose();
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam, float size);
	void set_position(sf::Vector2u to_position);
	bool repeatable{};
	bool overwrite{};
	bool unique{};

	bool highlighted{};
	bool selected{};

	// helpers
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto get_position() const -> sf::Vector2u { return position; }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2u { return dimensions; }
	[[nodiscard]] auto f_dimensions() const -> sf::Vector2f { return sf::Vector2f(dimensions); }
	[[nodiscard]] auto contains_position(sf::Vector2u test) const -> bool {
		for (auto i{0u}; i < dimensions.x; ++i) {
			for (auto j{0u}; j < dimensions.y; ++j) {
				if (position + sf::Vector2u{i, j} == test) { return true; }
			}
		}
		return false;
	}

  protected:
	bool m_textured{true};
	sf::Vector2<std::uint32_t> position{};
	sf::Vector2<std::uint32_t> dimensions{};
	fornani::io::Logger m_logger{"Pioneer"};

  private:
	int id{};
	std::string m_label{};
};

} // namespace pi
