#include "fornani/entities/item/Item.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/entities/player/Wardrobe.hpp"
#include <algorithm>

namespace fornani::item {

Item::Item(automa::ServiceProvider& svc, std::string_view label) : label(label), sprite(svc.assets.t_items), ui{.rarity{svc.text.fonts.title}, .quantity{svc.text.fonts.basic}} {
	auto const& in_data = svc.data.item[label];
	metadata.id = in_data["index"].as<int>();
	metadata.naive_title = in_data["naive_title"] ? in_data["naive_title"].as_string() : metadata.naive_title = in_data["title"].as_string();
	metadata.title = in_data["title"].as_string();
	metadata.hidden_description = in_data["hidden_description"] ? in_data["hidden_description"].as_string() : in_data["naive_description"].as_string();
	metadata.naive_description = in_data["naive_description"].as_string();
	metadata.rarity = static_cast<Rarity>(in_data["rarity"].as<int>());
	if (in_data["apparel_type"]) { metadata.apparel_type = static_cast<player::ApparelType>(in_data["apparel_type"].as<int>()); }
	if (in_data["value"]) {
		metadata.value = in_data["value"].as<int>();
		flags.set(ItemFlags::sellable);
	}

	gravitator = vfx::Gravitator(sf::Vector2<float>{}, sf::Color::Transparent, 0.8f);
	gravitator.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.8f}, 0.4f);
	gravitator.collider.physics.maximum_velocity = {640.f, 640.f};

	ui.rarity.setCharacterSize(16);
	ui.quantity.setCharacterSize(16);
	ui.quantity.setFillColor(svc.styles.colors.ui_white);
	switch (metadata.rarity) {
	case Rarity::common:
		ui.rarity.setString("common");
		ui.rarity.setFillColor(svc.styles.colors.bright_orange);
		break;
	case Rarity::uncommon:
		ui.rarity.setString("uncommon");
		ui.rarity.setFillColor(svc.styles.colors.periwinkle);
		break;
	case Rarity::rare:
		ui.rarity.setString("rare");
		ui.rarity.setFillColor(svc.styles.colors.red);
		break;
	case Rarity::priceless:
		ui.rarity.setString("priceless");
		ui.rarity.setFillColor(svc.styles.colors.treasure_blue);
		break;
	}

	if (in_data["unique"].as_bool()) { flags.set(ItemFlags::unique); }
	if (in_data["usable"].as_bool()) { flags.set(ItemFlags::usable); }
	if (in_data["equippable"].as_bool()) { flags.set(ItemFlags::equippable); }
	dimensions = {32.f, 32.f};
	auto u = static_cast<int>(((metadata.id - 1) % 16) * dimensions.x);
	auto v = static_cast<int>(std::floor((static_cast<float>(metadata.id - 1) / 16.f)) * dimensions.y);
	sprite.setTextureRect(sf::IntRect({u, v}, static_cast<sf::Vector2<int>>(dimensions)));

	//for debug
	drawbox.setSize(dimensions);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setOutlineThickness(-1);
}

void Item::update(automa::ServiceProvider& svc, int index, int items_per_row, sf::Vector2<float> offset) {
	gravitator.update(svc);
	auto y_pos = ui.pad.y + static_cast<float>(index / items_per_row) * ui.spacing;
	auto x_pos = ui.pad.x + static_cast<float>(index % items_per_row) * ui.spacing;
	auto inv_pos = sf::Vector2<float>{x_pos, y_pos} + ui.offset - sf::Vector2<float>{8.f, 8.f} + offset;
	if (flags.test(ItemFlags::unique)) {
		variables.quantity = std::clamp(variables.quantity, 0, 1);
	} else {
		ui.quantity.setString(std::format("x{}", variables.quantity));
		ui.quantity.setPosition(inv_pos + dimensions);
		ui.rarity.setOrigin({ui.rarity.getLocalBounds().size.x, 0.f});
	}
	gravitator.set_target_position(inv_pos);
	sprite.setPosition(gravitator.position());
	drawbox.setPosition(sprite.getPosition());
	selection_index = index;
	selected() ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.blue);
}

void Item::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	svc.greyblock_mode() ? win.draw(drawbox) : win.draw(sprite);
	if (!flags.test(ItemFlags::unique)) {
		win.draw(ui.quantity);
		if (selected()) { win.draw(ui.rarity); }
	}
}

void Item::add_item(int amount) { variables.quantity += amount; }

void Item::subtract_item(int amount) { variables.quantity -= amount; }

void item::Item::set_id(int new_id) { metadata.id = new_id; }

void Item::select() { ui_flags.set(UIFlags::selected); }

void Item::deselect() { ui_flags.reset(UIFlags::selected); }

void Item::toggle_equip() { is_equipped() ? state.reset(ItemState::equipped) : state.set(ItemState::equipped); }

void Item::set_rarity_position(sf::Vector2<float> position) { ui.rarity.setPosition(position); }

void Item::set_offset(sf::Vector2<float> offset) { ui.offset = offset; }

} // namespace player
