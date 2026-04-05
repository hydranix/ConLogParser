#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "conlog/events/Events.hpp"

namespace conlog::events {

using SubscriberId = std::uint64_t;

class EventBus {
public:
    template<typename E>
    [[nodiscard]] SubscriberId subscribe(std::function<void(const E&)> handler)
    {
        auto id = m_next_id.fetch_add(1, std::memory_order_relaxed);
        auto key = std::type_index(typeid(E));
        auto wrapper = [h = std::move(handler)](const GameEvent& ev) {
            if (const auto* p = std::get_if<E>(&ev)) {
                h(*p);
            }
        };

        std::unique_lock lock{m_mutex};
        m_subscribers[key].push_back(Subscriber{id, std::move(wrapper)});
        return id;
    }

    template<typename E>
    void publish(E event)
    {
        publish_impl(GameEvent{std::move(event)});
    }

    void unsubscribe(SubscriberId id)
    {
        std::unique_lock lock{m_mutex};
        for (auto& [key, subs] : m_subscribers) {
            std::erase_if(subs, [id](const Subscriber& s) {
                return s.id == id;
            });
        }
    }

private:
    struct Subscriber {
        SubscriberId id;
        std::function<void(const GameEvent&)> handler;
    };

    void publish_impl(GameEvent event)
    {
        auto key = std::visit(
            [](const auto& ev) { return std::type_index(typeid(ev)); },
            event
        );

        std::vector<Subscriber> snapshot;
        {
            std::shared_lock lock{m_mutex};
            auto it = m_subscribers.find(key);
            if (it == m_subscribers.end()) {
                return;
            }
            snapshot = it->second;
        }

        for (const auto& sub : snapshot) {
            sub.handler(event);
        }
    }

    std::atomic<SubscriberId> m_next_id{1};
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::type_index, std::vector<Subscriber>> m_subscribers;
};

} // namespace conlog::events
