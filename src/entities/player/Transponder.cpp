#include "Transponder.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace player {

void Transponder::update() {

	// execute action based on the state of the console
	// all of these functions will be called, but will only be executed
	// if the TextWriter is in the required state.

	// selection mode stuff
	if (up()) { svc::consoleLocator.get().writer.adjust_selection(-1); }
	if (down()) { svc::consoleLocator.get().writer.adjust_selection(1); }
	if (selected()) { svc::consoleLocator.get().writer.process_selection(); }

	// text stuff
	if (skipped_ahead()) {
		if (svc::consoleLocator.get().writer.writing() && svc::consoleLocator.get().writer.can_skip()) { svc::consoleLocator.get().writer.skip_ahead(); }
	}
	if (requested_next()) {
		if (!svc::consoleLocator.get().writer.writing()) { svc::soundboardLocator.get().console.set(audio::Console::next); }
		svc::consoleLocator.get().writer.request_next();
	}
	if (exited()) {
		if (svc::consoleLocator.get().writer.complete()) {
			svc::soundboardLocator.get().console.set(audio::Console::done);
			svc::consoleLocator.get().writer.shutdown();
			svc::consoleLocator.get().end();
		}
	}
	if (skip_released()) { svc::consoleLocator.get().writer.enable_skip(); }
	if (svc::consoleLocator.get().writer.writing()) { svc::soundboardLocator.get().console.set(audio::Console::speech); }

	end();
}

void Transponder::end() { actions = {}; }

void Transponder::skip_ahead() { actions.set(TransponderActions::skip_ahead); }

void Transponder::enable_skip() { actions.set(TransponderActions::skip_released); }

void Transponder::next() { actions.set(TransponderActions::next); }

void Transponder::exit() { actions.set(TransponderActions::exit); }

void Transponder::go_up() { actions.set(TransponderActions::up); }

void Transponder::go_down() { actions.set(TransponderActions::down); }

void Transponder::select() { actions.set(TransponderActions::select); }

bool Transponder::skipped_ahead() const { return actions.test(TransponderActions::skip_ahead); }

bool Transponder::skip_released() const { return actions.test(TransponderActions::skip_released); }

bool Transponder::requested_next() const { return actions.test(TransponderActions::next); }

bool Transponder::exited() const { return actions.test(TransponderActions::exit); }

bool Transponder::down() const { return actions.test(TransponderActions::down); }

bool Transponder::up() const { return actions.test(TransponderActions::up); }

bool Transponder::selected() const { return actions.test(TransponderActions::select); }

} // namespace player
