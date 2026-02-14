
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/Widget.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

Widget::Widget(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2i dim, int index, sf::Vector2f root) : Animatable{svc, tag, dim}, index(index), m_root{root}, m_shaking{800} { m_steering.physics.set_global_friction(0.995f); }

void Widget::update(automa::ServiceProvider& svc, int max) {
	m_shaking.update();
	m_steering.seek(m_root, 0.0001f);
	m_steering.physics.simple_update();
	maximum = max;
}

void Widget::render(sf::RenderWindow& win) {
	set_texture_rect(sf::IntRect{sf::Vector2i{static_cast<int>(p_state.actual), static_cast<int>(m_type)}.componentWiseMul(get_dimensions()), get_dimensions()});
	m_shaking.is_complete() ? set_position(m_root + m_offset) : set_position(m_steering.physics.position + m_offset);
	if (index < maximum) { win.draw(*this); }
}

} // namespace fornani::gui
