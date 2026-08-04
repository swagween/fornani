
#pragma once

#include <fornani/io/Logger.hpp>
#include <functional>
#include <vector>

namespace fornani::io {

class Loader {
  public:
	void add(std::function<void()> task);

	bool finished() const { return m_index == m_tasks.size(); }

	[[nodiscard]] auto progress() const -> float { return static_cast<float>(m_index) / m_tasks.size(); }

	void update();

  private:
	std::vector<std::function<void()>> m_tasks;
	std::size_t m_index{};

	io::Logger m_logger{"Loader"};
};

} // namespace fornani::io
