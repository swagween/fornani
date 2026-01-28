
#include <fornani/gui/NumberDisplay.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

NumberDisplay::NumberDisplay(automa::ServiceProvider& svc, int const amount, int const item_id) : Drawable{svc, "small_numbers"}, m_amount{amount}, m_item_id{item_id} {}

void NumberDisplay::set(int const to) { m_amount = to; }

void NumberDisplay::render(sf::RenderWindow& win, sf::Vector2f position) {
	auto digits = std::to_string(m_amount);
	set_origin({digits.length() * 5.f, 0.f});
	for (auto [i, ch] : std::views::enumerate(digits)) {
		if (ch < '0' || ch > '9') { continue; }
		int digit = ch - '0';
		set_texture_rect(sf::IntRect{{digit * 5, 0}, {5, 7}});
		set_position(position + sf::Vector2f{static_cast<float>(i) * 10.f, 0.f});
		win.draw(*this);
	}
}

} // namespace fornani::gui
