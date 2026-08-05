
#include <fornani/io/Loader.hpp>

namespace fornani::io {

void Loader::add(std::function<void()> task) { m_tasks.push_back(std::move(task)); }

void Loader::update() {
	if (finished()) { return; }
	m_tasks[m_index++]();
	if (finished()) { NANI_LOG_INFO(m_logger, "Loader finished."); }
}

} // namespace fornani::io
