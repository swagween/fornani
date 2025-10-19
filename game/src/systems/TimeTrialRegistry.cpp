
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/TimeTrialRegistry.hpp>

namespace fornani {

constexpr auto max_records_v = 8;

void TimeTrialRegistry::insert_time(automa::ServiceProvider& svc, int course, std::string_view tag, float time) {
	if (m_trial_attempts.contains(course)) {
		m_trial_attempts.at(course).push_back(TrialAttempt{tag.data(), time, calculate_rating(svc, course, time)});
		NANI_LOG_DEBUG(m_logger, "Rating: {}", calculate_rating(svc, course, time));
	} else {
		m_trial_attempts.insert({course, std::vector<TrialAttempt>{TrialAttempt{tag.data(), time, calculate_rating(svc, course, time)}}});
	}
}

bool TimeTrialRegistry::register_time(automa::ServiceProvider& svc, int course, std::string_view tag, float time) {
	bool was_registered{true};
	if (m_trial_attempts.contains(course)) {
		auto win_counter = 0;
		auto& list = m_trial_attempts.at(course);
		if (list.size() < max_records_v) { // we have room in the registry
			list.push_back(TrialAttempt{tag.data(), time, calculate_rating(svc, course, time)});
		} else {
			for (auto& record : list) {
				if (time < record.time) { ++win_counter; }
			}
			if (win_counter == 0) { // it's not fast enough
				was_registered = false;
			} else { // it's a new record, remove the worst time
				list.push_back(TrialAttempt{tag.data(), time, calculate_rating(svc, course, time)});
				auto worst = list.back();
				for (auto& entry : list) {
					if (entry.time > worst.time) { worst = entry; }
				}
				std::erase_if(list, [worst](auto const& e) { return e.time == worst.time; });
			}
		}
	} else {
		m_trial_attempts.insert({course, std::vector<TrialAttempt>{TrialAttempt{tag.data(), time, calculate_rating(svc, course, time)}}});
	}
	if (was_registered) {
		bool found{};
		for (auto [index, track] : std::views::enumerate(svc.data.time_trial_data["trials"].as_array())) {
			if (track["course_id"].as<int>() == course) {
				found = true;
				svc.data.time_trial_data["trials"][index]["times"].set_value(dj::Json::empty_array());
				for (auto const& registered_time : m_trial_attempts.at(course)) {
					auto t = dj::Json{};
					dj::to_json(t["player_tag"], registered_time.player_tag);
					dj::to_json(t["time"], registered_time.time);
					svc.data.time_trial_data["trials"][index]["times"].push_back(t);
				}
			}
		}
		if (!found) {
			auto c = dj::Json{};
			auto t = dj::Json{};
			dj::to_json(c["course_id"], course);
			dj::to_json(t["player_tag"], tag);
			dj::to_json(t["time"], time);
			c["times"].push_back(t);
			svc.data.time_trial_data["trials"].push_back(c);
		}
		if (!svc.data.time_trial_data.to_file((svc.finder.resource_path() + "/data/save/time_trials.json").c_str())) {
			NANI_LOG_ERROR(m_logger, "Failed to save time trial record!");
		} else {
			NANI_LOG_INFO(m_logger, "New Record! Registered [{}]'s Time for course {}: {}", tag.data(), course, time);
		}
	}
	return was_registered;
}

std::optional<std::vector<TrialAttempt>> TimeTrialRegistry::readout_attempts(int const course) {
	if (m_trial_attempts.contains(course)) { return m_trial_attempts.at(course); }
	return std::nullopt;
}

std::uint8_t TimeTrialRegistry::calculate_rating(automa::ServiceProvider& svc, int const course_id, float const time) const {
	auto standards = dj::Json{};
	standards = *dj::Json::from_file((svc.finder.resource_path() + "/data/trials/standards.json").c_str());
	assert(!standards.is_null());
	for (auto const& in_standard : standards["trials"].as_array()) {
		if (in_standard["course_id"].as<int>() == course_id) {
			auto ret = 0u;
			for (auto const& t : in_standard["standards"].as_array()) {
				if (time < t.as<float>()) { ++ret; }
			}
			return ret;
		}
	}
	return 0u;
}

} // namespace fornani
