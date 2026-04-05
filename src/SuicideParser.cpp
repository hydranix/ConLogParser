#include <conlog/parser/parsers/SuicideParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

SuicideParser::SuicideParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^(.+) (?:suicided|died)\.$)")
{}

void SuicideParser::operator()(std::string_view line)
{
    if (!line.ends_with(" suicided.") && !line.ends_with(" died.")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    m_bus.publish(events::SuicideEvent{
        .player = m[1].str()
    });
}

} // namespace conlog
