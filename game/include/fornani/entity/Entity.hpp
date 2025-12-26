
#pragma once

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {
class Player;
}

namespace fornani {

class Entity : public Animatable, public IWorldPositionable {
  public:
	explicit Entity(automa::ServiceProvider& svc, dj::Json const& in, std::string_view label, sf::Vector2i dim = constants::i_cell_vec);
	explicit Entity(automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim = {1, 1});

	virtual std::unique_ptr<Entity> clone() const;
	virtual void serialize(dj::Json& out);
	virtual void unserialize(dj::Json const& in);
	virtual void expose();
	virtual void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player);
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam, float size);
	void set_position(sf::Vector2u to_position);
	bool repeatable{};
	bool overwrite{};
	bool unique{};

	bool highlighted{};
	bool copyable{true};
	bool selected{};
	bool moved{};
	bool stackable{};

	// helpers
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto contains_point(sf::Vector2u test) const -> bool;

  protected:
	bool m_editor{};
	bool m_textured{true};
	io::Logger m_logger{"Pioneer"};

  private:
	int m_id{};
	std::string m_label{};
};

} // namespace fornani
