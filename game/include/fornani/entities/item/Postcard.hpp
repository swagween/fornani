
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <string_view>

namespace fornani {

class Postcard final : public Drawable {
  public:
	Postcard(automa::ServiceProvider& svc, std::string_view tag) : Drawable(svc, "postcard_" + std::string{tag}), m_tag{tag} {}
	void render(sf::RenderWindow& win) { win.draw(*this); };
	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tag; }

  private:
	std::string m_tag;
};

} // namespace fornani
