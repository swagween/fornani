
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <djson/json.hpp>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include "fornani/graphics/HelpText.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Decoder.hpp"
#include "fornani/utils/QuestCode.hpp"
#include "fornani/utils/Shipment.hpp"
#include "fornani/io/Logger.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

struct Message {
	sf::Text data;
	bool prompt{};
	int target{};
};

enum class Codes : uint8_t { prompt, quest, item, voice, emotion, hash };
enum class MessageState : uint8_t { writing, selection_mode, cannot_skip, response_trigger, done_writing, started_delay };
static constexpr int default_writing_speed{8};
static constexpr int fast_writing_speed{1};

class TextWriter {
  public:
	explicit TextWriter(automa::ServiceProvider& svc, dj::Json& source, std::string_view key);
	explicit TextWriter(automa::ServiceProvider& svc, std::string_view message);
	void start();
	void update();
	void flush();
	void set_position(sf::Vector2<float> pos);
	void set_bounds(sf::Vector2<float> new_bounds);
	void append(std::string_view content);
	void write_instant_message(sf::RenderWindow& win);
	void write_gradual_message(sf::RenderWindow& win);
	void write_responses(sf::RenderWindow& win);
	void activate();
	void deactivate();
	void request_next();
	void check_for_event(Message& msg, Codes code, bool response = false);
	void adjust_selection(int amount);
	void process_selection();
	void process_quest(util::QuestKey out);
	void shutdown();

	void skip_ahead();
	void enable_skip();
	void reset();

	[[nodiscard]] auto writing() const -> bool { return flags.test(MessageState::writing); };
	[[nodiscard]] auto complete() const -> bool { return !flags.test(MessageState::writing) && suite.empty(); };
	[[nodiscard]] auto selection_mode() const -> bool { return flags.test(MessageState::selection_mode); };
	[[nodiscard]] auto can_skip() const -> bool { return !flags.test(MessageState::cannot_skip); };
	[[nodiscard]] auto response_triggered() const -> bool { return flags.test(MessageState::response_trigger); }
	[[nodiscard]] auto responding() const -> bool { return selection_mode(); }
	[[nodiscard]] auto suite_size() const -> size_t { return suite.size(); }
	[[nodiscard]] auto empty() const -> bool { return suite.empty() && responses.empty(); }
	[[nodiscard]] auto delaying() const -> bool { return delay.running(); }

	void reset_delay() {
		flags.reset(MessageState::done_writing);
		flags.reset(MessageState::started_delay);
	}
	void reset_response() { flags.reset(MessageState::response_trigger); }
	void flush_communicators() { communicators = {}; }

	Message& current_message();	 // for debug
	Message& current_response(); // for debug
	int get_current_selection() const;
	int get_current_suite_set() const;

	// public for debugging
	int text_size{16};

	struct {
		util::Shipment out_item{};
		util::Shipment out_quest{};
		util::Shipment out_voice{};
		util::Shipment out_emotion{};
		util::Shipment reveal_item{};
	} communicators{};

	std::vector<util::QuestKey> response_keys{};
	util::QuestKey out_quest{};
	util::Decoder decoder{};

  private:
	explicit TextWriter(automa::ServiceProvider& svc);
	void load_single_message(std::string_view message);
	void load_message(dj::Json& source, std::string_view key);
	void stylize(sf::Text& msg, bool is_suite) const;
	void wrap();
	void constrain();

	std::deque<std::deque<Message>> suite{};
	std::deque<std::deque<Message>> responses{};

	struct {
		int current_suite_set{};
		int current_response_set{};
		int current_selection{};
	} iterators{};

	std::unordered_map<Codes, char> special_characters{};

	sf::Text working_message;

	graphics::HelpText help_marker;

	std::string working_str{};
	sf::Font* m_font;
	int glyph_count{};
	int tick_count{};
	int writing_speed{default_writing_speed};
	util::Cooldown delay{32};
	util::BitFlags<MessageState> flags{};
	sf::Vector2<float> position{};
	sf::Vector2<float> bounds{};
	sf::Vector2<float> response_position{300.f, 200.f};
	sf::Vector2<float> response_offset{120.f, 60.f};
	float pad{30};

	sf::RectangleShape indicator{};
	sf::RectangleShape cursor{};

	automa::ServiceProvider* m_services;

	Message zero_option; // for debug
	sf::RectangleShape bounds_box{};

	io::Logger m_logger{"TextWriter"};
};

} // namespace fornani::text
