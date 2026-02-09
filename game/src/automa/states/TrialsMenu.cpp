
#include <fornani/automa/states/TrialsMenu.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

TrialsMenu::TrialsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "trials"), m_loading{8}, m_stars{svc, "tt_stars", {12, 12}} {
	for (auto const& map : svc.data.map_table["rooms"].as_array()) {
		if (map["folder"].as_string() == "trials") {
			auto label = map["label"].as_string();
			auto dot = std::distance(label.begin(), std::find(label.begin(), label.end(), '.'));
			auto name = label.substr(0, dot);
			m_courses.push_back(CourseListing{name, map["room_id"].as<int>()});
			NANI_LOG_INFO(m_logger, "Added trial course [{}]", map["label"].as_string());
		}
	}

	// maually set options based on contents of trials folder
	options.clear();
	for (auto const& course : m_courses) { options.push_back(Option(svc, p_theme, course.label)); }
	auto ctr = 0;
	for (auto& option : options) {
		option.position = {64.f, top_buffer + ctr * spacing};
		option.index = ctr;
		option.update(current_selection.get());
		++ctr;
	}

	current_selection = util::Circuit{static_cast<int>(m_courses.size())};
	m_parent_menu = MenuType::play;
	m_loading.start();
	switch_selections(svc);
}

void TrialsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	m_loading.update();
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setOrigin({});
	}
	if (svc.input_system.menu_move(input::MoveDirection::up)) { switch_selections(svc); }
	if (svc.input_system.menu_move(input::MoveDirection::down)) { switch_selections(svc); }
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
		svc.state_controller.next_state = m_courses.at(current_selection.get()).id;
		svc.state_controller.actions.set(Actions::trials);
	}
}

void TrialsMenu::frame_update(ServiceProvider& svc) {}

constexpr auto tag_origin = sf::Vector2f{480.f, 80.f};
constexpr auto time_origin = sf::Vector2f{680.f, 80.f};
constexpr auto star_origin = sf::Vector2f{790.f, 76.f};
constexpr auto star_spacing = 26.f;

void TrialsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (m_loading.running()) { return; }
	MenuState::render(svc, win);
	auto ctr = 0.f;
	auto space = 32.f;
	for (auto& listing : m_listings) {
		listing.tag.setPosition(tag_origin + sf::Vector2f{0.f, space * ctr});
		win.draw(listing.tag);
		listing.time.setPosition(time_origin + sf::Vector2f{0.f, space * ctr});
		win.draw(listing.time);
		for (auto i = 0; i < 3; ++i) {
			m_stars.set_position(star_origin + sf::Vector2f{star_spacing * i, space * ctr});
			m_stars.set_texture_rect(i < listing.star_rating ? sf::IntRect{{12, 0}, {12, 12}} : sf::IntRect{{}, {12, 12}});
			win.draw(m_stars);
		}
		++ctr;
	}
}

void TrialsMenu::switch_selections(ServiceProvider& svc) {
	m_listings.clear();
	auto course = m_courses.at(current_selection.get()).id;
	auto list = svc.data.time_trial_registry.readout_attempts(course);
	if (list) {
		std::sort(list->begin(), list->end(), [](TrialAttempt const& a, TrialAttempt const& b) { return a.time < b.time; });
		auto ctr = 0;
		for (auto& time : *list) {
			auto next = TrialListing{{svc.text.fonts.basic}, {svc.text.fonts.title}};
			next.tag.setString(time.player_tag);
			next.tag.setCharacterSize(16);
			switch (ctr) {
			case 0:
				next.tag.setFillColor(colors::nani_white);
				next.time.setFillColor(colors::mythic_green);
				break;
			case 1:
				next.tag.setFillColor(colors::periwinkle);
				next.time.setFillColor(colors::dark_goldenrod);
				break;
			case 2:
				next.tag.setFillColor(colors::blue);
				next.time.setFillColor(colors::dark_orange);
				break;
			default:
				next.tag.setFillColor(colors::dark_grey);
				next.time.setFillColor(colors::dark_fucshia);
				break;
			}
			next.time.setString(std::format("{:.3f}", time.time));
			next.time.setCharacterSize(16);
			next.star_rating = time.star_rating;
			m_listings.push_back(next);
			++ctr;
		}
	}
}

} // namespace fornani::automa
