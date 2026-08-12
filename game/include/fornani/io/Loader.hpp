
#pragma once

#include <fornani/io/Logger.hpp>
#include <functional>
#include <string>
#include <vector>

namespace fornani::io {

struct Task {
	std::function<void()> func{};
	std::string label{};
};

class Loader {
  public:
	void add(std::function<void()> task, std::string label);

	bool finished() const { return m_index == m_tasks.size(); }

	[[nodiscard]] auto progress() const -> float { return static_cast<float>(m_index) / m_tasks.size(); }

	void update();

  private:
	std::vector<Task> m_tasks;
	std::size_t m_index{};

	io::Logger m_logger{"Loader"};
};

} // namespace fornani::io
