
#include "fornani/utils/RectPath.hpp"

#include <ccmath/math/misc/lerp.hpp>

#include "fornani/setup/ResourceFinder.hpp"

#include <djson/json.hpp>
#include <libassert/assert.hpp>

namespace fornani::util {

RectPath::RectPath(data::ResourceFinder& finder, std::filesystem::path source, std::string_view type, int interpolation) : m_scale{2.f}, m_interpolation{interpolation} {
	auto in_data = dj::Json::from_file((finder.resource_path() + source.string()).c_str());
	assert(!in_data.is_null());
	for (auto& path : in_data[type].array_view()) {
		m_sections.push_back(PathSection());
		m_sections.back().label = path["label"].as_string();
		for (auto& step : path["path"].array_view()) {
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
	auto previous_position{m_sections.at(m_current_section).path.at(std::clamp(m_current_step + (m_reverse ? -1 : -1), 0, end)).position};
	auto previous_dimensions{m_sections.at(m_current_section).path.at(std::clamp(m_current_step + (m_reverse ? -1 : -1), 0, end)).size};
	if (m_current_position == (m_reverse ? previous_position : target_position) && m_current_dimensions == (m_reverse ? previous_dimensions : target_dimensions)) {
		if (rev != m_reverse) { rev = m_reverse; }
		if (m_current_step != (m_reverse ? 0 : end)) { m_interpolation.start(); }
		m_current_step = std::clamp(m_current_step + (m_reverse ? -1 : 1), 0, end);
		return;
	}

	if (m_reverse) {
		m_current_position.x = ccm::lerp(target_position.x, previous_position.x, m_interpolation.get_inverse_cubic_normalized());
		m_current_position.y = ccm::lerp(target_position.y, previous_position.y, m_interpolation.get_inverse_cubic_normalized());
		m_current_dimensions.x = ccm::lerp(target_dimensions.x, previous_dimensions.x, m_interpolation.get_inverse_cubic_normalized());
		m_current_dimensions.y = ccm::lerp(target_dimensions.y, previous_dimensions.y, m_interpolation.get_inverse_cubic_normalized());
	} else {
		m_current_position.x = ccm::lerp(previous_position.x, target_position.x, m_interpolation.get_inverse_cubic_normalized());
		m_current_position.y = ccm::lerp(previous_position.y, target_position.y, m_interpolation.get_inverse_cubic_normalized());
		m_current_dimensions.x = ccm::lerp(previous_dimensions.x, target_dimensions.x, m_interpolation.get_inverse_cubic_normalized());
		m_current_dimensions.y = ccm::lerp(previous_dimensions.y, target_dimensions.y, m_interpolation.get_inverse_cubic_normalized());
	}
}

void RectPath::set_section(std::string_view to_section) {
	auto counter{0};
	for (auto& section : m_sections) {
		if (section.label == to_section.data()) { m_current_section = counter; }
		++counter;
	}
	m_current_position = m_sections.at(m_current_section).path.at(m_current_step).position;
	m_current_dimensions = m_sections.at(m_current_section).path.at(m_current_step).size;
}

void RectPath::reset() { m_current_step = 0; }

void RectPath::set_reverse(bool to) {
	if (m_reverse != to) { m_interpolation.invert(); }
	m_reverse = to;
}

auto RectPath::finished() const -> bool { return m_current_step == m_sections.at(m_current_section).path.size() - 1; }

} // namespace fornani::util
