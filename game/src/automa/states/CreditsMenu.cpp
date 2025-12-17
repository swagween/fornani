
#include "fornani/automa/states/CreditsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

CreditsMenu::CreditsMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "credits"), m_loading{8} {
	m_parent_menu = MenuType::options;
	m_loading.start();

	auto data_result = dj::Json::from_file((svc.finder.resource_path() + "/data/extras/credits.json").c_str());
	if (!data_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load room data for path {}.", svc.finder.resource_path() + "/data/extras/credits.json");
		return;
	}
	m_data = std::move(*data_result);
	assert(!m_data.is_null());
}

void CreditsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	m_loading.update();
	for (auto& option : options) {
		option.position.x = 64.f;
		option.update(svc, current_selection.get());
		option.label.setOrigin({});
	}
	std::string lookup = options.at(current_selection.get()).label.getString();
	m_credits.clear();
	for (auto const& credit : m_data[lookup].as_array()) {
		auto next = sf::Text{svc.text.fonts.basic};
		next.setFillColor(colors::bright_orange);
		next.setCharacterSize(16);
		next.setString(credit["name"].as_string());
		auto desc = next;
		desc.setString(credit["description"].as_string());
		desc.setFillColor(colors::dark_fucshia);
		m_credits.push_back(Credit{next, desc, credit["line_breaks"].as<int>()});
	}
}

void CreditsMenu::frame_update(ServiceProvider& svc) {}

void CreditsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (m_loading.running()) { return; }
	MenuState::render(svc, win);
	auto ctr = 0.f;
	auto spacing = 24.f;
	for (auto& credit : m_credits) {
		credit.name.setPosition({360.f, spacing * ctr + top_buffer});
		credit.description.setPosition({540.f, spacing * ctr + top_buffer});
		win.draw(credit.name);
		win.draw(credit.description);
		ctr += 1 + credit.line_breaks;
	}
}

} // namespace fornani::automa
