
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

	enum class ItemFlags { unique };

class Item : public entity::Entity {
  public:
	Item() = default;
	Item(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void add_item(int amount);
	void set_id(int new_id);
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_quantity() const -> int { return variables.quantity; }

	std::string_view label{};

  private:
	struct {
		int id;
		std::string_view title{};
		std::string_view naive_description{};
		std::string_view hidden_description{};
	} metadata{};

	util::BitFlags<ItemFlags> flags{};

	struct {
		int quantity{};
	} variables{};
};

} // namespace player
