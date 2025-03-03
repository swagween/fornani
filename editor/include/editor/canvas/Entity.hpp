
#pragma once

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

namespace pi {

class Entity {
  public:
	explicit Entity(std::string const& label, int const id = 0, sf::Vector2<uint32_t> dimensions = {}) : id(id), label(label), dimensions(dimensions) {};
	virtual ~Entity() = default;
	virtual std::unique_ptr<Entity> clone() const;
	virtual void serialize(dj::Json& out);
	virtual void unserialize(dj::Json& in);
	virtual void expose();
	virtual void render(sf::RenderWindow& win, sf::Vector2<float> cam, float size);
	void set_position(sf::Vector2u to_position);
	bool repeatable{};
	bool overwrite{};
	bool unique{};

	bool highlighted{};
	bool selected{};

	// helpers
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_label() const -> std::string { return label; }
	[[nodiscard]] auto get_position() const -> sf::Vector2u { return position; }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2u { return dimensions; }
	[[nodiscard]] auto f_dimensions() const -> sf::Vector2<float> { return sf::Vector2<float>(dimensions); }
	[[nodiscard]] auto contains_position(sf::Vector2u test) const -> bool {
		for (auto i{0u}; i < dimensions.x; ++i) {
			for (auto j{0u}; j < dimensions.y; ++j) {
				if (position + sf::Vector2u{i, j} == test) { return true; }
			}
		}
		return false;
	}

  private:
	int id{};
	std::string label{};
	sf::Vector2<uint32_t> position{};
	sf::Vector2<uint32_t> dimensions{};
};

} // namespace pi
