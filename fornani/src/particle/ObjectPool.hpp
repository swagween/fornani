
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace gen {

template <typename Type>
struct PoolFactory {
	Type operator()() const { return Type{}; }
};

template <typename Type, typename Factory = PoolFactory<Type>>
class ObjectPool {
  public:
	void push(Type&& t) { m_pool.push_back(std::move(t)); }

	void pop() {
		if (m_pool.empty()) {
			return;
		} else {
			m_pool.pop_back();
		}
	}

  private:
	std::vector<Type> m_pool{};
	Factory m_factory{};
};

}
