#include "Transponder.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace player {

void Transponder::update() {

	// if (requested_next()) { svc::consoleLocator.get().writer.next(); } // TODO
	if (exited()) {
		if (svc::consoleLocator.get().writer.complete()) {
			svc::soundboardLocator.get().console.set(audio::Console::done);
			svc::consoleLocator.get().end();
		}
	}
	// execute action based on the state of the console
	if (skipped_ahead()) {
		if (svc::consoleLocator.get().writer.active() && !svc::consoleLocator.get().writer.complete()) { svc::consoleLocator.get().writer.skip_ahead(); }
	}

}

void Transponder::end() { actions = {}; }

void Transponder::skip_ahead() { actions.set(TransponderActions::skip_ahead); }

void Transponder::next() { actions.set(TransponderActions::next); }

void Transponder::exit() { actions.set(TransponderActions::exit); }

bool Transponder::skipped_ahead() const { return actions.test(TransponderActions::skip_ahead); }

bool Transponder::requested_next() const { return actions.test(TransponderActions::next); }

bool Transponder::exited() const { return actions.test(TransponderActions::exit); }

} // namespace player
