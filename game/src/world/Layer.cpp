
#include "fornani/world/Layer.hpp"
#include "fornani/graphics/DayNightShifter.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/utils/WorldClock.hpp"

#include <ccmath/ccmath.hpp>
#include <ccmath/math/misc/lerp.hpp>

namespace fornani::world {

void Layer::generate_textures(sf::Texture const& tex) {
	if (middleground()) { m_barrier = sf::RenderTexture{}; }
	for (auto cycle{0}; cycle < static_cast<int>(fornani::TimeOfDay::END); ++cycle) {
		auto time = static_cast<fornani::TimeOfDay>(cycle);
		auto const is_day = time == fornani::TimeOfDay::day;
		auto const is_twilight = time == fornani::TimeOfDay::twilight;
		auto const is_night = time == fornani::TimeOfDay::night;
		auto barrier = m_barrier && cycle == 0;

		auto& texture = is_day ? m_texture.day : (is_twilight ? m_texture.twilight : m_texture.night);
		sf::Vector2u size = grid.dimensions * constants::ui_cell_resolution;
		if (!texture.resize(size)) { NANI_LOG_ERROR(m_logger, "Layer texture not created."); }
		texture.clear(sf::Color::Transparent);

		if (barrier) {
			if (!m_barrier->resize(size)) { NANI_LOG_ERROR(m_logger, "Barrier texture not created."); }
			m_barrier->clear(sf::Color::Transparent);
		}
		sf::Sprite tile{tex};
		for (auto& cell : grid.cells) {
			auto x_coord = static_cast<int>((cell.value % constants::tileset_dimensions.x + cycle * constants::tileset_dimensions.x) * constants::i_cell_resolution);
			auto y_coord = static_cast<int>(ccm::floor(cell.value / constants::tileset_dimensions.x) * constants::i_cell_resolution);
			tile.setTextureRect(sf::IntRect({x_coord, y_coord}, constants::i_resolution_vec));
			tile.setPosition(cell.position() / constants::f_scale_factor);
			if (cell.is_occupied() && !cell.is_special()) {
				texture.draw(tile);
				if (barrier) { m_barrier->draw(tile); }
			}
		}
		texture.display();
		if (barrier) { m_barrier->display(); }
	}
}

void Layer::render(automa::ServiceProvider& svc, sf::RenderWindow& win, graphics::DayNightShifter& shifter, float fade, sf::Vector2<float> cam, bool is_bg) const {
	if (background() != is_bg) { return; }
	if (m_barrier) {
		auto spr = sf::Sprite{m_barrier->getTexture()};
		auto border = sf::Vector2i{512, 512};
		spr.setTextureRect(sf::IntRect{-border, {sf::Vector2i{m_barrier->getSize()} + 2 * border}});
		spr.setScale(constants::f_scale_vec);
		spr.setPosition(-cam - sf::Vector2f{2 * border});
		win.draw(spr);
	}
	std::vector<sf::Sprite> sprites{sf::Sprite{m_texture.day.getTexture()}, sf::Sprite{m_texture.twilight.getTexture()}, sf::Sprite{m_texture.night.getTexture()}};
	auto ctr{0};
	for (auto& sprite : sprites) {
		std::uint8_t alpha = ccm::lerp(0, 255, fade);
		std::uint8_t revalpha = ccm::lerp(0, 255, 1.f - fade);
		sprite.setScale(constants::f_scale_vec);
		sprite.setPosition(-cam * m_parallax);
		if (obscuring()) { shifter.render(svc, win, sprite, ctr, alpha); }
		if (reverse_obscuring()) { shifter.render(svc, win, sprite, ctr, revalpha); }
		if (not_obscuring()) { shifter.render(svc, win, sprite, ctr); }
		++ctr;
	}
}

} // namespace fornani::world
