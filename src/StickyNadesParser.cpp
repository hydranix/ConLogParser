#include <conlog/parser/parsers/StickyNadesParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

StickyNadesParser::StickyNadesParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^\[StickyNades\] (.+) stuck (.+) with a Frag Grenade!$)")
{}

void StickyNadesParser::operator()(std::string_view line)
{
    if (!line.starts_with("[StickyNades] ")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    m_bus.publish(events::StickyNadeStuckEvent{
        .thrower = m[1].str(),
        .victim  = m[2].str()
    });
}

} // namespace conlog
