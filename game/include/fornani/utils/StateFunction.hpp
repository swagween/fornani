
#pragma once

#include <functional>
#include <utility>

namespace fornani::fsm {

/** A StateFunction is a function which does some computation and returns
 *  another StateFunction (possibly the same).
 *  Users of this class should leverage the move constructor whenever possible
 *  to avoid useless copies. If a copy-construction is needed, pass stateFunction.f,
 *  which will use the base constructor.
 */
struct StateFunction {
	template <typename T>
	StateFunction(T const& f) : f(f) {}
	StateFunction(StateFunction&& s) = default;
	StateFunction(StateFunction const& s) = delete;
	StateFunction operator()() { return f(); }
	StateFunction& operator=(StateFunction&& s) {
		f = std::move(s.f);
		return *this;
	}
	StateFunction& operator=(StateFunction const&) = delete;
	operator bool() const { return bool(f); }
	std::function<StateFunction()> f;
};

} // end namespace fsm