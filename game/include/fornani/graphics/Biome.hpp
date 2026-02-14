
#pragma once

#include <string_view>

namespace fornani {

class Biome {
  public:
	Biome() : m_label{"firstwind"} {}
	Biome(std::string_view label, std::size_t id) : m_label{label}, m_id{id} {}

	[[nodiscard]] auto get_label() const -> std::string_view { return m_label; }
	[[nodiscard]] auto get_id() const -> int { return static_cast<int>(m_id); }

  private:
	std::string m_label{};
	std::size_t m_id{};
};

} // namespace fornani
