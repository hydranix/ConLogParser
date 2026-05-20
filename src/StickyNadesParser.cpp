#include <conlog/parser/parsers/StickyNadesParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <algorithm>
#include <regex>
#include <string>

namespace conlog {

StickyNadesParser::StickyNadesParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^\[StickyNades\] (.+) stuck (.+) with a Frag Grenade!$)")
{}

void StickyNadesParser::operator()(std::string_view line)
{
    // Sticky lines arrive with CS:S color-code control bytes interleaved
    // (e.g. \001 \v \004 \005). Substring check tolerates them.
    if (line.find("StickyNades]") == std::string_view::npos) return;

    // Strip control bytes in [0x01, 0x20) before regex matching. UTF-8
    // continuation bytes (>= 0x80) and printable ASCII are preserved.
    std::string s{line};
    std::erase_if(s, [](unsigned char c) { return c > 0 && c < 0x20; });

    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    m_bus.publish(events::StickyNadeStuckEvent{
        .thrower = m[1].str(),
        .victim  = m[2].str()
    });
}

} // namespace conlog
