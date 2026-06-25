
#pragma once

#include <vector>

namespace fornani {

template <typename T>
class RingBuffer {
  public:
	explicit RingBuffer(std::size_t capacity) : m_history(capacity) {}

	void push(T const& value) {
		if (m_capacity() == 0) return;

		if (m_filled) { m_sum -= m_history[m_index]; }

		m_history[m_index] = value;
		m_sum += value;

		m_index = (m_index + 1) % m_capacity();

		if (!m_filled && m_index == 0) { m_filled = true; }
	}

	[[nodiscard]]
	T average() const {
		std::size_t count = size();

		if (count == 0) { return T{}; }

		return m_sum / static_cast<float>(count);
	}

	[[nodiscard]]
	std::size_t size() const {
		return m_filled ? m_capacity() : m_index;
	}

	void clear() {
		std::fill(m_history.begin(), m_history.end(), T{});
		m_sum = {};
		m_index = 0;
		m_filled = false;
	}

  private:
	[[nodiscard]]
	std::size_t m_capacity() const {
		return m_history.size();
	}

	std::vector<T> m_history;
	T m_sum{};

	std::size_t m_index = 0;
	bool m_filled = false;
};

} // namespace fornani
