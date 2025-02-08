
#pragma once

#include <imgui.h>
#include <SFML/Graphics.hpp>

#include <chrono>
#include <cstdint>
#include <format>
#include <optional>
#include <string_view>
#include <unordered_map>

#include "fornani/utils/FixedString.hpp"

namespace fornani::util {
struct AppLog {
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool AutoScroll;		   // Keep scrolling if already at the bottom.

	AppLog() {
		AutoScroll = true;
		clear();
	}

	void clear() {
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void add_log(char const* fmt, ...) IM_FMTARGS(2) {
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n') LineOffsets.push_back(old_size + 1);
	}

	void draw(char const* title, bool* p_open = nullptr) {
		if (!ImGui::Begin(title, p_open)) {
			ImGui::End();
			return;
		}

		// Main window
		ImGui::Text("Console");
		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			char const* buf = Buf.begin();
			char const* buf_end = Buf.end();
			if (Filter.IsActive()) {
				for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
					char const* line_start = buf + LineOffsets[line_no];
					if (char const* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end; Filter.PassFilter(line_start, line_end)) ImGui::TextUnformatted(line_start, line_end);
				}
			} else {
				ImGuiListClipper clipper;
				clipper.Begin(LineOffsets.Size);
				while (clipper.Step()) {
					for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
						char const* line_start = buf + LineOffsets[line_no];
						char const* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
						ImGui::TextUnformatted(line_start, line_end);
					}
				}
				clipper.End();
			}
			ImGui::PopStyleVar();
			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		ImGui::End();
	}
};

class Logger {
  public:
	void add_log(char const* message);
	void write_console(ImVec2 size, ImVec2 pos);

  private:
	AppLog log{};
};

} // namespace fornani::util

// Thread safe logger class
// If you have questions about this, ask Ian.

namespace fornani::io::logger {
/**
 * \brief Log Level.
 */
enum class Level : std::uint8_t { eError, eWarn, eInfo, eDebug, eCOUNT_ };

/**
 * \brief char representation of Level.
 */
constexpr char levelChar(Level const level) {
	switch (level) {
	case Level::eError: return 'E';
	case Level::eWarn: return 'W';
	case Level::eInfo: return 'I';
	case Level::eDebug: return 'D';
	default: return '?';
	}
}

/**
 * \brief Log Target (destination).
 */
struct Target {
	std::uint32_t value{};

	constexpr operator std::uint32_t() const { return value; }
};

/**
 * \brief Standard IO streams.
 */
inline constexpr Target console_v{.value = 1 << 0};

/**
 * \brief Log file.
 */
inline constexpr Target file_v{.value = 1 << 1};

/**
 * \brief Custom sinks.
 */
inline constexpr Target sinks_v{.value = 1 << 2};

inline constexpr Target all_v{.value = 0xffffffff};

/**
 * \brief Timestamp mode.
 */
enum class Timestamp { eLocal, eUtc };

struct Config {
	/**
	 * \brief Default format specification for log entries.
	 * Syntax: "{format_key}..."
	 * Supported format keys: level, thread, category, message, timestamp
	 */
	static constexpr std::string_view default_format_v{"[{level}][T{thread}] [{category}] {message} [{timestamp}]"};

	/**
	 * \brief Default format specification for verbose log entries.
	 * Supported format keys: level, thread, category, message, timestamp, func, file, line
	 */
	static constexpr std::string_view verbose_format_v{"[{level}][T{thread}] [{category}] {message} [{timestamp}] [F:{func}] [{file}:{line}]"};

	static constexpr std::size_t format_size_v{128};

	static_assert(default_format_v.size() < format_size_v);
	static_assert(verbose_format_v.size() < format_size_v);

	/**
	 * \brief Format specification for log entries.
	 */
	FixedString<format_size_v> format{default_format_v};

	/**
	 * \brief Max log Level.
	 */
	Level maxLevel{Level::eDebug};

	/**
	 * \brief Max log Level overrides per category.
	 */
	std::unordered_map<std::string_view, Level> categoryMaxLevels{};

	/**
	 * \brief Log Target overrides per Level.
	 */
	std::unordered_map<Level, Target> levelTargets{};

	/**
	 * \brief Timestamp mode.
	 */
	Timestamp timestamp{Timestamp::eLocal};
};

using Clock = std::chrono::system_clock;

/**
 * \brief Strongly typed integer representing logging thread ID.
 */
enum struct ThreadId : int {};

/**
 * \brief Log context.
 * Expected to be built at the call site and passed around.
 */
struct Context {
	std::string_view category{};
	Clock::time_point timestamp{};
	ThreadId thread{};
	Level level{};
	std::optional<std::string_view> func{};
	std::optional<std::string_view> file{};
	std::optional<int> line{};

	/**
	 * \brief Obtain this thread's ID.
	 * \returns Monotonically increasing IDs per thread, in order of being called for the first time.
	 */
	static ThreadId getThreadId();

	static Context make(std::string_view category, Level level);

	static Context make(std::string_view category, Level level, std::string_view function, std::string_view filePath, int currentLine);
};

/**
 * \brief Customizable log sink.
 */
struct Sink {
	virtual ~Sink() = default;

	/**
	 * \brief Customization point for intercepting logs.
	 * \param formatted Formatted log string.
	 * \param context Log context.
	 */
	virtual void handle(std::string_view formatted, Context const& context) = 0;
};

/**
 * \brief Logger Instance: a single instance must be created within main's scope.
 */
class Instance {
  public:
	/**
	 * \brief Error thrown if more than one instance is attempted to be created.
	 */
	struct DuplicateError : std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	Instance(Instance&&) = delete;

	Instance& operator=(Instance&&) = delete;

	Instance(Instance const&) = delete;

	Instance& operator=(Instance const&) = delete;

	/**
	 * \brief Create a logger Instance.
	 * \param filePath path to create/overwrite log file at.
	 * \param config Config to use.
	 */
	explicit Instance(char const* filePath = "breakout.log", Config config = {});

	~Instance();

	/**
	 * \brief Obtain a copy of the Config in use.
	 */
	[[nodiscard]] Config getConfig() const;

	/**
	 * \brief Overwrite the Config in use.
	 */
	void setConfig(Config config) const;

	/**
	 * \brief Add a custom sink.
	 */
	void addSink(std::unique_ptr<Sink> sink) const;

	/**
	 * \brief Entrypoint for logging (free) functions.
	 */
	static void print(std::string_view message, Context const& context);

  private:
	struct Impl;

	struct Deleter {
		void operator()(Impl const* ptr) const;
	};

	// NOLINTNEXTLINE
	inline static Impl* s_instance{};
	std::unique_ptr<Impl, Deleter> m_impl{};
};

void print(Level level, std::string_view category, std::string_view message);

void print(Level level, std::string_view category, std::string_view function, std::string_view filePath, int curLine, std::string_view message);
} // namespace fornani::io::logger

namespace fornani::io {
class Logger {
  public:
	using Level = logger::Level;

	explicit Logger(std::string_view category);

	template <typename... Args>
	void error(std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eError, m_category, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void verbose_error(std::string_view function, std::string_view filePath, int curLine, std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eError, m_category, function, filePath, curLine, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void warn(std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eWarn, m_category, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void verbose_warn(std::string_view function, std::string_view filePath, int curLine, std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eWarn, m_category, function, filePath, curLine, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void info(std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eInfo, m_category, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void verbose_info(std::string_view function, std::string_view filePath, int curLine, std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eInfo, m_category, function, filePath, curLine, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void log(std::format_string<Args...> fmt, Args&&... args) const {
		info(fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void verbose_log(std::string_view function, std::string_view filePath, int curLine, std::format_string<Args...> fmt, Args&&... args) const {
		verbose_info(function, filePath, curLine, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void debug(std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eDebug, m_category, std::format(fmt, std::forward<Args>(args)...));
	}

	template <typename... Args>
	void verbose_debug(std::string_view function, std::string_view filePath, int curLine, std::format_string<Args...> fmt, Args&&... args) const {
		logger::print(Level::eDebug, m_category, function, filePath, curLine, std::format(fmt, std::forward<Args>(args)...));
	}

  private:
	std::string_view m_category{};
};

namespace logger {
inline Logger const general{"general"};
} // namespace logger
} // namespace fornani::io

// NOLINTBEGIN
#define INTERNAL_NANI_LOG(logger, level, message, ...)                                                                                                                                                                                         \
	do { logger.verbose_##level(__func__, __FILE__, __LINE__, message, __VA_ARGS__); } while ((void)0, 0)

#define NANI_LOG(logger, message, ...) INTERNAL_NANI_LOG(logger, log, message, __VA_ARGS__)
#define NANI_LOG_ERROR(logger, message, ...) INTERNAL_NANI_LOG(logger, error, message, __VA_ARGS__)
#define NANI_LOG_WARN(logger, message, ...) INTERNAL_NANI_LOG(logger, warn, message, __VA_ARGS__)
#define NANI_LOG_INFO(logger, message, ...) INTERNAL_NANI_LOG(logger, info, message, __VA_ARGS__)
#define NANI_LOG_DEBUG(logger, message, ...) INTERNAL_NANI_LOG(logger, debug, message, __VA_ARGS__)
// NOLINTEND
