
#pragma once

#include <atomic>
#include <cassert>
#include <utility>
#include <vector>

namespace forloop {

template <typename T>
using Ptr = T*;

template <typename Type>
class Service {
  public:
	class Instance; // forward declare Instance

	static void provide(Ptr<Type> t) { s_t = t; } // give the user access to the service class
	static void unprovide() { s_t = nullptr; }	  // remove the user's access to the service class

	static Ptr<Type> find() { return s_t; }
	static bool contains() { return find() != nullptr; }
	static Type& get() { return (assert(contains()), *find()); }
	static Type& locate() { return get(); }

  private:
	inline static Ptr<Type> s_t{}; // the pointer to the service class
};

template <typename Type>
class Service<Type>::Instance {
  public:
	Instance& operator=(Instance&&) = delete;

	template <typename... Args>
		requires(std::is_constructible_v<Type, Args...>)
	Instance(Args&&... args) : m_t(std::forward<Args>(args)...) {
		provide(&m_t);
	}

	~Instance() { unprovide(); }

	Type& get() { return m_t; }
	Type const& get() const { return m_t; }

  private:
	Type m_t{};
};

} // namespace forloop