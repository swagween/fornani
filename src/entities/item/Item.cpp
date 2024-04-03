#include "Item.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"
#include <algorithm>

namespace item {

Item::Item(automa::ServiceProvider& svc, std::string_view label) : label(label) {
	auto const& in_data = svc.data.item[label];
	metadata.title = in_data["title"].as_string();
	metadata.naive_description = in_data["naive_description"].as_string();
	metadata.hidden_description = in_data["hidden_description"].as_string();
	if (in_data["unique"].as_bool()) { flags.set(ItemFlags::unique); }
}

void Item::update(automa::ServiceProvider& svc) {
	if (flags.test(ItemFlags::unique)) { variables.quantity = std::clamp(variables.quantity, 0, 1); }
}

void Item::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {}

void Item::add_item(int amount) { variables.quantity += amount; }
void item::Item::set_id(int new_id) { metadata.id = new_id; }
} // namespace player
