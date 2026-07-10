
#include <fornani/utils/Direction.hpp>

namespace fornani {

SimpleDirection::SimpleDirection(Direction const to) : lr{static_cast<LR>(to.lnr)} {};

Direction::Direction(CardinalDirection dir) {
	switch (dir.get()) {
	case UDLR::up: und = UND::up; break;
	case UDLR::down: und = UND::down; break;
	case UDLR::left: lnr = LNR::left; break;
	case UDLR::right: lnr = LNR::right; break;
	}
}

} // namespace fornani
