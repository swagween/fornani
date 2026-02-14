
#include <fornani/utils/Direction.hpp>

namespace fornani {

SimpleDirection::SimpleDirection(Direction const to) : lr{static_cast<LR>(to.lnr)} {};

}
