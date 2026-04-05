#include <conlog/parser/parsers/KillDeathParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

KillDeathParser::KillDeathParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^(.+) killed (.+) with (\S+?)\.( \(headshot\))?$)")
{}

void KillDeathParser::operator()(std::string_view line)
{
    if (!line.contains(" killed ")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    bool headshot = m[4].matched;
    m_bus.publish(events::KillEvent{
        .killer   = m[1].str(),
        .victim   = m[2].str(),
        .weapon   = m[3].str(),
        .headshot = headshot
    });
    m_bus.publish(events::DeathEvent{
        .victim = m[2].str(),
        .killer = m[1].str(),
        .weapon = m[3].str()
    });
}

} // namespace conlog
