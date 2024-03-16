#include "Frdog.hpp"
#include "../../../service/ServiceProvider.hpp"

namespace enemy {

Frdog::Frdog(automa::ServiceProvider& svc) : Enemy(svc, "frdog") {}

void Frdog::unique_update(automa::ServiceProvider& svc, world::Map& map) {
	update(svc, map);
	state_function = state_function();
}

fsm::StateFunction Frdog::update_idle() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_sleep() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_sit() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_turn() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_charge() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_run() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_hurt() { return BIND(update_idle); };
fsm::StateFunction Frdog::update_bark() { return BIND(update_idle); };

} // namespace enemy