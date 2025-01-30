#include "../../include/fornani/io/Logger.hpp"
#include <imgui.h>


namespace util {

void Logger::add_log(char const* message) {
	if (!message) {
		return; // Ensure `message` is not null to avoid undefined behavior.
	}
	std::string msg = message;
	msg += "\n";
	log.add_log(msg.c_str());
}

void Logger::write_console(ImVec2 size, ImVec2 pos) {
	ImGui::SetNextWindowBgAlpha(0.65f);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(size);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Console", NULL, window_flags)) { log.draw("Console", NULL); }
	ImGui::End();
}

} // namespace util

#include "../../include/fornani/io/Logger.hpp"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>

#if defined(_WIN32)
	#define WINDOWS_MEAN_AND_LEAN
    #include "Windows.h" // for OutputDebugStringA
#endif


namespace fornani::io::logger
{
    ThreadId Context::getThreadId()
    {
        auto const get_next_id = []
        {
            static auto s_prevId{std::atomic<int>{}};
            return s_prevId++;
        };
        thread_local auto const s_thisThreadId{get_next_id()};
        return ThreadId{s_thisThreadId};
    }

    Context Context::make(std::string_view category, Level level)
    {
        return Context{
            .category  = category,
            .timestamp = Clock::now(),
            .thread    = getThreadId(),
            .level     = level,
        };
    }

    Context Context::make(std::string_view category, Level level, std::string_view function, std::string_view filePath, int currentLine)
    {
        return Context{
            .category  = category,
            .timestamp = Clock::now(),
            .thread    = getThreadId(),
            .level     = level,
            .func      = function,
            .file      = filePath,
            .line      = currentLine,
        };
    }
} // namespace fornani::logger

namespace fornani::io::logger
{
    namespace
    {
        namespace fs = std::filesystem;

        void append_timestamp(std::string & out, Clock::time_point const & timestamp, Timestamp const mode)
        {
            static auto s_mutex{std::mutex{}};
            static constexpr std::size_t buf_size_v{64};
            auto buffer     = std::array<char, buf_size_v>{};
            auto const time = Clock::to_time_t(timestamp);

            auto lock = std::unique_lock{s_mutex};
            auto const tm_struct = mode == Timestamp::eUtc ? *std::gmtime(&time) : *std::localtime(&time);
            lock.unlock();
            std::strftime(buffer.data(), buffer.size(), "%F %T", &tm_struct);
            out.append(buffer.data());
        }

        struct Formatter
        {
            static constexpr auto open_v{'{'};
            static constexpr auto close_v{'}'};

            struct Data
            {
                FixedString<Config::format_size_v> format{};
                Timestamp timestamp{};
            };

            Data const & data;
            std::string_view message;
            Context const & context;

            std::string out{};
            std::string_view format{data.format};
            char current{};

            [[nodiscard]] bool at_end() const { return format.empty(); }

            bool advance()
            {
                if (at_end())
                {
                    current = {};
                    return false;
                }
                current = format.front();
                format  = format.substr(1);
                return true;
            }

            bool try_keyword()
            {
                assert(current == open_v);
                auto const close = format.find_first_of(close_v);
                if (close == std::string_view::npos) { return false; }

                auto const key = format.substr(0, close);
                if (!keyword(key)) { return false; }

                format = format.substr(close + 1);
                return true;
            }

            bool keyword(std::string_view key)
            {
                if (key == "level")
                {
                    out += levelChar(context.level);
                    return true;
                }

                if (key == "thread")
                {
                    std::format_to(std::back_inserter(out), "{}", static_cast<int>(context.thread));
                    return true;
                }

                if (key == "category")
                {
                    out += context.category;
                    return true;
                }

                if (key == "message")
                {
                    out.append(message);
                    return true;
                }

                if (key == "timestamp")
                {
                    append_timestamp(out, context.timestamp, data.timestamp);
                    return true;
                }

                if (key == "func")
                {
                    if (context.func.has_value()) { out += context.func.value(); }
                    return true;
                }

                if (key == "file")
                {
                    if (context.file.has_value()) { out += context.file.value(); }
                    return true;
                }

                if (key == "line")
                {
                    if (context.line.has_value()) { std::format_to(std::back_inserter(out), "{}", context.line.value()); }
                    return true;
                }

                return false;
            }

            std::string operator()()
            {
                static constexpr std::size_t reserve_v{128};
                out.reserve(message.size() + reserve_v);
                while (advance())
                {
                    if (current == open_v && try_keyword()) { continue; }
                    out += current;
                }
                out.append("\n");
                return std::move(out);
            }
        };
    } // namespace

    namespace
    {
        struct ConsoleSink : Sink
        {
            void handle(std::string_view const formatted, Context const & context) final
            {
                auto & stream = context.level == Level::eError ? std::cerr : std::cout;
                stream << formatted;
#if defined(_WIN32)
                OutputDebugStringA(formatted.data());
#endif
            }
        };

        struct FileSink : Sink
        {
            std::string path{};
            std::mutex mutex{};
            std::string buffer{};

            std::condition_variable_any cv{};
            std::jthread thread{};

            FileSink(std::string file_path) : path(std::move(file_path)), thread([this](std::stop_token const & stop) { run(stop); }) {}

            void run(std::stop_token const & stop)
            {
                if (fs::exists(path)) { fs::remove(path); }
                while (!stop.stop_requested())
                {
                    auto lock = std::unique_lock{mutex};
                    cv.wait(lock, stop, [this] { return !buffer.empty(); });
                    if (buffer.empty()) { continue; }
                    if (auto file = std::ofstream{path, std::ios::binary | std::ios::app})
                    {
                        file << buffer;
                        buffer.clear();
                    }
                }
            }

            void handle(std::string_view const formatted, [[maybe_unused]] Context const & context) final
            {
                auto lock = std::unique_lock{mutex};
                buffer.append(formatted);
                lock.unlock();
                cv.notify_one();
            }
        };
    } // namespace

    struct Instance::Impl
    {
        std::vector<std::unique_ptr<Sink>> sinks{};
        Config config{};
        std::mutex mutex{};

        ConsoleSink console{};
        FileSink file;

        static char const * nonEmptyFilePath(char const * input)
        {
            if (input == nullptr || *input == 0) { return "genesis.log"; }
            return input;
        }

        Impl(char const * filePath) : file(nonEmptyFilePath(filePath)) {}

        void print(std::string_view const message, Context const & context)
        {
            auto lock = std::unique_lock{mutex};
            if (auto const itr = config.categoryMaxLevels.find(context.category); itr != config.categoryMaxLevels.end())
            {
                if (context.level > itr->second) { return; }
            }
            else if (context.level > config.maxLevel) { return; }
            auto const target = [&]
            {
                if (auto const itr = config.levelTargets.find(context.level); itr != config.levelTargets.end()) { return itr->second; }
                return all_v;
            }();

#ifdef FORNANI_VERBOSE_LOGGING
            config.format = Config::verbose_format_v;
#endif

            auto const data = Formatter::Data{.format = config.format, .timestamp = config.timestamp};
            auto const sinks_empty = sinks.empty();
            lock.unlock();

            auto const formatted = Formatter{.data = data, .message = message, .context = context}();

            if ((target & console_v) == console_v) { console.handle(formatted, context); }
            if ((target & file_v) == file_v) { file.handle(formatted, context); }

            if ((target & sinks_v) == sinks_v && !sinks_empty)
            {
                lock.lock();
                for (auto const & sink : sinks) { sink->handle(formatted, context); }
            }
        }
    };

    void Instance::Deleter::operator()(Impl const * ptr) const
    {
        delete ptr;
    }

    Instance::Instance(char const * filePath, Config config) : m_impl(new Impl{filePath})
    {
        if (s_instance != nullptr) { throw DuplicateError{"Duplicate logger Instance"}; }
        s_instance = m_impl.get();

        m_impl->config = std::move(config);

        m_impl->print(std::format("logging to file: {}", filePath), Context::make("logger", Level::eInfo));
    }

    Instance::~Instance()
    {
        s_instance = {};
    }

    Config Instance::getConfig() const
    {
        assert(m_impl);
        auto lock = std::scoped_lock{m_impl->mutex};
        return m_impl->config;
    }

    void Instance::setConfig(Config config) const {
        assert(m_impl);
        auto lock    = std::scoped_lock{m_impl->mutex};
        m_impl->config = std::move(config);
    }

    void Instance::addSink(std::unique_ptr<Sink> sink) const {
        if (!sink) { return; }
        assert(m_impl != nullptr);
        auto lock = std::scoped_lock{m_impl->mutex};
        m_impl->sinks.push_back(std::move(sink));
    }

    void Instance::print(std::string_view const message, Context const & context)
    {
        if (s_instance == nullptr) { return; }
        s_instance->print(message, context);
    }
} // namespace fornani::logger

namespace fornani::io
{
    void logger::print(logger::Level level, std::string_view category, std::string_view message)
    {
        Instance::print(message, Context::make(category, level));
    }

    void logger::print(
        Level level, std::string_view category, std::string_view function, std::string_view filePath, int curLine, std::string_view message)
    {
        Instance::print(message, Context::make(category, level, function, filePath, curLine));
    }

    Logger::Logger(std::string_view const category) : m_category(category.empty() ? "unknown" : category)
    {
    }
} // namespace fornani
