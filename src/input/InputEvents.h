#pragma once

typedef enum InputEvent
{
    MOUSE_UP = 0,
    MOUSE_DOWN,

    MOUSE_MOVE,

    KEY_UP,
    KEY_DOWN,

    BUTTON_UP,
    BUTTON_DOWN,

    SCROLL,
} InputEvent;