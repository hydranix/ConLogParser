#include <conlog/parser/parsers/NoscopeParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

NoscopeParser::NoscopeParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^)" "\xe2\x98\x86" R"( (.+) Noscope(?: \+ Headshot)?\.?$)")
{}

void NoscopeParser::operator()(std::string_view line)
{
    if (!line.starts_with("\xe2\x98\x86 ")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    bool headshot = s.contains("+ Headshot");
    m_bus.publish(events::NoscopeEvent{
        .player   = m[1].str(),
        .headshot = headshot
    });
}

} // namespace conlog
