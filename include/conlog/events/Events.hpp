#pragma once

#include <string>
#include <variant>

namespace conlog::events {

struct KillEvent {
    std::string killer;
    std::string victim;
    std::string weapon;
    bool headshot{false};
};

struct DeathEvent {
    std::string victim;
    std::string killer;
    std::string weapon;
};

struct DamageEvent {
    std::string other_player; // who damage was given to or taken from
    int         damage{0};
    int         hits{0};
    bool        given{true};  // true = "Damage Given to", false = "Damage Taken from"
};

struct NoscopeEvent {
    std::string player;
    bool        headshot{false};
};

struct StickyNadeStuckEvent {
    std::string thrower;
    std::string victim;
};

struct SuicideEvent {
    std::string player;
};

struct PlayerConnectedEvent {
    std::string player;
};

struct ChatMessageEvent {
    std::string player;
    std::string message;
};

using GameEvent = std::variant<KillEvent, DeathEvent, DamageEvent,
                               NoscopeEvent, StickyNadeStuckEvent,
                               SuicideEvent, PlayerConnectedEvent,
                               ChatMessageEvent>;

} // namespace conlog::events
