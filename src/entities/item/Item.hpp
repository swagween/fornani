
#pragma once
#include <string_view>
#include "../../utils/BitFlags.hpp"
#include "../Entity.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace item {

enum class ItemFlags { unique, revealed };
enum class UIFlags { selected };

class Item : public entity::Entity {
  public:
	Item() = default;
	Item(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc, int index);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void add_item(int amount);
	void set_id(int new_id);
	void select();
	void deselect();
	[[nodiscard]] auto selected() const -> bool { return ui_flags.test(UIFlags::selected); }
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_quantity() const -> int { return variables.quantity; }
	[[nodiscard]] auto get_label() const -> std::string_view { return metadata.title; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return drawbox.getPosition(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return flags.test(ItemFlags::revealed) ? metadata.hidden_description : metadata.naive_description; }

	std::string_view label{};
	int selection_index{};

  private:
	struct {
		int id;
		std::string_view title{};
		std::string_view naive_description{};
		std::string_view hidden_description{};
	} metadata{};

	util::BitFlags<ItemFlags> flags{};
	util::BitFlags<UIFlags> ui_flags{};

	struct {
		int quantity{};
	} variables{};

	struct {
		sf::Vector2<float> pad{120.f, 120.f};
		float spacing{56.f};
	} ui{};
};

} // namespace player
