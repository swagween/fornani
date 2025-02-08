
#pragma once

#include "Layer.hpp"

#include <string>
#include <vector>

namespace pi {

class Map {
  public:
	std::vector<Layer> layers{};
	void set_labels();
	void set_middleground(int to_middleground);
	void delete_layer_at(std::size_t const index);
	void add_layer(int at, int direction);
	void reorder();
	[[nodiscard]] auto get_middleground() const -> int { return m_middleground; }
	[[nodiscard]] auto get_layer_name(int index) const -> char const* { return layers.at(index).label.c_str(); }
	struct {
		bool has_obscuring_layer{};
		bool has_reverse_obscuring_layer{};
	} m_flags{};

  private:
	int m_middleground{};
};

} // namespace
