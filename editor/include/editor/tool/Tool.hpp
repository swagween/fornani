
#pragma once

#include <stdio.h>
#include <optional>
#include "editor/canvas/Canvas.hpp"
#include "editor/canvas/Clipboard.hpp"
#include "editor/util/BitFlags.hpp"

namespace pi {

enum class ToolType { brush, fill, marquee, erase, hand, entity_editor, eyedropper };
enum class EntityType { none, portal, inspectable, critter, chest, animator, player_placer, platform, save_point, switch_button, switch_block, interactive_scenery, scenery };
enum class EntityMode { selector, placer, eraser, mover };
enum class ToolStatus { usable, unusable, destructive };
enum class SelectMode { none, select, clipboard };

class Tool {
  public:
	Tool& operator=(Tool const&) = delete;
	virtual void handle_events(Canvas& canvas, sf::Event& e) = 0;
	virtual void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key) = 0;
	virtual void update() = 0;
	virtual void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true) = 0;
	virtual void store_tile(int index) = 0;
	virtual void clear() = 0;
	virtual void set_usability(bool const flag);

	void set_position(sf::Vector2<float> to_position); 
	void set_window_position(sf::Vector2<float> to_position);
	void reset();
	void activate();
	void deactivate();
	void change_size(int amount);

	[[nodiscard]] auto get_tooltip() const -> std::string { return tooltip; }
	[[nodiscard]] auto f_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto scaled_position() const -> sf::Vector2<uint32_t> { return {static_cast<uint32_t>(std::floor(position.x / 32.f)), static_cast<uint32_t>(std::floor(position.y / 32.f))}; }
	[[nodiscard]] auto f_scaled_position() const -> sf::Vector2<float> { return {static_cast<float>(scaled_position().x), static_cast<float>(scaled_position().y)}; }
	[[nodiscard]] auto scaled_clicked_position() const -> sf::Vector2<uint32_t> { return {static_cast<uint32_t>(std::floor(clicked_position.x / 32.f)), static_cast<uint32_t>(std::floor(clicked_position.y / 32.f))}; }
	[[nodiscard]] auto get_window_position() const -> sf::Vector2<float> { return window_position; }
	[[nodiscard]] auto get_window_position_scaled() const -> sf::Vector2<float> { return window_position / 32.f; }
	[[nodiscard]] auto palette_interactable() const -> bool { return type == ToolType::marquee || type == ToolType::eyedropper; }
	[[nodiscard]] auto is_active() const -> bool { return active; }
	[[nodiscard]] auto clipboard() const -> bool { return mode == SelectMode::clipboard; }
	[[nodiscard]] auto is_usable() const -> bool { return status == ToolStatus::usable; }

	bool in_bounds(sf::Vector2<uint32_t>& bounds) const;
	bool is_paintable() const;

	bool pervasive{};
	bool contiguous{};
	bool ready{};
	bool just_clicked = true;
	bool has_palette_selection{};

	int xorigin{};
	int yorigin{};
	int tempx{};
	int tempy{};

	int size{1};

	bool primary{};
	bool trigger_switch{false};
	uint8_t tile{};
	std::optional<std::unique_ptr<Entity>> current_entity{};

	ToolType type{};
	EntityType ent_type{};
	ToolStatus status{};

  protected:
	bool active{};
	SelectMode mode{};
	float scale{};
	int max_size{32};
	sf::Vector2<float> position{};
	sf::Vector2<float> clicked_position{};
	sf::Vector2<float> relative_position{};
	sf::Vector2<float> window_position{};
	std::string tooltip{};
};

class Hand : public Tool {
  public:
	Hand() { type = ToolType::hand; }
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();
};

class Brush : public Tool {
  public:
	Brush() { type = ToolType::brush; }
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();

  private:
};

class Erase : public Tool {
  public:
	Erase() { type = ToolType::erase; }
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();

  private:
};

class Fill : public Tool {
  public:
	Fill() { type = ToolType::fill; };
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();

	void fill_section(uint8_t const prev_val, uint8_t const new_val, uint32_t i, uint32_t j, Canvas& canvas);
	void replace_all(uint8_t const prev_val, uint8_t const new_val, uint32_t i, uint32_t j, Canvas& canvas);
};

class EntityEditor : public Tool {
  public:
	EntityEditor(EntityMode to_mode = EntityMode::selector);
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();
	void set_mode(EntityMode to_mode);
	void set_usability(bool const flag) override;
	[[nodiscard]] auto selector_mode() const -> bool { return entity_mode == EntityMode::selector; }
	[[nodiscard]] auto placer_mode() const -> bool { return entity_mode == EntityMode::placer; }
	[[nodiscard]] auto eraser_mode() const -> bool { return entity_mode == EntityMode::eraser; }
	[[nodiscard]] auto mover_mode() const -> bool { return entity_mode == EntityMode::mover; }

  private:
	EntityMode entity_mode{};
};

struct SelectBox {
	SelectBox() = default;
	SelectBox(sf::Vector2<uint32_t> pos, sf::Vector2<uint32_t> dim) : position(pos), dimensions(dim) {}
	void clear() {
		position = {0, 0};
		dimensions = {0, 0};
	}
	void adjust(sf::Vector2<uint32_t> adjustment) { dimensions = adjustment; }
	[[nodiscard]] auto f_position() const -> sf::Vector2<float> { return {static_cast<float>(position.x), static_cast<float>(position.y)}; }
	[[nodiscard]] auto empty() const -> bool { return dimensions.x * dimensions.y == 0; }
	sf::Vector2<uint32_t> position{};
	sf::Vector2<uint32_t> dimensions{};
};


class Marquee : public Tool {
  public:
	Marquee() { type = ToolType::marquee; }
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void cut(Canvas& canvas);
	void copy(Canvas& canvas);
	void paste(Canvas& canvas);
	void clear();

  private:
	SelectBox selection{};
	std::optional<Clipboard> clipboard{};
};

class Eyedropper : public Tool {
  public:
	Eyedropper() { type = ToolType::eyedropper; }
	void handle_events(Canvas& canvas, sf::Event& e);
	void handle_keyboard_events(Canvas& canvas, sf::Keyboard::Key& key);
	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float> offset, bool transformed = true);
	void store_tile(int index);
	void clear();
};

} // namespace pi
