
#pragma once

#include <ksignal/ksignal.hpp>

namespace fornani {

template <typename... Args>
using Event = ksignal::Signal<Args...>;

} // namespace fornani
