
#include <fornani/io/Loader.hpp>

namespace fornani::io {

void Loader::add(std::function<void()> task) {
	m_tasks.push_back(std::move(task));
	NANI_LOG_INFO(m_logger, "add: index={} tasks={}", m_index, m_tasks.size());
	NANI_LOG_INFO(m_logger, "Loader={}", static_cast<void*>(this));
}

void Loader::update() {
	if (finished()) { return; }

	m_tasks[m_index++]();
	NANI_LOG_INFO(m_logger, "update: index={} tasks={}", m_index, m_tasks.size());

	if (finished()) { NANI_LOG_INFO(m_logger, "Loader finished."); }
}

} // namespace fornani::io
