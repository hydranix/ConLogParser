#pragma once

#include <conlog/events/EventBus.hpp>
#include <conlog/events/Events.hpp>

#include <mutex>
#include <vector>

namespace test {

// Subscribes to every event variant on the supplied EventBus and tallies them.
// All counters are guarded by a mutex; safe to use under parallel dispatch.
struct EventCounter {
    explicit EventCounter(conlog::events::EventBus& bus)
    {
        using namespace conlog::events;
        (void)bus.subscribe<KillEvent>([this](const KillEvent& e) {
            std::lock_guard lk{m}; kills.push_back(e);
        });
        (void)bus.subscribe<DeathEvent>([this](const DeathEvent& e) {
            std::lock_guard lk{m}; deaths.push_back(e);
        });
        (void)bus.subscribe<DamageEvent>([this](const DamageEvent& e) {
            std::lock_guard lk{m}; damages.push_back(e);
        });
        (void)bus.subscribe<NoscopeEvent>([this](const NoscopeEvent& e) {
            std::lock_guard lk{m}; noscopes.push_back(e);
        });
        (void)bus.subscribe<StickyNadeStuckEvent>([this](const StickyNadeStuckEvent& e) {
            std::lock_guard lk{m}; stickies.push_back(e);
        });
        (void)bus.subscribe<SuicideEvent>([this](const SuicideEvent& e) {
            std::lock_guard lk{m}; suicides.push_back(e);
        });
        (void)bus.subscribe<PlayerConnectedEvent>([this](const PlayerConnectedEvent& e) {
            std::lock_guard lk{m}; connects.push_back(e);
        });
        (void)bus.subscribe<ChatMessageEvent>([this](const ChatMessageEvent& e) {
            std::lock_guard lk{m}; chats.push_back(e);
        });
    }

    mutable std::mutex m;
    std::vector<conlog::events::KillEvent>            kills;
    std::vector<conlog::events::DeathEvent>           deaths;
    std::vector<conlog::events::DamageEvent>          damages;
    std::vector<conlog::events::NoscopeEvent>         noscopes;
    std::vector<conlog::events::StickyNadeStuckEvent> stickies;
    std::vector<conlog::events::SuicideEvent>         suicides;
    std::vector<conlog::events::PlayerConnectedEvent> connects;
    std::vector<conlog::events::ChatMessageEvent>     chats;
};

} // namespace test
