
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <string_view>

namespace fornani {

class Postcard final : public Drawable {
  public:
	Postcard(automa::ServiceProvider& svc, std::string_view tag, std::uint8_t index) : Drawable(svc, "postcard_" + std::string{tag}), m_tag{tag}, m_index{index} {}
	auto operator==(Postcard const& postcard) const -> bool { return postcard.m_tag == m_tag; }

	void render(sf::RenderWindow& win) { win.draw(*this); };
	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tag; }

  private:
	std::string m_tag;
	std::uint8_t m_index;
};

} // namespace fornani
