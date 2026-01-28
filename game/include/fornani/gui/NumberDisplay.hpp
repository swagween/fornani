
#pragma once

#include <fornani/graphics/Drawable.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

class NumberDisplay : public Drawable {
  public:
	NumberDisplay(automa::ServiceProvider& svc, int const amount, int const item_id);
	void set(int const amount);
	void render(sf::RenderWindow& win, sf::Vector2f position);
	[[nodiscard]] auto matches(int const check) const -> bool { return m_item_id == check; }

  private:
	int m_amount{};
	int m_item_id{};
};

} // namespace fornani::gui
