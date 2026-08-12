
#include <fornani/io/Loader.hpp>

namespace fornani::io {

void Loader::add(std::function<void()> task, std::string label) {
	m_tasks.push_back({std::move(task), label});
	NANI_LOG_INFO(m_logger, "Added task to loader queue: {}.", label);
}

void Loader::update() {
	if (finished()) { return; }
	NANI_LOG_INFO(m_logger, "Running task: {}.", m_tasks[m_index].label);
	m_tasks[m_index].func();
	++m_index;
	if (finished()) { NANI_LOG_INFO(m_logger, "Loader finished."); }
}

} // namespace fornani::io
