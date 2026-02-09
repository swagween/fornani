
#include <fornani/events/ConsoleEvent.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/events/InventoryEvent.hpp>
#include <fornani/events/SystemEvent.hpp>
#include <fornani/gui/console/Console.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/systems/InputSystem.hpp"

namespace fornani::gui {

Console::Console(automa::ServiceProvider& svc)
	: m_services(&svc), m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "standard", 64}, m_styling{.corner_factor{28}, .edge_factor{1}, .padding_scale{1.1f}},
	  m_nineslice(svc, svc.assets.get_texture("blue_console"), {m_styling.corner_factor, m_styling.corner_factor}, {m_styling.edge_factor, m_styling.edge_factor}), m_mode{ConsoleMode::writing}, m_response_offset{-192.f, 16.f},
	  m_exit_stall{650}, m_item_display_timer{1200}, m_launch{8} {
	NANI_LOG_DEBUG(m_logger, "Console ctor @{}", static_cast<void const*>(this));
	svc.input_system.set_action_set(input::ActionSet::Menu);
	text_suite = svc.text.console;
	m_path.set_section("open");
	m_began = true;
	m_path.update();
	m_position = m_path.get_position();
	m_launch.start();
}

Console::Console(automa::ServiceProvider& svc, std::string_view message) : Console(svc) { load_single_message(message); }

Console::Console(automa::ServiceProvider& svc, dj::Json const& source, OutputType type) : Console(svc) {
	if (type == OutputType::no_skip) { m_exit_stall.start(); }
	set_source(source);
	load_and_launch(type);
}

Console::Console(automa::ServiceProvider& svc, dj::Json const& source, std::string_view key, OutputType type, int target_index) : Console(svc) {
	if (type == OutputType::no_skip) { m_exit_stall.start(); }
	set_source(source);
	load_and_launch(key, type, target_index);
}

void Console::relaunch(automa::ServiceProvider& svc, dj::Json const& source, std::string_view key, OutputType type, int target_index) {
	m_mode = ConsoleMode::writing;
	NANI_LOG_DEBUG(m_logger, "Relaunched console.");
	if (type == OutputType::no_skip) { m_exit_stall.start(); }
	set_source(source);
	load_and_launch(key, type, target_index);
}

void Console::update(automa::ServiceProvider& svc) {
	m_exit_stall.update();
	m_launch.update();
	m_item_display_timer.update();
	if (m_item_display_timer.is_almost_complete()) { m_item_widget->send_out(); }
	if (!is_active()) { return; }
	if (!m_writer) { return; }
	m_began = false;
	handle_inputs(svc.input_system);
	if (is_active()) { m_path.update(); }
	if (m_path.finished() && m_writer) {
		if (m_writer->is_stalling()) { m_writer->start(); }
	}
	if (!m_writer) { return; }
	if (m_writer->is_writing()) { svc.soundboard.repeat_sound("console_speech", 0, {}, m_writer->is_sped_up() ? 1.6f : 1.f); }
	m_position = m_path.get_position();
	m_dimensions = m_path.get_dimensions();
	m_nineslice.set_offset(-m_path.get_local_center());
	m_nineslice.set_position(m_path.get_position());
	m_nineslice.set_dimensions(m_path.get_dimensions());
	auto vert_factor{1.5f};
	auto to_dim{m_dimensions.componentWiseMul({m_styling.padding_scale, m_styling.padding_scale * vert_factor})};
	m_writer->set_bounds(sf::FloatRect{m_position - to_dim * 0.5f, to_dim});
	m_writer->update();
	if (m_response) {
		m_response->set_position(m_position + m_response_offset);
		m_response->update();
	}
	if (m_item_widget) { m_item_widget->update(svc); }

	// check for response or message code
	bool processed{};
	if (auto codes = get_message_codes()) {
		for (auto& code : codes.value()) {
			if (code.is_response() && m_writer->is_available()) { m_writer->respond(); }
			if (code.is_reveal_item() && m_process_code_before) {
				m_services->events.reveal_item_by_id_event.dispatch(code.value);
				processed = true;
			}
			if (code.is_input_hint()) {
				auto action_id = code.extras ? code.extras->at(0) : 0;
				auto lookup = m_services->input_system.get_icon_lookup_by_action(static_cast<input::DigitalAction>(action_id));
				m_writer->insert_icon_at(code.value, lookup);
			}
			if (code.is(MessageCodeType::launch_cutscene) && m_process_code_before) {
				m_services->events.launch_cutscene_event.dispatch(*m_services, code.value);
				processed = true;
			}
			if (code.is(MessageCodeType::add_map_marker) && m_process_code_before && code.extras) {
				if (code.extras->size() > 1) {
					m_services->events.add_map_marker_event.dispatch(*m_services, code.value, code.extras->at(0), code.extras->at(1));
					processed = true;
				}
			}
			if (code.is_quest() && m_process_code_before) {
				if (code.extras) {
					if (code.extras->size() > 1) { svc.quest_table.progress_quest(svc.quest_registry.get_quest_metadata(code.value).get_tag(), code.extras->at(0), code.extras->at(1)); }
				}
				processed = true;
			}
			if (code.is_pop_conversation() && m_process_code_before) {
				m_services->events.npc_pop_conversation_event.dispatch();
				auto label = svc.data.get_npc_label_from_id(code.value);
				if (label && code.extras) {
					svc.quest_table.progress_quest("npc_dialogue", {label.value().data(), code.extras->at(0)}, 1, -1, code.value);
					NANI_LOG_DEBUG(m_logger, "Progressed NPC dialogue for NPC {} in room {}.", label.value().data(), code.extras->at(0));
				}
				processed = true;
			}
			if (code.is_play_song() && m_process_code_before) {
				m_services->events.play_song_event.dispatch(code.value);
				processed = true;
			}
			if (code.is_voice_cue() && m_process_code_before) {
				// m_services->events.get_or_add<NPCVoiceCueEvent>().dispatch(*m_services, code.value);
				m_services->events.npc_voice_cue_event.dispatch(*m_services, code.value);
				NANI_LOG_DEBUG(m_logger, "Voice Cue: {}", code.value);
				processed = true;
			}
			if (code.is_emotion() && m_process_code_before && m_npc_portrait) {
				m_npc_portrait->set_emotion(code.value);
				if (code.extras) {
					if (!code.extras->empty()) {
						if (code.extras->at(0) == 1) { m_npc_portrait->add_sparkler("portrait_sparkle"); }
					}
				} else {
					m_npc_portrait->remove_sparkler();
				}
				NANI_LOG_DEBUG(m_logger, "Emotion: {}", code.value);
				processed = true;
			}
		}
	}

	if (processed) { m_process_code_before = false; }
	if (m_npc_portrait) { m_npc_portrait->update(svc); }
	if (m_nani_portrait) { m_nani_portrait->update(svc); }
}

void Console::render(sf::RenderWindow& win) {
	if (m_launch.running()) { return; }
	// debug();
	if (!m_writer || !is_active()) { return; }
	m_nineslice.render(win);
	if (m_item_widget) { m_item_widget->render(*m_services, win); }
	if (m_npc_portrait) { m_npc_portrait->render(win); }
	if (m_nani_portrait) {
		m_nani_portrait->render(win);
		m_mode == ConsoleMode::responding ? m_nani_portrait->bring_in() : m_nani_portrait->send_out();
	}
	if (m_response) { m_response->render(win); }
	// m_writer->debug();
}

void Console::set_source(dj::Json const& json) { text_suite = json; }

void Console::set_nani_sprite(sf::Sprite const& sprite) {
	if (m_response) {
		if (m_response->has_flag_set(ResponseDialogFlags::hide_portrait)) { return; }
	}
	m_nani_portrait = Portrait(*m_services, sprite.getTexture(), 5, false);
}

void Console::handle_actions(int value) {
	switch (value) {
	case 1: // return to main menu
		NANI_LOG_DEBUG(m_logger, "Firing event from console: ReturnToMainMenuEvent");
		m_services->events.return_to_main_menu_event.dispatch();
		break;
	case 2: // restart save
		NANI_LOG_DEBUG(m_logger, "Firing event from console: ReloadSaveEvent");
		m_services->events.reload_save_event.dispatch(*m_services, value);
		break;
	case 3: m_services->state_controller.actions.set(automa::Actions::delete_file); break;
	}
}

void Console::load_and_launch(std::string_view key, OutputType type, int target_index) {
	m_writer = std::make_unique<TextWriter>(*m_services, text_suite, key, target_index);
	m_process_code_before = true;
	m_output_type = type;
	native_key = key;
}

void Console::load_and_launch(OutputType type) {
	m_writer = std::make_unique<TextWriter>(*m_services, text_suite);
	m_process_code_before = true;
	m_output_type = type;
	native_key = null_key;
}

void Console::load_single_message(std::string_view message) { m_writer = std::make_unique<TextWriter>(*m_services, message); }

void Console::display_item(std::string_view tag, bool sparkle) {
	m_item_widget = ItemWidget(*m_services, ItemWidgetType::item, tag);
	m_item_display_timer.start();
	if (!sparkle) { m_item_widget->remove_sparkler(); }
}

void Console::display_gun(std::string_view tag, bool sparkle) {
	m_item_widget = ItemWidget(*m_services, ItemWidgetType::gun, tag);
	if (!sparkle) { m_item_widget->remove_sparkler(); }
}

void Console::write(sf::RenderWindow& win, bool instant) {
	if (!is_active()) { return; }
	if (!m_writer) { return; }
	instant ? m_writer->write_instant_message(win) : m_writer->write_gradual_message(win);
}

void Console::write(sf::RenderWindow& win) { write(win, m_output_type == OutputType::instant); }

void Console::append(std::string_view key) { m_writer->append(key); }

void Console::end() {
	NANI_LOG_DEBUG(m_logger, "Ended console.");
	m_services->soundboard.flags.console.set(audio::Console::done);
	m_mode = ConsoleMode::off; // handle exit codes
	m_writer = {};
}

void Console::include_portrait(int id) { m_npc_portrait = Portrait{*m_services, id}; }

std::string Console::get_key() const { return native_key; }

void Console::handle_inputs(input::InputSystem& controller) {
	m_triggers = {};
	m_process_code_after = false;
	if (m_exit_stall.running()) { return; }
	auto const& up = controller.menu_move(input::MoveDirection::up);
	auto const& down = controller.menu_move(input::MoveDirection::down);
	auto const& next = controller.digital(input::DigitalAction::menu_select).triggered;
	auto const& exit = controller.digital(input::DigitalAction::menu_back).triggered && !m_flags.test(ConsoleFlags::no_exit);
	auto const& skip = controller.digital(input::DigitalAction::menu_select).held;
	auto const& released = controller.digital(input::DigitalAction::menu_select).released;
	static bool finished{};
	static bool can_skip{};
	static bool just_started{};
	static bool can_process{};
	bool responded{};

	if (next) {
		m_process_codes = true;
		if (m_writer->is_ready()) { m_process_code_after = true; }
	}

	// check for exit
	if (exit && can_exit()) {
		end();
		return;
	}

	// in response mode
	if (m_response) {
		if (m_response->handle_inputs(controller, m_services->soundboard)) {
			return;
		} else {
			// do something with response selection
			// set target suite if response code demands it
			if (auto response_codes = get_response_codes(m_response->get_selection())) {
				for (auto const& cde : response_codes.value()) {
					if (cde.is_response()) { m_writer->set_suite(cde.value); }
					if (cde.is_start_battle()) { m_services->events.start_battle_event.dispatch(); }
					if (cde.is_pop_conversation()) {
						m_services->events.npc_pop_conversation_event.dispatch();
						auto label = m_services->data.get_npc_label_from_id(cde.value);
						if (label && cde.extras) {
							m_services->quest_table.progress_quest("npc_dialogue", {label.value().data(), cde.extras->at(0)}, 1, -1, cde.value);
							NANI_LOG_DEBUG(m_logger, "Progressed NPC dialogue for NPC {} in room {}.", label.value().data(), cde.extras->at(0));
						}
					}
					if (cde.is_action()) { handle_actions(cde.value); }
					if (cde.is_quest()) {
						if (cde.extras) {
							if (cde.extras->size() > 1) { m_services->quest_table.progress_quest(m_services->quest_registry.get_quest_metadata(cde.value).get_tag(), cde.extras->at(0), cde.extras->at(1)); }
						}
					}
					if (cde.is_piggyback()) { m_services->events.npc_piggyback_event.dispatch(*m_services, cde.value); }
					if (cde.is_open_vendor()) { m_services->events.open_vendor_event.dispatch(*m_services, cde.value); }
					if (cde.is_item()) {
						m_services->events.acquire_item_from_console_event.dispatch(*m_services, cde.value);
						if (cde.extras) { m_services->data.destroy_inspectable(cde.extras->at(0)); }
						m_flags.set(ConsoleFlags::close_after_process);
					}
					if (cde.is_destructible()) { m_services->data.switch_destructible_state(cde.value); }
					if (cde.is_exit()) {
						end();
						return;
					}
					m_response = {};
					responded = true;
				}
			}
		}
	}

	// create response dialog
	if (next && m_writer->is_responding() && !responded) {

		if (auto codes = get_message_codes()) {
			for (auto& code : codes.value()) {
				if (code.is_response()) {
					// create a response dialog, feed it inputs, and await its closure before resuming m_writer
					m_response = ResponseDialog(m_services->text, text_suite, m_services->quest_table, native_key, code.value, m_position + m_response_offset);
					m_mode = ConsoleMode::responding;
					m_writer->wait();
					m_services->soundboard.flags.console.set(audio::Console::next);
					m_triggers.set(ConsoleTriggers::response_created);
					return;
				}
			}
		}
	}

	// player presses next at end of message
	if ((next && m_writer->is_ready()) || responded) {

		// process message codes that should trigger at the end of a message

		if (auto codes = get_message_codes()) {
			for (auto& code : codes.value()) {
				if (code.is_redirect() && !responded) {
					m_writer->set_suite(code.value);
					if (code.extras) { m_writer->set_index(code.extras.value()[0]); }
				}
				if (code.is_start_battle()) { m_services->events.start_battle_event.dispatch(); }
				if (code.is_reveal_item() && m_process_codes) { m_services->events.reveal_item_by_id_event.dispatch(code.value); }
				if (code.is_item() && m_process_code_after) { m_services->events.acquire_item_from_console_event.dispatch(*m_services, code.value); }
				if (code.is_weapon() && m_process_code_after) { m_services->events.acquire_weapon_from_console_event.dispatch(*m_services, code.value); }
				if (code.is_remove_weapon() && m_process_code_after) { m_services->events.remove_weapon_by_id_event.dispatch(*m_services, code.value); }
				if (code.is_open_vendor() && m_process_code_after) { m_services->events.open_vendor_event.dispatch(*m_services, code.value); }
				if (code.is_emotion() && m_process_code_after && m_npc_portrait && responded) { m_npc_portrait->set_emotion(code.value); }
				if (code.is_destroy_inspectable()) { m_services->data.destroy_inspectable(code.value); }
				if (code.is_destructible() && m_process_code_after) {
					auto inverse = code.extras ? code.extras->at(0) : 0;
					m_services->data.switch_destructible_state(code.value, static_cast<bool>(inverse));
				}
				NANI_LOG_DEBUG(m_logger, "Requested next with code {}", code.value);
			}
		}

		// go to next message or exit
		finished = m_writer->request_next();
		m_process_code_before = true;
		m_process_code_after = true;
		just_started = finished;
		can_skip = false;
		m_services->soundboard.flags.console.set(audio::Console::next);
	}

	// speed up text
	if (released || m_writer->is_first_message()) { can_skip = true; }
	if (m_writer->is_stalling()) { can_skip = false; }
	if (m_output_type == OutputType::no_skip) { can_skip = false; }
	(skip && can_skip) ? m_writer->speed_up() : m_writer->slow_down();

	if (finished) {
		if (m_writer->exit_requested()) {
			m_services->soundboard.flags.console.set(audio::Console::done);
			end();
			m_process_code_before = false;
		}
	}
}

void Console::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 256.f});
	if (ImGui::Begin("Console Debug")) {
		m_nani_portrait ? ImGui::Text("Nani Included") : ImGui::Text("<no nani portrait>");
		if (m_response) {
			ImGui::Text("Response Selection: %i", m_response->get_selection());
		} else {
			ImGui::Separator();
			ImGui::Text("<no response>");
		}
		if (m_writer) {
			if (auto codes = get_message_codes()) {
				for (auto& code : codes.value()) { code.debug(); }
			}
		} else {
			ImGui::Separator();
			ImGui::Text("<no writer>");
		}
		ImGui::End();
	}
}

auto Console::get_message_codes() const -> std::optional<std::vector<MessageCode>> {
	if (!m_writer) { return std::nullopt; }
	if (!m_writer->current_message().codes) { return std::nullopt; }
	return m_writer->current_message().codes;
}

auto Console::get_response_codes(int which) const -> std::optional<std::vector<MessageCode>> {
	if (!m_response) { return std::nullopt; }
	return m_response->get_codes(which);
}

void MessageCode::debug() {
	ImGui::SeparatorText("Message Code");
	ImGui::Text("Type: %i", static_cast<int>(type));
	ImGui::Text("Value: %i", value);
	if (extras) {
		for (auto& extra : *extras) { ImGui::Text("Extra: %i", extra); }
	}
}

} // namespace fornani::gui
