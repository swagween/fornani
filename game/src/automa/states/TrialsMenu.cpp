
#include <fornani/automa/states/TrialsMenu.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

TrialsMenu::TrialsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "trials"), m_loading{8} {
	m_parent_menu = MenuType::play;
	m_loading.start();
	switch_selections(svc);
}

void TrialsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	m_loading.update();
	for (auto& option : options) {
		option.position.x = 64.f;
		option.update(svc, current_selection.get());
		option.label.setOrigin({});
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) { switch_selections(svc); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) { switch_selections(svc); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		switch (current_selection.get()) {
		case 0:
			svc.state_controller.next_state = 9902;
			svc.state_controller.actions.set(Actions::trials);
			break;
		case 1:
			svc.state_controller.next_state = 990;
			svc.state_controller.actions.set(Actions::trials);
			break;
		}
	}
}

void TrialsMenu::frame_update(ServiceProvider& svc) {}

constexpr auto tag_origin = sf::Vector2f{480.f, 80.f};
constexpr auto time_origin = sf::Vector2f{680.f, 80.f};

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
		++ctr;
	}
}

void TrialsMenu::switch_selections(ServiceProvider& svc) {
	m_listings.clear();
	auto course = current_selection.get() == 0 ? 9902 : 990; // obviously do this differently later
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
			m_listings.push_back(next);
			++ctr;
		}
	}
}

} // namespace fornani::automa
