// End-to-end test: drive every default parser through the Dispatcher against the
// captured CS:S console log and verify each event type fires at least the
// minimum expected count. Minimums come from grep -c against the same sample
// (see tests/data/conlog_sample.txt).

#include "EventCounter.hpp"
#include "test_helpers.hpp"

#include <conlog/core/Dispatcher.hpp>
#include <conlog/events/EventBus.hpp>
#include <conlog/parser/ParserRegistry.hpp>
#include <conlog/parser/parsers/ChatParser.hpp>
#include <conlog/parser/parsers/DamageParser.hpp>
#include <conlog/parser/parsers/KillDeathParser.hpp>
#include <conlog/parser/parsers/NoscopeParser.hpp>
#include <conlog/parser/parsers/PlayerConnectedParser.hpp>
#include <conlog/parser/parsers/StickyNadesParser.hpp>
#include <conlog/parser/parsers/SuicideParser.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <print>
#include <string>

#ifndef CONLOG_TEST_SAMPLE_PATH
#error "CONLOG_TEST_SAMPLE_PATH must be defined by the build system"
#endif

using namespace conlog;

namespace {

struct LoadedSample {
    std::vector<std::string> lines;
};

LoadedSample load_sample()
{
    LoadedSample out;
    std::ifstream in{CONLOG_TEST_SAMPLE_PATH};
    if (!in) {
        std::println(stderr, "Failed to open sample at {}", CONLOG_TEST_SAMPLE_PATH);
        return out;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        out.lines.push_back(std::move(line));
    }
    return out;
}

} // namespace

TEST(Sample_file_exists_and_nonempty)
{
    auto sample = load_sample();
    CHECK_GE(sample.lines.size(), 1000u);
}

TEST(Dispatcher_runs_all_default_parsers_over_sample)
{
    events::EventBus bus;
    test::EventCounter counter{bus};

    ParserRegistry registry;
    registry.register_parser("kill_death",
        std::make_shared<KillDeathParser>(bus));
    registry.register_parser("damage",
        std::make_shared<DamageParser>(bus));
    registry.register_parser("noscope",
        std::make_shared<NoscopeParser>(bus));
    registry.register_parser("sticky_nades",
        std::make_shared<StickyNadesParser>(bus));
    registry.register_parser("suicide",
        std::make_shared<SuicideParser>(bus));
    registry.register_parser("player_connected",
        std::make_shared<PlayerConnectedParser>(bus));
    registry.register_parser("chat",
        std::make_shared<ChatParser>(bus));

    Dispatcher dispatcher{registry};

    auto sample = load_sample();
    for (const auto& line : sample.lines) {
        dispatcher.dispatch(line);
    }

    std::println("Events fired:");
    std::println("  kills      = {}", counter.kills.size());
    std::println("  deaths     = {}", counter.deaths.size());
    std::println("  damages    = {}", counter.damages.size());
    std::println("  noscopes   = {}", counter.noscopes.size());
    std::println("  stickies   = {}", counter.stickies.size());
    std::println("  suicides   = {}", counter.suicides.size());
    std::println("  connects   = {}", counter.connects.size());
    std::println("  chats      = {}", counter.chats.size());

    // Lower bounds: derived from grep -c on tests/data/conlog_sample.txt.
    // Using >= rather than == leaves room for regex tightening without
    // breaking the test, while still proving every parser fires.
    CHECK_GE(counter.kills.size(),    13000u);  // grep: 13574 strict matches
    CHECK_GE(counter.deaths.size(),   13000u);  // 1:1 with kills
    CHECK_GE(counter.damages.size(),   1900u);  // grep (strict): 1983
    CHECK_GE(counter.noscopes.size(),   390u);  // grep: 397
    CHECK_GE(counter.stickies.size(),   240u);  // grep: 245
    CHECK_GE(counter.suicides.size(),   110u);  // grep: 98 suicided + 17 died
    CHECK_GE(counter.connects.size(),   280u);  // grep: 286
    CHECK_GE(counter.chats.size(),     1000u);  // grep: 1092

    // Spot-check a few headshot kills exist (the sample log has none of the
    // " (headshot)" suffix variant, but it does have noscope headshots).
    std::size_t noscope_headshots = 0;
    for (const auto& n : counter.noscopes) if (n.headshot) ++noscope_headshots;
    CHECK_GE(noscope_headshots, 60u);  // grep: 63
}

int main()
{
    return test::run_all();
}
