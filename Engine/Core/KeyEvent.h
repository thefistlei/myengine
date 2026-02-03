/******************************************************************************
 * File: KeyEvent.h
 * Author: AI Assistant
 * Created: 2026-01-27
 * Description: Keyboard input events
 * Dependencies: Event.h, Platform/Input.h
 ******************************************************************************/

#pragma once

#include "Event.h"
#include "../Platform/Input.h"
#include <sstream>

namespace MyEngine {

/**
 * @brief Base class for keyboard events
 */
class KeyEvent : public Event {
public:
    KeyCode GetKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}

    KeyCode m_KeyCode;
};

/**
 * @brief Key pressed event
 */
class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(KeyCode keycode, bool isRepeat = false)
        : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

    bool IsRepeat() const { return m_IsRepeat; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << static_cast<int>(m_KeyCode) 
           << " (repeat = " << m_IsRepeat << ")";
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_IsRepeat;
};

/**
 * @brief Key released event
 */
class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << static_cast<int>(m_KeyCode);
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

/**
 * @brief Key typed event (character input)
 */
class KeyTypedEvent : public KeyEvent {
public:
    KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}

    std::string ToString() const override {
        std::stringstream ss;
        ss << "KeyTypedEvent: " << static_cast<int>(m_KeyCode);
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace MyEngine
