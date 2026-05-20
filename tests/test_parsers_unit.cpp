// Per-parser unit tests. Each test owns its own EventBus + parser instance and
// feeds a small set of hand-crafted lines (positive + negative cases) through it.

#include "EventCounter.hpp"
#include "test_helpers.hpp"

#include <conlog/events/EventBus.hpp>
#include <conlog/parser/parsers/ChatParser.hpp>
#include <conlog/parser/parsers/DamageParser.hpp>
#include <conlog/parser/parsers/KillDeathParser.hpp>
#include <conlog/parser/parsers/NoscopeParser.hpp>
#include <conlog/parser/parsers/PingParser.hpp>
#include <conlog/parser/parsers/PlayerConnectedParser.hpp>
#include <conlog/parser/parsers/StickyNadesParser.hpp>
#include <conlog/parser/parsers/SuicideParser.hpp>

#include <string>
#include <vector>

using namespace conlog;

TEST(KillDeathParser_basic_kill_publishes_kill_and_death)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    KillDeathParser parser{bus};

    parser("CASH killed Bloodlust with g3sg1.");

    CHECK_EQ(counter.kills.size(), 1u);
    CHECK_EQ(counter.deaths.size(), 1u);
    CHECK_EQ(counter.kills[0].killer, std::string{"CASH"});
    CHECK_EQ(counter.kills[0].victim, std::string{"Bloodlust"});
    CHECK_EQ(counter.kills[0].weapon, std::string{"g3sg1"});
    CHECK(!counter.kills[0].headshot);
    CHECK_EQ(counter.deaths[0].victim, std::string{"Bloodlust"});
    CHECK_EQ(counter.deaths[0].killer, std::string{"CASH"});
}

TEST(KillDeathParser_headshot_flag)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    KillDeathParser parser{bus};

    parser("Andy killed Bob with awp. (headshot)");

    CHECK_EQ(counter.kills.size(), 1u);
    CHECK(counter.kills[0].headshot);
    CHECK_EQ(counter.kills[0].weapon, std::string{"awp"});
}

TEST(KillDeathParser_ignores_chat_about_killing)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    KillDeathParser parser{bus};

    parser("Amy :  yay I killed a snake");
    parser("nara :  example: \"nara killed Runtime with deagle.\"");
    parser("");
    parser("some random line");

    CHECK_EQ(counter.kills.size(), 0u);
    CHECK_EQ(counter.deaths.size(), 0u);
}

TEST(DamageParser_given_and_taken)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    DamageParser parser{bus};

    parser("Damage Given to \"Runtime\" - 108 in 3 hits");
    parser("Damage Taken from \"Greasy_Monkey_Nutz!!\" - 16 in 1 hit");

    CHECK_EQ(counter.damages.size(), 2u);
    CHECK(counter.damages[0].given);
    CHECK_EQ(counter.damages[0].other_player, std::string{"Runtime"});
    CHECK_EQ(counter.damages[0].damage, 108);
    CHECK_EQ(counter.damages[0].hits, 3);

    CHECK(!counter.damages[1].given);
    CHECK_EQ(counter.damages[1].other_player, std::string{"Greasy_Monkey_Nutz!!"});
    CHECK_EQ(counter.damages[1].damage, 16);
    CHECK_EQ(counter.damages[1].hits, 1);
}

TEST(DamageParser_ignores_summary_header)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    DamageParser parser{bus};

    parser("Player: xıuɐɹpʎH - Damage Given");
    parser("Player: xıuɐɹpʎH - Damage Taken");
    parser("-------------------------");

    CHECK_EQ(counter.damages.size(), 0u);
}

TEST(NoscopeParser_plain_and_headshot)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    NoscopeParser parser{bus};

    parser("\xe2\x98\x86 George Noscope.");
    parser("\xe2\x98\x86 Lord Scoutah\xe2\x99\xa4 Noscope + Headshot.");

    CHECK_EQ(counter.noscopes.size(), 2u);
    CHECK_EQ(counter.noscopes[0].player, std::string{"George"});
    CHECK(!counter.noscopes[0].headshot);
    CHECK(counter.noscopes[1].headshot);
}

TEST(NoscopeParser_ignores_unrelated)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    NoscopeParser parser{bus};

    parser("George killed Bob with awp.");
    parser("");

    CHECK_EQ(counter.noscopes.size(), 0u);
}

TEST(StickyNadesParser_basic)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    StickyNadesParser parser{bus};

    parser("[StickyNades] Greasy_Monkey_Nutz!! stuck CASH with a Frag Grenade!");

    CHECK_EQ(counter.stickies.size(), 1u);
    CHECK_EQ(counter.stickies[0].thrower, std::string{"Greasy_Monkey_Nutz!!"});
    CHECK_EQ(counter.stickies[0].victim, std::string{"CASH"});
}

TEST(StickyNadesParser_tolerates_color_code_control_bytes)
{
    // Real CS:S sticky lines arrive with \001 \v \004 \005 interleaved.
    events::EventBus bus;
    test::EventCounter counter{bus};
    StickyNadesParser parser{bus};

    parser("\x01\v\x04[StickyNades] \x05"
           "Greasy_Monkey_Nutz!!\x01 stuck \x05"
           "CASH\x01 with a \x04"
           "Frag Grenade\x01!");

    CHECK_EQ(counter.stickies.size(), 1u);
    CHECK_EQ(counter.stickies[0].thrower, std::string{"Greasy_Monkey_Nutz!!"});
    CHECK_EQ(counter.stickies[0].victim, std::string{"CASH"});
}

TEST(SuicideParser_suicided_and_died)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    SuicideParser parser{bus};

    parser("phoad suicided.");
    parser("Bloodlust died.");
    parser("not a suicide line");

    CHECK_EQ(counter.suicides.size(), 2u);
    CHECK_EQ(counter.suicides[0].player, std::string{"phoad"});
    CHECK_EQ(counter.suicides[1].player, std::string{"Bloodlust"});
}

TEST(PlayerConnectedParser_basic)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    PlayerConnectedParser parser{bus};

    parser("xıuɐɹpʎH connected.");
    parser("that one lock step guy(dr exp) connected.");
    parser("connected.");  // edge: '.+' requires at least one char before " connected."

    CHECK_GE(counter.connects.size(), 2u);
    CHECK_EQ(counter.connects[0].player, std::string{"xıuɐɹpʎH"});
}

TEST(ChatParser_basic_and_spec)
{
    events::EventBus bus;
    test::EventCounter counter{bus};
    ChatParser parser{bus};

    parser("CASH :  yes");
    parser("*SPEC* xıuɐɹpʎH :  should i go T  or NotT?");

    CHECK_EQ(counter.chats.size(), 2u);
    CHECK_EQ(counter.chats[0].player, std::string{"CASH"});
    CHECK_EQ(counter.chats[0].message, std::string{"yes"});
    CHECK_EQ(counter.chats[1].player, std::string{"*SPEC* xıuɐɹpʎH"});
    CHECK_EQ(counter.chats[1].message, std::string{"should i go T  or NotT?"});
}

TEST(PingParser_collects_until_first_non_match)
{
    std::vector<std::string> collected;
    PingParser parser{[&](std::vector<std::string> names) {
        collected = std::move(names);
    }};

    parser("Client ping times:");
    parser("98 ms : xıuɐɹpʎH");
    parser("37 ms : Dr3w");
    parser("40 ms : 67 Stang");
    parser("Bloodlust killed Eliza with ak47.");  // terminator

    CHECK_EQ(collected.size(), 3u);
    CHECK_EQ(collected[0], std::string{"xıuɐɹpʎH"});
    CHECK_EQ(collected[1], std::string{"Dr3w"});
    CHECK_EQ(collected[2], std::string{"67 Stang"});

    // After completion further lines are no-ops; callback must not refire.
    parser("99 ms : Ghost");
    CHECK_EQ(collected.size(), 3u);
}

int main()
{
    return test::run_all();
}
