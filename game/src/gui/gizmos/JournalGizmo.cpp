
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/JournalGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>
#include <numbers>

namespace fornani::gui {

JournalGizmo::JournalGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Journal", false), m_text{.readout{svc.text.fonts.title.font}}, m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "journal_screen", 80, util::InterpolationType::quadratic}, m_services{&svc},
	  m_selector_sprite{svc.assets.get_texture("pioneer_pointer")}, m_indicator{svc, "pioneer_point"} {
	m_physics.position = sf::Vector2f{334.f, 100.f};
	m_placement = placement;
	m_text.readout.setFillColor(colors::pioneer_red);
	m_text.readout.setCharacterSize(svc.text.fonts.basic.glyph_size);
	m_dashboard_port = DashboardPort::journal;
	m_text.readout.setString(svc.data.gui_text["journal"]["quest"].as_string());
	for (auto i = 0; i < svc.quest_registry.get_size(); ++i) {
		auto const& meta = svc.quest_registry.get_quest_metadata(i);
		auto tag = meta.get_tag().data();
		if (!svc.quest_table.was_quest_started(tag)) { continue; }
		if (!svc.quest_registry.get_json(tag)["displayed"].as_bool()) { continue; }

		auto entry = QuestEntry{tag, svc.text.fonts.title.font};
		entry.title.setCharacterSize(svc.text.fonts.title.glyph_size);
		auto title = svc.quest_registry.get_json(tag)["title"].as_string();
		entry.title.setString(title);
		entry.title.setFillColor(colors::pioneer_dark_red);
		if (svc.quest_table.get_quest_progression(tag) >= svc.quest_registry.get_json(tag)["target"].as<int>()) { entry.title.setFillColor(colors::navy_blue); }

		m_text.listing.push_back(entry);
	}
	if (!m_screen.resize({24u, 24u})) {}
	m_screen.clear(colors::transparent);
	auto sprite = sf::Sprite{svc.assets.get_texture("pioneer_screen")};
	m_screen.draw(sprite);
	m_screen.setRepeated(true);

	m_path.set_section("start");

	m_selector_sprite.setScale(constants::f_scale_vec);
	m_indicator.center();
	for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
		if (svc.data.active_quest == svc.quest_registry.get_index_from_tag(entry.tag)) { m_selected_quest = i; }
	}
	m_text.readout.setOrigin({10.f, m_text.readout.getLocalBounds().size.y * 1.5f});
}

void JournalGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched || m_exit_trigger) {
		on_close(svc, player, map);
	}
	m_path.update();
	m_questlog_position = get_placement() + m_path.get_position() + sf::Vector2f{40.f, 20.f + position.y + m_text.readout.getLocalBounds().size.y};
	if (!is_selected()) { m_questlog_position -= sf::Vector2f{8.f, m_selected_quest * m_spacing}; }
	m_physics.simple_update();
	if (m_text.objective) { m_text.objective->update(); }
	if (m_selector) {
		m_selector->set_position(m_questlog_position + sf::Vector2f{-34.f, static_cast<float>(m_selector->get_current_selection())});
		m_selector->update();
	}

	// update list animation
	float accumulated_offset = 0.f;
	for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
		auto selected = m_selector ? m_selector->get_current_selection() == i : true;
		float target_y = i * m_spacing + accumulated_offset;
		entry.offset.y = std::lerp(entry.offset.y, target_y, 0.1f);
		entry.offset.x = selected ? -4.f : 0.f;
		if (selected && m_text.objective) { accumulated_offset = m_text.objective->current_message().data.getLocalBounds().size.y + 4.f; }
	}
}

void JournalGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	Gizmo::render(svc, win, player, shader, palette, cam, foreground);
	if (is_foreground() != foreground) { return; }

	auto sprite = sf::Sprite{m_screen.getTexture()};
	sprite.setScale(constants::f_scale_vec);
	sprite.setTextureRect(sf::IntRect{sf::Vector2i{m_path.get_dimensions() * 0.3f}, sf::Vector2i{m_path.get_dimensions()}});
	sprite.setPosition(get_placement() + m_path.get_position() - cam);

	if (is_selected()) { win.draw(sprite); }

	m_text.readout.setPosition(m_questlog_position - cam);
	win.draw(m_text.readout);
	for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
		auto pos = m_questlog_position - cam + entry.offset;
		entry.title.setPosition(pos);
		if (is_selected() || i == m_selected_quest) { win.draw(entry.title); }
		svc.quest_table.is_quest_complete(entry.tag) ? entry.title.setFillColor(colors::navy_blue) : entry.title.setFillColor(colors::pioneer_dark_red);
		if (!is_selected()) { is_hovered() ? entry.title.setFillColor(colors::pioneer_mid_red) : entry.title.setFillColor(colors::pioneer_dark_red); }
		if (!m_selector) { continue; }
		if (!is_selected() || !m_text.objective || m_selector->get_current_selection() != i) { continue; }
		auto spacing = 1.5f;
		m_services->quest_table.is_quest_complete(entry.tag) ? entry.title.setFillColor(colors::navy_blue) : entry.title.setFillColor(colors::pioneer_red);
		m_text.objective->set_bounds(sf::FloatRect{pos + sf::Vector2f{32.f, entry.title.getLocalBounds().size.y * spacing}, m_path.get_dimensions()}, true);
		m_text.objective->write_instant_message(win);
	}

	if (m_selector && is_selected() && !m_text.listing.empty()) {
		m_selector->render(win, m_selector_sprite, cam, {});
		m_indicator.set_position(m_text.listing.at(m_selected_quest).title.getPosition() + sf::Vector2f{-8.f, 11.f});
		if (is_quest()) { win.draw(m_indicator); }
	}
}

bool JournalGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {

	constexpr int count = static_cast<int>(JournalSection::END);
	if (controller.digital(input::DigitalAction::menu_tab_right).triggered) {
		m_section = static_cast<JournalSection>((static_cast<int>(m_section) + 1) % count);
		switch_sections(*m_services);
	}
	if (controller.digital(input::DigitalAction::menu_tab_left).triggered) {
		m_section = static_cast<JournalSection>((static_cast<int>(m_section) + count - 1) % count);
		switch_sections(*m_services);
	}

	if (!m_selector) { return Gizmo::handle_inputs(controller, soundboard); }
	if (m_text.listing.empty()) { return Gizmo::handle_inputs(controller, soundboard); }
	if (controller.menu_move(input::MoveDirection::up)) {
		if (m_selector->move_direction({0, -1}).up()) {}
		refresh();
	}
	if (controller.menu_move(input::MoveDirection::down)) {
		if (m_selector->move_direction({0, 1}).down()) {}
		refresh();
	}
	if (controller.digital(input::DigitalAction::menu_select).triggered && is_quest() && !m_text.listing.empty()) {
		m_services->data.active_quest = m_services->quest_registry.get_index_from_tag(m_text.listing.at(m_selector->get_current_selection()).tag);
		m_selected_quest = m_selector->get_current_selection();
		soundboard.play_sound("menu_select");
	}
	return Gizmo::handle_inputs(controller, soundboard);
}

void JournalGizmo::on_open(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	Gizmo::on_open(svc, player, map);
	m_path.set_section("open");
	m_selector.emplace(InventorySelector{{1, static_cast<int>(m_text.listing.size())}, {0.f, m_spacing}});
	if (m_text.listing.empty() || !m_selector) { return; }
	for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
		if (svc.data.active_quest == svc.quest_registry.get_index_from_tag(entry.tag)) {
			m_selector->set_selection({0, static_cast<int>(i)});
			m_selected_quest = i;
		}
	}
	m_text.objective.emplace(svc);
	m_text.objective->set_font(svc.text.fonts.basic);
	set_text();
	m_selector->set_lookup({{}, {8, 8}});
}

void JournalGizmo::on_close(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_path.set_section("close");
	m_text.objective.reset();
	if (m_section == JournalSection::bestiary) {
		m_section = JournalSection::quest;
		switch_sections(svc);
	}
}

void JournalGizmo::refresh() {
	if (m_text.objective) { m_text.objective->flush(); }
	set_text();
	m_services->soundboard.play_sound("menu_shift");

	for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
		m_services->quest_table.is_quest_complete(entry.tag) ? entry.title.setFillColor(colors::navy_blue) : entry.title.setFillColor(colors::pioneer_dark_red);
		if (!m_selector) { continue; }
		if (!is_selected() || !m_text.objective || m_selector->get_current_selection() != i) { continue; }
		m_services->quest_table.is_quest_complete(entry.tag) ? entry.title.setFillColor(colors::navy_blue) : entry.title.setFillColor(colors::pioneer_red);
	}
}

void JournalGizmo::set_text() {
	if (m_text.listing.empty()) { return; }
	if (is_quest()) {
		m_text.objective->load_single_message(m_services->quest_table.readout(m_text.listing.at(m_selector->get_current_selection()).tag));
		m_text.objective->set_font_color(colors::pioneer_mid_red);
		if (m_text.objective->current_message().data.getString().isEmpty()) {
			m_text.objective->load_single_message(m_services->data.gui_text["menu"]["completed"].as_string());
			m_text.objective->set_font_color(colors::navy_blue);
		}
	}
	if (is_bestiary()) {
		auto const& bestiary = m_services->data.get_bestiary();
		auto it = std::find_if(bestiary.begin(), bestiary.end(), [&](auto const& e) { return e.tag == m_text.listing.at(m_selector->get_current_selection()).tag; });
		if (it != bestiary.end()) { m_text.objective->load_single_message(m_services->data.gui_text["bestiary"]["fallen"].as_string() + std::to_string(it->fallen)); }
		m_text.objective->set_font_color(colors::pioneer_mid_red);
	}
}

void JournalGizmo::switch_sections(automa::ServiceProvider& svc) {
	m_text.listing.clear();
	m_text.objective.reset();
	switch (m_section) {
	case JournalSection::quest:
		m_text.readout.setString(svc.data.gui_text["journal"]["quest"].as_string());
		for (auto i = 0; i < svc.quest_registry.get_size(); ++i) {
			auto const& meta = svc.quest_registry.get_quest_metadata(i);
			auto tag = meta.get_tag().data();
			if (!svc.quest_table.was_quest_started(tag)) { continue; }
			if (!svc.quest_registry.get_json(tag)["displayed"].as_bool()) { continue; }

			auto entry = QuestEntry{tag, svc.text.fonts.title.font};
			entry.title.setCharacterSize(svc.text.fonts.title.glyph_size);
			auto title = svc.quest_registry.get_json(tag)["title"].as_string();
			entry.title.setString(title);
			entry.title.setFillColor(colors::pioneer_dark_red);
			if (svc.quest_table.get_quest_progression(tag) >= svc.quest_registry.get_json(tag)["target"].as<int>()) { entry.title.setFillColor(colors::navy_blue); }

			m_text.listing.push_back(entry);
		}
		for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
			if (svc.data.active_quest == svc.quest_registry.get_index_from_tag(entry.tag)) { m_selected_quest = i; }
		}
		break;
	case JournalSection::bestiary:
		m_text.readout.setString(svc.data.gui_text["journal"]["bestiary"].as_string());
		for (auto const& e : svc.data.get_bestiary()) {
			auto entry = QuestEntry{e.tag, svc.text.fonts.title.font};
			entry.title.setCharacterSize(svc.text.fonts.title.glyph_size);
			auto title = svc.data.enemy[e.tag]["metadata"]["name"].as_string();
			entry.title.setString(title);
			entry.title.setFillColor(colors::pioneer_dark_red);
			m_text.listing.push_back(entry);
		}
		break;
	}
	m_text.readout.setOrigin({10.f, m_text.readout.getLocalBounds().size.y * 1.5f});
	if (!m_text.listing.empty()) {
		m_selector.emplace(InventorySelector{{1, static_cast<int>(m_text.listing.size())}, {0.f, m_spacing}});
		for (auto [i, entry] : std::views::enumerate(m_text.listing)) {
			if (svc.data.active_quest == svc.quest_registry.get_index_from_tag(entry.tag)) {
				if (m_selector) { m_selector->set_selection({0, static_cast<int>(i)}); }
				m_selected_quest = i;
			}
		}
	}
	m_text.objective.emplace(svc);
	m_text.objective->set_font(svc.text.fonts.basic);
	set_text();
}

} // namespace fornani::gui
