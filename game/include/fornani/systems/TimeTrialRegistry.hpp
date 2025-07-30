
#pragma once

#include <fornani/io/Logger.hpp>
#include <optional>
#include <string>
#include <unordered_map>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

struct TrialAttempt {
	std::string player_tag{}; // unused for now
	float time{};
	std::uint8_t star_rating{};
};

class TimeTrialRegistry {
  public:
	bool register_time(automa::ServiceProvider& svc, int course, std::string_view tag, float time);
	void insert_time(automa::ServiceProvider& svc, int course, std::string_view tag, float time);
	std::optional<std::vector<TrialAttempt>> readout_attempts(int const course);

  private:
	std::uint8_t calculate_rating(automa::ServiceProvider& svc, int const course_id, float const time) const;
	std::unordered_map<int, std::vector<TrialAttempt>> m_trial_attempts{};

	io::Logger m_logger{"System"};
};

} // namespace fornani
