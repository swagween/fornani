
#include "fornani/gui/ItemWidget.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

ItemWidget::ItemWidget(automa::ServiceProvider& svc, ItemWidgetType type, int id)
	: m_id{id}, m_type{type}, m_sprites{.sticker{svc.assets.get_texture("item_sticker")}, .item = type == ItemWidgetType::item ? sf::Sprite{svc.assets.get_texture("inventory_items")} : sf::Sprite{svc.assets.get_texture("inventory_guns")}},
	  m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "sticker", 128, util::InterpolationType::quadratic} {
	bring_in();
	auto dim = type == ItemWidgetType::item ? sf::Vector2f{16.f, 16.f} : sf::Vector2f{24.f, 24.f};
	m_sprites.item.setScale(constants::f_scale_vec);
	m_sprites.sticker.setScale(constants::f_scale_vec);
	m_sprites.sticker.setOrigin(m_sprites.sticker.getLocalBounds().getCenter());
	m_sprites.item.setOrigin(dim / 2.f);
	sparkler = vfx::Sparkler(svc, constants::f_cell_vec, colors::ui_white, "item");
}

void ItemWidget::update(automa::ServiceProvider& svc) {
	sparkler.update(svc);
	m_path.update();
	m_sprites.sticker.setPosition(m_path.get_position());
	m_sprites.item.setPosition(m_path.get_position());
	sparkler.set_position(m_sprites.sticker.getPosition() - sparkler.get_dimensions() * 0.5f);
}

void ItemWidget::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {
	auto dim = m_type == ItemWidgetType::item ? sf::Vector2i{16, 16} : sf::Vector2i{24, 24};
	win.draw(m_sprites.sticker);
	auto idx = m_type == ItemWidgetType::item ? m_id - 1 : m_id;
	auto u = (idx % 16) * dim.x;
	auto v = static_cast<int>(std::floor((idx / 16)) * dim.y);
	m_sprites.item.setTextureRect(sf::IntRect({u, v}, dim));
	win.draw(m_sprites.item);
	sparkler.render(win, {});
}

void ItemWidget::bring_in() { m_path.set_section("in"); }

void ItemWidget::send_out() { m_path.set_section("out"); }

} // namespace fornani::gui
