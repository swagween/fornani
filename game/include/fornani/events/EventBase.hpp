
#pragma once

#include <fornani/utils/ID.hpp>
#include <ksignal/ksignal.hpp>

namespace fornani {

template <typename... Args>
using Event = ksignal::Signal<Args...>;

} // namespace fornani
