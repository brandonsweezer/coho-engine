#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <any>
#include <atomic>
#include <iostream>

template <typename EventType>
class EventSubscriber {
public:
    using Listener = std::function<void()>;

    int subscribe(EventType eventType, const Listener& listener) {
        int id = nextId++;
        auto& listeners = eventListeners[eventType];
        listeners.push_back({id, listener});

        return id;
    }

    void unsubscribe(EventType eventType, int id) {
        auto& listeners = eventListeners[eventType];
        listeners.erase(
            std::remove_if(
                listeners.begin(),
                listeners.end(),
                [id](const ListenerWrapper& lw) {
                    return lw.id == id;
                }),
            listeners.end()
        );
    }

    void emit(EventType eventType) {
        std::vector<ListenerWrapper>& listeners = eventListeners[eventType];
        for (ListenerWrapper listenerWrapper : listeners) {
            listenerWrapper.listener();
        }
    }

private:
    struct ListenerWrapper {
        int id;
        Listener listener;
    };

    std::unordered_map<EventType, std::vector<ListenerWrapper>> eventListeners;

    std::atomic<int> nextId{0};

};