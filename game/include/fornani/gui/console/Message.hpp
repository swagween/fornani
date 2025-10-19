
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/core/Common.hpp>
#include <optional>
#include <vector>

namespace fornani::gui {

struct MessageCode {
	MessageCode(dj::Json const& in);
	MessageCode(std::vector<int> in);
	MessageCodeType type{};
	int value{};
	std::optional<std::vector<int>> extras{};

	void serialize(dj::Json& out) const;
	void debug();

	[[nodiscard]] auto is_exit() const -> bool { return type == MessageCodeType::exit; }
	[[nodiscard]] auto is_response() const -> bool { return type == MessageCodeType::response; }
	[[nodiscard]] auto is_redirect() const -> bool { return type == MessageCodeType::redirect; }
	[[nodiscard]] auto is_quest() const -> bool { return type == MessageCodeType::quest; }
	[[nodiscard]] auto is_action() const -> bool { return type == MessageCodeType::action; }
	[[nodiscard]] auto is_item() const -> bool { return type == MessageCodeType::item; }
	[[nodiscard]] auto is_destructible() const -> bool { return type == MessageCodeType::destructible; }
	[[nodiscard]] auto is_input_hint() const -> bool { return type == MessageCodeType::input_hint; }
	[[nodiscard]] auto is_reveal_item() const -> bool { return type == MessageCodeType::reveal_item; }
	[[nodiscard]] auto is_start_battle() const -> bool { return type == MessageCodeType::start_battle; }
	[[nodiscard]] auto is_voice_cue() const -> bool { return type == MessageCodeType::voice; }
	[[nodiscard]] auto is_emotion() const -> bool { return type == MessageCodeType::emotion; }
	[[nodiscard]] auto is_pop_conversation() const -> bool { return type == MessageCodeType::pop_conversation; }
	[[nodiscard]] auto is_play_song() const -> bool { return type == MessageCodeType::play_song; }
	[[nodiscard]] auto is_weapon() const -> bool { return type == MessageCodeType::weapon; }
	[[nodiscard]] auto is_remove_weapon() const -> bool { return type == MessageCodeType::remove_weapon; }
	[[nodiscard]] auto is_remove_item() const -> bool { return type == MessageCodeType::remove_item; }
	[[nodiscard]] auto is_destroy_inspectable() const -> bool { return type == MessageCodeType::destroy_inspectable; }
	[[nodiscard]] auto is_piggyback() const -> bool { return type == MessageCodeType::piggyback; }
	[[nodiscard]] auto is_open_vendor() const -> bool { return type == MessageCodeType::open_vendor; }

	// editor helpers
	void mark_for_deletion() { delete_me = true; }
	[[nodiscard]] auto is_marked_for_deletion() const -> bool { return delete_me; }
	bool delete_me{};
};

struct Message {
	sf::Text data;
	std::optional<std::vector<MessageCode>> codes{};
};

// for creation of Inspectables
struct BasicMessage {
	std::string data{};
	std::optional<std::vector<MessageCode>> codes{};
};

} // namespace fornani::gui
