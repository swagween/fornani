
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/RectPath.hpp>
#include <string_view>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

class Notification {
  public:
	Notification(automa::ServiceProvider& svc, std::string_view message);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, float y_offset);
	void insert_input_hint(automa::ServiceProvider& svc, int action_id, int index);

	[[nodiscard]] auto is_complete() const -> bool { return m_fade.is_almost_complete(); }

  private:
	sf::Text m_message;
	std::optional<Drawable> m_input_icon{};
	std::string m_input_code;
	util::RectPath m_path;
	util::Cooldown m_stall;
	util::Cooldown m_fade;
};

} // namespace fornani
