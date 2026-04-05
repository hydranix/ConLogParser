#include <conlog/parser/parsers/ChatParser.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <regex>
#include <string>

namespace conlog {

ChatParser::ChatParser(events::EventBus& bus)
    : m_bus(bus)
    , m_pattern(R"(^(.+) :  (.+)$)")
{}

void ChatParser::operator()(std::string_view line)
{
    if (!line.contains(" :  ")) return; // fast pre-check

    std::string s{line};
    std::smatch m;
    if (!std::regex_match(s, m, m_pattern)) return;

    m_bus.publish(events::ChatMessageEvent{
        .player  = m[1].str(),
        .message = m[2].str()
    });
}

} // namespace conlog
