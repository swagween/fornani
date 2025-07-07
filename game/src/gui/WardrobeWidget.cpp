
#include "fornani/gui/WardrobeWidget.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

gui::WardrobeWidget::WardrobeWidget(automa::ServiceProvider& svc)
	: m_base{sf::Sprite{svc.assets.get_texture("wardrobe_base")}}, m_outfit{sf::Sprite{svc.assets.get_texture("wardrobe_outfits")}}, out_nani{sf::Sprite{svc.assets.get_texture("null")}} {
	background.setFillColor(colors::pioneer_black);
	background.setSize(f_dimensions());
	if (!nani.resize({128, 256})) { NANI_LOG_WARN(m_logger, "nani.resize() failed!"); }
	m_base.setScale(constants::f_scale_vec);
	m_outfit.setScale(constants::f_scale_vec);
}

void WardrobeWidget::update(player::Player& player) {
	player.catalog.wardrobe.update(player.texture_updater);
	nani.clear(sf::Color::Transparent);
	nani.draw(m_base);
	for (auto piece{static_cast<int>(player::ApparelType::END) - 1}; piece >= 0; --piece) {
		m_outfit.setTextureRect(get_lookup(static_cast<player::ApparelType>(piece), player));
		nani.draw(m_outfit);
	}
	nani.display();
	out_nani = sf::Sprite(nani.getTexture());
}

void WardrobeWidget::render(sf::RenderWindow& win, sf::Vector2f cam) {
	background.setPosition(position - cam);
	win.draw(background);
	out_nani.setPosition(position - cam);
	win.draw(out_nani);
}

auto WardrobeWidget::get_lookup(player::ApparelType type, player::Player& player) const -> sf::IntRect {
	auto i_type{static_cast<int>(type)};
	auto variant{player.catalog.wardrobe.get_variant(type)};
	auto coords{sf::Vector2i{variant, i_type}};
	return sf::IntRect{coords.componentWiseMul(m_dimensions), m_dimensions};
}

} // namespace fornani::gui
