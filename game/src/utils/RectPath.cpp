
#include "fornani/utils/RectPath.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <ccmath/ext/clamp.hpp>
#include <ccmath/math/misc/lerp.hpp>
#include <djson/json.hpp>
#include <libassert/assert.hpp>

namespace fornani::util {

RectPath::RectPath(ResourceFinder& finder, std::filesystem::path source, std::string_view type, int interpolation, util::InterpolationType it) : m_scale{2.f}, m_interpolation{interpolation}, m_interpolation_type{it} {
	auto in_data = *dj::Json::from_file((finder.resource_path() + source.string()).c_str());
	assert(!in_data.is_null());
	for (auto& path : in_data[type].as_array()) {
		m_sections.push_back(PathSection());
		m_sections.back().label = path["label"].as_string();
		for (auto& step : path["path"].as_array()) {
			auto next{sf::FloatRect{{step["position"][0].as<float>() * m_scale, step["position"][1].as<float>() * m_scale}, {step["dimensions"][0].as<float>() * m_scale, step["dimensions"][1].as<float>() * m_scale}}};
			m_sections.back().path.push_back(next);
		}
	}
}

void RectPath::update() {
	m_interpolation.update();
	static bool rev{};
	auto end{static_cast<int>(m_sections.at(m_current_section).path.size()) - 1};
	auto target_position{m_sections.at(m_current_section).path.at(m_current_step).position};
	auto target_dimensions{m_sections.at(m_current_section).path.at(m_current_step).size};
	auto previous_position{m_sections.at(m_current_section).path.at(ccm::ext::clamp(m_current_step + -1, 0, end)).position};
	auto previous_dimensions{m_sections.at(m_current_section).path.at(ccm::ext::clamp(m_current_step + -1, 0, end)).size};
	if (m_current_position == target_position && m_current_dimensions == target_dimensions) {
		if (m_current_step != end) { m_interpolation.start(); }
		m_current_step = ccm::ext::clamp(m_current_step + 1, 0, end);
		m_step_completed = true;
		return;
	}
	m_step_completed = false;
	float interpolation{};
	switch (m_interpolation_type) {
	case util::InterpolationType::linear: interpolation = m_interpolation.get_inverse_normalized(); break;
	case util::InterpolationType::quadratic: interpolation = m_interpolation.get_inverse_quadratic_normalized(); break;
	case util::InterpolationType::cubic: interpolation = m_interpolation.get_inverse_cubic_normalized(); break;
	}

	m_current_position.x = ccm::lerp(previous_position.x, target_position.x, interpolation);
	m_current_position.y = ccm::lerp(previous_position.y, target_position.y, interpolation);
	m_current_dimensions.x = ccm::lerp(previous_dimensions.x, target_dimensions.x, interpolation);
	m_current_dimensions.y = ccm::lerp(previous_dimensions.y, target_dimensions.y, interpolation);
}

void RectPath::set_section(std::string_view to_section) {
	m_current_step = 0;
	auto counter{0};
	for (auto& section : m_sections) {
		if (section.label == to_section.data()) { m_current_section = counter; }
		++counter;
	}
	m_current_position = m_sections.at(m_current_section).path.at(m_current_step).position;
	m_current_dimensions = m_sections.at(m_current_section).path.at(m_current_step).size;
}

void RectPath::reset() { m_current_step = 0; }

auto RectPath::finished() const -> bool { return m_current_step == m_sections.at(m_current_section).path.size() - 1 && m_step_completed; }

} // namespace fornani::util
