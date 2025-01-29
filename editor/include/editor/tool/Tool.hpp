
#pragma once

#include "editor/canvas/Canvas.hpp"
#include "editor/canvas/Clipboard.hpp"
#include "editor/util/BitFlags.hpp"

#include <stdio.h>
#include <optional>
#include <string_view>

namespace pi {

enum class ToolType { brush, fill, marquee, erase, hand, entity_editor, eyedropper };
enum class EntityType { none, portal, inspectable, critter, chest, animator, player_placer, platform, save_point, switch_button, switch_block, interactive_scenery, scenery };
enum class EntityMode { selector, placer, eraser, mover, editor };
enum class ToolStatus { usable, unusable, loaded };
enum class SelectMode { none, select, clipboard };

class Tool {
  public:
	Tool(std::string_view label, ToolType type) : label(label), type(type) {}
	Tool& operator=(Tool const&) = delete;
	~Tool() = default;
	virtual void update(Canvas& canvas);
	virtual void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode) = 0;
	virtual void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset) = 0;
	virtual void store_tile(int index) = 0;
	virtual void clear() = 0;
	virtual void set_usability(bool const flag);

	void set_position(sf::Vector2<float> to_position);
	void set_window_position(sf::Vector2<float> to_position);
	void activate();
	void deactivate();
	void suppress_until_released();
	void unsuppress();
	void neutralize();
	void click();
	void release();
	void change_size(int amount);

	[[nodiscard]] auto get_label() const -> std::string { return label; };
	[[nodiscard]] auto get_tooltip() const -> std::string { return tooltip; }
	[[nodiscard]] auto f_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<uint32_t> { return {static_cast<uint32_t>(std::floor(position.x / 32.f)), static_cast<uint32_t>(std::floor(position.y / 32.f))}; }
	[[nodiscard]] auto scaled_position_ceiling() const -> sf::Vector2<uint32_t> { return {static_cast<uint32_t>(std::ceil(position.x / 32.f)), static_cast<uint32_t>(std::ceil(position.y / 32.f))}; }
	[[nodiscard]] auto f_scaled_position() const -> sf::Vector2<float> { return {static_cast<float>(scaled_position().x), static_cast<float>(scaled_position().y)}; }
	[[nodiscard]] auto scaled_clicked_position() const -> sf::Vector2<uint32_t> { return {static_cast<uint32_t>(std::floor(clicked_position.x / 32.f)), static_cast<uint32_t>(std::floor(clicked_position.y / 32.f))}; }
	[[nodiscard]] auto get_window_position() const -> sf::Vector2<float> { return window_position; }
	[[nodiscard]] auto get_window_position_scaled() const -> sf::Vector2<float> { return window_position / 32.f; }
	[[nodiscard]] auto palette_interactable() const -> bool { return type == ToolType::marquee || type == ToolType::eyedropper; }
	[[nodiscard]] auto get_selection_type() const -> SelectionType { return selection_type; }
	[[nodiscard]] auto is_ready() const -> bool { return ready; }
	[[nodiscard]] auto is_active() const -> bool { return active; }
	[[nodiscard]] auto clicked() const -> bool { return just_clicked; }
	[[nodiscard]] auto clipboard() const -> bool { return mode == SelectMode::clipboard; }
	[[nodiscard]] auto is_usable() const -> bool { return status == ToolStatus::usable; }
	[[nodiscard]] auto highlight_canvas() const -> bool { return (is_paintable() || type == ToolType::erase) && !disable_highlight; }
	[[nodiscard]] auto is_paintable() const -> bool { return type == ToolType::brush || type == ToolType::fill; };

	bool in_bounds(sf::Vector2<uint32_t>& bounds) const;

	bool pervasive{};
	bool contiguous{};

	bool has_palette_selection{};
	bool palette_mode;
	bool entity_menu{};

	int xorigin{};
	int yorigin{};
	int tempx{};
	int tempy{};

	int size{1};

	bool primary{};
	bool trigger_switch{false};
	uint32_t tile{};
	std::optional<std::unique_ptr<Entity>> current_entity{};

	ToolType type{};
	EntityType ent_type{};
	EntityMode entity_mode{};
	ToolStatus status{};

  protected:
	std::string label{};
	bool active{};
	bool just_released{};
	bool just_clicked{true};
	bool disable_highlight{};
	SelectMode mode{};
	SelectionType selection_type{};
	float scale{};
	int max_size{32};
	sf::Vector2<float> position{};
	sf::Vector2<float> clicked_position{};
	sf::Vector2<float> relative_position{};
	sf::Vector2<float> window_position{};
	std::string tooltip{};

  private:
	bool ready{true};
};

class Hand : public Tool {
  public:
	Hand() : Tool("Hand", ToolType::hand) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();
};

class Brush : public Tool {
  public:
	Brush() : Tool("Brush", ToolType::brush) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();

  private:
};

class Erase : public Tool {
  public:
	Erase() : Tool("Eraser", ToolType::erase) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();

  private:
};

class Fill : public Tool {
  public:
	Fill() : Tool("Fill", ToolType::fill) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();

	void fill_section(uint32_t const prev_val, uint32_t const new_val, uint32_t i, uint32_t j, Canvas& canvas);
	void replace_all(uint32_t const prev_val, uint32_t const new_val, uint32_t i, uint32_t j, Canvas& canvas);
};

class EntityEditor : public Tool {
  public:
	EntityEditor(EntityMode to_mode = EntityMode::selector);
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();
	void set_mode(EntityMode to_mode);
	void set_usability(bool const flag) override;
	[[nodiscard]] auto selector_mode() const -> bool { return entity_mode == EntityMode::selector; }
	[[nodiscard]] auto placer_mode() const -> bool { return entity_mode == EntityMode::placer; }
	[[nodiscard]] auto eraser_mode() const -> bool { return entity_mode == EntityMode::eraser; }
	[[nodiscard]] auto mover_mode() const -> bool { return entity_mode == EntityMode::mover; }
	[[nodiscard]] auto editor_mode() const -> bool { return entity_mode == EntityMode::editor; }
};

class SelectBox {
  public:
	SelectBox(sf::Vector2<uint32_t> pos, sf::Vector2<uint32_t> dim, SelectionType type) : position(pos), dimensions(dim), type(type) {}
	void adjust(sf::Vector2<uint32_t> adjustment) { dimensions = adjustment; }
	[[nodiscard]] auto get_type() const -> SelectionType { return type; }
	[[nodiscard]] auto f_position() const -> sf::Vector2<float> { return {static_cast<float>(position.x), static_cast<float>(position.y)}; }
	[[nodiscard]] auto empty() const -> bool { return dimensions.x * dimensions.y == 0; }
	sf::Vector2<uint32_t> position{};
	sf::Vector2<uint32_t> dimensions{};

  private:
	SelectionType type{};
};

class Marquee : public Tool {
  public:
	Marquee() : Tool("Marquee", ToolType::marquee) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void cut(Canvas& canvas);
	void copy(Canvas& canvas);
	void paste(Canvas& canvas);
	void clear();

  private:
	std::optional<SelectBox> selection{};
	std::optional<Clipboard> clipboard{};
};

class Eyedropper : public Tool {
  public:
	Eyedropper() : Tool("Eyedropper", ToolType::eyedropper) {}
	void update(Canvas& canvas) override;
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Scancode scancode);
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset);
	void store_tile(int index);
	void clear();
};

} // namespace pi
