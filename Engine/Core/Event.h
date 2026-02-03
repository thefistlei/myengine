/******************************************************************************
 * File: Event.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Event system base classes and type definitions
 * Dependencies: <string>, <functional>
 ******************************************************************************/

#pragma once

#include <string>
#include <functional>
#include <cstdint>

namespace MyEngine {

/**
 * @brief Event type enumeration
 */
enum class EventType {
    None = 0,
    // Window events
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    // Application events
    AppTick, AppUpdate, AppRender,
    // Keyboard events
    KeyPressed, KeyReleased, KeyTyped,
    // Mouse events
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

/**
 * @brief Event category flags (can be combined with bitwise OR)
 */
enum EventCategory {
    None = 0,
    EventCategoryApplication = 1 << 0,
    EventCategoryInput       = 1 << 1,
    EventCategoryKeyboard    = 1 << 2,
    EventCategoryMouse       = 1 << 3,
    EventCategoryMouseButton = 1 << 4
};

/**
 * @brief Macro to implement GetStaticType() and GetEventType()
 */
#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char* GetName() const override { return #type; }

/**
 * @brief Macro to implement GetCategoryFlags()
 */
#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

/**
 * @brief Base event class
 */
class Event {
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    inline bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & category;
    }
};

/**
 * @brief Event dispatcher - dispatches events to handler functions
 */
class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_Event(event) {}

    /**
     * @brief Dispatch event to a handler function
     * @tparam T Event type
     * @tparam F Function type
     * @param func Handler function that returns bool (true if handled)
     * @return true if event was handled
     */
    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.Handled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

/**
 * @brief Event callback function type
 */
using EventCallbackFn = std::function<void(Event&)>;

} // namespace MyEngine
