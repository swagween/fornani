#include "Item.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"
#include <algorithm>

namespace item {

Item::Item(automa::ServiceProvider& svc, std::string_view label) : label(label) {
	auto const& in_data = svc.data.item[label];
	metadata.id = in_data["index"].as<int>();
	metadata.naive_title = in_data["naive_title"] ? in_data["naive_title"].as_string() : metadata.naive_title = in_data["title"].as_string();
	metadata.title = in_data["title"].as_string();
	metadata.hidden_description = in_data["hidden_description"] ? in_data["hidden_description"].as_string() : in_data["naive_description"].as_string();
	metadata.naive_description = in_data["naive_description"].as_string();
	if (in_data["unique"].as_bool()) { flags.set(ItemFlags::unique); }
	dimensions = {32.f, 32.f};
	sprite.setTexture(svc.assets.t_items);
	auto u = static_cast<int>(((metadata.id - 1) % 16) * dimensions.x);
	auto v = static_cast<int>(std::floor((static_cast<float>(metadata.id - 1) / 16.f)) * dimensions.y);
	sprite.setTextureRect(sf::IntRect({u, v}, static_cast<sf::Vector2<int>>(dimensions)));

	//for debug
	drawbox.setSize(dimensions);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setOutlineThickness(-1);
}

void Item::update(automa::ServiceProvider& svc, int index) {
	if (flags.test(ItemFlags::unique)) { variables.quantity = std::clamp(variables.quantity, 0, 1); }
	sprite.setPosition({index * ui.spacing + ui.pad.x, ui.pad.y});
	drawbox.setPosition(sprite.getPosition());
	selection_index = index;
	selected() ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.blue);
}

void Item::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { svc.greyblock_mode() ? win.draw(drawbox) : win.draw(sprite); }

void Item::add_item(int amount) { variables.quantity += amount; }

void item::Item::set_id(int new_id) { metadata.id = new_id; }

void Item::select() { ui_flags.set(UIFlags::selected); }

void Item::deselect() { ui_flags.reset(UIFlags::selected); }

} // namespace player
