#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <print>
#include <string>
#include <string_view>
#include <utility>

namespace ome {

#define LOG_LEVELS(X)                                                                              \
    /* Name | ANSI */                                                                              \
    X(Debug, "90m")                                                                                \
    X(Info, "96m")                                                                                 \
    X(Warning, "33m")                                                                              \
    X(Error, "31m")

#define X(name, _) name,
enum class LogLevel
{
    LOG_LEVELS(X)
};
#undef X

static constexpr const char *
ansi_prefix_of(LogLevel level)
{
    switch (level)
    {
#define X(name, color)                                                                             \
    case LogLevel::name:                                                                           \
        return "\033[" color;
        LOG_LEVELS(X)
#undef X
    default:
        return "\033[0m";
    }
}

static constexpr std::string_view
name_of(LogLevel level)
{
    switch (level)
    {
#define X(name, _)                                                                                 \
    case LogLevel::name:                                                                           \
        return #name;
        LOG_LEVELS(X)
#undef X
    default:
        return "?";
    }
}

inline std::string
upper_case(std::string_view s)
{
    std::string out(s);
    std::ranges::transform(
        out, out.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
}

static std::FILE *
output_stream_of(LogLevel level)
{
    return (level < LogLevel::Warning) ? stdout : stderr;
}

class Logger
{
  public:
    virtual ~Logger() = default;

    virtual void
    log(const std::string &message, LogLevel level = LogLevel::Info)
        = 0;
};

class ConsoleLogger : public Logger
{
  public:
    struct Settings
    {
        bool     include_level     = true;
        bool     include_timestamp = true;
        LogLevel min_level         = LogLevel::Debug;
    };

    Settings settings{};

    ConsoleLogger() = default;

    explicit ConsoleLogger(Settings settings)
        : settings(std::move(settings))
    {
    }

    void
    log(const std::string &message, LogLevel level) override
    {
        if (level < settings.min_level)
        {
            return;
        }

        using namespace std::chrono;

        auto now  = system_clock::now();
        auto time = floor<seconds>(now);

        auto *stream = output_stream_of(level);

        std::string out;
        out.reserve(message.size() + 128);

        std::string_view ansi_prefix = ansi_prefix_of(level);
        std::string      level_name  = upper_case(name_of(level));

        auto it = std::back_inserter(out);

        if (settings.include_level)
        {
            it = std::format_to(it, "{}[{}]\033[0m ", ansi_prefix, level_name);
        }

        if (settings.include_timestamp)
        {
            it = std::format_to(it, "[{:%H:%M:%S}] ", time);
        }

        std::format_to(it, "{}\n", message);

        std::print(stream, "{}", out);
    }
};

inline auto
address_string(auto *pointer)
{
    return std::format("{:x}", reinterpret_cast<std::uintptr_t>(pointer));
}

} // namespace ome
