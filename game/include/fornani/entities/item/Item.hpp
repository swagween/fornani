
#pragma once
#include <optional>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/entities/Entity.hpp"
#include "Drop.hpp"
#include "fornani/particle/Gravitator.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {
enum class ApparelType;
}

namespace fornani::item {

enum class ItemFlags : uint8_t { unique, revealed, usable, equippable, sellable };
enum class UIFlags : uint8_t { selected };
enum class ItemState : uint8_t { equipped };

class Item final : public entity::Entity {
  public:
	Item(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc, int index, int items_per_row, sf::Vector2<float> offset);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void add_item(int amount);
	void subtract_item(int amount);
	void set_id(int new_id);
	void select();
	void deselect();
	void toggle_equip();
	void reveal() { flags.set(ItemFlags::revealed); }
	void set_rarity_position(sf::Vector2<float> position);
	void set_offset(sf::Vector2<float> offset);
	[[nodiscard]] auto selected() const -> bool { return ui_flags.test(UIFlags::selected); }
	[[nodiscard]] auto usable() const -> bool { return flags.test(ItemFlags::usable); }
	[[nodiscard]] auto equippable() const -> bool { return flags.test(ItemFlags::equippable); }
	[[nodiscard]] auto sellable() const -> bool { return flags.test(ItemFlags::sellable); }
	[[nodiscard]] auto is_equipped() const -> bool { return state.test(ItemState::equipped); }
	[[nodiscard]] auto has_menu() const -> bool { return equippable() || usable(); }
	[[nodiscard]] auto depleted() const -> bool { return variables.quantity <= 0; }
	[[nodiscard]] auto get_id() const -> int { return metadata.id; }
	[[nodiscard]] auto get_quantity() const -> int { return variables.quantity; }
	[[nodiscard]] auto get_value() const -> int { return metadata.value; }
	[[nodiscard]] auto get_label() const -> std::string_view { return flags.test(ItemFlags::revealed) ? metadata.title : metadata.naive_title; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator.position(); }
	[[nodiscard]] auto get_description() const -> std::string_view { return flags.test(ItemFlags::revealed) ? metadata.hidden_description : metadata.naive_description; }
	[[nodiscard]] auto get_apparel_type() const -> player::ApparelType { return metadata.apparel_type ? metadata.apparel_type.value() : static_cast<player::ApparelType>(0); }

	std::string_view label{};
	int selection_index{};
	vfx::Gravitator gravitator{};

  private:
	struct {
		int id{};
		std::string_view title{};
		std::string_view naive_title{};
		std::string_view naive_description{};
		std::string_view hidden_description{};
		Rarity rarity{};
		std::optional<player::ApparelType> apparel_type{};
		int value{};
	} metadata{};

	sf::Sprite sprite;

	util::BitFlags<ItemFlags> flags{};
	util::BitFlags<ItemState> state{};
	util::BitFlags<UIFlags> ui_flags{};

	struct {
		int quantity{};
	} variables{};

	struct {
		sf::Text rarity;
		sf::Text quantity;
		sf::Vector2<float> pad{60.f, 60.f};
		sf::Vector2<float> offset{};
		float spacing{56.f};
	} ui;
};

} // namespace fornani::player
