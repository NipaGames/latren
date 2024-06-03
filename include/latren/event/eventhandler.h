#pragma once

#include <latren/defines/opengl.h>
#include <latren/util/idfactory.h>

#include <variant>
#include <unordered_map>
#include <functional>

template <typename... Params>
class SingleEventHandler : public IDFactory<> {
private:
    std::unordered_map<EventID, std::function<void(Params...)>> callbacks_;
public:
    void Dispatch(const Params&... params) const {
        for (const auto& [id, fn] : callbacks_) {
            fn(params...);
        }
    }
    EventID Subscribe(const std::function<void(Params...)>& fn) {
        EventID id = NextID();
        callbacks_.insert({ id, fn });
        return id;
    }
    void Unsubscribe(const EventID& hash) {
        callbacks_.erase(hash);
    }
    void ClearEvents() {
        callbacks_.clear();
    }
};

template <typename E, typename Fn>
class BasicEventHandler : public IDFactory<> {
protected:
    std::unordered_map<E, std::unordered_map<EventID, Fn>> events_;
public:
    virtual ~BasicEventHandler() = default;
    virtual EventID Subscribe(const E& e, const Fn& fn) {
        EventID id = NextID();
        events_[e].insert({ id, fn });
        return id;
    }
    virtual void Unsubscribe(const E& e, const EventID& id) {
        events_[e].erase(id);
    }
    virtual void ClearEvents() {
        events_.clear();
    }
};

template <typename E, typename... Params>
class EventHandler : public BasicEventHandler<E, std::function<void(Params...)>> {
public:
    void Dispatch(const E& e, const Params&... params) const {
        if (this->events_.count(e) == 0)
            return;
        for (const auto& [hash, fn] : this->events_.at(e)) {
            fn(params...);
        }
    }
};

template <typename E, typename... Variants>
class VariantEventHandler : public BasicEventHandler<E, std::variant<std::function<void()>, std::function<Variants>...>> {
typedef std::variant<std::function<void()>, std::function<Variants>...> Variant;
public:
    template <typename... Params>
    void Dispatch(const E& e, Params... params) const {
        if (this->events_.count(e) == 0)
            return;
        for (const auto& [hash, fn] : this->events_.at(e)) {
            if (std::holds_alternative<std::function<void()>>(fn))
                std::get<std::function<void()>>(fn)();
            else
                std::get<std::function<void(Params...)>>(fn)(params...);
        }
    }
};