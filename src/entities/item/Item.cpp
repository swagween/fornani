#include "Item.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"
#include <algorithm>

namespace item {

Item::Item(automa::ServiceProvider& svc, std::string_view label) : label(label) {
	auto const& in_data = svc.data.item[label];
	metadata.id = in_data["index"].as<int>();
	metadata.title = in_data["title"].as_string();
	metadata.naive_description = in_data["naive_description"].as_string();
	metadata.hidden_description = in_data["hidden_description"].as_string();
	if (in_data["unique"].as_bool()) { flags.set(ItemFlags::unique); }
	dimensions = {32.f, 32.f};
	sprite.setTexture(svc.assets.t_items);
	int u = (metadata.id - 1) * dimensions.x;
	int v = 0;
	sprite.setTextureRect(sf::IntRect({u, v}, static_cast<sf::Vector2<int>>(dimensions)));

	//for debug
	drawbox.setSize(dimensions);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setOutlineThickness(-1);
}

void Item::update(automa::ServiceProvider& svc, int index) {
	if (flags.test(ItemFlags::unique)) {
		variables.quantity = std::clamp(variables.quantity, 0, 1);
		sprite.setPosition({index * ui.spacing + ui.pad.x, ui.pad.y});
		drawbox.setPosition(sprite.getPosition());
	}
	selection_index = index;
	selected() ? drawbox.setOutlineColor(svc.styles.colors.green) : drawbox.setOutlineColor(svc.styles.colors.blue);
}

void Item::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { svc.greyblock_mode() ? win.draw(drawbox) : win.draw(sprite); }

void Item::add_item(int amount) { variables.quantity += amount; }

void item::Item::set_id(int new_id) { metadata.id = new_id; }

void Item::select() { ui_flags.set(UIFlags::selected); }

void Item::deselect() { ui_flags.reset(UIFlags::selected); }

} // namespace player
