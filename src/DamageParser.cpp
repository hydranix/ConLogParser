#include <conlog/parser/parsers/DamageParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <charconv>
#include <regex>
#include <string>
#include <system_error>

namespace conlog {

DamageParser::DamageParser(events::EventBus& bus)
    : m_bus(bus)
    // Matches:
    //   Damage Given to "PlayerName" - 52 in 2 hits
    //   Damage Taken from "PlayerName" - 120 in 5 hits
    , m_pattern(R"re(^Damage (Given to|Taken from) "([^"]+)" - (\d+) in (\d+) hits?$)re")
{}

void DamageParser::operator()(std::string_view line)
{
    if (!line.starts_with("Damage ")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    auto to_int = [](const std::string& str) -> int {
        int val = 0;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
        if (ec != std::errc{}) return 0;
        return val;
    };

    m_bus.publish(events::DamageEvent{
        .other_player = m[2].str(),
        .damage       = to_int(m[3].str()),
        .hits         = to_int(m[4].str()),
        .given        = (m[1].str() == "Given to"),
    });
}

} // namespace conlog
