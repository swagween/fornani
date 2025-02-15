#include "fornani/entities/player/Transponder.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

void Transponder::update(automa::ServiceProvider& svc, gui::Console& console) {

	// execute action based on the state of the console
	// all of these functions will be called, but will only be executed
	// if the TextWriter is in the required state.

	// shipment tracking
	track_shipments(console);

	// selection mode stuff
	if (up()) { console.writer->adjust_selection(-1); }
	if (down()) { console.writer->adjust_selection(1); }
	if (selected()) { console.writer->process_selection(); }

	// text stuff
	if (skipped_ahead()) {
		if (console.writer->writing() && console.writer->can_skip()) { console.writer->skip_ahead(); }
	}
	if (requested_next() && !console.writer->delaying()) {
		if (!console.writer->writing()) { svc.soundboard.flags.console.set(audio::Console::next); }
		console.writer->request_next();
		console.writer->reset_delay();
	}
	if (skip_released()) { console.writer->enable_skip(); }
	if (console.writer->writing()) { svc.soundboard.flags.console.set(audio::Console::speech); }

	if (exited()) {
		if (console.writer->complete()) {
			svc.soundboard.flags.console.set(audio::Console::done);
			flush_shipments();
			console.writer->shutdown();
			console.end();
		}
	}
	end();
}

void Transponder::track_shipments(gui::Console& console) {
	shipments.item.set(console.writer->communicators.out_item.get_residue());
	shipments.quest.set(console.writer->communicators.out_quest.get_residue());
	shipments.voice.set(console.writer->communicators.out_voice.get_residue());
	shipments.emotion.set(console.writer->communicators.out_emotion.get_residue());
	shipments.reveal_item.set(console.writer->communicators.reveal_item.get_residue());
	out_quest = console.writer->out_quest;
}

void Transponder::flush_shipments() { shipments = {}; }

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

} // namespace fornani::player
