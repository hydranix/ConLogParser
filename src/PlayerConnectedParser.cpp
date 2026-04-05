#include <conlog/parser/parsers/PlayerConnectedParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

PlayerConnectedParser::PlayerConnectedParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^(.+) connected\.$)")
{}

void PlayerConnectedParser::operator()(std::string_view line)
{
    if (!line.ends_with(" connected.")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    m_bus.publish(events::PlayerConnectedEvent{
        .player = m[1].str()
    });
}

} // namespace conlog
