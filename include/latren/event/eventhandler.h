#pragma once

#include <latren/defines/opengl.h>
#include <latren/util/idfactory.h>

#include <variant>
#include <unordered_map>
#include <functional>

template <typename... Params>
class SingleEventHandler : public IDFactory {
private:
    std::unordered_map<EventID, std::function<void(Params...)>> callbacks_;
public:
    void Dispatch(const Params&... params) {
        for (const auto& [hash, fn] : callbacks_) {
            fn(params...);
        }
    }
    EventID Subscribe(const std::function<void(Params...)>& fn) {
        EventID hash = NextID();
        callbacks_.insert({ hash, fn });
        return hash;
    }
    void Unsubscribe(const EventID& hash) {
        callbacks_.erase(hash);
    }
    void ClearEvents() {
        callbacks_.clear();
    }
};

template <typename E, typename Fn>
class IEventHandler : public IDFactory {
public:
    virtual EventID Subscribe(const E&, const Fn&) = 0;
    virtual void Unsubscribe(const E&, const EventID&) = 0;
    virtual void ClearEvents() = 0;
};

template <typename E, typename... Params>
class EventHandler : public IEventHandler<E, std::function<void(Params...)>> {
private:
    std::unordered_map<E, std::unordered_map<EventID, std::function<void(Params...)>>> events_;
public:
    void Dispatch(const E& e, const Params&... params) {
        for (const auto& [hash, fn] : events_[e]) {
            fn(params...);
        }
    }
    EventID Subscribe(const E& e, const std::function<void(Params...)>& fn) {
        EventID hash = NextID();
        events_[e].insert({ hash, fn });
        return hash;
    }
    void Unsubscribe(const E& e, const EventID& hash) {
        events_[e].erase(hash);
    }
    void ClearEvents() {
        events_.clear();
    }
};

template <typename E, typename... Variants>
class VariantEventHandler : public IEventHandler<E, std::variant<std::function<void()>, std::function<Variants>...>> {
typedef std::variant<std::function<void()>, std::function<Variants>...> Variant;
private:
    std::unordered_map<E, std::unordered_map<EventID, Variant>> events_;
public:
    template <typename... Params>
    void Dispatch(const E& e, Params... params) {
        for (const auto& [hash, fn] : events_[e]) {
            if (std::holds_alternative<std::function<void()>>(fn))
                std::get<std::function<void()>>(fn)();
            else
                std::get<std::function<void(Params...)>>(fn)(params...);
        }
    }
    EventID Subscribe(const E& e, const Variant& fn) {
        EventID hash = NextID();
        events_[e].insert({ hash, fn });
        return hash;
    }
    void Unsubscribe(const E& e, const EventID& hash) {
        events_[e].erase(hash);
    }
    void ClearEvents() {
        events_.clear();
    }
};