
#include "editor/canvas/Clipboard.hpp"
#include "editor/canvas/Canvas.hpp"
#include "editor/tool/Tool.hpp"

namespace pi {

Clipboard::Clipboard(sf::Vector2<std::uint32_t> dimensions) : m_dimensions(dimensions) {}

void Clipboard::write_to_clipboard(std::uint32_t value, size_t i, size_t j, size_t layer) {
	while (layer >= m_cell_values.size()) { m_cell_values.push_back(std::vector<std::uint32_t>{}); }
	for (auto u{0}; u < m_dimensions.x; ++u) {
		for (auto v{0}; v < m_dimensions.y; ++v) { m_cell_values.at(layer).push_back(0); }
	}
	auto index = i + j * static_cast<size_t>(m_dimensions.x);
	m_cell_values.at(layer).at(index) = value;
}


void Clipboard::clear_clipboard() { m_cell_values.clear(); }

std::uint32_t Clipboard::get_value_at(size_t i, size_t j) { return m_cell_values.back().at(i + j * m_dimensions.x); }

std::uint32_t Clipboard::get_value_at(size_t i, size_t j, size_t layer) {
	if (layer >= m_cell_values.size()) { return 0u; }
	return m_cell_values.at(layer).at(i + j * m_dimensions.x);
}

void Clipboard::cut(Canvas& canvas, Tool& tool) {
	if (!tool.selection) { return; }
	m_mode = SelectMode::clipboard;
	source = canvas.get_selection_type();
	m_dimensions = tool.selection.value().dimensions;
	canvas.save_state(tool, true);
	for (std::uint32_t i = 0; i < tool.selection.value().dimensions.x; ++i) {
		for (std::uint32_t j = 0; j < tool.selection.value().dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!tool.pervasive && k != canvas.active_layer) { continue; }
				write_to_clipboard(canvas.tile_val_at(tool.selection.value().position.x + i, tool.selection.value().position.y + j, k), i, j, k);
				if (canvas.editable()) { canvas.erase_at(tool.selection.value().position.x + i, tool.selection.value().position.y + j, k); }
			}
		}
	}
}

void Clipboard::copy(Canvas& canvas, Tool& tool) {
	if (!tool.selection) { return; }
	m_mode = SelectMode::clipboard;
	source = canvas.get_selection_type();
	m_dimensions = tool.selection.value().dimensions;
	for (std::uint32_t i = 0; i < tool.selection.value().dimensions.x; ++i) {
		for (std::uint32_t j = 0; j < tool.selection.value().dimensions.y; ++j) {
			for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) {
				if (!tool.pervasive && k != canvas.active_layer) { continue; }
				write_to_clipboard(canvas.tile_val_at(tool.selection.value().position.x + i, tool.selection.value().position.y + j, k), i, j, k);
			}
		}
	}
}

void Clipboard::paste(Canvas& canvas, Tool& tool) {
	if (!canvas.editable()) { return; }
	canvas.save_state(tool, true);
	for (std::uint32_t i = 0; i < scaled_dimensions().x; ++i) {
		for (std::uint32_t j = 0; j < scaled_dimensions().y; ++j) {
			auto edit_x = tool.scaled_position().x + i - scaled_dimensions().x + 1;
			auto edit_y = tool.scaled_position().y + j - scaled_dimensions().y + 1;
			if (edit_x < canvas.dimensions.x && edit_y < canvas.dimensions.y) {
				if (tool.pervasive && source != SelectionType::palette) {
					for (auto k{0}; k < canvas.get_layers().layers.size(); ++k) { canvas.edit_tile_at(edit_x, edit_y, get_value_at(i, j, k), k); }
				} else {
					canvas.edit_tile_at(edit_x, edit_y, get_value_at(i, j), canvas.active_layer);
				}
			}
		}
	}
}

void Clipboard::render(Canvas& canvas, Tool& tool, sf::RenderWindow& win, sf::Vector2<float> offset) {
	m_box.setOutlineColor(sf::Color{200, 200, 200, 40});
	m_box.setFillColor(sf::Color{150, 190, 110, 40});
	m_box.setOutlineThickness(-2);
	m_box.setSize(real_dimensions() * canvas.f_cell_size());
	m_box.setPosition(tool.f_scaled_position() * canvas.f_cell_size() + offset - real_dimensions() * canvas.f_cell_size() + sf::Vector2<float>{canvas.f_cell_size(), canvas.f_cell_size()});
	if (canvas.editable() && !tool.palette_mode) { win.draw(m_box); }
}

}
