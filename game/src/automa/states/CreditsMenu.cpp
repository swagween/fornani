
#include "fornani/automa/states/CreditsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

CreditsMenu::CreditsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "credits"), m_loading{8} {
	m_parent_menu = MenuType::options;
	m_loading.start();

	auto credits_path = std::filesystem::path{svc.finder.resource_path()} / "data" / "extras" / "credits.json";
	auto data_result = dj::Json::from_file(credits_path.string());
	if (!data_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load credits from path: {}.", credits_path.string());
		return;
	}
	m_data = std::move(*data_result);
	assert(!m_data.is_null());

	refresh(svc);
}

void CreditsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	auto prev_selection = current_selection.get();
	MenuState::tick_update(svc, engine);
	m_loading.update();
	for (auto& option : options) {
		option.position.x = 64.f;
		option.update(current_selection.get());
		option.label.setOrigin({});
	}
	if (current_selection.get() != prev_selection) { refresh(svc); }
}

void CreditsMenu::frame_update(ServiceProvider& svc) {}

void CreditsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (m_loading.running()) { return; }
	MenuState::render(svc, win);
	auto ctr = 0.f;
	auto spacing = 24.f;
	for (auto& credit : m_credits) {
		credit.name.setPosition({320.f, spacing * ctr + top_buffer});
		credit.description.setPosition({500.f, spacing * ctr + top_buffer});
		win.draw(credit.name);
		win.draw(credit.description);
		ctr += 1 + credit.line_breaks;
	}
}

void CreditsMenu::refresh(ServiceProvider& svc) {
	std::string lookup = options.at(current_selection.get()).label.getString();
	m_credits.clear();
	for (auto const& credit : m_data[lookup].as_array()) {
		auto next = sf::Text{svc.text.fonts.basic};
		next.setFillColor(colors::bright_orange);
		next.setCharacterSize(16);
		next.setString(credit["name"].as_string());
		auto desc = sf::Text{svc.text.fonts.basic};
		desc.setFillColor(colors::dark_fucshia);
		desc.setCharacterSize(16);
		desc.setString(credit["description"].as_string());
		m_credits.push_back(Credit{next, desc, credit["line_breaks"].as<int>()});
	}
}

} // namespace fornani::automa
